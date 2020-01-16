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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "rt_os.h"
#include "agent_main.h"
#include "config.h"

#if (CFG_OPEN_MODULE)
#define RT_DATA_PATH            "/data/redtea/"
#elif (CFG_STANDARD_MODULE)  // standard
#define RT_DATA_PATH            "/usrdata/redtea/"
#endif

#define RT_DEBUG_IN_TERMINAL    "terminal"
#define RT_MALLOC_CHECK         "MALLOC_CHECK_="
#define RT_ERR_LOG              "rt_err_log"
#define RT_STDERR_FD            2

typedef void (*debug_func)(const char *msg);

static volatile int32_t toStop = 0;

static void cfinish(int32_t sig)
{
    MSG_PRINTF(LOG_DBG, "recv signal %d, process exit !\r\n", sig);
    rt_os_signal(RT_SIGINT, NULL);
    toStop = 1;
}

static void SIGPIPE_handler(int32_t signo)
{
    rt_os_signal(signo, SIGPIPE_handler);
    
    switch(signo) {
        case RT_SIGPIPE:
            MSG_PRINTF(LOG_DBG, "%d, SIGPIPE recv !\n", signo);
            break;
            
        default:
            MSG_PRINTF(LOG_DBG, "%d signal unregister\n", signo);
            break;
    }
}

static int32_t init_system_signal(void *arg)
{
    rt_os_signal(RT_SIGINT, cfinish);    
    rt_os_signal(RT_SIGPIPE, SIGPIPE_handler);
    
    return RT_SUCCESS;
}

static void debug_with_printf(const char *msg)
{
    printf("%s", msg);
}

static char *get_restart_reason(int32_t argc, char **argv)
{
    char *reason = "";
    
    if (argc > 1) {
        reason = argv[1];
    }

    return reason;
}

static debug_func get_terminal_debug_func(int32_t argc, char **argv)
{
    debug_func func = NULL;
    
    if (argc > 1) {
        if (rt_os_strstr(argv[1], RT_DEBUG_IN_TERMINAL)) { // debug in terminal
            printf("main input: %s, argc=%d\r\n", argv[1], argc);
            func = debug_with_printf;   
        }
    }

    return func;
}

static rt_bool check_reopen_stderr(int32_t argc, char **argv)
{
    rt_bool reopen = RT_FALSE;

    if (argc > 2) {
        if (rt_os_strstr(argv[2], RT_MALLOC_CHECK)) { // enable malloc check
            char rt_err_log[128];
            int32_t fd;
            char timestamp[128];
            char content[1024] = {0};
            time_t  time_write;
            struct tm tm_Log;

            snprintf(rt_err_log, sizeof(rt_err_log), "%s/%s", RT_DATA_PATH, RT_ERR_LOG);
            fd = open(rt_err_log, O_RDWR | O_CREAT | O_APPEND, 0666);
            dup2(fd, RT_STDERR_FD);  // stderr >> rt_err_log
            time_write = time(NULL);
            localtime_r(&time_write, &tm_Log);
            strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S]", &tm_Log);
            snprintf(content, sizeof(content), "malloc check start @ %s\r\n", timestamp);
            write(fd, content, rt_os_strlen(content));

            reopen = RT_TRUE;
        }
    }

    return reopen;
}

/*
agent process start input param list :
1) ./rt_agent "restart reason # terminal" "MALLOC_CHECK_=1"
*/
int32_t main(int32_t argc, char **argv)
{
    debug_func func = NULL;
    const char *restart_reason = "";
    
    restart_reason = get_restart_reason(argc, argv);
    
    func = get_terminal_debug_func(argc, argv);

    check_reopen_stderr(argc, argv);

    config_set_restart_reason(restart_reason);
    
    agent_main(RT_DATA_PATH, func);

    init_system_signal(NULL);

    while (!toStop) {
        rt_os_sleep(3);
    }

    return RT_SUCCESS;
}
 
