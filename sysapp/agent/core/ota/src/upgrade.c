/*
 * upgrade.c
 *
 *  Created on: 2018??11??26??
 *      Author: xiangyinglai
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>
#include <sys/vfs.h>

#include "upgrade.h"
#include "md5.h"
#include "hash.h"
#include "rt_os.h"
#include "http_client.h"
#include "cJSON.h"
#include "upload.h"
#include "convert.h"
#include "config.h"
#include "agent_queue.h"

#define MAX_FILE_PATH_LEN               100

#define HASH_CHECK_BLOCK                1024    /* block size for HASH check */

#define MAX_DOWNLOAD_RETRY_CNT          9       /* plus 1 for max total download times */

#define DEFAULT_OTI_ENVIRONMENT_PORT    7082

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do {                 \
    snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
} while(0)

static const char *g_upgrade_oti_addr   = NULL;
static const char *g_upgrade_imei       = NULL;

static int32_t get_system_tf_free(uint32_t *free_byte)
{
    struct statfs diskInfo;
    unsigned long long totalBlocks;
    unsigned long long freeDisk;
    
    if (statfs("/data/", &diskInfo) < 0) {
        MSG_PRINTF(LOG_ERR, "get free byte fail\r\n");
        return RT_ERROR;
    }
    
    totalBlocks = diskInfo.f_bsize;
    freeDisk = diskInfo.f_bfree * totalBlocks;
    *free_byte = freeDisk;
    
    return RT_SUCCESS;
}

static rt_bool upgrade_check_sys_memory(upgrade_struct_t *d_info)
{
    uint32_t free_byte = 0;
    rt_bool ret = RT_FALSE;

    if (get_system_tf_free(&free_byte) == RT_SUCCESS) {
        MSG_PRINTF(LOG_INFO, "system freebyte: %d B (%d KB), file size: %d B\r\n", free_byte, free_byte/1024, d_info->size);
        if (d_info->size < free_byte) {
            ret = RT_TRUE;
        }
    }
    
    return ret;
}

static rt_bool ugrade_check_dir_permission(upgrade_struct_t *d_info)
{
    (void)d_info;
    return RT_TRUE;
}

static rt_bool upgrade_download_package(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    http_client_struct_t dw_struct = {0};
    cJSON  *post_info = NULL;
    int8_t *out;
    int8_t  buf[100];
    uint8_t down_try = 0;
    int32_t cnt = -1;  // used to count the number of download

    dw_struct.if_continue = 1;
    dw_struct.buf = NULL;

    /* build http body */
    post_info = cJSON_CreateObject();
    
    /* check target type frsit */
    if (d_info->type == TARGET_TYPE_DEF_SHARE_PROFILE) {
        STRUCTURE_OTI_URL(buf, sizeof(buf), g_upgrade_oti_addr, DEFAULT_OTI_ENVIRONMENT_PORT, "/default/agent/download");  // Build the OTI address
        cJSON_AddItemToObject(post_info, "swType", cJSON_CreateNumber(DEF_TARGET_TYPE_DEF_SHARE_PROFILE));
        cJSON_AddItemToObject(post_info, "imei", cJSON_CreateString(g_upgrade_imei));  // must have a real "imei"
    } else {
        STRUCTURE_OTI_URL(buf, sizeof(buf), g_upgrade_oti_addr, DEFAULT_OTI_ENVIRONMENT_PORT, "/api/v1/download");  // Build the OTI address
        cJSON_AddItemToObject(post_info, "ticket", cJSON_CreateString((char *)d_info->ticket));
        cJSON_AddItemToObject(post_info, "imei", cJSON_CreateString(""));  // must have a empty "imei"
    }
    
    dw_struct.file_path = (const char *)d_info->tmpFileName;
    dw_struct.manager_type = 1;
    dw_struct.http_header.method = 0;  // POST
    rt_os_memcpy(dw_struct.http_header.url, buf, rt_os_strlen(buf));
    dw_struct.http_header.url[rt_os_strlen(buf)] = '\0';
    dw_struct.http_header.version = 0;
    dw_struct.http_header.record_size = 0;
    
    out = (int8_t *)cJSON_PrintUnformatted(post_info);
    rt_os_memcpy(dw_struct.http_header.buf, out, rt_os_strlen(out));
    dw_struct.http_header.buf[rt_os_strlen(out)] = '\0';
    cJSON_free(out);

    snprintf((char *)buf, sizeof(buf), "%s:%d", g_upgrade_oti_addr, DEFAULT_OTI_ENVIRONMENT_PORT);
    http_set_header_record(&dw_struct, "HOST", buf);

    http_set_header_record(&dw_struct, "Content-Type", "application/json");
    http_set_header_record(&dw_struct, "Accept", "application/octet-stream");

    snprintf((char *)buf, sizeof(buf), "%d", rt_os_strlen(dw_struct.http_header.buf));
    http_set_header_record(&dw_struct, "Content-Length", (const char *)buf);

    /* build body with md5sum */
    get_md5_string((int8_t *)dw_struct.http_header.buf, buf);
    buf[MD5_STRING_LENGTH] = '\0';
    http_set_header_record(&dw_struct, "md5sum", (const char *)buf);

    while (1) {
        /* There is file need to download in system */
        if (rt_os_access((const int8_t *)dw_struct.file_path, RT_FS_F_OK) == RT_SUCCESS){
            uint32_t file_path_size = linux_file_size(dw_struct.file_path);
            snprintf(buf, sizeof(buf), "%d", file_path_size);
            http_set_header_record(&dw_struct, "Range", (const char *)buf);
            dw_struct.range = file_path_size;
        }
        MSG_PRINTF(LOG_DBG, "Download file_path : %s, size:%d\r\n", (const int8_t *)dw_struct.file_path, linux_file_size(dw_struct.file_path));

        /* If you force to send http download with range==file_size, it will response http status 406 */
        if (dw_struct.range == d_info->size) {
            MSG_PRINTF(LOG_DBG, "ota task has downloaded over before !\r\n");
            ret = RT_SUCCESS;
            break;
        }

        if (http_client_file_download(&dw_struct) == 0) {
            ret = RT_TRUE;
            MSG_PRINTF(LOG_WARN, "Download file_path : %s, size:%d\r\n", (const int8_t *)dw_struct.file_path, linux_file_size(dw_struct.file_path));
            break;
        }
        cnt++;
        MSG_PRINTF(LOG_DBG, "Download fail cnt: %d\r\n", cnt);
        if (cnt >= d_info->retryAttempts) {
            MSG_PRINTF(LOG_WARN, "Download fail too many times !\r\n");
            break;
        }
        rt_os_sleep(d_info->retryInterval);
    }
    cJSON_Delete(post_info);

    return ret;
}

static rt_bool upgrade_check_package(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    sha256_ctx_t sha_ctx;
    rt_fshandle_t fp = NULL;
    int8_t hash_result[MAX_FILE_HASH_LEN + 1];
    int8_t hash_out[MAX_FILE_HASH_BYTE_LEN + 1];
    int8_t hash_buffer[HASH_CHECK_BLOCK];
    uint32_t check_size;
    int32_t partlen;
    struct  stat f_info;

    /* default share profile ignore HASH check */
    if (d_info->type == TARGET_TYPE_DEF_SHARE_PROFILE) {
        return RT_TRUE;
    }

    RT_CHECK_ERR(stat((char *)d_info->tmpFileName, &f_info), -1);

    RT_CHECK_ERR(fp = linux_fopen((char *)d_info->tmpFileName, "r") , NULL);

    sha256_init(&sha_ctx);
    if (f_info.st_size < HASH_CHECK_BLOCK) {
        rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
        RT_CHECK_ERR(linux_fread(hash_buffer, f_info.st_size, 1, fp), 0);
        sha256_update(&sha_ctx, (uint8_t *)hash_buffer, f_info.st_size);
    } else {
        for (check_size = HASH_CHECK_BLOCK; check_size < f_info.st_size; check_size += HASH_CHECK_BLOCK) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(linux_fread(hash_buffer, HASH_CHECK_BLOCK, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, HASH_CHECK_BLOCK);
        }

        partlen = f_info.st_size + HASH_CHECK_BLOCK - check_size;
        if (partlen > 0) {
            rt_os_memset(hash_buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(linux_fread(hash_buffer, partlen, 1, fp), 0);
            sha256_update(&sha_ctx, (uint8_t *)hash_buffer, partlen);
        }
    }

    sha256_final(&sha_ctx, (uint8_t *)hash_out);
    bytestring_to_charstring(hash_out, hash_result, MAX_FILE_HASH_BYTE_LEN);

    MSG_PRINTF(LOG_WARN, "download  hash_result: %s\r\n", hash_result);
    MSG_PRINTF(LOG_WARN, "push file hash_result: %s\r\n", d_info->fileHash);
    RT_CHECK_NEQ(strncpy_case_insensitive(hash_result, d_info->fileHash, MAX_FILE_HASH_LEN), RT_TRUE);
    MSG_PRINTF(LOG_WARN, "file hash check ok !\r\n");
    ret = RT_TRUE;

end:

    if (fp != NULL) {
        linux_fclose(fp);
    }
    return ret;
}

static upgrade_result_e start_comman_upgrade_process(upgrade_struct_t *d_info)
{
    upgrade_result_e ret;
    
    /* check FS space */
    if (upgrade_check_sys_memory(d_info) != RT_TRUE) {
        ret = UPGRADE_FS_SPACE_NOT_ENOUGH_ERROR;
        MSG_PRINTF(LOG_WARN, "upgrade_sys space not enough\n");
        goto exit_entry;
    }

    /* check permission */
    if (ugrade_check_dir_permission(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "upgrade_dir permission False\n");
        ret = UPGRADE_DIR_PERMISSION_ERROR;
        goto exit_entry;
    }

    /* download package */
    if (upgrade_download_package(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "upgrade_download package False\n");
        ret = UPGRADE_DOWNLOAD_PACKET_ERROR;
        goto exit_entry;
    }

    /* file hash check */
    if (upgrade_check_package(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "check upgrade packet error\n");
        if (d_info->cleanup) {
            d_info->cleanup(d_info);
        }
        ret = UPGRADE_HASH_CHECK_ERROR;
        goto exit_entry;
    }

    /* private file check */
    if (d_info->check && (d_info->check(d_info) != RT_TRUE)) {
        MSG_PRINTF(LOG_WARN, "private check upgrade packet error\n");
        if (d_info->cleanup) {
            d_info->cleanup(d_info);
        }
        ret = UPGRADE_SIGN_CHECK_ERROR;
        goto exit_entry;
    }

    /* install app */
    if (d_info->install && (d_info->install(d_info) != RT_TRUE)) {
        MSG_PRINTF(LOG_WARN, "install upgrade packet error\n");
        if (d_info->cleanup) {
            d_info->cleanup(d_info);
        }
        ret = UPGRADE_INSTALL_APP_ERROR;
        goto exit_entry;
    }

    MSG_PRINTF(LOG_INFO, "Upgrade ok ! \n");

    ret = UPGRADE_NO_FAILURE;

exit_entry:

    return ret;
}

static upgrade_result_e start_fota_upgrade_process(upgrade_struct_t *d_info)
{
    return RT_TRUE;
}

static void check_upgrade_process(void *args)
{
    upgrade_struct_t *d_info = (upgrade_struct_t *)args;
    upgrade_result_e result;

    if (GET_UPDATEMODE(d_info) == 1) { /* full upgrade */
        result = start_comman_upgrade_process(d_info);
    } else if (GET_UPDATEMODE(d_info) == 2) { /* TODO: FOTA upgrade mode */
        result = start_fota_upgrade_process(d_info);
    }

    d_info->downloadResult = result;

    if (d_info->on_event) {
        d_info->on_event(d_info);
    }

    rt_exit_task(NULL);
}

int32_t upgrade_process_create(upgrade_struct_t **d_info)
{
    upgrade_struct_t *info = NULL;

    info = (upgrade_struct_t *)rt_os_malloc(sizeof(upgrade_struct_t));
    *d_info = info;

    return info ? 0: -1;
}

int32_t upgrade_process_start(upgrade_struct_t *d_info)
{
    rt_task id;
    
    if (rt_create_task(&id, (void *)check_upgrade_process, (void *)d_info) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "Create upgrade thread flase\n");
        return -1;
    }

    return 0;
}

int32_t init_upgrade(void *arg)
{
    public_value_list_t *public_value_list = (public_value_list_t *)arg;
    g_upgrade_oti_addr  = (const char *)public_value_list->config_info->oti_addr;
    g_upgrade_imei      = (const char *)public_value_list->device_info->imei;

    return 0;
}
