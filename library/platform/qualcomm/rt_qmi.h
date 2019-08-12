
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

#include "rt_os.h"
#include "rt_type.h"
#include <qmi-framework/qmi_client.h>

#if MANUFACTURE == MANUFACTURE_ZTE || MANUFACTURE ==  MANUFACTURE_GSW    // ZTE
#define RT_DMS_V01_IDL_MINOR_VERS  0x37
#else     // others
#define RT_DMS_V01_IDL_MINOR_VERS  0x39
#endif

int32_t rt_qmi_get_register_state(int32_t *register_state);
int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc);
int32_t rt_qmi_get_current_iccid(uint8_t *iccid);
int32_t rt_qmi_get_current_imsi(uint8_t *imsi);
int32_t rt_qmi_get_signal(int32_t *strength);
int32_t rt_qmi_get_imei(uint8_t *imei);
int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type,int8_t *apn, int8_t pdp_type);
int32_t rt_qmi_init(void);

#endif   // __RT_QMI_H__
