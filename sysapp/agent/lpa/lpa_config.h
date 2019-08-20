/*****************************************************************************
Update  : 2019-08-20 20:24:14
Description:
                        DO NOT MODIFY THIS FILE!
                          IT IS AUTO GENERATED!
*****************************************************************************/
#ifndef __LPA_CONFIG_H__
#define __LPA_CONFIG_H__

#include "lpa_error_codes.h"

#define VSIM_VERSION_MAJOR      0
#define VSIM_VERSION_MINOR      9
#define VSIM_VERSION_PATCH      4

#define PROJECT_NAME            "LPA"
#define PROJECT_BUILD_DATE      "2019-08-20"
#define PROJECT_BUILD_TIME      "20:24:14"

#define PLATFORM_INTEL_X86      0x10
#define PLATFORM_QCOM_9X07      0x20
#define PLATFORM_MT2503         0x30
#define PLATFORM                PLATFORM_QCOM_9X07

#define AT                      0x01
#define QMI                     0x02
#define SEND_APDU_METHOD        QMI

#define LOG_NONE                0x00
#define LOG_ERR                 0x01
#define LOG_WARN                0x02
#define LOG_DBG                 0x03
#define LOG_INFO                0x04
#define LOG_LEVEL               LOG_ERR

/**************************************************************************************************
                                    Configure CHECK
**************************************************************************************************/
#define RT_CHECK(x)    {int a = x; if((a) != RT_SUCCESS) {MSG_ERR("CK: %d\n", a); return a;}}
#define RT_CHECK_EQ(x, expt) {int a = x; if((a) != expt) {MSG_ERR("EQ: %d\n", a); return a;}}
#define RT_CHECK_GT(x, expt) {int a = x; if((a) <= expt) {MSG_ERR("GT: %d\n", a); return a;}}
#define RT_CHECK_LT(x, expt) {int a = x; if((a) >= expt) {MSG_ERR("LT: %d\n", a); return a;}}
#define RT_CHECK_GO(cond, ret_val, goto_label)  \
do {                                            \
    if (!(cond)) {                              \
        MSG_ERR("CHECK FAILED!\n");             \
        ret = ret_val;                          \
        goto goto_label;                        \
    }                                           \
} while(0)

/**************************************************************************************************
                                    Configure LOGGING
**************************************************************************************************/
#include <string.h>
#include <stdio.h>

#define LPA_LOG_FILE_NAME       "/data/lpa_log.txt"

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

// LOG_INFO
#if  LOG_LEVEL > LOG_DBG
#define MSG_ERR(format, ...)    RT_PRINTF("ERR[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   RT_PRINTF("WARN[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)    RT_PRINTF("DBG[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)   RT_PRINTF("INFO[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DUMP_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)

// LOG_DBG
#elif  LOG_LEVEL > LOG_WARN
#define MSG_ERR(format, ...)    RT_PRINTF("ERR[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   RT_PRINTF("WARN[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)    RT_PRINTF("DBG[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)
#define MSG_DUMP_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)

// LOG_WARN
#elif  LOG_LEVEL > LOG_ERR
#define MSG_ERR(format, ...)    RT_PRINTF("ERR[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   RT_PRINTF("WARN[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)
#define MSG_INFO(format, ...)
#define MSG_DUMP_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)

// LOG_ERR
#elif LOG_LEVEL > LOG_NONE
#define MSG_ERR(format, ...)    RT_PRINTF("ERR[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)
#define MSG_DBG(format, ...)
#define MSG_INFO(format, ...)
#define MSG_DUMP_ARRAY(tag, array, len)         INNER_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)

// LOG_NONE
#elif   LOG_LEVEL == LOG_NONE
#define MSG_ERR(format, ...)
#define MSG_WARN(format, ...)
#define MSG_DBG(format, ...)
#define MSG_INFO(format, ...)
#define MSG_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)
#else
#error "Please figure out LOG_LEVEL"
#endif

/**************************************************************************************************
                                    Configure AT CMD
**************************************************************************************************/
#define LPA_AT_PORT                             "/dev/smd8"
#define LPA_AT_TIME_OUT                         10000

// +CSIM: len(4~514),"data(max to 255 bytes)|status"\r\nOK\r\n\0
#define LPA_AT_MAX_RD_BUF                       538

// AT+CSIM=len(1~520),"CLA|INC|P1|P2|Lc|data(max to 255 bytes)"\r\n\0
#define LPA_AT_MAX_WR_BUF                       537
#define LPA_AT_BLOCK_BUF                        538

#define LPA_AT_CMD_MAX_LEN                      510  // 0xFF * 2

/**************************************************************************************************
                                    Configure TLS
**************************************************************************************************/
//#define TLS_VERIFY_CERT                         1
//#define TLS_VERIFY_CERT_9_AS_OK                 1
#define TLS_CERT_PATH                           "/data/ca-chain.pem"

#endif  //__LPA_CONFIG_H__
