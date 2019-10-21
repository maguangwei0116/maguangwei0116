
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : inspect_file.c
 * Date        : 2019.10.16
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "inspect_file.h"
#include "file.h"
#include "hash.h"

#define MONITOR                         "/data/monitor"
#define AGENT                           "/data/agent1"
#define HASH_CHECK_BLOCK                1024    /* block size for HASH check */
#define MAX_FILE_HASH_BYTE_LEN          32
#define PRIVATE_HASH_STR_LEN            64
#define MAX_FILE_HASH_LEN               64
#define RT_CHECK_ERR(process, result) \
    if((process) == result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}

static rt_bool monitor_inspect_file(uint8_t *file_name)
{
    rt_fshandle_t fp = NULL;
    sha256_ctx_t sha_ctx;
    int8_t hash_buffer[HASH_CHECK_BLOCK];
    int8_t hash_result[MAX_FILE_HASH_LEN + 1];
    int8_t hash_out[MAX_FILE_HASH_BYTE_LEN + 1];
    int8_t last_hash_buffer[PRIVATE_HASH_STR_LEN + 1] = {0};
    int32_t file_size = 0;
    uint32_t check_size;
    int32_t partlen;
    rt_bool ret = RT_FALSE;

    file_size = linux_file_size(file_name);

    RT_CHECK_ERR(fp = rt_fopen((char *)file_name, "r"), NULL);
    sha256_init(&sha_ctx);
    file_size -= PRIVATE_HASH_STR_LEN;
    if (file_size < HASH_CHECK_BLOCK) {
        rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
        RT_CHECK_ERR(linux_fread(hash_buffer, file_size, 1, fp), 0);
        sha256_update(&sha_ctx, (uint8_t *)hash_buffer, file_size);
    } else {
        for (check_size = HASH_CHECK_BLOCK; check_size < file_size; check_size += HASH_CHECK_BLOCK) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(linux_fread(hash_buffer, HASH_CHECK_BLOCK, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, HASH_CHECK_BLOCK);
        }

        partlen = file_size + HASH_CHECK_BLOCK - check_size;
        if (partlen > 0) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(linux_fread(hash_buffer, partlen, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, partlen);
        }

        RT_CHECK_ERR(linux_fread(last_hash_buffer, PRIVATE_HASH_STR_LEN, 1, fp), 0);
    }

    sha256_final(&sha_ctx, (uint8_t *)hash_out);
    bytestring_to_charstring(hash_out, hash_result, MAX_FILE_HASH_BYTE_LEN);
    if(strncpy_case_insensitive(hash_result, last_hash_buffer, MAX_FILE_HASH_LEN) != RT_TRUE) {
        goto end;
    }
    ret = RT_TRUE;

end:
    if (fp != NULL) {
        rt_fclose(fp);
    }

    return ret;
}

rt_bool inspect_monitor_file(void)
{
    return monitor_inspect_file(MONITOR);
}

rt_bool inspect_agent_file(void)
{
    return monitor_inspect_file(AGENT);
}

rt_bool inspect_abstract_content(uint8_t *hash, uint8_t *signature)
{
    rt_bool ret = RT_FALSE;

    ret = RT_TRUE;
    return ret;
}
