
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

volatile int32_t toStop = 0;

void cfinish(int32_t sig)
{
    rt_os_signal(RT_SIGINT, NULL);
    toStop = 1;
}

int32_t main(int32_t argc, int8_t **argv)
{
    rt_os_signal(RT_SIGINT, cfinish);
    rt_os_signal(RT_SIGINT, cfinish);
    rt_qmi_init(NULL);
    init_queue(NULL);
    init_card_manager(NULL);
    init_network_detection(NULL);
    while (!toStop) {
        sleep(3);
    }
}