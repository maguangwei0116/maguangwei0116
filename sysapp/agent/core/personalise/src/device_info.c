
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : device_info.c
 * Date        : 2019.09.05
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "device_info.h"
#include "md5.h"
#include "rt_qmi.h"
#include "agent_queue.h"

#if 0
IMEI不足15位，model不足8位，serialNo不足12位; 都要使用0x00补齐
#endif

#define MAX_IMEI_LEN    15
#define MAX_MODEL_LEN   8
#define MAX_SN_LEN      12

int32_t init_device_info(void *arg)
{
    static devicde_info_t info;
    MD5_CTX ctx;
    uint8_t device_id[DEVICE_ID_LEN/2 + 1];
    int32_t ret = RT_ERROR;

    rt_os_memset(&info, 0, sizeof(info));
    ret = rt_qmi_get_imei(info.imei);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get imei failed\n");
    }
    ret = rt_qmi_get_model(info.model);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get model failed\n");
    }
    info.model[7] = '\0';
    info.imei[DEVICE_IMEI_LEN] = '\0';
    
    MSG_PRINTF(LOG_INFO, "info.model:%s\n", info.model);
    MSG_PRINTF(LOG_INFO, "info.imei:%s\n", info.imei);
    MSG_PRINTF(LOG_INFO, "info.sn:%s, len:%d\n", info.sn, rt_os_strlen(info.sn));
    MD5Init(&ctx);
    MD5Update(&ctx, (uint8_t *)info.model, MAX_MODEL_LEN);
    MD5Update(&ctx, (uint8_t *)info.imei, MAX_IMEI_LEN);
    MD5Update(&ctx, (uint8_t *)info.sn, MAX_SN_LEN);
    MD5Final(&ctx, device_id);

    get_ascii_string((uint8_t *)device_id, DEVICE_ID_LEN/2, (uint8_t *)info.device_id);
    MSG_PRINTF(LOG_INFO, "info.device_id:%s\n", info.device_id);
    ((public_value_list_t *)arg)->device_info = &info;

    return RT_SUCCESS;
}
