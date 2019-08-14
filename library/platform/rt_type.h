
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_type.h
 * Date        : 2017.09.01
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include "log.h"

typedef enum RT_BOOL{
    RT_FALSE = 0,
    RT_TRUE
} rt_bool;

#define RT_ERROR                     -1
#define RT_SUCCESS                   0

#endif // __TYPE_H__
