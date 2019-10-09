
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
typedef enum QUEUE_MSG_TYPE {
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

#define MAX_RECV_QUEUE_CNT  100

static int32_t g_queue_id = -1;
static int32_t g_upload_queue_id = -1;
static card_info_t **g_card_info;

static void idle_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    
    downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    status = downstream_msg->handler(downstream_msg->private_arg, downstream_msg->event, &downstream_msg->out_arg);

    upload_event_report(downstream_msg->event, (const char *)downstream_msg->tranId, status, downstream_msg->out_arg);
}

static void issue_cert_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    int32_t status = 0;
    int32_t ret = RT_ERROR;
    downstream_msg_t *downstream_msg = (downstream_msg_t *)buf;

    (void)mode;
    
    ret = downstream_msg->parser(downstream_msg->msg, downstream_msg->tranId, &downstream_msg->private_arg);
    if (downstream_msg->msg) {
        rt_os_free(downstream_msg->msg);
        downstream_msg->msg = NULL;
    }
    if (ret == RT_ERROR) {
        return;
    }
    status = downstream_msg->handler(downstream_msg->private_arg, downstream_msg->event, &downstream_msg->out_arg);
}

// agent queue, communication between modules
static void agent_queue_task(void)
{
    agent_que_t que_t;
    int32_t len = sizeof(agent_que_t) - sizeof(long);

    while (1) {
        rt_os_memset(&que_t, 0, sizeof(agent_que_t));
        if (rt_receive_queue_msg(g_queue_id, (void *) &que_t, len, AGENT_QUEUE_MSG_TYPE, 0) == RT_SUCCESS) {
            MSG_PRINTF(LOG_INFO, "que_t.msg_id:%d\n", que_t.msg_id);
            switch (que_t.msg_id) {
                case MSG_ID_BOOT_STRAP:
                    bootstrap_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_CARD_MANAGER:
                    //MSG_PRINTF(LOG_INFO, "que_t.data_len:%d\n", que_t.data_len);
                    card_manager_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_LOG_MANAGER:
                    break;

                case MSG_ID_OTA_UPGRADE:
                    ota_upgrade_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_PERSONLLISE:
                    issue_cert_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_REMOTE_CONFIG:

                    break;

                case MSG_ID_NETWORK_DECTION:
                    network_detection_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_BROAD_CAST_NETWORK:
                    upload_event(que_t.data_buf, que_t.data_len, que_t.mode);                    
                    mqtt_connect_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    ota_upgrade_task_check_event(que_t.data_buf, que_t.data_len, que_t.mode);                    
                    if ((*g_card_info)->type == PROFILE_TYPE_PROVISONING) {
                        bootstrap_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    }  
                    card_manager_event(que_t.data_buf, que_t.data_len, que_t.mode); // It will waste a few time
                    break;

                case MSG_ID_MQTT:
                    mqtt_connect_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_IDLE:
                    idle_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                case MSG_ID_DETECT_NETWORK:
                    network_detect_event(que_t.data_buf, que_t.data_len, que_t.mode);
                    break;

                default: {
                    break;
                }
            }

            // MSG_PRINTF(LOG_INFO, "que_t.data_len:%d, que_t.data_buf:%p\n", que_t.data_len, que_t.data_buf);
            if (que_t.data_buf && que_t.data_len != 0) {
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
            //MSG_PRINTF(LOG_INFO, "upload queue dealing ... que_t.data_buf: %p\r\n", que_t.data_buf);
            ret = upload_http_post(que_t.host_addr, que_t.port, que_t.cb, que_t.data_buf, que_t.data_len);
            if (ret) {
                MSG_PRINTF(LOG_WARN, "upload http post fail, ret=%d\r\n", ret);
            }
            if (que_t.data_buf && que_t.data_len != 0) {
                rt_os_free(que_t.data_buf);
            }
        }
    }
}

static int32_t agent_queue_clear_msg(int32_t time_cnt)
{
    int32_t i;
    int32_t ret;
    agent_que_t agent_queue;
    int32_t agent_queue_len = sizeof(agent_queue) - sizeof(long);

    if (g_queue_id == -1) {
        return RT_ERROR;
    }

    for (i = 0; i < time_cnt; i++) {
        rt_os_memset(&agent_queue, 0, sizeof(agent_queue));
        ret = rt_receive_queue_msg(g_queue_id, &agent_queue, agent_queue_len, AGENT_QUEUE_MSG_TYPE, RT_IPC_NOWAIT);
        if (ret == RT_ERROR && !agent_queue.data_buf) {
            break;
        }
        usleep(10*1000);  // delay 10ms
    }

    return RT_SUCCESS;
}

static int32_t upload_queue_clear_msg(int32_t time_cnt)
{
    int32_t i;
    int32_t ret;
    upload_que_t upload_queue;
    int32_t upload_queue_len = sizeof(upload_queue) - sizeof(long);

    if (g_upload_queue_id == -1) {
        return RT_ERROR;
    }

    for (i = 0; i < time_cnt; i++) {
        rt_os_memset(&upload_queue, 0, sizeof(upload_queue));
        ret = rt_receive_queue_msg(g_upload_queue_id, &upload_queue, upload_queue_len, UPLOAD_QUEUE_MSG_TYPE, RT_IPC_NOWAIT); 
        if (ret == RT_ERROR && !upload_queue.data_buf) {
            break;
        }
        usleep(10*1000);  // delay 10ms
    }

    return RT_SUCCESS;
}


int32_t init_queue(void *arg)
{
    rt_task task_id = 0;
    rt_task upload_task_id = 0;
    int32_t ret = RT_ERROR;

    g_card_info = &(((public_value_list_t *)arg)->card_info);
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

    agent_queue_clear_msg(MAX_RECV_QUEUE_CNT);
    upload_queue_clear_msg(MAX_RECV_QUEUE_CNT);

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


    return RT_SUCCESS;
}

int32_t msg_send_agent_queue(int32_t msgid, int32_t mode, void *buffer, int32_t len)
{
    agent_que_t que_t;

    que_t.msg_typ = AGENT_QUEUE_MSG_TYPE;
    que_t.msg_id = msgid;
    que_t.mode = mode;
    if (len > 0) {
        que_t.data_buf = (void *) rt_os_malloc(len + 1);
        rt_os_memcpy(que_t.data_buf, buffer, len);
        *(((uint8_t *)que_t.data_buf) + len) = '\0';
    } else {
        que_t.data_buf = NULL;
    }
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
    ret = rt_send_queue_msg(g_upload_queue_id, (void *)&que_t, len, 0);
    if (ret < 0) {
        MSG_PRINTF(LOG_ERR, "send upload msg queue fail, ret=%d, err(%d)=%s\n", ret, errno, strerror(errno));
    }

    return ret;
}
