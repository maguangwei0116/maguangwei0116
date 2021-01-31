
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

#include "profile_parse.h"
#include "file.h"
#include "ProfileInfo1.h"
#include "FileInfo.h"
#include "TBHRequest.h"
#include "SetRootKeyRequest.h"
#include "BootstrapRequest.h"
#include "tlv.h"
#include "bertlv.h"
#include "agent_queue.h"
#include "convert.h"
#include "hash.h"
#include "rt_qmi.h"

typedef struct PLMN_INFO {
    unsigned int mcc;
    unsigned char rplmn[7];
    unsigned char hplmn[7];
} plmn_info_t;

static const plmn_info_t g_rt_plmn[] = {
    {202,   "02F250",    "23450"},
    {204,   "02F440",    "23450"},
    {208,   "02F810",    "23450"},
    {214,   "12F410",    "23450"},
    {216,   "12F607",    "23450"},
    {219,   "12F901",    "23450"},
    {220,   "22F010",    "23450"},
    {222,   "22F201",    "23450"},
    {226,   "22F610",    "23450"},
    {228,   "22F820",    "23450"},
    {230,   "32F030",    "23450"},
    {231,   "32F120",    "23450"},
    {232,   "32F250",    "23450"},
    {234,   "32F451",    "23450"},
    {238,   "32F802",    "23450"},
    {240,   "42F010",    "23450"},
    {242,   "42F210",    "23450"},
    {244,   "42F419",    "23450"},
    {246,   "42F610",    "23450"},
    {247,   "42F710",    "23450"},
    {248,   "42F810",    "23450"},
    {250,   "52F099",    "23450"},
    {255,   "52F530",    "23450"},
    {257,   "",          "23450"},
    {259,   "52F920",    "23450"},
    {260,   "62F060",    "23450"},
    {260,   "62F060",    "23450"},
    {262,   "62F220",    "23450"},
    {268,   "62F810",    "23450"},
    {270,   "72F010",    "23450"},
    {272,   "72F210",    "23450"},
    {274,   "72F410",    "23450"},
    {278,   "72F810",    "23450"},
    {280,   "82F010",    "23450"},
    {284,   "82F450",    "23450"},
    {286,   "82F620",    "23450"},
    {293,   "92F314",    "23450"},
    {295,   "92F550",    "23450"},
    {297,   "92F730",    "23450"},
    {302,   "302220",    "23450"},
    {310,   "310260",    "23450"},
    {400,   "04F020",    "23450"},
    {401,   "04F110",    "23450"},
    {404,   "",          "23450"},
    {405,   "",          "23450"},
    {414,   "14F460",    "23450"},
    {424,   "24F430",    "23450"},
    {425,   "24F530",    "23450"},
    {429,   "",          "23450"},
    {440,   "44F002",    "23450"},
    {450,   "54F080",    "23450"},
    {454,   "54F400",    "23450"},
    {455,   "54F510",    "23450"},
    {456,   "",          "23450"},
    {457,   "54F780",    "23450"},
    {460,   "64F000",    "23450"},
    {466,   "64F629",    "23450"},
    {502,   "05F261",    "23450"},
    {505,   "05F520",    "23450"},
    {510,   "15F010",    "23450"},
    {515,   "15F530",    "23450"},
    {520,   "25F030",    "23450"},
    {525,   "25F510",    "23450"},
    {530,   "35F010",    "23450"},
    {716,   "17F660",    "23450"},
    {722,   "27F270",    "23450"},
    {724,   "",          "23450"},
    {730,   "37F020",    "23450"},
    {732,   "732123",    "23450"},
    {734,   "37F440",    "23450"},
    {740,   "47F000",    "23450"},
    {748,   "47F870",    "23450"},
};

static profile_data_t g_data;
static uint8_t *g_buf = NULL;
static uint16_t g_buf_size = 0;
static char g_share_profile[128];

#define SHARED_PROFILE_NAME     "sharedprofile"

static rt_fshandle_t open_share_profile(const char *file_name, rt_fsmode_t mode)
{
#if (CFG_OPEN_MODULE)
    return linux_rt_fopen(file_name, mode);
#elif (CFG_STANDARD_MODULE)  // standard
    return linux_fopen(file_name, mode);
#endif
}

static int32_t sizeof_share_profile(const char *file_name)
{
    int32_t file_size = 0;

#if (CFG_OPEN_MODULE)
    file_size = linux_rt_file_size(file_name);
#elif (CFG_STANDARD_MODULE)  // standard
    file_size = linux_file_size(file_name);
#endif

    return file_size;
}

static uint32_t get_offset(rt_fshandle_t fp, uint8_t type, uint32_t *size)
{
    int ret = 0;
    uint8_t buf[9];
    uint32_t offset = 0;

    linux_fseek(fp, offset, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    if ((buf[0] != SHARED_PROFILE)) {
        return RT_ERROR;
    }
    offset += get_length(buf, 1);
    linux_fseek(fp, offset, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    if ((buf[0] != SHARED_PROFILE)) {
        return RT_ERROR;
    }
    offset += get_length(buf, 1);
    linux_fseek(fp, offset, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    while (buf[0] != type) {
        offset += get_length(buf, 1) + get_length(buf, 0);
        linux_fseek(fp, offset, RT_FS_SEEK_SET);
        if (linux_fread(buf, 1, 8, fp) != 8) {
            return RT_ERROR;
        }
    }
    return offset;
}

static uint16_t rt_init_file_info(rt_fshandle_t fp)
{
    uint8_t *p = NULL;
    uint8_t buf[100];

    linux_fseek(fp, g_data.file_info_offset, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 100, fp);
    p = get_value_buffer(buf);
    g_data.file_version_offset = g_data.file_info_offset + get_length(buf, 1) + get_length(p, 0) + get_length(p, 1);
    p = get_value_buffer(p);
    g_data.operator_num = p[0];

    MSG_PRINTF(LOG_DBG, "operator_num:%d\n", g_data.operator_num);
    MSG_PRINTF(LOG_TRACE, "file_version_offset:%d\n", g_data.file_version_offset);
}

static uint32_t rt_get_root_sk_offset(rt_fshandle_t fp, uint32_t *size)
{
    return get_offset(fp, ROOT_SK, size);
}

static uint32_t rt_get_aes_key_offset(rt_fshandle_t fp, uint32_t *size)
{
    return get_offset(fp, PROFILE_KEY, size);
}

static uint32_t rt_get_file_info_offset(rt_fshandle_t fp, uint32_t *size)
{
    return get_offset(fp, FILE_INFO, size);
}

static uint32_t rt_get_operator_profile_offset(rt_fshandle_t fp, uint32_t *size)
{
    return get_offset(fp, OPT_PROFILES, size);
}

static int32_t rt_check_hash_code_offset(rt_fshandle_t fp)
{
    uint8_t buf[BLOCK_SIZE] = {0};
    uint8_t hash_code_buf[HASH_CODE_LENGTH] = {0};
    uint8_t original_hash[HASH_CODE_LENGTH] = {0};
    uint8_t p[BLOCK_SIZE] = {0};
    uint32_t hash_off = 0;
    uint32_t profile_off = 0;
    uint32_t index = 0;
    sha256_ctx_t hash_code;
    int32_t file_size;

    file_size = sizeof_share_profile(g_share_profile);
    linux_fseek(fp, 0, RT_FS_SEEK_SET);
    rt_os_memset(buf, 0, sizeof(buf));
    linux_fread(buf, 1, sizeof(buf), fp);
    profile_off = get_length(buf,1);
    hash_off += profile_off;

    linux_fseek(fp, profile_off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, sizeof(buf), fp);
    hash_off += get_length(buf, 0) + get_length(buf, 1);
    MSG_PRINTF(LOG_TRACE, "file_size=%d, profile_off=%d, hash_off=%d\n", file_size, profile_off, hash_off);

    linux_fseek(fp, hash_off, RT_FS_SEEK_SET);
    rt_os_memset(buf, 0, sizeof(buf));
    linux_fread(buf, 1, sizeof(buf), fp);
    if (buf[0] != HASH_CODE || buf[1] != HASH_CODE_LENGTH) {
        MSG_PRINTF(LOG_ERR, "hash buffer failed %02x, %02x\n", buf[0], buf[1]);
        return RT_ERROR;
    }
    rt_os_memcpy(original_hash, get_value_buffer(buf), HASH_CODE_LENGTH);
    if (file_size < (hash_off + get_length(buf, 0) + get_length(buf, 1))){
        MSG_PRINTF(LOG_ERR, "The share profile is damaged.\n");
        return RT_ERROR;
    }

    linux_fseek(fp, profile_off, RT_FS_SEEK_SET);
    sha256_init(&hash_code);
    for (profile_off; profile_off < hash_off; profile_off += index){
        index = linux_fread(p, 1, (hash_off - profile_off > BLOCK_SIZE) ? BLOCK_SIZE : hash_off - profile_off, fp);
        sha256_update(&hash_code, p, index);
    }
    sha256_final(&hash_code, hash_code_buf);

    if (rt_os_memcmp(original_hash, hash_code_buf, HASH_CODE_LENGTH) != 0){
        MSG_PRINTF(LOG_ERR, "Share profile hash check failed\n");
        return RT_ERROR;
    }
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

static int32_t calc_hash(uint8_t *tbh, int32_t tbh_len, uint8_t *profile_hash)
{
    sha256_ctx_t profile_ctx;

    sha256_init(&profile_ctx);
    sha256_update(&profile_ctx, tbh, tbh_len);
    sha256_final(&profile_ctx, profile_hash);
    MSG_INFO_ARRAY("Current profile_hash: ", profile_hash, 32);
    return RT_SUCCESS;
}

static int32_t build_profile(uint8_t *profile_buffer, int32_t profile_len, int32_t selected_profile_index,
                            BOOLEAN_t sequential, uint16_t mcc, uint8_t *profile, uint16_t *len_out)
{
    uint8_t jt[4] = {0x08, 0x29, 0x43, 0x05};
    uint8_t profile_hash[32], imsi_buffer[2], bytes[12];
    uint16_t length;
    uint16_t tag;
    int32_t ret = RT_ERROR;
    int32_t i, imsi;
    uint32_t br_off, br_len;
    uint32_t tbh_off, tbh_len;
    uint32_t iccid_off, iccid_len;
    uint32_t imsi_off, imsi_len;
    uint32_t rplmn_off, rplmn_len;

    /*
    BootstrapRequest ::= [PRIVATE 127] SEQUENCE { --Tag ‘FF7F’
        tbhRequest TBHRequest,
        hashCode [APPLICATION 1] OCTET STRING     -- sha256
    }
    TBHRequest ::= SEQUENCE {
        iccid OCTET STRING,             -- ICCID为卡片文件存储格式，例：98680021436587092143, 20位
        imsi OCTET STRING,              -- IMSI为卡片文件存储格式，例如 084906001111212299，18 位
        key OCTET STRING,               -- 密文，32 位，Hex 格式
        opc OCTET STRING,               -- 密文，32 位，Hex 格式
        rotation OCTET STRING OPTIONAL, -- optional，遵从 SIM 格式，默认 4000204060，
        xoring OCTET STRING OPTIONAL,   -- optional, 遵从 SIM 格式，默认
        sqnFlag OCTET STRING OPTIONAL,  -- optional,
        rplmn OCTET STRING OPTIONAL,    -- optional, 单个，SIM 卡片文件存储格式，64F010，默认 FFFFFF
        fplmn OCTET STRING OPTIONAL,    -- optional, 多个，同上
        hplmn OCTET STRING OPTIONAL,    -- optional, SIM 卡片文件存储格式，多个，例：64F0100000 64F000000，默认 FFFFFF0000
        ehplmn OCTET STRING OPTIONAL,   -- optional, 同上
        oplmn OCTET STRING OPTIONAL     -- optional, 同上
    }
    */

    // BootstrapRequest value
    br_off = bertlv_get_tl_length(profile_buffer, &br_len);

    // tbhRequest value
    tbh_off = bertlv_get_tl_length(profile_buffer + br_off, &tbh_len);
    tbh_off += br_off;

    // find ICCID in tbhRequest
    iccid_off = bertlv_find_tag(profile_buffer + tbh_off, tbh_len, 0x80, 1);
    if (iccid_off == BERTLV_INVALID_OFFSET) {
        MSG_PRINTF(LOG_ERR, "iccid not found!");
        goto end;
    }
    iccid_off += tbh_off;
    // iccid value
    iccid_off += bertlv_get_tl_length(profile_buffer + iccid_off, &iccid_len);

    // find IMSI in tbhRequest
    imsi_off = bertlv_find_tag(profile_buffer + tbh_off, tbh_len, 0x81, 1);
    if (imsi_off == BERTLV_INVALID_OFFSET) {
        MSG_PRINTF(LOG_ERR, "imsi not found!");
        goto end;
    }
    imsi_off += tbh_off;
    // imsi value
    imsi_off += bertlv_get_tl_length(profile_buffer + imsi_off, &imsi_len);

    // copy tbhRequest
    utils_mem_copy(g_buf, profile_buffer + tbh_off, tbh_len);

    if (sequential == 0xFF) {
        imsi_buffer[0] = g_buf[imsi_off - tbh_off + 7];
        imsi_buffer[1] = g_buf[imsi_off - tbh_off + 8];
        swap_nibble(imsi_buffer, 2);
        imsi = selected_profile_index + utils_u08s_to_u16(imsi_buffer);
        utils_u16_to_u08s(imsi, imsi_buffer);
        swap_nibble(imsi_buffer, 2);
        // update new IMSI
        g_buf[imsi_off - tbh_off + 7] = imsi_buffer[0];
        g_buf[imsi_off - tbh_off + 8] = imsi_buffer[1];
    }

    {
        uint8_t buffer[21];
        char select_buffer[21];

        //MSG_INFO_ARRAY("selected imsi: ", bootstrap_request->tbhRequest.imsi.buf, bootstrap_request->tbhRequest.imsi.size);
        //MSG_INFO_ARRAY("selected iccid: ", bootstrap_request->tbhRequest.iccid.buf, bootstrap_request->tbhRequest.iccid.size);

        rt_os_memset(buffer, 0 ,sizeof(buffer));
        rt_os_memcpy(buffer, profile_buffer + imsi_off, imsi_len);
        swap_nibble(buffer, imsi_len);
        bytes2hexstring(buffer, imsi_len, select_buffer);
        MSG_PRINTF(LOG_DBG, "selected_imsi : %s\n", &select_buffer[3]);
        rt_os_memset(buffer, 0 ,sizeof(buffer));
        rt_os_memcpy(buffer, profile_buffer + iccid_off, iccid_len);
        swap_nibble(buffer, iccid_len);
        bytes2hexstring(buffer, iccid_len, select_buffer);
        MSG_PRINTF(LOG_DBG, "selected_iccid: %s\n", select_buffer);
    }

    if (rt_os_memcmp(profile_buffer + imsi_off, jt, 4) == 0){
        for (i = 0; i < ARRAY_SIZE(g_rt_plmn); ++i) {
            if (mcc == g_rt_plmn[i].mcc) {
                hexstring2bytes(g_rt_plmn[i].rplmn, bytes, &length); // must convert string to bytes
                MSG_PRINTF(LOG_INFO, "selected_rplmn : %s\n", g_rt_plmn[i].rplmn);
                
                // find rplmn in tbhRequest
                rplmn_off = bertlv_find_tag(g_buf, tbh_len, 0x87, 1);
                if (rplmn_off == BERTLV_INVALID_OFFSET) {
                    rplmn_len = 0;
                    // find insert offset
                    for (tag = 0x88; tag >= 0x8B; tag++) {
                        rplmn_off = bertlv_find_tag(g_buf, tbh_len, tag, 1);
                        if (rplmn_off != BERTLV_INVALID_OFFSET) {
                            break;
                        }
                    }
                    // if tag from '88' fplmn to '8B' oplmn all not found, set to end of tlv list
                    if (rplmn_off == BERTLV_INVALID_OFFSET) {
                        rplmn_off = tbh_len;
                    }
                } else {
                    rplmn_len = bertlv_get_tlv_length(g_buf + rplmn_off);
                }
                // modify or insert it
                if (length != 0) {
                    length = bertlv_build_tlv(0x87, length, bytes, bytes);
                }

                utils_mem_copy(g_buf + rplmn_off + length, g_buf + rplmn_off + rplmn_len, tbh_len - rplmn_off - rplmn_len);
                utils_mem_copy(g_buf + rplmn_off, bytes, length);
                tbh_len = tbh_len + length - rplmn_len;  // new tbh length

                break;
            }
        }
    }

    // build tbhRequest TLV
    g_buf_size = bertlv_build_tlv(0x30, tbh_len, g_buf, tbh_len);
    calc_hash(g_buf, g_buf_size, profile_hash);
    // build hash TLV
    g_buf_size += bertlv_build_tlv(0x41, profile_hash, 32, g_buf + g_buf_size);
    // build new BootstrapRequest
    g_buf_size = bertlv_build_tlv(0xFF7F, g_buf_size, g_buf, g_buf);

    rt_os_memcpy(profile, g_buf, g_buf_size);
    *len_out = g_buf_size;
    ret = RT_SUCCESS;

end:
    return ret;
}

static uint32_t get_selecte_profile_index(uint32_t total_num)
{
    static uint32_t g_selected_index = 0xFFFFFFFF;
    uint32_t random;
    uint32_t index;

    while (1) {
        random = (uint32_t)rt_get_random_num();
        index = random % total_num;

        /* never select the last selected card */
        if (g_selected_index != index) {
            g_selected_index = index;
            break;
        }

        if (total_num == 1) {
            break;
        }
        rt_os_msleep(100);
    }

    MSG_PRINTF(LOG_INFO, "The selected index/total = [%d/%d], random = %u\n", index+1, total_num, random);
    return index;
}

static int32_t decode_profile_info(rt_fshandle_t fp, uint32_t off, uint16_t mcc, char *apn, char *mcc_mnc, uint8_t *profile, uint16_t *len_out)
{
    uint32_t selected_profile_index, profile_len, size;
    uint8_t *profile_buffer = NULL;
    uint8_t buf[100];
    int32_t ret = RT_ERROR;
    uint32_t apn_off, apn_len;
    uint32_t apn_name_off, apn_name_len;
    uint32_t mcc_mnc_off, mcc_mnc_len;
    uint32_t total_num, total_num_len;;
    uint8_t sequential;

    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 100, fp);
    off += get_length(buf, 1);

    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 100, fp);
    size = get_length(buf, 0) + get_length(buf, 1);

    MSG_INFO_ARRAY("Profile Info: ", buf, size);

    apn_off = bertlv_get_tl_length(buf, &apn_len);  // ProfileInfo1 A0 TL
    apn_name_off = bertlv_get_tl_length(buf + apn_off, &apn_len);  // apn A0 TL
    apn_name_off += apn_off;
    apn_name_off += bertlv_get_tl_length(buf + apn_name_off, &apn_name_len);  // ApnList 30 TL
    apn_name_off += bertlv_get_tl_length(buf + apn_name_off, &apn_name_len);  // apnName 80 TL

    mcc_mnc_off = bertlv_get_tl_length(buf + apn_name_off + apn_name_len, &mcc_mnc_len);  // mccMnc 81 TL
    mcc_mnc_off += apn_name_off + apn_name_len;
    // get total number
    total_num = bertlv_get_integer(buf + apn_off + apn_len, &total_num_len);
    // get sequential BOOLEAN
    sequential = (uint8_t)bertlv_get_integer(buf + apn_off + apn_len + total_num_len, NULL);

    off += size;
    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    off += get_length(buf, 1);

    utils_mem_copy(apn, buf + apn_name_off, apn_name_len);
    apn[apn_name_len] = '\0';
    utils_mem_copy(mcc_mnc, buf + mcc_mnc_off, mcc_mnc_len);
    apn[mcc_mnc_len] = '\0';
    MSG_PRINTF(LOG_INFO, "apn:%s  mcc_mnc:%s\n", apn, mcc_mnc);

    selected_profile_index = get_selecte_profile_index(total_num);

    if (sequential == 0xFF) { // Successive profile: same iccid, successive imsi
        profile_len = get_length(buf, 0);
    } else { // non-successive profile
        profile_len = get_length(buf, 0) / total_num;
        off += selected_profile_index * profile_len;
    }
    profile_buffer = (uint8_t *) rt_os_malloc(profile_len);
    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(profile_buffer, 1, profile_len, fp);

    build_profile(profile_buffer, profile_len, selected_profile_index, request->sequential, mcc, profile, len_out);
    rt_os_free(profile_buffer);
    ret = RT_SUCCESS;
end:
    return ret;
}

static int32_t get_specify_data(uint8_t *data, int32_t *data_len, uint32_t offset)
{
    rt_fshandle_t fp = NULL;
    int32_t length = 0;
    uint8_t buf[256];
    uint8_t *buffer = NULL;

    fp = open_share_profile(g_share_profile, RT_FS_READ);
    if (!fp) {
        return RT_ERROR;
    }

    linux_fseek(fp, offset, RT_FS_SEEK_SET);
    linux_fread(buf, 1, sizeof(buf), fp);

    length = get_length(buf, 0);
    buffer = get_value_buffer(buf);
    if (data){
        rt_os_memcpy(data, buffer, length);
    }
    if (data_len) {
        *data_len = length;
    }
    if (fp) {
        linux_fclose(fp);
        fp = NULL;
    }
    return RT_SUCCESS;
}

int32_t bootstrap_get_key(void)
{
    uint8_t data[512];
    int32_t data_len = 0;
    int32_t ret = RT_ERROR;

    get_specify_data(data, &data_len, g_data.root_sk_offset);
    g_buf_size = bertlv_build_tlv(0x80, data_len, data, g_buf);

    get_specify_data(data, &data_len, g_data.aes_key_offset);
    g_buf_size += bertlv_build_tlv(0x81, data_len, data, g_buf + g_buf_size);

    g_buf_size = bertlv_build_tlv(TAG_LPA_SET_ROOT_KEY_REQ, g_buf_size, g_buf, g_buf);

    msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_SETTING_KEY, g_buf, g_buf_size);
    ret = RT_SUCCESS;
end:
    //ASN_STRUCT_FREE(asn_DEF_SetRootKeyRequest, &key_request);
    rt_os_free(g_buf);
    g_buf = NULL;

    return ret;
}

int32_t get_share_profile_version(char *batch_code, int32_t b_size, char *version, int32_t v_size)
{
    int32_t ret = RT_ERROR;
    int32_t data_len = 0;
    char version_data[256] = {0}; /* sample: B191031023631863078:skb-beta-0.0.2:aes-beta-0.0.3 */

    ret = get_specify_data(version_data, &data_len, g_data.file_version_offset);
    if (!ret) {
        char *p = NULL;
        char *p0 = version_data;
        char tmp[64] = {0};

        p = rt_os_strchr(version_data, ':');
        if (p) {
            rt_os_memcpy(tmp, p0, p - p0);
            if (batch_code) {
                snprintf(batch_code, b_size, "%s", tmp);
            }
            if (version) {
                snprintf(version, v_size, "%s", p+1);
            }
        }
    }

    return ret;
}

int32_t verify_profile_file(rt_bool absolute_path, const char *file)
{
    int32_t ret = RT_SUCCESS;

#ifdef CFG_SHARE_PROFILE_ECC_VERIFY
    char real_file_name[32] = {0};
    char absolute_file_path[256] = {0};

    if (absolute_path) {
        /* absolute path already */
        snprintf(absolute_file_path, sizeof(absolute_file_path), "%s", file);
    } else {
        /* get file path */
        linux_rt_file_abs_path(file, absolute_file_path, sizeof(absolute_file_path));
    }

    ret = ipc_file_verify_by_monitor(absolute_file_path, real_file_name);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "share profile verify fail !\n");
        return ret;
    }

    if (rt_os_strcmp(SHARED_PROFILE_NAME, real_file_name)) {
        MSG_PRINTF(LOG_ERR, "share profile name unmatched !\n");
        ret = RT_ERROR;
    }
#endif

    return ret;
}

int32_t init_profile_file(const char *file)
{
    int32_t ret = RT_ERROR;
    uint32_t len = 0;
    rt_fshandle_t fp = NULL;

    if (file) {
        snprintf(g_share_profile, sizeof(g_share_profile), "%s", (const char *)file);
    }

#if (CFG_OPEN_MODULE)
    if ((ret = verify_profile_file(RT_FALSE, g_share_profile)) == RT_ERROR) {
        return RT_ERROR;
    }
#elif (CFG_STANDARD_MODULE)  // standard
    if ((ret = verify_profile_file(RT_TRUE, g_share_profile)) == RT_ERROR) {
        return RT_ERROR;
    }
#endif

    fp = open_share_profile(g_share_profile, RT_FS_READ);
    if (!fp) {
        MSG_PRINTF(LOG_ERR, "g_share_profile: %s, fp is null !\n", g_share_profile);
        return RT_ERROR;
    }

    ret = rt_check_hash_code_offset(fp);
    if (ret == RT_SUCCESS){
        g_data.file_info_offset = rt_get_file_info_offset(fp, &len);
        rt_init_file_info(fp);
        g_data.root_sk_offset = rt_get_root_sk_offset(fp, &len);
        g_data.aes_key_offset = rt_get_aes_key_offset(fp, &len);
        g_data.operator_info_offset = rt_get_operator_profile_offset(fp, &len);
    }

    if (fp != NULL) {
        linux_fclose(fp);
        fp = NULL;
    }

    g_data.priority = 0;
    return ret;
}

int32_t selected_profile(uint16_t mcc, char *apn, char *mcc_mnc, uint8_t *profile, uint16_t *profile_len)
{
    rt_fshandle_t fp;
    uint8_t buf[8];
    uint32_t off = g_data.operator_info_offset;
    uint16_t selected_profile_index;
    int32_t ret = RT_ERROR;
    int32_t i = 0;

    fp = open_share_profile(g_share_profile, RT_FS_READ);
    if (!fp) {
        MSG_PRINTF(LOG_ERR, "Open file failed\n");
        return RT_ERROR;
    }
    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    if (buf[0] != OPT_PROFILES) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        goto end;
    }
    off += get_length(buf, 1);

    linux_fseek(fp, off, RT_FS_SEEK_SET);
    linux_fread(buf, 1, 8, fp);
    if (buf[0] != SHARED_PROFILE) {
        MSG_PRINTF(LOG_ERR, "Operator tag is error\n");
        goto end;
    }
    if (g_data.priority >= g_data.operator_num) {
        g_data.priority = 0;
    }
    for (i = 0; i < g_data.priority; i++) {
        off += get_length(buf, 1) + get_length(buf, 0);
        linux_fseek(fp, off, RT_FS_SEEK_SET);
        linux_fread(buf, 1, 8, fp);
    }
    decode_profile_info(fp, off, mcc, apn, mcc_mnc, profile, profile_len);

    g_data.priority++;
    ret = RT_SUCCESS;

end:

    if (fp) {
        linux_fclose(fp);
        fp = NULL;
    }
    return ret;
}
