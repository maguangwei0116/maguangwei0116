
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

rt_bool inspect_monitor_file(void);
rt_bool inspect_agent_file(void);
rt_bool inspect_abstract_content(uint8_t *hash, uint8_t *signature);

#endif // __INSPECT_FILE_H__
