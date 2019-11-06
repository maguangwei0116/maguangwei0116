
#include "rt_os.h"
#include "agent_queue.h"
#include "card_manager.h"
#include "card_detection.h"
#include "log.h"

#define CARD_DETECT_INTERVAL                10  // unit: seconds

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

static int32_t card_load_using_card(char *iccid, int32_t size, profile_type_e *type)
{
    if ((rt_os_strcmp(iccid, g_cur_iccid)) || (*type != *g_cur_profile_type)) {
        MSG_PRINTF(LOG_INFO, "iccid changed: (%s)[%d] ==> (%s)[%d]\r\n", iccid, *type, g_cur_iccid, *g_cur_profile_type);
        snprintf(iccid, size, "%s", g_cur_iccid);
        *type = *g_cur_profile_type;
        return RT_SUCCESS;
    } else {
        return RT_ERROR;
    }
}

static int32_t card_changed_handle(const char *iccid, profile_type_e type)
{
    int32_t ret = RT_ERROR;

    MSG_PRINTF(LOG_INFO, "card changed iccid: %s, type: %d\r\n", iccid, type);
    if (PROFILE_TYPE_OPERATIONAL == type) {
        card_set_opr_profile_apn();
    } else if (PROFILE_TYPE_PROVISONING == type) {
        msg_send_agent_queue(MSG_ID_BOOT_STRAP, MSG_BOOTSTRAP_SELECT_CARD, NULL, 0);
        card_detection_disable();
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

