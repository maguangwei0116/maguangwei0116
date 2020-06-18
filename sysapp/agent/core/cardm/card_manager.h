
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_manager.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __CARD_MANAGER_H__
#define __CARD_MANAGER_H__

#include "rt_type.h"
#include "lpa.h"
#include "msg_process.h"

#define THE_MAX_CARD_NUM            20
#define THE_ICCID_LENGTH            20
#define MAX_EID_LEN                 32
#define MAX_EID_HEX_LEN             16
#define MAX_APN_LEN                 32

typedef enum PROFILE_STATE {
    PROFILE_DISABLED                = 0,
    PROFILE_ENABLED                 = 1,
} profile_state_e;

typedef enum PROFILE_TYPE {
    PROFILE_TYPE_TEST               = 0,
    PROFILE_TYPE_PROVISONING        = 1,
    PROFILE_TYPE_OPERATIONAL        = 2,
#ifdef CFG_REDTEA_READY_ON
    PROFILE_TYPE_SIM                = 3,
#endif
} profile_type_e;

typedef enum CARD_STATE {
    CARD_STATE_DISABLED             = 0,
    CARD_STATE_ENABLED              = 1,
} card_state_e;

typedef enum INIT_PROFILE_TYPE {
    INIT_PROFILE_TYPE_PROVISONING   = 0,
    INIT_PROFILE_TYPE_OPERATIONAL   = 1,
    INIT_PROFILE_TYPE_LAST_USED     = 2,
} init_profile_type_e;

typedef enum PROFILE_SIM_CPIN {
    SIM_CPIN_ERROR                  = 0,
    SIM_CPIN_READY                  = 1,
} profile_sim_cpin_e;

typedef struct PROFILE_SIM_INFO {
    uint8_t             iccid[THE_ICCID_LENGTH + 1];
    profile_sim_cpin_e  state;
} profile_sim_info_t;

typedef struct CARD_INFO {
    profile_info_t                  info[THE_MAX_CARD_NUM];
    profile_sim_info_t              sim_info;
    uint8_t                         num;
    uint8_t                         eid[MAX_EID_LEN + 1];
    profile_type_e                  type;                        // used card type
    profile_type_e                  last_type;                   // used card type
    uint8_t                         iccid[THE_ICCID_LENGTH + 1]; // used card iccid
} card_info_t;

int32_t init_card_manager(void *arg);
int32_t card_manager_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t card_manager_update_profiles_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t card_update_profile_info(judge_term_e bootstrap_flag);
int32_t card_set_opr_profile_apn(void);
int32_t card_force_enable_provisoning_profile(void);
int32_t card_force_enable_provisoning_profile_update(void);
int32_t card_manager_install_profile_ok(void);
int32_t card_get_avariable_profile_num(int32_t *avariable_num);
int32_t card_switch_type(cJSON *switchparams);

#endif // __CARD_MANAGER_H__
