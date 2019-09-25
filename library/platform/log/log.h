
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
    LOG_UNKNOW  = -1,
    LOG_NONE    = 0,
    LOG_ERR     = 1,
    LOG_WARN    = 2,
    LOG_DBG     = 3,
    LOG_INFO    = 4,
    LOG_ALL     = 5,     
} log_level_e;

typedef enum LOG_MODE {
    LOG_PRINTF_TERMINAL = 0,
    LOG_PRINTF_FILE
} log_mode_e;

typedef enum LOG_LEVEL_FLAG {
    LOG_HAVE_LEVEL_PRINTF = 0,
    LOG_NO_LEVEL_PRINTF
} log_level_flag_e;

extern int32_t init_log_file(void *arg);
extern int32_t log_set_param(log_mode_e mode, log_level_e level, unsigned int max_size);
extern int32_t log_print(log_level_e level, log_level_flag_e level_flag, const char *msg, ...);
extern int32_t log_hexdump(const char *file, int32_t line, const char *title, const void *data, unsigned int len);
extern int32_t log_file_copy_out(const char* file_in, const char* file_out, log_level_e min_level);

#define __FILENAME__            (strrchr("/"__FILE__, '/') + 1)
#define ARRAY_PRINTF(tag, array, len)                                            \
    do {                                                                         \
        uint8_t *_p_ = (uint8_t *)array;                                         \
        uint16_t i;                                                              \
        log_print(LOG_DBG, LOG_NO_LEVEL_PRINTF,"%s", tag);                   \
        for (i = 0; i < len; i++) {                                              \
            log_print(LOG_DBG, LOG_NO_LEVEL_PRINTF, "%02X", _p_[i]);         \
        }                                                                        \
        log_print(LOG_DBG, LOG_NO_LEVEL_PRINTF, "\n");                       \
    } while(0)

#define MSG_INFO_ARRAY(tag, array, len)         ARRAY_PRINTF(tag, array, len)
#define MSG_PRINTF(LOG_LEVEL, format,...)       log_print(LOG_LEVEL, LOG_HAVE_LEVEL_PRINTF,"[ %d %s ] "format, __LINE__, __FILENAME__, ##__VA_ARGS__)
#define MSG_HEXDUMP(title, data, len)           log_hexdump(__FILE__, __LINE__, title, data, len)

#endif // __LOG_H__
