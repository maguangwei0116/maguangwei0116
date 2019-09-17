
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"

#include "cJSON.h"

static int32_t downstream_reboot_parser(const void *in, char *tranId, void **out)
{
    int32_t ret;
    cJSON *msg = NULL;
    cJSON *tran_id = NULL;

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

static int32_t downstream_reboot_handler(const void *in, const char *event, void **out)
{
    int32_t ret = 0;

    (void)in;
    (void)event;
    *out = NULL;
    
    MSG_PRINTF(LOG_WARN, "wait 3 seconds to restart agent ...\n");
    rt_os_sleep(3);
    rt_os_exit(-1);  // reboot agent only !!!

exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(REBOOT, MSG_ID_IDLE, NULL, downstream_reboot_parser, downstream_reboot_handler);

