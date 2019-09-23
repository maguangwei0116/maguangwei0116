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

#if defined(OPENSSL)
#include <openssl/ssl.h>
#endif

#include "rt_mqtt_common.h"

#if (1)
    #define RT_MQTT_COMMAN_DEBUG(fmt, arg...)   MSG_PRINTF(LOG_DBG, fmt, ##arg)
#else
    #define RT_MQTT_COMMAN_DEBUG(fmt, arg...)   do {} while(0)     
#endif

static REG_info reg_info;
static char reg_url[40];
static int reg_port = 8383;

typedef int (*YUNBA_CALLBACK)(char *p);

int http_post_json(char *json_data, char *hostname, uint16_t port, char *path, PCALLBACK cb)
{
    int ret = -1;
    int sockfd = -1;
    int h;
    socklen_t len;
    fd_set   t_set1;
    struct sockaddr_in servaddr;
    char buf[4096];
    char md5_out[32+1];

    do{
        if(!json_data || !hostname || !path) {
            RT_MQTT_COMMAN_DEBUG("path json data error\n");
            break;
        }
        memset(buf, 0, sizeof(buf));
        RT_MQTT_COMMAN_DEBUG("json_data:%s\n",json_data);
        get_md5_string(json_data, md5_out);
        md5_out[32] = '\0';

    #if defined(WIN32) || defined(WIN64)
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            printf("Init Windows Socket Failed::%d\n", GetLastError());
            break;
        }
    #else

    #endif

        RT_MQTT_COMMAN_DEBUG("md5_out:%s\n",md5_out);
        RT_MQTT_COMMAN_DEBUG("hostname:%s,port:%d\n",hostname,port);
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
            RT_MQTT_COMMAN_DEBUG("create socket error\n");
            break;
        }

        //RT_MQTT_COMMAN_DEBUG("get host by name 1\r\n");
        struct hostent *host_entry = gethostbyname(hostname);
        //RT_MQTT_COMMAN_DEBUG("get host by name 2\r\n");

        if(NULL == host_entry) {
            break;
        }

        char* p = inet_ntoa(*((struct in_addr *)host_entry->h_addr));
        if(!p) {
            RT_MQTT_COMMAN_DEBUG("11p error\n");
            break;
        }

        if (inet_pton(AF_INET, p, &servaddr.sin_addr) <= 0) {
            RT_MQTT_COMMAN_DEBUG("22p error\n");
            break;
        }
        
        RT_MQTT_COMMAN_DEBUG("servaddr.sin_addr:%x\n",servaddr.sin_addr);
        struct timeval timeout;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            RT_MQTT_COMMAN_DEBUG("setsockopt0 error\n");
            break;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
            RT_MQTT_COMMAN_DEBUG("setsockopt error\n");
            break;
        }
        
        //TODO: 超时处理.
        RT_MQTT_COMMAN_DEBUG("sockfd:%d\n",sockfd);
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            RT_MQTT_COMMAN_DEBUG("connect error %s errno: %d\n", strerror(errno), errno);
            break;
        }

        char temp[128];
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

        RT_MQTT_COMMAN_DEBUG("json data len:%d\n", strlen(json_data));
        //TODO:　可能没写完？
        RT_MQTT_COMMAN_DEBUG("send buf:%s\n", buf);
        
    #if defined(WIN32) || defined(WIN64)
        ret = send(sockfd, buf, strlen(buf), 0);
    #else
        ret = write(sockfd, buf, strlen(buf));
    #endif
        if (ret < 0) {
            RT_MQTT_COMMAN_DEBUG("write data error\n");
            break;
        }

        struct timeval  tv;
        FD_ZERO(&t_set1);
        FD_SET(sockfd, &t_set1);
        tv.tv_sec= 6;
        tv.tv_usec= 0;
        h = select(sockfd + 1, &t_set1, NULL, NULL, &tv);
        if (h > 0) {
            memset(buf, 0, sizeof(buf));
    #if defined(WIN32) || defined(WIN64)
            ssize_t  i = recv(sockfd, buf, sizeof(buf), 0);
    #else
            ssize_t  i= read(sockfd, buf, sizeof(buf));
    #endif

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
            //取body
            RT_MQTT_COMMAN_DEBUG("rcv buff:%s\n",buf);
            char *temp = strstr(buf, "\r\n\r\n");
            if (temp) {
                temp += 4;
                char *tran = rt_os_strstr(buf,"Transfer-Encoding");
                if(tran) {
                    temp = strstr(temp,"\r\n");
                }
                ret = cb(temp);
                RT_MQTT_COMMAN_DEBUG("cb ret:%d\n",ret);
                break;
            } else {
                RT_MQTT_COMMAN_DEBUG("ret:%d\n",ret);
                ret = -1;
                break;
            }
        } else {
            RT_MQTT_COMMAN_DEBUG("ret:%d\n",ret);
            ret = -1;
            break;
        }
        ret = 0;
        break;
    }while(1);

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

//EMQ的ticiet server回调处理
static int rt_reg_cb(const char *json_data) 
{
    int ret = -1;
    char buf[500];
    char *str = NULL;
    cJSON *root;
    
    snprintf(buf, sizeof(buf), "%s", json_data);
    RT_MQTT_COMMAN_DEBUG("buf:%s\n",buf);
    
    root = cJSON_Parse(buf);
    if(root == NULL){
        return -1;
    }
    
    str = cJSON_Print(root);
    if (str) {
        RT_MQTT_COMMAN_DEBUG("%s\n",str);
        cJSON_free(str);
    }
    
    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj){
            cJSON * pUsername = cJSON_GetObjectItem(obj,"username");
            cJSON * pPassword = cJSON_GetObjectItem(obj,"password");
            
            if(pUsername && pPassword){
                snprintf(reg_info.username, sizeof(reg_info.username), "%s", pUsername->valuestring);
                snprintf(reg_info.password, sizeof(reg_info.password), "%s", pPassword->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

//云吧的ticket server回调处理
static int reg_cb(const char *json_data) 
{
    int ret = -1;
    char buf[500];
    cJSON *root;
    
    snprintf(buf, sizeof(buf), "%s", json_data);

    root = cJSON_Parse(buf);
    if (root) {
        int ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * pClientId   = cJSON_GetObjectItem(root, "c");
            cJSON * pUsername   = cJSON_GetObjectItem(root, "u");
            cJSON * pPassword   = cJSON_GetObjectItem(root, "p");
            cJSON * pDevId      = cJSON_GetObjectItem(root, "d");
            
            if (pClientId && pUsername && pPassword && pDevId) {
                snprintf(reg_info.client_id, sizeof(reg_info.client_id), "%s", pClientId->valuestring);
                snprintf(reg_info.username, sizeof(reg_info.username), "%s", pUsername->valuestring);
                snprintf(reg_info.password, sizeof(reg_info.password), "%s", pPassword->valuestring);
                snprintf(reg_info.device_id, sizeof(reg_info.device_id), "%s", pDevId->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

//用于红茶adapter server回调处理
static int reg_cb1(const char *json_data) 
{
    int ret = -1;
    char buf[500];
    cJSON *data;
    cJSON *root;
    
    snprintf(buf, sizeof(buf), "%s", json_data);

    data = cJSON_Parse(buf);
    if (!data) {
        MSG_PRINTF(LOG_ERR, "json data parse fail !\r\n"); 
        return ret;
    }
    
    root = cJSON_GetObjectItem(data, "data");
    if (root) {
        int ret_size = cJSON_GetArraySize(root);
        if (ret_size >= 4) {
            cJSON * charnel     = cJSON_GetObjectItem(root, "s");
            cJSON * pUsername   = cJSON_GetObjectItem(root, "u");
            cJSON * pPassword   = cJSON_GetObjectItem(root, "p");
            cJSON * pClientId   = cJSON_GetObjectItem(root, "c");
            cJSON * host        = cJSON_GetObjectItem(root, "h");
            cJSON * port        = cJSON_GetObjectItem(root, "o");
            cJSON * ticket_url  = cJSON_GetObjectItem(root, "r");
            
            if (pUsername && pPassword && charnel && ticket_url && host && port ) {
                snprintf(reg_info.username, sizeof(reg_info.username), "%s", pUsername->valuestring);
                snprintf(reg_info.password, sizeof(reg_info.password), pPassword->valuestring);
                snprintf(reg_info.rt_channel, sizeof(reg_info.rt_channel), charnel->valuestring);
                snprintf(reg_info.ticket_server, sizeof(reg_info.ticket_server), ticket_url->valuestring);
                snprintf(reg_info.rt_url, sizeof(reg_info.rt_url), "%s:%d", host->valuestring, port->valueint);
                ret = 0;
            }

            if(!rt_os_strncmp(reg_info.rt_channel, "YUNBA", 5) && pClientId){
                snprintf(reg_info.client_id, sizeof(reg_info.client_id), pClientId->valuestring);
            }
        }

        cJSON_Delete(data);
    }
    
    return ret;
}

//  云吧获取mqtt连接参数接口
int MQTTClient_setup_with_appkey_and_deviceid(const char* appkey, const char *deviceid, mqtt_info *info)
{
    int ret;
    int json_data_len = 1024;
    char *json_data = NULL;

    if (!appkey) {
        ret = -1;
        goto exit_entry;
    }

    json_data = (char *)rt_os_malloc(json_data_len);
    if (!json_data) {
        ret = -2;
        goto exit_entry;
    }
    
    if (deviceid == NULL) {
        snprintf(json_data, json_data_len, "{\"a\": \"%s\", \"p\":4}", appkey);
    } else {
        snprintf(json_data, json_data_len, "{\"a\": \"%s\", \"p\":4, \"d\": \"%s\"}", appkey, deviceid);
    }
    
    ret = http_post_json(json_data, reg_url, reg_port, "/device/reg/", (PCALLBACK)reg_cb);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "http post json yunba error, %s:%d, ret=%d\r\n", reg_url, reg_port, ret);
        ret = -3;
        goto exit_entry;
    }

    strcpy(info->client_id, reg_info.client_id);
    strcpy(info->username, reg_info.username);
    strcpy(info->password, reg_info.password);
    strcpy(info->device_id, reg_info.device_id);

    ret = 0;
    
exit_entry:
    if (json_data) {
        rt_os_free(json_data);
    }
    
    return ret;
}

//EMQ获取MQTT连接参数接口
int MQTTClient_setup_with_appkey(char* appkey, mqtt_info *info)
{
    int ret;
    char json_data[1024];

    if (!appkey) {
        return -1;
    }

    snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);

    ret = http_post_json(json_data, reg_url, reg_port, "/clientService/getEmqUser", (PCALLBACK)rt_reg_cb);
    if (ret < 0) {
        return -1;
    }

    strcpy(info->client_id, reg_info.client_id);
    strcpy(info->username, reg_info.username);
    strcpy(info->password, reg_info.password);
    
    return 0;
}


//红茶adapter服务器获取
int rt_mqtt_setup_with_appkey(const char *appkey, mqtt_info *info, const char *eid)
{
    char json_data[1024];
    int ret;

    if (appkey == NULL){
        MSG_PRINTF(LOG_ERR, "appkey is NULL\n");
        return -1;
    }

    if (info->device_id == NULL) {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"a\": \"%s\",\"d\": \"%s\",\"c\":\"%s\",\"s\":\"%d\"}", \
            appkey, info->device_id, eid, info->last_connect_status);
    }

    MSG_PRINTF(LOG_DBG, "reg_url:%s, reg_port:%d\r\n", reg_url, reg_port);
    ret = http_post_json(json_data, reg_url, reg_port, "/api/v1/ticket", (PCALLBACK)reg_cb1);
    if (ret < 0){
        MSG_PRINTF(LOG_ERR, "http_post_json error, ret=%d\r\n", ret);
        return -1;
    }
    strcpy(info->client_id, reg_info.client_id);
    strcpy(info->username, reg_info.username);
    strcpy(info->password, reg_info.password);
    strcpy(info->rt_channel, reg_info.rt_channel);
    strcpy(info->ticket_server, reg_info.ticket_server);
    strcpy(info->rt_url, reg_info.rt_url);
    MSG_PRINTF(LOG_DBG, "client_id     : %s\r\n", info->client_id);
    MSG_PRINTF(LOG_DBG, "username      : %s\r\n", info->username);
    MSG_PRINTF(LOG_DBG, "password      : %s\r\n", info->password);
    MSG_PRINTF(LOG_DBG, "rt_channel    : %s\r\n", info->rt_channel);
    MSG_PRINTF(LOG_DBG, "ticket_server : %s\r\n", info->ticket_server);
    MSG_PRINTF(LOG_DBG, "rt_url        : %s\r\n", info->rt_url);
    
    return 0;
}

static char url_host[200];
static char url_port[8];

static size_t rt_get_broker_cb(const char *json_data)
{
    int ret = -1;
    char buf[500];
    cJSON *root;
    char *str = NULL;

    snprintf(buf, sizeof(buf), "%s", json_data);
    RT_MQTT_COMMAN_DEBUG("buf:%s\n",buf);

    root = cJSON_Parse(buf);    
    str = cJSON_Print(root);
    RT_MQTT_COMMAN_DEBUG("%s\n",str);
    cJSON_free(str);
    
    if (root) {
        cJSON *obj = cJSON_GetObjectItem(root, "obj");
        if(obj){
            cJSON *pURL = cJSON_GetObjectItem(obj,"ip");
            cJSON *PORT = cJSON_GetObjectItem(obj,"port");
            if ((pURL != NULL) && (PORT != NULL)) {
                strcpy(url_host, pURL->valuestring);
                strcpy(url_port, PORT->valuestring);
                ret = 0;
            }
        }
        cJSON_Delete(root);
    }
    return ret;
}

// static size_t get_broker_cb(const char *json_data)
// {
//  int ret = -1;
//  char buf[500];
//  memset(buf, 0, sizeof(buf));
//  memcpy(buf, json_data, strlen(json_data));
//  cJSON *root = cJSON_Parse(buf);
//  if (root) {
//      int ret_size = cJSON_GetArraySize(root);
//      if (ret_size >= 1) {
//          cJSON * pURL = cJSON_GetObjectItem(root,"c");
//          if (pURL != NULL) {
//              strcpy(url_host, pURL->valuestring);
//              ret = 0;
//          }
//      }
//      cJSON_Delete(root);
//  }
//  return ret;
// }

void set_reg_url(const char url[20], int port)
{
    rt_os_memset(reg_url,0,sizeof(reg_url));
    strcpy(reg_url, url);
    reg_port = port;
}

int MQTTClient_get_host(char *nodeName, char *url, const char *appkey)
{
    int ret = -1;
    char json_data[1024];
    
    if(!nodeName){
        snprintf(json_data, sizeof(json_data), "{\"appKey\":\"%s\"}", appkey);
    } else {
        snprintf(json_data, sizeof(json_data), "{\"nodeName\":\"%s\",\"appKey\":\"%s\"}", nodeName, appkey);
    }

    ret = http_post_json(json_data, reg_url, reg_port, "/clientService/getNodes", (PCALLBACK)rt_get_broker_cb);
    if (ret < 0) {
        return -1;
    }
    
    sprintf(url, "%s:%s", url_host, url_port);
    return 0;
}


