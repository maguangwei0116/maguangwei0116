#include "apdu.h"
#include "at.h"
#include "converter.h"
#include "lpa_config.h"

#include <string.h>
#include <stdio.h>



#if (SEND_APDU_METHOD == AT)
#define SW_BUFFER_LEN               100
#define OPEN_CHANNEL_CMD            "0070000001"
#elif (SEND_APDU_METHOD == QMI)
#include "qmi_uim.h"
#elif (SEND_APDU_METHOD == FIBCOM)
#include "fibofwk.h"
#include "fibo_info_interface.h"
#include "fibo_sim_interface.h"
#include "fibo_mrc_interface.h"

#define SW_BUFFER_LEN               1024
#define OPEN_CHANNEL_CMD            "0070000001"
#endif

const static uint8_t euicc_aid[] = {0xA0, 0x00, 0x00, 0x05, 0x59, 0x10, 0x10, 0xFF,
                                    0xFF, 0xFF, 0xFF, 0x89, 0x00, 0x00, 0x01, 0x00};
static int8_t g_channel = -1;

#if SEND_APDU_METHOD == AT
static uint16_t get_sw(char *rsp)
{
    uint16_t sw;
    uint8_t tmp[2];
    int len = strlen(rsp);

    RT_CHECK(hexstring2bytes(&rsp[len-4], tmp, &sw));
    sw = ((uint16_t)tmp[0] << 8) + tmp[1];

    return sw;
}
static int select_aid(int8_t channel)
{
    uint16_t sw;
    apdu_t apdu = {0};
    char buf[2 * (sizeof(euicc_aid) + 5) + 1];
    char rsp[SW_BUFFER_LEN];

    apdu.cla = channel & 0x03;
    apdu.ins = 0xA4;
    apdu.p1 = 0x04;
    apdu.p2 = 0x00;
    apdu.lc = sizeof(euicc_aid);
    apdu.data = euicc_aid;

    RT_CHECK(bytes2hexstring((uint8_t *)&apdu, 5, buf));
    RT_CHECK(bytes2hexstring((uint8_t *)apdu.data, apdu.lc, buf + 10));
    RT_CHECK(lpa_send_at(buf, rsp));

    sw = get_sw(rsp);
    if ((sw != SW_NORMAL) && ((sw & 0x6100) != 0x6100)) {
        MSG_ERR("SW: %04X\n", sw);
        // SW should be 9000 OR 61xx
        return RT_ERR_APDU_SELECT_AID_FAIL;
    }

    MSG_INFO("Select AID: %s\n", rsp);

    return RT_SUCCESS;
}
int cmd_get_response(uint8_t channel, uint8_t size, char *rsp)
{
    int ret = RT_SUCCESS;
    char cmd[11] = "80C0000000";

    cmd[1] = (channel & 0x03) + '0';  // Channel should be 0~3, and convert to hexstring
    RT_CHECK(bytes2hexstring(&size, 1, &cmd[8]));
    MSG_DBG("CMD: %s\n", cmd);

    ret = lpa_send_at(cmd, rsp);
    if (ret != RT_SUCCESS) {
        rsp[0] = '\0';
    }
    MSG_DBG("RSP: %s\n", rsp);

    return ret;
}
#elif SEND_APDU_METHOD == QMI     //used qmi to send apdu

static uint16_t get_sw(char *rsp,int len)
{
    uint16_t sw;
    sw = ((uint16_t)rsp[len-2] << 8) + rsp[len-1];
    return sw;
}

#elif SEND_APDU_METHOD == FIBCOM  // use fibcom api to send apdu

#define MAX_APDU_RESPONSE_LEN       2048
static uint16_t get_sw(char *rsp)
{
    uint16_t sw;
    uint8_t tmp[2];
    int len = strlen(rsp);

    RT_CHECK(hexstring2bytes(&rsp[len-4], tmp, &sw));
    sw = ((uint16_t)tmp[0] << 8) + tmp[1];

    return sw;
}

static fibo_result_t fibcom_send_apdu(const uint8_t *req, uint16_t req_len, uint8_t *resp, uint16_t *resp_len)
{
    fibo_result_t result;
    uint8_t *response_apdu = NULL;
    size_t  response_apdu_num_elements = 0;

    if ((response_apdu = malloc(MAX_APDU_RESPONSE_LEN)) == NULL) {
        return RT_ERR_NULL_POINTER;
    }

    fibo_sim_ConnectService();
    result = fibo_sim_SendApdu(FIBO_SIM_EXTERNAL_SLOT_1, req, req_len, response_apdu, &response_apdu_num_elements);
    MSG_DBG("fibo_sim_SendApdu:%s response_apdu => %s, response_apdu_num_elements => %d, result:%s\n", (char *)req, (char *)response_apdu, response_apdu_num_elements, FIBO_RESULT_TXT(result));
    if (result != FIBO_OK) {
        result = fibo_sim_SendApdu(FIBO_SIM_EXTERNAL_SLOT_1, req, req_len, response_apdu, &response_apdu_num_elements);
        MSG_DBG("fibo_sim_SendApdu:%s response_apdu => %s, response_apdu_num_elements => %d, result:%s\n", (char *)req, (char *)response_apdu, response_apdu_num_elements, FIBO_RESULT_TXT(result));
    }
    memcpy(resp, response_apdu, response_apdu_num_elements);
    resp[response_apdu_num_elements] = '\0';
    *resp_len = response_apdu_num_elements;
    fibo_sim_DisconnectService();
    free(response_apdu);
    return result;
}

static int select_aid(int8_t channel)
{
    uint16_t sw;
    apdu_t apdu = {0};
    char buf[2 * (sizeof(euicc_aid) + 6) + 1];
    char rsp[SW_BUFFER_LEN];
    uint16_t rsp_len;

    apdu.cla = channel & 0x03;
    apdu.ins = 0xA4;
    apdu.p1 = 0x04;
    apdu.p2 = 0x00;
    apdu.lc = sizeof(euicc_aid);
    apdu.data = euicc_aid;
    apdu.le = 0x00;

    RT_CHECK(bytes2hexstring((uint8_t *)&apdu, 5, buf));
    RT_CHECK(bytes2hexstring((uint8_t *)apdu.data, apdu.lc, buf + 10));
    RT_CHECK(bytes2hexstring(&apdu.le, 1, buf + 10 + apdu.lc * 2));
    buf[2 * (sizeof(euicc_aid) + 6)] = '\0';
    RT_CHECK_EQ(fibcom_send_apdu(buf, strlen(buf), rsp, &rsp_len), FIBO_OK);

    sw = get_sw(rsp);
    if ((sw != SW_NORMAL) && ((sw & 0x6100) != 0x6100)) {
        MSG_ERR("SW: %04X\n", sw);
        // SW should be 9000 OR 61xx
        return RT_ERR_APDU_SELECT_AID_FAIL;
    }

    MSG_INFO("Select AID: %s\n", rsp);
    return RT_SUCCESS;
}

int cmd_get_response(uint8_t channel, uint8_t size, char *rsp)
{
    int ret = RT_SUCCESS;
    char cmd[11] = "80C0000000";
    fibo_result_t result;
    uint16_t rsp_len;

    cmd[1] = (channel & 0x03) + '0';  // Channel should be 0~3, and convert to hexstring
    RT_CHECK(bytes2hexstring(&size, 1, &cmd[8]));
    MSG_DBG("CMD: %s\n", cmd);

    result = fibcom_send_apdu(cmd, 10, rsp, &rsp_len);
    if (result != FIBO_OK) {
        rsp[0] = '\0';
    }
    MSG_DBG("RSP: %s\n", rsp);

    return ret;
}


#endif

static int open_channel(int8_t *channel)
{
    int ret = RT_SUCCESS;
#if SEND_APDU_METHOD == AT
    char rsp[SW_BUFFER_LEN + 2] = {0};

    RT_CHECK(lpa_send_at(OPEN_CHANNEL_CMD, rsp));

    *channel = ((rsp[0] & 0x0F) << 4) + (rsp[1] & 0x0F);
    if (*channel == 0x00) {  // Open failed, close channel 1 and retry
        RT_CHECK(lpa_send_at("0070800100", rsp));
        RT_CHECK(lpa_send_at(OPEN_CHANNEL_CMD, rsp));
        *channel = ((rsp[0] & 0x0F) << 4) + (rsp[1] & 0x0F);
    }

    if (get_sw(rsp) != SW_NORMAL) {
        *channel = 0;
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
    }
#elif SEND_APDU_METHOD == QMI
    ret = qmi_open_channel(euicc_aid,sizeof(euicc_aid),channel);
#elif SEND_APDU_METHOD == FIBCOM
    char rsp[SW_BUFFER_LEN + 2] = {0};
    uint16_t rsp_len;

    RT_CHECK_EQ(fibcom_send_apdu((const uint8_t *)OPEN_CHANNEL_CMD, 10, (uint8_t *)rsp, &rsp_len), FIBO_OK);
    *channel = (((rsp[0] - '0') & 0x0F) << 4) + ((rsp[1] - '0') & 0x0F);

    if (*channel == 0x00) {  // Open failed, close channel 1 and retry
        RT_CHECK_EQ(fibcom_send_apdu((const uint8_t *)"0070800100", 10, (uint8_t *)rsp, &rsp_len), FIBO_OK);
        RT_CHECK_EQ(fibcom_send_apdu((const uint8_t *)OPEN_CHANNEL_CMD, 10, (uint8_t *)rsp, &rsp_len), FIBO_OK);
        *channel = ((rsp[0] & 0x0F) << 4) + (rsp[1] & 0x0F);
    }

    if (get_sw(rsp) != SW_NORMAL) {
        *channel = 0;
        ret = RT_ERR_APDU_OPEN_CHANNEL_FAIL;
    }
#endif
    MSG_INFO("Open Channel: %02X\n", *channel);
    return ret;
}

static int close_channel(int8_t channel)
{
    int ret = RT_SUCCESS;
#if SEND_APDU_METHOD == AT
    char cmd[11] = "0070800100";
    char rsp[SW_BUFFER_LEN] = {0};

    cmd[6] = (channel & 0xF0) + '0';
    cmd[7] = (channel & 0x0F) + '0';

    ret = lpa_send_at(cmd, rsp);
    if (ret == RT_ERR_AT_WRONG_RSP) {
        ret = RT_SUCCESS;
    }
#elif SEND_APDU_METHOD == QMI
    ret = qmi_close_channel(channel);
#elif SEND_APDU_METHOD == FIBCOM
    char cmd[11] = "0070800100";
    char rsp[SW_BUFFER_LEN] = {0};
    uint16_t rsp_len;

    cmd[6] = (channel & 0xF0) + '0';
    cmd[7] = (channel & 0x0F) + '0';

    ret = fibcom_send_apdu((const uint8_t *)cmd, 10, (uint8_t *)rsp, &rsp_len);
    if (ret == FIBO_OK) {
        ret = RT_SUCCESS;
    }
#endif
    return ret;
}


int cmd_manage_channel(channel_opt_t operation)
{
#if SEND_APDU_METHOD == AT
    char buf[8];  // Expect to get ATE0\n OR OK\r\n\r\n, that's 5 OR 6
    rt_send_at("ATE0\r\n", buf, 1000);
#endif
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

#if SEND_APDU_METHOD == QMI
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
        ret = qmi_send_apdu(cbuf,apdu->lc+6,rsp,rsp_len,g_channel);
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
                ret = qmi_send_apdu(cmd,5,rsp,&size,g_channel);
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
#elif SEND_APDU_METHOD == AT
#define APDU_BLOCK_SIZE         249
int cmd_store_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    int i;
    int ret = RT_SUCCESS;
    uint16_t sw;
    uint8_t cnt, left;
    apdu_t apdu = {0};
    char cbuf[LPA_AT_BLOCK_BUF];
    char rbuf[SW_BUFFER_LEN];

    apdu.cla = 0x80;
    apdu.ins = 0xE2;

    cnt = data_len / APDU_BLOCK_SIZE;
    left = data_len % APDU_BLOCK_SIZE;

    if (left != 0) {
        cnt++;
    }

    if (g_channel < 0) {
        RT_CHECK(cmd_manage_channel(OPEN_CHANNEL));
    }

    RT_CHECK(select_aid(g_channel));

    apdu.cla |= g_channel;
    for (i = 0; i < cnt; i++) {
        memset(cbuf, 0, LPA_AT_BLOCK_BUF);
        apdu.p2 = i;
        apdu.data = data + i * APDU_BLOCK_SIZE;
        if (i == (cnt - 1)) {   // Last block
            apdu.p1 = 0x91;
            apdu.lc = (left == 0 ? APDU_BLOCK_SIZE : left);
        } else {                // More blocks
            apdu.p1 = 0x11;
            apdu.lc = APDU_BLOCK_SIZE;
        }
        RT_CHECK(bytes2hexstring((uint8_t *)&apdu, 5, cbuf));
        RT_CHECK(bytes2hexstring((uint8_t *)apdu.data, apdu.lc, cbuf + 10));
        cbuf[2 * (apdu.lc + 5)]      = (apdu.le & 0xF0) + '0';  // Append le(default 0x00)
        cbuf[2 * (apdu.lc + 5) + 1]  = (apdu.le & 0x0F) + '0';
        MSG_DBG("CMD: %s\n", cbuf);
        RT_CHECK(lpa_send_at(cbuf, rbuf));
        MSG_DBG("RSP: %s\n", rbuf);

        *rsp_len = 2;
        sw = get_sw(rbuf);
        do {
            if ((sw & 0xFF00) == 0x6100) {
                uint8_t *p;  // For reading response
                uint16_t size;
                size = (sw & 0xFF);
                *rsp_len -= 2;
                p = rsp + *rsp_len;
                ret = cmd_get_response(g_channel, size, cbuf);
                if (ret != RT_SUCCESS) {
                    return ret;
                }
                RT_CHECK(hexstring2bytes(cbuf, p, &size));
                *rsp_len += size;
                sw = get_sw(cbuf);
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
#elif SEND_APDU_METHOD == FIBCOM
#define APDU_BLOCK_SIZE         255
int cmd_store_data(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    int i;
    int ret = RT_SUCCESS;
    uint16_t sw;
    uint8_t cnt, left;
    apdu_t apdu = {0};
    char cbuf[LPA_AT_BLOCK_BUF];
    char rbuf[SW_BUFFER_LEN];

    apdu.cla = 0x80;
    apdu.ins = 0xE2;

    cnt = data_len / APDU_BLOCK_SIZE;
    left = data_len % APDU_BLOCK_SIZE;

    if (left != 0) {
        cnt++;
    }

    if (g_channel < 0) {
        RT_CHECK(cmd_manage_channel(OPEN_CHANNEL));
    }

    RT_CHECK(select_aid(g_channel));

    apdu.cla |= g_channel;
    for (i = 0; i < cnt; i++) {
        memset(cbuf, 0, LPA_AT_BLOCK_BUF);
        apdu.p2 = i;
        apdu.data = data + i * APDU_BLOCK_SIZE;
        if (i == (cnt - 1)) {   // Last block
            apdu.p1 = 0x91;
            apdu.lc = (left == 0 ? APDU_BLOCK_SIZE : left);
        } else {                // More blocks
            apdu.p1 = 0x11;
            apdu.lc = APDU_BLOCK_SIZE;
        }
        RT_CHECK(bytes2hexstring((uint8_t *)&apdu, 5, cbuf));
        RT_CHECK(bytes2hexstring((uint8_t *)apdu.data, apdu.lc, cbuf + 10));
//        cbuf[2 * (apdu.lc + 5)]      = (apdu.le & 0xF0) + '0';  // Append le(default 0x00)
//        cbuf[2 * (apdu.lc + 5) + 1]  = (apdu.le & 0x0F) + '0';
        RT_CHECK_EQ(fibcom_send_apdu(cbuf, strlen(cbuf), rbuf, rsp_len), FIBO_OK);

        *rsp_len = 2;
        sw = get_sw(rbuf);
        do {
            if (((sw & 0xFF00) == 0x6100) || ((sw & 0xFF00) == 0x9100)) {
                uint8_t *p;  // For reading response
                uint16_t size;
                size = (sw & 0xFF);
                *rsp_len -= 2;
                p = rsp + *rsp_len;
                ret = cmd_get_response(g_channel, size, rbuf);
                if (ret != RT_SUCCESS) {
                    return ret;
                }
                RT_CHECK(hexstring2bytes(rbuf, p, &size));
                *rsp_len += size;
                sw = get_sw(rbuf);
            } else if (sw == SW_NORMAL) {
                // uint8_t *p;  // For reading response
                // p = rsp;
                // RT_CHECK(hexstring2bytes(rbuf, p, rsp_len));
                break;
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
#endif
