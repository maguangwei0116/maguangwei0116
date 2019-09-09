
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"

#include "cJSON.h"

static int32_t downstream_inpsect_parser(const void *in, char *tranId, void **out)
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

static int32_t downstream_inpsect_handler(const void *in, const char *event, void **out)
{
    int32_t ret = 0;

    (void)in;
    (void)event;
    *out = NULL;
    MSG_PRINTF(LOG_WARN, "\n");

exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(INSPECT, MSG_ID_IDLE, INFO, downstream_inpsect_parser, downstream_inpsect_handler);


