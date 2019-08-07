
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : dial_up.c
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "dial_up.h"
#include "fallback.h"
#include "rt_os.h"
#include "net_monitor.h"
#include "api.h"

#if (PLATFORM == PLATFORM_9X07)
#include "dial_up.h"
    #if (MANUFACTURE == MANUFACTURE_ZTE)
         #include "libzte_wan.h"
    #elif (MANUFACTURE == MANUFACTURE_GSW)
         #include "libgsw_wan.h"
    #endif
#elif PLATFORM == PLATFORM_FIBCOM
#include "fibo_inps_interface.h"
#include "fibofwk.h"
#endif

#define PING_ADDR1                              "ping -c 1 -W 3 18.136.190.97"
#define PING_ADDR2                              "ping -c 1 -W 3 23.91.101.68"


network_state_info g_network_state = NETWORK_DIS_CONNECTED;
extern uint8_t g_apn_name[MAX_APN_LENGTH + 1];

#if(PLATFORM == PLATFORM_9X07)  // 1-1
#if (MANUFACTURE == MANUFACTURE_QUECTEL || MANUFACTURE == MANUFACTURE_FORGE)  // 2-1

#define DAIL_UP_WAIT      5
#define has_more_argv() ((opt < argc) && (argv[opt][0] != '-'))

static int signal_event_fd[2];
static int dsi_event_fd[2];
//static char * meige_apn = "cmiot";

static void dsi_net_init_cb_func(void *user_data)
{
    dsi_call_info_t *phndl = (dsi_call_info_t *)user_data;
    //dial_printf("%s user_data=%p\n", __func__, user_data);
    phndl->dsi_inited = 1;
}

static void dsi_net_cb_fcn( dsi_hndl_t hndl, void * user_data, dsi_net_evt_t evt, dsi_evt_payload_t *payload_ptr )
{
    int32_t signo = evt;
    dsi_call_info_t *phndl = (dsi_call_info_t *)user_data;
    if (evt == DSI_EVT_WDS_CONNECTED) {
        phndl->ip_type = payload_ptr->ip_type;
    }
    write(dsi_event_fd[0], &signo, sizeof(signo));
}

static int32_t get_ipv4_net_conf(dsi_call_info_t *phndl)
{
    int8_t iface[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];
    int32_t rval;
    dsi_addr_info_t addr_info;
    int32_t num_entries = 1;
    int8_t ip_str[20];
    int8_t command[200];
    struct in_addr public_ip, gw_addr, pri_dns_addr, sec_dns_addr;

    public_ip.s_addr = gw_addr.s_addr = pri_dns_addr.s_addr = sec_dns_addr.s_addr = 0;
    memset(iface, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2);
    memset(&addr_info, 0, sizeof(dsi_addr_info_t));

    rval = dsi_get_device_name(phndl->handle, iface, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
    if (rval != DSI_SUCCESS) {
        MSG_WARN ("Couldn't get ipv4 rmnet name. rval %d\n", rval);
        strncpy((int8_t *)iface, "rmnet0", DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
        return RT_ERROR;
    }

    rval = dsi_get_ip_addr(phndl->handle, &addr_info, num_entries);
    if (rval != DSI_SUCCESS) {
        MSG_WARN("Couldn't get ipv4 ip address. rval %d\n", rval);
        return RT_ERROR;
    }

    if (addr_info.iface_addr_s.valid_addr) {
        if (SASTORAGE_FAMILY(addr_info.iface_addr_s.addr) == AF_INET){
            memset(ip_str, 0, 20);
            snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.iface_addr_s.addr)[0], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[1], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[2], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[3]);
            public_ip.s_addr = inet_addr(ip_str);
        }
    }

    if (addr_info.gtwy_addr_s.valid_addr) {
        memset(ip_str, 0, 20);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[0], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[1], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[2], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[3]);
        gw_addr.s_addr = inet_addr(ip_str);
    }

    if (addr_info.dnsp_addr_s.valid_addr) {
        memset(ip_str, 0, 20);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[3]);
        pri_dns_addr.s_addr = inet_addr(ip_str);
    }

    if (addr_info.dnss_addr_s.valid_addr) {
        memset(ip_str, 0, 20);
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[3]);
        sec_dns_addr.s_addr = inet_addr(ip_str);
    }

    MSG_DBG("public_ip: %s\n", inet_ntoa(public_ip));

#if 0
    snprintf(command, sizeof(command), "ip route add default via %s dev %s", inet_ntoa(gw_addr), iface);
    MSG_DBG("%s\n", command);
    ds_system_call(command, strlen(command));
#endif

///etc/iproute2/rt_tables
#if 0
    snprintf(command, sizeof(command), "ip route del default dev %s table %s", iface, iface);
    MSG_DBG("%s\n", command);
    ds_system_call(command, strlen(command));


    snprintf(command, sizeof(command), "ip route add default via %s dev %s table %s", inet_ntoa(gw_addr), iface, iface);
    MSG_DBG("%s\n", command);
    ds_system_call(command, strlen(command));
#endif

    snprintf(command, sizeof(command), " iptables -t filter -F ");
    ds_system_call(command, rt_os_strlen(command));

    snprintf(command, sizeof(command), " iptables -t nat -F ");
    ds_system_call(command, rt_os_strlen(command));

    snprintf(command, sizeof(command), "route add default dev rmnet_data0");
    ds_system_call(command, rt_os_strlen(command));

#if (ROUTE_SET)
    snprintf(command, sizeof(command), "echo 1 > /proc/sys/net/ipv4/ip_forward");
    MSG_DBG("%s\n", command);
    ds_system_call(command, rt_os_strlen(command));

    snprintf(command, sizeof(command), "iptables -t nat -A POSTROUTING -o rmnet_data0 -j MASQUERADE --random");
    ds_system_call(command, rt_os_strlen(command));
#endif
    if (pri_dns_addr.s_addr) {
        snprintf(command, sizeof(command), "echo 'nameserver %s' > /etc/resolv.conf", inet_ntoa(pri_dns_addr));
        ds_system_call(command, rt_os_strlen(command));

        if (sec_dns_addr.s_addr) {
            snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf", inet_ntoa(sec_dns_addr));
            ds_system_call(command, rt_os_strlen(command));
        }
    }
    return RT_SUCCESS;
}

static int32_t usage(const int8_t *progname)
{
    return 0;
}

int32_t set_dsi_net_info(dsi_call_info_t *dsi_net_hndl)
{
    int32_t argc = 10;
    int8_t *argv[10];
    int32_t opt = 1;
    dsi_net_hndl->apn = dsi_net_hndl->user = dsi_net_hndl->password = "";
    dsi_net_hndl->auth_pref = DSI_AUTH_PREF_CHAP_ONLY_ALLOWED;
    return -1;

    while  (opt < argc) {
        if (argv[opt][0] != '-')
            return usage(argv[0]);

        switch (argv[opt++][1]) {
            case 's': {
                dsi_net_hndl->apn = dsi_net_hndl->user = dsi_net_hndl->password = "";
                dsi_net_hndl->auth_pref = DSI_AUTH_PREF_CHAP_ONLY_ALLOWED;
                if (has_more_argv()) {
                    dsi_net_hndl->apn = argv[opt++];
                }
                if (has_more_argv()) {
                    dsi_net_hndl->user = argv[opt++];
                }
                if (has_more_argv()) {
                    dsi_net_hndl->password = argv[opt++];
                    if (dsi_net_hndl->password && dsi_net_hndl->password[0]) {
                        dsi_net_hndl->auth_pref = DSI_AUTH_PREF_CHAP_ONLY_ALLOWED; //default chap, customers may miss auth
                    }
                }
                if (has_more_argv()) {
                    dsi_net_hndl->auth_pref = argv[opt++][0] - '0';
                }
                break;
            }

            case 'n': {
                if (has_more_argv()) {
                    dsi_net_hndl->cdma_profile_index = dsi_net_hndl->umts_profile_index = argv[opt++][0] - '0';
                }
                break;
            }

            case '6': {
                dsi_net_hndl->ip_version = DSI_IP_VERSION_6;
                break;
            }

            default:
                return usage(argv[0]);
                break;
        }
    }
}

int32_t dial_up_init(dsi_call_info_t *dsi_net_hndl)
{
    dsi_call_param_value_t param_info;
    int32_t rval = 0;
    // static int8_t ii = 0;
    dsi_net_hndl->call_state = DSI_STATE_CALL_IDLE;
    set_dsi_net_info(dsi_net_hndl);
    rt_os_memset(dsi_net_hndl, 0x00, sizeof(dsi_call_info_t));
    dsi_net_hndl->cdma_profile_index = dsi_net_hndl->umts_profile_index = 0;
    dsi_net_hndl->ip_version = DSI_IP_VERSION_4_6;
    dsi_net_hndl->apn = get_apn_data(NULL);
    dsi_net_hndl->user = dsi_net_hndl->password = NULL;
    dsi_net_hndl->auth_pref = DSI_AUTH_PREF_PAP_CHAP_NOT_ALLOWED;

    dsi_net_hndl->handle = dsi_get_data_srvc_hndl(dsi_net_cb_fcn, (void*) dsi_net_hndl);
    if (dsi_net_hndl->handle == NULL){
        MSG_WARN("dsi_get_data_srvc_hndl fail!!!\n");
        dsi_release(DSI_MODE_GENERAL);
        return -3;
    }
    // ii=0;
    param_info.buf_val = NULL;
    param_info.num_val = DSI_RADIO_TECH_UNKNOWN;
    dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_TECH_PREF, &param_info);

    param_info.buf_val = NULL;
    param_info.num_val = dsi_net_hndl->umts_profile_index;;
    dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_UMTS_PROFILE_IDX, &param_info);

    param_info.buf_val = NULL;
    param_info.num_val = dsi_net_hndl->cdma_profile_index;
    dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_CDMA_PROFILE_IDX, &param_info);

    param_info.buf_val = NULL;
    param_info.num_val = dsi_net_hndl->ip_version;
    dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_IP_VERSION, &param_info);
    if (dsi_net_hndl->apn && dsi_net_hndl->apn[0]) {
        MSG_DBG("start set apn:%s\n",dsi_net_hndl->apn);
        rt_modify_profile(1,0,dsi_net_hndl->apn,0);
        param_info.buf_val = dsi_net_hndl->apn;
        param_info.num_val = strlen(param_info.buf_val);
        dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_APN_NAME, &param_info);
#if 0
        param_info.buf_val = strdup(dsi_net_hndl.user);
        param_info.num_val = strlen(param_info.buf_val);
        dsi_set_data_call_param(dsi_net_hndl.handle, DSI_CALL_INFO_USERNAME, &param_info);

        param_info.buf_val = strdup(dsi_net_hndl.password);
        param_info.num_val = strlen(param_info.buf_val);
        dsi_set_data_call_param(dsi_net_hndl.handle, DSI_CALL_INFO_PASSWORD, &param_info);
#endif
        param_info.buf_val = NULL;
        param_info.num_val = dsi_net_hndl->auth_pref;
        dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_AUTH_PREF, &param_info);
    }
    return 1;
}

int32_t dial_up_stop(dsi_call_info_t *dsi_net_hndl)
{
    if (dsi_net_hndl->call_state!=DSI_STATE_CALL_IDLE) {
        dsi_stop_data_call(dsi_net_hndl->handle);
    }
    dsi_rel_data_srvc_hndl(dsi_net_hndl->handle);
    close(dsi_event_fd[0]);
    close(dsi_event_fd[1]);
    return RT_SUCCESS;
}

rt_bool dial_up_reinit(dsi_call_info_t *dsi_net_hndl)
{
    dial_up_stop(dsi_net_hndl);
    rt_os_sleep(15);
    dial_up_init(dsi_net_hndl);
    return RT_TRUE;
}

static int32_t dial_up_to_connect(dsi_call_info_t *dsi_net_hndl)
{

    dsi_ce_reason_t dsicallend;
    int32_t rval, signo;
    int32_t count = 0;
    int32_t ne = 0;
    int32_t ret = 0;
    struct pollfd pollfds[1];
    int32_t nevents = 0;
    int32_t fd = 0;
    int16_t revents = 0;

    socketpair( AF_LOCAL, SOCK_STREAM, 0, dsi_event_fd);
    dial_up_init(dsi_net_hndl);

    pollfds[0].fd = dsi_event_fd[1];
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;
    nevents = sizeof(pollfds)/sizeof(pollfds[0]);
    while (1) {
        if (rt_get_register_state() == RT_TRUE) {
            if (dsi_net_hndl->call_state == DSI_STATE_CALL_IDLE) {
                rval = dsi_start_data_call(dsi_net_hndl->handle);
                if (DSI_SUCCESS != rval) {
                    MSG_WARN("dsi_start_data_call rval = %d\n", rval);
                } else {
                    dsi_net_hndl->call_state = DSI_STATE_CALL_CONNECTING;
                }
            }

            do {
                ret = poll(pollfds,nevents,5);
                rt_os_sleep(2);
            } while (ret < 0);
            for (ne = 0; ne < nevents; ne++) {
                fd = pollfds[0].fd;
                revents = pollfds[0].revents;
                // dial_printf("fd:%d revents:%d\n",fd,revents);
                if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    MSG_DBG("epoll fd = %d, events = 0x%04x\n", fd, revents);
                    if (revents & POLLHUP){
                        break;
                    }
                }
                if ((revents & POLLIN) == 0) {
                    if ((dsi_net_hndl->call_state == DSI_STATE_CALL_CONNECTING)||
                            (get_network_state() == NETWORK_DIS_CONNECTED)||
                            (get_network_state() == NETWORK_GET_IP)) {
                        MSG_DBG("call_count:%d\n",++count);
                        if (count >= DAIL_UP_WAIT) {
                            count = 0;
                            return RT_ERROR;
                        }
                    }
                    continue;
                }
                if (fd == dsi_event_fd[1]) {
                    if (read(fd, &signo, sizeof(signo)) == sizeof(signo)) {
                        switch (signo){
                            case DSI_EVT_WDS_CONNECTED:
                                if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V4) {
                                    MSG_DBG("DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_V4\n");
                                } else if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V6){
                                    MSG_DBG("DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_V6\n");
                                } else {
                                    MSG_DBG("DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_UNKNOW\n");
                                }
                            break;
                            case DSI_EVT_NET_IS_CONN:
                                MSG_DBG("DSI_EVT_NET_IS_CONN\n");
                                count = 0;
                                dsi_net_hndl->call_state = DSI_STATE_CALL_CONNECTED;
                                if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V4) {
                                    get_ipv4_net_conf(dsi_net_hndl);
                                } else if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V6) {
                                    MSG_DBG("donot support DSI_IP_FAMILY_V6 by now!!!!\n");
                                }
                                set_network_state(NETWORK_GET_IP);
                            break;
                            case DSI_EVT_NET_NO_NET:
                                MSG_DBG("DSI_EVT_NET_NO_NET\n");
                                dsi_net_hndl->call_state = DSI_STATE_CALL_IDLE;
                                if (dsi_get_call_end_reason(dsi_net_hndl->handle, &dsicallend, dsi_net_hndl->ip_type) == DSI_SUCCESS) {
                                    MSG_DBG("dsi_get_call_end_reason handle＝％d type=%d reason code =%d \n",dsicallend.reason_type,dsicallend.reason_code);
                                }
                                if (get_network_state() != NETWORK_DIS_CONNECTED) {
                                    set_network_state(NETWORK_DIS_CONNECTED);
                                }
                            break;
                            default:
                            break;
                        }
                    }
                }
            }

        } else {
            rt_os_sleep(3);
            set_network_state(NETWORK_DIS_CONNECTED);
        }

        if (get_vsim_state() != ESIM_STATE_INSERTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
            break;
        }
        rt_os_sleep(2);
    }
    return RT_SUCCESS;
}

void dial_up_task(void)
{
    dsi_init(DSI_MODE_GENERAL);
    while(1) {
        if (get_vsim_state() == ESIM_STATE_INSERTED) {
            dsi_call_info_t dsi_net_hndl;
            dial_up_to_connect(&dsi_net_hndl);
            dial_up_stop(&dsi_net_hndl);
        }
        MSG_DBG("dial_up_task\n");
        rt_os_sleep(3);
    }
}
#elif MANUFACTURE == MANUFACTURE_HONGDIAN  // 2-2
static int32_t dial_up_to_connect(void)
{
    while (1) {
        if (get_register_state() == RT_TRUE) {
            if (get_network_state() == NETWORK_DIS_CONNECTED) {
                if (detect_network_state() == RT_TRUE) {
                    set_network_state(NETWORK_GET_IP);
                }
            }
        } else {
            set_network_state(NETWORK_DIS_CONNECTED);
        }

        if (get_vsim_state() != ESIM_STATE_INSERTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
            break;
        }
        rt_os_sleep(3);
    }
    return RT_SUCCESS;
}

#elif MANUFACTURE == MANUFACTURE_GSW  // 2-3
static int32_t gsw_to_connect(void)
{
    gsw_wan_op_state_e_type  wan_ret = GSW_WAN_OP_FAILED;
    while (1) {
        if ((rt_get_register_state() == RT_TRUE) && (get_network_state() == NETWORK_DIS_CONNECTED)) {
            wan_ret = gsw_wan_set_pdp_call_cmd(GSW_WAN_IPV4_START_CALL, GSW_WAN_ATCOP_FTP_CALL);
            MSG_DBG("wan_ret:%d\n",wan_ret);
            if (GSW_WAN_OP_SUCCESS != wan_ret) {
                return RT_ERROR;
            } else {
                set_network_state(NETWORK_GET_IP);
            }
        }
        if (get_vsim_state() != ESIM_STATE_INSERTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
            break;
        }
        rt_os_sleep(3);
    }
    return RT_SUCCESS;
}
void dial_up_task(void)
{
    while(1) {
        if (get_vsim_state() == ESIM_STATE_INSERTED) {
            rt_modify_profile(1,0,get_apn_data(NULL),0);
            gsw_to_connect();
        }
        rt_os_sleep(3);
    }
}
#elif MANUFACTURE == MANUFACTURE_ZTE  // 2-3
static int32_t zte_to_connect(void)
{
    zte_wan_op_state_e_type  wan_ret = ZTE_WAN_OP_FAILED;
    while (1) {
        if ((rt_get_register_state()==RT_TRUE) && (get_network_state()==NETWORK_DIS_CONNECTED)) {
            wan_ret = zte_wan_set_pdp_call_cmd(ZTE_WAN_IPV4_START_CALL, ZTE_WAN_ATCOP_FTP_CALL);
            MSG_DBG("wan_ret:%d\n",wan_ret);
            if (ZTE_WAN_OP_SUCCESS != wan_ret) {
                return RT_ERROR;
            } else {
                set_network_state(NETWORK_GET_IP);
            }
        }
        if (get_vsim_state() != ESIM_STATE_INSERTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
            break;
        }
        rt_os_sleep(3);
    }
    return RT_SUCCESS;
}
void dial_up_task(void)
{
    while(1) {
        if (get_vsim_state() == ESIM_STATE_INSERTED) {
            rt_modify_profile(1,0,get_apn_data(NULL),0);
            zte_to_connect();
        }
        rt_os_sleep(3);
    }
}
#endif  // 2-e

#elif (PLATFORM == PLATFORM_FIBCOM)  // 1-2
#define DEFAULT_PROFILE_INDEX   1
#define MAX_CALL_COUNT          3

static void  display_net_ipv4_info(fibo_inps_ProfileRef_t profileRef)
{
    fibo_result_t     res;
    char ipAddr[100] = {0};

    res = fibo_inps_GetIPAddress(profileRef, ipAddr, sizeof(ipAddr));
    MSG_DBG("fibo_inps_GetIPAddress ipAddr:%s result:%s\n", ipAddr, FIBO_RESULT_TXT(res));
}

static void connection_state_handler(fibo_inps_ProfileRef_t profileRef,fibo_inps_ConState_t state,void* contextPtr)
{
    //LOG_DEBUG("ConnectionStateHandler state:%d", state, 0);

    if (state == FIBO_INPS_DISCONNECTED) {
        MSG_DBG("NETWORK DISCONNECTED\n");
        if (get_network_state()!= NETWORK_DIS_CONNECTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
        }
    } else if (state == FIBO_INPS_CONNECTING){
        MSG_DBG("NETWORK CONNECTING\n");
    } else if (state == FIBO_INPS_CONNECTED) {
        MSG_DBG("NETWORK CONNECTED\n");
        set_network_state(NETWORK_GET_IP);
        display_net_ipv4_info(profileRef);
    }

}

void data_callback(void *args)
{
    fibo_inps_ConnectService();
    fibo_inps_ProfileRef_t ProfileRef = (fibo_inps_ProfileRef_t)args;
    fibo_inps_AddSessionStateHandler(ProfileRef, connection_state_handler, NULL);
    fibo_event_RunLoop();
}

static int32_t fibcom_data_connect(void)
{
    fibo_result_t     res;
    fibo_inps_ProfileRef_t inpsProfileRef = NULL;
    fibo_inps_ConState_t state = FIBO_INPS_DISCONNECTED;
    int8_t call_count = 0;
    fibo_thread_Ref_t data_thread;

    MSG_DBG("Set apn:%s\n", g_apn_name);
    inpsProfileRef = fibo_inps_GetProfile(g_apn_name);
    if(!inpsProfileRef)
    {
        MSG_WARN("fibo_inps_GetProfile return NULL");
        return;
    }
    data_thread = fibo_thread_Create("data_callback", data_callback, inpsProfileRef);
    fibo_thread_SetJoinable(data_thread);
    fibo_thread_Start(data_thread);
    while(1) {
        if ((rt_get_register_state() == RT_TRUE) && (get_network_state() == NETWORK_DIS_CONNECTED)) {
            res = fibo_inps_GetSessionState(inpsProfileRef, &state);
            if (state == FIBO_INPS_CONNECTED) {
                res = fibo_inps_StopSession(inpsProfileRef);
                MSG_WARN("fibo_inps_StopSession result:%s\n", FIBO_RESULT_TXT(res));
            }
            if(cfg_set("rndis_cid", "1") != FIBO_OK){
                MSG_ERR("cfg_set rndis_cid fail");
                return;
            }
            res = fibo_inps_StartSession(inpsProfileRef);
            if (res == FIBO_OK) {
                rt_os_sleep(10);
            } else {
                MSG_DBG("fibo_inps_StartSession result:%s\n", FIBO_RESULT_TXT(res));
            }

            MSG_DBG("call_count:%d\n", call_count);
            if(call_count > MAX_CALL_COUNT){
                break;
            }
        }

        if (get_vsim_state() != ESIM_STATE_INSERTED) {
            set_network_state(NETWORK_DIS_CONNECTED);
            break;
        }
        rt_os_sleep(3);
    }
    fibo_thread_Cancel(data_thread);
    fibo_thread_Join(data_thread, NULL);
}

void dial_up_task(void)
{
    fibo_inps_ConnectService();  // enable fibcom mdc service
    set_vsim_state(ESIM_STATE_INSERTED);
    while(1) {
        if (get_vsim_state() == ESIM_STATE_INSERTED) {
//            rt_modify_profile(1,0,get_apn_data(NULL),0);
            fibcom_data_connect();
        }
        rt_os_sleep(3);
    }
}
#endif  // 1-e

int32_t init_dial_up(void)
{
    int32_t ret = RT_ERROR;
    rt_task id;
    ret = rt_create_task("dial_up_task", &id, (void *)dial_up_task, NULL);
    if (ret == RT_ERROR) {
        MSG_WARN("creat pthread error\n");
    }
    return RT_SUCCESS;
}

network_state_info get_network_state(void)
{
    return g_network_state;
}

void set_network_state(network_state_info state)
{
    g_network_state = state;
}

/*****************************************************************************
 * FUNCTION
 *  get_ping_state
 * DESCRIPTION
 *  ping emq
 * PARAMETERS
 *  @void
 * RETURNS
 *  @bool
 *****************************************************************************/
rt_bool get_ping_state(void)
{
    int32_t ping_state = 0;
    if (get_network_state() == NETWORK_USING) {
        return RT_TRUE;
    }
    if (system(PING_ADDR1) == 0 || system(PING_ADDR2) == 0) {
        return RT_TRUE;
    }
    MSG_WARN("ping error\r\n");
    return RT_FALSE;
}
