#ifndef __LOG__
#define __LOG__

#include <stdio.h>
#include "types.h"
#include <string.h>

int softsim_printf(int leve, int flag, const char *msg, ...);

/*******************************************
* Log configure
*******************************************/
typedef enum LOG_LEVE {
    LOG_ERR = 0,
    LOG_WARN,
    LOG_DBG,
    LOG_INFO,
    LOG_ALL,
} log_leve_e;

// debug level, the higher the more info to print, can use the vsim bug leve.
#define DEBUG_LEVEL LOG_DBG

// to print apdu or not
#define PRINT_APDU_RESP
/******************************************/
// normal
#ifndef LOG_TAG
#define LOG_TAG "eSIM"
#endif

#define INNER_DUMP_ARRAY(array, len, tag, function)        \
    do {                                                   \
        uint8_t *_p_ = (uint8_t *)array;                   \
        uint16_t i;                                        \
        softsim_printf(LOG_DBG, 1, "%s", tag);             \
        for (i = 0; i < len; i++) {                        \
            softsim_printf(LOG_DBG, 1, "%02X", _p_[i]);    \
        }                                                  \
        softsim_printf(LOG_DBG, 1, "\n");                  \
    } while(0)

// into card
#ifndef LOG_TAG_IN
#define LOG_TAG_IN  "eSIM RSQ:"
#endif

// out card
#ifndef LOG_TAG_OUT
#define LOG_TAG_OUT "eSIM RSP:"
#endif

#define __FILENAME__            (strrchr("/"__FILE__, '/') + 1)

#if DEBUG_LEVEL >= LOG_ALL
#define LOGA(format,...) softsim_printf(LOG_ALL, 0, "[ %d %s ] "format"\n", __LINE__, __FILENAME__, ##__VA_ARGS__);
#else
#define LOGD(format,...)
#endif

#if DEBUG_LEVEL >= LOG_DBG
#define LOGD(format,...) softsim_printf(LOG_DBG, 0, "[ %d %s ] "format"\n", __LINE__, __FILENAME__, ##__VA_ARGS__);
#else
#define LOGD(format,...)
#endif

#if DEBUG_LEVEL >= LOG_INFO
#define LOGI(format,...) softsim_printf(LOG_INFO, 0, "[ %d %s ] "format"\n", __LINE__, __FILENAME__, ##__VA_ARGS__);
#else
#define LOGI(format,...)
#endif

#if DEBUG_LEVEL >= LOG_ERR
#define LOGE(format,...) softsim_printf(LOG_ERR, 0, "[ %d %s ] "format"\n", __LINE__, __FILENAME__, ##__VA_ARGS__);
#else
#define LOGE(format,...)
#endif

#define LOG_DEBUG_ARRAY(array, len, tag, fun)         INNER_DUMP_ARRAY(array, len, tag, fun)
#define LOG_INFO_ARRAY(array, len, tag, fun)          INNER_DUMP_ARRAY(array, len, tag, fun)

#define LOGA_FUNC()     //LOGA("%s", __FUNCTION__)

#endif  // __LOG__
