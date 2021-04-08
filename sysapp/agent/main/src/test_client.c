
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "ipc_agent_client.h"

static const char *opt_string = "sviedglnh?";

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
    fprintf(stderr, "  -n\tGet network state\n");    
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
    int  reg_state = 0;
    int  dial_up_state = 0;

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
                ret = ipc_agent_get_card_type((agent_switch_card_param_e*)&value);
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
                ret = ipc_agent_get_sim_monitor((agent_set_sim_monitor_param_e*)&value);
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
            case 'n':
                ret = ipc_agent_get_network_state((agent_registration_state_e*)&reg_state, (agent_dsi_state_call_state_e*)&dial_up_state);
                if (ret != RT_SUCCESS) {
                    fprintf(stderr, "Get network state FAILED: %d\n", ret);
                    goto end;
                }
                fprintf(stderr, "registration state is: %d, dial-up state is: %d\n", reg_state, dial_up_state);
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

