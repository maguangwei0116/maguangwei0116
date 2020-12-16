
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "at.h"
#include "rt_os.h"
#include "rt_type.h"
#include "log.h"
#include "agent_queue.h"

#define AT_CONFIG_ROAMSERVICE       "AT+QCFG=\"ROAMSERVICE\",2\r\n"     // 2 is force open
#define AT_CONFIG_DIAL              "AT+QCFG=\"QCAUTOCONNECT\",0\r\n"
#define AT_CONFIG_ECM               "AT+QCFG=\"USBNET\",1\r\n"
#define AT_ECM_STATE                "AT+QCFG=\"USBNET\"\r\n"
#define AT_ECM_SELECT               "usbnet"

#define MBN_USED_ITEM               "at+qmbncfg=\"select\"\r\n"
#define MBN_CONFIG_ONE_ITEM         "at+qmbncfg=\"select\",\"%s\"\r\n"
#define MBN_AUTO_SELECT_STATE       "at+qmbncfg=\"autosel\"\r\n"
#define MBN_CONFIG_AUTO_STATE       "at+qmbncfg=\"autosel\",%d\r\n"
#define MNB_CONFIG_DEACTUCATE       "at+qmbncfg=\"Deactivate\"\r\n"
#define MBN_ROW_ITEM                "ROW_Generic_3GPP"
#define MBN_AUTO_SELECT             "AutoSel"
#define MBN_ECHO_OFF                "ATE0\r\n"
#define MBN_ECHO_ON                 "ATE1\r\n"
#define MBN_RTY_SEND_AT_TIMES       3
#define MBN_AT_TIMEOUT              1000

static rt_bool mbn_judge_used_item(const char *item)
{
    char at_rsp[128];
    int32_t num = MBN_RTY_SEND_AT_TIMES;
    
    while (num != 0) {
        if (at_send_recv(MBN_USED_ITEM, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
            MSG_PRINTF(LOG_WARN, "send data error\n");
        }
        if (rt_os_strstr(at_rsp, item) != NULL) {
            break;
        }
        num--;
        rt_os_sleep(3);
    }
    if (num == 0) {
        return RT_FALSE;
    }
    return RT_TRUE;
}

static rt_bool mbn_set_item(const char *item)
{
    char at_rsp[128];
    char at_req[128];
    
    snprintf(at_req, sizeof(at_req), MBN_CONFIG_ONE_ITEM, item);
    if (at_send_recv(at_req, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "send data error\n");
        return RT_FALSE;
    }
    return RT_TRUE;
}

static rt_bool mbn_get_auto_state(void)
{
    char at_rsp[128];
    char *str = NULL;
    int32_t num = MBN_RTY_SEND_AT_TIMES;
    
    while (num != 0) {
        if (at_send_recv(MBN_AUTO_SELECT_STATE, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
            MSG_PRINTF(LOG_WARN, "send data error\n");
        }
        str = rt_os_strstr(at_rsp, MBN_AUTO_SELECT);
        if (str != NULL) {
            break;
        }
        num--;
        sleep(3);
    }
    if (num == 0) {
        return RT_TRUE;
    }
    if (str[9] == '0') {
        return RT_FALSE;
    }
    return RT_TRUE;
}

static rt_bool mbn_set_auto_state(int32_t state)
{
    char at_rsp[128];
    char at_req[128];

    if (at_send_recv(MNB_CONFIG_DEACTUCATE, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "send data error\n");
    }

    snprintf(at_req, sizeof(at_req), MBN_CONFIG_AUTO_STATE, state);
    if (at_send_recv(at_req, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
        MSG_PRINTF(LOG_WARN, "send data error\n");
    }

    return RT_TRUE;
}

static rt_bool mbn_get_ecm_state(void)
{
    char at_rsp[128];
    char *str = NULL;
    int32_t num = MBN_RTY_SEND_AT_TIMES;

    while (num != 0) {
        if (at_send_recv(AT_ECM_STATE, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT) != RT_SUCCESS) {
            MSG_PRINTF(LOG_WARN, "send data error\n");
        }
        MSG_PRINTF(LOG_WARN, "ecm rsp: %s\n", at_rsp);
        str = rt_os_strstr(at_rsp, AT_ECM_SELECT);
        if (str != NULL) {
            break;
        }
        num--;
        sleep(3);
    }
    if (num == 0) {
        return RT_FALSE;
    }

    MSG_PRINTF(LOG_WARN, "strstr str: %s\n", str);

    if (str[8] == '1') {
        return RT_TRUE;
    }

    return RT_FALSE;
}

static int32_t mbn_config_device(void)
{
    char at_rsp[128];

    if (mbn_get_ecm_state() == RT_FALSE) {
        at_send_recv(AT_CONFIG_ECM, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT);            // Set ECM
        MSG_PRINTF(LOG_INFO, "ECM config ok\r\n");
    }

    at_send_recv(AT_CONFIG_DIAL, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT);               // Cancel automatic dialing
    at_send_recv(AT_CONFIG_ROAMSERVICE, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT);        // Set roaming switch
    at_send_recv(MBN_ECHO_OFF, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT);

    if (mbn_get_auto_state() == RT_FALSE) {
        if (mbn_judge_used_item(MBN_ROW_ITEM) == RT_TRUE) {
            at_send_recv(MBN_ECHO_ON, at_rsp, sizeof(at_rsp), MBN_AT_TIMEOUT);  // open ATE for sifar special !!!
            MSG_PRINTF(LOG_INFO, "MBN config ok\r\n");
            return RT_SUCCESS;
        }
    }

    mbn_set_auto_state(0);
    mbn_set_item(MBN_ROW_ITEM);
    MSG_PRINTF(LOG_WARN, "Reboot to active MBN config ...\r\n");
    rt_os_sleep(3);
    rt_os_reboot();
    
    return RT_ERROR;
}

int32_t init_mbn(void *arg)
{
    int32_t ret;
    uint8_t mbn_enable;

    mbn_enable = ((public_value_list_t *)arg)->config_info->mbn_enable;
    
    ret = init_at(NULL);
    if (!ret) {
        if (mbn_enable) {
            ret = mbn_config_device();
        }
    }

    return ret;
}
