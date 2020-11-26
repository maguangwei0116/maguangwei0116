
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_quectel.h
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_QUEUE_H__
#define __AGENT_QUEUE_H__

#include "rt_type.h"
#include "device_info.h"
#include "card_manager.h"
#include "upgrade.h"
#include "config.h"

/* turn on agent msg queue log info */
#define AGENT_MSG_DEBUG     1

typedef enum AGENT_MSG_ID {
    MSG_ID_CARD_MANAGER = 0,
    MSG_ID_LOG_MANAGER,
    MSG_ID_BOOT_STRAP,
    MSG_ID_OTA_UPGRADE,
    MSG_ID_PERSONLLISE,
    MSG_ID_NETWORK_DECTION,  // 5
    MSG_ID_REMOTE_CONFIG,
    MSG_ID_BROAD_CAST_NETWORK,
    MSG_ID_MQTT,
    MSG_ID_IDLE,
    MSG_ID_DETECT_NETWORK,  // 10
} msg_id_e;

typedef enum MSG_MODE {
    MSG_FROM_MQTT = 0,      // public for all module
    MSG_CARD_SETTING_KEY,
    MSG_CARD_SETTING_PROFILE,
    MSG_CARD_SETTING_CERTIFICATE,
    MSG_CARD_ENABLE_EXIST_CARD,
    MSG_START_NETWORK_DETECTION, // 5
    MSG_NETWORK_CONNECTED,
    MSG_NETWORK_DISCONNECTED,
    MSG_BOOTSTRAP_DISCONNECTED,
    MSG_MQTT_SUBSCRIBE_EID,
    MSG_BOOTSTRAP_SELECT_CARD,  // 10
    MSG_MQTT_CONNECTED,
    MSG_MQTT_DISCONNECTED,
    MSG_BOOTSTRAP_START_TIMER,
    MSG_CARD_UPDATE,
    MSG_CARD_DISABLE_EXIST_CARD,
    MSG_CARD_UPDATE_SEED,
    MSG_SYNC_DOWNSTREAM_INFO,
    MSG_SWITCH_CARD,
} msg_mode_e;

typedef struct TARGET_VERSION {
    char                name[MAX_FILE_NAME_LEN + 1];
    char                version[MAX_VERSION_NAME_LEN + 1];
    char                chipModel[MAX_CHIP_MODEL_LEN + 1];
} target_version_t;

typedef struct TARGET_VERSIONS {
    target_version_t    versions[TARGET_TYPE_MAX];
} target_versions_t;

typedef struct PUBLIC_VALUE_LIST {
    config_info_t *     config_info;
    devicde_info_t *    device_info;
    card_info_t *       card_info;
    const char *        push_channel;
    target_versions_t * version_info;
    int32_t *           profile_damaged;
} public_value_list_t;

int32_t init_queue(void *arg);

#if (AGENT_MSG_DEBUG)
#include "log.h"
extern const char * g_msg_id_e[];
extern const char * g_msg_mode_e[];
int32_t _msg_send_agent_queue(int32_t msgid, int32_t mode, void *buffer, int32_t len);
#define msg_send_agent_queue(id, mode, buffer, len)\
    ({\
        MSG_PRINTF(LOG_TRACE, "=>send agent queue: [%s], mode: [%s]\r\n", g_msg_id_e[(id)], g_msg_mode_e[(mode)]);\
        int32_t i_ret = _msg_send_agent_queue(id, mode, buffer, len);\
        i_ret;\
    })
#else
int32_t msg_send_agent_queue(int32_t msgid, int32_t mode, void *buffer, int32_t len);
#endif

int32_t msg_send_upload_queue(void *buffer, int32_t len);

#endif // __AGENT_QUEUE_H__

