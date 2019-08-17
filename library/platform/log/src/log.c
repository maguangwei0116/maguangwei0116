
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

int32_t file_fd = -1;

int32_t log_file()
{
    file_fd = open(LOG_NAME, O_RDWR|O_CREAT|O_APPEND, 0666);
    if(file_fd != -1) {
    }
    return 0;
}

int32_t write_log_fun(const int8_t *msg, ...)
{
    int32_t size = 0;
    int8_t content[200] = {0};
    int8_t final[400] = {0};   //
    time_t  time_write;
    struct tm tm_Log;

    size = log_file_size();
    if (size > LOG_FILE_SIZE) {
        clear_file();
    }
    va_list vl_list;
    va_start(vl_list, msg);

    vsnprintf((char *)content, sizeof(content), (const char *)msg, vl_list);   //
    va_end(vl_list);

    time_write = time(NULL);        //
    localtime_r(&time_write, &tm_Log);
    strftime((char *)final, sizeof(final), "[%Y-%m-%d %H:%M:%S] ", &tm_Log);

    strncat((char *)final, (const char *)content, rt_os_strlen(content));
    write(file_fd, final, rt_os_strlen(final));
    return 0;
}

int32_t write_log(const int8_t *msg, ...)
{
    int32_t size = 0;
    int8_t final[400] = {0};   //
    va_list vl_list;
    int8_t content[200] = {0};

    size = log_file_size();
    if (size > LOG_FILE_SIZE * 1024 * 1024) {
        clear_file();
    }
    va_start(vl_list, msg);
    vsprintf((char *)content, (const char *)msg, vl_list);   //
    va_end(vl_list);

    strncat((char *)final, (const char *)content, rt_os_strlen(content));
    write(file_fd, final, rt_os_strlen(final));
    return 0;
}

int32_t log_file_size(void)
{
    struct stat statbuf;
    int32_t size;

    stat(LOG_NAME, &statbuf);
    size = statbuf.st_size;
    return size;
}

int32_t clear_file(void)
{
    int32_t ret = open(LOG_NAME, O_WRONLY | O_TRUNC);
    if (ret == -1) {
        MSG_WARN("clear_file open file is fail!\n");
        return -1;
    }
    close(ret);
    log_file();
    return 0;
}

void close_file(void)
{
    close(file_fd);
}