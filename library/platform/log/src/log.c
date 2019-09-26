
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
#include "file.h"

#define LOG_TIMESTAMP_FORMAT    "[%Y-%m-%d %H:%M:%S]"
#define DATA_REDTEA_PATH        "/data/redtea"
#define LOG_NAME                "/data/redtea/rt_log"
#define LOG_FILE_SIZE           1024 * 1024  // 1MB
#define LOG_BUF_SIZE            1024
#define LOG_MAX_LINE_SIZE       2 * 1024
#define LOG_LEVEL_LEN           6
#define LOG_FILE_NAME_LEN       64
#define LOG_HEXDUMP_LEVEL       LOG_INFO
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           sizeof(a)/sizeof(a[0])
#endif

typedef struct LOG_LEVEL_ITEM {
    log_level_e     level;
    const char *    label;
} log_item_t;

typedef struct LOG_PARAM {
    log_level_e     level;
    log_mode_e      mode;
    uint32_t        max_size;                   // max log size
    uint32_t        cur_size;                   // cur log size
    rt_fshandle_t   fp;
    char            file[LOG_FILE_NAME_LEN];    // log file name
} log_param_t;

static log_item_t g_log_item_table[] =
{
    {LOG_NONE,      "[NONE]",},
    {LOG_ERR,       "[ERR ]",},
    {LOG_WARN,      "[WARN]",},
    {LOG_DBG,       "[DBG ]",},
    {LOG_INFO,      "[INFO]",},
    {LOG_ALL,       "[ALL ]",},
};

static log_param_t g_log_param =
{
    LOG_INFO,
    LOG_PRINTF_TERMINAL,
    LOG_FILE_SIZE,
    0,
    NULL,
    {LOG_NAME},
};

static int32_t init_data_redtea_path(void *arg)
{
    if (rt_os_access(DATA_REDTEA_PATH, F_OK)) {
        rt_os_mkdir(DATA_REDTEA_PATH);
    }

    return 0;
}

int32_t log_set_param(log_mode_e mode, log_level_e level, unsigned int max_size)
{
    g_log_param.mode = mode;
    g_log_param.level = level;
    if (max_size > 0) {
        g_log_param.max_size = max_size;
    }

    return 0;
}

static int32_t log_file_size(const char *file)
{
    struct stat statbuf;
    int32_t size;

    if (rt_os_access(file, F_OK)) { /* log file isn't exist */
        return 0;
    }

    stat(file, &statbuf);
    size = statbuf.st_size;
    return size;
}

static void log_file_open(void)
{
    g_log_param.fp = linux_fopen(g_log_param.file, "a+");
    if(!g_log_param.fp) {
        printf("log file (%s) open error, err(%d)=%s\n", g_log_param.file, errno, strerror(errno));
        return;
    }
    g_log_param.cur_size = log_file_size(g_log_param.file);
}

static void log_file_close(void)
{
    linux_fclose(g_log_param.fp);
    g_log_param.fp = NULL;
}

int32_t init_log_file(void *arg)
{
    const char *log_file = (const char *)arg;

    if (log_file && rt_os_strlen(log_file)) {
        snprintf(g_log_param.file, sizeof(g_log_param.file), "%s", log_file);
    }

    init_data_redtea_path(NULL);

    log_file_open();

    return 0;
}

static int32_t log_file_clear(void)
{
    g_log_param.fp = linux_fopen(g_log_param.file, "w+");
    if(!g_log_param.fp) {
        printf("log file open error\n");
        return -1;
    }

    linux_fclose(g_log_param.fp);
    g_log_param.fp = NULL;

    return 0;
}

static int32_t log_file_write(const char *data, int32_t len)
{
    if (g_log_param.fp) {
        linux_fwrite(data, 1, len, g_log_param.fp);
        linux_fflush(g_log_param.fp);
    }
    return 0;
}

static void log_local_print(const char *data, int32_t len)
{
    int32_t size = 0;
    if (g_log_param.mode == LOG_PRINTF_TERMINAL) {
        printf("%s", data);
    } else {
        //printf("g_log_param.cur_size: %d/%d \r\n", g_log_param.cur_size, g_log_param.max_size);
        if (g_log_param.cur_size > g_log_param.max_size) {
            log_file_close();
            log_file_clear();
            log_file_open();
        }
        log_file_write(data, len);
        g_log_param.cur_size += len;

#if 0  // only for test
        {
            printf("g_log_param.cur_size: %d, max: %d\r\n", g_log_param.cur_size, g_log_param.max_size);
        }
#endif

    }
}

int32_t log_print(log_level_e level, log_level_flag_e level_flag, const char *msg, ...)
{
    char content[LOG_BUF_SIZE] = {0};
    time_t  time_write;
    struct tm tm_Log;
    uint32_t len = 0;
    va_list vl_list;

    if (level > g_log_param.level) {
        return -1;
    }

    if (level_flag == LOG_HAVE_LEVEL_PRINTF) {
        rt_os_memcpy(content, g_log_item_table[level].label, LOG_LEVEL_LEN);
        len = LOG_LEVEL_LEN;
        time_write = time(NULL);
        localtime_r(&time_write, &tm_Log);
        strftime((char *)&content[len], sizeof(content) - len, LOG_TIMESTAMP_FORMAT, &tm_Log);
        len = rt_os_strlen(&content[len]) + LOG_LEVEL_LEN;
    }

    va_start(vl_list, msg);
    vsnprintf((char *)&content[len], sizeof(content) - len, (const char *)msg, vl_list);
    va_end(vl_list);

    log_local_print(content, rt_os_strlen(content));

    return 0;
}

int32_t log_hexdump(const char *file, int32_t line, const char *title, const void *data, unsigned int len)
{
    char str[160], octet[10];
    int ofs, i, k, d;
    const unsigned char *buf = (const unsigned char *)data;
    const char dimm[] = "+------------------------------------------------------------------------------+";

    if (g_log_param.level < LOG_HEXDUMP_LEVEL) {
        return 0;
    }

    MSG_PRINTF(LOG_HEXDUMP_LEVEL, "[%s, %d] %s (%d bytes):\r\n", file, line, title, len);
    MSG_PRINTF(LOG_HEXDUMP_LEVEL, "%s\r\n", dimm);
    MSG_PRINTF(LOG_HEXDUMP_LEVEL, "| Offset  : 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F   0123456789ABCDEF |\r\n");
    MSG_PRINTF(LOG_HEXDUMP_LEVEL, "%s\r\n", dimm);

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
        MSG_PRINTF(LOG_HEXDUMP_LEVEL, "%s |\r\n", str);
    }

    MSG_PRINTF(LOG_HEXDUMP_LEVEL, "%s\r\n", dimm);

    return 0;
}

static rt_bool log_check_level(const char *header, log_level_e min_level)
{
    static log_level_e last_log_level = LOG_UNKNOW;
    log_level_e cur_level = LOG_UNKNOW;
    int32_t i;

    for (i = 0; i < ARRAY_SIZE(g_log_item_table); i++) {
        if (!rt_os_strcmp(g_log_item_table[i].label, header)) {
            cur_level = g_log_item_table[i].level;
            break;
        }
    }

    if (cur_level == LOG_UNKNOW) {
        cur_level = last_log_level;
    } else {
        last_log_level = cur_level;
    }

    return (cur_level <= min_level) ? RT_TRUE: RT_FALSE;
}

int32_t log_file_copy_out(const char* file_in, const char* file_out, log_level_e min_level)
{
    char *data = NULL;
    rt_fshandle_t fp_in = NULL;
    rt_fshandle_t fp_out = NULL;
    int32_t i = 0;
    int32_t ret;
    char header[LOG_LEVEL_LEN+1] = {0};

    data = (char *)rt_os_malloc(LOG_MAX_LINE_SIZE);
    if(!data) {
        MSG_PRINTF(LOG_WARN, "memory alloc fail\n");
        ret = -1;
        goto exit_entry;
    }

    fp_in = linux_fopen(file_in, "r");
    if(!fp_in) {
        MSG_PRINTF(LOG_WARN, "can't open file %s\n", file_in);
        ret = -1;
        goto exit_entry;
    }

    fp_out = linux_fopen(file_out, "a+");
    if(!fp_out) {
        MSG_PRINTF(LOG_WARN, "can't open file %s\n", file_out);
        ret = -1;
        goto exit_entry;
    }

    while(!linux_feof(fp_in)) {
        if (linux_fgets(data, LOG_MAX_LINE_SIZE, fp_in) != NULL) {
            memcpy(header, data, LOG_LEVEL_LEN);
            if (log_check_level(header, min_level)) {
                //printf("line %d=== %s", ++i, data);
                linux_fwrite(data, 1, rt_os_strlen(data), fp_out);
            }
        }
    }
    ret = 0;

exit_entry:
    if (fp_in) {
        linux_fclose(fp_in);
    }

    if (fp_out) {
        linux_fclose(fp_out);
    }

    if (data) {
        rt_os_free(data);
        data = NULL;
    }

    return ret;
}

