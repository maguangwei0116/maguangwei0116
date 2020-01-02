
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : card_detection.h
 * Date        : 2019.11.05
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __CARD_DETECTION_H__
#define __CARD_DETECTION_H__

#include "rt_type.h"

int32_t init_card_detection(void *arg);
int32_t card_detection_event(const uint8_t *buf, int32_t len, int32_t mode);
int32_t card_detection_enable(void);
int32_t card_detection_disable(void);

#endif // __CARD_DETECTION_H__

