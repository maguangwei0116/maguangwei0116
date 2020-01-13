
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "rt_type.h"

extern int32_t  init_timer(void *arg);
extern int32_t  register_timer(int sec, int usec, void (*action)(void));
extern uint32_t rt_os_alarm(uint32_t seconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RT_TIMER_H__ */

