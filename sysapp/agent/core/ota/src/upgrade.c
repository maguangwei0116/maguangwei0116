/*
 * upgrade.c
 *
 *  Created on: 2018年11月26日
 *      Author: xiangyinglai
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <error.h>

#include "upgrade.h"
#include "md5.h"
#include "hash.h"
//#include "agent.h"
#include "rt_os.h"
#include "http_client.h"
#include "cJSON.h"
#include "agent_config.h"
#include "upload.h"
#include "dial_up.h"
#include "convert.h"
#include "config.h"

typedef enum NETWORK_STATE {
    NETWORK_STATE_INIT = 0,
    NETWORK_GET_IP,
    NETWORK_CONNECTING,
    NETWORK_DIS_CONNECTED,
    NETWORK_USING
} network_state_info_e;

#define MAX_DOWNLOAD_RETRY_CNT      9   /* plus 1 for max total download times */

extern int8_t  *g_imei;

int8_t g_download_flag = 0;

#if 0
#define RT_CHECK_ERR(process, result) \
    if((process) == result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_NEQ(process, result) \
    if((process) != result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_LESE(process, result) \
    if((process) <= result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#define RT_CHECK_LES(process, result) \
    if((process) < result){ MSG_PRINTF(LOG_WARN, "%s error result:%s\n", #process, strerror(errno));  goto end;}
#endif

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do{                 \
   snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
}while(0)

static rt_bool upgrade_check_sys_memory()
{
    return RT_TRUE;
}

static rt_bool ugrade_check_dir_permission()
{
    return RT_TRUE;
}

/********************************************************
* name：int32_t upgrade_compare_version(upgrade_struct_t *d_info)
* funcition：比较参数传入的版本与该软件的版本
* parameter：
* -d_info--需要进行升级的信息
* return value:
* -0--需要进行软件更新
********************************************************/
static rt_bool upgrade_compare_version(upgrade_struct_t *d_info)
{
#if 0
    /* 对比客户类型 */
    if (rt_os_strncmp(d_info->make, MAKE, rt_os_strlen(MAKE)) != 0) {
        return RT_FALSE;
    }
#endif

#if 0
    if (GET_FORCEUPDATE(d_info) == 0) {
        /* 对比版本号 */
        if (d_info->versioncode > VERSION_CODE) {
            return RT_TRUE;
        } else {
            return RT_FALSE;
        }
    }
#endif
    return RT_TRUE;
}

static rt_bool upgrade_download_package(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    http_client_struct_t dw_struct;
    int8_t file_path[MAX_FILE_PATH_LEN + 1];
    cJSON  *post_info = NULL;
    int8_t *out;
    int8_t  buf[100];
    uint8_t down_try = 0;
    int32_t cnt = -1;  // used to count the number of download

    g_download_flag = 1;  // set download flag
    dw_struct.if_continue = 1;
    dw_struct.buf = NULL;

    /* 构建文件下载Http请求body */
    snprintf((char *)file_path, MAX_FILE_PATH_LEN + 1, "%s%s", BACKUP_PATH, d_info->fileName);  // Build a complete path to download files
    dw_struct.file_path = (const char *)file_path;
    dw_struct.manager_type = 1;
    dw_struct.http_header.method = 0;  // POST
    STRUCTURE_OTI_URL(buf, 100, OTI_ENVIRONMENT_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT, "/api/v1/download");  // Build the OTI address
    rt_os_memcpy(dw_struct.http_header.url, buf, rt_os_strlen(buf));
    dw_struct.http_header.url[rt_os_strlen(buf)] = '\0';
    dw_struct.http_header.version = 0;
    dw_struct.http_header.record_size = 0;
    
    post_info = cJSON_CreateObject();
    cJSON_AddItemToObject(post_info,"ticket",cJSON_CreateString((char *)d_info->ticket));
    out = (int8_t *)cJSON_PrintUnformatted(post_info);
    rt_os_memcpy(dw_struct.http_header.buf, out, rt_os_strlen(out));
    dw_struct.http_header.buf[rt_os_strlen(out)] = '\0';

    snprintf((char *)buf, 100, "%s:%d", OTI_ENVIRONMENT_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT);
    http_set_header_record(&dw_struct, "HOST", buf);

    http_set_header_record(&dw_struct, "Content-Type", "application/json");
    http_set_header_record(&dw_struct, "Accept", "application/octet-stream");
    http_set_header_record(&dw_struct, "imei", "");

    snprintf((char *)buf, 100, "%d", rt_os_strlen(dw_struct.http_header.buf));
    http_set_header_record(&dw_struct, "Content-Length", (const char *)buf);

    /* 计算body的md5校验码 */
    get_md5_string((int8_t *)dw_struct.http_header.buf, buf);
    buf[MD5_STRING_LENGTH] = '\0';
    http_set_header_record(&dw_struct, "md5sum", (const char *)buf);

    do {
        #if 0
        /* wait network connecting*/
        while(get_network_state() != NETWORK_CONNECTING && get_network_state() != NETWORK_USING) {
            rt_os_sleep(1);
        };
        #endif
        
        /*There is file need to download in system*/
        if (rt_os_access((const int8_t *)file_path, F_OK) == RT_SUCCESS){
            snprintf(buf, 100, "%d", get_file_size(file_path));
            http_set_header_record(&dw_struct, "Range", (const char *)buf);
        }

        if (http_client_file_download(&dw_struct) == 0) {
            g_download_flag = 0;  // download success reset flag
            ret = RT_TRUE;
            break;
        }
        cnt++;
        MSG_PRINTF(LOG_WARN, "Download fail cnt: %d\r\n", cnt);
        
        rt_os_sleep(d_info->retryInterval);
    } while(cnt < d_info->retryAttempts);
    
    cJSON_free(out);
    cJSON_Delete(post_info);
    return ret;
}

static rt_bool upgrade_check_package(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    sha256_ctx sha_ctx;
    FILE *fp = NULL;
    int8_t file_path[MAX_FILE_PATH_LEN + 1];
    int8_t hash_result[MAX_FILE_HASH_LEN + 1];  // hash运算计算结果
    int8_t hash_out[MAX_FILE_HASH_LEN + 1];
    uint32_t check_size;
    int32_t partlen;
    struct  stat f_info;

    snprintf((char *)file_path, MAX_FILE_PATH_LEN + 1, "%s%s", BACKUP_PATH, d_info->fileName);

    RT_CHECK_ERR(stat((char *)file_path, &f_info), -1);

    RT_CHECK_ERR(fp = fopen((char *)file_path, "r") , NULL);

    sha256_init(&sha_ctx);
    if (f_info.st_size < HASH_CHECK_BLOCK) {
        rt_os_memset(d_info->buffer, 0, HASH_CHECK_BLOCK);
        RT_CHECK_ERR(fread(d_info->buffer, f_info.st_size, 1, fp), 0);
        sha256_update(&sha_ctx,(uint8_t *)d_info->buffer, f_info.st_size);
    } else {
        for (check_size = HASH_CHECK_BLOCK; check_size < f_info.st_size; check_size += HASH_CHECK_BLOCK) {
            rt_os_memset(d_info->buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(fread(d_info->buffer, HASH_CHECK_BLOCK, 1, fp), 0);
            sha256_update(&sha_ctx,(uint8_t *)d_info->buffer, HASH_CHECK_BLOCK);
        }

        partlen = f_info.st_size + HASH_CHECK_BLOCK - check_size;
        if (partlen > 0) {
            rt_os_memset(d_info->buffer, 0, HASH_CHECK_BLOCK);
            RT_CHECK_ERR(fread(d_info->buffer, partlen, 1, fp), 0);
            sha256_update(&sha_ctx,(uint8_t *)d_info->buffer, partlen);
        }
    }

    sha256_final(&sha_ctx,(uint8_t *)hash_out);
    bytestring_to_charstring(hash_out, hash_result, 32);

    RT_CHECK_NEQ(strncpy_case_insensitive(hash_result, d_info->fileHash, MAX_FILE_HASH_LEN), RT_TRUE);

    ret = RT_TRUE;
end:

    if (fp != NULL) {
        fclose(fp);
    }
    return ret;
}

/* 进行本地文件替换 */
static rt_bool replace_process(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    int8_t file_path[MAX_FILE_PATH_LEN + 1];

    snprintf((char *)file_path, MAX_FILE_PATH_LEN + 1, "%s%s", BACKUP_PATH, d_info->fileName);

    /* 进行agent替换 */
    RT_CHECK_NEQ(rt_os_rename(file_path, AGENT_PATH), 0);

    RT_CHECK_NEQ(chmod(AGENT_PATH, S_IRWXU | S_IRWXG | S_IRWXO), 0);

    SET_UPGRADE_STATUS(d_info, 1);  // 设置升级成功标志位

    /* 连续两次sync保证新软件同步到本地flash */
    rt_os_sync();
    rt_os_sync();

    ret = RT_TRUE;
end:
    return ret;
}

static void upgrade_package_cleanup(upgrade_struct_t *d_info)
{
    int8_t cmd[100];
    int8_t file_path[MAX_FILE_PATH_LEN + 1];

    snprintf((char *)file_path, MAX_FILE_PATH_LEN + 1, "%s%s", BACKUP_PATH, d_info->fileName);
    rt_os_unlink(file_path);
}

/* 进行普通升级操作 */
static rt_upgrade_result_e start_comman_upgrade_process(upgrade_struct_t *d_info)
{
    /* 检测系统内存 */
    upgrade_check_sys_memory();

    /* 检测文件系统权限 */
    ugrade_check_dir_permission();

    /* 对比版本号 */
    if (upgrade_compare_version(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "upgrade_compare_version False\n");
        return UPGRADE_VERSION_NUM_ERROR;
    }

    /* 下载升级包 */
    if (upgrade_download_package(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "upgrade_download_package False\n");
        upgrade_package_cleanup(d_info);
        return UPGRADE_DOWNLOAD_PACKET_ERROR;
    }

    /* 校验升级包 */
    if (upgrade_check_package(d_info) != RT_TRUE) {
        MSG_PRINTF(LOG_WARN, "Check Upgrade Packet Error \n");
        upgrade_package_cleanup(d_info);
        return UPGRADE_CHECK_PACKET_ERROR;
    }

    /* 替换升级包 */
    if (replace_process(d_info) == RT_FALSE) {
        upgrade_package_cleanup(d_info);
        return UPGRADE_REPLACE_APP_ERROR;
    }

    return UPGRADE_NO_FAILURE;
}


/* 进行fota升级*/
static rt_upgrade_result_e start_fota_upgrade_process(upgrade_struct_t *d_info)
{
    return RT_TRUE;
}

void * check_upgrade_process(void *args)
{
    upgrade_struct_t *d_info = (upgrade_struct_t *)args;
    rt_upgrade_result_e result;

    MSG_PRINTF(LOG_INFO, "111111 = %d\n", GET_UPDATEMODE(d_info));
    
    /* 全量升级模式 */
    if (GET_UPDATEMODE(d_info) == 1) {
        result = start_comman_upgrade_process(d_info);

       /* FOTA升级模式 */
    } else if (GET_UPDATEMODE(d_info) == 2) {
        result = start_fota_upgrade_process(d_info);
    }

    /* 上报升级结果 */
    //msg_upload_data(d_info->tranid, ON_UPGRADE, (int)result, (void *)d_info);
}
