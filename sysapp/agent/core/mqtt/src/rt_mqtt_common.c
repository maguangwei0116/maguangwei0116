/*
 * yunba_common.c
 *
 *  Created on: Nov 16, 2015
 *      Author: yunba
 */

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
#include "rt_mqtt_common.h"

#if defined(OPENSSL)
#include <openssl/ssl.h>
#endif

#define HTTP_RECV_TIMEOUT   30

typedef int32_t (*YUNBA_CALLBACK)(char *p);

static mqtt_reg_info_t g_mqtt_reg_info;
static char g_mqtt_reg_url[40];
static int32_t g_mqtt_reg_port = 8383;
static char g_mqtt_url_host[200];
static char g_mqtt_url_port[8];

int32_t http_post_json(const char *json_data, char *hostname, uint16_t port, char *path, PCALLBACK cb)
{
    int32_t ret = RT_ERROR;
    int32_t sockfd = RT_ERROR;
    int32_t h;
    socklen_t len;
    fd_set   t_set1;
    struct sockaddr_in servaddr;
    char buf[4096];
    char md5_out[32+1];
    struct timeval timeout;
    struct hostent *host_entry;
    char *p = NULL;
    char temp[128];

    do{
        if(!json_data || !hostname || !path) {
            MSG_PRINTF(LOG_DBG, "path json data error\n");
            break;
        }
        
        MSG_PRINTF(LOG_INFO, "json_data:%s\n",json_data);
        get_md5_string((char *)json_data, md5_out);
        md5_out[32] = '\0';

    #if defined(WIN32) || defined(WIN64)
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0) {
            printf("Init Windows Socket Failed::%d\n", GetLastError());
            ret = RT_ERROR;
            break;
        }
    #else

    #endif

        //MSG_PRINTF(LOG_INFO, "md5_out:%s\n",md5_out);
        //MSG_PRINTF(LOG_INFO, "hostname:%s,port:%d\n",hostname,port);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
            MSG_PRINTF(LOG_WARN, "create socket error\n");
            ret = RT_ERROR;
            break;
        }

        //RT_MQTT_COMMAN_DEBUG("get host by name 1\r\n");
        host_entry = gethostbyname(hostname);
        //RT_MQTT_COMMAN_DEBUG("get host by name 2\r\n");
        if(!host_entry) {
            MSG_PRINTF(LOG_WARN, "get hosy by name fail\n");
            ret = RT_ERROR;
            break;
        }

        p = inet_ntoa(*((struct in_addr *)host_entry->h_addr));
        if(!p) {
            MSG_PRINTF(LOG_WARN, "11p error\n");
            ret = RT_ERROR;
            break;
        }

        if (inet_pton(AF_INET, p, &servaddr.sin_addr) <= 0) {
            MSG_PRINTF(LOG_WARN, "22p error\n");
            ret = RT_ERROR;
            break;
        }
        
        //MSG_PRINTF(LOG_INFO, "servaddr.sin_addr:%x\n",servaddr.sin_addr);        
        timeout.tv_sec = HTTP_RECV_TIMEOUT;
        timeout.tv_usec = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            MSG_PRINTF(LOG_WARN, "setsockopt0 error\n");
            break;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            MSG_PRINTF(LOG_WARN, "setsockopt error\n");
            ret = RT_ERROR;
            break;
        }

        //MSG_PRINTF(LOG_INFO, "sockfd:%d\n",sockfd);
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            MSG_PRINTF(LOG_WARN, "connect err(%d): %s\n", errno, strerror(errno));
            ret = RT_ERROR;
            break;
        }

        memset(buf, 0, sizeof(buf));
        snprintf(temp, sizeof(temp), "POST %s HTTP/1.1", path);
        strcat(buf, temp);
        strcat(buf, "\r\n");
        snprintf(temp, sizeof(temp), "Host: %s:%d", hostname, port),
        strcat(buf, temp);
        strcat(buf, "\r\n");
        strcat(buf, "Accept: */*\r\n");
        strcat(buf, "Content-Type: application/json;charset=UTF-8\r\n");
        strcat(buf, "md5sum:");
        snprintf(temp, sizeof(temp), "%s\r\n", md5_out);
        strcat(buf, temp);
        strcat(buf, "Content-Length: ");
        snprintf(temp, sizeof(temp), "%d", strlen(json_data)),
        strcat(buf, temp);
        strcat(buf, "\r\n\r\n");
        strcat(buf, json_data);

        //MSG_PRINTF(LOG_INFO, "json data len:%d\n", strlen(json_data));
        MSG_PRINTF(LOG_INFO, "send buf:%s\n", buf);
        
    #if defined(WIN32) || defined(WIN64)
        ret = send(sockfd, buf, strlen(buf), 0);
    #else
        ret = write(sockfd, buf, strlen(buf));
    #endif
        if (ret < 0) {
            MSG_PRINTF(LOG_WARN, "write data error\n");
            ret = RT_ERROR;
            break;
        }

        FD_ZERO(&t_set1);
        FD_SET(sockfd, &t_set1);
        h = select(sockfd + 1, &t_set1, NULL, NULL, &timeout);
        if (h > 0) {
            memset(buf, 0, sizeof(buf));
    #if defined(WIN32) || defined(WIN64)
            ret = recv(sockfd, buf, sizeof(buf), 0);
    #else
            ret = read(sockfd, buf, sizeof(buf));
    #endif
            if (ret < 0) {
                MSG_PRINTF(LOG_WARN, "read data error\n");
                ret = RT_ERROR;
                break;
            }

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
        memset(buf, 0, sizeof(buf));
        rt_os_memcpy(buf, tmp_data, strlen(tmp_data));
    #endif
            /* get http body */
            MSG_PRINTF(LOG_INFO, "rcv buff: %s\n", buf);
            p = strstr(buf, "\r\n\r\n");
            if (p) {
                p += 4;
                char *tran = rt_os_strstr(buf, "Transfer-Encoding");
                if(tran) {
                    p = strstr(p, "\r\n");
                }
                ret = cb(p);
                MSG_PRINTF(LOG_DBG, "cb ret:%d\n",ret);
                break;
            } else {
                MSG_PRINTF(LOG_DBG, "ret:%d\n",ret);
                ret = RT_ERROR;
                break;
            }
        } else {
            MSG_PRINTF(LOG_DBG, "h:%d\n",h);
            ret = RT_ERROR;
            break;
        }
        ret = RT_SUCCESS;
        break;
    }while(0);

    if(sockfd > 0){
    #if defined(WIN32) || defined(WIN64)
        closesocket(sockfd);
        WSACleanup();
    #else
        close(sockfd);
    #endif
    }
    
    return ret;
}

/* callback for EMQ ticket server */
static int32_t mqtt_emq_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    char *str = NULL;
    cJSON *root;

    MSG_PRINTF(LOG_INFO, "buf:%s\n", json_data);    
    root = cJSON_Parse(json_data);
    if(!root){
        return RT_ERROR;
    }
    
    str = cJSON_Print(root);
    if (str) {
        MSG_PRINTF(LOG_INFO, "%s\n", str);
        cJSON_free(str);
    }
    
    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj){
            cJSON * username = cJSON_GetObjectItem(obj, "username");
            cJSON * password = cJSON_GetObjectItem(obj, "password");
            
            if(username && password){
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), "%s", password->valuestring);
                ret = RT_SUCCESS;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

//云吧的ticket server回调处理
/* callback for YUNBA ticket server */
static int32_t mqtt_yunba_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    cJSON *root;

    root = cJSON_Parse(json_data);
    if (root) {
        int32_t ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * client_id   = cJSON_GetObjectItem(root, "c");
            cJSON * username    = cJSON_GetObjectItem(root, "u");
            cJSON * password    = cJSON_GetObjectItem(root, "p");
            cJSON * device_id   = cJSON_GetObjectItem(root, "d");
            
            if (client_id && username && password && device_id) {
                snprintf(g_mqtt_reg_info.client_id, sizeof(g_mqtt_reg_info.client_id), "%s", client_id->valuestring);
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), "%s", password->valuestring);
                snprintf(g_mqtt_reg_info.device_id, sizeof(g_mqtt_reg_info.device_id), "%s", device_id->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

/* callback for REDTEA ticket server */
static int32_t mqtt_redtea_ticket_server_cb(const char *json_data) 
{
    int32_t ret = RT_ERROR;
    cJSON *data;
    cJSON *root;

    data = cJSON_Parse(json_data);
    if (!data) {
        MSG_PRINTF(LOG_ERR, "json data parse fail !\r\n"); 
        return ret;
    }
    
    root = cJSON_GetObjectItem(data, "data");
    if (root) {
        int32_t ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * channel     = cJSON_GetObjectItem(root, "s");
            cJSON * username    = cJSON_GetObjectItem(root, "u");
            cJSON * password    = cJSON_GetObjectItem(root, "p");
            cJSON * client_id   = cJSON_GetObjectItem(root, "c");
            cJSON * host        = cJSON_GetObjectItem(root, "h");
            cJSON * port        = cJSON_GetObjectItem(root, "o");
            cJSON * ticket_url  = cJSON_GetObjectItem(root, "r");
            
            if (username && password && channel && ticket_url && host && port ) {
                snprintf(g_mqtt_reg_info.username, sizeof(g_mqtt_reg_info.username), "%s", username->valuestring);
                snprintf(g_mqtt_reg_info.password, sizeof(g_mqtt_reg_info.password), password->valuestring);
                snprintf(g_mqtt_reg_info.channel, sizeof(g_mqtt_reg_info.channel), channel->valuestring);
                snprintf(g_mqtt_reg_info.ticket_server, sizeof(g_mqtt_reg_info.ticket_server), ticket_url->valuestring);
                snprintf(g_mqtt_reg_info.url, sizeof(g_mqtt_reg_info.url), "%s:%d", host->valuestring, port->valueint);
                ret = 0;
            }

            if(!rt_os_strncmp(g_mqtt_reg_info.channel, "YUNBA", 5) && client_id){
                snprintf(g_mqtt_reg_info.client_id, sizeof(g_mqtt_reg_info.client_id), client_id->valuestring);
            }
        }

        cJSON_Delete(data);
    }
    
    return ret;
}

/* get YUNBA MQTT connect param API */
int32_t MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_info_t *info)
{
    int32_t ret;
    int32_t json_data_len = 1024;
    char *json_data = NULL;

    if (!appkey) {
        ret = RT_ERROR;
        goto exit_entry;
    }

    json_data = (char *)rt_os_malloc(json_data_len);
    if (!json_data) {
        ret = RT_ERROR;
        goto exit_entry;
    }
    
    if (!deviceid) {
        snprintf(json_data, json_data_len, "{\"a\": \"%s\", \"p\":4}", appkey);
    } else {
        snprintf(json_data, json_data_len, "{\"a\": \"%s\", \"p\":4, \"d\": \"%s\"}", appkey, deviceid);
    }
    
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, "/device/reg/", (PCALLBACK)mqtt_yunba_ticket_server_cb);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "http post json yunba error, %s:%d, ret=%d\r\n", g_mqtt_reg_url, g_mqtt_reg_port, ret);
        ret = RT_ERROR;
        goto exit_entry;
    }

    snprintf(info->client_id, sizeof(info->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(info->username, sizeof(info->username), "%s", g_mqtt_reg_info.username);
    snprintf(info->password, sizeof(info->password), "%s", g_mqtt_reg_info.password);
    snprintf(info->device_id, sizeof(info->device_id), "%s", g_mqtt_reg_info.device_id);

    ret = RT_SUCCESS;
    
exit_entry:
    if (json_data) {
        rt_os_free(json_data);
    }
    
    return ret;
}

/* get EMQ MQTT connect param API */
int32_t MQTTClient_setup_with_appkey(const char* appkey, mqtt_info_t *info)
{
    int32_t ret;
    char json_data[1024];

    if (!appkey) {
        return RT_ERROR;
    }

    snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);

    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/clientService/getEmqUser", (PCALLBACK)mqtt_emq_ticket_server_cb);
    if (ret < 0) {
        return RT_ERROR;
    }

    snprintf(info->client_id, sizeof(info->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(info->username, sizeof(info->username), "%s", g_mqtt_reg_info.username);
    snprintf(info->password, sizeof(info->password), "%s", g_mqtt_reg_info.password);
    
    return RT_SUCCESS;
}

/* get connect param with REDTEA adapter API */
int32_t mqtt_adapter_setup_with_appkey(const char *appkey, mqtt_info_t *info, const char *eid)
{
    char json_data[1024];
    int32_t ret;

    if (!appkey){
        MSG_PRINTF(LOG_ERR, "appkey is NULL\n");
        return RT_ERROR;
    }

    if (!info->device_id) {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\",\"d\": \"%s\",\"c\":\"%s\",\"s\":\"%d\"}", \
            appkey, info->device_id, eid, info->last_connect_status);
    }

    MSG_PRINTF(LOG_INFO, "reg_url:%s, reg_port:%d\r\n", g_mqtt_reg_url, g_mqtt_reg_port);
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/api/v1/ticket", (PCALLBACK)mqtt_redtea_ticket_server_cb);
    if (ret < 0){
        MSG_PRINTF(LOG_ERR, "http_post_json error, ret=%d\r\n", ret);
        return RT_ERROR;
    }
    snprintf(info->client_id, sizeof(info->client_id), "%s", g_mqtt_reg_info.client_id);
    snprintf(info->username, sizeof(info->username), "%s", g_mqtt_reg_info.username);
    snprintf(info->password, sizeof(info->password), "%s", g_mqtt_reg_info.password);
    snprintf(info->channel, sizeof(info->channel), "%s", g_mqtt_reg_info.channel);
    snprintf(info->ticket_server, sizeof(info->ticket_server), "%s", g_mqtt_reg_info.ticket_server);
    snprintf(info->url, sizeof(info->url), "%s", g_mqtt_reg_info.url);

#if 0
    MSG_PRINTF(LOG_DBG, "client_id     : %s\r\n", info->client_id);
    MSG_PRINTF(LOG_DBG, "username      : %s\r\n", info->username);
    MSG_PRINTF(LOG_DBG, "password      : %s\r\n", info->password);
    MSG_PRINTF(LOG_DBG, "channel       : %s\r\n", info->channel);
    MSG_PRINTF(LOG_DBG, "ticket_server : %s\r\n", info->ticket_server);
    MSG_PRINTF(LOG_DBG, "url           : %s\r\n", info->url);
#endif
    
    return RT_SUCCESS;
}

static size_t mqtt_get_broker_cb(const char *json_data)
{
    int32_t ret = RT_ERROR;
    cJSON *root;
   
    MSG_PRINTF(LOG_INFO, "-------------------------- json_data: %s\n", json_data);
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
                snprintf(g_mqtt_url_host, sizeof(g_mqtt_url_host), "%s", url->valuestring);
                snprintf(g_mqtt_url_port, sizeof(g_mqtt_url_port), "%s", port->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

void mqtt_set_reg_url(const char url[20], int32_t port)
{
    snprintf(g_mqtt_reg_url, sizeof(g_mqtt_reg_url), "%s", url);
    g_mqtt_reg_port = port;
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

    MSG_PRINTF(LOG_INFO, "-------------------------- json_data: %s\n", json_data);
    ret = http_post_json((const char *)json_data, g_mqtt_reg_url, g_mqtt_reg_port, \
                            "/clientService/getNodes", (PCALLBACK)mqtt_get_broker_cb);
    if (ret < 0) {
        return RT_ERROR;
    }
    
    sprintf(url, "%s:%s", g_mqtt_url_host, g_mqtt_url_port);
    return RT_SUCCESS;
}

