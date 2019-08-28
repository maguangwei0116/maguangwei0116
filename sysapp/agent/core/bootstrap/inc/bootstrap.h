
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : bootstrap.h
 * Date        : 2019.08.07
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __BOOTSTRAP_H__
#define __BOOTSTRAP_H__

#include "rt_type.h"

int32_t init_bootstrap(int32_t *arg);

void bootstrap_event(const uint8_t *buf, int32_t len, int32_t mode);

#endif // __BOOTSTRAP_H__
