
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "md5.h"
#include "cJSON.h"

static int32_t downstream_reset_parser(const void *in, char *tranId, void **out)
{
    int32_t ret = RT_ERROR;
    cJSON *msg = NULL;
    cJSON *tran_id = NULL;
    static int8_t md5_out_pro[MD5_STRING_LENGTH + 1];
    int8_t md5_out_now[MD5_STRING_LENGTH + 1];

    get_md5_string((int8_t *)in, md5_out_now);
    md5_out_now[MD5_STRING_LENGTH] = '\0';
    if (rt_os_strcmp(md5_out_pro, md5_out_now) == 0) {
        MSG_PRINTF(LOG_ERR, "The data are the same!!\n");
        return ret;
    }
    rt_os_strcpy(md5_out_pro, md5_out_now);

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

static int32_t downstream_reset_handler(const void *in, const char *event, void **out)
{
    int32_t ret = 0;

    (void)in;
    (void)event;
    *out = NULL;
    
    MSG_PRINTF(LOG_WARN, "wait 3 seconds to reset terminal ...\n");
    rt_os_sleep(3);
    rt_os_reboot();  // reset terminal

exit_entry:

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(RESET, MSG_ID_IDLE, NULL, downstream_reset_parser, downstream_reset_handler);

