
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : base64.h
 * Date        : 2018.08.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __BASE64_H__
#define __BASE64_H__

#include <rt_type.h>

#define RT_ERR_BASE64_BAD_MSG                   -2002

int rt_base64_encode(const uint8_t *in, uint16_t in_len, char *out);
int rt_base64_decode(const char *in, uint8_t *out, uint16_t *out_len);

#endif  //__BASE64_H__
