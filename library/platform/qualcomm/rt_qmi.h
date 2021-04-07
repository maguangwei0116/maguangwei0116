
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_qmi.h
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __RT_QMI_H__
#define __RT_QMI_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rt_os.h"
#include "rt_type.h"

#define MIN_ICCID_LEN           20
#define MIN_IMSI_LEN            15
#define MIN_IMEI_LEN            15
#define MIN_MODEL_LEN           16
#define MIN_NETWORK_TYPE_LEN    8

typedef enum RT_QMI_OPERATING_MODE {
    RT_QMI_OP_MODE_ONLINE               = 0,
    RT_QMI_OP_MODE_LOW_POWER            = 1,
    RT_QMI_OP_MODE_FACTORY              = 2,
    RT_QMI_OP_MODE_OFFLINE              = 3,
    RT_QMI_OP_MODE_RESETTING            = 4,
    RT_QMI_OP_MODE_SHUTTING_DOWN        = 5,
    RT_QMI_OP_MODE_PERSISTENT_LOW_POWER = 6,
    RT_QMI_OP_MODE_ONLY_LOW_POWER       = 7,
    RT_QMI_OP_MODE_NET_TEST_GW          = 8    
} rt_qmi_operating_mode_e;

int32_t rt_qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel);
int32_t rt_qmi_close_channel(uint8_t channel);
int32_t rt_qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel);

int32_t rt_qmi_send_apdu_vuicc(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel);
int32_t rt_qmi_close_channel_vuicc(uint8_t channel);
int32_t rt_qmi_open_channel_vuicc(const uint8_t *aid, uint16_t aid_len, uint8_t *channel);

int32_t rt_qmi_get_register_state(int32_t *register_state);
int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc);
int32_t rt_qmi_get_current_iccid(char *iccid, int32_t size);
int32_t rt_qmi_get_current_cpin_state(char *cpin);
int32_t rt_qmi_get_current_imsi(char *imsi, int32_t size);
int32_t rt_qmi_get_signal(int32_t *strength);
int32_t rt_qmi_get_signal_level(int32_t *level);
int32_t rt_qmi_get_imei(char *imei, int32_t size);
int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type, int8_t pdp_type, const char *apn, const char *mcc_mnc);
int32_t rt_qmi_get_model(char *model, int32_t size);
int32_t rt_qmi_get_network_type(char *network_type, int32_t size);
int32_t rt_qmi_get_operating_mode(rt_qmi_operating_mode_e *mode);
int32_t rt_qmi_init(void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __RT_QMI_H__ */

