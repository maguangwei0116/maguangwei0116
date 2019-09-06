
#ifndef __NEW_UPLOAD_H__
#define __NEW_UPLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http.h"
#include "rt_type.h"
#include "cJSON.h"

#define NORMAL_TRAN_ID_LEN          32
#define MAX_TRAN_ID_LEN             64

typedef cJSON *(*packer_func)(void *arg);

typedef struct _upload_event_t {
    const char *    event;
    packer_func     packer;
} upload_event_t;

#define UPLOAD_EVENT_OBJ_INIT(event, packer)\
    static const upload_event_t upload_event_##event##_obj \
    __attribute__((section(".upload.event.init.obj"))) = \
    {#event, packer}

#define UPLOAD_EVENT_OBJ_EXTERN(event) \
    const upload_event_t * g_upload_event_##event = (const upload_event_t * )&upload_event_##event##_obj

#define UPLOAD_EVENT_OBJ_EXTERN_HERE(event) \
    extern const upload_event_t * g_upload_event_##event

UPLOAD_EVENT_OBJ_EXTERN_HERE(START);
UPLOAD_EVENT_OBJ_EXTERN_HERE(END);

int32_t upload_event_report(const char *event, const char *tran_id, int32_t status, void *private_arg);

int32_t upload_http_post(const char *host_addr, int32_t port, socket_call_back cb, void *buffer, int32_t len);

int32_t init_upload(void *arg);
int32_t upload_cmd_no_cert(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __NEW_UPLOAD_H__*/

