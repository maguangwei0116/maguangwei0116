//
// Created by admin on 2019-08-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "rt_rplmn.h"
#include "file.h"
#include "profile_file_parsing.h"
#include "ProfileInfo1.h"
#include "FileInfo.h"
#include "TBHRequest.h"
#include "BootstrapRequest.h"
#include "tlv.h"
#include "agent_queue.h"
#include "convert.h"
#include "hash.h"
#include "rt_qmi.h"

#define SHARE_PROFILE "profile_list.der"

typedef struct profile_data {
    uint16_t file_info_offset;
    uint16_t root_sk_offset;
    uint16_t aes_key_offset;
    uint16_t operator_info_offset;
    int32_t priority;
    int32_t operator_num;
    uint16_t hash_code_offset;
} profile_data_t;

profile_data_t data;
uint8_t profile_buffer1[300];
uint16_t g_buf_size = 0;

static uint16_t get_offset(rt_fshandle_t fp, uint8_t type, uint8_t *asset, uint16_t *size)
{
    int ret = 0;
    uint8_t buf[4];
    uint16_t offset = 0;
    rt_fseek(fp, offset, RT_FS_SEEK_SET);

    rt_fread(buf, 1, 4, fp);
    if (buf[0] != SHARED_PROFILE) {
        return 101;
    }
    if (buf[1] != ASN1_LENGTH_2BYTES) {
        return 102;
    }

    rt_fread(buf, 1, 4, fp);
    if (buf[0] != SHARED_PROFILE) {
        return 103;
    }
    if (buf[1] != ASN1_LENGTH_2BYTES) {
        return 104;
    }

    offset = 8;

    rt_fread(buf, 1, 4, fp);
    while (buf[0] != type) {
        offset += 4;
        if (buf[1] == ASN1_LENGTH_2BYTES) {
            offset += ((uint16_t) buf[2] << 8) + buf[3];
        } else if (buf[1] == ASN1_LENGTH_1BYTES /*|| buf[1] == 0x22*/) {
            offset += buf[2] - 1;
        } else if ((buf[1] & 0x80) == 0) {
            offset += buf[1] - 2;
        } else {
            ret = 105;
            break;
        }

        rt_fseek(fp, offset, RT_FS_SEEK_SET);
        if (rt_fread(buf, 1, 4, fp) != 4) {
            return 106;
        }
    }
    return offset;
}

static uint16_t rt_get_root_sk_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size)
{
    return get_offset(fp, ROOT_SK, sk, size);
}

static uint16_t rt_get_aes_key_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size)
{
    return get_offset(fp, PROFILE_KEY, sk, size);
}

static uint16_t rt_get_file_info_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size)
{
    return get_offset(fp, FILE_INFO, sk, size);
}

static uint16_t rt_get_operator_profile_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size)
{
    return get_offset(fp, OPT_PROFILES, sk, size);
}

static uint16_t rt_get_hash_code_offset(rt_fshandle_t fp)
{
    uint8_t buf[4], hash_code_buf[32];
    uint16_t off = 0;
    sha256_ctx hash_code;

    off = data.operator_info_offset;
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    off += 4;
    if (buf[1] == ASN1_LENGTH_2BYTES) {
        off += ((uint16_t) buf[2] << 8) + buf[3];
    } else if (buf[1] == ASN1_LENGTH_1BYTES) {
        off += buf[2] - 1;
    } else if ((buf[1] & 0x80) == 0) {
        off += buf[1] - 2;
    }

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    if (buf[0] != 0x41 || buf[1] != 0x20) {
        return RT_ERROR;
    }

//    sha256_init(hash_code);
//    sha256_update(hash_code, hash_code_buf, int32_t len);
//    sha256_final(sha256_ctx *ctx, uint8_t hash[]);
    return off;
}

static uint8_t get_share_profile(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x30, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0x30, p, *left_len, tag_len, left_len);
    }
    return p;
}

static uint8_t get_profile_info(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x30, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0xA0, p, *left_len, tag_len, left_len);
    }
    return p;
}

static uint8_t *get_tbh_request(uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_value_buffer(profile_buffer1);
    return p;
}

static int decode_file_info(rt_fshandle_t fp) {
    uint8_t buf[100];
    int size;
    FileInfo_t *request;

    rt_fseek(fp, data.file_info_offset, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);
    asn_dec_rval_t dc;
    size = get_length(buf, 0) + get_length(buf, 1);
    buf[0] = 0x30;
    dc = ber_decode(NULL, &asn_DEF_FileInfo, (void **) &request, buf, size);

    if (dc.code != RC_OK) {
        printf("\n%ld\n", dc.consumed);
        return 0;// 报错
    }
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_FileInfo, request);
    }

    return 0;
}

static uint8_t decode_profile(rt_fshandle_t fp, uint16_t off, int length) {
    BootstrapRequest_t *request = NULL;
    asn_dec_rval_t dc;
    uint8_t buf[length];

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, length, fp);
    dc = ber_decode(NULL, &asn_DEF_BootstrapRequest, (void **) &request, buf, length);
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        return RT_ERROR;// 报错
    }
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_BootstrapRequest, request);
    }
    return RT_SUCCESS;
}

static int encode_cb_fun(const void *buffer, size_t size, void *app_key) {
    rt_os_memcpy(profile_buffer1 + g_buf_size, buffer, size);
    g_buf_size += size;
    return 0;
}

static int32_t update_hash(int32_t profile_len, uint8_t *profile_hash) {
    uint16_t tag_len = 0, left_len = 0;
    uint8_t *p = NULL;
    sha256_ctx profile_ctx;
    int32_t size = 0;

    p = get_tbh_request(profile_len, &tag_len, &left_len);

    size = get_length(p, 0) + get_length(p, 1);
    sha256_init(&profile_ctx);
    sha256_update(&profile_ctx, p, size);
    sha256_final(&profile_ctx, profile_hash);
    return RT_SUCCESS;
}

static int32_t build_profile(uint8_t *profile_buffer, int32_t profile_len, int32_t selected_profile_index) {
    BootstrapRequest_t *bootstrap_request = NULL;
    asn_dec_rval_t dc;
    asn_enc_rval_t ec;
    uint8_t profile_hash[32];

    dc = ber_decode(NULL, &asn_DEF_BootstrapRequest, (void **) &bootstrap_request, profile_buffer, profile_len);
    if (dc.code != RC_OK) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        return RT_ERROR;// 报错
    }
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

    ec = der_encode(&asn_DEF_BootstrapRequest, bootstrap_request, encode_cb_fun, NULL);
    if (ec.encoded == -1) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        return RT_ERROR;// 报错
    }
    update_hash(profile_len, profile_hash);

    rt_os_memcpy(bootstrap_request->hashCode.buf, profile_hash, bootstrap_request->hashCode.size);
    g_buf_size = 0;
    ec = der_encode(&asn_DEF_BootstrapRequest, bootstrap_request, encode_cb_fun, NULL);
    if (ec.encoded == -1) {
        MSG_PRINTF(LOG_ERR, "consumed:%ld\n", dc.consumed);
        return RT_ERROR;// 报错
    }

    if (bootstrap_request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_BootstrapRequest, bootstrap_request);
    }

    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_SETTING_PROFILE, profile_buffer1, profile_len);
    return RT_SUCCESS;
}

static int32_t decode_profile_info(rt_fshandle_t fp, uint16_t off, int32_t random) {
    int32_t selected_profile_index, profile_len, size;
    uint8_t buf[300];
    ProfileInfo1_t *request = NULL;
    asn_dec_rval_t dc;
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 300, fp);
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
        printf("\n%ld\n", dc.consumed);
        return 0;// 报错
    }
    off += size;
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    off += get_length(buf, 1);

    MSG_PRINTF(LOG_ERR, "apn:%s\n", request->apn.list.array[0]->apnName.buf);
    rt_qmi_modify_profile(1, 0, request->apn.list.array[0]->apnName.buf, 0);

    selected_profile_index = random % request->totalNum;
    if (request->sequential == 0xFF) {
        profile_len = get_length(buf, 0);
    } else {
        profile_len = get_length(buf, 0) / request->totalNum;
        off += selected_profile_index * profile_len;
    }
    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileInfo1, request);
    }

    uint8_t profile_buffer[profile_len];

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(profile_buffer, 1, profile_len, fp);

    MSG_PRINTF(LOG_ERR, "selected_profile_index:%d  off:%d\n", selected_profile_index, off);
    if (request->sequential == 0xFF) { // 连号profile处理
        build_profile(profile_buffer, profile_len, selected_profile_index);
    } else { // 非连号profile处理
        MSG_INFO_ARRAY("profile:", profile_buffer, profile_len);
        msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_SETTING_PROFILE, profile_buffer, profile_len);
    }

    return RT_SUCCESS;
}

int32_t selected_profile(int32_t random)
{
    rt_fshandle_t fp;
    uint8_t buf[8];
    uint16_t off = data.operator_info_offset;
    uint16_t profile_len, selected_profile_index;
    int32_t i = 0;

    fp = rt_fopen(SHARE_PROFILE, RT_FS_READ);
    if (fp == NULL) {
        MSG_PRINTF(LOG_ERR, "Open file failed\n");
        return RT_ERROR;
    }
    printf("1.off:%d\n", off);
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    MSG_INFO_ARRAY("opt:", buf, 8);
    if (buf[0] != 0xA3) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        return RT_ERROR;
    }
    off += 4;

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    if (buf[0] != 0x30) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        return RT_ERROR;
    }
    printf("2.off:%d\n", off);
    printf("data.priority:%d\n", data.priority);
    printf("data.operator_num:%d\n", data.operator_num);

    // 启卡次数大于运营商个数则重置
    if (data.priority >= data.operator_num) {
        data.priority = 0;
    }
    // 根据启卡次数计算应选运营商的偏移量
    for (i = 0; i < data.priority; i++) {
        off += 4;
        printf("buf:%02X%02X%02X%02X\n", buf[0], buf[1], buf[2], buf[3]);
        if (buf[1] == ASN1_LENGTH_2BYTES) {
            off += ((uint16_t) buf[2] << 8) + buf[3];
        } else if (buf[1] == ASN1_LENGTH_1BYTES) {
            off += buf[2] - 1;
        } else if ((buf[1] & 0x80) == 0) {
            off += buf[1] - 2;
        }
        printf("3.off:%d\n", off);
    }
    decode_profile_info(fp, off, random);

    if (fp != NULL) {
        rt_fclose(fp);
    }
    data.priority++;// todo 选卡成功后更改运营商
    return RT_SUCCESS;
}

int32_t init_profile_file(int32_t *arg)
{
    int32_t ret = RT_SUCCESS;
    uint8_t buf[500];
    uint16_t len = 0;
    rt_fshandle_t fp;

    fp = rt_fopen(SHARE_PROFILE, RT_FS_READ);
    if (fp == NULL) {
        return RT_ERROR;
    }
    data.file_info_offset = rt_get_file_info_offset(fp, buf, &len);
    data.root_sk_offset = rt_get_root_sk_offset(fp, buf, &len);
    data.aes_key_offset = rt_get_aes_key_offset(fp, buf, &len);
    data.operator_info_offset = rt_get_operator_profile_offset(fp, buf, &len);
    data.hash_code_offset = rt_get_hash_code_offset(fp);

    if (fp != NULL) {
        rt_fclose(fp);
    }

    data.priority = 0;
    data.operator_num = 2;
    return ret;
}
