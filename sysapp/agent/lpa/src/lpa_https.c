
#include "rt_type.h"
#include "https.h"
#include "lpa_https.h"

/**************************************************************************************************
                                    Configure TLS
**************************************************************************************************/
//#define TLS_VERIFY_CERT               1
//#define TLS_VERIFY_CERT_9_AS_OK       1
#define TLS_CERT_PATH                   "./ca-chain.pem"  /* TODO: NO implementation */
#define CHUNKED                         -1
#define SEND_BUFFER_SIZE                1024
#define PREFERRED_CIPHERS               "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS"

static https_ctx_t g_https_ctx          = {-1, NULL, NULL};
static const char *g_proxy_server_url   = NULL;

/**
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

    rt_os_strncpy(dst, start, len);
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

int lpa_https_set_url(const char *server_url)
{
    g_proxy_server_url  = server_url;

    return RT_SUCCESS;
}

int lpa_https_post(const char *addr, const char *api, const char *body, char *buffer, int *size /* out */)
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
        strcat(api_t, addr);
        strcat(api_t, api);
        api = api_t;
        addr = g_proxy_server_url;
    }

    MSG_PRINTF(LOG_INFO, "addr:%s, api:%s\n", addr, api);
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

    if (g_https_ctx.ssl == NULL) {
        MSG_PRINTF(LOG_INFO, "https_init\n");
        status = https_init(&g_https_ctx, host, port, TLS_CERT_PATH, 0);
        if (status < 0) {
            https_free(&g_https_ctx);
            return status;
        }
    }

    snprintf(request, sizeof(request),
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

    snprintf(request + done_size, sizeof(request) - done_size, "%s", body);
    https_post(&g_https_ctx, (const char *)request);

    done_size = sizeof(request) - 1 - done_size;
    left_size -= done_size;

    while (left_size > 0) {
        memset(request, 0, sizeof(request));
        snprintf(request, sizeof(request), "%s", body + done_size);
        https_post(&g_https_ctx, (const char *)request);
        done_size += sizeof(request) - 1;
        left_size -= sizeof(request) - 1;
    }

    memset(buffer, 0, *size);
    *size = https_read(&g_https_ctx, buffer, *size);    
    if (*size == 0 || *size == -1) {
        MSG_PRINTF(LOG_ERR, "https read, *size=%d\r\n", *size);
        return RT_ERR_HTTPS_POST_FAIL;
    }

    p = process_header(buffer, &status, &len);
    if (p == NULL) {
        MSG_PRINTF(LOG_ERR, "%s\n", buffer);
        return RT_ERR_HTTPS_SMDP_ERROR;
    }

    if (len == CHUNKED) {
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

    return status;
}

int lpa_https_close(void)
{
    https_free(&g_https_ctx);

    return RT_SUCCESS;
}