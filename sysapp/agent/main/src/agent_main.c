
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_main.c
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "agent_queue.h"
#include "card_manager.h"
#include "ipc_socket_client.h"
#include "network_detection.h"
#include "rt_mqtt.h"
#include "config.h"
#include "bootstrap.h"

#include "rt_qmi.h"

#define INIT_OBJ(func, arg)     {#func, func, arg}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)           (sizeof((a)) / sizeof((a)[0]))
#endif

typedef int32_t (*init_func)(void *arg);

typedef struct _init_obj_t {
    const char *    name;
    init_func       init;
    void *          arg;
} init_obj_t ;

static volatile int32_t toStop = 0;

static void cfinish(int32_t sig)
{
    MSG_PRINTF(LOG_DBG, "recv signal %d, process exit !\r\n", sig);
    rt_os_signal(RT_SIGINT, NULL);
    toStop = 1;
}

static int32_t init_system_signal(void *arg)
{
    rt_os_signal(RT_SIGINT, cfinish); 
    return RT_SUCCESS;
}

/*
List your init call here !
*/
static const init_obj_t g_init_objs[] = 
{
    INIT_OBJ(init_system_signal,        NULL), 
    INIT_OBJ(rt_config_init,            NULL),
    INIT_OBJ(rt_qmi_init,               NULL),  
    INIT_OBJ(init_queue,                NULL), 
    INIT_OBJ(init_card_manager,         NULL), 
    INIT_OBJ(init_network_detection,    NULL), 
//    INIT_OBJ(init_mqtt,                 NULL),
};

static int32_t agent_init_call(void)
{
    int32_t i;
    int32_t ret;

    for (i = 0; i < ARRAY_SIZE(g_init_objs); i++) {
        ret = g_init_objs[i].init(g_init_objs[i].arg);
        MSG_PRINTF(LOG_DBG, "%-30s[%s]\r\n", g_init_objs[i].name, !ret ? " OK " : "FAIL");
    }

    return RT_SUCCESS;
}

int32_t main(int32_t argc, int8_t **argv)
{
    init_timer();
    agent_init_call();

    while (!toStop) {
        rt_os_sleep(3);
    }

    return 0;
}