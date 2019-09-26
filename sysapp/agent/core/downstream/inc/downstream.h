
#ifndef __DOWNSTREAM_H__
#define __DOWNSTREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rt_type.h"
#include "cJSON.h"
#include "agent_queue.h"
#include "upload.h"


typedef int32_t (*parser_func)(const void *in, char *tranId, void **out);
typedef int32_t (*handler_func)(const void *in, const char *event, void **out);

typedef struct DOWNSTREAM_METHOD {
    const char *    method;
    const char *    event;
    msg_id_e        msg_id;
    parser_func     parser;
    handler_func    handler;
} downstream_method_t;

typedef struct DOWNSTREAM_MSG {
    char *          msg;
    uint32_t        msg_len;
    const char *    method;
    const char *    event;
    parser_func     parser;
    handler_func    handler;
    void *          private_arg;
    void *          out_arg;
    char            tranId[MAX_TRAN_ID_LEN + 1];
} downstream_msg_t;

#define DOWNSTREAM_METHOD_OBJ_INIT(method, msg_id, event, parser, handler)\
    static const downstream_method_t downstream_method_##method##_obj \
    __attribute__((section(".downstream.method.init.obj"))) = \
    {#method, #event, msg_id, parser, handler}

#define DOWNSTREAM_METHOD_OBJ_EXTERN(method) \
    const downstream_method_t * g_downstream_method_##method = (const downstream_method_t * )&downstream_method_##method##_obj

#define DOWNSTREAM_METHOD_OBJ_EXTERN_HERE(event) \
    extern const downstream_method_t * g_downstream_method_##event

DOWNSTREAM_METHOD_OBJ_EXTERN_HERE(START);
DOWNSTREAM_METHOD_OBJ_EXTERN_HERE(END);

int32_t downstream_msg_handle(const void *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __DOWNSTREAM_H__*/

