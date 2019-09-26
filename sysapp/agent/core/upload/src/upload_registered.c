
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

extern const char *g_push_channel;

static cJSON *upload_registered_packer(void *arg)
{
    int32_t ret;
    cJSON *content = NULL;
    const char *event = "REGISTERED";
    const char *pushChannel = g_push_channel ? (const char *)g_push_channel : "EMQ";
   
    content = cJSON_CreateObject();
    if (!content) {
        MSG_PRINTF(LOG_WARN, "The content is error\n");
        ret = -1;
        goto exit_entry;
    }

    CJSON_ADD_NEW_STR_OBJ(content, pushChannel); 
    
    ret = 0;

exit_entry:

    return !ret ? content : NULL;
}

UPLOAD_EVENT_OBJ_INIT(REGISTERED, TOPIC_DEVICEID_OR_EID, upload_registered_packer);

