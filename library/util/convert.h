/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : convert.h
 * Date        : 2017.09.01
 * Note        :
 * Description : Some common conversion function
 *******************************************************************************/
#ifndef __CONVERT_H_
#define __CONVERT_H_

#include "rt_type.h"

extern void bytestring_to_charstring(int8_t *bytestring,int8_t *charstring,int16_t length);
extern rt_bool strncpy_case_insensitive(int8_t *src,int8_t *obj,int16_t len);
extern int8_t to_ascii(int8_t ch);

#endif
