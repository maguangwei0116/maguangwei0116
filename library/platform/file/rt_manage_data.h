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

extern int32_t rt_write_data(const char *file_name, uint32_t offset, const uint8_t *data_buffer, uint32_t len);
extern int32_t rt_read_data(const char *file_name, uint32_t offset, uint8_t *data_buffer, uint32_t len);
extern int32_t rt_create_file(const char *file_name);
extern int32_t rt_truncate_data(const char *file_name, uint32_t offset);
extern int32_t rt_rm_dir(const char *dir_path);
extern int32_t rt_rm(const char *file_name);

#endif   // __RT_MANAGE_DATA_H__
