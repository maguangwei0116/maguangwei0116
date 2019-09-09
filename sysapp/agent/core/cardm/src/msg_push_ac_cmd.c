
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : msg_process.h
 * Date        : 2019.09.04
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#include "rt_type.h"

static int32_t push_ac_parser(const void *in, char *tranId, void **out)
{
    int32_t ret;


    return ret;
}

static int32_t push_ac_handler(const void *in, void **out)
{
    int32_t ret = 0;

    MSG_PRINTF(LOG_WARN, "\n");

    return ret;
}

DOWNSTREAM_METHOD_OBJ_INIT(PUSH_AC, MSG_ID_CARD_MANAGER, ON_PUSH_AC, push_ac_parser, push_ac_handler);
