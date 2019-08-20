
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : log.h
 * Date        : 2019.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

typedef enum LOG_LEVE {
    LOG_INFO = 0,
    LOG_ERR,
    LOG_WARN,
    LOG_DBG
} log_leve_e;


int32_t write_log_fun(log_leve_e leve, const int8_t *msg, ...);

#define MSG_PRINTF(LOG_LEVE, format,...)    write_log_fun(LOG_LEVE, "[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif // __LOG_H__
