
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

static cJSON *upload_event_boot_network_info(void)
{
    int32_t ret = 0;
    cJSON *network = NULL;
    char *iccid = "12345678901234567890";
    char *mccmnc = "46000";
    char *type = "4G";
    int32_t dbm = -70;
    char *level = "3";

    network = cJSON_CreateObject();
    if (!network) {
        MSG_PRINTF(LOG_WARN, "The network is error\n");
        ret = -1;
        goto exit_entry;
    }

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

