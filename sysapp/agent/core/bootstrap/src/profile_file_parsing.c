//
// Created by admin on 2019-08-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include "rt_rplmn.h"
#include "TBHRequest.h"
#include "der_encoder.h"
#include "ber_decoder.h"
#include "file.h"
#include "profile_file_parsing.h"
#include "ProfileInfo1.h"
#include "Bootstrap_TBHRequest.h"
#include "Bootstrap_BootstrapRequest.h"
#include "tlv.h"

typedef struct profile_offset {
    uint16_t file_info;
    uint16_t root_sk;
    uint16_t aes_key;
    uint16_t operator_info;
    uint16_t hash_code;
} profile_offset_t;

int32_t share_profile_num;  // share_profile数目
uint16_t share_profile_length;  // share_profile长度
int16_t share_profile_sequential;  // 是否连续为share_profile
int16_t profile_offset;  // profile偏移量

profile_offset_t offset;
ProfileInfo1_t *request;

static uint16_t get_offset(rt_fshandle_t fp, uint8_t type, uint8_t *asset, uint16_t *size) {
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

static uint16_t rt_get_root_sk_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size) {
    return get_offset(fp, ROOT_SK, sk, size);
}

static uint16_t rt_get_aes_key_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size) {
    return get_offset(fp, PROFILE_KEY, sk, size);
}

static uint16_t rt_get_file_info_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size) {
    return get_offset(fp, FILE_INFO, sk, size);
}

static uint16_t rt_get_operator_profile_offset(rt_fshandle_t fp, uint8_t *sk, uint16_t *size) {
    return get_offset(fp, OPT_PROFILES, sk, size);
}

static uint16_t rt_get_hash_code_offset(rt_fshandle_t fp) {
    uint8_t buf[4];
    uint16_t off = 0;
    off = offset.operator_info;
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
    return off;
}

static uint8_t *get_share_profile(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x30, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0x30, p, *left_len, tag_len, left_len);
    }
    return p;
}

static uint8_t *get_profile_info(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x30, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0xA0, p, *left_len, tag_len, left_len);
    }
    return p;
}

static int decode_profile_info(rt_fshandle_t fp, uint16_t off) {
    uint8_t buf[100];
    int size;

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 100, fp);

    asn_dec_rval_t dc;

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

    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_ProfileInfo1, request);
    }

    return 0;
}

static uint16_t get_profile_offset(rt_fshandle_t fp, uint16_t off) {
    uint8_t *p;
    uint16_t tag_len = 0;
    uint16_t left_len = 0;
    uint8_t buf[300];

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 300, fp);

    off += get_length(buf, 1);
    decode_profile_info(fp, off);

    p = get_profile_info(buf, 300, &tag_len, &left_len);
    off += get_length(p, 1);
    off += tag_len;
    return off;
}

static uint8_t decode_profile(rt_fshandle_t fp, uint16_t off, int length) {
    uint8_t buf[length];

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, length, fp);

    Bootstrap_BootstrapRequest_t *request = NULL;
    asn_dec_rval_t dc;

    dc = ber_decode(NULL, &asn_DEF_Bootstrap_BootstrapRequest, (void **) &request, buf, length);

    if (dc.code != RC_OK) {
        printf("\nconsumed:%ld\n", dc.consumed);
        return 0;// 报错
    }

    if (request != NULL) {
        ASN_STRUCT_FREE(asn_DEF_Bootstrap_BootstrapRequest, request);
    }

    printf("size:%d\n", request->tbhRequest.imsi.size);
    return buf;
}

int num = 1;
int priority = 0;
int operator_num = 2;

int32_t selected_profile(int random,int enable_num) {
    rt_fshandle_t fp;
    uint8_t buf[4];
    uint16_t off = offset.operator_info;
    uint16_t profile_len, selected_profile_index;
    int i;

    fp = rt_fopen("continuous_profile.der", RT_FS_READ);
    if (fp == NULL) {
        return 100;
    }

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    if (buf[0] != 0xA3) {
        return -1;
    }

    rt_fread(buf, 1, 4, fp);
    if (buf[0] != 0x30) {
        return -1;
    }

    off += 4;

    // 启卡次数大于运营商个数则重置
    if (num >= operator_num) {
        num = 0;
    }
    // 根据启卡次数计算应选运营商的偏移量
    for (i = 0; i < num; i++) {
        off += 4;
        if (buf[1] == ASN1_LENGTH_2BYTES) {
            off += ((uint16_t) buf[2] << 8) + buf[3];
        } else if (buf[1] == ASN1_LENGTH_1BYTES) {
            off += buf[2] - 1;
        } else if ((buf[1] & 0x80) == 0) {
            off += buf[1] - 2;
        }
    }

    // todo：off已为选中运营商的偏移量
    off = get_profile_offset(fp, off);
    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 4, fp);
    off += get_length(buf, 1);

    selected_profile_index = random % request->totalNum;
    if (request->sequential == 0xFF) {
        profile_len = get_length(buf, 0);

    } else {
        profile_len = get_length(buf, 0) / request->totalNum;
    }
    decode_profile(fp, off, profile_len);

    if (fp != NULL) {
        rt_fclose(fp);
    }
}

int32_t init_profile_file(int32_t *arg) {
    int ret;
    uint8_t buf[500];
    uint16_t len = 0;
    rt_fshandle_t fp;

    fp = rt_fopen("continuous_profile.der", RT_FS_READ);
    if (fp == NULL) {
        return -1;
    }

    offset.file_info = rt_get_file_info_offset(fp, buf, &len);
    offset.root_sk = rt_get_root_sk_offset(fp, buf, &len);
    offset.aes_key = rt_get_aes_key_offset(fp, buf, &len);
    offset.operator_info = rt_get_operator_profile_offset(fp, buf, &len);
    offset.hash_code = rt_get_hash_code_offset(fp);

    if (fp != NULL) {
        rt_fclose(fp);
    }
    return ret;
}
