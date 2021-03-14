
#include <string.h>
#include <stdio.h>
#include "apdu.h"
#include "convert.h"
#include "lpa_config.h"
#include "lpa_error_codes.h"
#include "rt_qmi.h"
#include "agent2monitor.h"
#include "lpa.h"

#define SW_BUFFER_LEN               100

const static uint8_t euicc_aid[] = {0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF,
                                    0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00};

static lpa_channel_type_e g_channel_mode = LPA_CHANNEL_BY_IPC;

static int32_t local_exchange_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
#ifdef CFG_PLATFORM_ANDROID
    extern int32_t rt_qmi_exchange_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len);
    return rt_qmi_exchange_apdu(data, data_len, rsp, rsp_len);
#else
    return ipc_send_data(SERVER_PATH, data, data_len, rsp, rsp_len);
#endif
}

void init_apdu_channel(lpa_channel_type_e channel_mode)
{
    g_channel_mode = channel_mode;
}

static int32_t lpa_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    if (g_channel_mode == LPA_CHANNEL_BY_IPC) {
        return local_exchange_apdu(data, data_len, rsp, rsp_len);
    } else if (g_channel_mode == LPA_CHANNEL_BY_QMI) {
        return rt_qmi_send_apdu(data, data_len, rsp, rsp_len, channel);
    }
}

static uint16_t get_sw(uint8_t *rsp, uint16_t len)
{
    uint16_t sw = 0;
    sw = ((uint16_t)rsp[len-2] << 8) + rsp[len-1];
    return sw;
}

static int32_t select_aid(uint8_t channel)
{
    // select aid
    uint8_t sw_61xx_req_cmd[5] = {0x80, 0xC0, 0x00, 0x00, 0x00};
    int ret = RT_SUCCESS;
    uint16_t sw = 0;
    uint8_t cbuf[LPA_AT_BLOCK_BUF] = {0};
    apdu_t *apdu = (apdu_t *)cbuf;
    uint8_t rsp[SW_BUFFER_LEN + 2] = {0};
    uint16_t rsp_len = sizeof(rsp);

    apdu->cla = channel & 0x03;
    apdu->ins = 0xA4;
    apdu->p1 = 0x04;
    apdu->p2 = 0x00;
    apdu->lc = sizeof(euicc_aid);
    memcpy(&apdu->data, euicc_aid, apdu->lc);
    cbuf[apdu->lc+5] = 0x00;
    ret = lpa_send_apdu((uint8_t *)cbuf, apdu->lc + 6, cbuf, &rsp_len, channel);
    if (ret != RT_SUCCESS) {
        return RT_ERR_APDU_SEND_FAIL;
    }

    sw = get_sw(cbuf, rsp_len);
    if ((sw & 0xFF00) == 0x6100) {
        rsp_len = (sw & 0xFF);
        sw_61xx_req_cmd[0] = (channel & 0x03) | 0x80;   // Channel should be 0~3, and convert to hexstring
        sw_61xx_req_cmd[4] = rsp_len;
        ret = lpa_send_apdu(sw_61xx_req_cmd, sizeof(sw_61xx_req_cmd), rsp, &rsp_len, channel);
        if (ret != RT_SUCCESS) {
            return RT_ERR_APDU_SEND_FAIL;
        }
        sw = get_sw(rsp, rsp_len);
    }
    if (sw != SW_NORMAL) {
        return RT_ERR_APDU_STORE_DATA_FAIL;
    }
    return ret;
}

int open_channel(uint8_t *channel)
{
    int ret = RT_SUCCESS;

    if (g_channel_mode == LPA_CHANNEL_BY_IPC) {
        uint8_t sw_61xx_req_cmd[5] = {0x80, 0xC0, 0x00, 0x00, 0x00};
        const uint8_t open_channel_cmd[] = {0x00, 0x70, 0x00, 0x00, 0x01};
        uint8_t rsp[SW_BUFFER_LEN + 2] = {0};
        uint16_t sw = 0;
        uint16_t len = sizeof(rsp);

        ret = local_exchange_apdu(open_channel_cmd, sizeof(open_channel_cmd), rsp, &len);
        if (ret != RT_SUCCESS) {
            return RT_ERR_APDU_SEND_FAIL;
        }

        sw = get_sw(rsp, len);
        if ((sw & 0xFF00) == 0x6100) {
            len = (sw & 0xFF);
            sw_61xx_req_cmd[4] = len;
            ret = local_exchange_apdu(sw_61xx_req_cmd, sizeof(sw_61xx_req_cmd), rsp, &len);
            if (ret != RT_SUCCESS) {
                return RT_ERR_APDU_SEND_FAIL;
            }
            sw = get_sw(rsp, len);
        }

        if (sw != SW_NORMAL) {
            return RT_ERR_UNKNOWN_ERROR;
        }
        *channel = rsp[0];
        MSG_DBG("open channel %d, ret:%d\n", *channel, ret);
        if(!ret) {
            ret = select_aid(*channel);
        }
    } else {
        ret = rt_qmi_open_channel(euicc_aid, sizeof(euicc_aid), channel);
    }
    return ret;
}

int close_channel(uint8_t channel)
{
    int ret = RT_SUCCESS;

    if (g_channel_mode == LPA_CHANNEL_BY_IPC) {
        uint8_t sw_61xx_req_cmd[5] = {0x80, 0xC0, 0x00, 0x00, 0x00};
        uint8_t close_channel_cmd[] = {0x00, 0x70, 0x80, 0x01, 0x00};
        uint8_t rsp[SW_BUFFER_LEN + 2] = {0};
        uint16_t sw = 0;
        uint16_t len = sizeof(rsp);

        close_channel_cmd[3] = channel;  // channel id fill into p2
        ret = local_exchange_apdu(close_channel_cmd, sizeof(close_channel_cmd), rsp, &len);
        if (ret != RT_SUCCESS) {
            return RT_ERR_APDU_SEND_FAIL;
        }

        sw = get_sw(rsp, len);
        if ((sw & 0xFF00) == 0x6100) {
            len = (sw & 0xFF);
            sw_61xx_req_cmd[0] = (channel & 0x03) | 0x80;   // Channel should be 0~3, and convert to hexstring
            sw_61xx_req_cmd[4] = len;
            ret = local_exchange_apdu(sw_61xx_req_cmd, sizeof(sw_61xx_req_cmd), rsp, &len);
            if (ret != RT_SUCCESS) {
                return RT_ERR_APDU_SEND_FAIL;
            }
            sw = get_sw(rsp, len);
        }

        if (sw != SW_NORMAL) {
            return RT_ERR_UNKNOWN_ERROR;
        }
    } else {
        ret = rt_qmi_close_channel(channel);
    }

    return ret;
}

#define APDU_BLOCK_SIZE         255
int cmd_store_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    int i;
    int ret = RT_SUCCESS;
    uint16_t sw = 0;
    uint8_t cnt, left;
    uint8_t cbuf[LPA_AT_BLOCK_BUF] = {0};
    apdu_t *apdu = (apdu_t *)cbuf;

    cnt = data_len / APDU_BLOCK_SIZE;
    left = data_len % APDU_BLOCK_SIZE;

    if (left != 0) {
        cnt++;
    }

    if (channel < 0) {
        return RT_ERR_APDU_OPEN_CHANNEL_FAIL;
    }

    apdu->cla = 0x80;     //cla
    apdu->ins = 0xE2;     //ins
    apdu->cla |= channel;
    for (i = 0; i < cnt; i++) {
        apdu->p2 = i;            //p2
        if (i == (cnt - 1)) {    // Last block
            apdu->p1 = 0x91;     //p1
            apdu->lc = (left == 0 ? APDU_BLOCK_SIZE : left);
        } else {                 // More blocks
            apdu->p1 = 0x11;     //p1
            apdu->lc = APDU_BLOCK_SIZE;
        }
        memset(&apdu->data, 0x00, apdu->lc);
        memcpy(&apdu->data, data + i * APDU_BLOCK_SIZE, apdu->lc);
        cbuf[apdu->lc + 5] = 0x00;
        ret = lpa_send_apdu(cbuf, apdu->lc + 6, rsp, rsp_len, channel);
        if (ret != RT_SUCCESS) {
            return RT_ERR_APDU_SEND_FAIL;
        }

        sw = get_sw(rsp, *rsp_len);
        do {
            if ((sw & 0xFF00) == 0x6100) {
                uint16_t size;
                uint8_t sw_61xx_req_cmd[5] = {0x80, 0xC0, 0x00, 0x00, 0x00};

                size = (sw & 0xFF);
                sw_61xx_req_cmd[0] = (channel & 0x03) | 0x80;   // Channel should be 0~3, and convert to hexstring
                sw_61xx_req_cmd[4] = size;
                ret = lpa_send_apdu(sw_61xx_req_cmd, sizeof(sw_61xx_req_cmd), rsp, &size, channel);
                if (ret != RT_SUCCESS) {
                    return RT_ERR_APDU_SEND_FAIL;
                }
                *rsp_len += size - 2;
                sw = get_sw(rsp, size);
                rsp += size - 2;
            } else if ((sw & 0xFF00) == SW_NORMAL) {
                *rsp_len -= 2;   //Remove 9000
                break;
            } else {
                break;
            }
        } while (1);

        if ((sw & 0xF000) != SW_NORMAL) {
                rsp[0] = (sw & 0xFF00) >> 8;
                rsp[1] = sw & 0xFF;
                *rsp_len = 2;
                MSG_ERR("error SW: %04X\n", sw);
                return RT_ERR_APDU_STORE_DATA_FAIL;
        }
    }
    return RT_SUCCESS;
}
