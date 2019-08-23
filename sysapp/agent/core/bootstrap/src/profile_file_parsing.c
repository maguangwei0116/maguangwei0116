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
#include "tlv.h"

int32_t share_profile_num;  // share_profile数目
uint16_t share_profile_length;  // share_profile长度
int16_t share_profile_sequential;  // 是否连续为share_profile
int16_t profile_offset;  // profile偏移量

typedef struct profile_offset {
    uint16_t file_info;
    uint16_t root_sk;
    uint16_t aes_key;
    uint16_t operator_info;
    uint16_t hash_code;
} profile_offset_t;

typedef struct profile_info {
    uint16_t total_num;
    uint16_t sequential;
    uint16_t length;
} profile_t;

profile_offset_t offset;
profile_t profile;

static int parsing_file_info(const char *file_name, uint8_t type, uint8_t *asset, uint16_t *size) {
    int ret = 0;
    uint8_t buf[4];
    uint16_t offset;
    rt_fshandle_t fp;
    int i;

    fp = rt_fopen(file_name, RT_FS_READ);
    if (fp == NULL) {
        return 100;
    }

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

    rt_fread(buf, 1, 2, fp);
    if (buf[0] != FILE_INFO) {
        return 110;
    }
    offset = 10;

    rt_fread(buf, 1, 4, fp);
    while (buf[0] != type) {
        offset += 4;
        if (buf[1] == ASN1_LENGTH_2BYTES) {
            offset += ((uint16_t) buf[2] << 8) + buf[3];
        } else if (buf[1] == ASN1_LENGTH_1BYTES || buf[1] == 0x22) {
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

    if (type == FILE_INFO || type == ROOT_SK || type == PROFILE_KEY) {
        offset += 2;
        *size = buf[1];
    } else {
        offset += 4;
        *size = ((uint16_t) buf[2] << 8) + buf[3];
    }
    if (asset != NULL) {
        rt_fseek(fp, offset, RT_FS_SEEK_SET);
        rt_fread(asset, 1, *size, fp);
        for (i = 0; i < *size; ++i) {
            printf("%02X", asset[i]);
        }
    }

    if (fp != NULL) {
        rt_fclose(fp);
    }
    return ret;
}

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

static int parsing_profile(const char *file_name, uint8_t type, uint8_t *asset, uint16_t *size) {
    int ret = 0;
    uint8_t buf[4];
    uint16_t offset;
    rt_fshandle_t fp;
    int i;

    fp = rt_fopen(file_name, RT_FS_READ);
    if (fp == NULL) {
        return 100;
    }

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
        } else if (buf[1] == ASN1_LENGTH_1BYTES) {
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

    if (type == OPT_PROFILES) {
        offset += 2;
        *size = ((uint16_t) buf[2] << 8) + buf[3];
    } else {
        offset += 4;
        *size = ((uint16_t) buf[2] << 8) + buf[3];
    }

    rt_fseek(fp, offset, RT_FS_SEEK_SET);
    rt_fread(asset, 1, 4, fp);

    if (asset != NULL) {
        rt_fseek(fp, offset, RT_FS_SEEK_SET);
        rt_fread(asset, 1, *size, fp);
        for (i = 0; i < *size; ++i) {
            printf("%02X", asset[i]);
        }
    }

    if (fp != NULL) {
        rt_fclose(fp);
    }
    return ret;
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

static int rt_get_profile_info(const char *file_name, uint8_t *sk, uint16_t *size) {
    return parsing_profile(file_name, OPT_PROFILES, sk, size);
}

static uint16_t rt_get_hash_code(rt_fshandle_t fp) {
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

//static int rt_get_profile(const char *file_name, uint8_t *sk, uint16_t *size) {
//    return parsing_profile_file1(file_name, 0xA3, sk, size);
//}

static uint8_t *get_share_profile(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x30, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0x30, p, *left_len, tag_len, left_len);
    }
    return p;
}

static uint8_t *get_operator_profile(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0xA3, buffer, len, tag_len, left_len);
    if (p) {
        p = get_simple_tlv(0x30, p, *left_len, tag_len, left_len);
    }
    return p;
}

static void get_profile_length(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    get_simple_tlv(0x81, buffer, len, tag_len, left_len);
}

int parsing_profile_file() {
    int ret;
    uint16_t tag_len = 0;
    uint16_t left_len = 0;
    uint16_t profile_len = 0;
    uint8_t buf[2048];
    uint8_t *p, *total_num, *sequential;
    int i;
    rt_fshandle_t fp;

    fp = rt_fopen("continuous_profile.der", RT_FS_READ);
    if (fp == NULL) {
        return 100;
    }

    rt_fread(buf, 1, 300, fp);
    p = get_share_profile(buf, 300, &tag_len, &left_len);

    p = get_operator_profile(p, left_len, &tag_len, &left_len);

    // profile个数
//    total_num = get_total_num(p, left_len, &tag_len, &left_len);
    // 是否连续profile，00为否、FF为是
//    sequential = get_sequential(p, left_len, &tag_len, &left_len);

    get_profile_length(p, left_len, &tag_len, &left_len);

    share_profile_length = tag_len / (uint16_t) * total_num;

    share_profile_num = *total_num;

    if (fp != NULL) {
        rt_fclose(fp);
    }

    return ret;
}

static uint8_t get_total_num(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x81, buffer, len, tag_len, left_len);
    return p;
}

static uint8_t get_sequential(uint8_t *buffer, uint16_t len, uint16_t *tag_len, uint16_t *left_len) {
    uint8_t *p = NULL;
    p = get_simple_tlv(0x82, buffer, len, tag_len, left_len);
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

static uint16_t get_profile_offset(rt_fshandle_t fp, uint16_t off) {
    uint8_t *p;
    uint16_t tag_len = 0;
    uint16_t left_len = 0;
    uint8_t buf[300];

    rt_fseek(fp, off, RT_FS_SEEK_SET);
    rt_fread(buf, 1, 300, fp);

    off += get_length(buf, 1);
    p = get_profile_info(buf, 300, &tag_len, &left_len);
    off += get_length(p, 1);
    off += tag_len;

//    profile.total_num = get_total_num(p, left_len, &tag_len, &left_len);
//    profile.sequential = get_sequential(p, left_len, &tag_len, &left_len);
//
//    for (int i = 0; i < ; ++i) {
//
//    }
//    printf("%02X%02X%02X%02X\n", p[0], p[1], p[2], p[3]);
    return off;

}

int num = 0;
int operator_num = 2;

static int selected_profile(/*int random, int priority*/) {
    rt_fshandle_t fp;
    uint8_t buf[4];
    uint16_t off = offset.operator_info;
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
    profile.length = get_length(buf, 0) / profile.total_num;
    if (profile.sequential == 0xFF) {

    } else {

    }

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
    offset.hash_code = rt_get_hash_code(fp);

    if (fp != NULL) {
        rt_fclose(fp);
    }
    ret = selected_profile();
    return ret;
}
