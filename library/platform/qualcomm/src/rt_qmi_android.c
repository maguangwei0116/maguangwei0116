
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
#include "rt_os.h"
#include "log.h"
#include "../../../../sysapp/agent/main/agent_main.h"

extern struct arguments *args;

int32_t rt_qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    MSG_PRINTF(LOG_DBG, "start callback transmit_apdu()");
    return jni_transmit_apdu(data, data_len, rsp, rsp_len, channel);
}

int32_t rt_qmi_close_channel(uint8_t channel)
{
    MSG_PRINTF(LOG_DBG, "start callback close_channel()");
    return jni_close_channel(channel);

}

int32_t rt_qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel)
{
    MSG_PRINTF(LOG_DBG, "start callback open_channel()");
    return jni_open_channel(channel);
}

int32_t rt_qmi_command_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    MSG_PRINTF(LOG_DBG, "start callback rt_qmi_command_apdu()");
    return jni_command_apdu(data, data_len, rsp, rsp_len);
}

int32_t rt_qmi_get_register_state(int32_t *register_state)
{
}

int32_t rt_qmi_get_mcc(uint16_t *mcc)
{
    if(mcc == NULL){
        return -1;
    }
    return jni_get_mcc(mcc);
}

int32_t rt_qmi_get_mnc(uint16_t *mnc)
{
    if(mnc == NULL){
        return -1;
    }
    return jni_get_mnc(mnc);
}

int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc)
{
    MSG_PRINTF(LOG_DBG, "start callback getMccMnc()");
    int32_t result = rt_qmi_get_mcc(mcc);
    result = rt_qmi_get_mnc(mnc);
    return 0;
}

int32_t rt_qmi_get_current_iccid(uint8_t *iccid)
{
    MSG_PRINTF(LOG_DBG, "start callback getIccid()");
    jni_get_current_iccid(iccid);
}

int32_t rt_qmi_get_current_imsi(uint8_t *imsi)
{
    MSG_PRINTF(LOG_DBG, "start callback getImsi()");
    jni_get_current_imsi(imsi);
}

int32_t rt_qmi_get_signal(int32_t *strength)
{
    return jni_get_signal_dbm(strength);
}

int32_t rt_qmi_get_signal_level(int32_t *level)
{
    return jni_get_signal_level(level);
}

int32_t rt_qmi_get_imei(uint8_t *imei)
{
    MSG_PRINTF(LOG_DBG, "start callback getImei()");
    return jni_get_imei(imei);
}

int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type, int8_t pdp_type, int8_t *apn, int8_t *mcc_mnc)
{
    MSG_PRINTF(LOG_DBG, "start callback rt_qmi_modify_profile()");
    return jni_set_apn(apn, mcc_mnc);
}

int32_t rt_qmi_get_model(uint8_t *model)
{
    rt_os_strcpy((char *)model, "QUECTEL");  // only for test 
}

int32_t rt_qmi_get_network_type(uint8_t *network_type)
{
    return jni_get_network_type(network_type);
}

int32_t rt_qmi_init(void *arg)
{
}

