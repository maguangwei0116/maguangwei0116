
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : file.c
 * Date        : 2019.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "file.h"

rt_dir_t linux_opendir(const char *name)
{
    return opendir(name);
}

int linux_closedir(rt_dir_t dir)
{
    return closedir(dir);
}

rt_dirent_t linux_readdir(rt_dir_t dir)
{
    return readdir(dir);
}

rt_fshandle_t linux_fopen(const char *filename, rt_fsmode_t mode)
{
    rt_fshandle_t fp = NULL;

    fp = fopen(filename, mode);
    if(fp != NULL) {
        setbuf(fp, NULL);
    }

    return fp;
}

int linux_fclose(rt_fshandle_t fp)
{
    return fclose(fp);
}

int linux_fseek(rt_fshandle_t fp, long int offset, int whence)
{
    return fseek(fp, offset, whence);
}

int linux_feof(rt_fshandle_t fp)
{
    return feof(fp);
}

char *linux_fgets(char *s, int size, rt_fshandle_t fp)
{
    return fgets(s, size, fp);
}

long int linux_ftell(rt_fshandle_t fp)
{
    return ftell(fp);
}

long int linux_fflush(rt_fshandle_t fp)
{
    return fflush(fp);
}

size_t linux_fread(void *ptr, size_t size, size_t count, rt_fshandle_t fp)
{
    return fread(ptr, size, count, fp);
}

size_t linux_fwrite(const void *ptr, size_t size, size_t count, rt_fshandle_t fp)
{
    return fwrite(ptr, size, count, fp);
}

bool linux_dir_exist(const char *dir)
{
    if(access(dir, RT_FS_F_OK) == 0)
        return true;

    return false;
}

bool linux_file_exist(const char *file)
{
    if(access(file, RT_FS_F_OK) == 0)
        return true;

    return false;
}

int linux_create_dir(const char *dir)
{
    return mkdir(dir, RT_S_IRWXU | RT_S_IROTH | RT_S_IXOTH);
}

int linux_delete_dir(const char *dir)
{
    char tmp[64];

    snprintf(tmp, sizeof(tmp), "rm -rf %s", dir);

    return system(tmp);
}

int linux_delete_file(const char *file)
{
    return remove(file);
}

int linux_truncate(const char *filename, off_t offset)
{
    if (filename == NULL) {
        return RT_ERROR;
    }
    return truncate(filename, offset);
}

int linux_file_size(const char *file)
{
    struct stat statbuf;
    int32_t size;

    if (rt_os_access(file, RT_FS_F_OK)) { /* file isn't exist */
        return 0;
    }

    stat(file, &statbuf);
    size = statbuf.st_size;

    return size;
}

int linux_stat(const char *pathname, rt_stat_t *buf)
{
    return stat(pathname, buf);
}

int linux_fstat(int fd, rt_stat_t *buf)
{
    return fstat(fd, buf);
}

int linux_lstat(const char *pathname, rt_stat_t *buf)
{
    return lstat(pathname, buf);
}

size_t shell_cmd(const int8_t *cmd, uint8_t *buf, size_t size)
{
    rt_fshandle_t fp;
    size_t num = 0;

    fp = popen(cmd, RT_FS_READ);
    if (fp == NULL) {
        MSG_PRINTF(LOG_ERR, "Popen failed!!\n");
        return num;
    }
    num = linux_fread(buf, 1, size, fp);
    pclose(fp);

    return num;
}

