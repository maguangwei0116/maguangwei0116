
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

#define HASH_CHECK_BLOCK                1024    /* block size for HASH check */
#define MAX_FILE_HASH_BYTE_LEN          32
#define MAX_FILE_INFO_LEN               20
#define PRIVATE_HASH_STR_LEN            128     /* There is a '\0' at the end */
#define MAX_FILE_HASH_LEN               64
#define MAX_PK_LEN                      128
#define RT_CHECK_ERR(process, result)   if((process) == result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}

static const uint8_t *g_verify_pk = "B37F3BAD94DFCC1FBDB0FBF608802FA72D38FAEE3AB8CBBF63BF6C99DA9E31FAE1465F1BCFCAF85A6626B938D1BD12D6901833047C50FE8ED67B84CFCFECCFEA";

/**
 * function: verify a section data by input public key
 * return value: 
 * - 0 (flase) : verify fail
 * - 1 (true)  : verify ok
 * notes: This function was defined in libvuicc.a
 */
extern int ecc_hash_verify_signature(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int sig_len);

rt_bool monitor_get_file_sign_data(const char *file_name, uint8_t *sign, int32_t *len)
{
    rt_fshandle_t fp = NULL;
    int8_t sign_buffer[PRIVATE_HASH_STR_LEN + 1] = {0};
    rt_bool ret = RT_FALSE;
    
    RT_CHECK_ERR(fp = linux_fopen((char *)file_name, "r"), NULL);
    linux_fseek(fp, -(PRIVATE_HASH_STR_LEN), SEEK_CUR);
    RT_CHECK_ERR(linux_fread(sign_buffer, PRIVATE_HASH_STR_LEN, 1, fp), 0);

    if (sign) {
        rt_os_memcpy(sign, sign_buffer, PRIVATE_HASH_STR_LEN);
    }
    
    if (len) {
        *len = PRIVATE_HASH_STR_LEN;
    }

    ret = RT_TRUE;

end:
    if (fp != NULL) {
        linux_fclose(fp);
    }

    return ret;
}

static int32_t get_real_file_name(char *name, int32_t len)
{
    int32_t i = 0;

    for (i = 0; i < len; i++) {
        if (name[i] == 'F') {
            rt_os_memset(&name[i], 0, len - i);
            break;
        }
    }

    return RT_SUCCESS;
}

/* file_name: Relative path in data-redtea */
rt_bool monitor_inspect_file(const char *file_name, const char *exp_real_file_name)
{
    rt_fshandle_t fp = NULL;
    sha256_ctx_t sha_ctx;
    int8_t hash_buffer[HASH_CHECK_BLOCK];
    int8_t hash_result[MAX_FILE_HASH_LEN + 1];
    int8_t hash_out[MAX_FILE_HASH_BYTE_LEN + 1];
    int8_t file_info[MAX_FILE_INFO_LEN + 1] = {0};
    int8_t sign_buffer[PRIVATE_HASH_STR_LEN + 1] = {0};
    int32_t file_size = 0;
    uint32_t check_size;
    int32_t partlen;
    rt_bool ret = RT_FALSE;

    file_size = linux_file_size(file_name);
    MSG_PRINTF(LOG_INFO, "%s, file_size:%d\n", file_name, file_size);

    RT_CHECK_ERR(fp = linux_fopen((char *)file_name, "r"), NULL);
    sha256_init(&sha_ctx);
    file_size -= PRIVATE_HASH_STR_LEN;
    if (file_size < HASH_CHECK_BLOCK) {
        partlen = file_size;
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

        RT_CHECK_ERR(linux_fread(sign_buffer, PRIVATE_HASH_STR_LEN, 1, fp), 0);
        MSG_PRINTF(LOG_INFO, "signature_buffer:%s\n", sign_buffer);
    }
    rt_os_memcpy(file_info, &hash_buffer[partlen - MAX_FILE_INFO_LEN], MAX_FILE_INFO_LEN);
    MSG_PRINTF(LOG_INFO, "file_info:%s\n", file_info);
    sha256_final(&sha_ctx, (uint8_t *)hash_out);

    bytestring_to_charstring(hash_out, hash_result, MAX_FILE_HASH_BYTE_LEN);

    if (inspect_abstract_content(hash_result, sign_buffer) != RT_TRUE) {
        MSG_PRINTF(LOG_ERR, "file_name:%s, verify signature failed!!\n", file_name);
        goto end;
    }

    get_real_file_name(file_info, MAX_FILE_INFO_LEN);
    if (rt_os_strcmp(file_info, exp_real_file_name)) {
        MSG_PRINTF(LOG_ERR, "file info unmatched, [%s] != [%s]\n", file_info, exp_real_file_name);
        goto end;
    }

    ret = RT_TRUE;

end:
    if (fp != NULL) {
        linux_fclose(fp);
    }

    return ret;
}

rt_bool inspect_abstract_content(uint8_t *hash, uint8_t *signature)
{
    rt_bool ret = RT_FALSE;

    ret = ecc_hash_verify_signature(hash, MAX_FILE_HASH_LEN, \
        g_verify_pk, MAX_PK_LEN, signature, PRIVATE_HASH_STR_LEN);

    return ret;
}
