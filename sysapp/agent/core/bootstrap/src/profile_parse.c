
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.c
 * Date        : 2019.08.28
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>
#include "profile_parse.h"
#include "rt_rplmn.h"
#include "file.h"
#include "ProfileInfo1.h"
#include "FileInfo.h"
#include "TBHRequest.h"
#include "BootstrapRequest.h"
#include "tlv.h"
#include "agent_queue.h"
#include "convert.h"
#include "hash.h"
#include "rt_qmi.h"

profile_data_t g_data;
static uint8_t *g_buf = NULL;
static uint16_t g_buf_size = 0;

static uint32_t get_offset(rt_fshandle_t fp, uint8_t type, uint8_t *asset, uint32_t *size)
{
    int ret = 0;
    uint8_t buf[9];
    uint32_t offset = 0;

    rt_fseek(fp, offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    if ((buf[0] != SHARED_PROFILE)) {
        return RT_ERROR;
    }
    offset += get_length(buf, 1);
    MSG_INFO_ARRAY("111file info:", buf, 8);
    rt_fseek(fp, offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    if ((buf[0] != SHARED_PROFILE)) {
        return RT_ERROR;
    }
    offset += get_length(buf, 1);
    MSG_INFO_ARRAY("222file info:", buf, 8);
    rt_fseek(fp, offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    while (buf[0] != type) {
        offset += get_length(buf, 1) + get_length(buf, 0);
        rt_fseek(fp, offset, RT_FS_SEEK_SET);
        if (rt_fread(buf, 1, 8, fp) != 8) {
            return RT_ERROR;
        }
    }
    return offset;
}

static uint16_t rt_init_file_info(rt_fshandle_t fp)
{
    uint8_t *p = NULL;
    uint8_t buf[100];

    rt_fseek(fp, g_data.file_info_offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    p = get_value_buffer(buf);
    g_data.file_version_offset = g_data.file_info_offset + get_length(buf, 1) + get_length(p, 0) + get_length(p, 1);
    p = get_value_buffer(p);
    g_data.operator_num = p[0];
    
    MSG_PRINTF(LOG_INFO, "operator_num:%d\n", g_data.operator_num);
    MSG_PRINTF(LOG_INFO, "file_version_offset:%d\n", g_data.file_version_offset);
}

static uint32_t rt_get_root_sk_offset(rt_fshandle_t fp, uint8_t *sk, uint32_t *size)
{
    return get_offset(fp, ROOT_SK, sk, size);
}

static uint32_t rt_get_aes_key_offset(rt_fshandle_t fp, uint8_t *sk, uint32_t *size)
{
    return get_offset(fp, PROFILE_KEY, sk, size);
}

static uint32_t rt_get_file_info_offset(rt_fshandle_t fp, uint8_t *sk, uint32_t *size)
{
    return get_offset(fp, FILE_INFO, sk, size);
}

static uint32_t rt_get_operator_profile_offset(rt_fshandle_t fp, uint8_t *sk, uint32_t *size)
{
    return get_offset(fp, OPT_PROFILES, sk, size);
}

void check_hash_code()
{
    // sha256_init(hash_code);
    // sha256_update(hash_code, hash_code_buf, int32_t len);
    // sha256_final(sha256_ctx *ctx, uint8_t hash[]);
}

static uint32_t rt_get_hash_code_offset(rt_fshandle_t fp)
{
    uint8_t buf[8], hash_code_buf[32];
    uint32_t off = 0;
    sha256_ctx hash_code;

    off = g_data.operator_info_offset;
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    off += get_length(buf, 0) + get_length(buf, 1);

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    if (buf[0] != 0x41 || buf[1] != 0x20) {
        return RT_ERROR;
    }

    // rt_task *task_id;
    // rt_create_task(&task_id, NULL, check_hash_code, NULL);
    // pthread_join(task_id, NULL);
    return off;
}

static int32_t decode_file_info(rt_fshandle_t fp)
{
    uint8_t buf[100];
    int32_t size;
    FileInfo_t *request;

    rt_fseek(fp, g_data.file_info_offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    asn_dec_rval_t dc;
    size = get_length(buf, 0) + get_length(buf, 1);
    buf[0] = 0x30;
    dc = ber_decode(NULL, &asn_DEF_FileInfo, (void **) &request, buf, size);

    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "%ld\n", dc.consumed);
        return RT_ERROR;
    }
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_FileInfo, request);
    }
    return RT_SUCCESS;
}

static int32_t decode_profile(rt_fshandle_t fp, uint16_t off, int length)
{
    BootstrapRequest_t *request = NULL;
    asn_dec_rval_t dc;
    uint8_t *buf = NULL;

    buf = (uint8_t *) rt_os_malloc(length);
    if (!buf) {
        MSG_PRINTF(LOG_ERR, "malloc failed!\n");
        return RT_ERROR;
    }
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, length, fp);
    dc = ber_decode(NULL, &asn_DEF_BootstrapRequest, (void **) &request, buf, length);
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
    }
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_BootstrapRequest, request);
    }
    rt_os_free(buf);
    return RT_SUCCESS;
}

static int32_t encode_cb_fun(const void *buffer, size_t size, void *app_key)
{
    g_buf = rt_os_realloc(g_buf, g_buf_size + size);
    if (!g_buf) {
        MSG_PRINTF(LOG_ERR, "realloc failed!!\n");
        return RT_ERROR;
    }
    rt_os_memcpy(g_buf + g_buf_size, (void *) buffer, size);
    g_buf_size += size;
    return RT_SUCCESS;
}

static int32_t update_hash(uint8_t *buf, int32_t profile_len, uint8_t *profile_hash)
{
    uint8_t *p = NULL;
    sha256_ctx profile_ctx;
    int32_t size = 0;

    p = get_value_buffer(buf);
    size = get_length(p, 0) + get_length(p, 1);
    sha256_init(&profile_ctx);
    sha256_update(&profile_ctx, p, size);
    sha256_final(&profile_ctx, profile_hash);
    MSG_INFO_ARRAY("Current profile_hash:", profile_hash, 32);
    return RT_SUCCESS;
}

static int32_t build_profile(uint8_t *profile_buffer, int32_t profile_len, int32_t selected_profile_index, BOOLEAN_t sequential)
{
    BootstrapRequest_t *bootstrap_request = NULL;
    asn_dec_rval_t dc;
    asn_enc_rval_t ec;
    uint8_t profile_hash[32];
    uint16_t mcc;
    int32_t i;
    int32_t ret = RT_ERROR;
    uint16_t length;
    uint8_t bytes[10];

    dc = ber_decode(NULL, &asn_DEF_BootstrapRequest, (void **) &bootstrap_request, profile_buffer, profile_len);
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        return RT_ERROR;
    }
    if (sequential == 0xFF) {
        uint8_t imsi_buffer[2];
        imsi_buffer[0] = bootstrap_request->tbhRequest.imsi.buf[7];
        imsi_buffer[1] = bootstrap_request->tbhRequest.imsi.buf[8];
        swap_nibble(imsi_buffer, 2);
        char imsi_buf[5] = {0};
        bytes2hexstring(imsi_buffer, 2, imsi_buf);
        uint16_t imsi_len = sizeof(imsi_buffer);
        int32_t imsi = selected_profile_index + atoi(imsi_buf);
        snprintf(imsi_buf, sizeof(imsi_buf), "%04d", imsi);
        hexstring2bytes(imsi_buf, imsi_buffer, &imsi_len);
        swap_nibble(imsi_buffer, 2);
        bootstrap_request->tbhRequest.imsi.buf[7] = imsi_buffer[0];
        bootstrap_request->tbhRequest.imsi.buf[8] = imsi_buffer[1];
    }

    rt_qmi_get_mcc_mnc(&mcc, NULL);
    for (i = 0; i < ARRAY_SIZE(rt_plmn); ++i) {
        if (mcc == rt_plmn[i].mcc) {
            hexstring2bytes(rt_plmn[i].rplmn, bytes, &length); // must convert string to bytes
            bootstrap_request->tbhRequest.rplmn = OCTET_STRING_new_fromBuf(
                    &asn_DEF_TBHRequest, bytes, length);
            // bootstrap_request->tbhRequest.hplmn = OCTET_STRING_new_fromBuf(
            //        &asn_DEF_TBHRequest, rt_plmn[i].hplmn, rt_os_strlen(rt_plmn[i].hplmn));
            break;
        }
    }

    g_buf_size = 0;
    ec = der_encode(&asn_DEF_BootstrapRequest, bootstrap_request, encode_cb_fun, NULL);
    if (ec.encoded == -1) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        goto end;
    }
    update_hash(g_buf, profile_len, profile_hash);

    rt_os_memcpy(bootstrap_request->hashCode.buf, profile_hash, bootstrap_request->hashCode.size);
    g_buf_size = 0;
    ec = der_encode(&asn_DEF_BootstrapRequest, bootstrap_request, encode_cb_fun, NULL);
    if (ec.encoded == -1) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        goto end;
    }

    if (bootstrap_request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_BootstrapRequest, bootstrap_request);
    }

    MSG_INFO_ARRAY("Current profile:", g_buf, g_buf_size);
    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_SETTING_PROFILE, g_buf, g_buf_size);
    ret = RT_SUCCESS;
end:
    rt_os_free(g_buf);
    g_buf = NULL;
    return ret;
}

static int32_t decode_profile_info(rt_fshandle_t fp, uint32_t off, int32_t random)
{
    uint32_t selected_profile_index, profile_len, size;
    uint8_t *profile_buffer = NULL;
    ProfileInfo1_t *request = NULL;
    uint8_t buf[100];
    asn_dec_rval_t dc;

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    off += get_length(buf, 1);

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    size = get_length(buf, 0) + get_length(buf, 1);

    // 如果tag为A0则与子项tag重名导致无法解析
    if (buf[0] == 0xA0) {
        buf[0] = 0x30;
    }
    dc = ber_decode(NULL, &asn_DEF_ProfileInfo1, (void **) &request, buf, size);

    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "%ld\n", dc.consumed);
        return RT_ERROR;
    }
    off += size;
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    off += get_length(buf, 1);

    MSG_PRINTF(LOG_INFO, "apn:%s\n", request->apn.list.array[0]->apnName.buf);
    rt_qmi_modify_profile(1, 0, request->apn.list.array[0]->apnName.buf, 0);

    selected_profile_index = random % request->totalNum;
    if (request->sequential == 0xFF) {
        profile_len = get_length(buf, 0);
    } else {
        profile_len = get_length(buf, 0) / request->totalNum;
        off += selected_profile_index * profile_len;
    }
    profile_buffer = (uint8_t *) rt_os_malloc(profile_len);
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(profile_buffer, 1, profile_len, fp);

    build_profile(profile_buffer, profile_len, selected_profile_index, request->sequential);
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileInfo1, request);
    }
    rt_os_free(profile_buffer);

    return RT_SUCCESS;
}

int32_t selected_profile(int32_t random)
{
    rt_fshandle_t fp;
    uint8_t buf[8];
    uint32_t off = g_data.operator_info_offset;
    uint16_t profile_len, selected_profile_index;
    int32_t i = 0;
    fp = rt_fopen(SHARE_PROFILE, RT_FS_READ);
    if (fp == NULL) {
        MSG_PRINTF(LOG_ERR, "Open file failed\n");
        return RT_ERROR;
    }
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    if (buf[0] != 0xA3) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        return RT_ERROR;
    }
    off += get_length(buf, 1);

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 8, fp);
    if (buf[0] != 0x30) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        return RT_ERROR;
    }
    if (g_data.priority >= g_data.operator_num) {
        g_data.priority = 0;
    }
    for (i = 0; i < g_data.priority; i++) {
        off += get_length(buf, 1) + get_length(buf, 0);
        rt_fseek(fp, off, RT_FS_SEEK_SET);
        rt_fread(buf, 1, 8, fp);
    }
    decode_profile_info(fp, off, random);

    if (fp != NULL) {
        rt_fclose(fp);
    }
    g_data.priority++;

    return RT_SUCCESS;
}

int32_t init_profile_file(int32_t *arg)
{
    int32_t ret = RT_SUCCESS;
    uint8_t buf[500];
    uint32_t len = 0;
    rt_fshandle_t fp;

    fp = rt_fopen(SHARE_PROFILE, RT_FS_READ);
    if (fp == NULL) {
        return RT_ERROR;
    }
    g_data.file_info_offset = rt_get_file_info_offset(fp, buf, &len);
    rt_init_file_info(fp);
    g_data.root_sk_offset = rt_get_root_sk_offset(fp, buf, &len);
    g_data.aes_key_offset = rt_get_aes_key_offset(fp, buf, &len);
    g_data.operator_info_offset = rt_get_operator_profile_offset(fp, buf, &len);
    g_data.hash_code_offset = rt_get_hash_code_offset(fp);

    if (fp != NULL) {
        rt_fclose(fp);
    }

    g_data.priority = 0;
    return ret;
}
