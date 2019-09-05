
#include "upload.h"
#include "rt_type.h"
#include "rt_os.h"

#include "cJSON.h"

extern const char *g_push_channel;

#define CJSON_ADD_STR_OBJ(father_item, sub_item)        cJSON_AddItemToObject(father_item, #sub_item, (cJSON *)sub_item)
#define CJSON_ADD_NEW_STR_OBJ(father_item, str_item)    cJSON_AddItemToObject(father_item, #str_item, cJSON_CreateString(str_item))
#define CJSON_ADD_NEW_INT_OBJ(father_item, int_item)    cJSON_AddItemToObject(father_item, #int_item, cJSON_CreateNumber(int_item))

static cJSON *upload_registered_packer(void *arg)
{
    int32_t ret;
    cJSON *content = NULL;
    const char *event = "REGISTERED";
    const char *pushChannel = g_push_channel ? (const char *)g_push_channel : "EMQ";

    MSG_PRINTF(LOG_WARN, "\n----------------->%s g_push_channel=%p\n", event, g_push_channel);
    
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

UPLOAD_EVENT_OBJ_INIT(REGISTERED, upload_registered_packer);

