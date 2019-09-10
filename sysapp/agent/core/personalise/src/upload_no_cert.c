
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"
#include "device_info.h"

#include "cJSON.h"

extern const devicde_info_t *g_upload_device_info;

static cJSON *upload_no_cert_packer(void *arg)
{
    int32_t ret;
    int32_t status = 0;
    cJSON *upload = NULL;
    cJSON *content = NULL;
    char *upload_json_pag = NULL;
    const char *event = "NO_CERT";
    const char *imei = g_upload_device_info->imei;
    const char *deviceId = g_upload_device_info->device_id;
    char *model = NULL;
    char *fileVersion = NULL;

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = -1;
        goto exit_entry;
    }
    CJSON_ADD_NEW_STR_OBJ(content, imei);
    CJSON_ADD_NEW_STR_OBJ(content, deviceId);

    ret = 0;

exit_entry:

    return !ret ? content : NULL;
}

UPLOAD_EVENT_OBJ_INIT(NO_CERT, upload_no_cert_packer);

