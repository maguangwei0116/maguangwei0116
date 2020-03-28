
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : vuicc_apdu03
 * Date        : 2020.03.12
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "vuicc_apdu.h"
#include "cos_api.h"
#include "parse_backup.h"
#include "trigger.h"
#include "log.h"

typedef enum APDU_RESPONSE_STATE{
    APDU_RESPONSE_NOT_USED = 0,
    APDU_RESPONSE_BASIC_USED,
    APDU_RESPONSE_LOGIC_USED
} apdu_response_state_e;

static apdu_response_state_e g_response_state = APDU_RESPONSE_NOT_USED;
static pthread_mutex_t g_apdu_mutex;

int32_t vuicc_lpa_cmd(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t cmd = 0;
    uint16_t sw = 0;
    static rt_bool reset_flag = RT_FALSE;

    if ((data[1] != 0xC0) && (g_response_state == APDU_RESPONSE_LOGIC_USED)) {
        rt_mutex_unlock(&g_apdu_mutex);
    } else if (g_response_state == APDU_RESPONSE_NOT_USED) {
        rt_mutex_lock(&g_apdu_mutex);
    }

    MSG_INFO_ARRAY("L-APDU REQ: ", data, data_len);
    cmd = (data[5] << 8) + data[6];
    cos_client_transport(IO_PACKET_TYPE_DATA, (uint8_t *)data, data_len, rsp, rsp_len);
    MSG_INFO_ARRAY("L-APDU RSP: ", rsp, *rsp_len);

    sw = ((uint16_t)rsp[*rsp_len - 2] << 8) + rsp[*rsp_len - 1];

    if ((sw & 0xFF00) == 0x6100) {
        g_response_state = APDU_RESPONSE_LOGIC_USED;
    } else {
        g_response_state = APDU_RESPONSE_NOT_USED;
        rt_mutex_unlock(&g_apdu_mutex);
    }
    MSG_PRINTF(LOG_INFO, "g_response_state:%d\n", g_response_state);

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

static int32_t vuicc_get_atr(uint8_t *atr, uint8_t *atr_size)
{
    rt_mutex_lock(&g_apdu_mutex);
    cos_client_reset((uint8_t *)atr, (uint16_t *)atr_size);
    rt_mutex_unlock(&g_apdu_mutex);
}

static int32_t vuicc_trigger_cmd(const uint8_t *apdu, uint16_t apdu_len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint16_t sw = 0;

    if ((apdu[1] != 0xC0) && (g_response_state == APDU_RESPONSE_BASIC_USED)) {
        rt_mutex_unlock(&g_apdu_mutex);
    } else if (g_response_state == APDU_RESPONSE_NOT_USED) {
        rt_mutex_lock(&g_apdu_mutex);
    }

    MSG_INFO_ARRAY("M-APDU REQ: ", apdu, apdu_len);
    cos_client_transport(IO_PACKET_TYPE_DATA, (uint8_t *)apdu, apdu_len, rsp, rsp_len);
    MSG_INFO_ARRAY("M-APDU RSP: ", rsp, *rsp_len);
    sw = ((uint16_t)rsp[*rsp_len - 2] << 8) + rsp[*rsp_len - 1];
    if ((sw & 0xFF00) == 0x6100) {
        g_response_state = APDU_RESPONSE_BASIC_USED;
    } else {
        g_response_state = APDU_RESPONSE_NOT_USED;
        rt_mutex_unlock(&g_apdu_mutex);
    }
    MSG_PRINTF(LOG_INFO, "g_response_state:%d\n", g_response_state);
}

void init_trigger(uint8_t uicc_switch)
{
    int32_t cos_oid = 0;
    if (uicc_switch == LPA_CHANNEL_BY_IPC) {
        do {
            cos_oid = cos_client_connect(NULL);
            if (cos_oid == -1) {
                sleep(1);
            }
        } while(cos_oid == -1);
        trigegr_regist_reset(vuicc_get_atr);
        trigegr_regist_cmd(vuicc_trigger_cmd);
        trigger_swap_card(1);
    }
}

static void start_cos(void *arg)
{
    init_callback_ops(arg);
    return;
}

int32_t init_vuicc(void *arg)
{
    unsigned long task_id;
    int32_t ret = RT_SUCCESS;

    ret = rt_mutex_init(&g_apdu_mutex);
    if (ret != 0) {
        MSG_PRINTF(LOG_ERR, "Mutex init failed. %d \n", ret);
    }

    if (ret = rt_create_task(&task_id, (void *)start_cos, arg) == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "Create thread failed\n");
    }

    return ret;
}
