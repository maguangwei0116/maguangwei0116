
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
    LOG_ERR = 0,
    LOG_WARN,
    LOG_DBG,
    LOG_INFO
} log_leve_e;

typedef enum LOG_MODE {
    LOG_PRINTF_TERMINAL = 0,
    LOG_PRINTF_FILE
} log_mode_e;

typedef enum LOG_LEVE_FLAG {
    LOG_HAVE_LEVE_PRINTF = 0,
    LOG_NO_LEVE_PRINTF
} log_leve_flag_e;

int32_t write_log_fun(log_leve_e leve, log_leve_flag_e leve_flag, const int8_t *msg, ...);

#define __FILENAME__            (strrchr("/"__FILE__, '/') + 1)
#define ARRAY_PRINTF(tag, array, len)                                            \
    do {                                                                         \
        uint8_t *_p_ = (uint8_t *)array;                                         \
        uint16_t i;                                                              \
        write_log_fun(LOG_DBG, LOG_NO_LEVE_PRINTF,"%s", tag);                    \
        for (i = 0; i < len; i++) {                                              \
            write_log_fun(LOG_DBG, LOG_NO_LEVE_PRINTF, "%02X", _p_[i]);          \
        }                                                                        \
        write_log_fun(LOG_DBG, LOG_NO_LEVE_PRINTF, "\n");                        \
    } while(0)

#define MSG_INFO_ARRAY(tag, array, len)         ARRAY_PRINTF(tag, array, len)
#define MSG_PRINTF(LOG_LEVE, format,...)        write_log_fun(LOG_LEVE, LOG_HAVE_LEVE_PRINTF,"[ %d %s ] "format, __LINE__, __FILENAME__, ##__VA_ARGS__)

#endif // __LOG_H__
