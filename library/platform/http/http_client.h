/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : http_client.h
 * Date        : 2017.09.01
 * Note        :
 * Description : Implement the HTTP file upload and download
 *******************************************************************************/
#ifndef __HTTP_CLIENT_H__
#define __HTTP_CLIENT_H__

#include <stdio.h>
#include "file.h"

#if 0
#define RT_CHECK_ERR(process, result) \
    if((process) == result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_NEQ(process, result) \
    if((process) != result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_LESE(process, result) \
    if((process) <= result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_LES(process, result) \
    if((process) < result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#else
#define RT_CHECK_ERR(process, result) \
    if((process) == result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}
#define RT_CHECK_NEQ(process, result) \
    if((process) != result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}
#define RT_CHECK_LESE(process, result) \
    if((process) <= result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}
#define RT_CHECK_LES(process, result) \
    if((process) < result){ MSG_PRINTF(LOG_WARN, "[%s] error\n",  #process);  goto end;}
#endif

#define MAX_KEY_LEN                           30
#define MAX_VALUE_LEN                         100
typedef struct HTTP_REQUEST_HEADER_RECORD {
    char key[MAX_KEY_LEN + 1];
    char value[MAX_VALUE_LEN + 1];
} http_request_header_record_t;

#define MAX_HEADER_REQ_BODY_LEN               256
#define MAX_URL_LEN                           200
#define MAX_IP_ADDR_LEN                       20
#define MAX_RESUEST_HEADER_RECORD_SIZE        10
typedef struct HTTP_HEADER {
    unsigned char method;
    char                         url[MAX_URL_LEN + 1];
    char                         url_interface[MAX_URL_LEN + 1];
    unsigned int                 port;
    unsigned char                addr[MAX_IP_ADDR_LEN + 1];
    unsigned char                version;
    http_request_header_record_t record[MAX_RESUEST_HEADER_RECORD_SIZE];
    char                         record_size;    //the number of record
    char                         buf[MAX_HEADER_REQ_BODY_LEN];
} http_header_t;

#define MAX_BLOCK_LEN                         4096  // date block size
#define MAX_FILE_PATH_LEN                     100
#define MAX_TRY_COUNT                         3
typedef struct HTTP_CLIENT_STRUCT {
    int                if_continue;      //  if needed continue download
    char               manager_type;     // management type, 0:upload  1:download
    http_header_t      http_header;
    const char         *file_path;
    unsigned int       file_length;      // the total length of send \ recv
    unsigned int       process_length;   // Has the length of the data processing
    unsigned int       remain_length;    // Has the length of the data processing
    unsigned int       process_set;      // The need to deal with the length of the data
    unsigned int       range;
    unsigned char      try_count;        // Data processing of attempts count
    rt_fshandle_t      fp;
    int                socket;
    char               *buf;
    /* the buffer which used to send block data in the upload process and
     * used to send body in the download process*/
} http_client_struct_t;

int http_client_file_download(http_client_struct_t *d_state);
int http_client_file_upload(http_client_struct_t *up_state);
int http_set_header_record(http_client_struct_t *obj, const char *key, const char *value);

#endif  // __HTTP_CLIENT_H__
