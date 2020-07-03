
#define _GNU_SOURCE /* for pthread_mutexattr_settype */
#include <stdlib.h>
#if !defined(WIN32) && !defined(WIN64)
    #include <sys/time.h>
#endif

#include "MQTTClient.h"
#if !defined(NO_PERSISTENCE)
#include "MQTTPersistence.h"
#endif

#include "utf-8.h"
#include "MQTTProtocol.h"
#include "MQTTProtocolOut.h"
#include "Thread.h"
#include "SocketBuffer.h"
#include "StackTrace.h"
#include "Heap.h"
#include "cJSON.h"
#include "rt_type.h"
#include "md5.h"
#include "rt_os.h"
#include "log.h"
#include "http.h"
#include "rt_mqtt.h"

#if defined(OPENSSL)
#include <openssl/ssl.h>
#endif

static mqtt_reg_info_t g_mqtt_reg_info;
static mqtt_reg_url_t g_mqtt_reg_url;
static char g_mqtt_url_host_port[200];

mqtt_reg_info_t * mqtt_get_reg_info(void)
{
    return &g_mqtt_reg_info;
}

void mqtt_set_reg_url(const char url[20], int32_t port)
{
    snprintf(g_mqtt_reg_url.url, sizeof(g_mqtt_reg_url.url), "%s", url);
    g_mqtt_reg_url.port = port;
}

mqtt_reg_url_t * mqtt_get_reg_url(void)
{
    return &g_mqtt_reg_url;
}

rt_bool mqtt_get_ip_pair(const char *url, int8_t *addr, int32_t *port)
{
    char *p = (int8_t *)strstr((char *)url, "http://");
    if (p) {
        p += 7;
        char *q = (int8_t *)strstr((char *)p, ":");
        if (q) {
            int32_t len = rt_os_strlen(p) - rt_os_strlen(q);
            if (len > 0) {
                sprintf((char *)addr, "%.*s", len, p);
                *port = atoi((char *)(q + 1));
                return RT_TRUE;
            }
        }
    }
    return RT_FALSE;
}

static int32_t mqtt_get_broker_cb(const char *json_data)
{
    int32_t ret = RT_ERROR;
    cJSON *root;
   
    MSG_PRINTF(LOG_TRACE, "json_data: %s\n", json_data);
    root = cJSON_Parse(json_data);  
    
#if 0  // only for test
    {
        char *str = NULL;
        str = cJSON_Print(root);
        MSG_PRINTF(LOG_INFO, "%s\n",str);
        cJSON_free(str);
    }
#endif

    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj) {
            cJSON *url = cJSON_GetObjectItem(obj, "ip");
            cJSON *port = cJSON_GetObjectItem(obj, "port");
            if ((url != NULL) && (port != NULL)) {
                snprintf(g_mqtt_url_host_port, sizeof(g_mqtt_url_host_port), \
                                "%s:%s", url->valuestring, port->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

int32_t MQTTClient_get_host(const char *node_name, char *url, const char *appkey)
{
    int32_t ret = RT_ERROR;
    char json_data[1024];    
    
    if(!node_name){
        snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"nodeName\":\"%s\",\"appKey\":\"%s\"}", node_name, appkey);
    }

    MSG_PRINTF(LOG_TRACE, "json_data: %s\n", json_data);

#if (CFG_UPLOAD_HTTPS_ENABLE)
    ret = mqtt_https_post_json((const char *)json_data, g_mqtt_reg_url.url, g_mqtt_reg_url.port, \
                            "/clientService/getNodes", mqtt_get_broker_cb);
    #else
    ret = mqtt_http_post_json((const char *)json_data, g_mqtt_reg_url.url, g_mqtt_reg_url.port, \
                            "/clientService/getNodes", mqtt_get_broker_cb);
#endif

    if (ret < 0) {
        return RT_ERROR;
    }
    
    sprintf(url, "%s", g_mqtt_url_host_port);
    return RT_SUCCESS;
}

int32_t mqtt_http_post_json(const char *json_data, const char *host_ip, uint16_t port, const char *path, PCALLBACK cb)
{
    int32_t ret = RT_ERROR;
    int32_t socket_fd = RT_ERROR;
    char buf[BUFFER_SIZE * 4];
    char md5_out[MD5_STRING_LENGTH+1];
    char *p = NULL;
    char temp[128];
    char convert_ip[128] = {0};

    do{
        if(!json_data || !host_ip || !path) {
            MSG_PRINTF(LOG_DBG, "path json data error\n");
            break;
        }
        
        MSG_PRINTF(LOG_TRACE, "json_data:%s\n",json_data);
        get_md5_string((const char *)json_data, md5_out);
        md5_out[MD5_STRING_LENGTH] = '\0';

        http_get_ip_addr(host_ip, convert_ip);
        MSG_PRINTF(LOG_TRACE, "convert_ip:%s, port:%d\r\n", convert_ip, port);
        socket_fd = http_tcpclient_create(convert_ip, port);       // connect network
        if (socket_fd < 0) {
            ret = HTTP_SOCKET_CONNECT_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient create failed\n");
            break;
        }

        rt_os_memset(buf, 0, sizeof(buf));
        snprintf(temp, sizeof(temp), "POST %s HTTP/1.1", path);
        rt_os_strcat(buf, temp);
        rt_os_strcat(buf, "\r\n");
        snprintf(temp, sizeof(temp), "Host: %s:%d", host_ip, port),
        rt_os_strcat(buf, temp);
        rt_os_strcat(buf, "\r\n");
        rt_os_strcat(buf, "Accept: */*\r\n");
        rt_os_strcat(buf, "Content-Type: application/json;charset=UTF-8\r\n");
        rt_os_strcat(buf, "md5sum:");
        snprintf(temp, sizeof(temp), "%s\r\n", md5_out);
        rt_os_strcat(buf, temp);
        rt_os_strcat(buf, "Content-Length: ");
        snprintf(temp, sizeof(temp), "%d", rt_os_strlen(json_data)),
        rt_os_strcat(buf, temp);
        rt_os_strcat(buf, "\r\n\r\n");
        rt_os_strcat(buf, json_data);

        MSG_PRINTF(LOG_TRACE, "send buf:%s\n", buf);
        if (http_tcpclient_send(socket_fd, buf, rt_os_strlen(buf)) < 0) {      // send data
            ret = HTTP_SOCKET_SEND_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient send failed..\n");
            break;
        }

        /*it's time to recv from server*/
        rt_os_memset(buf, 0, sizeof(buf));        
        if (http_tcpclient_recv(socket_fd, buf, sizeof(buf)) <= 0) {     // recv data
            ret = HTTP_SOCKET_RECV_ERROR;
            MSG_PRINTF(LOG_WARN, "http tcpclient recv failed\n");
            break;
        } else {
        #if 0  // only for test
            const char *tmp_data =  "HTTP/1.1 502 Bad Gateway\r\n"
                                    "Server: nginx\r\n"
                                    "Date: Mon, 16 Sep 2019 09:02:08 GMT\r\n"
                                    "Content-Type: text/html\r\n"
                                    "Content-Length: 166\r\n"
                                    "Connection: keep-alive\r\n"
                                    "Keep-Alive: timeout=20\r\n"
                                    "\r\n"
                                    "<html>\r\n"
                                    "<head><title>502 Bad Gateway</title></head>\r\n"
                                    "<body bgcolor=\"white\">\r\n"
                                    "<center><h1>502 Bad Gateway</h1></center>\r\n"
                                    "<hr><center>nginx</center>\r\n"
                                    "</body>\r\n"
                                    "</html>\r\n";
            rt_os_memset(buf, 0, sizeof(buf));
            rt_os_memcpy(buf, tmp_data, rt_os_strlen(tmp_data));
        #endif

            /* get http body */
            MSG_PRINTF(LOG_TRACE, "recv buff: %s\n", buf);
            p = rt_os_strstr(buf, "\r\n\r\n");
            if (p) {
                p += 4;
                char *tran = rt_os_strstr(buf, "Transfer-Encoding");
                if(tran) {
                    p = rt_os_strstr(p, "\r\n");
                }
                ret = cb(p);
                MSG_PRINTF(LOG_DBG, "cb ret:%d\n", ret);
                ret = RT_SUCCESS;
                break;
            } else {
                MSG_PRINTF(LOG_ERR, "error recv data\n");
                ret = RT_ERROR;
                break;
            }
        }        
        break;
    } while(0);

    if(socket_fd > 0){
        MSG_PRINTF(LOG_TRACE, "close sock fd=%d\r\n", socket_fd);
        http_tcpclient_close(socket_fd);
    }
    
    return ret;
}

