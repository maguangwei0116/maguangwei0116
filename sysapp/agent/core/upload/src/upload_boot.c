
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"
#include "card_manager.h"
#include "device_info.h"
#include "bootstrap.h"
#include "libcomm.h"
#include "upgrade.h"
#include "agent_queue.h"
#include "rt_qmi.h"

#include "usrdata.h"
#include "cJSON.h"

extern const devicde_info_t *g_upload_device_info;
extern const card_info_t *g_upload_card_info;
extern const target_versions_t *g_upload_ver_info;

static rt_bool device_key_check_memory(const void *buf, int32_t len, int32_t value)
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

static cJSON *upload_event_boot_device_info(void)
{
    int32_t ret             = RT_ERROR;
    cJSON *deviceInfo       = NULL;
    const char *imei        = g_upload_device_info->imei;
    const char *deviceId    = g_upload_device_info->device_id;
    const char *sn          = g_upload_device_info->sn;
    const char *model       = g_upload_device_info->model;
    char deviceKey[DEVICE_KEY_SIZE + 1] = {0};

    deviceInfo = cJSON_CreateObject();
    if (!deviceInfo) {
        MSG_PRINTF(LOG_WARN, "The deviceInfo is error\n");
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(deviceInfo, imei);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, deviceId);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, sn);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, model);

#ifdef CFG_REDTEA_READY_ON
    rt_read_devicekey(0, deviceKey, DEVICE_KEY_SIZE);
    if (device_key_check_memory(deviceKey, DEVICE_KEY_SIZE, 'F')) {
        MSG_PRINTF(LOG_WARN, "upload device key : %s\n", deviceKey);
    } else {
        deviceKey[DEVICE_KEY_SIZE] = '\0';
        CJSON_ADD_NEW_STR_OBJ(deviceInfo, deviceKey);
    }
    
#endif

    ret = RT_SUCCESS;
    
exit_entry:

    return !ret ? deviceInfo : NULL;
}

static cJSON *upload_event_boot_profiles_info(void)
{
    int32_t i = 0;
    int32_t ret = RT_ERROR;
    cJSON *profiles = NULL;
    cJSON *profile = NULL;
    const char *iccid = NULL;
    int32_t type;
    uint8_t profiles_num = 0;

    profiles = cJSON_CreateArray();
    if (!profiles) {
        MSG_PRINTF(LOG_WARN, "The profiles is error\n");
        goto exit_entry;
    }

    if (!g_upload_card_info) {
        MSG_PRINTF(LOG_WARN, "The g_upload_card_info is error\n");
        goto exit_entry;
    }

    profiles_num = g_upload_card_info->num;

    for (i = 0; i < profiles_num; i++) {
        profile = cJSON_CreateObject();
        if (!profile) {
            MSG_PRINTF(LOG_WARN, "The profile is error\n");
            goto exit_entry;
        }
    
        iccid = g_upload_card_info->info[i].iccid;
        type = g_upload_card_info->info[i].class;
        CJSON_ADD_NEW_STR_OBJ(profile, iccid);
        CJSON_ADD_NEW_INT_OBJ(profile, type);
        cJSON_AddItemToArray(profiles, profile);
    }

#ifdef CFG_REDTEA_READY_ON
    if (g_upload_card_info->sim_info.state == 1) {
        profile = cJSON_CreateObject();
        if (!profile) {
            MSG_PRINTF(LOG_WARN, "The profile is error\n");
            goto exit_entry;
        }

        iccid = g_upload_card_info->sim_info.iccid;
        type = PROFILE_TYPE_SIM;
        CJSON_ADD_NEW_STR_OBJ(profile, iccid);
        CJSON_ADD_NEW_INT_OBJ(profile, type);
        cJSON_AddItemToArray(profiles, profile);
    }
#endif

    ret = RT_SUCCESS;
    
exit_entry:

    return !ret ? profiles : NULL;
}

static int32_t last_3_dbms_func(rt_bool in, int32_t *dbm)
{
    static int8_t  index = 0;
    static int32_t last_3_dbms[3] = {0}; 

    if (in) {
        last_3_dbms[index] = *dbm;
        index = ((index + 1) >= 3) ? 0 : (index + 1);
    } else {
        *dbm = (last_3_dbms[0] + last_3_dbms[1] + last_3_dbms[2]) / 3;
    }

    return RT_SUCCESS;
}

/*
return last 3 times average dbm when qmi-get-signal fail !!
*/
static int32_t rt_get_cur_signal(int32_t *dbm)
{
    int32_t ret = rt_qmi_get_signal(dbm);
    if (ret) {
        MSG_PRINTF(LOG_WARN, "get current signal fail, try to get average dbm, ret=%d\r\n", ret);
        last_3_dbms_func(RT_FALSE, dbm);  // get average dbm        
    } else {
        last_3_dbms_func(RT_TRUE, dbm);  // set current dbm
    }

    return ret;
}

/*****************************************************************************
 * FUNCTION
 *  get_network_info
 * DESCRIPTION
 *  get mccmnc,network type and signial leve
 * PARAMETERS
 *  @mcc_mnc  regist operational
 *  @net_type network type(2g,3g or 4g)
 *  @leve     signial leve
 * RETURNS
 *  @void
 *****************************************************************************/
static void rt_get_network_info(char *mcc_mnc, int32_t mcc_mnc_size,
                                    char *net_type, int32_t net_type_size,
                                    uint8_t *level, int32_t *dbm, 
                                    char *iccid, int32_t iccid_size, 
                                    int32_t *profileType)
{
    uint16_t mcc_int = 0;
    uint16_t mnc_int = 0;
    int32_t j = 0;
    int32_t ret = RT_ERROR;

    if (mcc_mnc == NULL) {
        return;
    }

    /* used qmi to get network info */
    ret = rt_qmi_get_current_iccid(iccid, iccid_size);
    if (ret) {
        /* get local current iccid instead */
        if (g_upload_card_info) {
            MSG_PRINTF(LOG_WARN, "get card manager using iccid ...\r\n");
            snprintf(iccid, iccid_size, "%s", g_upload_card_info->iccid);
        }
    }

    /* get current profile type */
    if (g_upload_card_info && profileType) {
        *profileType = (int32_t)g_upload_card_info->type;
    }
    
    rt_qmi_get_mcc_mnc(&mcc_int, &mnc_int);
    snprintf(mcc_mnc, mcc_mnc_size, "%03d%02d", mcc_int, mnc_int);

    /* signal level: [1,5] */
    rt_get_cur_signal(dbm);
    if (*dbm < -100) {
        level[0] = '1';
    } else if (*dbm < -85) {
        level[0] = '2';
    } else if (*dbm < -70) {
        level[0] = '3';
    } else if (*dbm < -60) {
        level[0] = '4';
    } else {
        level[0] = '5';
    }
    level[1]='\0';
    rt_qmi_get_network_type(net_type, net_type_size);

#ifdef CFG_PLATFORM_ANDROID
    {
        /* get android network signal level */
        int32_t signal_level = 0;
        rt_qmi_get_signal_level(&signal_level);
        if (0 <= signal_level && signal_level <= 6) {
            level[0] = '1' + signal_level;
        }
    }
#endif

    j = rt_os_strlen(iccid);
    for (;j < THE_ICCID_LENGTH; j++) {
        iccid[j] = 'F';
    }
    iccid[j] = '\0';
    MSG_PRINTF(LOG_INFO, "*level:%c\n", *level);
}

static cJSON *upload_event_boot_network_info(void)
{
    int32_t ret = RT_ERROR;
    cJSON *network = NULL;
    char iccid[THE_ICCID_LENGTH + 1] = {0};
    char mccmnc[8] = {0};
    char type[8] = {0};
    int32_t profileType = 0;
    int32_t dbm = 0;
    char signalLevel[8] = {0};

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
        goto exit_entry;
    }

    rt_get_network_info(mccmnc, sizeof(mccmnc), type, sizeof(type), signalLevel, &dbm, iccid, sizeof(iccid), &profileType);

    CJSON_ADD_NEW_STR_OBJ(network, iccid);
    CJSON_ADD_NEW_INT_OBJ(network, profileType);
    CJSON_ADD_NEW_STR_OBJ(network, mccmnc);
    CJSON_ADD_NEW_STR_OBJ(network, type);
    CJSON_ADD_NEW_INT_OBJ(network, dbm);
    CJSON_ADD_NEW_STR_OBJ(network, signalLevel);
    
    ret = RT_SUCCESS;
    
exit_entry:

    return !ret ? network : NULL;
}

static cJSON *upload_event_software_version_info(void)
{
    int32_t ret = RT_ERROR;
    cJSON *software = NULL;
    cJSON *single_version = NULL;
    int32_t i_type;
    target_type_e type;
    const char *name = "";
    const char *version = "";
    const char *chipModel = "";

    software = cJSON_CreateArray();
    if (!software) {
        MSG_PRINTF(LOG_WARN, "The software is error\n");
        goto exit_entry;
    }
    
    for (i_type = 0; i_type < TARGET_TYPE_MAX; i_type++) {
        single_version = cJSON_CreateObject();
        if (!single_version) {
            MSG_PRINTF(LOG_WARN, "The single_version is error\n");
            goto exit_entry;
        } else {
            type = (target_type_e)i_type;
            name = g_upload_ver_info->versions[i_type].name;
            version = g_upload_ver_info->versions[i_type].version;
            chipModel = g_upload_ver_info->versions[i_type].chipModel;
            CJSON_ADD_NEW_STR_OBJ(single_version, name);
            CJSON_ADD_NEW_STR_OBJ(single_version, version);
            CJSON_ADD_NEW_STR_OBJ(single_version, chipModel);        
            CJSON_ADD_NEW_INT_OBJ(single_version, type);
            cJSON_AddItemToArray(software, single_version);
        }
    }

    ret = RT_SUCCESS;
    
exit_entry:

    return !ret ? software : NULL;
}

cJSON *upload_event_boot_info(const char *str_event, rt_bool only_profile_network)
{
    int32_t ret = RT_ERROR;
    int32_t status = 0;
    cJSON *content = NULL;
    cJSON *deviceInfo = NULL;
    cJSON *profiles = NULL;
    cJSON *network = NULL;
    cJSON *software = NULL;
    const char *event = str_event;

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
    }

    if (!only_profile_network) {
        deviceInfo = cJSON_CreateObject();
        if (!deviceInfo) {
            MSG_PRINTF(LOG_WARN, "The deviceInfo is error\n");
        }
    }

    profiles = cJSON_CreateObject();
    if (!profiles) {
        MSG_PRINTF(LOG_WARN, "The profiles is error\n");
    }

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
    }

    if (!only_profile_network) {
        software = cJSON_CreateObject();
        if (!software) {
            MSG_PRINTF(LOG_WARN, "The software is error\n");
        }
    }

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        goto exit_entry;
    }

    if (!only_profile_network) {
        deviceInfo = upload_event_boot_device_info();
        CJSON_ADD_STR_OBJ(content, deviceInfo);
    }

    profiles = upload_event_boot_profiles_info();
    CJSON_ADD_STR_OBJ(content, profiles);

    network = upload_event_boot_network_info();
    CJSON_ADD_STR_OBJ(content, network);
    
    if (!only_profile_network) {
        software = upload_event_software_version_info();
        CJSON_ADD_STR_OBJ(content, software);
    }

    ret = RT_SUCCESS;
    
exit_entry:

    return !ret ? content : NULL;
}

static cJSON *upload_boot_packer(void *arg)
{
    return upload_event_boot_info("BOOT", RT_FALSE);
}

UPLOAD_EVENT_OBJ_INIT(BOOT, TOPIC_DEVICEID_OR_EID, upload_boot_packer);

