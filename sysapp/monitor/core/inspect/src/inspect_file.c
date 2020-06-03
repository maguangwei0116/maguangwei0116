
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
#define TAG_LEN                         2
#define RT_CHECK_ERR(process, result)   if((process) == result){ MSG_PRINTF(LOG_WARN, "[%s] error\n", #process);  goto end;}

// static const uint8_t *g_verify_pk = "B37F3BAD94DFCC1FBDB0FBF608802FA72D38FAEE3AB8CBBF63BF6C99DA9E31FAE1465F1BCFCAF85A6626B938D1BD12D6901833047C50FE8ED67B84CFCFECCFEA";
#if (CFG_ENV_TYPE_PROD)
static const uint8_t g_verify_pk[] = {0x00, 0x33, 0x9F, 0x06, 0x30, 0x13, 0xE2, 0xC7, 0x22, 0xF9, 0x72, 0x42, 0x74, 0x7C, 0x42, 0xEB, 0x15, 0xBF, 0x21, 0x28, 0xF5, 0x69, 0xA1, 0xF2, 0x39, 0x36, 0xEE, 0xF4, 0x6F, 0x9D, 0xE3, 0x06, 0xCD, 0x21, 0xA4, 0x4C, 0x3E, 0x28, 0x79, 0x1C, 0x9D, 0x49, 0x54, 0x8E, 0x0F, 0x41, 0xA5, 0xA9, 0x5D, 0xF5, 0xA1, 0xE2, 0xBC, 0xDF, 0x5A, 0xB2, 0x21, 0x71, 0xC5, 0xC7, 0x83, 0xFE, 0x18, 0xB5, 0xC7};
#else
static const uint8_t g_verify_pk[] = {0x00, 0x00, 0xB7, 0xEE, 0x6A, 0x54, 0x9E, 0xB9, 0xF3, 0xFA, 0x3B, 0xA3, 0x37, 0xAA, 0xC9, 0x1F, 0xC3, 0x5A, 0x13, 0x5E, 0x2E, 0x60, 0x6C, 0xA6, 0x23, 0xC9, 0xFF, 0x4C, 0x85, 0xA7, 0x55, 0x2F, 0xB4, 0x99, 0x98, 0x7E, 0x66, 0x5D, 0x0A, 0x3D, 0x31, 0x08, 0xCF, 0xB4, 0x62, 0x97, 0xF5, 0xD9, 0x08, 0x27, 0xD3, 0x4C, 0x1F, 0x47, 0xF6, 0xF0, 0x5E, 0x10, 0x37, 0x6A, 0xE1, 0x6C, 0x3E, 0xED, 0x29};
#endif

/**
 * function: verify a section data by input public key
 * return value:
 * - 0 (flase) : verify fail
 * - 1 (true)  : verify ok
 * notes: This function was defined in libvuicc.a
 */
extern int ecc_verify(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int  sig_len);
// extern int ecc_hash_verify_signature(uint8_t *input, int input_len, const uint8_t *pubkey, int pubkey_len, uint8_t *sigBuff, int sig_len);

rt_bool monitor_get_file_sign_data(const char *file_name, uint8_t *sign, int32_t *len)
{
    rt_fshandle_t fp = NULL;
    int8_t sign_buffer[PRIVATE_HASH_STR_LEN + 1] = {0};
    rt_bool ret = RT_FALSE;

    RT_CHECK_ERR(fp = linux_fopen((char *)file_name, "r"), NULL);
    RT_CHECK_ERR(linux_fseek(fp, -(PRIVATE_HASH_STR_LEN), RT_FS_SEEK_END), -1);
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
    int8_t sign_buffer_out[PRIVATE_HASH_STR_LEN/2 + 1] = {0};
    uint16_t sign_buffer_out_len = 0;
    int32_t file_size = 0;
    uint32_t check_size;
    int32_t partlen;
    rt_bool ret = RT_FALSE;

    file_size = linux_file_size(file_name);
    MSG_PRINTF(LOG_TRACE, "%s, file_size:%d\n", file_name, file_size);

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
        MSG_PRINTF(LOG_TRACE, "%s, file_size: %d, check_size: %d, partlen:%d\n", file_name, file_size, check_size, partlen);
        if (partlen > 0) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(linux_fread(hash_buffer, partlen, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, partlen);
        }
    }

    RT_CHECK_ERR(linux_fread(sign_buffer, PRIVATE_HASH_STR_LEN, 1, fp), 0);
    MSG_PRINTF(LOG_DBG, "signature_buffer:%s\n", sign_buffer);

    linux_fseek(fp, -(PRIVATE_HASH_STR_LEN + MAX_FILE_INFO_LEN), RT_FS_SEEK_END);
    RT_CHECK_ERR(linux_fread(file_info, MAX_FILE_INFO_LEN, 1, fp), 0);
    MSG_PRINTF(LOG_DBG, "file_info:%s\n", file_info);
    sha256_final(&sha_ctx, (uint8_t *)hash_out);

    bytestring_to_charstring((const char *)hash_out, (char *)hash_result, MAX_FILE_HASH_BYTE_LEN);

    hexstring2bytes(sign_buffer, sign_buffer_out, &sign_buffer_out_len);

    if (inspect_abstract_content(hash_result, sign_buffer_out) != RT_TRUE) {
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

rt_bool inspect_abstract_content(uint8_t *input, uint8_t *signature)
{
    uint8_t default_apdu[512];
    uint32_t all_len = 0;
    uint8_t buf[256];
    uint16_t rsp_len = 0;
    uint8_t ca_hm[80] = {0};
    uint8_t channel = 0xFF;
    int i = 0;
    int ret = 0;
    // tag for inspect
    default_apdu[0] = 0xFF;
    default_apdu[1] = 0x22;
    // 0x81 for one byte
    default_apdu[2] = 0x81;
    default_apdu[3] = 0xD0;
    // ecc or sign
    default_apdu[4] = 0x80;
    default_apdu[5] = 0x01;
    default_apdu[6] = 0x00;
    // length
    default_apdu[7] = 0xA1;
    default_apdu[8] = 0x81;
    default_apdu[9] = 0xCA;
    // length
    default_apdu[10] = 0xA0;
    default_apdu[11] = 0x81;
    default_apdu[12] = 0xC7;

    /******************************************************************/
    // data
    default_apdu[13] = 0x80;
    default_apdu[14] = 0x40;
    for (i = 0; i < 64; i++) {
        default_apdu[15+i] = input[i];  // 最后数组AsciiNum就是字符串每个字符所对应ASCII码值的数组
    }

    /******************************************************************/
    // pk
    default_apdu[15 + MAX_FILE_HASH_LEN] = 0x81;
    default_apdu[15 + MAX_FILE_HASH_LEN + 1] = 0x41;
    rt_os_memcpy(default_apdu + 15 + MAX_FILE_HASH_LEN + 1 + 1, g_verify_pk, MAX_PK_LEN/2+1);

    /******************************************************************/
    // sk
    default_apdu[15 + MAX_FILE_HASH_LEN + 2 + sizeof(g_verify_pk)] = 0x82;
    default_apdu[15 + MAX_FILE_HASH_LEN + 2 + sizeof(g_verify_pk) + 1] = 0x40;
    rt_os_memcpy(default_apdu + 15 + MAX_FILE_HASH_LEN + 2 + sizeof(g_verify_pk) + 2, signature, MAX_PK_LEN/2);

    all_len = 13 * 2 + 4 + MAX_FILE_HASH_LEN * 2 + 4 + sizeof(g_verify_pk) * 2 + 4 + PRIVATE_HASH_STR_LEN;

    MSG_PRINTF(LOG_DBG, "all_len is %d\n", all_len);
    MSG_INFO_ARRAY("default_apdu: ", default_apdu, all_len);
    rt_open_channel(&channel);
    ret = cmd_store_data(default_apdu, all_len, buf, &rsp_len, channel);
    rt_close_channel(channel);

    if (ret == 0) {
        MSG_PRINTF(LOG_DBG, "inspect file ret is %d\n", ret);
        return RT_TRUE;
    } else {
        MSG_PRINTF(LOG_ERR, "inspect file ret is %d\n", ret);
        return RT_FALSE;
    }
}
