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

#include "rt_os.h"
#include "agent_main.h"
#include "config.h"

#if (CFG_OPEN_MODULE)
#define RT_DATA_PATH            "/data/redtea/"
#elif (CFG_STANDARD_MODULE)  // standard
#define RT_DATA_PATH            "/usrdata/redtea/"
#endif

typedef void (*debug_func)(const char *msg);

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

static void debug_with_printf(const char *msg)
{
    printf("%s", msg);
}

int32_t main(int32_t argc, char **argv)
{
    debug_func func = NULL;
    const char *restart_reason = "";
    
    if (argc > 2) {  // debug in terminal
        printf("main input: %s\r\n", argv[1]);
        restart_reason = argv[2];
        func = debug_with_printf;
    } else if (argc > 1) {
        restart_reason = argv[1];
    }

    config_set_restart_reason(restart_reason);
    
    agent_main(RT_DATA_PATH, func);

    init_system_signal(NULL);

    while (!toStop) {
        rt_os_sleep(3);
    }

    return RT_SUCCESS;
}
 
