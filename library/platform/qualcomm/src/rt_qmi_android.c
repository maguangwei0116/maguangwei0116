
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_qmi_adnroid.c
 * Date        : 2018.09.18
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/
#include <stdarg.h>
#include "rt_type.h"
#include "rt_qmi.h"
#include "rt_os.h"
#include "log.h"

#define WAEK_API __attribute__((weak))

/* These APIs defined in android JNI project */
extern WAEK_API int32_t jni_get_imei(uint8_t *imei);
extern WAEK_API int32_t jni_get_mcc(uint16_t *mcc);
extern WAEK_API int32_t jni_get_mnc(uint16_t *mnc);
extern WAEK_API int32_t jni_get_current_iccid(uint8_t *iccid);
extern WAEK_API int32_t jni_get_current_imsi(uint8_t *imsi);
extern WAEK_API int32_t jni_get_signal_level(int32_t *level);
extern WAEK_API int32_t jni_get_signal_dbm(int32_t *dbm);
extern WAEK_API int32_t jni_get_network_type(uint8_t *type);
extern WAEK_API int32_t jni_set_apn(uint8_t *apn, uint8_t *mcc_mnc);
extern WAEK_API int32_t jni_get_model(uint8_t *model);
extern WAEK_API int32_t jni_get_monitor_version(uint8_t *monitor_version);

extern WAEK_API int32_t jni_euicc_open_channel(uint8_t *channel);
extern WAEK_API int32_t jni_euicc_close_channel(uint8_t *channel);
extern WAEK_API int32_t jni_euicc_transmit_apdu(const uint8_t *data, const uint32_t *data_len, \
                                    uint8_t *rsp, uint32_t *rsp_len, const uint32_t *channel);

extern WAEK_API int32_t jni_vuicc_transmit_apdu(const uint8_t *data, const uint32_t *data_len, 
                                    uint8_t *rsp, uint32_t *rsp_len);

#define MAX_PARAM_CNT           6

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#endif

typedef struct THREAD_PARAM {
    void *                  param1;
    void *                  param2;
    void *                  param3;
    void *                  param4;
    void *                  param5;
    void *                  param6;
} thread_param_t;

typedef struct THREAD_DATA {
    const char *            name;
    rt_pthread_mutex_t *    mutex;
    rt_sem_t *              send_sem;
    rt_sem_t *              recv_sem;
    thread_param_t          param;
    int32_t *               ret;
    rt_task                 task_id;
} thread_data_t;

static thread_data_t g_thread_data;

static int32_t jni_api_common(const char *func, int32_t cnt, ...)
{
    va_list vl_list;
    int32_t i = 0;
    int32_t ret = RT_ERROR;
    void *tmp = NULL;    
    void *param_list[MAX_PARAM_CNT] = {0};

    MSG_PRINTF(LOG_INFO, "%s start ...\r\n", func);

    linux_mutex_lock(g_thread_data.mutex);

    g_thread_data.name = func;
    g_thread_data.ret = &ret;

    if (cnt > MAX_PARAM_CNT) {
        MSG_PRINTF(LOG_WARN, "param cnt too many, cnt=%d\r\n", cnt);
        goto exit_entry;  
    }

    va_start(vl_list, cnt);
    for(i = 0; i < cnt; i++) {
        tmp = va_arg(vl_list, void *);
        //MSG_PRINTF(LOG_INFO, "param %d: %p\r\n", i+1, tmp);
        param_list[i] = tmp;
    }
    va_end(vl_list);

    rt_os_memcpy((uint8_t *)&g_thread_data.param, (uint8_t *)&param_list, sizeof(thread_param_t));
    //MSG_INFO_ARRAY("thread-param: ", &g_thread_data.param, sizeof(thread_param_t));

    linux_sem_post(g_thread_data.send_sem);
    //MSG_PRINTF(LOG_INFO, "%s post send sem ...\r\n", __func__);
    linux_sem_wait(g_thread_data.recv_sem);
    //MSG_PRINTF(LOG_INFO, "%s wait recv sem ...\r\n", __func__);

exit_entry:

    linux_mutex_unlock(g_thread_data.mutex);

    return ret;
}

/*
The following 3 group APIs for euicc
*/
int32_t rt_qmi_send_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len, uint8_t channel)
{
    uint32_t data_len_in = data_len;
    uint32_t rsp_len_in = *rsp_len;
    uint32_t channel_in = channel;
    int32_t ret;

    ret = jni_api_common(__func__, 5, data, &data_len_in, rsp, &rsp_len_in, &channel_in);

    *rsp_len = rsp_len_in;

    return ret;
}

static int32_t rt_qmi_send_apdu_handle(void)
{
    const uint8_t *data = (const uint8_t *)g_thread_data.param.param1;
    const uint32_t *data_len = (const uint32_t *)g_thread_data.param.param2;
    uint8_t *rsp = (uint8_t *)g_thread_data.param.param3;
    uint32_t *rsp_len = (uint32_t *)g_thread_data.param.param4;
    const uint32_t *channel = (const uint32_t *)g_thread_data.param.param5;
    int32_t ret;

    ret = jni_euicc_transmit_apdu(data, data_len, rsp, rsp_len, channel);

    return ret;
}

int32_t rt_qmi_close_channel(uint8_t channel)
{
    uint32_t channel_in = channel;
    int32_t ret;

    ret = jni_api_common(__func__, 1, &channel_in);

    return ret;
}

static int32_t rt_qmi_close_channel_handle(void)
{
    const uint32_t *channel = (const uint32_t *)g_thread_data.param.param1;
    int32_t ret;
    
    ret = jni_euicc_close_channel(channel);

    return ret;
}

int32_t rt_qmi_open_channel(const uint8_t *aid, uint16_t aid_len, uint8_t *channel)
{
    uint32_t channel_in = channel;
    int32_t ret;

    ret = jni_api_common(__func__, 1, &channel_in);

    *channel = channel_in;

    return ret;
}

static int32_t rt_qmi_open_channel_handle(void)
{
    const uint32_t *channel = (const uint32_t *)g_thread_data.param.param1;
    int32_t ret;
    
    ret = jni_euicc_open_channel(channel);

    return ret;
}

/*
apdu API for vuicc
*/
int32_t rt_qmi_exchange_apdu(const uint8_t *data, uint16_t data_len, uint8_t *rsp, uint16_t *rsp_len)
{
    uint32_t data_len_in = data_len;
    uint32_t rsp_len_in = *rsp_len;
    int32_t ret;

    ret = jni_api_common(__func__, 4, data, &data_len_in, rsp, &rsp_len_in);

    *rsp_len = rsp_len_in;

    return ret;
}

static int32_t rt_qmi_exchange_apdu_handle(void)
{
    const uint8_t *data = (const uint8_t *)g_thread_data.param.param1;
    const uint32_t *data_len = (const uint32_t *)g_thread_data.param.param2;
    uint8_t *rsp = (uint8_t *)g_thread_data.param.param3;
    uint32_t *rsp_len = (uint32_t *)g_thread_data.param.param4;
    int32_t ret;

    ret = jni_vuicc_transmit_apdu(data, data_len, rsp, rsp_len);

    return ret;
}

int32_t rt_qmi_get_register_state(int32_t *register_state)
{
    int32_t ret;

    ret = jni_api_common(__func__, 1, register_state);

    return ret;
}

static int32_t rt_qmi_get_register_state_handle(void)
{
    int32_t *register_state = (int32_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    (void)register_state;

    return ret;
}

int32_t rt_qmi_get_mcc_mnc(uint16_t *mcc, uint16_t *mnc)
{
    return jni_api_common(__func__, 2, mcc, mnc);
}

static int32_t rt_qmi_get_mcc_mnc_handle(void)
{
    uint16_t *mcc = (uint16_t *)g_thread_data.param.param1;
    uint16_t *mnc = (uint16_t *)g_thread_data.param.param2;
    int32_t ret = RT_SUCCESS;

    if(mcc != NULL){
        ret = jni_get_mcc(mcc);
    }
    if(mnc != NULL){
        ret = jni_get_mnc(mnc);
    }

    return ret;
}

int32_t rt_qmi_get_current_iccid(uint8_t *iccid)
{
    return jni_api_common(__func__, 1, iccid);
}

static int32_t rt_qmi_get_current_iccid_handle(void)
{
    uint8_t *iccid = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_current_iccid(iccid);

    return ret;
}

int32_t rt_qmi_get_current_imsi(uint8_t *imsi)
{
    return jni_api_common(__func__, 1, imsi);
}

static int32_t rt_qmi_get_current_imsi_handle(void)
{
    uint8_t *imsi = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_current_imsi(imsi);

    return ret;
}

int32_t rt_qmi_get_signal(int32_t *strength)
{
    return jni_api_common(__func__, 1, strength);
}

static int32_t rt_qmi_get_signal_handle(void)
{
    int32_t *strength = (int32_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_signal_dbm(strength);

    return ret;
}

int32_t rt_qmi_get_signal_level(int32_t *level)
{
    return jni_api_common(__func__, 1, level);
}

static int32_t rt_qmi_get_signal_level_handle(void)
{
    int32_t *level = (int32_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_signal_level(level);

    return ret;
}

int32_t rt_qmi_get_imei(uint8_t *imei)
{
    return jni_api_common(__func__, 1, imei);
}

static int32_t rt_qmi_get_imei_handle(void)
{
    uint8_t *imei = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_imei(imei);

    return ret;
}

int32_t rt_qmi_modify_profile(int8_t index, int8_t profile_type, int8_t pdp_type, int8_t *apn, int8_t *mcc_mnc)
{
    (void)index;
    (void)profile_type;
    (void)pdp_type;

    return jni_api_common(__func__, 2, apn, mcc_mnc);
}

static int32_t rt_qmi_modify_profile_handle(void)
{
    int8_t *apn = (int8_t *)g_thread_data.param.param1;
    int8_t *mcc_mnc = (int8_t *)g_thread_data.param.param2;
    int32_t ret = RT_SUCCESS;

    ret = jni_set_apn(apn, mcc_mnc);

    return ret;
}

int32_t rt_qmi_get_model(uint8_t *model)
{
    return jni_api_common(__func__, 1, model);   
}

static int32_t rt_qmi_get_model_handle(void)
{
    uint8_t *model = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_model(model);
    
    return ret;
}

int32_t rt_qmi_get_network_type(uint8_t *network_type)
{
    return jni_api_common(__func__, 1, network_type);
}

static int32_t rt_qmi_get_network_type_handle(void)
{
    uint8_t *network_type = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_network_type(network_type);

    return ret;
}

int32_t rt_qmi_get_monitor_version(uint8_t *version)
{
    return jni_api_common(__func__, 1, version);   
}

static int32_t rt_qmi_get_monitor_version_handle(void)
{
    uint8_t *version = (uint8_t *)g_thread_data.param.param1;
    int32_t ret = RT_SUCCESS;

    ret = jni_get_monitor_version(version);
    
    return ret;
}

typedef int32_t (*jni_handle_func)(void);

typedef struct JNI_API {
    const char *        name;
    jni_handle_func     handle;
} jni_api_t;

#define JNI_API_DEF(api)    {#api, (jni_handle_func)api##_handle}

static const jni_api_t g_jni_apis[] = 
{
    JNI_API_DEF(rt_qmi_send_apdu),
    JNI_API_DEF(rt_qmi_close_channel),
    JNI_API_DEF(rt_qmi_open_channel),
    JNI_API_DEF(rt_qmi_exchange_apdu),
    JNI_API_DEF(rt_qmi_get_register_state),
    JNI_API_DEF(rt_qmi_get_mcc_mnc),
    JNI_API_DEF(rt_qmi_get_current_iccid),
    JNI_API_DEF(rt_qmi_get_current_imsi),
    JNI_API_DEF(rt_qmi_get_signal),
    JNI_API_DEF(rt_qmi_get_signal_level),
    JNI_API_DEF(rt_qmi_get_imei),
    JNI_API_DEF(rt_qmi_modify_profile),
    JNI_API_DEF(rt_qmi_get_model),
    JNI_API_DEF(rt_qmi_get_network_type),
    JNI_API_DEF(rt_qmi_get_monitor_version),
};

static int32_t jni_apis_handle(void)
{
    int32_t i;
    int32_t ret = RT_ERROR;

    for (i = 0; i < ARRAY_SIZE(g_jni_apis); i++) {
        if (!rt_os_strcmp(g_thread_data.name, g_jni_apis[i].name)) {
            ret = g_jni_apis[i].handle();
            *g_thread_data.ret = ret;
            return ret;
        }
    }

    MSG_PRINTF(LOG_ERR, "%s not found !\r\n", g_thread_data.name);

    return RT_ERROR;
}

static void android_jni_thread(void)
{   
    rt_setcancelstate_task(RT_PTHREAD_CANCEL_ENABLE, NULL);
    
    while (1) {
        linux_sem_wait(g_thread_data.send_sem);
        //MSG_PRINTF(LOG_INFO, "%s wait send sem ...\r\n", __func__);

        jni_apis_handle();

        linux_sem_post(g_thread_data.recv_sem);
        //MSG_PRINTF(LOG_INFO, "%s post recv sem ...\r\n", __func__);
    }

    linux_mutex_release(g_thread_data.mutex);
    linux_sem_destroy(g_thread_data.send_sem);
    linux_sem_destroy(g_thread_data.recv_sem);
    
    rt_exit_task(NULL);
}

int32_t rt_qmi_init(void *arg)
{
    int32_t ret = RT_ERROR;
    rt_task id_connect;
    static int32_t call_index = 1;

    if (call_index != 1) {
        rt_cancel_task(g_thread_data.task_id);      //send cancel signal to sub-thread
        rt_join_task(g_thread_data.task_id, NULL);  //wait sub-thread exit
        g_thread_data.task_id = 0;
        linux_mutex_release(g_thread_data.mutex);
        linux_sem_destroy(g_thread_data.send_sem);
        linux_sem_destroy(g_thread_data.recv_sem);
    } else {  // frist call
        call_index = 2;       
    }

    g_thread_data.mutex = linux_mutex_init();
    g_thread_data.send_sem = linux_sem_init(0, 0);
    g_thread_data.recv_sem = linux_sem_init(0, 0);

    ret = rt_create_task(&id_connect, (void *)android_jni_thread, NULL);
    if (ret == RT_ERROR) {
        MSG_PRINTF(LOG_ERR, "create jni pthread error, err(%d)=%s\r\n", errno, strerror(errno));
    } else {
        g_thread_data.task_id = id_connect;
    }
    
    return ret;
}

int32_t rt_jni_init(void *arg)
{
    return rt_qmi_init(arg);   
}

