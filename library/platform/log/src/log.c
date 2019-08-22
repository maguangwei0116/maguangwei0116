
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : log.c
 * Date        : 2019.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "log.h"

#define LOG_NAME            "/data/redtea/rt_log"
#define LOG_FILE_SIZE       1024

static int32_t g_file_fd = -1;
static int32_t g_log_printf_type = LOG_PRINTF_TERMINAL;

int32_t log_file(void)
{
    g_file_fd = open(LOG_NAME, O_RDWR|O_CREAT|O_APPEND, 0666);
    if(g_file_fd != -1) {
    }
    return 0;
}

static int32_t log_file_size(void)
{
    struct stat statbuf;
    int32_t size;

    stat(LOG_NAME, &statbuf);
    size = statbuf.st_size;
    return size;
}

static int32_t clear_file(void)
{
    int32_t ret = open(LOG_NAME, O_WRONLY | O_TRUNC);
    if (ret == -1) {
        MSG_PRINTF(LOG_WARN, "clear_file open file is fail!\n");
        return -1;
    }
    close(ret);
    log_file();
    return 0;
}

static void close_file(void)
{
    close(g_file_fd);
}

static void printf_log(int8_t *data)
{
    int32_t size = 0;
    if (g_log_printf_type == LOG_PRINTF_TERMINAL) {
        printf("%s", data);
    } else {
        size = log_file_size();
        if (size > LOG_FILE_SIZE) {
            clear_file();
        }
        write(g_file_fd, data, rt_os_strlen(data));
    }
}

int32_t write_log_fun(log_leve_e leve, log_leve_flag_e leve_flag, const int8_t *msg, ...)
{
    int8_t content[200] = {0};
    int8_t final[400] = {0};
    time_t  time_write;
    struct tm tm_Log;

    va_list vl_list;
    va_start(vl_list, msg);
    vsnprintf((int8_t *)content, sizeof(content), (const int8_t *)msg, vl_list);   //
    va_end(vl_list);
    if (leve_flag == LOG_HAVE_LEVE_PRINTF) {
        time_write = time(NULL);        //
        localtime_r(&time_write, &tm_Log);
        strftime((int8_t *)final, sizeof(final), "[%Y-%m-%d %H:%M:%S] ", &tm_Log);
        strncat((int8_t *)final, (const int8_t *)content, rt_os_strlen(content));
        printf_log(final);
    } else {
        printf_log(content);
    }
    return 0;
}

