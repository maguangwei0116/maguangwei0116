
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.c
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "bootstrap.h"
#include "file.h"

#ifdef CFG_AGENT_BOOTSRRAP_ON
#warning AGENT_BOOTSRRAP_ON on ...
#endif

int32_t bootstrap_main(void)
{
    rt_fshandle_t fp;
    fp = rt_fopen("/data/redtea/rt_log", 0);
    printf("Hello world\r\n");
    return 0;
}

void boot_strap_event(const uint8_t *buf, int32_t len, int32_t mode)
{
    MSG_PRINTF(LOG_INFO, "Help us choose the card\n");
}
