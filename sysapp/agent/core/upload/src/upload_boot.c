
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

#include "cJSON.h"

extern const devicde_info_t *g_upload_device_info;
extern const card_info_t *g_upload_card_info;
extern const target_versions_t *g_upload_ver_info;

static cJSON *upload_event_boot_device_info(void)
{
    int32_t ret         = 0;
    cJSON *deviceInfo   = NULL;
    const char *imei    = g_upload_device_info->imei;
    const char *deviceId= g_upload_device_info->device_id;
    const char *sn      = g_upload_device_info->sn;
    const char *model   = g_upload_device_info->model;

    deviceInfo = cJSON_CreateObject();
    if (!deviceInfo) {
        MSG_PRINTF(LOG_WARN, "The deviceInfo is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(deviceInfo, imei);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, deviceId);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, sn);
    CJSON_ADD_NEW_STR_OBJ(deviceInfo, model);
    
    ret = 0;
    
exit_entry:

    return !ret ? deviceInfo : NULL;
}

static cJSON *upload_event_boot_profiles_info(void)
{
    int32_t i = 0;
    int32_t ret = 0;
    cJSON *profiles = NULL;
    cJSON *profile = NULL;
    const char *iccid = NULL;
    int32_t type;
    uint8_t profiles_num = 0;

    profiles = cJSON_CreateArray();
    if (!profiles) {
        MSG_PRINTF(LOG_WARN, "The profiles is error\n");
        ret = -1;
        goto exit_entry;
    }

    if (!g_upload_card_info) {
        MSG_PRINTF(LOG_WARN, "The g_upload_card_info is error\n");
        ret = -2;
        goto exit_entry;
    }

    profiles_num = g_upload_card_info->num;

    for (i = 0; i < profiles_num; i++) {
        profile = cJSON_CreateObject();
        if (!profile) {
            MSG_PRINTF(LOG_WARN, "The profile is error\n");
            ret = -3;
            goto exit_entry;
        }
    
        iccid = g_upload_card_info->info[i].iccid;
        type = g_upload_card_info->info[i].class;
        CJSON_ADD_NEW_STR_OBJ(profile, iccid);
        CJSON_ADD_NEW_INT_OBJ(profile, type);
        cJSON_AddItemToArray(profiles, profile);
    }

    ret = 0;
    
exit_entry:

    return !ret ? profiles : NULL;
}

/*****************************************************************************
 * FUNCTION
 *  get_euicc_iccid.
 * DESCRIPTION
 *  get the iccid of provisioning card.
 * PARAMETERS
 *  @iccid    provisioning card iccid.
  * RETURNS
 *  @void.
 *****************************************************************************/
static int32_t rt_get_euicc_iccid(uint8_t *iccid)
{
#if 1 || (PLATFORM == PLATFORM_9X07)
    return rt_qmi_get_current_iccid(iccid);
#elif PLATFORM == PLATFORM_FIBCOM
    return rt_fibcom_get_iccid(iccid);
#endif
    return 0;
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
static void rt_get_network_info(uint8_t *mcc_mnc, uint8_t *net_type, uint8_t *level, int32_t *dbm, uint8_t *iccid)
{
    uint16_t mcc_int = 0;
    uint16_t mnc_int = 0;
    int32_t j = 0;
    int32_t ret;

    if (mcc_mnc == NULL) {
        return;
    }

    /* used qmi to get network info */
    ret = rt_qmi_get_current_iccid(iccid);
    if (ret) {
        /* get local current iccid instead */
        if (g_upload_card_info) {
            MSG_PRINTF(LOG_WARN, "get card manager using iccid ...\r\n");
            sprintf(iccid, "%s", g_upload_card_info->iccid);
        }
    }

    
    rt_qmi_get_mcc_mnc(&mcc_int,&mnc_int);
    j += sprintf(mcc_mnc, "%03d", mcc_int);
    j += sprintf(mcc_mnc+j, "%02d", mnc_int);

    /* signal level: [1,5] */
    rt_qmi_get_signal(dbm);
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
    rt_qmi_get_network_type(net_type);

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
    for (;j < 20; j++) {
        iccid[j] = 'F';
    }
    iccid[j] = '\0';
    MSG_PRINTF(LOG_DBG, "*level:%c\n", *level);
}

static cJSON *upload_event_boot_network_info(void)
{
    int32_t ret = 0;
    cJSON *network = NULL;
    char iccid[21] = {0};
    char mccmnc[8] = {0};
    char type[8] = {0};
    int32_t dbm = 0;
    char signalLevel[8] = {0};

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
        ret = -1;
        goto exit_entry;
    }

    rt_get_network_info(mccmnc, type, signalLevel, &dbm, iccid);

    CJSON_ADD_NEW_STR_OBJ(network, iccid);
    CJSON_ADD_NEW_STR_OBJ(network, mccmnc);
    CJSON_ADD_NEW_STR_OBJ(network, type);
    CJSON_ADD_NEW_INT_OBJ(network, dbm);
    CJSON_ADD_NEW_STR_OBJ(network, signalLevel);
    
    ret = 0;
    
exit_entry:

    return !ret ? network : NULL;
}

static cJSON *upload_event_software_version_info(void)
{
    int32_t ret = 0;
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
        ret = -1;
        goto exit_entry;
    }
    
    for (i_type = 0; i_type < TARGET_TYPE_MAX; i_type++) {
        single_version = cJSON_CreateObject();
        if (!single_version) {
            MSG_PRINTF(LOG_WARN, "The single_version is error\n");
            ret = -2;
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

    ret = 0;
    
exit_entry:

    return !ret ? software : NULL;
}

cJSON *upload_event_boot_info(const char *str_event, rt_bool only_profile_network)
{
    int32_t ret = 0;
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
        ret = -1;
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

    ret = 0;
    
exit_entry:

    return !ret ? content : NULL;
}

static cJSON *upload_boot_packer(void *arg)
{
    return upload_event_boot_info("BOOT", RT_FALSE);
}

UPLOAD_EVENT_OBJ_INIT(BOOT, TOPIC_DEVICEID_OR_EID, upload_boot_packer);

