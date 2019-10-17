
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : inspect_file.h
 * Date        : 2019.10.16
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __INSPECT_FILE_H__
#define __INSPECT_FILE_H__

#include "rt_type.h"

typedef enum FILE_TYPE {
    FILE_TYPE_MONITOR = 0,
    FILE_TYPE_SHARE_PROFILE,
    FILE_TYPE_AGENT
} file_type_e;

rt_bool monitor_inspect_file(file_type_e type);

#endif // __INSPECT_FILE_H__
