
#include "rt_type.h"
#include "rt_os.h"
#include "log.h"
#include "agent2monitor.h"
#include "hash.h"
#include "file.h"
#include "lpa.h"
#ifdef CFG_PLATFORM_ANDROID
#include "rt_qmi.h"
#else
#include "trigger.h"
#endif

int32_t ipc_get_uicc_atr(uint8_t *atr, uint8_t *atr_size)
{
    uint8_t data[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xEE};
    ipc_send_data((const uint8_t *)data, sizeof(data), atr, atr_size);
}

int32_t ipc_send_lpa_cmd(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    uint16_t sw = 0;
    static rt_bool reset_flag = RT_FALSE;

    cmd = (data[5] << 8) + data[6];
    ipc_send_data(data, data_len, rsp, rsp_len);

    /* enable profile and load bootstrap profile, need to reset */
    if ((cmd == 0xBF31) || (cmd == 0xFF7F)) {
        cmd = (rsp[0] << 8) + rsp[1];
        if ((cmd & 0xFF00) == 0x6100) {
            reset_flag = RT_TRUE;
            return RT_SUCCESS;
        } else {
            reset_flag = RT_TRUE;
        }
    }

    if (reset_flag == RT_TRUE) {
        reset_flag = RT_FALSE;
        trigger_swap_card(1);
    }
}


int32_t ipc_send_trigger_cmd(const uint8_t *apdu, uint16_t apdu_len, uint8_t *rsp, uint16_t *rsp_len)
{
    ipc_send_data((const uint8_t *)apdu, apdu_len, rsp, rsp_len);
}

void init_trigger(uint8_t uicc_switch)
{
    if (uicc_switch == LPA_CHANNEL_BY_IPC) {
        trigegr_regist_reset(ipc_get_uicc_atr);
        trigegr_regist_cmd(ipc_send_trigger_cmd);
        trigger_swap_card(1);
    }
}

int32_t ipc_set_monitor_param(config_info_t *config_info)
{
    info_vuicc_data_t info = {0};
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = sizeof(info_vuicc_data_t);
    uint16_t ret_len = 0;

    //MSG_PRINTF(LOG_INFO, "atom len:%d\n", sizeof(atom_data_t));

    info.vuicc_switch = config_info->lpa_channel_type;
    info.log_level = config_info->monitor_log_level;
    info.log_size = config_info->log_max_size;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = 0x00;
    c_data.data_len = (uint8_t)len;
    c_data.data = (uint8_t *)&info;
    len = sizeof(atom_data_t) - sizeof(uint8_t *);
    rt_os_memcpy(&buf[0], &c_data, len);
    rt_os_memcpy(&buf[len], c_data.data, c_data.data_len);
    len += c_data.data_len;

    //MSG_PRINTF(LOG_INFO, "len:%d, log_max_size:%d, %08x\n", len, info.log_size, info.log_level);

#ifdef CFG_PLATFORM_ANDROID
    {
        extern int32_t rt_qmi_exchange_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len);

        if (config_info->lpa_channel_type == LPA_CHANNEL_BY_QMI) {
            return RT_SUCCESS; /* neend't config monitor param */
        }

        rt_qmi_exchange_apdu((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);
    }
#else
    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);
    init_trigger(info.vuicc_switch);
#endif

    if (ret_len == 1 && buf[0] == RT_TRUE) {
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

int32_t ipc_get_monitor_version(char *name, int32_t n_size, char *version, \
                                        int32_t v_size, char *chip_modle, int32_t c_size)
{
    monitor_version_t m_version = {0};
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = 0;
    uint16_t ret_len = 0;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = CMD_GET_MONITOR_VER;
    c_data.data_len = (uint8_t)len;
    c_data.data = (uint8_t *)&m_version;
    len = sizeof(atom_data_t) - sizeof(uint8_t *);
    rt_os_memcpy(&buf[0], &c_data, len);
    len += c_data.data_len;

    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);

    if (ret_len == sizeof(monitor_version_t)) {
        rt_os_memcpy((uint8_t *)&m_version, (uint8_t *)buf, ret_len);
        //MSG_HEXDUMP("version", &m_version, sizeof(monitor_version_t));

        snprintf(name, n_size, "%s", m_version.name);
        snprintf(version, v_size, "%s", m_version.version);
        snprintf(chip_modle, c_size, "%s", m_version.chip_model);

        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

int32_t ipc_sign_verify_by_monitor(const char *hash, const char *sign)
{
#ifdef CFG_PLATFORM_ANDROID
    /* close signature data verify on android paltform */
    (void)hash;
    (void)sign;
    return RT_SUCCESS;
#else
    signature_data_t sign_data = {0};
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = sizeof(signature_data_t);
    uint16_t ret_len = 0;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = CMD_SIGN_CHK;
    c_data.data_len = (uint8_t)len;
    rt_os_memcpy(sign_data.hash, hash, rt_os_strlen(hash));
    rt_os_memcpy(sign_data.signature, sign, rt_os_strlen(sign));
    c_data.data = (uint8_t *)&sign_data;
    len = sizeof(atom_data_t) - sizeof(uint8_t *);
    rt_os_memcpy(&buf[0], &c_data, len);
    rt_os_memcpy(&buf[len], c_data.data, c_data.data_len);
    len += c_data.data_len;

    //MSG_HEXDUMP("send-buf", buf, len);
    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);
    //MSG_PRINTF(LOG_INFO, "len=%d, buf[0]=%02x\n", len, buf[0]);

    if (ret_len == 1 && buf[0] == RT_TRUE) {
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
#endif
}

#define PRIVATE_ECC_HASH_STR_LEN        128
#define MAX_FILE_HASH_LEN               64
#define MAX_FILE_HASH_BYTE_LEN          32
#define HASH_CHECK_BLOCK                1024    /* block size for HASH check */
#define MAX_NAME_BLOCK_SIZE             20

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

/*
file tail data as follow:
agentFFFFFFFFFFFFFFFB37F3BAD94DFCC1FBDB0FBF608802FA72D38FAEE3AB8CBBF63BF6C99DA9E31FA91B42B6134E73BC8914B84BF2521FD6291F
5C51670300A3FD13E8EB9966D8FDC
*/
int32_t ipc_file_verify_by_monitor(const char *abs_file, char *real_file_name)
{
    int32_t ret = RT_ERROR;
    sha256_ctx_t sha_ctx;
    rt_fshandle_t fp = NULL;
    int8_t hash_result[MAX_FILE_HASH_LEN + 1] = {0};
    int8_t hash_out[MAX_FILE_HASH_BYTE_LEN + 1] = {0};
    int8_t hash_buffer[HASH_CHECK_BLOCK] = {0};
    int8_t last_sign_buffer[PRIVATE_ECC_HASH_STR_LEN + 1] = {0};
    int8_t real_name[MAX_NAME_BLOCK_SIZE + 1] = {0};
    uint32_t check_size = 0;
    int32_t partlen = 0;
    int32_t file_size = 0;

    file_size = linux_file_size(abs_file);
    if (file_size <= (PRIVATE_ECC_HASH_STR_LEN + MAX_NAME_BLOCK_SIZE)) {
        MSG_PRINTF(LOG_ERR, "error file size: %d\r\n", file_size);
        goto exit_entry;
    }

    if ((fp = linux_fopen(abs_file, "r")) == NULL) {
        MSG_PRINTF(LOG_ERR, "error open file\n");
        goto exit_entry;
    }

    sha256_init(&sha_ctx);
    file_size -= PRIVATE_ECC_HASH_STR_LEN;
    if (file_size < HASH_CHECK_BLOCK) {
        rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
        if (linux_fread(hash_buffer, file_size, 1, fp) != 1) {
            MSG_PRINTF(LOG_ERR, "error read file\n");
            goto exit_entry;
        }
        sha256_update(&sha_ctx, (uint8_t *)hash_buffer, file_size);
    } else {
        for (check_size = HASH_CHECK_BLOCK; check_size < file_size; check_size += HASH_CHECK_BLOCK) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            if (linux_fread(hash_buffer, HASH_CHECK_BLOCK, 1, fp) != 1) {
                MSG_PRINTF(LOG_ERR, "error read file\n");
                goto exit_entry;
            }
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, HASH_CHECK_BLOCK);
        }

        partlen = file_size + HASH_CHECK_BLOCK - check_size;
        if (partlen > 0) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            if (linux_fread(hash_buffer, partlen, 1, fp) != 1){
                MSG_PRINTF(LOG_ERR, "error read file\n");
                goto exit_entry;
            }
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, partlen);
        }
    }

    if (linux_fread(last_sign_buffer, PRIVATE_ECC_HASH_STR_LEN, 1, fp) != 1){
        MSG_PRINTF(LOG_ERR, "error read file\n");
        goto exit_entry;
    }

    linux_fseek(fp, -(PRIVATE_ECC_HASH_STR_LEN + MAX_NAME_BLOCK_SIZE), RT_FS_SEEK_CUR);
    if (linux_fread(real_name, MAX_NAME_BLOCK_SIZE, 1, fp) != 1){
        MSG_PRINTF(LOG_ERR, "error read file\n");
        goto exit_entry;
    }

    sha256_final(&sha_ctx, (uint8_t *)hash_out);
    bytestring_to_charstring((const char *)hash_out, (char *)hash_result, MAX_FILE_HASH_BYTE_LEN);

    ret = ipc_sign_verify_by_monitor(hash_result, last_sign_buffer);
    if (!ret) {
        get_real_file_name(real_name, MAX_NAME_BLOCK_SIZE);
        rt_os_strcpy(real_file_name, real_name);
        MSG_PRINTF(LOG_INFO, "real_file_name: %s\r\n", real_file_name);
    } else {
        rt_os_memset(real_name, 0, sizeof(real_name));
        MSG_PRINTF(LOG_WARN, "read real_name  : %s\r\n", real_name);
        MSG_PRINTF(LOG_WARN, "calc hash_result: %s\r\n", hash_result);
        MSG_PRINTF(LOG_WARN, "tail sign_buffer: %s\r\n", last_sign_buffer);
    }

exit_entry:

    if (fp != NULL) {
        linux_fclose(fp);
    }

    MSG_PRINTF(LOG_INFO, "private file sign verify %s !\r\n", !ret ? "ok" : "fail");
    return ret;
}

int32_t ipc_restart_monitor(uint8_t delay)
{
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = 1;
    uint16_t ret_len = 0;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = CMD_RESTART_MONITOR;
    c_data.data_len = (uint8_t)len;
    c_data.data = (uint8_t *)&delay;
    len = sizeof(atom_data_t) - sizeof(uint8_t *);
    rt_os_memcpy(&buf[0], &c_data, len);
    rt_os_memcpy(&buf[len], c_data.data, c_data.data_len);
    len += c_data.data_len;

    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);

    if (ret_len == 1 && buf[0] == RT_TRUE) {
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

int32_t ipc_select_profile_by_monitor(void)
{
    atom_data_t c_data = {0};
    uint8_t buf[256] = {0};
    uint16_t len = 0;
    uint16_t ret_len = 0;

    rt_os_memset(c_data.start, 0xFF, sizeof(c_data.start));
    c_data.cmd = CMD_SELECT_PROFILE;
    c_data.data_len = (uint8_t)len;
    len = sizeof(atom_data_t) - sizeof(uint8_t *);
    rt_os_memcpy(&buf[0], &c_data, len);
    len += c_data.data_len;

    ipc_send_data((const uint8_t *)buf, len, (uint8_t *)buf, &ret_len);

    if (len == 1 && buf[0] == RT_TRUE) {
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

