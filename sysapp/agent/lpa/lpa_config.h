/*****************************************************************************
Update  : 2019-08-20 20:24:14
Description:
                        DO NOT MODIFY THIS FILE!
                          IT IS AUTO GENERATED!
*****************************************************************************/
#ifndef __LPA_CONFIG_H__
#define __LPA_CONFIG_H__

#include "lpa_error_codes.h"
#include <string.h>
#include <stdio.h>

/**************************************************************************************************
                                    Configure LOGGING
**************************************************************************************************/
#define RT_PRINTF               printf
#define __FILENAME__            (strrchr("/"__FILE__, '/') + 1)

#define MSG_ERR(format, ...)    RT_PRINTF("ERR[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   RT_PRINTF("WARN[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)    RT_PRINTF("DBG[%s-%d %s] "format,  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)   RT_PRINTF("INFO[%s-%d %s] "format, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DUMP_ARRAY(tag, array, len)
#define MSG_INFO_ARRAY(tag, array, len)
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
        MSG_ERR("CHECK FAILED!\n");    \
        ret = ret_val;                          \
        goto goto_label;                        \
    }                                           \
} while(0)

/**************************************************************************************************
                                    Configure TLS
**************************************************************************************************/
//#define TLS_VERIFY_CERT                         1
//#define TLS_VERIFY_CERT_9_AS_OK                 1
#define TLS_CERT_PATH                           "/data/ca-chain.pem"

#endif  //__LPA_CONFIG_H__
