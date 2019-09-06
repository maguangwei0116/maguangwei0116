
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"
#include "card_manager.h"

#include "cJSON.h"

extern const char *g_upload_imei;
extern const profiles_info_t *g_upload_profiles_info;

static cJSON *upload_event_boot_device_info(void)
{
    int32_t ret = 0;
    cJSON *deviceInfo = NULL;
    const char *imei = g_upload_imei;
    char *deviceId = "2E11F8D5928F37D918797ECC46C9B763";
    char *sn = "0123456789ABDEF";
    char *model = "QUECTEL-EC20";

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

    if (!g_upload_profiles_info) {
        MSG_PRINTF(LOG_WARN, "The g_upload_profiles_info is error\n");
        ret = -2;
        goto exit_entry;
    }

    profiles_num = g_upload_profiles_info->num;

    for (i = 0; i < profiles_num; i++) {
        profile = cJSON_CreateObject();
        if (!profile) {
            MSG_PRINTF(LOG_WARN, "The profile is error\n");
            ret = -3;
            goto exit_entry;
        }
    
        iccid = g_upload_profiles_info->info[i].iccid;
        type = g_upload_profiles_info->info[i].class;
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
static void rt_get_network_info(uint8_t *mcc_mnc,uint8_t *net_type,uint8_t *leve,int32_t *dbm,uint8_t *iccid)
{
    uint16_t mcc_int = 0;
    uint16_t mnc_int = 0;
    int32_t j = 0;

    if (mcc_mnc == NULL) {
        return;
    }
    
#if 1 || (PLATFORM == PLATFORM_9X07)
    //used qmi to get network info
    rt_qmi_get_current_iccid(iccid);
    rt_qmi_get_mcc_mnc(&mcc_int,&mnc_int);
    j += sprintf(mcc_mnc, "%03d", 666);
    j += sprintf(mcc_mnc+j, "%02d", mnc_int);
    rt_qmi_get_signal(dbm);
    if (*dbm < -100) {
        leve[0] = '0';
    } else if (*dbm < -85) {
        leve[0] = '1';
    } else if (*dbm < -70) {
        leve[0] = '2';
    } else if (*dbm < -60) {
        leve[0] = '3';
    } else {
        leve[0] = '4';
    }
    leve[1]='\0';
    
#elif PLATFORM == PLATFORM_FIBCOM
    //used fibcom api to get network info
    int32_t signal;
    rt_fibcom_get_iccid(iccid);
    rt_fibcom_get_mcc_mnc(&mcc_int,&mnc_int);
    j += sprintf(mcc_mnc, "%03d", mcc_int);
    j += sprintf(mcc_mnc+j, "%02d", mnc_int);
    rt_fibcom_get_signal(&signal);
    sprintf(leve, "%d", signal);
    if (signal == 5) {
        *dbm = -51;
    } else if (signal == 4) {
        *dbm = -62;
    } else if (signal == 3) {
        *dbm = -71;
    } else if (signal == 2) {
        *dbm = -86;
    } else if (signal == 1) {
        *dbm = -93;
    } else if (signal == 0) {
        *dbm = -105;
    }
#endif

    j = rt_os_strlen(iccid);
    for (;j < 20; j++) {
        iccid[j] = 'F';
    }
    iccid[j] = '\0';
    MSG_PRINTF(LOG_DBG, "*leve:%c\n",*leve);
}

static cJSON *upload_event_boot_network_info(void)
{
    int32_t ret = 0;
    cJSON *network = NULL;
    char iccid[21] = {0};
    char mccmnc[8] = {0};
    char type[8] = {0};
    int32_t dbm = 0;
    char level[8] = {0};

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
        ret = -1;
        goto exit_entry;
    }

    rt_get_network_info(mccmnc, type, level, &dbm, iccid);

    CJSON_ADD_NEW_STR_OBJ(network, iccid);
    CJSON_ADD_NEW_STR_OBJ(network, mccmnc);
    CJSON_ADD_NEW_STR_OBJ(network, type);
    CJSON_ADD_NEW_INT_OBJ(network, dbm);
    CJSON_ADD_NEW_STR_OBJ(network, level);
    
    ret = 0;
    
exit_entry:

    return !ret ? network : NULL;
}

static cJSON *upload_event_boot_version_info(void)
{
    int32_t ret = 0;
    cJSON *software = NULL;
    cJSON *app_version = NULL;
    const char *name = AGENT_LOCAL_NAME;
    const char *version = AGENT_LOCAL_VERSION;

    software = cJSON_CreateArray();
    if (!software) {
        MSG_PRINTF(LOG_WARN, "The software is error\n");
        ret = -1;
        goto exit_entry;
    }

    app_version = cJSON_CreateObject();
    if (!app_version) {
        MSG_PRINTF(LOG_WARN, "The app_version is error\n");
        ret = -2;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(app_version, name);
    CJSON_ADD_NEW_STR_OBJ(app_version, version);
    cJSON_AddItemToArray(software, app_version);
    
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

    MSG_PRINTF(LOG_WARN, "\n----------------->%s\n", event);

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
        software = upload_event_boot_version_info();
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

UPLOAD_EVENT_OBJ_INIT(BOOT, upload_boot_packer);

