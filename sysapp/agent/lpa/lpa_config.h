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
#ifdef CFG_LPA_LOG_ON

#include "log.h"

#define MSG_ERR(format, ...)                MSG_PRINTF(LOG_ERR, "[%s] "format, __FUNCTION__, ##__VA_ARGS__)  
#define MSG_WARN(format, ...)               MSG_PRINTF(LOG_WARN, "[%s] "format, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)                MSG_PRINTF(LOG_DBG, "[%s] "format, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)               MSG_PRINTF(LOG_INFO, "[%s] "format, __FUNCTION__, ##__VA_ARGS__)  
#define MSG_DUMP_ARRAY(tag, array, len)     MSG_INFO_ARRAY(tag, array, len)

#else

#define MSG_ERR(format, ...)                do {} while(0)                 
#define MSG_WARN(format, ...)               do {} while(0)                
#define MSG_DBG(format, ...)                do {} while(0)                
#define MSG_INFO(format, ...)               do {} while(0)              
#define MSG_DUMP_ARRAY(tag, array, len)     do {} while(0)   

#endif

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

#endif  //__LPA_CONFIG_H__
