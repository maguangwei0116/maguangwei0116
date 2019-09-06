
#ifndef __NEW_UPLOAD_H__
#define __NEW_UPLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "http.h"

int32_t upload_http_post(const char *host_addr, int32_t port, socket_call_back cb, void *buffer, int32_t len);
int32_t init_upload(void *arg);
int32_t upload_cmd_no_cert(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* __NEW_UPLOAD_H__*/

