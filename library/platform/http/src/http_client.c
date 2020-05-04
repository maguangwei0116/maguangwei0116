/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : http_client.c
 * Date        : 2018.12.7
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "rt_type.h"
#include "log.h"
#include "http_client.h"

static void display_progress(const http_client_struct_t *obj)
{
#if (VERSION_TYPE == DEBUG) // only for debug version
    int percentage = ((float)obj->process_length / (obj->file_length + obj->range)) * 100;
    MSG_PRINTF(LOG_TRACE, "file download [%s] : %3d%% (%7d/%-7d)\r\n", \
                    obj->file_path, percentage, obj->process_length, (obj->file_length + obj->range));
    //rt_os_sleep(1);  // only for test
#else
    (void)obj;
#endif
}

static int http_client_upload_init(http_client_struct_t *obj)
{
    int ret = RT_ERROR;
    struct sockaddr_in server_addr;
    struct in_addr ipAddr;
    struct timeval timeout = {30,0};

    RT_CHECK_ERR(obj, NULL);

    MSG_PRINTF(LOG_INFO, "upload file_name:%s\n", obj->file_path);

    RT_CHECK_LESE(obj->file_length  = linux_rt_file_size(obj->file_path), 0);

    obj->remain_length = obj->file_length;
    obj->process_length = 0;
    MSG_PRINTF(LOG_TRACE, "upload file_name:%s,file_size:%d\n", obj->file_path, obj->file_length);

    RT_CHECK_ERR(obj->fp = linux_rt_fopen(obj->file_path, "r"), NULL);  // ´ò¿ªÎÄ¼þ
    RT_CHECK_ERR(obj->buf = (char *)rt_os_malloc(MAX_BLOCK_LEN), NULL);

    /* connect server */
    RT_CHECK_NEQ(http_parse_url(obj->http_header.url,
                                obj->http_header.addr,
                                obj->http_header.url_interface,
                                &obj->http_header.port), 0);

    MSG_PRINTF(LOG_INFO, "addr:%s,port:%d\n", obj->http_header.addr, obj->http_header.port);

    ipAddr.s_addr = inet_addr((char *)obj->http_header.addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(obj->http_header.port);
    server_addr.sin_addr   = ipAddr;

    RT_CHECK_ERR(obj->socket = socket(AF_INET, SOCK_STREAM, 0), -1);

    setsockopt(obj->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    setsockopt(obj->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));

    RT_CHECK_ERR(connect(obj->socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)), -1);

     ret = RT_SUCCESS;
end:
    return ret;
}

static int http_client_download_init(http_client_struct_t *obj)
{
    int ret = -1;
    struct sockaddr_in server_addr;
    struct in_addr ipAddr;
    struct timeval timeout={30,0};

    RT_CHECK_ERR(obj, NULL);

    MSG_PRINTF(LOG_INFO, "download file_name:%s\n", obj->file_path);

    RT_CHECK_ERR(obj->fp = linux_rt_fopen(obj->file_path, "a"), NULL);
    obj->process_length = 0;
    obj->process_set = 0;
    obj->remain_length = 0;

    RT_CHECK_NEQ(http_parse_url(obj->http_header.url,
                                obj->http_header.addr,
                                obj->http_header.url_interface,
                                &obj->http_header.port), 0);

    MSG_PRINTF(LOG_INFO, "addr:%s,port:%d\n", obj->http_header.addr, obj->http_header.port);

    ipAddr.s_addr           = inet_addr((char *)obj->http_header.addr);
    server_addr.sin_family  = AF_INET;
    server_addr.sin_port    = htons(obj->http_header.port);
    server_addr.sin_addr    = ipAddr;

    RT_CHECK_ERR(obj->buf = (char *)rt_os_malloc(MAX_BLOCK_LEN), NULL);
    RT_CHECK_ERR(obj->socket = socket(AF_INET, SOCK_STREAM, 0), -1);

    setsockopt(obj->socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    setsockopt(obj->socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(struct timeval));

    RT_CHECK_ERR(connect(obj->socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)), -1);

    ret = 0;
end:
    return ret;
}

static void http_client_release(http_client_struct_t *obj)
{
    if (obj->buf != NULL) {
        rt_os_free(obj->buf);
        obj->buf = NULL;
    }

    if (obj->fp != NULL) {
        linux_fclose(obj->fp);
        obj->fp = NULL;
    }

    if (obj->socket > 0) {
        close(obj->socket);
        obj->socket = -1;
    }
}

static int http_client_send(http_client_struct_t *obj)
{
    int sent = 0;
    int tmpres = 0;

    while (sent < obj->process_set) {
        tmpres = send(obj->socket, obj->buf + sent, obj->process_set - sent, MSG_NOSIGNAL);
        if (tmpres == -1) {
            MSG_PRINTF(LOG_WARN, "tmpres is error:%s\n", strerror(errno));
            if (errno == SIGPIPE) {
                MSG_PRINTF(LOG_WARN, "socket disconnected by peer !\r\n");
                return -2;
            }
            return -1;
        }
        sent += tmpres;
    }
    return sent;
}

static int http_client_recv(http_client_struct_t *obj)
{
    int cnt = 0;
    int recvnum = 0;

    rt_os_memset(obj->buf, 0, MAX_BLOCK_LEN);
    while(1) {
        cnt = (int)recv(obj->socket, obj->buf + recvnum, obj->process_set, MSG_WAITALL);
        //MSG_PRINTF(LOG_WARN, "Recv cnt: %d, obj->process_set=%d !!!\n", cnt, obj->process_set);
        if (cnt > 0) {
            recvnum += cnt;
            break;
        } else if (cnt == 0) {
            MSG_PRINTF(LOG_WARN, "Recv error because socket disconnect!!!\n");
            recvnum = -1;
            break;
        } else {
            if((cnt < 0) && (errno == EINTR)){
              continue;
            }
            recvnum = -1;
            MSG_PRINTF(LOG_WARN, "Recv data error result:%s\n", strerror(errno));
            break;
        }
    }
    return recvnum;
}

static int http_client_send_header(http_client_struct_t *obj)
{
    int ret = -1;
    int index;

    RT_CHECK_ERR(obj, NULL);

    rt_os_memset(obj->buf,0 ,MAX_BLOCK_LEN);
    if (obj->http_header.method == 0) {
        rt_os_strcat(obj->buf, "POST");
    } else {
        MSG_PRINTF(LOG_WARN, "Unknow http request!!\n");
        return -1;
    }
    rt_os_strcat(obj->buf, " ");
    rt_os_strcat(obj->buf, obj->http_header.url_interface);
    rt_os_strcat(obj->buf, " ");

    if (obj->http_header.version == 0) {
        rt_os_strcat(obj->buf, "HTTP/1.1");
    } else {
        MSG_PRINTF(LOG_WARN, "Unknow http request Version!!\n");
        return -1;
    }
    rt_os_strcat(obj->buf, "\r\n");

    for (index = 0; index < obj->http_header.record_size; index++) {
        rt_os_strcat(obj->buf, obj->http_header.record[index].key);
        rt_os_strcat(obj->buf, ":");
        rt_os_strcat(obj->buf, obj->http_header.record[index].value);
        rt_os_strcat(obj->buf, "\r\n");
    }
    rt_os_strcat(obj->buf, "\r\n");

    MSG_PRINTF(LOG_TRACE, "Http request header:\n%s%s\n", obj->buf, obj->http_header.buf);
    obj->process_set = rt_os_strlen(obj->buf);

    return http_client_send(obj);

end:
    return ret;
}

static int http_client_send_body(http_client_struct_t *obj)
{
    int ret = -1;

    RT_CHECK_ERR(obj, NULL);

    obj->try_count = 0;
    obj->process_length = 0;

    if (obj->manager_type == 0) { /* do http upload */
       /* If the upload process, cycle to send data */
        while (obj->remain_length > 0) {
            rt_os_memset(obj->buf, 0, MAX_BLOCK_LEN);
            if (obj->remain_length >= MAX_BLOCK_LEN) {
              obj->process_set = MAX_BLOCK_LEN;
            } else {
              obj->process_set = obj->remain_length;
            }

            ret = linux_fread(obj->buf, obj->process_set , 1, obj->fp);
            if (ret == 0) {
                MSG_PRINTF(LOG_WARN, "Read Block Data Error,result:%s\n", strerror(errno));
                break;
            } else if (ret != 1) {
                MSG_PRINTF(LOG_WARN, "Read Block Data Error,result:%s\n", strerror(errno));
                rt_os_sleep(1);
                continue;
            }

            ret = http_client_send(obj);
            if (ret <= 0) {
                /* return error because of send error (broken pipe) */
                if (ret == -2) {
                    return -1;
                }
                /* send data error more than MAX_TRY_COUNT times */
                if (obj->try_count++ > MAX_TRY_COUNT) {
                    return -1;
                    MSG_PRINTF(LOG_WARN, "http_client send More than most trying times\n");
                }
                MSG_PRINTF(LOG_WARN, "http_client send error,continue\n");
                rt_os_sleep(1);
                linux_fseek(obj->fp, RT_FS_SEEK_SET, obj->process_length);
            } else {
                obj->remain_length -= obj->process_set;
                obj->process_length += obj->process_set;
            }
            //MSG_PRINTF(LOG_WARN, "file upload [%s] : (%7d/%-7d)\r\n", obj->file_path, obj->process_length, obj->file_length);
        }
    } else { /* do http download */
        rt_os_memset(obj->buf,0 ,MAX_BLOCK_LEN);
        rt_os_memcpy(obj->buf, obj->http_header.buf, rt_os_strlen(obj->http_header.buf));
        obj->process_set = rt_os_strlen(obj->http_header.buf);
        if (http_client_send(obj) <= 0) {
            MSG_PRINTF(LOG_WARN, "http_client send error\n");
            return -1;
        }
    }

    ret = 0;
end:
    return ret;
}

static int http_client_get_resp_header(http_client_struct_t *obj)
{
    int flag = 0;

    obj->process_length = 0;
    rt_os_memset(obj->buf,0 ,MAX_BLOCK_LEN);

    while (recv(obj->socket, obj->buf + obj->process_length, 1, 0) == 1) {
        /* http header end with a empty line (at least 3 '\r' or '\n', normal for 4 '\r' or '\n') */
        if (flag < 3) {
            if (obj->buf[obj->process_length] == '\r' || obj->buf[obj->process_length] == '\n') {
                flag++;
            } else {
                flag = 0;
            }
        } else {
            obj->buf[obj->process_length] = '\0';
            MSG_PRINTF(LOG_TRACE,"recv http header ok, flag:%d, buf:%s\n", flag, obj->buf);
            return 0;
        }
        //MSG_PRINTF(LOG_INFO,"->>> flag:%d, char:%c\n", flag, obj->buf[obj->process_length]);
        obj->process_length++;
    }
    MSG_PRINTF(LOG_WARN,"recv http header fail, flag:%d, buf:%s\n", flag, obj->buf);
    return -1;
}

static int http_client_recv_data(http_client_struct_t *obj)
{
    int ret = -1;

    RT_CHECK_ERR(obj, NULL);
    obj->try_count = 0;
    obj->process_length = obj->range;

    MSG_PRINTF(LOG_TRACE, "obj->range=%d, obj->process_length=%d\n", obj->range, obj->process_length);

    /* If the process for download, loop to receive data */
    while (obj->remain_length > 0) {
        if (obj->remain_length >= MAX_BLOCK_LEN) {
            obj->process_set = MAX_BLOCK_LEN;
        } else {
            obj->process_set = obj->remain_length;
        }

        RT_CHECK_NEQ(http_client_recv(obj), obj->process_set);
        obj->remain_length -= obj->process_set;
        obj->process_length += obj->process_set;

        //MSG_PRINTF(LOG_WARN, "obj->process_length=%d, obj->range=%d\r\n", obj->process_length, obj->range);
        if (obj->process_length > obj->range) {
            RT_CHECK_NEQ(linux_fwrite(obj->buf, obj->process_set, 1, obj->fp), 1);
            linux_fflush(obj->fp);
            rt_os_sync();
            rt_os_sync();
            display_progress((const http_client_struct_t *)obj);
        }
    }

    ret = 0;
end:
    return ret;
}

static uint32_t msg_string_to_int(uint8_t* str)
{
    uint32_t length = 0;

    if (str == NULL) {
        MSG_PRINTF(LOG_WARN, "The string is error\n");
        return 0;
    }
    while (*str != '\0') {
        if ((*str >= '0') && (*str <= '9')) {
            length = length * 10 + *str - '0';
        }
        str++;
    }

    return length;
}

static int http_client_error_prase(http_client_struct_t *obj)
{
    int ret = RT_ERROR;
    int resp_status;
    char *pos = NULL;
    char *end = NULL;
    char length[16];
    char length_char_len;

    RT_CHECK_ERR(obj, NULL);

    MSG_PRINTF(LOG_TRACE, "respond header:\n%s\n", obj->buf);
    rt_os_memset(length, 0, sizeof(length));

    RT_CHECK_ERR(pos = rt_os_strstr((const char *)obj->buf, (const char *)"HTTP/"), NULL);
    rt_os_memcpy(length, pos+9, 3);
    length[3] = '\0';
    resp_status = msg_string_to_int((uint8_t *)length);

    if (resp_status != 200) {
        MSG_PRINTF(LOG_WARN, "false resp_status:%d\n", resp_status);
        goto end;
    }

    if (obj->manager_type == 0) {
    } else {
        rt_os_memset(length,0,sizeof(length));
        RT_CHECK_ERR(pos = rt_os_strstr((const char *)obj->buf, (const char *)"Content-Length"), NULL);
        RT_CHECK_ERR(end = rt_os_strstr((const char *)pos, (const char *)"\r\n"), NULL);
        length_char_len = end - pos -16;
        rt_os_memcpy(length, pos + 16, length_char_len);
        length[length_char_len] = '\0';
        obj->file_length = msg_string_to_int((uint8_t *)length);
        MSG_PRINTF(LOG_TRACE, "Download file size:%d\n", obj->file_length);
        obj->remain_length = obj->file_length;
    }

    ret = RT_SUCCESS;
end:
    return ret;
}

int http_set_header_record(http_client_struct_t *obj, const char *key, const char *value)
{
    int8_t i;

    if (obj == NULL || key == NULL || value ==NULL) {
        MSG_PRINTF(LOG_WARN, "The param is NULL\n");
        return RT_ERROR;
    }

    if (obj->http_header.record_size > MAX_RESUEST_HEADER_RECORD_SIZE - 1) {
        MSG_PRINTF(LOG_WARN, "THE http header record is full!!\n");
        return RT_ERROR;
    }

    for (i = 0; i < obj->http_header.record_size; i++) {
        if(rt_os_strncmp(obj->http_header.record[i].key, key, rt_os_strlen(key)) == 0) {
            rt_os_memset(obj->http_header.record[i].value, 0x00, MAX_VALUE_LEN + 1);
            rt_os_memcpy(obj->http_header.record[i].value, value, rt_os_strlen(value));
            return RT_SUCCESS;
        }
    }

    rt_os_memcpy(obj->http_header.record[obj->http_header.record_size].key, key, rt_os_strlen(key));
    obj->http_header.record[obj->http_header.record_size].key[rt_os_strlen(key)] = '\0';

    rt_os_memcpy(obj->http_header.record[obj->http_header.record_size].value, value, rt_os_strlen(value));
    obj->http_header.record[obj->http_header.record_size].value[rt_os_strlen(value)] = '\0';
    obj->http_header.record_size++;

    return RT_SUCCESS;
}

int http_client_file_download(http_client_struct_t *d_state)
{
    int ret = RT_ERROR;

    RT_CHECK_ERR(d_state, NULL);
    RT_CHECK_NEQ(http_client_download_init(d_state), 0);
    RT_CHECK_LES(http_client_send_header(d_state), 0);
    RT_CHECK_LES(http_client_send_body(d_state), 0);
    RT_CHECK_LES(http_client_get_resp_header(d_state), 0);
    RT_CHECK_LES(http_client_error_prase(d_state), 0);
    RT_CHECK_ERR(http_client_recv_data(d_state), -1);
    MSG_PRINTF(LOG_INFO, "Download  %s success\n", d_state->file_path);
    ret = RT_SUCCESS;

end:
    http_client_release(d_state);

    return ret;
}

int http_client_file_upload(http_client_struct_t *up_state)
{
    int ret = RT_ERROR;

    RT_CHECK_ERR(up_state, NULL);
    RT_CHECK_NEQ(http_client_upload_init(up_state), 0);
    RT_CHECK_LES(http_client_send_header(up_state), 0);
    RT_CHECK_LES(http_client_send_body(up_state), 0);
    RT_CHECK_LES(http_client_get_resp_header(up_state), 0);
    RT_CHECK_LES(http_client_error_prase(up_state), 0);
    MSG_PRINTF(LOG_INFO, "Uplaad  %s success\n", up_state->file_path);

    ret = RT_SUCCESS;
end:
    http_client_release(up_state);  // ÊÍ·Å½á¹¹
    return ret;
}
