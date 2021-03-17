
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : ipc_task.c
 * Date        : 2021.03.13
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_os.h"
#include "ipc_task.h"
#include "agent_command.h"
#include "ipc_socket_server.h"

#ifdef CFG_OPEN_MODULE
#define AGENT_SERVER_PATH                             "./data/redtea/agent_server"

static void ipc_server_task(void *arg)
{
    ipc_socket_server(AGENT_SERVER_PATH);
}

int32_t init_ipc_task(void *arg)
{
    rt_task task_id = 0;
    int32_t ret = RT_ERROR;

    init_agent_cmd(arg);

    ipc_regist_callback(agent_cmd);

    ret = rt_create_task(&task_id, (void *)ipc_server_task, NULL);
    if (ret != RT_SUCCESS) {
        MSG_PRINTF(LOG_ERR, "create IPC server task failed\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

#endif // CFG_OPEN_MODULE

