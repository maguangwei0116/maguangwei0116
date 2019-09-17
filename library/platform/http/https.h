#ifndef __HTTPS_H__
#define __HTTPS_H__

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include "rt_type.h"

/**************************************************************************************************
                                    Configure TLS
**************************************************************************************************/
//#define TLS_VERIFY_CERT                         1
//#define TLS_VERIFY_CERT_9_AS_OK                 1
#define TLS_CERT_PATH                           "/data/redtea/ca-chain.pem"

typedef enum ERR_HTTPS_CODE {
    RT_ERR_HTTPS_GET_HOSTNAME_FAIL = -300,
    RT_ERR_HTTPS_GET_SOCKET_FAIL = -301,
    RT_ERR_HTTPS_CONNECT_SOCKET_FAIL = -302,
    RT_ERR_HTTPS_NEW_SSL_CTX_FAIL = -303,
    RT_ERR_HTTPS_CLIENT_CRT_NOT_FOUND = -304,
    RT_ERR_HTTPS_SERVER_CRT_NOT_FOUND = -305,
    RT_ERR_HTTPS_SERVER_CRT_VERIFY_FAIL = -306,
    RT_ERR_HTTPS_CREATE_SSL_FAIL = -307,
    RT_ERR_HTTPS_CONNECT_SSL_FAIL = -308,
    RT_ERR_HTTPS_SSL_HANDSHAKE_FAIL = -309,
    RT_ERR_HTTPS_POST_FAIL = -310,
    RT_ERR_HTTPS_SMDP_ERROR = -311
} err_https_code_e;

typedef struct https_context {
    int socket;
    SSL *ssl;
    SSL_CTX *ssl_cxt;
} https_ctx_t;

int https_init(https_ctx_t *https_ctx, const char *host, const char *port, const char *ca);
int https_post(https_ctx_t *https_ctx, const char *request);
int https_read(https_ctx_t *https_ctx, char *buffer, int buffer_size);
void https_free(https_ctx_t *https_ctx);

int lpa_https_post(const char *addr, const char *api, const char *body,
                    char *buffer, int *size /* out */);
void lpa_https_close_socket(void);

#endif  // __HTTPS_H__
