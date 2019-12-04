
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
#include <dirent.h>
#include <sys/stat.h>

#include "rt_type.h"

typedef FILE *                      rt_fshandle_t;
typedef const char *                rt_fsmode_t;
typedef DIR *                       rt_dir_t;
typedef struct dirent *             rt_dirent_t;
typedef struct stat                 rt_stat_t;

#define RT_FS_CREATE                "w+"
#define RT_FS_READ                  "r"
#define RT_FS_WRITE                 "w"
#define RT_FS_READ_WRITE            "r+"

#define RT_FS_SEEK_SET              SEEK_SET
#define RT_FS_SEEK_CUR              SEEK_CUR
#define RT_FS_SEEK_END              SEEK_END

#ifndef RT_FS_F_OK
#define RT_FS_F_OK                  0
#endif

rt_dir_t linux_opendir(const char *dir_name);
int linux_closedir(rt_dir_t dir);
rt_dirent_t linux_readdir(rt_dir_t dir);
rt_fshandle_t linux_fopen(const char *file_name, rt_fsmode_t mode);
int linux_fclose(rt_fshandle_t fp);
int linux_fseek(rt_fshandle_t fp, long int offset, int whence);
int linux_feof(rt_fshandle_t fp);
char *linux_fgets(char *s, int size, rt_fshandle_t fp);
long int linux_ftell(rt_fshandle_t fp);
long int linux_fflush(rt_fshandle_t fp);
size_t linux_fread(void *ptr, size_t size, size_t count, rt_fshandle_t fp);
size_t linux_fwrite(const void *ptr, size_t size, size_t count, rt_fshandle_t fp);
rt_bool linux_dir_exist(const char *dir_name);
rt_bool linux_file_exist(const char *file_name);
int linux_create_dir(const char *dir_name);
int linux_delete_dir(const char *dir_name);
int linux_delete_file(const char *file_name);
int linux_truncate(const char *file_name, off_t offset);
int linux_file_size(const char *file_name);
int linux_stat(const char *path_name, rt_stat_t *buf);
int linux_fstat(int fd, rt_stat_t *buf);
int linux_lstat(const char *path_name, rt_stat_t *buf);
int shell_cmd(const int8_t *cmd, uint8_t *buf, int size);

/* API for redtea path file */
int init_rt_file_path(void *arg);
rt_fshandle_t linux_rt_fopen(const char *file_name, rt_fsmode_t mode);
int linux_rt_file_size(const char *file_name);
int linux_rt_file_copy(const char *src, const char *dst);
int linux_rt_truncate(const char *file_name, off_t offset);
int linux_rt_mkdir(const char *path_name);
rt_dir_t linux_rt_opendir(const char *path_name);
rt_bool linux_rt_file_exist(const char *file_name);
int linux_rt_delete_file(const char *file_name);
int linux_rt_rename_file(const char *relative_src_file_path, const char *absolute_dst_file_path);
int linux_rt_file_abs_path(const char *file_name, char *file_abs_path, int len);

#endif // __FILE_H__

