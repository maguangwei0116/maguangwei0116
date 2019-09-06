
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"
#include "agent_queue.h"

#include "cJSON.h"

#define DOWNSTREAM_METHOD_STR           "\"method\""
#define DOWNSTREAM_METHOD_STRLEN        8
#define DOWNSTREAM_METHOD_STR2          "\\\"method\\\""
#define DOWNSTREAM_METHOD_STRLEN2       10

static int32_t string_trim_single(const char *str_in, int32_t len_in, char *str_out, char ch)
{
    int32_t i = 0;
    int32_t j = len_in - 1;
    int32_t ret = 0;

    while(str_in[i] == ch)
        ++i;

    while(str_in[j] == ch)
        --j;
    
    strncpy(str_out, str_in + i , j - i + 1);
    str_out[j - i + 1] = '\0';

    ret = j - i + 1;

    return ret;
}

// delete ' '
static int32_t string_trim_space(const char *str_in, int32_t len_in, char *str_out)
{
    return string_trim_single(str_in, len_in, str_out, ' ');
}

// delete '\'
static int32_t string_trim_backslash(const char *str_in, int32_t len_in, char *str_out)
{
    return string_trim_single(str_in, len_in, str_out, '\\');
}

static int32_t downstream_msg_get_method(const char *msg, char *method)
{
    int32_t ret = -1;
    const char *p = NULL;
    const char *p0 = NULL;
    const char *p1 = NULL;
    int32_t method_str_len = 0;

    p = rt_os_strstr(msg, DOWNSTREAM_METHOD_STR);
    if (!p) {
        p = rt_os_strstr(msg, DOWNSTREAM_METHOD_STR2);
        if (!p) {        
            MSG_PRINTF(LOG_WARN, "get msg method failed 1\n");
            goto exit_entry;
        } else {
            method_str_len = DOWNSTREAM_METHOD_STRLEN2;
        }        
    } else {
        method_str_len = DOWNSTREAM_METHOD_STRLEN;
    }

    p += method_str_len;
    p0 = rt_os_strstr(p, "\"");
    if (!p0) {
        MSG_PRINTF(LOG_WARN, "get msg method failed 2\n");
        goto exit_entry;
    }

    p1 = rt_os_strstr(p0 + 1, "\"");
    if (!p1) {
        MSG_PRINTF(LOG_WARN, "get msg method failed 3\n");
        goto exit_entry;
    }

    ret = string_trim_space(p0 + 1, p1 - p0 - 1, method);
    string_trim_backslash(method, ret, method);

    //MSG_PRINTF(LOG_INFO, "get msg method : [%s]\n", method);
    MSG_PRINTF(LOG_WARN, "\n<-----------------%s\n", method);

    ret = 0;

exit_entry:

    return ret;
}

int32_t downstream_msg_handle(const void *data, uint32_t len)
{
    int32_t ret = -1;
    int32_t id;
    char method[64] = {0};
    const downstream_method_t *obj = NULL;
    const char *msg = (const char *)data;

    (void)len;
    ret = downstream_msg_get_method(msg, method);
    if (ret) {
        goto exit_entry;
    }

    for (obj = g_downstream_method_START; obj <= g_downstream_method_END; obj++) {
        MSG_PRINTF(LOG_WARN, "upload %p, %s, %d, %s ...\r\n", obj, obj->method, obj->msg_id, obj->event);
        if (!rt_os_strcmp(obj->method, method)) {
            downstream_msg_t downstream_msg = {0};

            downstream_msg.msg_len = rt_os_strlen(msg);
            downstream_msg.msg = (char *)rt_os_malloc(downstream_msg.msg_len + 1);
            rt_os_memcpy(downstream_msg.msg, msg, downstream_msg.msg_len);
            downstream_msg.msg[downstream_msg.msg_len] = '\0';
            downstream_msg.method       = obj->method;
            downstream_msg.event        = obj->event;
            downstream_msg.parser       = obj->parser;
            downstream_msg.handler      = obj->handler;
            downstream_msg.private_arg  = NULL;
            downstream_msg.out_arg      = NULL;
            MSG_PRINTF(LOG_WARN, "tranId: %p, msg: %p\r\n", downstream_msg.tranId, downstream_msg.msg);
            
            ret = msg_send_agent_queue(obj->msg_id, MSG_FROM_MQTT, &downstream_msg, sizeof(downstream_msg_t));
            
            return ret;
        }
    }

    MSG_PRINTF(LOG_WARN, "Unknow downstream msg method [%s] !!!\r\n", method);

exit_entry:

    return ret;
}

