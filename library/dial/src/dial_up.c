
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
#include "rt_os.h"
#include "rt_qmi.h"
#include "errno.h"

#define DAIL_UP_WAIT                5           // timers
#define CHK_REG_STATE_INTERVAL      2           // seconds
#define MAX_CHK_REG_STATE          (5 * 60)    // for total 5 mins
#define MAX_CHK_REG_STATE_CNT      (MAX_CHK_REG_STATE / CHK_REG_STATE_INTERVAL) 
#define DIAL_UP_INTERVAL            5           // seconds
#define MAX_CHK_DIAL_STATE          (5 * 60)    // for total 5 mins
#define MAX_CHK_DIAL_STATE_CNT      (MAX_CHK_DIAL_STATE / DIAL_UP_INTERVAL) 
#define has_more_argv()             ((opt < argc) && (argv[opt][0] != '-'))

typedef enum LOCAL_DIAL_UP_STATE {
    LOCAL_DIAL_UP_NO_NET = 0,
    LOCAL_DIAL_UP_IS_CONN,
} local_dial_up_e;

typedef void (*dial_callback)(int32_t state);

static dial_callback g_dial_state_func;
static int32_t g_dsi_event_fd[2] = {-1, -1};

static void dsi_net_init_cb_func(void *user_data)
{
    dsi_call_info_t *phndl = (dsi_call_info_t *)user_data;
    phndl->dsi_inited = 1;
}

static void dsi_net_cb_fcn(dsi_hndl_t hndl, void * user_data, dsi_net_evt_t evt, dsi_evt_payload_t *payload_ptr)
{
    int32_t signo = evt;
    dsi_call_info_t *phndl = (dsi_call_info_t *)user_data;
    
    if (evt == DSI_EVT_WDS_CONNECTED) {
        phndl->ip_type = payload_ptr->ip_type;
    }

    /* Pass on the EVENT to upper application */
    write(g_dsi_event_fd[0], &signo, sizeof(signo));

    MSG_PRINTF(LOG_INFO, "create a dsi event (%d) ...\r\n", signo);
}

static int32_t get_ipv4_net_conf(dsi_call_info_t *phndl)
{
    char iface[DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2];  
    char ip_str[20];
    char command[200];
    struct in_addr public_ip, gw_addr, pri_dns_addr, sec_dns_addr;
    dsi_addr_info_t addr_info;
    int32_t rval;
    int32_t num_entries = 1;

    public_ip.s_addr = gw_addr.s_addr = pri_dns_addr.s_addr = sec_dns_addr.s_addr = 0;
    rt_os_memset(iface, 0, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 2);
    rt_os_memset(&addr_info, 0, sizeof(dsi_addr_info_t));

    rval = dsi_get_device_name(phndl->handle, iface, DSI_CALL_INFO_DEVICE_NAME_MAX_LEN + 1);
    if (rval != DSI_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "Couldn't get ipv4 rmnet name. rval %d\n", rval);
        snprintf(iface, sizeof(iface), "%s", "rmnet0");
        //return RT_ERROR;
    }

    rval = dsi_get_ip_addr(phndl->handle, &addr_info, num_entries);
    if (rval != DSI_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "Couldn't get ipv4 ip address. rval %d\n", rval);
        return RT_ERROR;
    }

    if (addr_info.iface_addr_s.valid_addr) {
        if (SASTORAGE_FAMILY(addr_info.iface_addr_s.addr) == AF_INET) {
            rt_os_memset(ip_str, 0, sizeof(ip_str));
            snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.iface_addr_s.addr)[0], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[1], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[2], SASTORAGE_DATA(addr_info.iface_addr_s.addr)[3]);
            public_ip.s_addr = inet_addr(ip_str);
        }
    }

    if (addr_info.gtwy_addr_s.valid_addr) {
        rt_os_memset(ip_str, 0, sizeof(ip_str));
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[0], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[1], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[2], SASTORAGE_DATA(addr_info.gtwy_addr_s.addr)[3]);
        gw_addr.s_addr = inet_addr(ip_str);
    }

    if (addr_info.dnsp_addr_s.valid_addr) {
        rt_os_memset(ip_str, 0, sizeof(ip_str));
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnsp_addr_s.addr)[3]);
        pri_dns_addr.s_addr = inet_addr(ip_str);
    }

    if (addr_info.dnss_addr_s.valid_addr) {
        rt_os_memset(ip_str, 0, sizeof(ip_str));
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[0], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[1], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[2], SASTORAGE_DATA(addr_info.dnss_addr_s.addr)[3]);
        sec_dns_addr.s_addr = inet_addr(ip_str);
    }

    MSG_PRINTF(LOG_DBG, "public_ip: %s\n", inet_ntoa(public_ip));

#if 0
    snprintf(command, sizeof(command), "ip route add default via %s dev %s", inet_ntoa(gw_addr), iface);
    MSG_PRINTF(LOG_DBG, "%s\n", command);
    ds_system_call(command, rt_os_strlen(command));
#endif

///etc/iproute2/rt_tables
#if 0
    snprintf(command, sizeof(command), "ip route del default dev %s table %s", iface, iface);
    MSG_PRINTF(LOG_DBG, "%s\n", command);
    ds_system_call(command, rt_os_strlen(command));


    snprintf(command, sizeof(command), "ip route add default via %s dev %s table %s", inet_ntoa(gw_addr), iface, iface);
    MSG_PRINTF(LOG_DBG, "%s\n", command);
    ds_system_call(command, rt_os_strlen(command));
#endif

#if !(CFG_STANDARD_MODULE) // unsupported on standard module
    snprintf(command, sizeof(command), "iptables -t filter -F");
    ds_system_call(command, rt_os_strlen(command));
#endif

    snprintf(command, sizeof(command), "iptables -t nat -F");
    ds_system_call(command, rt_os_strlen(command));

    snprintf(command, sizeof(command), "route add default dev rmnet_data0");
    ds_system_call(command, rt_os_strlen(command));

#if 1
    snprintf(command, sizeof(command), "echo 1 > /proc/sys/net/ipv4/ip_forward");
    MSG_PRINTF(LOG_DBG, "%s\n", command);
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
    (void)progname;
    return RT_SUCCESS;
}

static int32_t set_dsi_net_info(dsi_call_info_t *dsi_net_hndl)
{
    int32_t argc = 10;
    int8_t *argv[10];
    int32_t opt = 1;
    dsi_net_hndl->apn = dsi_net_hndl->user = dsi_net_hndl->password = "";
    dsi_net_hndl->auth_pref = DSI_AUTH_PREF_CHAP_ONLY_ALLOWED;
    
    return RT_ERROR;

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
    int32_t rval = RT_SUCCESS;

    rval = dsi_init(DSI_MODE_GENERAL);
    if (rval != DSI_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "dsi init failed!!!\n");
        return RT_ERROR;
    }

    dsi_net_hndl->call_state = DSI_STATE_CALL_IDLE;
    set_dsi_net_info(dsi_net_hndl);
    rt_os_memset(dsi_net_hndl, 0x00, sizeof(dsi_call_info_t));
    dsi_net_hndl->cdma_profile_index = dsi_net_hndl->umts_profile_index = 0;
    dsi_net_hndl->ip_version = DSI_IP_VERSION_4_6;
    dsi_net_hndl->apn = NULL;
    dsi_net_hndl->user = dsi_net_hndl->password = NULL;
    dsi_net_hndl->auth_pref = DSI_AUTH_PREF_PAP_CHAP_NOT_ALLOWED;
    rt_os_sleep(1);
    dsi_net_hndl->handle = dsi_get_data_srvc_hndl(dsi_net_cb_fcn, (void*) dsi_net_hndl);
    if (dsi_net_hndl->handle == NULL){
        MSG_PRINTF(LOG_ERR, "dsi_get_data_srvc hndl fail!!!\n");
        return RT_ERROR;
    }

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
        param_info.buf_val = dsi_net_hndl->apn;
        param_info.num_val = rt_os_strlen(param_info.buf_val);
        dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_APN_NAME, &param_info);
        param_info.buf_val = NULL;
        param_info.num_val = dsi_net_hndl->auth_pref;
        dsi_set_data_call_param(dsi_net_hndl->handle, DSI_CALL_INFO_AUTH_PREF, &param_info);
    }
    
    return RT_SUCCESS;
}

int32_t dial_up_deinit(dsi_call_info_t *dsi_net_hndl)
{
    dsi_rel_data_srvc_hndl(dsi_net_hndl->handle);  // it will release all handle, and you should reinit again !!! 

    return RT_SUCCESS;
}

/* force to create a NO_NET event */
int32_t dial_up_reset(void)
{
    int32_t signo = DSI_EVT_NET_NO_NET;
    
    /* Pass on the EVENT to upper application */
    write(g_dsi_event_fd[0], &signo, sizeof(signo));

    MSG_PRINTF(LOG_ERR, "force to create a NO_NET event ...\r\n");
}

static rt_bool dial_up_get_regist_state(void)
{
    int32_t regist_state = 0;
    int32_t ret;
    
    ret = rt_qmi_get_register_state(&regist_state);
    if (regist_state == 1) {
        MSG_PRINTF(LOG_INFO, "regist state:%d\n", regist_state);
        return RT_TRUE;
    }
    MSG_PRINTF(LOG_INFO, "regist state:%d, ret=%d\n", regist_state, ret);
    
    return RT_FALSE;
}

static int32_t dial_up_check_register_state(int32_t interval, int32_t max_cnt)
{
    int32_t cgk_reg_state_cnt = 0;

    while (dial_up_get_regist_state() != RT_TRUE) {
        rt_os_sleep(interval);
        if (++cgk_reg_state_cnt >= max_cnt) {
            return RT_ERROR;
        }
    }

    return RT_SUCCESS;
}

#if 0  // only for debug
#define DSI_CALL(func, handle) \
({ \
    int32_t __ret = RT_ERROR; \
    static int32_t index = 0; \
    __ret = func((handle)); \
    index++; \
    MSG_PRINTF(LOG_INFO, "----------------------------------- %25s --- %5d, ret=%d\n", #func, index, __ret); \
    __ret; \
})
#else
#define DSI_CALL(func, handle)  func((handle))
#endif

static int32_t dial_up_start(dsi_call_info_t *dsi_net_hndl, int32_t interval, int32_t max_cnt)
{
    int32_t rval;
    int32_t dsi_start_cnt = 0;

    if (g_dsi_event_fd[0] < 0 && g_dsi_event_fd[1] < 0) {
        socketpair(AF_LOCAL, SOCK_STREAM, 0, g_dsi_event_fd);
        MSG_PRINTF(LOG_INFO, "< create two new sockets --- g_dsi_event_fd(%d,%d) >\n", g_dsi_event_fd[0], g_dsi_event_fd[1]);
    }

    while (1) {
        rval = DSI_CALL(dsi_start_data_call, dsi_net_hndl->handle);        
        if (rval == RT_SUCCESS) {
            break;
        }
        MSG_PRINTF(LOG_INFO, "dsi start data call rval = %d\n", rval);
        
        rt_os_sleep(interval);
        if (++dsi_start_cnt >= max_cnt) {
            return RT_ERROR;
        }
    }

    return RT_SUCCESS;
}

static int32_t dial_up_stop(dsi_call_info_t *dsi_net_hndl)
{
    int32_t rval;

#if 0
    /* keep socket pair fd open */
    if (g_dsi_event_fd[0] > 0) {
        close(g_dsi_event_fd[0]);
        g_dsi_event_fd[0] = -1;
    }
    if (g_dsi_event_fd[1] > 0) {
        close(g_dsi_event_fd[1]);
        g_dsi_event_fd[1] = -1;
    }
#endif

    rval = DSI_CALL(dsi_stop_data_call, dsi_net_hndl->handle);
    MSG_PRINTF(LOG_INFO, "dsi stop data call rval = %d\r\n", rval);
    rt_os_sleep(2);

    return RT_SUCCESS;
}

static int32_t dial_up_check_connect_state(dsi_call_info_t *dsi_net_hndl, local_dial_up_e *state)
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
    rt_bool exit_flag = RT_FALSE;

    pollfds[0].fd = g_dsi_event_fd[1];
    pollfds[0].events = POLLIN;
    pollfds[0].revents = 0;
    nevents = sizeof(pollfds)/sizeof(pollfds[0]);

    while (1) {
        //MSG_PRINTF(LOG_INFO, "< [BGN]: detect POLLIN event on sockets > \n");        
        do {            
            ret = poll(pollfds, nevents, -1);        
        } while ((ret < 0) && (errno == EINTR));        
        //MSG_PRINTF(LOG_INFO, "< [END]: detect POLLIN event on sockets > \n");

        for (ne = 0; ne < nevents; ne++) {
            fd = pollfds[0].fd;
            revents = pollfds[0].revents;

            /* Check the current events after poll() returns. */
            if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
                MSG_PRINTF(LOG_DBG, "epoll fd = %d, events = 0x%04x\n", fd, revents);
                if (revents & POLLHUP){
                    break;
                }
            }

            /* If the curerent event isn't POLLERR / POLLHUP / POLLNVAL / POLLIN, discard the event. */
            if ((revents & POLLIN) == 0) {
                MSG_PRINTF(LOG_DBG, "no pollin ...\r\n");
                continue;
            }

            /* Handle the POLLIN event. */
            if (fd == g_dsi_event_fd[1]) {
                if (read(fd, &signo, sizeof(signo)) == sizeof(signo)) {
                    switch (signo) {
                        /* Data call is disconnected */
                        case DSI_EVT_NET_NO_NET:
                            MSG_PRINTF(LOG_DBG, "DSI_EVT_NET_NO_NET\n");
                            exit_flag = RT_TRUE;
                            *state = LOCAL_DIAL_UP_NO_NET;
                            if (dsi_get_call_end_reason(dsi_net_hndl->handle, &dsicallend, dsi_net_hndl->ip_type) == DSI_SUCCESS) {
                                MSG_PRINTF(LOG_DBG, "dsi_get_call_end_reason handle reason type=%d, reason code=%d\n",
                                    dsicallend.reason_type,dsicallend.reason_code);
                            }
                        break;

                        /* WDS connected */
                        case DSI_EVT_WDS_CONNECTED:
                            if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V4) {
                                MSG_PRINTF(LOG_DBG, "DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_V4\n");
                            } else if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V6){
                                MSG_PRINTF(LOG_DBG, "DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_V6\n");
                            } else {
                                MSG_PRINTF(LOG_DBG, "DSI_EVT_WDS_CONNECTED DSI_IP_FAMILY_UNKNOW\n");
                            }
                        break;

                        /* Data call is connected */
                        case DSI_EVT_NET_IS_CONN:
                            if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V4) {
                                if (RT_SUCCESS == get_ipv4_net_conf(dsi_net_hndl)) {
                                    MSG_PRINTF(LOG_DBG, "DSI_EVT_NET_IS_CONN\n");
                                    count = 0;
                                    exit_flag = RT_TRUE;
                                    *state = LOCAL_DIAL_UP_IS_CONN;                                    
                                }
                            } else if (dsi_net_hndl->ip_type == DSI_IP_FAMILY_V6) {
                                MSG_PRINTF(LOG_DBG, "donot support DSI_IP_FAMILY_V6 by now!!!!\n");
                            }
                        break;
                        
                        case DSI_EVT_NET_PARTIAL_CONN:                            
                            MSG_PRINTF(LOG_DBG, "DSI_EVT_PARTIAL_CONN\n");
                            break;

                        // Net ip address is generated                        
                        case DSI_EVT_NET_NEWADDR:                            
                            MSG_PRINTF(LOG_DBG, "DSI_EVT_NET_NEWADDR\n");
                            break;
                        
                        default:
                            break;
                    }
                }
            }
        }

        if (exit_flag) {
            break;
        }
        
    }
    
    return RT_SUCCESS;
}

#define dial_up_state_changed(handle, new_state)\
    do {\
        MSG_PRINTF(LOG_INFO, "DIAL-UP STATE: %d ==> %d (%s)\r\n", (handle)->call_state, new_state, #new_state);\
        (handle)->call_state = new_state;\
        g_dial_state_func((handle)->call_state);\
    } while (0)

static int32_t dial_up_state_mechine_start(dsi_call_info_t *dsi_net_hndl)
{
    local_dial_up_e state;

    while (1) {
        switch (dsi_net_hndl->call_state) {
            case DSI_STATE_CALL_IDLE:
                MSG_PRINTF(LOG_WARN, "Start dial up\r\n");
                if (dial_up_check_register_state(CHK_REG_STATE_INTERVAL, MAX_CHK_REG_STATE_CNT) == RT_SUCCESS) {
                    dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_CONNECTING);
                } else {
                    #if 0
                    dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_DISCONNECTING);
                    #else
                    /* 
                    call stop-data-call directly to avoid network state change,  
                    lead to a network-state changed in card detection,  
                    lead to a new select card when it's using provisoning profile. 
                    */
                    dial_up_stop(dsi_net_hndl);
                    #endif
                }
                break;

            case DSI_STATE_CALL_CONNECTING:
                if (dial_up_start(dsi_net_hndl, DIAL_UP_INTERVAL, MAX_CHK_DIAL_STATE_CNT) == RT_SUCCESS) {                    
                    dial_up_check_connect_state(dsi_net_hndl, &state);
                    if (state == LOCAL_DIAL_UP_NO_NET) {
                        dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_DISCONNECTING);
                    } else if (state == LOCAL_DIAL_UP_IS_CONN) {
                        dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_CONNECTED);
                    }
                } else {
                    dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_DISCONNECTING);
                }
                break;

            case DSI_STATE_CALL_CONNECTED:
                dial_up_check_connect_state(dsi_net_hndl, &state);
                if (state == LOCAL_DIAL_UP_NO_NET) {
                    dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_DISCONNECTING);
                } else if (state == LOCAL_DIAL_UP_IS_CONN) {
                    dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_CONNECTED);
                }
                break;

            case DSI_STATE_CALL_DISCONNECTING:
                dial_up_stop(dsi_net_hndl);
                dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_IDLE);
                rt_os_sleep(5);  // wait some time to start a new dial-up operation !!!
                break;

            default:
                MSG_PRINTF(LOG_ERR, "==>unexpected dial-up state %d !!!\r\n", dsi_net_hndl->call_state);
                dial_up_state_changed(dsi_net_hndl, DSI_STATE_CALL_DISCONNECTING);
                break;
        }
    }

    return RT_SUCCESS;
}

int32_t dial_up_to_connect(dsi_call_info_t *dsi_net_hndl)
{
    return dial_up_state_mechine_start(dsi_net_hndl);  
}
    
void dial_up_set_dial_callback(void* func)
{
    g_dial_state_func = (dial_callback)func;
}

