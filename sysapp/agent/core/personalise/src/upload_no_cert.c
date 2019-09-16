
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"
#include "device_info.h"
#include "profile_parse.h"
#include "file.h"

#include "cJSON.h"

extern const devicde_info_t *g_upload_device_info;
extern profile_data_t g_data;

static cJSON *upload_no_cert_packer(void *arg)
{
    int32_t ret;
    rt_fshandle_t fp;
    uint8_t buf[100];
    int32_t status = 0;
    int32_t length=0;
    cJSON *upload = NULL;
    cJSON *content = NULL;
    char *upload_json_pag = NULL;
    const char *event = "NO_CERT";
    const char *imei = g_upload_device_info->imei;
    const char *deviceId = g_upload_device_info->device_id;
    const char *model = g_upload_device_info->model;
    char fileVersion[128] = {0};
    uint8_t *p = NULL;

    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = -1;
        goto exit_entry;
    }

    fp = rt_fopen(SHARE_PROFILE, RT_FS_READ);
    if (fp == NULL) {
        ret = -1;
        goto exit_entry;
    }
    rt_fseek(fp, g_data.file_version_offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    p = (uint8_t * )get_value_buffer(buf);
    length = get_length(buf,0);
    rt_os_memcpy(fileVersion, p, length);

    CJSON_ADD_NEW_STR_OBJ(content, imei);
    CJSON_ADD_NEW_STR_OBJ(content, deviceId);
    CJSON_ADD_NEW_STR_OBJ(content, model);
    CJSON_ADD_NEW_STR_OBJ(content, fileVersion);

    ret = 0;

exit_entry:
    if (fp != NULL) {
        rt_fclose(fp);
    }

    return !ret ? content : NULL;
}

UPLOAD_EVENT_OBJ_INIT(NO_CERT, upload_no_cert_packer);

