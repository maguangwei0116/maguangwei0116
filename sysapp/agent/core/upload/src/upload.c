
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : upload.c
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "upload.h"
#include "message_process.h"
#include "fallback.h"
#include "md5.h"
#include "upgrade.h"
#include "agent_config.h"
#include "api.h"
#include "lpa.h"
#include "rt_config.h"
#include "dial_up.h"

typedef struct MESSAGE_QUEUE {
    int64_t msg_typ;
    int32_t msg_id;
    int8_t tranid[THE_TRANDID_LEN];
    int32_t state;
    void *msg_buf;
} msg_que_t;

static int32_t g_queue_id = -1;
extern uint8_t g_current_iccid[];
extern uint8_t g_current_mcc[];
extern uint8_t g_eid[];
extern uint8_t g_imei[];


/*****************************************************************************
 * FUNCTION
 *  upload_create_nw_info
 * DESCRIPTION
 *  create network info json package
 * PARAMETERS
 *  content
 * RETURNS
 *  void
 *****************************************************************************/
static void upload_create_nw_info(cJSON *content)
{
    uint8_t mcc_mnc[7];
    uint8_t net_type[3] = "4g";
    uint8_t leve[2];
    uint8_t iccid[21];
    int32_t dbm = 0;
    cJSON *network_info = NULL;

    rt_get_network_info(mcc_mnc, net_type, leve, &dbm, iccid);
    network_info = cJSON_CreateObject();
    if (network_info == NULL) {
        MSG_PRINTF(LOG_WARN, "network_info error\n");
        return;
    }
    cJSON_AddItemToObject(network_info, "iccid", cJSON_CreateString(iccid));
    cJSON_AddItemToObject(network_info, "mccmnc", cJSON_CreateString(mcc_mnc));
    cJSON_AddItemToObject(network_info, "type", cJSON_CreateString(net_type));
    cJSON_AddItemToObject(network_info, "dbm", cJSON_CreateNumber(dbm));
    cJSON_AddItemToObject(network_info, "signalLevel", cJSON_CreateString(leve));
    cJSON_AddItemToObject(content, "networkInfo", network_info);
}

/*****************************************************************************
 * FUNCTION
 *  upload_create_all_profiles
 * DESCRIPTION
 *  create all profile json package
 * PARAMETERS
 *  content
 * RETURNS
 *  void
 *****************************************************************************/
static void upload_create_all_profiles(cJSON *content, fallback_result_t *info)
{
    cJSON *allProfile = NULL;
    cJSON *profile = NULL;
    cJSON *firmware = NULL;
    cJSON *components = NULL;
    cJSON *record = NULL;
    uint8_t *buf[100];
    euicc_card_t *all_Pf_info;
    uint8_t ii, jj;

    allProfile = cJSON_CreateArray();
    if (allProfile == NULL) {
        MSG_PRINTF(LOG_WARN, "allProfile error\n");
        return;
    }
    cJSON_AddItemToObject(content, "profiles", allProfile);

    all_Pf_info = get_all_profile();
    for (ii = 0; ii < all_Pf_info->num; ii++) {
        profile = cJSON_CreateObject();
        cJSON_AddItemToObject(profile, "iccid", cJSON_CreateString(all_Pf_info->profile_info[ii].iccid));
        cJSON_AddItemToObject(profile, "profileType", cJSON_CreateNumber(all_Pf_info->profile_info[ii].class));
        cJSON_AddItemToObject(profile, "state", cJSON_CreateNumber(all_Pf_info->profile_info[ii].state));
        cJSON_AddItemToObject(profile, "provider", cJSON_CreateString("redteamobile"));
        cJSON_AddItemToObject(profile, "owner", cJSON_CreateString("redtea"));
        if (info != NULL) {
            for (jj = 0; jj < all_Pf_info->num; jj++) {
                if (rt_os_strncmp(all_Pf_info->profile_info[ii].iccid, info->iccid[jj], rt_os_strlen(all_Pf_info->profile_info[ii].iccid)) == 0) {
                    if (info->fallback[jj] == RT_TRUE) {
                        cJSON_AddItemToObject(profile, "fallback", cJSON_CreateTrue());
                        cJSON_AddItemToObject(profile, "failureReason", cJSON_CreateString("No internet connection"));
                    } else {
                        cJSON_AddItemToObject(profile, "fallback", cJSON_CreateFalse());
                    }
                }
            }
        }
        cJSON_AddItemToArray(allProfile, profile);
    }
    firmware = cJSON_CreateObject();
    if (firmware == NULL) {
        MSG_PRINTF(LOG_WARN, "firmware error\n");
    }
    snprintf(buf, 100, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_REVSION);
    components = cJSON_CreateArray();
    if (components == NULL) {
        MSG_PRINTF(LOG_WARN, "components error\n");
    }
    record = cJSON_CreateObject();
    cJSON_AddItemToObject(record, "name", cJSON_CreateString(PROJECT_GIT_BRANCH));
    cJSON_AddItemToObject(record, "versionName", cJSON_CreateString(buf));
    cJSON_AddItemToObject(record, "versionCode", cJSON_CreateNumber(VERSION_CODE));
    cJSON_AddItemToArray(components, record);
    cJSON_AddItemToObject(firmware, "components", components);
    cJSON_AddItemToObject(firmware, "make", cJSON_CreateString(MAKE));
    cJSON_AddItemToObject(content, "firmware", firmware);
    upload_create_nw_info(content);
}

static void upload_add_items(cJSON *upload)
{
    //eid
    cJSON_AddItemToObject(upload, "cid", cJSON_CreateString(g_eid));
    //imei
    cJSON_AddItemToObject(upload, "imei", cJSON_CreateString(g_imei));
    //mcc
    cJSON_AddItemToObject(upload, "mcc", cJSON_CreateString((int8_t *)g_current_mcc));
}


static rt_bool upload_on_config(cJSON *upload, msg_que_t *que_info)
{
    cJSON *event_content = cJSON_CreateObject();
    cJSON *spec = cJSON_CreateObject();
    cJSON *sys_spec = cJSON_CreateObject();
    config_struct_t *config_data = (config_struct_t *)que_info->msg_buf;

    cJSON_AddItemToObject(spec, "initialProfileType", cJSON_CreateNumber(INIT_PROFILE_TYPE));

    cJSON_AddItemToObject(spec, "fallbackDuration", cJSON_CreateNumber(DIS_CONNECT_WAIT_TIME / 20));

    RPLMN_ENABLE?cJSON_AddItemToObject(spec, "rplmnSwitch", cJSON_CreateTrue()):cJSON_AddItemToObject(spec, "rplmnSwitch", cJSON_CreateFalse());

    config_data->ecm_enabled ? cJSON_AddItemToObject(sys_spec, "ecmEnabled", cJSON_CreateTrue()):cJSON_AddItemToObject(sys_spec, "ecmEnabled", cJSON_CreateFalse());
    config_data->telnet_enabled ? cJSON_AddItemToObject(sys_spec, "telnetEnabled", cJSON_CreateTrue()):cJSON_AddItemToObject(sys_spec, "telnetEnabled", cJSON_CreateFalse());
    config_data->ssh_enabled ? cJSON_AddItemToObject(sys_spec, "sshEnabled", cJSON_CreateTrue()):cJSON_AddItemToObject(sys_spec, "sshEnabled", cJSON_CreateFalse());

    cJSON_AddItemToObject(spec, "sys_spec", sys_spec);
    cJSON_AddItemToObject(event_content, "spec", spec);
    cJSON_AddItemToObject(upload,"customContent", event_content);
}


/*****************************************************************************
 * FUNCTION
 *  upload_deal_back_data
 * DESCRIPTION
 *  deal with back data.
 * PARAMETERS
 *  msg backup data.
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t upload_deal_back_data(int8_t *msg)
{
    int32_t state = RT_ERROR;
    cJSON *agent_msg = NULL;
    cJSON *back_state = NULL;

    agent_msg = cJSON_Parse(msg);
    if (agent_msg == NULL) {
        MSG_PRINTF(LOG_WARN, "agent_msg error\n");
        return RT_ERROR;
    }
    back_state = cJSON_GetObjectItem(agent_msg, "status");
    if (back_state == NULL) {
        MSG_PRINTF(LOG_WARN, "back_state error\n");
    } else {
        if (back_state->valueint == 0) {
            state = RT_SUCCESS;
        }
    }
    if (agent_msg != NULL) {
        cJSON_Delete(agent_msg);
    }
    return state;
}


/*****************************************************************************
 * FUNCTION
 *  upload_wait_network
 * DESCRIPTION
 *  wait disable enabel download delete.
 * PARAMETERS
 *  msg_que     queue data.
 * RETURNS
 *  int32_t
 *****************************************************************************/
static void upload_wait_network(msg_que_t *que_info)
{
    if (que_info == NULL) {
        return RT_ERROR;
    }

    msg_t *info = (msg_t *)que_info->msg_buf;
    if (que_info->msg_id == COMMAND_INIT) {
        MSG_PRINTF(LOG_WARN, "The command is error\n");
        return;
    }

    if (info != NULL) {
        if (que_info->state == GO_TO_WAIT) {
            switch(que_info->msg_id) {
                case ON_PUSH_ACTIVATION_CODE:
                    info->content = msg_wait_download(info->iccid, info->p_info.custom_content, NULL);
                    break;
                case ON_ENABLE_PROFILE:
                    que_info->state = msg_wait_enable(info->iccid);
                    break;
                case ON_DISABLE_PROFILE:
                    que_info->state = msg_wait_disable(info->iccid);
                    break;
                case ON_DELETE_PROFILE:
                    que_info->state = msg_wait_delete(info->iccid);
                    break;
                default:
                    break;
            }
        }
    }
}

/*****************************************************************************
 * FUNCTION
 *  upload_creat_json_pag
 * DESCRIPTION
 *  create json package.
 * PARAMETERS
 *  que_info  upload info.
 * RETURNS
 *  void
 *****************************************************************************/
static int32_t upload_creat_json_pag(msg_que_t *que_info,cJSON *upload)
{
    cJSON *command = NULL;
    cJSON *event_content = NULL;
    uint8_t *command_str =NULL;
    cJSON *all_profiles = NULL;
    cJSON *upgrade = NULL;
    msg_t *info = NULL;
    fallback_result_t *r_info = NULL;

    if (que_info->msg_id == COMMAND_INIT) {
        MSG_PRINTF(LOG_WARN, "The command is error\n");
        return RT_ERROR;
    }
    event_content = cJSON_CreateObject();
    if (event_content==NULL) {
        MSG_PRINTF(LOG_WARN, "The event_content is error\n");
    }

    cJSON_AddItemToObject(upload, "eventContent", event_content);

    if (que_info->msg_id != REGISTER_PUSH_ID) {
        cJSON_AddItemToObject(upload, "eventId", cJSON_CreateString("CUSTOM"));
        command = cJSON_CreateObject();
    }
    switch (que_info->msg_id) {
        case REGISTER_PUSH_ID:
            MSG_PRINTF(LOG_DBG, ("\n----------------->REGISTER_PUSH_ID\n");
            cJSON_AddItemToObject(upload, "eventId", cJSON_CreateString("REGISTER_PUSH_ID"));
            cJSON_AddItemToObject(event_content, "pushId", cJSON_CreateString(g_eid));
            cJSON_AddItemToObject(event_content, "pushChannel", cJSON_CreateString((const char *)rt_mqtt_get_channel()));
            break;

        case ON_PUSH_ACTIVATION_CODE:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_PUSH_ACTIVATION_CODE\n");
            info = (msg_t *)que_info->msg_buf;

            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_PUSH_ACTIVATION_CODE"));
                //eventContent
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            cJSON_AddItemToObject(command, "mode", cJSON_CreateString("1"));
            cJSON_AddItemToObject(command,"customContent", info->content);
            break;

        case ON_ENABLE_PROFILE:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_ENABLE_PROFILE\n");
            info = (msg_t *)que_info->msg_buf;
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_ENABLE_PROFILE"));
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            cJSON *profile_enable = cJSON_CreateObject();
            cJSON_AddItemToObject(profile_enable, "iccid", cJSON_CreateString(info->iccid));
            upload_create_nw_info(profile_enable);
            cJSON_AddItemToObject(command,"customContent", profile_enable);

            break;

        case ON_DISABLE_PROFILE:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_DISABLE_PROFILE\n");
            info = (msg_t *)que_info->msg_buf;
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_DISABLE_PROFILE"));
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            cJSON *profile_disable = cJSON_CreateObject();
            cJSON_AddItemToObject(profile_disable, "iccid", cJSON_CreateString(info->iccid));
            upload_create_nw_info(profile_disable);
            cJSON_AddItemToObject(command,"customContent", profile_disable);
            break;

        case ON_DELETE_PROFILE:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_DELETE_PROFILE\n");
            info = (msg_t *)que_info->msg_buf;
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_DELETE_PROFILE"));
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            cJSON *profile_delete = cJSON_CreateObject();
            cJSON_AddItemToObject(profile_delete, "iccid", cJSON_CreateString(info->iccid));
            upload_create_nw_info(profile_delete);
            cJSON_AddItemToObject(command,"customContent", profile_delete);
            break;

        case DEVICE_REBOOTED:
            MSG_PRINTF(LOG_DBG, ("\n----------------->DEVICE_REBOOTED\n");
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("DEVICE_REBOOTED"));
            break;

        case INTERNET_RECONNECTED:
            MSG_PRINTF(LOG_DBG, ("\n----------------->INTERNET_RECONNECTED\n");
            r_info = (fallback_result_t *)que_info->msg_buf;
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("INTERNET_RECONNECTED"));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(0));
            all_profiles = cJSON_CreateObject();
            cJSON_AddItemToObject(command,"customContent", all_profiles);
            upload_create_all_profiles(all_profiles, r_info);
            break;

        case ON_EUICC_LOOKUP:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_EUICC_LOOKUP\n");
            info = (msg_t *)que_info->msg_buf;
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_EUICC_LOOKUP"));
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            all_profiles = cJSON_CreateObject();
            cJSON_AddItemToObject(command,"customContent", all_profiles);
            upload_create_all_profiles(all_profiles, NULL);
            break;
        case ON_UPGRADE:
             MSG_PRINTF(LOG_DBG, ("\n----------------->ON_UPGRADE\n");
             upgrade_struct_t *upgrade_info = (upgrade_struct_t *)que_info->msg_buf;

             cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_UPGRADE"));
             cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(upgrade_info->tranid));
             cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
             upgrade = cJSON_CreateObject();
             cJSON_AddItemToObject(upgrade, "make", cJSON_CreateString(upgrade_info->make));
             cJSON_AddItemToObject(upgrade, "name", cJSON_CreateString(upgrade_info->fileName));
             if (que_info->state == 0) {
                 cJSON_AddItemToObject(upgrade, "currentVersion", cJSON_CreateString(upgrade_info->versionName));
             } else {
                 int8_t buf[10];
                 snprintf(buf, 100, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_REVSION);
                 cJSON_AddItemToObject(upgrade, "currentVersion",
                         cJSON_CreateString(buf));
             }

             cJSON_AddItemToObject(command,"customContent", upgrade);
            break;
        case ON_CONFIG:
            MSG_PRINTF(LOG_DBG, ("\n----------------->ON_CONFIG\n");
            cJSON_AddItemToObject(command, "customId", cJSON_CreateString("ON_CONFIG"));
            cJSON_AddItemToObject(command, "tranId", cJSON_CreateString(que_info->tranid));
            cJSON_AddItemToObject(command, "status", cJSON_CreateNumber(que_info->state));
            upload_on_config(command, que_info);
            break;
        default:
            MSG_PRINTF(LOG_WARN, "command error\n");
            break;
    }
    if (que_info->msg_id != REGISTER_PUSH_ID) {
        command_str = cJSON_PrintUnformatted(command);
        cJSON_AddItemToObject(event_content, "command", cJSON_CreateString((int8_t *)command_str));
        if (command != NULL) {
            cJSON_Delete(command);
        }
        rt_os_free(command_str);
    }
}


/*****************************************************************************
 * FUNCTION
 *  upload_send_request
 * DESCRIPTION
 *  used http to send data.
 * PARAMETERS
 *  out  send data.
 * RETURNS
 *  int32_t
 *****************************************************************************/
static int32_t upload_send_request(int8_t *out)
{
    int32_t ret = RT_ERROR;
    int8_t md5_out[MD5_STRING_LENGTH + 1];
    int8_t upload_url[MAX_OTI_URL_LEN + 1];

    if (out == NULL) {
        MSG_PRINTF(LOG_WARN, "out buffer error\n");
        return ret;
    }

    get_md5_string(out, md5_out);
    md5_out[MD5_STRING_LENGTH] = '\0';

    //send report by http
    STRUCTURE_OTI_URL(upload_url, MAX_OTI_URL_LEN + 1, OTI_ENVIRONMENT_ADDR, 7082, "/api/v1/report");
    ret = http_post(upload_url, out, md5_out, upload_deal_back_data);
    return ret;
}


#define WAIT_TIME_BASIC         20
#define UPLOAD_MAX_TRY_TIME     5
static void upload_post_data(msg_que_t *info)
{
    int8_t count = 0;
    cJSON *upload = NULL;
    int8_t *json_pag = NULL;
    http_result_e ret;
    upload = cJSON_CreateObject();
    if (upload == NULL) {
        MSG_PRINTF(LOG_WARN, "The upload is error\n");
        return;
    }
    upload_creat_json_pag(info, upload);
    upload_add_items(upload);
    json_pag = cJSON_PrintUnformatted(upload);
    MSG_PRINTF(LOG_INFO, "Upload:%s", json_pag);
    if (upload != NULL) {
        cJSON_Delete(upload);
    }
    while(1) {
        if (get_info_flag() == RT_SUCCESS) {
            ret = upload_send_request(json_pag);
            if (ret == HTTP_SYSTEM_CALL_ERROR ||
                       ret == HTTP_PARAMETER_ERROR ||
                       ret == HTTP_SOCKET_RECV_ERROR ||
                       ret == HTTP_RESPOND_ERROR ||
                       ret == HTTP_SUCCESS) {
                if (info->msg_id == REGISTER_PUSH_ID) {
                    set_push_state(PUSH_STATE_SUCCESS);
                } else if (info->msg_id == ON_UPGRADE) {
                    rt_os_reboot();
                }
                break;
            } else if (ret == HTTP_SOCKET_CONNECT_ERROR ||
                       ret == HTTP_SOCKET_SEND_ERROR) {
                if (count < UPLOAD_MAX_TRY_TIME) {
                    rt_os_sleep(WAIT_TIME_BASIC * pow(2, count++));
                    continue;
                } else {
                    MSG_PRINTF(LOG_WARN, "------Upload message false\n");
                    if (info->msg_id == REGISTER_PUSH_ID) {
                        set_push_state(PUSH_STATE_ERROR);
                    }
                    break;
                }
            }
        } else {
            rt_os_sleep(3);
            continue;
        }
        rt_os_sleep(5);
    }
    if (info->msg_id == ON_PUSH_ACTIVATION_CODE) {  // enable card
        msg_t *msg = (msg_t *)info->msg_buf;
        if (get_used_profile_type() == PROFILE_PROVISIONING) {
            msg_wait_enable(msg->iccid);
        }
    }
    rt_os_free(json_pag);
}

/*****************************************************************************
 * FUNCTION
 *  upload_task
 * DESCRIPTION
 *  deal with download event,create Json page,upload data.
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
static void upload_task(void)
{
    msg_que_t msg_data;
    int32_t len = sizeof(msg_que_t) - sizeof(int64_t);
    while(1) {
        if (rt_receive_queue_msg(g_queue_id, &msg_data, len) == RT_SUCCESS) {
            upload_wait_network(&msg_data);
            while(get_network_state() != NETWORK_USING) {
                rt_os_sleep(1);
            }
            upload_post_data(&msg_data);
            rt_os_free(msg_data.msg_buf);
        }
        rt_os_sleep(3);
    }
}


/*****************************************************************************
 * FUNCTION
 *  upload_task
 * DESCRIPTION
 *  deal with download event,create Json page,upload data.
 * PARAMETERS
 *  tranid
 *  cmd     customId id
 *  state   process result
 *  info    upload value
 * RETURNS
 *  void
 *****************************************************************************/
void upload_set_values(up_command_e cmd, void *info_buf)
{
    msg_upload_data(NULL, cmd, 0, info_buf);
}

/*****************************************************************************
 * FUNCTION
 *  msg_upload_data
 * DESCRIPTION
 *  make upload data.
 * PARAMETERS
 *  tranid
 *  cmd    command
 *  state
 *  info  iccid and all
 * RETURNS
 *  void
 *****************************************************************************/
void msg_upload_data(const int8_t *tranid, up_command_e cmd, int32_t state, void *info1)
{
    msg_que_t msg_data;
    int32_t len = 0;

    if ((cmd == UN_KNOWN) || (cmd == COMMAND_INIT)) {
        return;
    }

    if (tranid != NULL) {   //tranid
        rt_os_memcpy(msg_data.tranid, tranid, rt_os_strlen(tranid));
        msg_data.tranid[rt_os_strlen(tranid)] = '\0';
    }

    if (info1 != NULL) {
        msg_data.msg_buf = info1;
    }

    msg_data.msg_id = cmd;
    msg_data.state = state;
    msg_data.msg_buf = info1;
    len = sizeof(msg_que_t) - sizeof(int64_t);
    msg_data.msg_typ = 1;
    if (rt_send_queue_msg(g_queue_id, &msg_data, len) < 0) {
        MSG_PRINTF(LOG_WARN, "send queue data error\n");
    }
}

/*****************************************************************************
 * FUNCTION
 *  init_upload
 * DESCRIPTION
 *  create upload task,and msg queue
 * PARAMETERS
 *  void
 * RETURNS
 *  int
 *****************************************************************************/
int32_t init_upload(void)
{
    rt_task task_id = 0;
    int32_t ret = 0;
    ret = rt_create_task("upload_task", &task_id,(void *) upload_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "create upload task error\n");
    }
    g_queue_id = rt_creat_msg_queue();
    if (g_queue_id < RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "creat msg queue error\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

