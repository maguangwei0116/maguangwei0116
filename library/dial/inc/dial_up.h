
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : dail_up.h
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __DIAL_UP_H__
#define __DIAL_UP_H__

#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include "agent_config.h"

#if (PLATFORM == PLATFORM_9X07)
#include "dsi_netctrl.h"
#include "ds_util.h"
#endif
#include "agent.h"

typedef enum NETWORK_STATE {
    NETWORK_STATE_INIT = 0,
    NETWORK_GET_IP,
    NETWORK_CONNECTING,
    NETWORK_DIS_CONNECTED,
    NETWORK_USING
} network_state_info;

#if (PLATFORM == PLATFORM_9X07)
extern in_addr_t inet_addr (const char *__cp);
extern char *inet_ntoa (struct in_addr __in);

#define SA_FAMILY(addr)         (addr).sa_family
#define SA_DATA(addr)           (addr).sa_data
#define SASTORAGE_FAMILY(addr)  (addr).ss_family
#define SASTORAGE_DATA(addr)    (addr).__ss_padding

typedef enum {
    DSI_STATE_CALL_IDLE,
    DSI_STATE_CALL_CONNECTING,
    DSI_STATE_CALL_CONNECTED,
    DSI_STATE_CALL_DISCONNECTING,
    DSI_STATE_CALL_MAX
} dsi_state_call_state_t;

typedef struct {
    dsi_hndl_t handle;
    volatile boolean dsi_inited;
    dsi_state_call_state_t call_state;
    int32_t cdma_profile_index;
    int32_t umts_profile_index;
    int32_t ip_version;
    dsi_ip_family_t ip_type;
    int8_t *apn;
    int8_t *user;
    int8_t *password;
    dsi_auth_pref_t auth_pref;
} dsi_call_info_t;

static void dsi_net_init_cb_func(void *user_data);
static void dsi_net_cb_fcn( dsi_hndl_t hndl, void * user_data, dsi_net_evt_t evt, dsi_evt_payload_t *payload_ptr );
void quit_dsi(dsi_call_info_t dsi_net_hndl);
#endif

int32_t create_dial_up_task(void);
int32_t init_dial_up(void);
void dial_up_task(void);
network_state_info get_network_state(void);
void set_network_state(network_state_info state);
rt_bool get_ping_state(void);

#endif  //__DIAL_UP_H__
