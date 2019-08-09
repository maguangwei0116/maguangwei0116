#ifndef __HTTPS_H__
#define __HTTPS_H__

#include <openssl/ssl.h>
#include <openssl/bio.h>

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
