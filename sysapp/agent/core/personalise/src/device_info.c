
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

int32_t init_device_info(void *arg)
{
    static devicde_info_t info;
    MD5_CTX ctx;
    uint8_t device_id[DEVICE_ID_LEN + 1];
    int32_t ret = RT_ERROR;

    ret = rt_qmi_get_imei(info.imei);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get imei failed\n");
    }
    ret = rt_qmi_get_model(info.model);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get model failed\n");
    }
    rt_os_memset(info.sn, 0x00, DEVICE_SN_LEN);
    MD5Init(&ctx);
    MD5Update(&ctx, info.imei, DEVICE_IMEI_LEN);
    MD5Update(&ctx, info.model, DEVICE_MODEL_LEN);
    MD5Update(&ctx, info.sn, DEVICE_SN_LEN);
    MD5Final(&ctx, device_id);

    get_ascii_string((int8_t *)device_id, (int8_t *)info.device_id, DEVICE_ID_LEN/2);
    MSG_PRINTF(LOG_INFO, "info.model:%s\n", info.model);
    MSG_PRINTF(LOG_INFO, "info.imei:%s\n", info.imei);
    MSG_PRINTF(LOG_INFO, "info.device_id:%s\n", info.device_id);
    ((public_value_list_t *)arg)->device_info = &info;

    return RT_SUCCESS;
}
