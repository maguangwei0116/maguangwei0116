
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_msg_queue.c
 * Date        : 2019.10.31
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/
 
#include "rt_type.h"
#include "rt_os.h"

static pthread_mutex_t g_queue_mutex;

typedef struct queue_struct         queue_t;
typedef struct queue_node_struct    queue_node_t;

typedef struct queue_node_struct {
    void *                  data;
    int32_t                 len;
    queue_node_t *          next;
} queue_node_t;

typedef struct queue_struct {
    queue_node_t *          head;
    queue_node_t *          tail;
    pthread_mutex_t         mutex;
    int32_t                 count;    
} queue_t;

static queue_node_t *queue_node_new(const void *data, int32_t len)
{
    queue_node_t *qn = NULL;

    qn = (queue_node_t *)rt_os_malloc(sizeof(queue_node_t));
    if (!qn) {
        MSG_PRINTF(LOG_ERR, "Qn malloc fialed\n");
        return NULL;
    }

    if (len > 0) {
        qn->data = (void *)rt_os_malloc(len + 1);
        qn->len = len;
        rt_os_memcpy(qn->data, data, len);
        *(((uint8_t *)qn->data) + len) = '\0';
    } else {
        qn->data = NULL;
    }
    qn->next = NULL;

    return qn;
}

static void queue_node_free(queue_node_t *qn)
{
    if (qn) {
        if (qn->data) {
            rt_os_free(qn->data);
        }
        rt_os_free(qn);
    }
}

static queue_t *queue_new(void)
{
    queue_t *q = (queue_t *)rt_os_malloc(sizeof(queue_t));

    if (q == NULL) {
        MSG_PRINTF(LOG_ERR, "Q malloc failed\n");
        return NULL;
    }
    q->count = 0;
    q->head = NULL;
    q->tail = NULL;

    return q;
}

static void queue_free(queue_t *q)
{
    queue_node_t *q_node;
    queue_node_t *item;

    if (q) {
        item = q->head;
        while (item) {
            q_node = item;
            item = item->next;
            queue_node_free(q_node);
        }
    }
}

static int32_t send_queue_data(queue_t *q, const void *data, int32_t len)
{
    queue_node_t *q_node = NULL;
    int32_t ret = RT_SUCCESS;

    q_node = queue_node_new(data ,len);
    if (!q_node) {
        MSG_PRINTF(LOG_ERR, "Queue new queue node fialed!!\n");
        ret = RT_ERROR;
    } else {
        rt_mutex_lock(&q->mutex);
        if(q->head == NULL) {
            q->head = q->tail = q_node;
        } else {
            q->tail->next = q_node;
            q->tail = q_node;
        }
        q->count++;
        rt_mutex_unlock(&q->mutex);
    }

    return ret;
}

static int32_t receive_queue_data(queue_t *q, void *data, int32_t *len)
{
    queue_node_t *q_node;
    int32_t ret = RT_SUCCESS;

    if (q->head) {
        rt_mutex_lock(&q->mutex);
        rt_os_memcpy(data, q->head->data, q->head->len);
        *len = q->head->len;
        q_node = q->head;
        q->head = q->head->next;
        if (q->head == NULL) {
            q->tail = NULL;
        }
        queue_node_free(q_node);
        q->count--;
        rt_mutex_unlock(&q->mutex);
    } else {
        rt_os_msleep(500);
        ret = RT_ERROR;
    }

    return ret;
}

int32_t queue_size(queue_t *q)
{
    return q->count;
}

int32_t queue_empty(queue_t *q)
{
    if (q->head == NULL) {
        return RT_SUCCESS;
    }

    return RT_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_MSG_QUEUE_CNT   10
#define MAX_BUFFER_SIZE     2048

typedef long MSG_QUEUE_TYPE;

typedef struct RT_MSG_QUEUE {
    queue_t *               queue;
    int32_t                 id;
} rt_msg_queue_t;

static rt_msg_queue_t g_msg_queue_list[MAX_MSG_QUEUE_CNT] = {0};

int32_t rt_init_msg_queue(void *args)
{
    int32_t ret;

    ret = rt_mutex_init(&g_queue_mutex);

    return ret;
}

int32_t rt_creat_msg_queue(int8_t *pathname, int8_t proj_id)
{
    int32_t msgid = -1;
    int32_t i;
    
    rt_mutex_lock(&g_queue_mutex);

    for (i = 0; i < MAX_MSG_QUEUE_CNT; i++) {
        if (!g_msg_queue_list[i].queue) {
            g_msg_queue_list[i].queue = queue_new();
            if (!g_msg_queue_list[i].queue) {
                goto exit_entry;   
            }
            g_msg_queue_list[i].id = i;
            rt_mutex_init(&g_msg_queue_list[i].queue->mutex);
            msgid = i;
            break;
        }
    }

exit_entry:
    
    rt_mutex_unlock(&g_queue_mutex);

    return msgid;
}

int32_t rt_receive_msg_queue(int32_t msgid, void *buffer, int32_t len, int64_t msgtyp, int32_t msgflg)
{
    queue_t *q = NULL;
    int32_t ret = RT_ERROR;
    uint8_t recv_buffer[MAX_BUFFER_SIZE];
    int32_t recv_len = 0;

    if (0 <= msgid && msgid < MAX_MSG_QUEUE_CNT) {
        q = g_msg_queue_list[msgid].queue;
        ret = receive_queue_data(q, recv_buffer, &recv_len);
        if (!ret) {
            /* check msgtype, see doc in https://blog.csdn.net/gavin_john/article/details/23039157 */
            MSG_QUEUE_TYPE recv_msg_typ = 0;

            rt_os_memcpy(&recv_msg_typ, recv_buffer, sizeof(MSG_QUEUE_TYPE));
            if (recv_msg_typ != (MSG_QUEUE_TYPE)msgtyp) {
                MSG_PRINTF(LOG_WARN, "unexpected msg type %d !\n", recv_msg_typ);   
            } else {
                //MSG_PRINTF(LOG_INFO, "msg queue recv: len %d, recv_len %d\n", len, recv_len); 
                rt_os_memcpy(buffer, recv_buffer, recv_len);
            }
        } else {
            //MSG_PRINTF(LOG_WARN, "msg queue recv fail sizeof(long):%d, sizeof(void *):%d\n", sizeof(MSG_QUEUE_TYPE), sizeof(void *)); 
        }
    } 

    return ret;
}

int32_t rt_send_msg_queue(int32_t msgid, const void *buffer, int32_t len, int32_t msgflg)
{
    queue_t *q = NULL;
    int32_t ret = RT_ERROR;

    if (0 <= msgid && msgid < MAX_MSG_QUEUE_CNT) {
        q = g_msg_queue_list[msgid].queue;
        ret = send_queue_data(q, buffer, len + sizeof(MSG_QUEUE_TYPE)); 
        //MSG_PRINTF(LOG_INFO, "msg queue send: len %d\n", len + sizeof(MSG_QUEUE_TYPE)); 
    } 

    return ret;
}

