
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : vuicc_callback.c
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "rt_type.h"
#include "vuicc_callback.h"
#include "cos_api.h"
#include "cb_io.h"

//#define NVM_SIZE                    0x20000UL // 128*1024UL
//#define NVM_INIT_VALUE              0xFF
#define VUICC_FILE_NAME_LEN    64
static char g_vuicc_file[VUICC_FILE_NAME_LEN];

uint32_t linux_random(uint32_t key)
{
    srand(key);
    return rand();
}

void linux_printf(const uint8_t *buf, uint32_t len)
{
    log_print(LOG_INFO, LOG_NO_LEVEL_PRINTF, buf, len);
}

static FILE *fp = NULL;
int32_t linux_nvm_init(void)
{
    unsigned long i;
    char buf[1] = {NVM_INIT_VALUE};

    if (access((const char *)g_vuicc_file, F_OK) != 0) {
        fp = fopen((const char *)g_vuicc_file, "wb+");
    } else {
        fp = fopen((const char *)g_vuicc_file, "rb+");
        printf("nvm init [ok]\n");
        return 0;
    }
    if (!fp) {
        printf("nvm init [fail]\n");
        // assert(fp);
        return -1;
    }

    for (i = 0; i < NVM_SIZE; i++) {
        fwrite(buf, 1, 1, fp);
    }
    printf("nvm init [ok]\n");
    return 0;
}

int32_t linux_nvm_write(const uint8_t *src, uint32_t dest, uint32_t length)
{
    if (fp == NULL) {
        fp = fopen((const char *)g_vuicc_file, "r+");
        if (!fp) {
            printf("fopen false\n");
            return -1;
        }
    }

    if (fseek(fp, dest, SEEK_SET) < 0) {
        printf("fseek false:%s\n", strerror(errno));
    }

    if(fwrite(src, 1, length, fp) == length) {
        if(fdatasync(fileno(fp)) != 0) {
            return -1;
        }
        return 0;
    } else {
        printf("fwrite false:%s\n", strerror(errno));
        return -1;
    }
}

int32_t linux_nvm_read(const uint8_t *dest, uint32_t src, uint32_t length)
{
    if (fp == NULL) {
        fp = fopen((const char *)g_vuicc_file, "rb+");
        if (!fp) {
            printf("fopen false\n");
            return -1;
        }
    }

    if (fseek(fp, src, SEEK_SET) < 0) {
        printf("fseek false:%s\n", strerror(errno));
    }

    if(fread((uint8_t *)dest, 1, length, fp) == length) {
        return 0;
    } else {
        printf("fread false:%s\n", strerror(errno));
        return -1;
    }
}

cos_server_operation_t server_opt = {
    linux_printf,
    (_cos_memset *)rt_os_memset,
    (_cos_memcpy *)rt_os_memcpy,
    (_cos_memmove *)rt_os_memmove,
    (_cos_memcmp *)rt_os_memcmp,
    rt_os_malloc,
    rt_os_calloc,
    rt_os_realloc,
    rt_os_free,
    (_cos_strlen *)rt_os_strlen,
    linux_random,
    linux_nvm_init,
    linux_nvm_write,
    linux_nvm_read,
    linux_io_server_init,
    linux_io_server_wait,
    linux_io_server_send,
    linux_io_server_recv
};

cos_client_operation_t client_opt = {
    linux_io_client_connect,
    linux_io_client_close,
    linux_io_client_send,
    linux_io_client_recv
};

int init_callback_ops(void *arg)
{
    rt_os_memcpy(g_vuicc_file, (uint8_t *)arg, rt_os_strlen((uint8_t *)arg));
    g_vuicc_file[rt_os_strlen((uint8_t *)arg)] = '\0';
    MSG_PRINTF(LOG_INFO, "g_vuicc_file:%s\n", g_vuicc_file);

    if (cos_init(&server_opt, &client_opt) == -1) {
        MSG_PRINTF(LOG_ERR, "cos init failed.\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}
