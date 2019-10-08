
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
#include "downstream.h"
#include "cJSON.h"
#include "rt_qmi.h"

#define  MSG_ONE_BLOCK_SIZE                             128
#define  APN_LIST                                       "/data/redtea/rt_apn_list"
#define  RT_PROXY_SERVER_ADDR                           "smdp-test.redtea.io" // stage(smdp-test.redtea.io) prod(smdp.redtea.io) qa(smdp-test.redtea.io)

static const card_info_t *g_card_info;
static const char *g_smdp_proxy_addr = NULL;

int32_t init_msg_process(void *arg, void *proxy_addr)
{
    g_card_info = (card_info_t *)arg;
    g_smdp_proxy_addr = (const char *)proxy_addr;
    if (rt_os_access(APN_LIST, 0)) {
        rt_create_file(APN_LIST);
    }
    return RT_SUCCESS;
}

rt_bool msg_check_iccid_state(const char *iccid)
{
    MSG_PRINTF(LOG_INFO, "g_iccid:%s,iccid:%s\n", g_card_info->iccid, iccid);
    if (rt_os_strncmp(g_card_info->iccid, iccid, THE_ICCID_LENGTH) == 0){
        return RT_TRUE;
    }
    return RT_FALSE;
}

int32_t msg_download_profile(const char *ac, const char *cc, char iccid[21])
{
    return lpa_download_profile(ac, cc, iccid, RT_PROXY_SERVER_ADDR);
}

int32_t msg_set_apn(const char *iccid)
{
    char apn_name[100] = {0};
    msg_get_op_apn_name(iccid, apn_name);
    MSG_PRINTF(LOG_INFO, "iccid:%s, apn_name:%s\n", iccid, apn_name);
    rt_qmi_modify_profile(1, 0, apn_name, 0);

    return RT_SUCCESS;
}

#if 0
EnableProfileResponse ::= [49] SEQUENCE { -- Tag 'BF31'
enableResult INTEGER {ok(0), iccidOrAidNotFound (1), 
profileNotInDisabledState(2), disallowedByPolicy(3), wrongProfileReenabling(4), 
catBusy(5), undefinedError(127)}
}
#endif
static int32_t msg_enable_profile_check(const char *iccid)
{
    int32_t i = 0;
    int32_t ret = RT_FALSE;
    
    if (msg_check_iccid_state(iccid) == RT_TRUE) {
        /* iccid is enable now ! */
        MSG_PRINTF(LOG_WARN, "iccid:%s not in disable state !\n", iccid);
        return 2;
    }   

    for (i = 0; i < g_card_info->num; i++) {
        if (!rt_os_strncmp(g_card_info->info[i].iccid, iccid, THE_ICCID_LENGTH)){
            /* found exist iccid */
            ret = lpa_enable_profile(iccid);
            if (ret == RT_SUCCESS) {
                /* set apn info when enable profile ok */
                msg_set_apn(iccid);
            }

            return ret;
        }   
    }

    /* iccid isn't exist ! */
    MSG_PRINTF(LOG_WARN, "iccid:%s not exist!\n", iccid);
    return 1;
}

int32_t msg_enable_profile(const char *iccid)
{
    return msg_enable_profile_check(iccid);
}

int32_t msg_delete_profile(const char *iccid, rt_bool *iccid_using)
{
    if (msg_check_iccid_state(iccid) == RT_TRUE) {
        if (iccid_using) {
            *iccid_using = RT_TRUE;
        }
        lpa_disable_profile(iccid);
        rt_os_sleep(1);
    }
    return lpa_delete_profile(iccid);
}

int32_t msg_debug_apn_list(void)
{
#if 0
    int32_t ret;
    int32_t i = 0;
    char apn_list_data[MSG_ONE_BLOCK_SIZE * 20] = {0};

    // ret = rt_read_all_file_data(APN_LIST, apn_list_data, sizeof(apn_list_data));
    while (i < ret) {
        apn_list_data[i + MSG_ONE_BLOCK_SIZE - 1] = ' ';
        i += MSG_ONE_BLOCK_SIZE;
    }
    MSG_INFO(LOG_INFO, "apn list data: %s\r\n", apn_list_data);
#endif
    return RT_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  msg_get_free_block
 * DESCRIPTION
 *  get one free block id.
 * PARAMETERS
 *  buffer
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t msg_get_free_block(uint8_t *buffer)
{
    int32_t ii = 0;
    while (1) {
        if (rt_read_data(APN_LIST, ii * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
            break;
        }
        ii++;
    }
    return ii;
}

/*****************************************************************************
 * FUNCTION
 *  msg_select
 * DESCRIPTION
 *  According iccid to data.
 * PARAMETERS
 *  iccid     iccid
 *  buffer    apn info
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t msg_select(const char *iccid, uint8_t *buffer)
{
    cJSON *s_iccid = NULL;
    cJSON *agent_msg = NULL;
    int32_t ii = 0;
    uint8_t tmp_buffer[MSG_ONE_BLOCK_SIZE + 1] = {0};

    while (1) {
        if (rt_read_data(APN_LIST, ii * MSG_ONE_BLOCK_SIZE, tmp_buffer, MSG_ONE_BLOCK_SIZE) < 0) {
            break;
        }
        agent_msg = cJSON_Parse(tmp_buffer);
        if (agent_msg != NULL) {
            s_iccid = cJSON_GetObjectItem(agent_msg, "iccid");
            if (s_iccid != NULL) {
                if(! rt_os_memcmp(iccid, s_iccid->valuestring, rt_os_strlen(s_iccid->valuestring))) {
                    cJSON_Delete(agent_msg);
                    memcpy(buffer, tmp_buffer, MSG_ONE_BLOCK_SIZE);
                    return ii;
                }
            } else {
                cJSON_Delete(agent_msg);
                MSG_PRINTF(LOG_WARN, "The iccid is NULL\n");
            }
        }
        ii++;
    }
    MSG_PRINTF(LOG_WARN, "can't find apn for iccid: %s \r\n", iccid);
    msg_debug_apn_list();
    return RT_ERROR;
}

/*****************************************************************************
 * FUNCTION
 *  msg_insert
 * DESCRIPTION
 *  insert one item into file.
 * PARAMETERS
 *  iccid
 *  buffer      The data will be inserted.
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t msg_insert(uint8_t *iccid, uint8_t *buffer)
{
    int32_t num = 0;
    uint8_t buff[MSG_ONE_BLOCK_SIZE];

    num = msg_select(iccid, buff);
    if (num == RT_ERROR) {
        num = msg_get_free_block(buff);
    }
    MSG_PRINTF(LOG_INFO, "Insert iccid %s apn data !\r\n", iccid);
    rt_write_data(APN_LIST, num * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE);
    return RT_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  msg_delete
 * DESCRIPTION
 *  delete one item from file.
 * PARAMETERS
 *  iccid
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t msg_delete(uint8_t *iccid)
{
    uint8_t buffer[MSG_ONE_BLOCK_SIZE+1];
    int32_t block = 0;
    int32_t num = 0;

    num = msg_get_free_block(buffer);
    block = msg_select(iccid, buffer);
    if (block >= num) {
        MSG_PRINTF(LOG_WARN, "can not find iccid\n");
        return RT_ERROR;
    }
    MSG_PRINTF(LOG_DBG, "num:%d,block:%d\n", num, block);
    if (block != num - 1) {
        if (rt_read_data(APN_LIST, (num - 1) * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
            MSG_PRINTF(LOG_WARN, "rt read data failed\n");
            return RT_ERROR;
        } else {
            if (rt_write_data(APN_LIST, block * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
                MSG_PRINTF(LOG_WARN, "rt write data failed\n");
                return RT_ERROR;
            }
        }
    }
    rt_truncate_data(APN_LIST, (num - 1) * MSG_ONE_BLOCK_SIZE);
    msg_debug_apn_list();
    MSG_PRINTF(LOG_INFO, "delete iccid: %s\r\n", iccid);

    return RT_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  msg_get_op_apn_name
 * DESCRIPTION
 *  get apn name
 * PARAMETERS
 *  @apn_name back apn name.
 *  @iccid    according iccid to find apn name.
 * RETURNS
 *  void
 *****************************************************************************/
int32_t msg_get_op_apn_name(const char *iccid, char *apn_name)
{
    cJSON *agent_msg = NULL;
    cJSON *apn_list = NULL;
    cJSON *apn_item = NULL;
    cJSON *apn = NULL;
    cJSON *mcc_mnc = NULL;
    uint8_t iccid_t[THE_ICCID_LENGTH + 1] = {0};
    int32_t apn_num = 0;
    int32_t ii = 0;
    uint8_t buffer[MSG_ONE_BLOCK_SIZE + 1] = {0};

    msg_select(iccid, buffer);
    //MSG_PRINTF(LOG_INFO, "buffer=%s\r\n", buffer);
    agent_msg = cJSON_Parse(buffer);
    if (!agent_msg) {
        MSG_PRINTF(LOG_WARN, "agent_msg error, parse apn name fail !\n");
        return RT_FALSE;
    }
    apn_list = cJSON_GetObjectItem(agent_msg, "apnInfos");
    if (apn_list != NULL) {
        apn_num = cJSON_GetArraySize(apn_list);
        MSG_PRINTF(LOG_INFO, "apn_num: %d\r\n", apn_num);
        for (ii = 0; ii < apn_num; ii++) {
            MSG_PRINTF(LOG_WARN, "apn index: %d\r\n", ii);
            apn_item = cJSON_GetArrayItem(apn_list, ii);
            mcc_mnc = cJSON_GetObjectItem(apn_item, "mccmnc");
            if (!mcc_mnc) {
                MSG_PRINTF(LOG_WARN, "mcc mnc is error\n");
                continue;
            }
            apn = cJSON_GetObjectItem(apn_item, "apn");
            if (apn != NULL) {
                rt_os_memcpy(apn_name, apn->valuestring, rt_os_strlen(apn->valuestring));
                apn_name[rt_os_strlen(apn->valuestring)] = '\0';
                break;
            } else {
                apn_name[0] = '\0';
                MSG_PRINTF(LOG_WARN, "apn is error\n");
            }
        }
    } else {
        MSG_PRINTF(LOG_WARN, "apn list is error\n");
    }
    MSG_PRINTF(LOG_INFO, "APN_NAME:%s\n", apn_name);
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }

    return RT_SUCCESS;
}

/*****************************************************************************
 * FUNCTION
 *  msg_deal_with_download
 * DESCRIPTION
 *  input json data.
 * PARAMETERS
 *  command_content
 * RETURNS
 *  int
 *****************************************************************************/
int32_t msg_analyse_apn(cJSON *command_content, uint8_t *iccid)
{
    cJSON *apn_list = NULL;
    int8_t *out = NULL;
    uint8_t buffer[MSG_ONE_BLOCK_SIZE] = {0};  // must avoid stack buffer overflow
    int32_t ret = RT_ERROR;

    apn_list = cJSON_GetObjectItem(command_content, "apnInfos");
    if (!apn_list) {
        MSG_PRINTF(LOG_WARN, "Error before JSON: [%s]\n", cJSON_GetErrorPtr());
        return ret;
    }

    out = cJSON_PrintUnformatted(command_content);
    rt_os_memset(buffer, 'F', MSG_ONE_BLOCK_SIZE);
    rt_os_memcpy(buffer, out, rt_os_strlen(out));
    buffer[sizeof(buffer) - 1] = '\0';
    MSG_PRINTF(LOG_INFO, "iccid=%s, buffer=%s, rt_os_strlen(out)=%d\r\n", iccid, buffer, rt_os_strlen(out));
    ret = msg_insert(iccid, buffer);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "insert error\n");
    }

    rt_os_free(out);
    return ret;
}

int32_t mqtt_msg_event(const uint8_t *buf, int32_t len)
{
    int32_t status = 0;
    int32_t ret = RT_ERROR;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    //MSG_PRINTF(LOG_INFO, "msg: %s ==> method: %s ==> event: %s\n", downstream_msg->msg, downstream_msg->method, downstream_msg->event);

    ret = downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    if (ret == RT_ERROR) {
        return ret;
    }
    
    // MSG_PRINTF(LOG_WARN, "tranId: %s, %p\n", downstream_msg->tranId, downstream_msg->tranId);
    status = downstream_msg->handler(downstream_msg->private_arg,  downstream_msg->event, &downstream_msg->out_arg);
    upload_event_report(downstream_msg->event, (const char *)downstream_msg->tranId, status, downstream_msg->out_arg);

    return RT_SUCCESS;
}
