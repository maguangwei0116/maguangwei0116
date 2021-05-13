
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : usrdata_manager.c
 * Date        : 2020.05.14
 * Note        : manage usr data for runing
 * Description :
 * Contributors: yuxiang - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "usrdata.h"
#include "stdint.h"
#include "rt_type.h"
#include "config.h"

#define DEFAULT_STRATEGY "{\"enabled\":true,\"interval\":10,\"type\":1,\"strategies\":[{\"domain\":\"cthulhu.easyiot.ai\",\"level\":1}]}"

static rt_bool network_strategy_check_memory(const void *buf, int32_t len, int32_t value)
{
    int32_t i = 0;
    const uint8_t *p = (const uint8_t *)buf;

    for (i = 0; i < len; i++) {
        if (p[i] != value) {
            return RT_FALSE;
        }
    }
    return RT_TRUE;
}

int32_t rt_write_default_strategy()
{
    return rt_write_strategy(0, DEFAULT_STRATEGY, sizeof(DEFAULT_STRATEGY));
}

void rt_check_strategy_data(inspect_strategy_e type)
{
    uint8_t network_strategy_buff[CHECK_STRATEGY_HEAD + 1] = {0};

    rt_read_strategy(0, network_strategy_buff, CHECK_STRATEGY_HEAD);
    if (type == RT_BOOT_CHECK) {
        if(network_strategy_check_memory(network_strategy_buff, CHECK_STRATEGY_HEAD, 'F')) {
            MSG_PRINTF(LOG_ERR, "Read data is empty, Use default monitor strategy !\n");
            rt_write_default_strategy();
        }

    } else if(type == RT_RUN_CHECK) {
        if(rt_os_strncmp(network_strategy_buff, DEFAULT_STRATEGY, CHECK_STRATEGY_HEAD) != RT_SUCCESS) {
            MSG_PRINTF(LOG_ERR, "Read data is error, Use default monitor strategy !\n");
            rt_write_default_strategy();
        }
    }
}

int32_t init_run_config()
{
    uint8_t init_buff[RT_APN_LIST_OFFSET + 1] = {0};                // init data before apn

    rt_os_memset(init_buff, 'F', RT_APN_LIST_OFFSET);

    if (!linux_rt_file_exist(RUN_CONFIG_FILE)) {
        rt_create_file(RUN_CONFIG_FILE);
        rt_write_data(RUN_CONFIG_FILE, 0, init_buff, RT_APN_LIST_OFFSET);
        rt_write_prov_ctrl_counter(0, 0);
    }

    rt_check_strategy_data(RT_BOOT_CHECK);

    return RT_SUCCESS;
}

int32_t rt_write_card_type(int32_t offset, uint8_t *card_type, int32_t len)
{
    int32_t ret;
    uint8_t init_buf[RT_CARD_TYPE_LEN + 1];

    if (len > RT_CARD_TYPE_LEN) {
        MSG_PRINTF(LOG_WARN, "Data is too long, cannot be written ...\n");
        return RT_ERROR;
    }

    rt_os_memset(init_buf, 'F', RT_CARD_TYPE_LEN);
    rt_write_data(RUN_CONFIG_FILE, RT_CARD_TYPE_OFFSET, init_buf, RT_CARD_TYPE_LEN);
    ret = rt_write_data(RUN_CONFIG_FILE, RT_CARD_TYPE_OFFSET + offset, card_type, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write card type fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_card_type(int32_t offset, uint8_t *card_type, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_CARD_TYPE_OFFSET + offset, card_type, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read card type fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_write_eid(int32_t offset, uint8_t *eid, int32_t len)
{
    int32_t ret;
    uint8_t init_buf[RT_LAST_EID_LEN + 1];

    if (len > RT_LAST_EID_LEN) {
        MSG_PRINTF(LOG_WARN, "Data is too long, cannot be written ...\n");
        return RT_ERROR;
    }

    rt_os_memset(init_buf, 'F', RT_LAST_EID_LEN);
    rt_write_data(RUN_CONFIG_FILE, RT_LAST_EID_OFFSET, init_buf, RT_LAST_EID_LEN);
    ret = rt_write_data(RUN_CONFIG_FILE, RT_LAST_EID_OFFSET + offset, eid, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write eid fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_eid(int32_t offset, uint8_t *eid, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_LAST_EID_OFFSET + offset, eid, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read eid fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_write_prov_ctrl_counter(int32_t offset, int32_t counter)
{
    int32_t ret = RT_ERROR;
    uint8_t init_buf[RT_PROV_COUNTER_LEN + 1];

    rt_os_memset(init_buf, 'F', RT_PROV_COUNTER_LEN);
    init_buf[offset + 0] = (uint8_t)((counter >> 24) & 0xFF);
    init_buf[offset + 1] = (uint8_t)((counter >> 16) & 0xFF);
    init_buf[offset + 2] = (uint8_t)((counter >> 8) & 0xFF);
    init_buf[offset + 3] = (uint8_t)(counter & 0xFF);
    MSG_PRINTF(LOG_DBG, "counter : %d, size : %d\n", counter, sizeof(counter));
    MSG_INFO_ARRAY("init_buf: ", init_buf, sizeof(counter));
    ret = rt_write_data(RUN_CONFIG_FILE, RT_PROV_COUNTER_OFFSET, init_buf, RT_PROV_COUNTER_LEN);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write prov control counter fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_prov_ctrl_counter(int32_t offset, int32_t *counter)
{
    uint8_t init_buf[RT_PROV_COUNTER_LEN + 1];
    int32_t count = 0;
    int32_t ret = RT_ERROR;

    rt_os_memset(init_buf, 'F', RT_PROV_COUNTER_LEN);
    ret = rt_read_data(RUN_CONFIG_FILE, RT_PROV_COUNTER_OFFSET, init_buf, RT_PROV_COUNTER_LEN);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read prov control counter fail, ret : %d\n", ret);
    }

    count |= ((uint32_t)init_buf[offset + 0] << 24) & 0xFF000000;
    count |= ((uint32_t)init_buf[offset + 1] << 16) & 0x00FF0000;
    count |= ((uint32_t)init_buf[offset + 2] << 8) & 0x0000FF00;
    count |= ((uint32_t)init_buf[offset + 3] & 0x000000FF);

    MSG_INFO_ARRAY("init_buf: ", init_buf, sizeof(count));
    MSG_PRINTF(LOG_DBG, "counter : %d, size : %d\n", count, sizeof(count));

    *counter = count;

    return ret;
}

int32_t rt_write_devicekey(int32_t offset, const uint8_t *devicekey, int32_t len)
{
    int32_t ret;
    uint8_t init_buf[RT_DEVICE_KEY_LEN + 1];

    if (len > RT_DEVICE_KEY_LEN) {
        MSG_PRINTF(LOG_WARN, "Data is too long, cannot be written ...\n");
        return RT_ERROR;
    }

    rt_os_memset(init_buf, 'F', RT_DEVICE_KEY_LEN);
    rt_write_data(RUN_CONFIG_FILE, RT_DEVICE_KEY_OFFSET, init_buf, RT_DEVICE_KEY_LEN);
    ret = rt_write_data(RUN_CONFIG_FILE, RT_DEVICE_KEY_OFFSET + offset, devicekey, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write device key fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_devicekey(int32_t offset, uint8_t *devicekey, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_DEVICE_KEY_OFFSET + offset, devicekey, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read device key fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_write_ticket(int32_t offset, uint8_t *ticket, int32_t len)
{
    int32_t ret;
    uint8_t init_buf[RT_TICKET_SERVER_LEN + 1];

    if (len > RT_TICKET_SERVER_LEN) {
        MSG_PRINTF(LOG_WARN, "Data is too long, cannot be written ...\n");
        return RT_ERROR;
    }

    rt_os_memset(init_buf, 'F', RT_TICKET_SERVER_LEN);
    rt_write_data(RUN_CONFIG_FILE, RT_TICKET_SERVER_OFFSET, init_buf, RT_TICKET_SERVER_LEN);
    ret = rt_write_data(RUN_CONFIG_FILE, RT_TICKET_SERVER_OFFSET + offset, ticket, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write ticket server fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_ticket(int32_t offset, uint8_t *ticket, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_TICKET_SERVER_OFFSET + offset, ticket, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read ticket server fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_write_strategy(int32_t offset, uint8_t *strategy, int32_t len)
{
    int32_t ret;
    uint8_t init_buf[RT_STRATEGY_LIST_LEN + 1];

    if (len > RT_STRATEGY_LIST_LEN) {
        MSG_PRINTF(LOG_WARN, "Data is too long, cannot be written ...\n");
        return RT_ERROR;
    }

    rt_os_memset(init_buf, 'F', RT_STRATEGY_LIST_LEN);
    rt_write_data(RUN_CONFIG_FILE, RT_STRATEGY_LIST_OFFSET, init_buf, RT_STRATEGY_LIST_LEN);
    ret = rt_write_data(RUN_CONFIG_FILE, RT_STRATEGY_LIST_OFFSET + offset, strategy, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Write strategy fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_read_strategy(int32_t offset, uint8_t *strategy, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_STRATEGY_LIST_OFFSET + offset, strategy, len);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "Read strategy fail, ret : %d\n", ret);
    }

    return ret;
}

int32_t rt_write_apnlist(int32_t offset, uint8_t *apnlist, int32_t len)
{
    int32_t ret = rt_write_data(RUN_CONFIG_FILE, RT_APN_LIST_OFFSET + offset, apnlist, len);
    return ret;
}

int32_t rt_read_apnlist(int32_t offset, uint8_t *apnlist, int32_t len)
{
    int32_t ret = rt_read_data(RUN_CONFIG_FILE, RT_APN_LIST_OFFSET + offset, apnlist, len);
    return ret;
}

int32_t rt_truncate_apnlist(int32_t offset)
{
    rt_truncate_data(RUN_CONFIG_FILE, RT_APN_LIST_OFFSET + offset);
}
