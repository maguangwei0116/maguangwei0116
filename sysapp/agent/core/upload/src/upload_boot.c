
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

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
        CJSON_ADD_STR_OBJ(content, deviceInfo);
    }
    CJSON_ADD_STR_OBJ(content, profiles);
    CJSON_ADD_STR_OBJ(content, network);
    if (!only_profile_network) {
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

