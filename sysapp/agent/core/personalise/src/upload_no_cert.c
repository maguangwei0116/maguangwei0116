
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

extern const char *g_push_channel;

static cJSON *upload_no_cert_packer(void *arg)
{
    int32_t ret;
    int32_t status = 0;
    cJSON *upload = NULL;
    cJSON *content = NULL;
    char *upload_json_pag = NULL;
    const char *event = "NO_CERT";
    const char *imei = "866713030010830";
    const char *deviceId = "6e9d01b1f732b2704d9b2c8db0f6800e";
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
