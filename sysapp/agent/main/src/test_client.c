
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "ipc_agent_client.h"

static const char *opt_string = "sviedgh?";

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
    fprintf(stderr, "  -h\tList this help\n");
    fprintf(stderr, "  -?\tThe same as -h\n");
}

int main(int argc, char **argv)
{
    int ret;
    int opt = 0;
    int value = 0;
    
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

