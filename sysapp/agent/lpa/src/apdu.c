
#include <string.h>
#include <stdio.h>
#include "apdu.h"
#include "convert.h"
#include "lpa_config.h"
#include "lpa_error_codes.h"
#include "rt_qmi.h"

const static uint8_t euicc_aid[] = {0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF,
                                    0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00};
static int8_t g_channel = -1;

static uint16_t get_sw(char *rsp,int len)
{
    uint16_t sw;
    sw = ((uint16_t)rsp[len-2] << 8) + rsp[len-1];
    return sw;
}

static int open_channel(int8_t *channel)
{
    int ret = RT_SUCCESS;
    ret = rt_qmi_open_channel(euicc_aid,sizeof(euicc_aid),channel);
    MSG_INFO("Open Channel: %02X\n", *channel);
    return ret;
}

static int close_channel(int8_t channel)
{
    int ret = RT_SUCCESS;
    ret = rt_qmi_close_channel(channel);
    return ret;
}


int cmd_manage_channel(channel_opt_t operation)
{
    if (operation == OPEN_CHANNEL) {
        if (g_channel == -1) {
            RT_CHECK(open_channel(&g_channel));
        }
    } else if (operation == CLOSE_CHANNEL){
        if (g_channel != -1) {
            RT_CHECK(close_channel(g_channel));
            g_channel = -1;
        }
    }
    return RT_SUCCESS;
}

#define APDU_BLOCK_SIZE         255
int cmd_store_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    int i;
    int ret = RT_SUCCESS;
    uint16_t sw;
    uint8_t cnt, left;
    uint8_t cbuf[LPA_AT_BLOCK_BUF];
    apdu_t *apdu = (apdu_t *)cbuf;
    memset(cbuf,0x00,LPA_AT_BLOCK_BUF);
    apdu->cla = 0x80;     //cla
    apdu->ins = 0xE2;     //ins

    cnt = data_len / APDU_BLOCK_SIZE;
    left = data_len % APDU_BLOCK_SIZE;

    if (left != 0) {
        cnt++;
    }
    if (g_channel < 0) {
        if(cmd_manage_channel(OPEN_CHANNEL) != RT_SUCCESS) {
            MSG_ERR("Open channel error\n");
            return RT_ERR_APDU_OPEN_CHANNEL_FAIL;
        }
    }

    apdu->cla |= g_channel;
    for (i = 0; i < cnt; i++) {
        apdu->p2 = i;            //p2
        if (i == (cnt - 1)) {    // Last block
            apdu->p1 = 0x91;     //p1
            apdu->lc = (left == 0 ? APDU_BLOCK_SIZE : left);
        } else {                 // More blocks
            apdu->p1 = 0x11;     //p1
            apdu->lc = APDU_BLOCK_SIZE;
        }
        memset(&apdu->data,0x00,apdu->lc);
        memcpy(&apdu->data,data + i * APDU_BLOCK_SIZE,apdu->lc);
        cbuf[apdu->lc+5] = 0x00;
        ret = rt_qmi_send_apdu(cbuf,apdu->lc+6,rsp,rsp_len,g_channel);
        if (ret != RT_SUCCESS) {
            return RT_ERR_APDU_SEND_FAIL;
        }
        sw = get_sw(rsp,*rsp_len);
        *rsp_len = 0;
        do {
            if ((sw & 0xFF00) == 0x6100) {
                uint16_t size;
                uint8_t cmd[6] = {0x80,0xC0,0x00,0x00,0x00};
                size = (sw & 0xFF);
                cmd[0] = (g_channel & 0x03) | 0x80;   // Channel should be 0~3, and convert to hexstring
                cmd[4] = size;
                ret = rt_qmi_send_apdu(cmd,5,rsp,&size,g_channel);
                if (ret != RT_SUCCESS) {
                    return RT_ERR_APDU_SEND_FAIL;
                }
                *rsp_len += size-2;
                sw = get_sw(rsp,size);
                rsp += size-2;
            } else {
                break;
            }
        } while (1);
        if (sw != SW_NORMAL) {
                rsp[0] = (sw & 0xFF00) >> 8;
                rsp[1] = sw & 0xFF;
                *rsp_len = 2;
                MSG_ERR("SW: %04X\n", sw);
                return RT_ERR_APDU_STORE_DATA_FAIL;
        }
    }
    return RT_SUCCESS;
}
