
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

#ifndef __USRDATA_H__
#define __USRDATA_H__

#include "stdint.h"

typedef enum REDTEA_READY_NETWORK_DETECT_CMD {
    DEVICE_KEY_SUCESS       = 0,
    DEVICE_KEY_ERROR,
    NETWORK_DETECT_ENABLE,
    NETWORK_DETECT_DISABLE,
} redtea_ready_network_detect_cmd_e;

typedef enum INSPECT_STRATEGY {
    RT_BOOT_CHECK           = 0,
    RT_RUN_CHECK            = 1,
} inspect_strategy_e;

#if (CFG_OPEN_MODULE)
#define RT_DATA_PATH                        "/data/redtea/"
#elif (CFG_STANDARD_MODULE)
#define RT_DATA_PATH                        "/usrdata/redtea/"
#endif
#define RUN_CONFIG_FILE                     "rt_run_config"

#define MAX_FILE_HASH_LEN                   64
#define MAX_FILE_HASH_BYTE_LEN              32
#define DEVICE_KEY_SIZE                     32
#define DEVICE_KEY_LEN                      16
#define CHECK_STRATEGY_HEAD                 16

#define RT_CARD_TYPE_LEN                    64
#define RT_LAST_EID_LEN                     64
#define RT_DEVICE_KEY_LEN                   128
#define RT_TICKET_SERVER_LEN                256
#define RT_STRATEGY_LIST_LEN                512

#define RT_CARD_TYPE_OFFSET                 0
#define RT_LAST_EID_OFFSET                  RT_CARD_TYPE_OFFSET     + RT_CARD_TYPE_LEN          // 64
#define RT_DEVICE_KEY_OFFSET                RT_LAST_EID_OFFSET      + RT_LAST_EID_LEN           // 64 + 64
#define RT_TICKET_SERVER_OFFSET             RT_DEVICE_KEY_OFFSET    + RT_DEVICE_KEY_LEN         // 64 + 64 + 128
#define RT_STRATEGY_LIST_OFFSET             RT_TICKET_SERVER_OFFSET + RT_TICKET_SERVER_LEN      // 64 + 64 + 128 + 256
#define RT_APN_LIST_OFFSET                  RT_STRATEGY_LIST_OFFSET + RT_STRATEGY_LIST_LEN      // 64 + 64 + 128 + 256 + 512


int32_t init_run_config();

int32_t rt_write_default_strategy();

int32_t rt_write_card_type(int32_t offset, uint8_t *card_type, int32_t len);
int32_t rt_read_card_type(int32_t offset, uint8_t *card_type, int32_t len);

int32_t rt_write_eid(int32_t offset, uint8_t *eid, int32_t len);
int32_t rt_read_eid(int32_t offset, uint8_t *eid, int32_t len);

int32_t rt_write_devicekey(int32_t offset, const uint8_t *devicekey, int32_t len);
int32_t rt_read_devicekey(int32_t offset, uint8_t *devicekey, int32_t len);

int32_t rt_write_ticket(int32_t offset, uint8_t *ticket, int32_t len);
int32_t rt_read_ticket(int32_t offset, uint8_t *ticket, int32_t len);

int32_t rt_write_strategy(int32_t offset, uint8_t *strategy, int32_t len);
int32_t rt_read_strategy(int32_t offset, uint8_t *strategy, int32_t len);

int32_t rt_write_apnlist(int32_t offset, uint8_t *apnlist, int32_t len);
int32_t rt_read_apnlist(int32_t offset, uint8_t *apnlist, int32_t len);
int32_t rt_truncate_apnlist(int32_t offset);

void rt_inspect_monitor_strategy(inspect_strategy_e type);

#endif