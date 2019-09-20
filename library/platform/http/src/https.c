
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/err.h>
#include "https.h"

#define CHUNKED             -1
#define SEND_BUFFER_SIZE    1024
#define PREFERRED_CIPHERS   "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS"

static https_ctx_t g_https_ctx = {-1,NULL,NULL};
uint8_t *g_proxy_server_url = NULL;
/*
Get a piece of string from a line wihtout white space
eg:
char *p;
char dst[16];
src = "HTTP/1.1 200 ";
p = strtoken(src, dst, 16);
p would be src + 9
dst would be HTTP/1.1
*/
static const char *strtoken(const char *src, char *dst, int size)
{
    const char *p, *start, *end;
    int  len = 0;

    // Trim left
    p = src;
    while (1) {
        // value is not exists
        if ((*p == '\n') || (*p == 0)) {
            return NULL;
        }
        // Get the first non-whitespace character
        if ((*p != ' ') && (*p != '\t')) {
            break;
        }
        // Skip white spaces
        p++;
    }

    start = p;
    while (1) {
        end = p;
        // Meet blank or tab, end
        if (*p == ' ') {
            p++;    // Skip it
            break;
        }
        // Meet return or line end
        if ((*p == '\n') || (*p == 0)) {
            break;
        }
        // Search forward
        p++;
    }

    // TODO: remove
    // Trim right
    while (1) {
        end--;
        if (start == end) {
            break;
        }

        if ((*end != ' ') && (*end != '\t')) {
            break;
        }
    }

    len = (int)(end - start + 1);
    if ((size > 0) && (len >= size)) {
        len = size - 1;
    }

    strncpy(dst, start, len);
    dst[len]=0;

    return p;
}

// Return body start pointer
static const char *process_header(const char *rsp, int *status, int *content_length)
{
    const char *p, *token, *left;
    int len;
    char key[19];
    char value[8];

    *status = 0;
    *content_length = 0;
    MSG_PRINTF(LOG_INFO, "rsp:%s\n",rsp);
    p = rsp;
    do {
        left = strstr(p, "\r\n");
        if (left == NULL) {
            return NULL;
        }

        len = left - p;
        if (len == 0) {
            return (p + 2);    // Skip CR+LF
        }

        token = strtoken(p, key, 19);
        if (token == NULL) {
            return NULL;  // TODO: define return code
        }

        token = strtoken(token, value, 8);
        if (p == NULL) {
            return NULL;  // TODO: define return code
        }

        // Take care about status code and body size
        if (strncasecmp(key, "HTTP", 4) == 0) {
            *status = atoi(value);
        } else if (strncasecmp(key, "content-length:", 15) == 0) {
            *content_length = atoi(value);
        } else if (strncasecmp(key, "transfer-encoding:", 18) == 0) {
            if (strncasecmp(value, "chunked", 7) == 0) {
                *content_length = -1;
            }
        }

        // Don't care about others fields
        p = left + 2;
    } while (*p != '\0');

    return NULL;  // TODO: define return code
}

// Establish a regular tcp connection
int connect_tcp(const char *host_name, const char *addr)
{
    int error, handle;
    struct hostent *host;
    struct sockaddr_in server;

    host = gethostbyname(host_name);
    if (host == NULL) {
        MSG_PRINTF(LOG_ERR, "Get hostname failed\n");
        return RT_ERR_HTTPS_GET_HOSTNAME_FAIL;
    }

    handle = socket(AF_INET, SOCK_STREAM, 0);
    if (handle == -1) {
        MSG_PRINTF(LOG_ERR, "Get socket failed\n");
        return RT_ERR_HTTPS_GET_SOCKET_FAIL;
    } else {
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(addr));
        server.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server.sin_zero), 8);
        struct timeval timeout;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
            MSG_PRINTF(LOG_ERR, "setsockopt0 error\n");
            return RT_ERR_HTTPS_CONNECT_SOCKET_FAIL;
        }

        if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
            MSG_PRINTF(LOG_ERR, "setsockopt error\n");
            return RT_ERR_HTTPS_CONNECT_SOCKET_FAIL;
        }
        error = connect(handle, (struct sockaddr *)&server, sizeof(server));
        if (error == -1) {
            MSG_PRINTF(LOG_ERR, "Connect TCP failed\n");
            return RT_ERR_HTTPS_CONNECT_SOCKET_FAIL;
        }
    }

    return handle;
}

int https_init(https_ctx_t *https_ctx, const char *host, const char *port, const char *ca)
{
    int res = -1;
    memset(https_ctx, 0, sizeof(https_ctx_t));

    https_ctx->socket = connect_tcp(host, port);
    if (https_ctx->socket < 0) {
        return https_ctx->socket;
    }

    // Register the error strings for libcrypto & libssl
    SSL_load_error_strings();

    // Register the available ciphers and digests
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    // New context saying we are a client, and using SSL 2 or 3
    https_ctx->ssl_cxt = SSL_CTX_new(SSLv23_client_method());
    if (https_ctx->ssl_cxt == NULL) {
        ERR_print_errors_fp(stderr);
        return RT_ERR_HTTPS_NEW_SSL_CTX_FAIL;
    }

    SSL_CTX_set_verify(https_ctx->ssl_cxt, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_verify_depth(https_ctx->ssl_cxt, 4);
    res = SSL_CTX_load_verify_locations(https_ctx->ssl_cxt, ca, NULL);
    if (res < 0) {
        MSG_PRINTF(LOG_ERR, "No Certificate found on Client!\n");
#ifdef TLS_VERIFY_CERT
        return RT_ERR_HTTPS_CLIENT_CRT_NOT_FOUND;
#endif
    }

    // Create an SSL struct for the connection
    https_ctx->ssl = SSL_new(https_ctx->ssl_cxt);
    if (https_ctx->ssl == NULL) {
        ERR_print_errors_fp(stderr);
        return RT_ERR_HTTPS_CREATE_SSL_FAIL;
    }

    // Connect the SSL struct to our connection
    if (!SSL_set_fd(https_ctx->ssl, https_ctx->socket)) {
        ERR_print_errors_fp(stderr);
        return RT_ERR_HTTPS_CONNECT_SSL_FAIL;
    }

    // Initiate SSL handshake
    if (SSL_connect(https_ctx->ssl) != 1) {
        ERR_print_errors_fp(stderr);
        return RT_ERR_HTTPS_SSL_HANDSHAKE_FAIL;
    }

    X509* cert = SSL_get_peer_certificate(https_ctx->ssl);
    if (cert) {  // Free immediately
        char *line;
        //MSG_PRINTF(LOG_INFO, "cert: %s\n", cert);
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        MSG_PRINTF(LOG_INFO, "Subject Name: %s\n", line);
        rt_os_free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        MSG_PRINTF(LOG_INFO, "Issuer: %s\n", line);
        res = SSL_get_verify_result(https_ctx->ssl);
        if (res != X509_V_OK) {
            MSG_PRINTF(LOG_ERR, "Certificate verification failed: %d\n", res);
#ifdef TLS_VERIFY_CERT
#ifdef TLS_VERIFY_CERT_9_AS_OK
            if (res == X509_V_ERR_CERT_NOT_YET_VALID) {
                // Do nothing, consider X509_V_ERR_CERT_NOT_YET_VALID as verification passed
            } else
#endif
            {
                return RT_ERR_HTTPS_SERVER_CRT_VERIFY_FAIL;
            }
#endif
        }
        rt_os_free(line);
        X509_free(cert);
    } else {
        MSG_PRINTF(LOG_ERR, "No Certificate found on Server!\n");
#ifdef TLS_VERIFY_CERT
            return RT_ERR_HTTPS_SERVER_CRT_NOT_FOUND;
#endif
    }

    return RT_SUCCESS;
}

int https_post(https_ctx_t *https_ctx, const char *request)
{
    MSG_PRINTF(LOG_DBG, "request[%d]:\n%s\n\n\n", (int)strlen(request), request);
    return SSL_write(https_ctx->ssl, request, strlen(request));
}

int https_read(https_ctx_t *https_ctx, char *buffer, int buffer_size)
{
    return SSL_read(https_ctx->ssl, buffer, buffer_size);
}

void https_free(https_ctx_t *https_ctx)
{
    if (https_ctx->socket > 0) {
        close(https_ctx->socket);
    }

    if (https_ctx->ssl) {
        SSL_shutdown(https_ctx->ssl);
        SSL_free(https_ctx->ssl);
    }

    if (https_ctx->ssl_cxt) {
        SSL_CTX_free(https_ctx->ssl_cxt);
    }
}

int lpa_https_post(const char *addr, const char *api, const char *body,
                    char *buffer, int *size /* out */)
{
    const char *p;
    int i,k=0;
    int len = 0;
    int status = 0;
    char port[6] = {0};
    char host[64] = {0};
    char api_t[100] = {0};
    int done_size, left_size;
    char request[SEND_BUFFER_SIZE] = {0};

    if (g_proxy_server_url != NULL) {       //add proxy server
        api_t[0] = '/';
        strcat(api_t,addr);
        strcat(api_t ,api);
        api = api_t;
        addr = g_proxy_server_url;
    }

    MSG_PRINTF(LOG_INFO, "addr:%s\n", addr);
    MSG_PRINTF(LOG_INFO, "api:%s\n", api);
    p = strstr(addr, ":");
    if (p == NULL) {
        // Use default port
        strncpy(port, "443", 4);
        strncpy(host, addr, 63);
    } else {
        strncpy(port, p+1, 5);
        // TODO: fix potential bug: p - addr > 63
        strncpy(host, addr, p - addr);
    }

    if (g_https_ctx.ssl == NULL) {
        MSG_PRINTF(LOG_INFO, "https_init\n");
        status = https_init(&g_https_ctx, host, port, TLS_CERT_PATH);
        if (status < 0) {
            https_free(&g_https_ctx);
            return status;
        }
    }

    snprintf(request, SEND_BUFFER_SIZE,
            "POST %s HTTP/1.1\r\n"
            "User-Agent: gsma-rsp-lpad\r\n"
            "X-Admin-Protocol: gsma/rsp/v2.0.0\r\n"
            "Host: %s:%s\r\n"
            "Connection: Keep-Alive\r\n"
            "Accept: */*\r\n"
            "Content-Type: application/json; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "\r\n",
            api,
            host, port,
            (int)strlen(body));

    done_size = strlen(request);
    left_size = strlen(body);

    snprintf(request + done_size, SEND_BUFFER_SIZE - done_size, "%s", body);
    SSL_write(g_https_ctx.ssl, request, strlen(request));
    MSG_PRINTF(LOG_INFO, "sent[%d]:\n%s\n", (int)strlen(request), request);

    done_size = SEND_BUFFER_SIZE - 1 - done_size;
    left_size -= done_size;

    while (left_size > 0) {
        memset(request, 0, SEND_BUFFER_SIZE);
        snprintf(request, SEND_BUFFER_SIZE, "%s", body + done_size);
        SSL_write(g_https_ctx.ssl, request, strlen(request));
        MSG_PRINTF(LOG_INFO, "sent[%d]:\n%s\n", (int)strlen(request), request);
        done_size += SEND_BUFFER_SIZE - 1;
        left_size -= SEND_BUFFER_SIZE - 1;
    }

    *size = SSL_read(g_https_ctx.ssl, buffer, *size);
    if (*size == 0) {
        return RT_ERR_HTTPS_POST_FAIL;
    }

    p = process_header(buffer, &status, &len);
    if (p == NULL) {
        MSG_PRINTF(LOG_ERR, "%s\n", buffer);
        return RT_ERR_HTTPS_SMDP_ERROR;
    }

    if (len == CHUNKED) {
        char sbuf[11] = {0}; // For content length, MAX to 0x7FFFFFF(2147483647)
        p = strtoken(p, sbuf, 11);
        p += 1;             // Skip '\n'
        *size = strtol(sbuf, NULL, 16);
    } else {
        *size = len;
    }
    for (i=0; i<*size; i++) {
        if (p[i] == '\n' || p[i] == '\r') {
            k++;
        }
        buffer[i] = p[i+k];
    }
    buffer[*size-k] = '\0';

    return status;
}

void lpa_https_close_socket(void)
{
    if(g_https_ctx.socket<0){
        return;
    }
    close(g_https_ctx.socket);
    g_https_ctx.ssl = NULL;
}


