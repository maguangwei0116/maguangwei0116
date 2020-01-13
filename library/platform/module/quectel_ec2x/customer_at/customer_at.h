
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : customer_at.h
 * Date        : 2019.12.09
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __CUSTOEMR_AT_H__
#define __CUSTOEMR_AT_H__

#include "rt_type.h"

typedef int32_t (*atcommand_callback)(const char *cmd, char *rsp, int32_t len);

typedef struct AT_CMD {
    char                    label[64];      // customer AT cmd name
    atcommand_callback      handle;         // customer AT cmd handle function
} at_cmd_t;

int32_t init_customer_at(void *arg);

#endif // __CUSTOEMR_AT_H__
