
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

int32_t main(void)
{
    rt_fshandle_t fp;
    fp = rt_fopen("/data/redtea/rt_log", 0);
    printf("Hello world\r\n");
    return 0;
}
