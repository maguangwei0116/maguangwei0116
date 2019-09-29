/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_config.h
 * Date        : 2017.09.01
 * Note        :
 * Description : The configuration file is used to configure file function

 *******************************************************************************/
#ifndef __RT_CONFIG_H__
#define __RT_CONFIG_H__

#include "stdint.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

extern char *OTI_ENVIRONMENT_ADDR;
extern char *EMQ_SERVER_ADDR;
extern char *PROXY_SERVER_ADDR;

int32_t init_config(void *arg);

#endif  // __RT_CONFIG_H__
