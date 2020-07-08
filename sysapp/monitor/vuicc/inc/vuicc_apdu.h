
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : vuicc_callback.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __VUICC_APDU_H__
#define __VUICC_APDU_H__

#include <stdio.h>
#include <stdbool.h>
#include "file.h"
#include "rt_os.h"
#include "rt_type.h"

int32_t init_vuicc(void *arg, int32_t *vuicc_mode);
void init_trigger(uint8_t uicc_switch);
int32_t vuicc_lpa_cmd(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len);

#endif  // __VUICC_APDU_H__
