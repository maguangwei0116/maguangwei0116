
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_timer.h
 * Date        : 2019.08.27
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __RT_TIMER_H__
#define __RT_TIMER_H__

#include "rt_type.h"

int32_t init_timer(void *arg);
int32_t register_timer(int sec, int usec, void (*action)());
uint32_t rt_os_alarm(uint32_t seconds);

#endif // __RT_TIMER_H__
