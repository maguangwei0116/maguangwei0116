
#include <time.h>
#include "rt_os.h"
#include "agent_queue.h"
#include "card_manager.h"
#include "card_detection.h"
#include "log.h"

#define CARD_DETECT_INTERVAL                10  // unit: seconds
#define SUCCESSIVE_NO_NET_TIME              60  // successive interval time no net for a provisoning card, empirical value !!!
#define SUCCESSIVE_NO_NET_NUM               3   // check 3 timestamp
#define MAX_PROVISONING_KEEP_NUM            3   // one provisoning iccid max keep for 3 times

static int32_t g_card_detect_interval       = CARD_DETECT_INTERVAL;
static rt_bool g_card_detecting_flg         = RT_FALSE;
static const char *g_cur_iccid              = NULL;
static profile_type_e *g_cur_profile_type   = NULL;

int32_t card_detection_enable(void)
{
    g_card_detecting_flg = RT_TRUE;

    return RT_SUCCESS;
}

int32_t card_detection_disable(void)
{
    g_card_detecting_flg = RT_FALSE;

    return RT_SUCCESS;
}

/*
Provisoning iccid conflict cases:
1.The next three times of DSI_EVT_NET_NO_NET time are less than 60 seconds;
2.The provisoning iccid DSI_EVT_NET_NO_NET for 5 times.
Solution:
Start bootstrap to select a new provisoning iccid.
*/
static int32_t card_check_provisoning_conflict(rt_bool clear_flg)
{
    uint32_t i = 0;
    static uint32_t index = 0;
    static uint32_t cur_time[SUCCESSIVE_NO_NET_NUM] = {0};
    static char cur_iccid[THE_ICCID_LENGTH + 1] = {0};
    static uint32_t cur_cnt = 0;

    if (PROFILE_TYPE_OPERATIONAL == *g_cur_profile_type) {
        return RT_ERROR;
    }

    if (clear_flg) {
        index = 0;
        cur_cnt = 0;
        rt_os_memset(&cur_time, 0, sizeof(cur_time));
        return RT_SUCCESS;
    }

    /* check time interval frist */
    if (index != SUCCESSIVE_NO_NET_NUM) {
        cur_time[index++] = time(NULL);
    } else {
        for (i = 0; i < (SUCCESSIVE_NO_NET_NUM - 1); i++) {
            cur_time[i] = cur_time[i + 1];  
        }
        cur_time[i] = time(NULL);
    }

    MSG_PRINTF(LOG_INFO, "cur_time: %ld, [%ld], %ld, [%ld], %ld\r\n", \
        cur_time[0], cur_time[1] - cur_time[0], cur_time[1], cur_time[2] - cur_time[1], cur_time[2]);
    for (i = 0; i < (SUCCESSIVE_NO_NET_NUM - 1); i++) {
        if ((cur_time[i + 1] == 0) || (cur_time[i] == 0) || (cur_time[i + 1] - cur_time[i]) > SUCCESSIVE_NO_NET_TIME) {
            break;
        }
    }
    if (i == (SUCCESSIVE_NO_NET_NUM - 1)) {
        goto exit_entry;
    }

    /* check counter secondly */
    if (rt_os_strcmp(cur_iccid, g_cur_iccid)) {
        snprintf(cur_iccid, sizeof(cur_iccid), "%s", g_cur_iccid);
        cur_cnt = 1;
    } else {
        cur_cnt++;
    }
    MSG_PRINTF(LOG_INFO, "same provionsing iccid cnt: %d\r\n", cur_cnt);
    if (cur_cnt >= MAX_PROVISONING_KEEP_NUM) {
        MSG_PRINTF(LOG_WARN, "same provionsing iccid NO_NET too many, select a new one !\r\n");
        return RT_SUCCESS;
    }

    return RT_ERROR;

exit_entry:

    MSG_PRINTF(LOG_WARN, "provionsing iccid conflict detected !!!\r\n");
    return RT_SUCCESS;
}

static int32_t card_load_using_card(char *iccid, int32_t size, profile_type_e *type)
{
    if (PROFILE_TYPE_PROVISONING == *g_cur_profile_type || PROFILE_TYPE_TEST == *g_cur_profile_type) {
        if (*type != *g_cur_profile_type) { /* provisoning -> operational */
            MSG_PRINTF(LOG_WARN, "provionsing iccid detected [%d] ==> [%d]\r\n", *type, *g_cur_profile_type);
            *type = *g_cur_profile_type;
            return RT_SUCCESS;
        } else {     
            MSG_PRINTF(LOG_INFO, "provionsing iccid detected ...\r\n");
            *type = *g_cur_profile_type;
            return RT_ERROR;
        }
    }
    
    if (PROFILE_TYPE_OPERATIONAL == *g_cur_profile_type) {
        if ((rt_os_strcmp(iccid, g_cur_iccid)) || (*type != *g_cur_profile_type)) {
            MSG_PRINTF(LOG_WARN, "iccid changed: (%s)[%d] ==> (%s)[%d]\r\n", iccid, *type, g_cur_iccid, *g_cur_profile_type);
            snprintf(iccid, size, "%s", g_cur_iccid);
            *type = *g_cur_profile_type;
            return RT_SUCCESS;
        }
    } 

    return RT_ERROR;
}

static int32_t card_changed_handle(const char *iccid, profile_type_e type)
{
    int32_t ret = RT_ERROR;

    MSG_PRINTF(LOG_INFO, "card changed iccid: %s, type: %d\r\n", iccid, type);
    if (PROFILE_TYPE_OPERATIONAL == type) {
        card_set_opr_profile_apn();
    } else if (PROFILE_TYPE_PROVISONING == type || PROFILE_TYPE_TEST == type) {
        msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_BOOTSTRAP_SELECT_CARD, NULL, 0);
        card_detection_disable();
        card_check_provisoning_conflict(RT_TRUE);
    } else { 
        ;
    }

    return ret;
}

static void card_detection_task(void)
{
    profile_type_e  type = PROFILE_TYPE_TEST;
    char            iccid[THE_ICCID_LENGTH+1] = {0};
    
    rt_os_sleep(5);
    card_load_using_card(iccid, sizeof(iccid), &type);
    MSG_PRINTF(LOG_INFO, "g_cur_iccid: %s, g_cur_profile_type: %d\r\n", g_cur_iccid, *g_cur_profile_type);
    
    while (1) {
        if (g_card_detecting_flg) {
            msg_send_agent_queue(MSG_ID_CARD_MANAGER, MSG_CARD_UPDATE, NULL, 0);
            rt_os_sleep(2);
            if (RT_SUCCESS == card_load_using_card(iccid, sizeof(iccid), &type)) {
                card_changed_handle((const char *)iccid, type);
            }

            rt_os_sleep(g_card_detect_interval);
        }

        rt_os_msleep(100);
    }

    rt_exit_task(NULL);
}

int32_t card_detection_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t ret = RT_ERROR;

    switch (mode) {
        case MSG_NETWORK_DISCONNECTED:
            if (RT_SUCCESS == card_check_provisoning_conflict(RT_FALSE)) {
                card_changed_handle("", PROFILE_TYPE_PROVISONING);   
            }
            break;
            
        case MSG_NETWORK_CONNECTED:
            break;
    }

    return ret;
}

int32_t init_card_detection(void *arg)
{
    int32_t ret = RT_ERROR;
    rt_task id_detection;

    g_cur_profile_type  = &(((public_value_list_t *)arg)->card_info->type);
    g_cur_iccid         = (const char *)&(((public_value_list_t *)arg)->card_info->iccid);

    ret = rt_create_task(&id_detection, (void *)card_detection_task, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "create card detection task error, err(%d)=%s\r\n", errno, strerror(errno));
    }
    
    return ret;
}

