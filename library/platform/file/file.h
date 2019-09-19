
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : file.h
 * Date        : 2019.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#ifndef __FILE_H__
#define __FILE_H__

#include <stdio.h>
#include <stdbool.h>
#include "rt_type.h"

typedef FILE *                      rt_fshandle_t;
typedef const char *                rt_fsmode_t;

#define RT_FS_CREATE                "w+"
#define RT_FS_READ                  "r"
#define RT_FS_WRITE                 "w"
#define RT_FS_READ_WRITE            "r+"

#define RT_FS_SEEK_SET              SEEK_SET
#define RT_FS_SEEK_CUR              SEEK_CUR
#define RT_FS_SEEK_END              SEEK_END

rt_fshandle_t rt_fopen(const char *filename, rt_fsmode_t mode);
int rt_fclose(rt_fshandle_t fp);
int rt_fseek(rt_fshandle_t fp, long int offset, int whence);
long int rt_ftell(rt_fshandle_t fp);
long int rt_fflush(rt_fshandle_t fp);
size_t rt_fread(void *ptr, size_t size, size_t count, rt_fshandle_t fp);
size_t rt_fwrite(const void *ptr, size_t size, size_t count, rt_fshandle_t fp);
bool rt_dir_exist(const char *dir);
bool rt_file_exist(const char *file);
int rt_create_dir(const char *dir);
int rt_delete_dir(const char *dir);
int rt_delete_file(const char *file);

#endif // __FILE_H__
