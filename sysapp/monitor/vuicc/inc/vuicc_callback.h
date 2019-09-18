
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : vuicc_callback.h
 * Date        : 2019.08.19
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text
 *******************************************************************************/

#ifndef __VUICC_CALLBACK_H__
#define __VUICC_CALLBACK_H__

#include <stdio.h>
#include <stdbool.h>
#include "file.h"

typedef struct _file_ops_t {

    rt_fshandle_t (*fopen)(const char *filename, rt_fsmode_t mode);

    int (*fclose)(rt_fshandle_t fp);

    int (*fseek)(rt_fshandle_t fp, long int offset, int whence);

    long int (*ftell)(rt_fshandle_t fp);

    size_t (*fread)(void *ptr, size_t size, size_t count, rt_fshandle_t fp);

    size_t (*fwrite)(const void *ptr, size_t size, size_t count, rt_fshandle_t fp);

    bool (*dir_exist)(const char *dir);

    bool (*file_exist)(const char *file);

    int (*create_dir)(const char *dir);

    int (*delete_dir)(const char *dir);

    int (*delete_file)(const char *file);

} file_ops_t;

#define STRUCT_MEMBER(name, var)   .name = var

#define FILE_OPS_DEFINITION(platform) \
static const file_ops_t g_file_ops_for_##platform = \
{ \
    STRUCT_MEMBER(fopen,        platform##_fopen), \
    STRUCT_MEMBER(fclose,       platform##_fclose), \
    STRUCT_MEMBER(fseek,        platform##_fseek), \
    STRUCT_MEMBER(ftell,        platform##_ftell), \
    STRUCT_MEMBER(fread,        platform##_fread), \
    STRUCT_MEMBER(fwrite,       platform##_fwrite), \
    STRUCT_MEMBER(dir_exist,    platform##_dir_exist), \
    STRUCT_MEMBER(file_exist,   platform##_file_exist), \
    STRUCT_MEMBER(create_dir,   platform##_create_dir), \
    STRUCT_MEMBER(delete_dir,   platform##_delete_dir), \
    STRUCT_MEMBER(delete_file,  platform##_delete_file), \
};

extern int vsim_file_ops_init(const file_ops_t *ops);
extern int init_file_ops(void);

#define _file_ops_init(platform) vsim_file_ops_init(&g_file_ops_for_##platform)

#endif  // __VUICC_CALLBACK_H__
