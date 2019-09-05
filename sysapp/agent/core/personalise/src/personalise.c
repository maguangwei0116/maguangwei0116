
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : personalise.c
 * Date        : 2019.08.21
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifdef CFG_AGENT_PERSONALISE_ON
#warning AGENT_PERSONALISE_ON on ...

#include "rt_type.h"
#include "personalise.h"
#include "device_info.h"

int32_t init_personalise(void *arg)
{
    init_device_info(arg);
    return RT_SUCCESS;
}

#endif
