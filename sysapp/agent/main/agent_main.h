
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_main.h
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_MAIN_H__
#define __AGENT_MAIN_H__

#include <stdio.h>
#include <stdint.h>
#include <jni.h>

struct arguments {
    JNIEnv *env;
    jclass type;
};

int32_t jni_get_imei(uint8_t *imei);
int32_t jni_get_mcc(uint16_t *mcc);
int32_t jni_get_mnc(uint16_t *mnc);
int32_t jni_get_current_iccid(uint8_t *iccid);
int32_t jni_get_current_imsi(uint8_t *imsi);
int32_t jni_get_signal_level(int32_t *level);
int32_t jni_get_signal_dbm(int32_t *dbm);
int32_t jni_get_network_type(uint8_t *type);
int32_t jni_set_apn(uint8_t *apn, uint8_t *mcc_mnc);

int32_t jni_open_channel(uint8_t *channel);
int32_t jni_close_channel(uint8_t *channel);
int32_t jni_transmit_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel);

int32_t jni_command_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len);

int32_t jni_open_vuicc_channel(uint8_t *channel);
int32_t jni_close_vuicc_channel(uint8_t *channel);
int32_t jni_transmit_vuicc_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel);

extern int32_t agent_main(void *arg,void *log);

#endif // __AGENT_MAIN_H__
