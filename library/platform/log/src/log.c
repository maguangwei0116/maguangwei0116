
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
#define LOG_LEVEL_LEN           6
#define LOG_FILE_NAME_LEN       64
#define LOG_HEXDUMP_LEVEL       LOG_INFO

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
    {LOG_ERR,       "[ERR ]",},
    {LOG_WARN,      "[WARN]",},
    {LOG_DBG,       "[DBG ]",},
    {LOG_INFO,      "[INFO]",},
    {LOG_NONE,      "[NONE]",},
};

static log_param_t g_log_param = 
{
    LOG_INFO,
    LOG_PRINTF_FILE,
    LOG_FILE_SIZE,
    0,
    NULL,
    {0},
};

static int32_t init_data_redtea_path(void *arg)
{
    if (rt_os_access(DATA_REDTEA_PATH, 0)) {
        rt_os_mkdir(DATA_REDTEA_PATH);
    }

    return 0;
}

int32_t log_set_param(log_mode_e mode, log_level_e level, const char *log_file)
{
    g_log_param.mode = mode;
    g_log_param.level = level;
    snprintf(g_log_param.file, sizeof(g_log_param.file), "%s", log_file);
    
    return 0;
}

static int32_t log_file_size(void)
{
    struct stat statbuf;
    int32_t size;

    if (rt_os_access(LOG_NAME, F_OK)) { /* log file isn't exist */
        return 0;
    }

    stat(LOG_NAME, &statbuf);
    size = statbuf.st_size;
    return size;
}

static void log_file_open(void)
{
    g_log_param.fp = rt_fopen(LOG_NAME, "a+");
    if(!g_log_param.fp) {
        printf("log file open error\n");
        return;
    }
    g_log_param.cur_size = log_file_size();
}

static void log_file_close(void)
{
    rt_fclose(g_log_param.fp);
    g_log_param.fp = NULL;
}

int32_t init_log_file(void *arg)
{
    uint32_t *param = (uint32_t *)arg;
    if (param) {
        g_log_param.max_size = *param;   
    }

    init_data_redtea_path(NULL);

    log_file_open();
    
    return 0;
}

static int32_t log_file_clear(void)
{
    g_log_param.fp = rt_fopen(LOG_NAME, "w+");
    if(!g_log_param.fp) {
        printf("log file open error\n");
        return -1;
    }
    
    rt_fclose(g_log_param.fp);
    g_log_param.fp = NULL;
    
    return 0;
}

static int32_t log_file_write(const char *data, int32_t len)
{
    if (g_log_param.fp) {
        rt_fwrite(data, 1, len, g_log_param.fp);
        rt_fflush(g_log_param.fp); 
    }
    return 0;
}

static void log_local_print(const char *data, int32_t len)
{
    int32_t size = 0;
    if (g_log_param.mode == LOG_PRINTF_TERMINAL) {
        printf("%s", data);
    } else {
        if (g_log_param.cur_size > g_log_param.max_size) {
            log_file_close();
            log_file_clear();
            log_file_open();
        }
        log_file_write(data, len);
        
#if 0  // only for test
        {
            char tmp[128];
            g_log_param.cur_size += len;
            snprintf(tmp, sizeof(tmp), "g_log_param.cur_size: %d, max: %d\r\n", g_log_param.cur_size, g_log_param.max_size);
            printf(tmp);
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

