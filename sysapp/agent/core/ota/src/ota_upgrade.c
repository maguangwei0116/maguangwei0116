

#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "downstream.h"
#include "upload.h"

#include "cJSON.h"

typedef struct _ota_upgrade_target_t {
    
};

static int32_t ota_upgrade_parser(const void *in, char *tranId, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tran_id = NULL;

    MSG_PRINTF(LOG_WARN, "\n");

    msg =  cJSON_Parse((const char *)in);
    if (!msg) {
        MSG_PRINTF(LOG_WARN, "msg error\n");
        ret = -1;
        goto exit_entry;
    }

    tran_id = cJSON_GetObjectItem(msg, "tranId");
    if (!tran_id) {
        MSG_PRINTF(LOG_WARN, "tran_id error\n");
        ret = -2;
        goto exit_entry;
    }
    
    rt_os_strcpy(tranId, tran_id->valuestring);
    MSG_PRINTF(LOG_WARN, "tranId: %s, %p, stelen=%d\n", tranId, tranId, rt_os_strlen(tran_id->valuestring));

    ret = 0;

exit_entry:

    if (msg != NULL) {
        cJSON_Delete(msg);
        msg = NULL;
    }

    return ret;
}

static int32_t ota_upgrade_handler(const void *in, void **out)
{
    int32_t ret = 0;

    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return ret;
}

static cJSON *ota_upgrade_packer(void *arg)
{
    int32_t ret = 0;
    cJSON *app_version = NULL;
    const char *name = AGENT_LOCAL_NAME;
    const char *version = AGENT_LOCAL_VERSION;
    const char *chipModel = AGENT_LOCAL_PLATFORM_TYPE;

    app_version = cJSON_CreateObject();
    if (!app_version) {
        MSG_PRINTF(LOG_WARN, "The app_version is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(app_version, name);
    CJSON_ADD_NEW_STR_OBJ(app_version, version);
    CJSON_ADD_NEW_STR_OBJ(app_version, chipModel);
    
    ret = 0;
    
exit_entry:

    return !ret ? app_version : NULL;   
}

DOWNSTREAM_METHOD_OBJ_INIT(UPGRADE, MSG_ID_OTA_UPGRADE, ON_UPGRADE, ota_upgrade_parser, ota_upgrade_handler);

UPLOAD_EVENT_OBJ_INIT(ON_UPGRADE, ota_upgrade_packer);

