
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
    uint8_t device_id[MAX_DEVICE_ID_LEN/2 + 1];
    int32_t ret = RT_ERROR;

    rt_os_memset(&info, 0, sizeof(info));
    ret = rt_qmi_get_imei(info.imei, sizeof(info.imei));
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get imei failed\n");
    }

    ret = rt_qmi_get_model(info.model, sizeof(info.model));
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Get model failed\n");
    }

    info.model[MAX_DEVICE_MODEL_LEN - 1] = '\0';  // max (MAX_DEVICE_MODEL_LEN-1) bytes on valid model string value !!!
    info.imei[MAX_DEVICE_IMEI_LEN] = '\0';    
    
    MD5Init(&ctx);
    MD5Update(&ctx, (uint8_t *)info.imei, MAX_DEVICE_IMEI_LEN);
    MD5Final(&ctx, device_id);
    get_ascii_string((uint8_t *)device_id, MAX_DEVICE_ID_LEN/2, (uint8_t *)info.device_id);
    
    MSG_PRINTF(LOG_INFO, "device_id:[%s] imei:[%s] model:[%s] sn:[%s]\n", info.device_id, info.imei, info.model, info.sn);
    
    ((public_value_list_t *)arg)->device_info = &info;

    return RT_SUCCESS;
}
