
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_manager.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_os.h"
#include "card_manager.h"
#include "agent_queue.h"
#include "msg_process.h"
#include "card_ext.h"
#include "lpa.h"
#include "lpa_error_codes.h"

#define RT_LAST_EID                 "rt_last_eid"
#define RT_LAST_USED_CARD_TYPE      "rt_last_card_type"
#define RT_PROFILE_STATE_ENABLED    2

static card_info_t                  g_p_info;
static uint8_t                      g_last_eid[MAX_EID_LEN + 1] = {0};
static rt_bool                      g_frist_bootstrap_ok = RT_FALSE;

static rt_bool eid_check_memory(const void *buf, int32_t len, int32_t value)
{
    int32_t i = 0;
    const uint8_t *p = (const uint8_t *)buf;

    for (i = 0; i < len; i++) {
        if (p[i] != value) {
            return RT_FALSE;
        }
    }
    return RT_TRUE;
}

static int32_t card_check_init_upload(const uint8_t *eid)
{
    rt_bool update_last_eid = RT_FALSE;

    if (eid_check_memory(eid, MAX_EID_LEN, 'F') || eid_check_memory(eid, MAX_EID_LEN, '0')) {
        update_last_eid = RT_TRUE;
    }

    if (rt_os_strcmp((const char *)g_last_eid, (const char *)eid) && !update_last_eid) {
        MSG_PRINTF(LOG_INFO, "g_last_eid: %s, cur_eid: %s\r\n", g_last_eid, eid);
        MSG_PRINTF(LOG_WARN, "EID changed, upload INIT event\n");
        update_last_eid = RT_TRUE;
        upload_event_report("INIT", NULL, 0, NULL);
        msg_send_agent_queue(MSG_ID_MQTT, MSG_MQTT_SUBSCRIBE_EID, NULL, 0);
    }

    if (update_last_eid) {
        snprintf(g_last_eid, sizeof(g_last_eid), "%s", (const char *)eid);
        rt_write_data(RT_LAST_EID, 0, g_last_eid, sizeof(g_last_eid));
    }

    return RT_SUCCESS;
}

static int32_t card_last_eid_init(void)
{
    if (!linux_rt_file_exist(RT_LAST_EID)) {
        rt_create_file(RT_LAST_EID);
    } else {
        rt_read_data(RT_LAST_EID, 0, g_last_eid, sizeof(g_last_eid));
        MSG_PRINTF(LOG_INFO, "g_last_eid=%s\r\n", g_last_eid);
    }

    return RT_SUCCESS;
}

static int32_t card_last_type_init(void)
{
    if (!linux_rt_file_exist(RT_LAST_USED_CARD_TYPE)) {
        rt_create_file(RT_LAST_USED_CARD_TYPE);
    } else {
        rt_read_data(RT_LAST_USED_CARD_TYPE, 0, &(g_p_info.last_type), sizeof(profile_type_e));
        MSG_PRINTF(LOG_INFO, "g_p_info.last_type=%d\r\n", g_p_info.last_type);
    }

    return RT_SUCCESS;
}

static int32_t card_update_eid(rt_bool init)
{
    int32_t ret = RT_ERROR;
    uint8_t eid[MAX_EID_HEX_LEN] = {0};

    MSG_PRINTF(LOG_WARN, "card_update_eid()\n");
    ret = lpa_get_eid(eid);
    if (!ret) {
        bytes2hexstring(eid, sizeof(eid), g_p_info.eid);
        MSG_PRINTF(LOG_WARN, "g_p_info.eid=%s\r\n", g_p_info.eid);
    }

    if (!ret && !init) {
        card_check_init_upload(g_p_info.eid);
    }

    return ret;
}

int32_t card_update_profile_info(judge_term_e bootstrap_flag)
{
    int32_t ret = RT_ERROR;
    int32_t i;

    ret = lpa_get_profile_info(g_p_info.info, &g_p_info.num, THE_MAX_CARD_NUM);
    if (ret == RT_SUCCESS) {
        /* get current profile type */
        for (i = 0; i < g_p_info.num; i++) {
            MSG_PRINTF(LOG_INFO, "iccid   #%2d: %s state:%d type:%d\n", i + 1, g_p_info.info[i].iccid,
                g_p_info.info[i].state, g_p_info.info[i].class);
        }
        for (i = 0; i < g_p_info.num; i++) {
            if (g_p_info.info[i].state == PROFILE_ENABLED) {
                g_p_info.type = g_p_info.info[i].class;
                rt_os_memcpy(g_p_info.iccid, g_p_info.info[i].iccid, THE_MAX_CARD_NUM);
                g_p_info.iccid[THE_MAX_CARD_NUM] = '\0';
                break;
            }
        }
        if (i == g_p_info.num) {
            g_p_info.type = PROFILE_TYPE_TEST;
        }
        MSG_PRINTF(LOG_WARN, "using iccid: %s, type: %d, profile num: %d\n",
                g_p_info.iccid, g_p_info.type, g_p_info.num);
        if ((g_p_info.type == PROFILE_TYPE_TEST) ||
            (g_p_info.type == PROFILE_TYPE_PROVISONING)) {
            rt_write_data(RT_LAST_USED_CARD_TYPE, 0, &(g_p_info.type), sizeof(profile_type_e));
            if (bootstrap_flag == UPDATE_JUDGE_BOOTSTRAP) {
                rt_os_sleep(1);  // dealy some time after get profile info
                msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_BOOTSTRAP_SELECT_CARD, NULL, 0);
            }
        }
    }

    return ret;
}

static int32_t card_enable_profile(const uint8_t *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    MSG_PRINTF(LOG_INFO, "iccid:%s, len:%d\n", iccid, rt_os_strlen(iccid));
    for (ii = 0; ii < g_p_info.num; ii++) {
        if (rt_os_strncmp(g_p_info.info[ii].iccid, iccid, THE_ICCID_LENGTH) == 0) {
            if (g_p_info.info[ii].state == PROFILE_DISABLED) {
                if (g_p_info.info[ii].class == PROFILE_TYPE_PROVISONING) {
                    ret = lpa_enable_profile(iccid);
                } else if (g_p_info.info[ii].class == PROFILE_TYPE_OPERATIONAL) {
                    ret = msg_enable_profile(iccid);
                }
                if (ret != RT_SUCCESS) {
                    MSG_PRINTF(LOG_ERR, "Card enable %s failed ret:%d\n", iccid, ret);
                } else {
                    rt_os_sleep(1);  // must have
                }
                card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
            } else {
                ret = RT_PROFILE_STATE_ENABLED;
            }
        }
    }

    return ret;
}

static int32_t card_disable_profile(const uint8_t *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    MSG_PRINTF(LOG_INFO, "iccid:%s, len:%d\n", iccid, rt_os_strlen(iccid));
    for (ii = 0; ii < g_p_info.num; ii++) {
        if (rt_os_strncmp(g_p_info.info[ii].iccid, iccid, THE_ICCID_LENGTH) == 0) {
            if (g_p_info.info[ii].class == PROFILE_TYPE_PROVISONING) {
                if (g_p_info.info[ii].state != CARD_STATE_DISABLED) {
                    ret = lpa_disable_profile(iccid);
                } else {
                    ret = RT_SUCCESS;
                }
                if (ret != RT_SUCCESS) {
                    MSG_PRINTF(LOG_ERR, "Card enable %s failed ret:%d\n", iccid, ret);
                } else {
                    rt_os_sleep(1);  // must have
                }
                card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
            }
        }
    }

    return ret;
}

static int32_t card_get_provisioning_profile_iccid(char *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    for (ii = 0; ii < g_p_info.num; ii++) {
        if (g_p_info.info[ii].class == PROFILE_TYPE_PROVISONING) {
            rt_os_memcpy(iccid, g_p_info.info[ii].iccid, THE_ICCID_LENGTH);
            iccid[THE_ICCID_LENGTH] = '\0';
            ret = RT_SUCCESS;
            break;
        }
    }

    return ret;
}

static int32_t card_get_frist_operational_profile_iccid(char *iccid)
{
    int32_t ret = RT_ERROR;
    int32_t ii = 0;

    for (ii = 0; ii < g_p_info.num; ii++) {
        if (g_p_info.info[ii].class == PROFILE_TYPE_OPERATIONAL) {
            rt_os_memcpy(iccid, g_p_info.info[ii].iccid, THE_ICCID_LENGTH);
            iccid[THE_ICCID_LENGTH] = '\0';
            ret = RT_SUCCESS;
            break;
        }
    }

    return ret;
}

/* only enable provsioning profile */
int32_t card_force_enable_provisoning_profile(void)
{
    char iccid[THE_ICCID_LENGTH + 1] = {0};

    card_get_provisioning_profile_iccid(iccid);
    return msg_enable_profile(iccid);
}

/* enable provsioning profile && get current profile list */
int32_t card_force_enable_provisoning_profile_update(void)
{
    card_force_enable_provisoning_profile();
    rt_os_sleep(3);
    card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);

    return RT_SUCCESS;
}

static int32_t card_load_profile(const uint8_t *buf, int32_t len)
{
    int32_t ret = RT_SUCCESS;
    char iccid[THE_ICCID_LENGTH + 1] = {0};

    ret = card_get_provisioning_profile_iccid(iccid);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card get provisioning profile iccid fail\r\n");
    }

    ret = card_enable_profile(iccid);
    if (ret && ret != 2) { /* ret value 2: the iccid is enabled now ! */
        MSG_PRINTF(LOG_WARN, "card enable profile fail, ret=%d\r\n", ret);
    }

    if (ret != RT_PROFILE_STATE_ENABLED) {
        rt_os_sleep(3); // must have
    }

    if ((ret == RT_SUCCESS) || (ret == RT_PROFILE_STATE_ENABLED)) {
        ret = lpa_load_profile(buf, len);
        if (ret) {
            MSG_PRINTF(LOG_WARN, "lpa load porfile fail, ret=%d\r\n", ret);
        } else {
            /* set frist bootstrap flag */
            if (!g_frist_bootstrap_ok) {
                g_frist_bootstrap_ok = RT_TRUE;
            }
            rt_os_sleep(3); // must have
        }
    }

    ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card update profile info fail, ret=%d\r\n", ret);
    }

    return ret;
}

static int32_t card_load_cert(const uint8_t *buf, int32_t len)
{
    int32_t ret = RT_ERROR;

    do {
        MSG_PRINTF(LOG_INFO, "lpa load cert ...\r\n");
        ret = lpa_load_cert(buf, len);
        if (ret) {
            MSG_PRINTF(LOG_WARN, "lpa load cert fail, ret=%d\r\n", ret);
            break;
        }

        ret = card_update_eid(RT_FALSE);
        if (ret) {
            MSG_PRINTF(LOG_WARN, "card update eid fail, ret=%d\r\n", ret);
            break;
        }
    } while(0);

    return ret;
}

int32_t card_set_opr_profile_apn(void)
{
    int32_t ii = 0;

    for (ii = 0; ii < g_p_info.num; ii++) {
        if (g_p_info.info[ii].state == PROFILE_ENABLED && g_p_info.info[ii].class == PROFILE_TYPE_OPERATIONAL) {
            msg_set_apn(g_p_info.info[ii].iccid);
            return RT_SUCCESS;
        }
    }
    return RT_ERROR;
}

static int32_t card_init_profile_type(init_profile_type_e type)
{
    int32_t ret = RT_SUCCESS;

    MSG_PRINTF(LOG_INFO, "init profile type = %d\r\n", type);
    if (INIT_PROFILE_TYPE_LAST_USED != type) {
        char iccid[THE_ICCID_LENGTH + 1] = {0};

        ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
        if (ret) {
            MSG_PRINTF(LOG_WARN, "card update profile info fail, ret=%d\r\n", ret);
        }

        if (INIT_PROFILE_TYPE_PROVISONING == type) {
            if (g_p_info.type == PROFILE_TYPE_OPERATIONAL) { /* only enable profile when type is unmatched */
                ret = card_get_provisioning_profile_iccid(iccid);
                if (ret) {
                    MSG_PRINTF(LOG_WARN, "card get provisioning profile iccid fail\r\n");
                }

                ret = card_enable_profile(iccid);
                if (ret) {
                    MSG_PRINTF(LOG_WARN, "card enable profile fail, ret=%d\r\n", ret);
                }

                rt_os_sleep(1);
            }
        } else if (INIT_PROFILE_TYPE_OPERATIONAL == type) {
            if (g_p_info.type != PROFILE_TYPE_OPERATIONAL) { /* only enable profile when type is unmatched */
                ret = card_get_frist_operational_profile_iccid(iccid);
                if (ret) {
                    MSG_PRINTF(LOG_WARN, "card get provisioning profile iccid fail\r\n");
                }

                ret = card_enable_profile(iccid);
                if (ret) {
                    MSG_PRINTF(LOG_WARN, "card enable profile fail, ret=%d\r\n", ret);
                }

                rt_os_sleep(1);
            }
        }
    }

    return ret;
}

int32_t init_card_manager(void *arg)
{
    int32_t ret = RT_ERROR;

    init_profile_type_e init_profile_type;

    init_profile_type = ((public_value_list_t *)arg)->config_info->init_profile_type;
    ((public_value_list_t *)arg)->card_info = &g_p_info;
    init_msg_process(&g_p_info, ((public_value_list_t *)arg)->config_info->proxy_addr);
    rt_os_memset(&g_p_info, 0x00, sizeof(g_p_info));
    rt_os_memset(&g_p_info.eid, 'F', MAX_EID_LEN);
    rt_os_memset(&g_last_eid, 'F', MAX_EID_LEN);

    ret = card_last_type_init();
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card update last card type fail, ret=%d\r\n", ret);
    }

    ret = card_update_eid(RT_TRUE);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card update eid fail, ret=%d\r\n", ret);
        if (((public_value_list_t *)arg)->config_info->lpa_channel_type == LPA_CHANNEL_BY_QMI) {
#ifdef CFG_PLATFORM_ANDROID
            MSG_PRINTF(LOG_WARN, "eUICC mode with no EID on android paltform, waiting EID ready ...\r\n");
            while (1) {
                rt_os_sleep(5);
                ret = card_update_eid(RT_TRUE);
                if (!ret) {
                    break;
                }
            }
#else
            MSG_PRINTF(LOG_WARN, "eUICC mode with no EID, stay here forever !\r\n");
            while (1) {
                rt_os_sleep(1);
            }
#endif
        }
    } else {
        rt_os_sleep(1);
    }

    ret = card_init_profile_type(init_profile_type);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card init profile type fail, ret=%d\r\n", ret);
    }

    ret = card_update_profile_info(UPDATE_JUDGE_BOOTSTRAP);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card update profile info fail, ret=%d\r\n", ret);
    }

    ret = card_last_eid_init();
    if (ret) {
        MSG_PRINTF(LOG_WARN, "card update last eid fail, ret=%d\r\n", ret);
    }

    card_set_opr_profile_apn();

#ifdef CFG_PLATFORM_ANDROID
    if(g_p_info.type == PROFILE_TYPE_OPERATIONAL) {
        ret = lpa_enable_profile(g_p_info.iccid);
        if (ret == RT_PROFILE_STATE_ENABLED) {
            ret = RT_SUCCESS; /* profile enabled stateis ok */
        }
        if (ret != RT_SUCCESS) {
            MSG_PRINTF(LOG_WARN, "card enable profile fail, ret=%d\r\n", ret);
        }
    }
#endif

    return ret;
}

int32_t card_manager_install_profile_ok(void)
{
    return g_frist_bootstrap_ok;
}

int32_t card_get_avariable_profile_num(int32_t *avariable_num)
{
    if (avariable_num) {
        *avariable_num = THE_MAX_CARD_NUM - g_p_info.num;
        MSG_PRINTF(LOG_INFO, "profile num: %d - %d = %d\r\n", THE_MAX_CARD_NUM, g_p_info.num, *avariable_num);
        return RT_SUCCESS;
    }

    return RT_ERROR;
}

int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t ret = RT_ERROR;

    switch (mode) {
        case MSG_CARD_SETTING_KEY:
            break;

        case MSG_CARD_SETTING_PROFILE:
            ret = card_load_profile(buf, len);
            break;

        case MSG_CARD_SETTING_CERTIFICATE:
            ret = card_load_cert(buf, len);
            break;

        case MSG_FROM_MQTT:
            ret = mqtt_msg_event(buf, len);
            break;

        case MSG_NETWORK_DISCONNECTED:
            #if 0  // card detection do the same things
            rt_os_sleep(1);  // must have
            ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
            #endif
            break;

        case MSG_CARD_ENABLE_EXIST_CARD:
            ret = card_enable_profile(buf);
            if (ret == RT_ERR_APDU_STORE_DATA_FAIL) {
                MSG_PRINTF(LOG_INFO, "try to enable %s after 3 seconds\r\n", (const char *)buf);
                rt_os_sleep(3);  // wait 3 seconds, and try again !!!
                ret = card_enable_profile(buf);
            }
            break;

        case MSG_CARD_UPDATE:
            ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
            break;

        case MSG_CARD_UPDATE_SEED:
            ret = card_update_profile_info(UPDATE_JUDGE_BOOTSTRAP);
            break;

        case MSG_CARD_DISABLE_EXIST_CARD:
            ret = card_disable_profile(buf);
            break;
            
        default:
            //MSG_PRINTF(LOG_WARN, "unknow command\n");
            break;
    }

    return ret;
}

/* only update profiles when network connected */
int32_t card_manager_update_profiles_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t ret = RT_ERROR;

    switch (mode) {
        case MSG_NETWORK_CONNECTED:
            ret = card_check_init_upload(g_p_info.eid);
            ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);           
            break;
            
        default:
            //MSG_PRINTF(LOG_WARN, "unknow command\n");
            break;
    }

    return ret;
}

/* card operate API externally */
int32_t card_ext_get_eid(char *eid, int32_t size)
{
    int32_t ret = RT_ERROR;
    
    if (eid && size > MAX_EID_LEN) {
        snprintf(eid, size, "%s", g_p_info.eid);  // get eid from RAM
    } else {
        MSG_PRINTF(LOG_WARN, "NULL, or size too less\r\n"); 
    }

    return ret;
}

int32_t card_ext_get_profiles_info(char *profiles_info_json, int32_t size)
{
    int32_t ret = RT_ERROR;
    int32_t i = 0;
    cJSON *profiles_obj = NULL;
    cJSON *profiles = NULL;
    cJSON *profile = NULL;
    const char *iccid = NULL;
    char *profile_str = NULL;
    int32_t type;
    int32_t state;

    ret = card_update_profile_info(UPDATE_NOT_JUDGE_BOOTSTRAP);
    if (!ret) {
        profiles_obj = cJSON_CreateObject();
        profiles = cJSON_CreateArray();
        if (profiles_obj && profiles) {
            for (i = 0; i < g_p_info.num; i++) {
                profile = cJSON_CreateObject();
                if (profile) {
                    iccid = g_p_info.info[i].iccid;
                    type  = g_p_info.info[i].class;
                    state = g_p_info.info[i].state;
                    CJSON_ADD_NEW_STR_OBJ(profile, iccid);
                    CJSON_ADD_NEW_INT_OBJ(profile, type);
                    CJSON_ADD_NEW_INT_OBJ(profile, state);
                    cJSON_AddItemToArray(profiles, profile);
                }
            }
            CJSON_ADD_STR_OBJ(profiles_obj, profiles);
            profile_str = (char *)cJSON_PrintUnformatted(profiles_obj);
            if (profile_str && size >= MIN_PROFILES_LEN && profiles_info_json) {
                snprintf(profiles_info_json, size, "%s", profile_str);
                ret = RT_SUCCESS;
            } else {
                MSG_PRINTF(LOG_WARN, "NULL, or size too less\r\n"); 
                ret = RT_ERROR;
            }
        } else {
            MSG_PRINTF(LOG_WARN, "NULL, or size too less\r\n"); 
            ret = RT_ERROR;
        }
    } else {
        MSG_PRINTF(LOG_WARN, "update profiles fail, ret=%d\r\n", ret); 
    }

    if (profiles_obj) {
        cJSON_Delete(profiles_obj);
        profiles_obj = NULL;
    }

    if (profile_str) {
        cJSON_free(profile_str);
        profile_str = NULL;
    }

    return ret;
}

int32_t card_ext_delete_profile(const char *iccid)
{
    return lpa_delete_profile(iccid);
}

int32_t card_ext_enable_profile(const char *iccid)
{
    return lpa_enable_profile(iccid);
}

int32_t card_ext_disable_profile(const char *iccid)
{
    return lpa_disable_profile(iccid);
}

