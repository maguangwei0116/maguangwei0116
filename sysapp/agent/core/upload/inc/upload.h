
#ifndef __NEW_UPLOAD_H__
#define __NEW_UPLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http.h"

int32_t upload_http_post(const char *host_addr, int32_t port, socket_call_back cb, void *buffer, int32_t len);
int32_t init_upload(void *arg);

#include "rt_type.h"
#include "cJSON.h"

typedef cJSON *(*packer_func)(const char *tran_id, int32_t status, const cJSON *content);

typedef struct _upload_cmd_t {
    const char *    cmd;
    packer_func     packer;
} upload_cmd_t;

#define UPLOAD_CMD_OBJ_INIT(cmd, packer)\
    static const upload_cmd_t upload_cmd_##cmd##_obj \
    __attribute__((section(".upload.cmd.init.obj"))) = \
    {#cmd, packer}

#define UPLOAD_CMD_OBJ_EXTERN(cmd) \
    do { \
        return &upload_cmd_##cmd##_obj; \
    } while(0)

const upload_cmd_t *upload_cmd_get_start(void);
const upload_cmd_t *upload_cmd_get_end(void);

int32_t upload_msg_pack(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* __NEW_UPLOAD_H__*/

