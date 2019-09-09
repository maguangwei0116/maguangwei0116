
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
#define LOG_FILE_SIZE       1024 * 1024  // 1MB
#define LOG_BUF_SIZE        1024

static int32_t g_file_fd = -1;
static int32_t g_log_printf_mode = LOG_PRINTF_TERMINAL;
static int32_t g_log_level = LOG_INFO;

int32_t log_set_param(log_mode_e mode, log_level_e level)
{
	g_log_printf_mode = mode;
	g_log_level = level;

	return 0;
}

int32_t log_file(void)
{
    g_file_fd = open(LOG_NAME, O_RDWR|O_CREAT|O_APPEND, 0666);
    if(g_file_fd < 0) {
        printf("log file open error\n");
        return -1;
    }
    return 0;
}

static int32_t log_file_size(void)
{
    struct stat statbuf;
    int32_t size;

    if (rt_os_access(LOG_NAME, F_OK)) {
        return 0;
    }

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
    return 0;
}

static void close_file(void)
{
    close(g_file_fd);
    g_file_fd = -1;
}

static void printf_log(int8_t *data)
{
    int32_t size = 0;
    if (g_log_printf_mode == LOG_PRINTF_TERMINAL) {
        printf("%s", data);
    } else {
        size = log_file_size();
        if (size > LOG_FILE_SIZE) {
            clear_file();
            close_file();
        }
        if (g_file_fd < 0) {
        	log_file();
        }
        write(g_file_fd, data, rt_os_strlen(data));
        fsync(g_file_fd);
    }
}

int32_t write_log_fun(log_level_e level, log_level_flag_e level_flag, const char *msg, ...)
{
    char content[LOG_BUF_SIZE] = {0};
    time_t  time_write;
    struct tm tm_Log;
    int32_t len = 0;
    va_list vl_list;

    if (level > g_log_level) {
        return -1;
    }

    if (level_flag == LOG_HAVE_LEVEL_PRINTF) {
        switch(level) {
            case LOG_ERR:
                rt_os_memcpy(content, "ERR ", 4);
            break;
            case LOG_WARN:
                rt_os_memcpy(content, "WARN", 4);
            break;
            case LOG_DBG:
                rt_os_memcpy(content, "DBG ", 4);
            break;
            case LOG_INFO:
                rt_os_memcpy(content, "INFO", 4);
            break;
        }
        len = 4;
        content[4] = 0;
        time_write = time(NULL);        //
        localtime_r(&time_write, &tm_Log);
        strftime((char *)&content[len], LOG_BUF_SIZE, "[%Y-%m-%d %H:%M:%S]", &tm_Log);
        len = rt_os_strlen(&content[len]) + 4;
    }
	
    va_start(vl_list, msg);
    vsnprintf((char *)&content[len], LOG_BUF_SIZE - len, (const char *)msg, vl_list);   //
    va_end(vl_list);
    printf_log(content);
    return 0;
}

void hexdump(const char *file, int32_t line, const char *title, const void *data, unsigned int len)
{
    #if 1
    char str[160], octet[10];
    int ofs, i, k, d;
    const unsigned char *buf = (const unsigned char *)data;
    const char dimm[] = "+------------------------------------------------------------------------------+";

    if (g_log_level < LOG_INFO) {
        return;
    }

    MSG_PRINTF(LOG_INFO, "[%s, %d] %s (%d bytes):\r\n", file, line, title, len);
    MSG_PRINTF(LOG_INFO, "%s\r\n", dimm);
    MSG_PRINTF(LOG_INFO, "| Offset  : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F   0123456789ABCDEF |\r\n");
    MSG_PRINTF(LOG_INFO, "%s\r\n", dimm);

    for (ofs = 0; ofs < (int)len; ofs += 16) {
        d = snprintf( str, sizeof(str), "| %08x: ", ofs );

        for (i = 0; i < 16; i++) {
            if ((i + ofs) < (int)len) {
                snprintf( octet, sizeof(octet), "%02x ", buf[ofs + i] );
            } else {
                snprintf( octet, sizeof(octet), "   " );
            }

            d += snprintf( &str[d], sizeof(str) - d, "%s", octet );
        }
        d += snprintf( &str[d], sizeof(str) - d, "  " );
        k = d;

        for (i = 0; i < 16; i++) {
            if ((i + ofs) < (int)len) {
                str[k++] = (0x20 <= (buf[ofs + i]) &&  (buf[ofs + i]) <= 0x7E) ? buf[ofs + i] : '.';
            } else {
                str[k++] = ' ';
            }
        }

        str[k] = '\0';
        MSG_PRINTF(LOG_INFO, "%s |\r\n", str);
    }

    MSG_PRINTF(LOG_INFO, "%s\r\n", dimm);
    #endif
}

