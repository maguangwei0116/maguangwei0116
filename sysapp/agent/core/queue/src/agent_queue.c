
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_quectel.c
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_os.h"
#include "rt_type.h"
#include "agent_queue.h"

typedef struct AGENT_QUEUE {
    int64_t msg_typ;
    agent_msg_id_e msg_id;
    int32_t data_len;
    void *data_buf;
} agent_que_t;

typedef struct UPLOAD_QUEUE {
    int64_t msg_typ;
    int32_t data_len;
    void *data_buf;
} upload_que_t;

static int32_t g_queue_id = -1;
static int32_t g_upload_queue_id = -1;

// agent queue, communication between modules
static void agent_queue_task(void)
{
    agent_que_t que_t;
    int32_t len = 0;
    while (1) {
        if (rt_receive_queue_msg(g_queue_id, &que_t, len, 0, 0) == 0) {
            switch (que_t.msg_id) {
                case MSG_ID_BOOT_STRAP:

                break;
                case MSG_ID_CARD_MANAGER:

                break;
                case MSG_ID_LOG_MANAGER:

                break;
                case MSG_ID_OTA_UPGRADE:

                break;
                case MSG_ID_PERSONLLISE:

                break;
                case MSG_ID_REMOTE_CONFIG:

                break;
                default: {
                    break;
                }
            }
        }
        rt_os_free(que_t.data_buf);
    }
}

// upload queue, only deal with upload msg
static void upload_queue_task(void)
{
    upload_que_t que_t;
    int32_t len = 0;
    while (1) {
        if (rt_receive_queue_msg(g_upload_queue_id, &que_t, len, 0, 0) == 0) {
        }
        rt_os_free(que_t.data_buf);
    }
}

int32_t init_queue(void *arg)
{
    rt_task task_id = 0;
    rt_task upload_task_id = 0;
    int32_t ret = RT_ERROR;

    ret = rt_create_task(&task_id, (void *) agent_queue_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }

    ret = rt_create_task(&upload_task_id, (void *) upload_queue_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create task fail\n");
        return RT_ERROR;
    }
    g_queue_id = rt_creat_msg_queue("./", 168);
    if (g_queue_id < 0) {
        MSG_PRINTF(LOG_ERR, "creat msg queue fail\n");
        return RT_ERROR;
    }

    g_upload_queue_id = rt_creat_msg_queue("./", 169);
    if (g_upload_queue_id < 0) {
        MSG_PRINTF(LOG_ERR, "creat upload msg queue fail\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t msg_send_agent_queue(int32_t msgid, void *buffer, int32_t len)
{
    agent_que_t que_t;
    que_t.msg_id = msgid;
    que_t.data_buf = (void *)rt_os_malloc(len);
    rt_os_memcpy(que_t.data_buf, buffer, len);
    que_t.data_len = len;
    return rt_send_queue_msg(g_queue_id, (void *)&que_t, len, 0);
}