/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_manage_data.h
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/
#ifndef __RT_MANAGE_DATA_H__
#define __RT_MANAGE_DATA_H__

#include <stdio.h>
#include "rt_type.h"

extern int32_t rt_write_data(uint8_t *addr,uint32_t offset,const uint8_t *data_buffer,uint32_t len);
extern int32_t rt_read_data(uint8_t *addr,uint32_t offset,uint8_t *data_buffer,uint32_t len);
extern int32_t rt_create_file(uint8_t *file_name);
extern int32_t rt_truncate_data(uint8_t *filename,int32_t offset);
extern int32_t rm_dir(const int8_t *dirpath);
extern int32_t rm(const int8_t *file_name);

#endif   // __RT_MANAGE_DATA_H__
