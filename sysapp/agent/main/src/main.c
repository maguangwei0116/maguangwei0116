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

#if (CFG_OPEN_MODULE)
#define RT_DATA_PATH            "/data/redtea/"
#elif (CFG_STANDARD_MODULE)  // standard
#define RT_DATA_PATH            "/usrdata/redtea/"
#endif

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
    if (argc > 1) {  // debug in terminal
        printf("main input: %s\r\n", argv[1]);
        agent_main(RT_DATA_PATH, debug_with_printf);
    } else {
        agent_main(RT_DATA_PATH, NULL);
    }

    init_system_signal(NULL);

    while (!toStop) {
        rt_os_sleep(3);
    }

    return RT_SUCCESS;
}
 
