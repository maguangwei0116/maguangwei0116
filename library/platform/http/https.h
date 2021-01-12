#ifndef __HTTPS_H__
#define __HTTPS_H__

#include <openssl/ssl.h>
#include <openssl/bio.h>

#include "rt_type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* switch for upload HTTPS */
// #define CFG_UPLOAD_HTTPS_ENABLE                1

/* error code for https handle */
typedef enum ERR_HTTPS_CODE {
    RT_ERR_HTTPS_GET_HOSTNAME_FAIL      = -300,
    RT_ERR_HTTPS_GET_SOCKET_FAIL        = -301,
    RT_ERR_HTTPS_CONNECT_SOCKET_FAIL    = -302,
    RT_ERR_HTTPS_NEW_SSL_CTX_FAIL       = -303,
    RT_ERR_HTTPS_CLIENT_CRT_NOT_FOUND   = -304,
    RT_ERR_HTTPS_SERVER_CRT_NOT_FOUND   = -305,
    RT_ERR_HTTPS_SERVER_CRT_VERIFY_FAIL = -306,
    RT_ERR_HTTPS_CREATE_SSL_FAIL        = -307,
    RT_ERR_HTTPS_CONNECT_SSL_FAIL       = -308,
    RT_ERR_HTTPS_SSL_HANDSHAKE_FAIL     = -309,
    RT_ERR_HTTPS_POST_FAIL              = -310,
    RT_ERR_HTTPS_SMDP_ERROR             = -311
} err_https_code_e;

typedef struct HTTPS_CXT {
    int                                 socket;     // tcp socket fd
    SSL *                               ssl;        // openssl handle
    SSL_CTX *                           ssl_cxt;    // openssl ssl CTX
} https_ctx_t;

extern int  https_init(https_ctx_t *https_ctx, const char *host, const char *port, const char *ca, int is_tls);
extern int  https_post(https_ctx_t *https_ctx, const char *request);
extern int  https_read(https_ctx_t *https_ctx, char *buffer, int buffer_size);
extern void https_free(https_ctx_t *https_ctx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __HTTPS_H__ */

