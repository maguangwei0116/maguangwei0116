
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_type.h
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <stdint.h>
#include <errno.h>

typedef enum RT_BOOL{
    RT_FALSE = 0,
    RT_TRUE
} rt_bool;

#define RT_ERROR                   -1
#define RT_SUCCESS                 0

#define RT_CHECK_ERR(process, result) if((process) == result){ MSG_WARN("%s error result:%s\n", #process, strerror(errno))  goto end;}
#define RT_CHECK_NEQ(process, result) if((process) != result){ MSG_WARN("%s error result:%s\n", #process, strerror(errno))  goto end;}
#define RT_CHECK_LESE(process, result) if((process) <= result){ MSG_WARN("%s error result:%s\n", #process, strerror(errno))  goto end;}
#define RT_CHECK_LES(process, result) if((process) < result){ MSG_WARN("%s error result:%s\n", #process, strerror(errno))  goto end;}


#if VERSION_TYPE == DEBUG
#define TRACE_PRINT(format,...) \
    do {\
        printf(format,##__VA_ARGS__); \
    } while(0);

#define TRACE_ERROR(format,...) \
    do {\
        printf(format,##__VA_ARGS__); \
    } while(0);

#elif VERSION_TYPE == RELEASE
#define TRACE_PRINT(format,...) \
    do {\
        //write_log_fun((int8_t *)format,##__VA_ARGS__); \
    } while(0);

#define TRACE_ERROR(format,...) \
    do {\
        //write_log_fun((int8_t *)format,##__VA_ARGS__); \
    } while(0);
#endif



#define LOG_ERR                 0x01
#define LOG_WARN                0x02
#define LOG_DBG                 0x03
#define LOG_INFO                0x04

#define LOG_LEVEL               LOG_ERR
/******************************************************************************
 * 以下为log level的定义
 * MSG_INFO:开放协议交互消息的log
 * MSG_DBG：输出必要的调试信息
 * MSG_WARN：输出警告信息
 * MSG_ERR：输出错误信息
 *******************************************************************************/

// LOG_INFO
#if  LOG_LEVEL > LOG_DBG
#define MSG_ERR(format, ...)    TRACE_PRINT("ERR[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   TRACE_PRINT("WARN[ %d %s] "format,__LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)    TRACE_PRINT("DBG[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)    TRACE_PRINT("INFO[%d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)

// LOG_DBG
#elif  LOG_LEVEL > LOG_WARN
#define MSG_ERR(format, ...)    TRACE_PRINT("ERR[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   TRACE_PRINT("WARN[ %d %s] "format,__LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)    TRACE_PRINT("DBG[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_INFO(format, ...)

// LOG_WARN
#elif  LOG_LEVEL > LOG_ERR
#define MSG_ERR(format, ...)    TRACE_PRINT("ERR[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)   TRACE_PRINT("WARN[ %d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_DBG(format, ...)
#define MSG_INFO(format, ...)

// LOG_ERR
#elif LOG_LEVEL > LOG_NONE
#define MSG_ERR(format, ...)    TRACE_PRINT("ERR[%d %s] "format, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define MSG_WARN(format, ...)
#define MSG_DBG(format, ...)
#define MSG_INFO(format, ...)

#endif

#endif // __TYPE_H__
