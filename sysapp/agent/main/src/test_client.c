
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "ipc_agent_client.h"

static const char *opt_string = "sviedglrh?";

static void display_usage(void)
{
    fprintf(stderr, "This is a clent test tool for rt_agent.\n");
    fprintf(stderr, "Usage: test_client option [arguments]\n");
    fprintf(stderr, "  -s\tSwitch card to SIM\n");
    fprintf(stderr, "  -v\tSwitch card to vUICC\n");
    fprintf(stderr, "  -i\tGet card type\n");
    fprintf(stderr, "  -e\tEnable SIM Monitor\n");
    fprintf(stderr, "  -d\tDisable SIM Monitor\n");
    fprintf(stderr, "  -g\tGet SIM Monitor mode\n");    
    fprintf(stderr, "  -l\tList ICCIDs\n");    
    fprintf(stderr, "  -r\tReset vSIM(remove all operational profiles)\n");
    fprintf(stderr, "  -h\tList this help\n");
    fprintf(stderr, "  -?\tThe same as -h\n");
}

int main(int argc, char **argv)
{
    int  ret;
    int  opt = 0;
    int  value = 0;
    char iccid_list[512];
    int  size = 0;
    int  i = 0;
    int  cnt = 0;
    char *p = NULL;
    char *p1 = NULL;

    if (argc < 2) {
        display_usage();
        return -1;
    }

    opt = getopt(argc, argv, opt_string);
    while (opt != -1) {
        switch (opt) {
            case 's':
                ret = ipc_agent_switch_card(AGENT_SWITCH_TO_SIM);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Switch to SIM FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "Switch to SIM DONE!\n");
                break;

            case 'v':
                ret = ipc_agent_switch_card(AGENT_SWITCH_TO_VSIM);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Switch to vSIM FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "Switch to vSIM DONE!\n");
                break;
            case 'i':
                ret = ipc_agent_get_card_type((uint8_t*)&value);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get Card type FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "Card type mode is: %s!\n", value == 0 ? "VSIM" : "SIM");
                break;  
            case 'e':
                ret = ipc_agent_set_sim_monitor(AGENT_SIM_MONITOR_ENABLE);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Enable SIM monitor FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "Enable SIM monitor DONE!\n");
                break;

            case 'd':
                ret = ipc_agent_set_sim_monitor(AGENT_SIM_MONITOR_DISABLE);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Disalbe SIM monitor FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "Disalbe SIM monitor DONE!\n");
                break;
            case 'g':
                ret = ipc_agent_get_sim_monitor((uint8_t*)&value);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get SIM monitor FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "SIM monitor mode is: %s!\n", value == 0 ? "Disable" : "Enable");
                break;            
            case 'l':
                memset(iccid_list, 0, sizeof(iccid_list));
                size = sizeof(iccid_list);
                ret = ipc_agent_get_iccid_list(iccid_list, &size);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get ICCID list FAILED: %d\n", ret);
                    goto end;
                }
                cnt = atoi(iccid_list);
                fprintf(stderr, "count: %d\n", cnt);
                //fprintf(stderr, "iccid_list: %s\n", iccid_list);

                p = strchr(&iccid_list[0], ',');
                for (i=0; i<cnt; i++) {
                    p1 = p + 1; // iccid
                    p = strchr(&p[1], ','); // class
                    p = strchr(&p[1], ','); // state
                    p = strchr(&p[1], ','); // next iccid
                    if (p != NULL) {
                        p[0] = 0x00;
                    } 
                    fprintf(stderr, "%s\n", p1);
                }
                break;
            case 'r':
                ret = ipc_agent_reset();
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "RESET vSIM FAILED\n");
                    goto end;
                }
                fprintf(stderr, "RESET vSIM SUCCESS\n");
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

end:

    return ret;
}

