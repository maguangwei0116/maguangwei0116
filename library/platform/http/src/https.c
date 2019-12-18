
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
#include "dns.h"

#define TCP_CONNECT_TIMEOUT     30  // unit: seconds

// Establish a regular tcp connection
static int connect_tcp(const char *host_name, const char *addr)
{
    int error, handle;
    struct hostent *host;
    struct sockaddr_in server;
    
#ifdef CFG_USR_DNS_API
    MSG_PRINTF(LOG_INFO, "Get hostname by rt api ...\r\n");
    host = rt_gethostbyname(host_name);
#else
    host = gethostbyname(host_name);
#endif
    if (!host) {
        MSG_PRINTF(LOG_ERR, "Get hostname failed\n");
        return RT_ERR_HTTPS_GET_HOSTNAME_FAIL;
    }

    handle = socket(AF_INET, SOCK_STREAM, 0);
    if (handle < 0) {
        MSG_PRINTF(LOG_ERR, "Get socket failed\n");
        return RT_ERR_HTTPS_GET_SOCKET_FAIL;
    } else {
        struct timeval timeout;
    
        server.sin_family = AF_INET;
        server.sin_port = htons(atoi(addr));
        server.sin_addr = *((struct in_addr *)host->h_addr);
        bzero(&(server.sin_zero), 8);        
        timeout.tv_sec = TCP_CONNECT_TIMEOUT;
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
        if (error < 0) {
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
    MSG_PRINTF(LOG_INFO, "request[%d]:\n%s\n\n\n", (int)strlen(request), request);
    return SSL_write(https_ctx->ssl, request, strlen(request));
}

int https_read(https_ctx_t *https_ctx, char *buffer, int buffer_size)
{
    return SSL_read(https_ctx->ssl, buffer, buffer_size);
}

void https_free(https_ctx_t *https_ctx)
{
    /* release ssl handle */
    if (https_ctx->ssl) {
        SSL_shutdown(https_ctx->ssl);
        SSL_free(https_ctx->ssl);
        https_ctx->ssl = NULL;
    }

    /* release socket fd */
    if (https_ctx->socket > 0) {
        close(https_ctx->socket);
        https_ctx->socket = -1;
    }

    /* release ssl CTX */
    if (https_ctx->ssl_cxt) {
        SSL_CTX_free(https_ctx->ssl_cxt);
        https_ctx->ssl_cxt = NULL;
    }
}

