
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
#include "dsi_netctrl.h"
#include "ds_util.h"
#include "rt_type.h"

extern in_addr_t inet_addr (const char *__cp);
extern char *inet_ntoa (struct in_addr __in);

#define SA_FAMILY(addr)         (addr).sa_family
#define SA_DATA(addr)           (addr).sa_data
#define SASTORAGE_FAMILY(addr)  (addr).ss_family
#define SASTORAGE_DATA(addr)    (addr).__ss_padding

typedef enum {
    DSI_STATE_CALL_IDLE = 0,
    DSI_STATE_CALL_CONNECTING,
    DSI_STATE_CALL_CONNECTED,
    DSI_STATE_CALL_DISCONNECTING,
    DSI_STATE_CALL_MAX
} dsi_state_call_state_e;

typedef struct {
    dsi_hndl_t handle;
    volatile boolean dsi_inited;
    dsi_state_call_state_e call_state;
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
int32_t dial_up_init(dsi_call_info_t *dsi_net_hndl);
int32_t dial_up_to_connect(dsi_call_info_t *dsi_net_hndl);
void regist_dial_callback(void *fun);

#endif  //__DIAL_UP_H__
