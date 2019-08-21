
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


int32_t write_log_fun(log_leve_e leve, const int8_t *msg, ...);

#define MSG_PRINTF(LOG_LEVE, format,...)    write_log_fun(LOG_LEVE, "[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#define RT_PRINTF               printf
#define __FILENAME__            (strrchr("/"__FILE__, '/') + 1)

#define INNER_DUMP_ARRAY(tag, array, len)       \
    do {                                        \
        uint8_t *_p_ = (uint8_t *)array;        \
        uint16_t i;                             \
        RT_PRINTF("%s", tag);                   \
        for (i = 0; i < len; i++) {             \
            RT_PRINTF("%02X", _p_[i]);          \
        }                                       \
        RT_PRINTF("\n");                        \
    } while(0)

#define MSG_INFO_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)

#endif // __LOG_H__
