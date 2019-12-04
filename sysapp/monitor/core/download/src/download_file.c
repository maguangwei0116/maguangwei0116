
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : download_file.c
 * Date        : 2019.10.29
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "download_file.h"
#include "rt_os.h"
#include "md5.h"
#include "hash.h"
#include "http_client.h"
#include "cJSON.h"
#include "rt_qmi.h"
#include "inspect_file.h"

#define DEFAULT_OTI_ENVIRONMENT_PORT    7082

#if (CFG_ENV_TYPE_PROD)
#define DOWNLOAD_OTA_ADDR               "52.220.34.227"
#else
#define DOWNLOAD_OTA_ADDR               "54.222.248.186"
#endif

#define STRUCTURE_OTI_URL(buf, buf_len, addr, port, interface) \
do {                 \
   snprintf((char *)buf, buf_len, "http://%s:%d%s", addr, port, interface);     \
} while(0)

static upgrade_struct_t g_d_info;

static rt_bool download_file_process(upgrade_struct_t *d_info)
{
    rt_bool ret = RT_FALSE;
    http_client_struct_t dw_struct = {0};
    cJSON  *post_info = NULL;
    int8_t *out;
    int8_t  buf[100];
    uint8_t  imei[16];
    int32_t cnt = -1;  // used to count the number of download

    dw_struct.if_continue = 1;
    dw_struct.buf = NULL;
    /* build http body */
    dw_struct.file_path = (const char *)d_info->file_name;
    dw_struct.manager_type = 1;
    dw_struct.http_header.method = 0;  // POST
    STRUCTURE_OTI_URL(buf, sizeof(buf), DOWNLOAD_OTA_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT, "/default/agent/download");  // Build the OTI address
    rt_os_memcpy(dw_struct.http_header.url, buf, rt_os_strlen(buf));
    dw_struct.http_header.url[rt_os_strlen(buf)] = '\0';
    dw_struct.http_header.version = 0;
    dw_struct.http_header.record_size = 0;

    post_info = cJSON_CreateObject();
    rt_qmi_get_imei(imei);
    cJSON_AddItemToObject(post_info, "swType", cJSON_CreateNumber(0)); // 0 for agent
    cJSON_AddItemToObject(post_info, "imei", cJSON_CreateString(imei));  // must have a empty "imei"
    out = (int8_t *)cJSON_PrintUnformatted(post_info);
    rt_os_memcpy(dw_struct.http_header.buf, out, rt_os_strlen(out));
    dw_struct.http_header.buf[rt_os_strlen(out)] = '\0';
    cJSON_free(out);

    snprintf((char *)buf, sizeof(buf), "%s:%d", DOWNLOAD_OTA_ADDR, DEFAULT_OTI_ENVIRONMENT_PORT);
    http_set_header_record(&dw_struct, "HOST", buf);

    http_set_header_record(&dw_struct, "Content-Type", "application/json");
    http_set_header_record(&dw_struct, "Accept", "application/octet-stream");

    snprintf((char *)buf, sizeof(buf), "%d", rt_os_strlen(dw_struct.http_header.buf));
    http_set_header_record(&dw_struct, "Content-Length", (const char *)buf);

    /* build body with md5sum */
    get_md5_string((int8_t *)dw_struct.http_header.buf, buf);
    buf[MD5_STRING_LENGTH] = '\0';
    http_set_header_record(&dw_struct, "md5sum", (const char *)buf);
    dw_struct.range = 0;

    while (1) {
        MSG_PRINTF(LOG_DBG, "Download file_path : %s, size:%d\r\n", (const int8_t *)dw_struct.file_path, linux_file_size(dw_struct.file_path));

        if (http_client_file_download(&dw_struct) == 0) {
            ret = RT_TRUE;
            MSG_PRINTF(LOG_ERR, "Download file_path : %s, size:%d\r\n", (const int8_t *)dw_struct.file_path, linux_file_size(dw_struct.file_path));
            break;
        }
        cnt++;
        MSG_PRINTF(LOG_DBG, "Download fail cnt: %d\r\n", cnt);
        if (cnt >= 3) {  // retry 3 times
            MSG_PRINTF(LOG_ERR, "Download fail too many times !\r\n");
            break;
        }

        /* default agent download don't support [Breakpoint-renewal] now */
        dw_struct.range = 0;
        linux_delete_file((const int8_t *)dw_struct.file_path);
    }
    cJSON_Delete(post_info);

    return ret;
}

static void upgrade_process(void *args)
{
    upgrade_struct_t *d_info = (upgrade_struct_t *)args;
    char tmp_file_name[128];
    char final_file_name[128];
    char *p = NULL;

    /* change to tmp file name */
    p = rt_os_strrchr(d_info->file_name, '/');
    if (p) {
        p++;
    } else {
        p = d_info->file_name;
    }
    snprintf(final_file_name, sizeof(final_file_name), "%s", d_info->file_name);
    snprintf(tmp_file_name, sizeof(tmp_file_name), "%s.tmp", p);
    snprintf((char *)d_info->file_name, sizeof(d_info->file_name), "%s", tmp_file_name);

    /* download tmp file [step 1] */
    if (download_file_process(d_info) != RT_TRUE) {               // download file
        MSG_PRINTF(LOG_ERR, "Download file failed !\r\n");
        goto end;
    }

    /* install tmp file [step 2] */
    if (linux_rt_rename_file((const char *)tmp_file_name, final_file_name) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "re-name error\n");
        goto end;
    }

    /* verify tmp file [step 3] (step order is important) */
    if (monitor_inspect_file((const char *)final_file_name, d_info->real_file_name) != RT_TRUE) {     // inspect file
        MSG_PRINTF(LOG_ERR, "Inspect file failed !\r\n");
        //linux_rt_delete_file((const char *)final_file_name);
        goto end;
    }

    if (rt_os_chmod(final_file_name, RT_S_IRWXU | RT_S_IRWXG | RT_S_IRWXO) == RT_SUCCESS) {      // chmod file
#if 0
        MSG_PRINTF(LOG_DBG, "Download agent success, reboot device after 10 seconds !\r\n");
        rt_os_sleep(10);
        rt_os_reboot();    // reboot device
#else
        MSG_PRINTF(LOG_DBG, "Download agent success, monitor exit to restart again after 10 seconds !\r\n");
        rt_os_sleep(10);
        rt_os_exit(-1);
#endif
    }
    
end:
    return;
}

void init_download(void *args)
{
    rt_os_memcpy(&g_d_info, (upgrade_struct_t *)args, sizeof(upgrade_struct_t));
}

int32_t download_start(void)
{
    rt_task id;

    if (rt_create_task(&id, (void *)upgrade_process, (void *)&g_d_info) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "Create upgrade thread flase\n");
        return -1;
    }

    return 0;
}
