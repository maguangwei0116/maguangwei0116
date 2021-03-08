
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
#include "lpa_error_codes.h"
#include "usrdata.h"
#include "file.h"
#include "hash.h"
#include "config.h"

#define MSG_ONE_BLOCK_SIZE                  128
#define INSPECT_FILE_SIZE                   128
#define DEVICE_FIXED_KEY                    "e30211481a3ead603da4fbb06c62ed4c46aaac3394c31f20fbca686ba2efb043"
#define RT_CHECK_ERR(process, result)       if((process) == result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}

static proj_mode_e project_mode;
static const card_info_t *g_card_info;
static const char *g_smdp_proxy_addr = NULL;

int32_t init_msg_process(void *arg, void *proxy_addr, int32_t project)
{
    g_card_info = (card_info_t *)arg;
    g_smdp_proxy_addr = (const char *)proxy_addr;
    project_mode = project;

    if (!linux_rt_file_exist(RUN_CONFIG_FILE)) {
        rt_create_file(RUN_CONFIG_FILE);
    }

    return RT_SUCCESS;
}

static rt_bool msg_check_iccid_state(const char *iccid, profile_type_e *type)
{
    MSG_PRINTF(LOG_DBG, "g_iccid : %s, iccid : %s\n", g_card_info->iccid, iccid);
    if (rt_os_strncmp(g_card_info->iccid, iccid, THE_ICCID_LENGTH) == 0){
        if (type) {
            *type = g_card_info->type;
        }
        return RT_TRUE;
    }
    return RT_FALSE;
}

/**
 * FUNCTION
 *  msg_select
 * DESCRIPTION
 *  According iccid to data.
 * PARAMETERS
 *  iccid     iccid
 *  buffer    apn info
 * RETURNS
 *  int32_t
*/
static int32_t msg_select(const char *iccid, uint8_t *buffer)
{
    cJSON *s_iccid = NULL;
    cJSON *agent_msg = NULL;
    int32_t ii = 0;
    uint8_t tmp_buffer[MSG_ONE_BLOCK_SIZE + 1] = {0};

    while (1) {
        if (rt_read_apnlist(ii * MSG_ONE_BLOCK_SIZE, tmp_buffer, MSG_ONE_BLOCK_SIZE) < 0) {
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
    MSG_PRINTF(LOG_DBG, "can't find apn for iccid: %s \r\n", iccid);
    return RT_ERROR;
}

/**
 * FUNCTION
 *  msg_get_free_block
 * DESCRIPTION
 *  get one free block id.
 * PARAMETERS
 *  buffer
 * RETURNS
 *  int32_t
*/
static int32_t msg_get_free_block(uint8_t *buffer)
{
    int32_t ii = 0;
    while (1) {
        if (rt_read_apnlist(ii * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
            break;
        }
        ii++;
    }
    return ii;
}

/**
 * FUNCTION
 *  msg_insert
 * DESCRIPTION
 *  insert one item into file.
 * PARAMETERS
 *  iccid
 *  buffer      The data will be inserted.
 * RETURNS
 *  int32_t
*/
static int32_t msg_insert(uint8_t *iccid, uint8_t *buffer)
{
    int32_t num = 0;
    int32_t ret = RT_ERROR;
    uint8_t buff[MSG_ONE_BLOCK_SIZE];

    num = msg_select(iccid, buff);
    if (num == RT_ERROR) {
        num = msg_get_free_block(buff);
    }
    MSG_PRINTF(LOG_INFO, "Insert iccid %s apn data !\r\n", iccid);
    ret = rt_write_apnlist(num * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE);
    return ret;
}

/**
 * FUNCTION
 *  msg_get_op_apn_name
 * DESCRIPTION
 *  get apn name
 * PARAMETERS
 *  @apn_name back apn name.
 *  @iccid    according iccid to find apn name.
 * RETURNS
 *  void
*/
static int32_t msg_get_op_apn_name(const char *iccid, char *apn_name, char *mcc_mnc_out)
{
    cJSON *agent_msg = NULL;
    cJSON *apn_list = NULL;
    cJSON *apn_item = NULL;
    cJSON *apn = NULL;
    cJSON *mcc_mnc = NULL;
    uint8_t iccid_t[THE_ICCID_LENGTH + 1] = {0};
    int32_t apn_num = 0;
    int32_t ii = 0;
    int32_t num;
    uint8_t buffer[MSG_ONE_BLOCK_SIZE + 1] = {0};

    num = msg_select(iccid, buffer);
    if (num == RT_ERROR) {
        return RT_ERROR;
    }

    //MSG_PRINTF(LOG_INFO, "buffer=%s\r\n", buffer);
    agent_msg = cJSON_Parse(buffer);
    if (!agent_msg) {
        MSG_PRINTF(LOG_WARN, "agent_msg error, parse apn name fail !\n");
        return RT_ERROR;
    }

    apn_list = cJSON_GetObjectItem(agent_msg, "apnInfos");
    if (apn_list != NULL) {
        apn_num = cJSON_GetArraySize(apn_list);
        for (ii = 0; ii < apn_num; ii++) {
            MSG_PRINTF(LOG_TRACE, "apn index: %d/%d\r\n", ii+1, apn_num);
            apn_item = cJSON_GetArrayItem(apn_list, ii);
            mcc_mnc = cJSON_GetObjectItem(apn_item, "mccmnc");
            if (mcc_mnc != NULL) {
                rt_os_memcpy(mcc_mnc_out, mcc_mnc->valuestring, rt_os_strlen(mcc_mnc->valuestring));
                mcc_mnc_out[rt_os_strlen(mcc_mnc->valuestring)] = '\0';
            } else {
                mcc_mnc_out[0] = '\0';
                MSG_PRINTF(LOG_WARN, "mcc mnc is error\n");
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

    //MSG_PRINTF(LOG_INFO, "APN_NAME: %s, MCC_MNC: %s\n", apn_name, mcc_mnc_out);

    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }

    return RT_SUCCESS;
}

static int32_t msg_delete(const char *iccid)
{
    uint8_t buffer[MSG_ONE_BLOCK_SIZE+1] = {0};
    int32_t block = 0;
    int32_t num = 0;

    num = msg_get_free_block(buffer);
    block = msg_select(iccid, buffer);
    if (block == RT_ERROR || block >= num) {
        MSG_PRINTF(LOG_WARN, "can not find iccid\n");
        return RT_ERROR;
    }

    MSG_PRINTF(LOG_DBG, "num:%d,block:%d\n", num, block);

    if (block != num - 1) {
        if (rt_read_apnlist((num - 1) * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
            MSG_PRINTF(LOG_WARN, "rt read data failed\n");
            return RT_ERROR;
        } else {
            if (rt_write_apnlist(block * MSG_ONE_BLOCK_SIZE, buffer, MSG_ONE_BLOCK_SIZE) < 0) {
                MSG_PRINTF(LOG_WARN, "rt write data failed\n");
                return RT_ERROR;
            }
        }
    }

    rt_truncate_apnlist((num - 1) * MSG_ONE_BLOCK_SIZE);
    MSG_PRINTF(LOG_INFO, "delete iccid: %s\r\n", iccid);

    return RT_SUCCESS;
}

/**
EnableProfileResponse ::= [49] SEQUENCE { -- Tag 'BF31'
enableResult INTEGER {ok(0), iccidOrAidNotFound (1),
profileNotInDisabledState(2), disallowedByPolicy(3), wrongProfileReenabling(4),
catBusy(5), undefinedError(127)}
}
**/

static int32_t msg_enable_profile_check(const char *iccid)
{
    int32_t i = 0;
    int32_t ret = RT_FALSE;

    if (msg_check_iccid_state(iccid, NULL) == RT_TRUE) {
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

int32_t msg_delete_profile(const char *iccid, rt_bool *opr_iccid_using)
{
    int32_t ret;
    profile_type_e type;

    if (msg_check_iccid_state(iccid, &type) == RT_TRUE) {
        if (PROFILE_TYPE_OPERATIONAL == type || PROFILE_TYPE_SIM == type) {
            /* delete using operational profile */
            *opr_iccid_using = RT_TRUE;

            /* only disable profile when it's a operational profile */
            lpa_disable_profile(iccid);
            rt_os_sleep(3);
        }
    }
    ret = lpa_delete_profile(iccid);

    /* delete apn data when delete profile success or iccid not found */
    if (ret == 0 || ret == 1) {
        msg_delete(iccid);
    }

    return ret;
}

int32_t msg_download_profile(const char *ac, const char *cc, char iccid[21], int32_t avariable_num)
{
    if (avariable_num < 0) {
        MSG_PRINTF(LOG_WARN, "avariable prifle num: %d\n", avariable_num);
        return RT_ERR_PROFILE_NUM;
    }

    return lpa_download_profile(ac, cc, iccid, (uint8_t *)g_smdp_proxy_addr);
}

int32_t msg_set_apn(const char *iccid)
{
    char apn_name[128] = {0};
    char mcc_mnc[32] = {0};

    if (RT_SUCCESS == msg_get_op_apn_name(iccid, apn_name, mcc_mnc)) {
        MSG_PRINTF(LOG_INFO, "iccid:%s, set apn_name:%s  mcc_mnc:%s\n", iccid, apn_name, mcc_mnc);
        rt_qmi_modify_profile(1, 0, 0, apn_name, mcc_mnc);
    }

    return RT_SUCCESS;
}

/**
 * FUNCTION
 *  msg_deal_with_download
 * DESCRIPTION
 *  input json data.
 * PARAMETERS
 *  command_content
 * RETURNS
 *  int
*/
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
    if (out != NULL) {
        rt_os_memset(buffer, 'F', MSG_ONE_BLOCK_SIZE);
        rt_os_memcpy(buffer, out, rt_os_strlen(out));
        buffer[sizeof(buffer) - 1] = '\0';
        MSG_PRINTF(LOG_TRACE, "iccid=%s, buffer=%s, rt_os_strlen(out)=%d\r\n", iccid, buffer, rt_os_strlen(out));
        ret = msg_insert(iccid, buffer);
        if (ret != RT_SUCCESS) {
            MSG_PRINTF(LOG_WARN, "insert error\n");
        }

        rt_os_free(out);
    }

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

int32_t msg_apnlist_handler(cJSON *apnparams_list)
{
    cJSON *new_command_content = NULL;
    cJSON *apnparams_item = NULL;
    cJSON *apn_list = NULL;
    cJSON *iccid = NULL;
    int32_t state = RT_ERROR;
    int32_t apnparams_num = 0;
    uint8_t *apn_list_c;
    int16_t count_index = 0;
    int16_t ii = 0;

    apnparams_num = cJSON_GetArraySize(apnparams_list);
    count_index = apnparams_num - 1;
    for (ii = 0; ii < apnparams_num; ii++) {
        apnparams_item = cJSON_GetArrayItem(apnparams_list, ii);
        if (apnparams_item != NULL) {
            iccid = cJSON_GetObjectItem(apnparams_item, "iccid");
            apn_list = cJSON_GetObjectItem(apnparams_item, "apnInfos");

            if (apn_list != NULL) {
                // write file
                new_command_content = cJSON_CreateObject();
                if (!new_command_content) {
                    MSG_PRINTF(LOG_WARN, "The new_command_content is error!!\n");
                } else {
                    /* add iccid information */
                    cJSON_AddItemToObject(new_command_content, "iccid", cJSON_CreateString((const char *)iccid->valuestring));
                    /* add apn list information */
                    apn_list_c = cJSON_PrintUnformatted(apn_list);
                    if (apn_list_c != NULL) {
                        /* create a new apn list json object */
                        cJSON_AddItemToObject(new_command_content, "apnInfos", cJSON_Parse((const char *)apn_list_c));
                        state = msg_analyse_apn(new_command_content, iccid->valuestring);
                        cJSON_free(apn_list_c);
                    }
                    cJSON_Delete(new_command_content);

                    if (state == RT_ERROR) {
                        MSG_PRINTF(LOG_WARN, "start delete apn list\n");
                        if (ii >= count_index) {
                            cJSON_DeleteItemFromArray(apn_list, count_index);
                        } else {
                            cJSON_DeleteItemFromArray(apn_list, ii);
                        }
                        count_index--;
                    }
                }
            }
        }
    }

    return state;
}

rt_bool inspect_device_key(const char *file_name)
{
    rt_fshandle_t fp = NULL;
    sha256_ctx_t  sha_ctx;
    rt_bool ret = RT_FALSE;
    int8_t  file_buffer[DEVICE_KEY_LEN + 1];
    int8_t  hash_result[MAX_FILE_HASH_LEN + 1];
    int8_t  hash_out[MAX_FILE_HASH_BYTE_LEN + 1];
    int8_t  inspect_buff[DEVICE_KEY_LEN + 1];

    rt_os_memset(file_buffer, 0, DEVICE_KEY_LEN);
    rt_os_memset(inspect_buff, 0, DEVICE_KEY_LEN);

    RT_CHECK_ERR(fp = linux_fopen((char *)file_name, "r"), NULL);
    linux_fseek(fp, RT_DEVICE_KEY_OFFSET, RT_FS_SEEK_SET);
    linux_fread(file_buffer, DEVICE_KEY_LEN, 1, fp);

    sha256_init(&sha_ctx);
    sha256_update(&sha_ctx, (uint8_t *)file_buffer, DEVICE_KEY_LEN);         // app key
    sha256_update(&sha_ctx, (uint8_t *)DEVICE_FIXED_KEY, MAX_FILE_HASH_LEN);   // fix key
    sha256_final(&sha_ctx, (uint8_t *)hash_out);
    bytestring_to_charstring((const char *)hash_out, (char *)hash_result, MAX_FILE_HASH_BYTE_LEN);

    rt_read_devicekey(DEVICE_KEY_LEN, inspect_buff, DEVICE_KEY_LEN);
    inspect_buff[DEVICE_KEY_LEN] = '\0';
    if (!rt_os_strncasecmp(hash_result, inspect_buff, DEVICE_KEY_LEN)) {
        ret = RT_TRUE;
    } else {
        MSG_PRINTF(LOG_ERR, "Device Key compare fail!\n");
    }

end:
    if (fp != NULL) {
        linux_fclose(fp);
    }

    return ret;
}

rt_bool rt_get_device_key_status(void)
{
    return RT_TRUE;    
}

int32_t config_update_device_key(const char *devicekey)
{
    char send_buff[1];
    rt_bool ret = RT_FALSE;

    rt_write_devicekey(0, devicekey, DEVICE_KEY_SIZE);
    ret = rt_get_device_key_status();

    return ret;
}

int32_t msg_analyse_strategy(cJSON *command_content)
{
    int8_t *out = NULL;
    uint8_t buffer[RT_STRATEGY_LIST_LEN + 1] = {0};
    int ret = RT_ERROR;

    out = cJSON_PrintUnformatted(command_content);
    if (out != NULL) {
        rt_os_memset(buffer, 'F', RT_STRATEGY_LIST_LEN);
        rt_os_memcpy(buffer, out, rt_os_strlen(out));
        buffer[sizeof(buffer) - 1] = '\0';
        // MSG_PRINTF(LOG_INFO, "buffer=%s, rt_os_strlen(out)=%d\r\n", buffer, rt_os_strlen(out));

        ret = rt_write_strategy(0, buffer, RT_STRATEGY_LIST_LEN);
        rt_os_free(out);
    }

    return ret;
}
