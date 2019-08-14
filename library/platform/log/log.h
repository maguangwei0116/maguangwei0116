
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
#endif // __LOG_H__
