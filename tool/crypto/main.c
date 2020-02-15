
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "crypto.h"
#include "base64.h"

extern const char curve_parameter[192];
static const char *opt_string = "glll:s:?:v:h:i:s:p:f:b:t:";
volatile int32_t toStop = 0;

static void display_usage(void)
{
    fprintf(stderr, "This is crypto tool.\n");
    fprintf(stderr, "Usage: crypto option [arguments]\n");
    fprintf(stderr, "  -g\tgenerate key pair\n");
    fprintf(stderr, "  -h\tfile hash\n");
    fprintf(stderr, "  -i\thash signature\n");
    fprintf(stderr, "  -p\tpublic key\n");
    fprintf(stderr, "  -s\tsecuret key\n");

    fprintf(stderr, "  -f\thash signature base64 format\n");
    fprintf(stderr, "  -b\tpublic key base64 format\n");
}

int softsim_printf (int leve, int flag, const char *msg, ...)
{
    char content[1024] = {0};
    uint32_t len = 0;
    va_list vl_list;

    va_start(vl_list, msg);
    vsnprintf((char *)&content[len], sizeof(content) - len, (const char *)msg, vl_list);
    va_end(vl_list);

    printf(content, strlen(content));

    return 0;
}


int main(int argc, char * const *argv)
{
    int opt = 0;
    uint8_t *hash = NULL;
    uint8_t *signature = NULL;
    uint8_t *sk_key = NULL;
    uint8_t *pk_key = NULL;
    uint8_t *input_text = NULL;
    uint8_t pk[64];
    uint8_t sk[64];
    int pk_len = 0;
    int sk_len = 0;
    int s_len = 0;
    int h_len = 0;
    uint8_t output[64];
    uint8_t base64_out[512];
    int output_len = 0;
    int input_len = 0;

    init_curve_parameter(curve_parameter);

    if (argc < 2) {
        display_usage();
        return -1;
    }
    opt = getopt(argc, argv, opt_string);
    while (opt != -1) {
        switch (opt) {
            case 'g':
                ecc_generate_key(pk, &pk_len, sk, &sk_len);
                LOG_INFO_ARRAY(sk, sk_len, "sk:");
                LOG_INFO_ARRAY(pk, pk_len, "pk:");
                break;
            case 'e':
                break;
            case 'h':
                hash = (uint8_t *)optarg;
                h_len = (int)strlen((char *)hash);
                printf("hash:%s, len:%d\n", hash, h_len);
                break;
            case 'i':
                signature = (uint8_t *)optarg;
                s_len = (int)strlen((char *)signature);
                printf("signature:%s, len:%d\n", signature, s_len);
                break;
            case 's':
                sk_key = (uint8_t *)optarg;
                sk_len = (int)strlen((char *)sk_key);
                printf("sk:%s, len:%d\n", sk_key, sk_len);
                break;
            case 'p':
                pk_key = (uint8_t *)optarg;
                pk_len = (int)strlen((char *)pk_key);
                printf("pk:%s, len:%d\n", pk_key, pk_len);
                break;

            case 'f':
                signature = (uint8_t *)optarg;
                rt_base64_decode(signature, base64_out, &s_len);
                bytes2hexstring(base64_out, s_len, signature);
                s_len = (int)strlen((char *)signature);
                printf("signature:%s, len:%d\n", signature, s_len);
                break;
            case 'b':
                pk_key = (uint8_t *)optarg;
                rt_base64_decode(pk_key, base64_out, &pk_len);
                bytes2hexstring(base64_out, pk_len, pk_key);
                pk_len = (int)strlen((char *)pk_key);
                printf("pk:%s, len:%d\n", pk_key, pk_len);
                break;
            case 't':
                input_text = (uint8_t *)optarg;
                input_len = (int)strlen((char *)input_text);
                printf("string:%s, len:%d\n", input_text, input_len);
                break;
            case '?':
                display_usage();
                break;

            default:
                break;
        }
        opt = getopt(argc, argv, opt_string);
    }
    if ((hash != NULL) && (signature != NULL) && (pk_key != NULL)) {
        ecc_verify_signature( hash, h_len, pk_key, pk_len, signature, s_len);
    } else if ((hash != NULL) && (signature == NULL) && (sk_key != NULL)) {
        ecc_sign_hash( hash, h_len, sk_key, sk_len, output, &output_len);
        LOG_INFO_ARRAY(output, output_len, "signature:");
    } else if ((input_text != NULL) && (signature != NULL) && (pk_key != NULL)) {
        ecc_verify(input_text, input_len, pk_key, pk_len, signature, s_len);
    } else if (pk_len == 0) {
        printf("Input parameter error!!\n");
    }

}
