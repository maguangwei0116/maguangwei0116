
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : agent_main.h
 * Date        : 2019.08.15
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __AGENT_MAIN_H__
#define __AGENT_MAIN_H__

#include <stdio.h>
#include <stdint.h>

typedef void (*log_func)(const char *msg);

extern int32_t agent_main(const char *app_path, log_func logger);

#endif // __AGENT_MAIN_H__
