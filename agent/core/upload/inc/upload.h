/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : upload.h
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __UPLOAD_H__
#define __UPLOAD_H__

#include <stdint.h>
#include "rt_type.h"
#include "http.h"
#include "cJSON.h"

#define MAX_OTI_URL_LEN            100

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do{                 \
   snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
}while(0)

typedef enum UP_COMMAND {
    COMMAND_INIT = 0,
    REGISTER_PUSH_ID,
    PROFILE_RELOADED,
    MCC_CHANGED,
    DEVICE_REBOOTED,
    INTERNET_RECONNECTED,
    ON_PUSH_ACTIVATION_CODE,
    ON_ENABLE_PROFILE,
    ON_DISABLE_PROFILE,
    ON_DELETE_PROFILE,
    ON_EUICC_LOOKUP,
    ON_SWITCH_COS,
    ON_LOG,
    ON_UPGRADE,
    ON_CONFIG,
    UN_KNOWN
} up_command_e;


typedef struct MESSAGE_QUEUE msg_que_t;
typedef struct MSG msg_t;
int32_t init_upload(void);
void   upload_send_reconnected(void *buff);
void   upload_send_profile_reload(void);
void   upload_set_values(up_command_e cmd,void *info_buf);
void msg_upload_data(const int8_t *tranid, up_command_e cmd, int32_t state, void *info1);

#endif // _UPLOAD_H_
