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

#include "log.h"
#include "https.h"
#include "dns.h"
#if (CFG_UPLOAD_HTTPS_ENABLE)
    #include "http.h"
    typedef int32_t (*PCALLBACK)(const char *json_data);
#endif

#define TCP_CONNECT_TIMEOUT     30  // unit: seconds

// Establish a regular tcp connection
static int connect_tcp(const char *host_name, const char *addr)
{
    int error, handle;
    struct hostent *host;
    struct sockaddr_in server;
    int i = 0;

    host = gethostbyname(host_name);
#ifdef CFG_USR_DNS_API
    if (!host) {
        MSG_PRINTF(LOG_WARN, "Native get hostname fail, try to use rt api ...\r\n");
        host = rt_gethostbyname(host_name);
    }
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

int https_init(https_ctx_t *https_ctx, const char *host, const char *port, const char *ca, int is_tls)
{
    int res = -1;
    int count = 0;

    rt_os_memset(https_ctx, 0, sizeof(https_ctx_t));

    https_ctx->socket = connect_tcp(host, port);
    if (https_ctx->socket < 0) {
        return https_ctx->socket;
    }

    // Register the error strings for libcrypto & libssl
    SSL_load_error_strings();

    // Register the available ciphers and digests
    SSL_library_init();
    OpenSSL_add_all_algorithms();

    if (0 == is_tls) {
        // New context saying we are a client, and using SSL 2 or 3
        https_ctx->ssl_cxt = SSL_CTX_new(SSLv23_client_method());
    } else {
        https_ctx->ssl_cxt = SSL_CTX_new(TLSv1_2_client_method());
    }

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
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        MSG_PRINTF(LOG_DBG, "Subject Name: %s\n", line);
        rt_os_free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        MSG_PRINTF(LOG_DBG, "Issuer: %s\n", line);
        res = SSL_get_verify_result(https_ctx->ssl);
        if (res != X509_V_OK) {
            MSG_PRINTF(LOG_WARN, "Certificate verification res: %d\n", res);
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
    // MSG_PRINTF(LOG_TRACE, "request[%d]:\n%s\n\n\n", (int)rt_os_strlen(request), request);
    return SSL_write(https_ctx->ssl, request, rt_os_strlen(request));
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

#if (CFG_UPLOAD_HTTPS_ENABLE)

    static int new_system(char *cmd)
    {
        return 0;
        #if 0
        int status = system(cmd);
        MSG_PRINTF(LOG_TRACE, "cmd=%s\r\n", cmd);
        if (-1 == status) {
            MSG_PRINTF(LOG_WARN, "system error!\r\n");
        } else {
            MSG_PRINTF(LOG_DBG, "exit status value = [0x%x]\n", status);
            if (WIFEXITED(status)) {
                if (0 == WEXITSTATUS(status)) {
                    MSG_PRINTF(LOG_INFO, "run shell script successfully.\n");
                    return 0;
                } else {
                    MSG_PRINTF(LOG_WARN, "run shell script fail, script exit code: %d\n", WEXITSTATUS(status));
                }
            } else {
                MSG_PRINTF(LOG_WARN, "exit status = [%d]\n", WEXITSTATUS(status));
            }
        }

        return -1;
        #endif
    }

    const char *strtoken(const char *src, char *dst, int size)
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

        rt_os_strncpy(dst, start, len);
        dst[len]=0;

        return p;
    }

    // Return body start pointer
    const char *process_header(const char *rsp, int *status, int *content_length)
    {
        const char *p, *token, *left;
        int len;
        char key[19];
        char value[8];

        *status = 0;
        *content_length = 0;
        MSG_PRINTF(LOG_TRACE, "rsp:%s\n",rsp);
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

    int upload_https_post(const char *addr, const char *api, const char *body, char *buffer, int *size /* out */)
    {
        const char *p;
        int i,k=0;
        int len = 0;
        int status = 0;
        char port[6] = {0};
        char host[64] = {0};
        char api_t[100] = {0};
        int done_size, left_size;
        char request[2048] = {0};
        https_ctx_t https_ctx = {-1, NULL, NULL};
        char md5_out[32 + 1] = {0};
        int post_res = -1;
        int ping_count = 0;

        MSG_PRINTF(LOG_TRACE, "addr:%s, api:%s\n", addr, api);
        p = strstr(addr, ":");
        if (p == NULL) {
            // Use default port
            rt_os_strncpy(port, "443", 4);
            rt_os_strncpy(host, addr, 63);
        } else {
            rt_os_strncpy(port, p+1, 5);
            // TODO: fix potential bug: p - addr > 63
            rt_os_strncpy(host, addr, p - addr);
        }

        if (https_ctx.ssl == NULL) {
            MSG_PRINTF(LOG_TRACE, "tls https_init\n");
#if (CFG_UPLOAD_HTTPS_ENABLE)
        // dns function High Probability fail, so add this codes
        #if 0
        while (1) {
            if (0 == new_system(PING_ADDR)) {
                break;
            }
            if (ping_count > 5) {
                break;
            }
            sleep(2);
            MSG_PRINTF(LOG_WARN, "ping_count is %d ...\r\n", ping_count);
            ping_count++;
        }
        #endif
#endif
            status = https_init(&https_ctx, host, port, "./ca-chain.pem", 1); // "./ca-chain.pem" unuse
            if (status < 0) {
                https_free(&https_ctx);
                MSG_PRINTF(LOG_ERR, "https_init is status %d ...\r\n", status);
                return HTTP_SOCKET_CONNECT_ERROR;
            }
        }
        get_md5_string((int8_t *)body, md5_out);
        snprintf(request, sizeof(request),
                "POST %s HTTP/1.1\r\n"
                "Host: %s:%s\r\n"
                "Accept: */*\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "md5sum:%s\r\n"
                "Content-Length: %d\r\n"
                "\r\n",
                api,
                host, port,md5_out,
                (int)strlen(body));

        done_size = strlen(request);
        left_size = strlen(body);

        snprintf(request + done_size, sizeof(request) - done_size, "%s", body);
        post_res = https_post(&https_ctx, (const char *)request);
        if (post_res < 0) {
            MSG_PRINTF(LOG_ERR, "https_post is post_res %d ...\r\n", post_res);
            return HTTP_SOCKET_SEND_ERROR;
        }

        done_size = sizeof(request) - 1 - done_size;
        left_size -= done_size;

        while (left_size > 0) {
            memset(request, 0, sizeof(request));
            snprintf(request, sizeof(request), "%s", body + done_size);
            post_res = https_post(&https_ctx, (const char *)request);
            if (post_res < 0) {
                MSG_PRINTF(LOG_ERR, "https_post is post_res %d ...\r\n", post_res);
                return HTTP_SOCKET_SEND_ERROR;
            }
            done_size += sizeof(request) - 1;
            left_size -= sizeof(request) - 1;
        }

        memset(buffer, 0, *size);
        *size = https_read(&https_ctx, buffer, *size);
        if (*size == 0 || *size == -1) {
            MSG_PRINTF(LOG_ERR, "https read, *size=%d\r\n", *size);
            return HTTP_SOCKET_RECV_ERROR;
        }

        p = process_header(buffer, &status, &len);
        if (p == NULL) {
            MSG_PRINTF(LOG_ERR, "%s\n", buffer);
            return RT_ERR_HTTPS_SMDP_ERROR;
        }

        if (len == -1) {
            char sbuf[11] = {0}; // For content length, MAX to 0x7FFFFFF(2147483647)
            p = strtoken(p, sbuf, sizeof(sbuf));
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
        https_free(&https_ctx);
        return status;
    }

    int https_post_raw(const char *addr, const char *api, const char *body, char *buffer, int *size , socket_call_back cb)
    {
        char out_buffer[5120] = {0};
        int out_size = 5120;
        int ret = 0;
        MSG_PRINTF(LOG_TRACE, "addr is %s\n", addr);
        MSG_PRINTF(LOG_TRACE, "api is %s\n", api);
        MSG_PRINTF(LOG_TRACE, "body is %s\n", body);
        ret = upload_https_post(addr, api, body, out_buffer, &out_size);
        if (200 == ret) {
            cb(out_buffer);
            MSG_PRINTF(LOG_DBG, "out_buffer is %s\n", out_buffer);
            return 0;
        } else {
            MSG_PRINTF(LOG_WARN, "upload_https_post return is %d the buffer is %s\n", ret, out_buffer);
            return ret;
        }
    }


    int32_t mqtt_https_post_json(const char *json_data, const char *host_ip, uint16_t port, const char *path, PCALLBACK cb)
    {
        int32_t ret = RT_ERROR;
        int32_t socket_fd = RT_ERROR;
        char md5_out[32+1];
        char *p = NULL;
        char out_buffer[1024] = {0};
        int out_size = 1024;

        do{
            if(!json_data || !host_ip || !path) {
                MSG_PRINTF(LOG_WARN, "path json data error\n");
                break;
            }

            MSG_PRINTF(LOG_DBG, "json_data:%s\n",json_data);
            get_md5_string((const char *)json_data, md5_out);
            md5_out[32] = '\0';

            MSG_PRINTF(LOG_DBG, "host_ip:%s, port:%d\r\n", host_ip, port);

            ret = upload_https_post(host_ip, path, json_data, out_buffer, &out_size);
            MSG_PRINTF(LOG_DBG, "out_buffer is %s\n", out_buffer);
            if (200 == ret) {
                cb(out_buffer);
                MSG_PRINTF(LOG_DBG, "out_buffer is %s\n", out_buffer);
            } else {
                MSG_PRINTF(LOG_WARN, "upload_https_post return is %d the buffer is %s\n", ret, out_buffer);
            }

            /* get http body */
            MSG_PRINTF(LOG_DBG, "recv buff: %s\n", out_buffer);

            ret = cb(out_buffer);
        } while(0);

        return ret;
    }

#endif
