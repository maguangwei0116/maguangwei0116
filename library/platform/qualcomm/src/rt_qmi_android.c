
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_qmi_adnroid.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include "rt_qmi.h"

int32_t rt_qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
}

int32_t rt_qmi_close_channel(uint8_t channel)
{
}

int32_t rt_qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel)
{
}

int32_t rt_qmi_get_register_state(int32_t *register_state)
{
}

int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc)
{
}

int32_t rt_qmi_get_current_iccid(uint8_t *iccid)
{
}

int32_t rt_qmi_get_current_imsi(uint8_t *imsi)
{
}

int32_t rt_qmi_get_signal(int32_t *strength)
{
}

int32_t rt_qmi_get_imei(uint8_t *imei)
{
}

int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type,int8_t *apn, int8_t pdp_type)
{
}

int32_t rt_qmi_get_model(uint8_t *model)
{
}

int32_t rt_qmi_get_network_type(uint8_t *network_type)
{
}

int32_t rt_qmi_init(void *arg)
{
}

