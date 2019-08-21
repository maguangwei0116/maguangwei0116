
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

#include <qmi-framework/qmi_client.h>
#include "rt_os.h"
#include "rt_type.h"

#define RT_DMS_V01_IDL_MINOR_VERS  0x39

#define rt_qmi_send_apdu(data, data_len, rsp, rsp_len, channel)    qmi_send_apdu(data, data_len, rsp, rsp_len, channel)
#define rt_qmi_close_channel(channel)    qmi_close_channel(channel)
#define rt_qmi_open_channel(aid, aid_len, channel)     qmi_open_channel(aid, aid_len, channel)

int32_t rt_qmi_get_register_state(int32_t *register_state);
int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc);
int32_t rt_qmi_get_current_iccid(uint8_t *iccid);
int32_t rt_qmi_get_current_imsi(uint8_t *imsi);
int32_t rt_qmi_get_signal(int32_t *strength);
int32_t rt_qmi_get_imei(uint8_t *imei);
int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type,int8_t *apn, int8_t pdp_type);
int32_t rt_qmi_init(void);

#endif   // __RT_QMI_H__
