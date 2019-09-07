
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
#include <errno.h>
#include "agent_queue.h"
#include "bootstrap.h"
#include "card_manager.h"
#include "downstream.h"
#include "rt_type.h"
#include "rt_os.h"

/*
queue msg type must large than 0, it's a nonpositive mtype value !
or it will return errno(22,EINVAL), see "man msgsnd" !!
*/
typedef enum {
    UPLOAD_QUEUE_MSG_TYPE   = 1,
    AGENT_QUEUE_MSG_TYPE    = 2,
} queue_msg_type_e;

typedef struct AGENT_QUEUE {
    long                msg_typ;
    int32_t             msg_id;
    int32_t             mode;
    void *              data_buf;
    int32_t             data_len;
} agent_que_t;

typedef struct UPLOAD_QUEUE {
    long                msg_typ;
    char                host_addr[16];  // server ip addr
    int32_t             port;           // server port
    void *              cb;             // recv data callback function
    void *              data_buf;
    int32_t             data_len;
} upload_que_t;

static int32_t g_queue_id = -1;
static int32_t g_upload_queue_id = -1;

static void idle_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    MSG_PRINTF(LOG_INFO, "msg: %s ==> method: %s ==> event: %s\n", downstream_msg->msg, downstream_msg->method, downstream_msg->event);
    
    downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    //MSG_PRINTF(LOG_WARN, "tranId: %s, %p\n", downstream_msg->tranId, downstream_msg->tranId);

    status = downstream_msg->handler(downstream_msg->private_arg, &downstream_msg->out_arg);

    upload_event_report(downstream_msg->event, (const char *)downstream_msg->tranId, status, downstream_msg->out_arg);
}

// agent queue, communication between modules
static void agent_queue_task(void)
{
    agent_que_t que_t;
    int32_t len = sizeof(agent_que_t) - sizeof(long);
    while (1) {
        if (rt_receive_queue_msg(g_queue_id, (void *) &que_t, len, AGENT_QUEUE_MSG_TYPE, 0) == RT_SUCCESS) {
            MSG_PRINTF(LOG_INFO, "que_t.msg_id:%d\n", que_t.msg_id);
            switch (que_t.msg_id) {
                case MSG_ID_BOOT_STRAP:
                    bootstrap_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;
                    
                case MSG_ID_CARD_MANAGER:
                    MSG_INFO_ARRAY("que_t.data_buf:", (uint8_t *) que_t.data_buf, que_t.data_len);
                    card_manager_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;
                    
                case MSG_ID_LOG_MANAGER:
                    
                    break;
                    
                case MSG_ID_OTA_UPGRADE:
                    ota_upgrade_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;
                    
                case MSG_ID_PERSONLLISE:

                    break;
                    
                case MSG_ID_REMOTE_CONFIG:

                    break;

                case MSG_ID_NETWORK_DECTION:
                    network_detection_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;
                    
                case MSG_ID_BROAD_CAST_NETWORK:
                    card_manager_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    bootstrap_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_IDLE:
                    idle_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;
                    
                default: {
                    break;
                }
            }
            
            //MSG_PRINTF(LOG_INFO, "que_t.data_len:%d, que_t.data_buf:%p\n", que_t.data_len, que_t.data_buf);
            if (que_t.data_len != 0) {
                rt_os_free(que_t.data_buf);
            }
        }
    }
}

// upload queue, only deal with upload msg
static void upload_queue_task(void)
{
    upload_que_t que_t;
    int32_t ret;
    int32_t len = sizeof(upload_que_t) - sizeof(long);

    while (1) {
        rt_os_memset(&que_t, 0, sizeof(upload_que_t));
        if (rt_receive_queue_msg(g_upload_queue_id, &que_t, len, UPLOAD_QUEUE_MSG_TYPE, 0) == 0) {
            MSG_PRINTF(LOG_INFO, "upload queue dealing ...\r\n");
            ret = upload_http_post(que_t.host_addr, que_t.port, que_t.cb, que_t.data_buf, que_t.data_len);
            MSG_PRINTF(LOG_INFO, "upload http post:%d\n", ret);
            if (que_t.data_buf) {
                rt_os_free(que_t.data_buf);
            }
        }
        MSG_PRINTF(LOG_INFO, "upload queue task ...\n");
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

int32_t msg_send_agent_queue(int32_t msgid, int32_t mode, void *buffer, int32_t len)
{
    agent_que_t que_t;

    que_t.msg_typ = AGENT_QUEUE_MSG_TYPE;
    que_t.msg_id = msgid;
    que_t.mode = mode;
    if (len > 0) {
        que_t.data_buf = (void *) rt_os_malloc(len);
        rt_os_memcpy(que_t.data_buf, buffer, len);
    } else {
        que_t.data_buf = NULL;
    }
    MSG_PRINTF(LOG_INFO, "len:%d, data_buf:%p\n", len, que_t.data_buf);
    que_t.data_len = len;
    len = sizeof(agent_que_t) - sizeof(long);
    return rt_send_queue_msg(g_queue_id, (void *) &que_t, len, 0);
}

int32_t msg_send_upload_queue(const char *host_addr, int32_t port, void *cb, void *buffer, int32_t len)
{
    upload_que_t que_t;
    int32_t ret;

    snprintf(que_t.host_addr, sizeof(que_t.host_addr), "%s", host_addr);
    que_t.msg_typ   = UPLOAD_QUEUE_MSG_TYPE;
    que_t.port      = port;
    que_t.cb        = cb;
    que_t.data_len  = len;
    que_t.data_buf  = (void *)rt_os_malloc(que_t.data_len + 1);
    rt_os_memcpy(que_t.data_buf, buffer, len);
    *(((uint8_t *)que_t.data_buf) + len) = '\0';
    len = sizeof(upload_que_t) - sizeof(long);
    MSG_PRINTF(LOG_INFO, "len:%d, %p\n", len, que_t.data_buf);
    ret = rt_send_queue_msg(g_upload_queue_id, (void *)&que_t, len, 0);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "send upload msg queue fail, ret=%d, err(%d)=%s\n", ret, errno, strerror(errno));
    }

    return ret;
}