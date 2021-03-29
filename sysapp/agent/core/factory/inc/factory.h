/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_factory.h
 * Date        : 2021.03.03
 * Note        :
 * Description : The factory file is used to factory mode function

 *******************************************************************************/
#ifndef __RT_FACTORY_H__
#define __RT_FACTORY_H__

#ifdef CFG_FACTORY_MODE_ON

#include "stdint.h"

typedef enum FACTORY_MODE {
    FACTORY_DISABLE                 = 0,
    FACTORY_ENABLE                  = 1,
} factory_mode_e;

int32_t  init_factory(void *arg);
int32_t  factory_mode_operation(const char* active_code);
factory_mode_e factory_get_mode(void);
uint32_t factory_get_profile_index(void);

#endif  // CFG_FACTORY_MODE_ON

#endif  // __RT_FACTORY_H__

