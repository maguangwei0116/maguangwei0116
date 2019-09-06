
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_process.c
 * Date        : 2019.09.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "msg_process.h"
#include "cJSON.h"

#define  MD5_STRING_LENGTH  32
/*****************************************************************************
 * FUNCTION
 *  msg_parse
 * DESCRIPTION
 *  parse iot command,then write upload info into queue.
 * PARAMETERS
 *  message buffer data
 *  len  data len
 * RETURNS
 *  void
 *****************************************************************************/
static int32_t msg_parse(const uint8_t *message, int32_t len)
{
    int32_t state = 0;

    cJSON *tran_id = NULL;

    return RT_SUCCESS;
}

int32_t msg_push_ac(const uint8_t *buf, int32_t len)
{
    cJSON *agent_msg = NULL;
    cJSON *payload = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];

    get_md5_string(buf, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_WARN, "The data are the same\n");
        return RT_ERROR;            // for test
    }
    do {
        agent_msg = cJSON_Parse((const char *)buf);
        if (!agent_msg) {
            MSG_PRINTF(LOG_ERR, "Message parse failed\n");
            break;
        }
        payload = cJSON_GetObjectItem(agent_msg, "payload");
        if (!payload) {
            MSG_PRINTF(LOG_ERR, "No payload information\n");
            break;
        }
    } while(0);
    msg_parse(buf, len);
    return RT_SUCCESS;
}
