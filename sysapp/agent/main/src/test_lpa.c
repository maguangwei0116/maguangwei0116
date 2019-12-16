
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "lpa.h"
#include "lpa_config.h"
#include "apdu.h"
#include "log.h"

#if (CFG_OPEN_MODULE)
#define RT_DATA_PATH            "/data/redtea/"
#elif (CFG_STANDARD_MODULE)  // standard
#define RT_DATA_PATH            "/usrdata/redtea/"
#endif

// #define ACTIVATION_CODE         "1$QUARK-QA.REDTEA.IO$TK3J73RBQ3K91NUP$$1"
// #define CONFIRMATION_CODE       "redtea_test"

#define ACTIVATION_CODE         "1$QUARK-QA.REDTEA.IO$WSTBFXLFN3$$1"
#define CONFIRMATION_CODE       "chucktest"
#define VERSION                 "V1.0.0"
#define MAX_PROFILE_NUM         20

// #define ACTIVATION_CODE         "1$esim.wo.cn$SATKB8Z-I4YBPHJS$1.3.6.1.4.1.47814.2.4$1"
// #define CONFIRMATION_CODE       "891985"

static const char *opt_string = "g:EqlLi:r:e:d:a:c:u:vh?";
static lpa_channel_type_e g_chan_mode = LPA_CHANNEL_BY_QMI;
static log_level_e g_log_level = LOG_INFO;

static void display_usage(void)
{
    fprintf(stderr, "This is C-LPA test tool.\n");
    fprintf(stderr, "Usage: test_lpa option [arguments]\n");
    fprintf(stderr, "  -g\tLog level: 1-[ERR], 2-[WARN], 3-[DBG], 4-[INFO], sample: -g 1 means LOG_ERROR\n");
    fprintf(stderr, "  -E\tForce in eUICC mode\n");
    fprintf(stderr, "  -q\tQuery EID\n");
    fprintf(stderr, "  -l\tList profiles\n");
    fprintf(stderr, "  -r\tRemove(Delete) profile with iccid\n");
    fprintf(stderr, "  -e\tEnable profile with iccid\n");
    fprintf(stderr, "  -d\tDisable profile with iccid\n");
    fprintf(stderr, "  -a\tDownload profile with Activation Code\n");
    fprintf(stderr, "  -c\tConfirmation Code\n");
    fprintf(stderr, "  -u\tproxy server addr\n");
    fprintf(stderr, "  -i\tswitch eid\n");
    fprintf(stderr, "  -L\tList all eid info\n");
    fprintf(stderr, "  -v\tList C-LPA test tool version\n");
    fprintf(stderr, "  -h\tList this help\n");
    fprintf(stderr, "  -?\tThe same as -h\n");
}

static void print_array(const char *tag, uint8_t *array, uint16_t len, bool cr_lf)
{
    int i = 0;
    uint8_t *p = (uint8_t *)array;

    fprintf(stderr, "%s", tag);
    for (i = 0; i < len; i++) {
        fprintf(stderr, "%02X", p[i]);
    }
    if (cr_lf) {
        fprintf(stderr, "\n");
    }
}

static void print_version(void)
{
#define YEAR        ((((__DATE__ [7] - '0') * 10 + (__DATE__ [8] - '0')) * 10 +\
                    (__DATE__ [9] - '0')) * 10 + (__DATE__ [10] - '0'))
#define MONTH       (__DATE__ [2] == 'c' ? 0 \
                    : __DATE__ [2] == 'b' ? 1 \
                    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 2 : 3) \
                    : __DATE__ [2] == 'y' ? 4 \
                    : __DATE__ [2] == 'n' ? 5 \
                    : __DATE__ [2] == 'l' ? 6 \
                    : __DATE__ [2] == 'g' ? 7 \
                    : __DATE__ [2] == 'p' ? 8 \
                    : __DATE__ [2] == 't' ? 9 \
                    : __DATE__ [2] == 'v' ? 10 : 11)
#define DAY         ((__DATE__ [4] == ' ' ? 0 : __DATE__ [4] - '0') * 10 \
                    + (__DATE__ [5] - '0'))
#define DATE_AS_INT (((YEAR - 2000) * 12 + MONTH) * 31 + DAY)

    fprintf(stderr, "Build  time : %d-%02d-%02d %s\n", YEAR, MONTH + 1, DAY, __TIME__);
    fprintf(stderr, "Tool version: %s\n", VERSION);
}

static int get_uicc_mode(void)
{
    const char *cmd = "cat "RT_DATA_PATH"rt_config.ini | grep UICC_MODE | awk {\' print $3\'}";
    int ret = -1;
    int mode = 0;
    char rsp[1024] = {0};
    
    ret = shell_cmd(cmd, rsp, sizeof(rsp));
    if (ret > 0) {
        mode = atoi(rsp);
        fprintf(stderr, "Config UICC mode: %s\n", rsp);
        if (mode == LPA_CHANNEL_BY_QMI || mode == LPA_CHANNEL_BY_IPC) {
            g_chan_mode = mode;            
        } 
    }

    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    int i;
    char iccid[21] = {0};
    profile_info_t *p_pi;
    uint8_t eid_list[11][33] = {0};

    char *ac = NULL;
    char *cc = NULL;
    uint8_t *url = NULL;

    uint8_t cnt = 0;
    uint8_t buf[4096] = {0};
    int opt = 0;

    uint8_t rsp[32] = {0};
    uint16_t rsp_size = sizeof(rsp);
    FILE *fp = NULL;

    (void)rsp_size;
    (void)fp;

    log_set_param(LOG_PRINTF_TERMINAL, g_log_level, 1*1024*1024); //debug in terminal
    rt_qmi_init(NULL);               // must init qmi
    get_uicc_mode();
    init_apdu_channel(g_chan_mode);  // fore to eUICC mode
    
    if (argc < 2) {
        display_usage();
        return -1;
    }

    fprintf(stderr, "UICC mode : (%d)%s\n", g_chan_mode, (g_chan_mode == LPA_CHANNEL_BY_QMI) ? "eUICC" : "vUICC");
    fprintf(stderr, "log level : (%d)\n", g_log_level);

    opt = getopt(argc, argv, opt_string);
    while (opt != -1) {
        switch (opt) {
            case 'g':
                g_log_level = atoi(optarg);
                fprintf(stderr, "config log level : (%d)\n", g_log_level);
                log_set_param(LOG_PRINTF_TERMINAL, g_log_level, 1*1024*1024); //debug in terminal
                break;
                
            case 'E':
                g_chan_mode = LPA_CHANNEL_BY_QMI;
                init_apdu_channel(g_chan_mode);
                fprintf(stderr, "Force eUICC mode !!!\n");
                break;
                
            case 'q':
                ret = lpa_get_eid(buf);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get EID FAILED: %d\n", ret);
                    goto end;
                }
                print_array("EID: ", buf, 16, true);
                break;

            case 'l':
                p_pi = (profile_info_t *)buf;
                ret = lpa_get_profile_info(p_pi, &cnt, MAX_PROFILE_NUM);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get Profile Info FAILED: %d\n", ret);
                    goto end;
                }
                for (i = 0; i < cnt; i++) {
                    fprintf(stderr, "ICCID #%2d: %s, Class: %d, State: %d\n",
                            i+1, p_pi[i].iccid, p_pi[i].class, p_pi[i].state);
                }
                break;

            case 'r':
                ret = lpa_delete_profile(optarg);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Delete %s FAILED: %d\n", optarg, ret);
                    goto end;
                }
                fprintf(stderr, "Delete %s DONE!\n", optarg);
                break;

            case 'e':
                ret = lpa_enable_profile(optarg);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Enable %s FAILED: %d\n", optarg, ret);
                    goto end;
                }
                fprintf(stderr, "Enable %s DONE!\n", optarg);
                break;

            case 'd':
                ret = lpa_disable_profile(optarg);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Disalbe %s FAILED: %d\n", optarg, ret);
                    goto end;
                }
                fprintf(stderr, "Disalbe %s DONE!\n", optarg);
                break;

            case 'a':
                ac = optarg;
                break;

            case 'c':
                cc = optarg;
                break;

            case 'u':
                url = (uint8_t *)optarg;
                break;
                
            case 'i':
                lpa_switch_eid((uint8_t *)optarg);
                break;
                
            case 'L':
                lpa_get_eid_list(eid_list);
                break;
                
            case 'v':
                print_version();
                break;

            case 'h':   // fall-through
            case '?':
                display_usage();
                break;

            default:
                break;
        }
        opt = getopt(argc, argv, opt_string);
    }

    if (ac != NULL) {
        fprintf(stderr, "download : AC: %s  CC: %s  proxy_url: %s\n", ac, cc, url);
        ret = lpa_download_profile(ac, cc, iccid, url);
        if (ret != RT_SUCCESS) {
            fprintf(stderr, "Download %s FAILED: %d\n", iccid, ret);
            goto end;
        }
        fprintf(stderr, "Download %s DONE!\n", iccid);
    }

end:

    return ret;
}

