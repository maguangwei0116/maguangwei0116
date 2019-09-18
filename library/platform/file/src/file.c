
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

rt_fshandle_t rt_fopen(const char *filename, rt_fsmode_t mode)
{
    rt_fshandle_t fp = NULL;
    MSG_PRINTF(LOG_INFO, "filename:%s\n", filename);
    fp = fopen(filename, mode);
    if(fp != NULL) {
        setbuf(fp, NULL);
    }

    return fp;
}

int rt_fclose(rt_fshandle_t fp)
{
    return fclose(fp);
}

int rt_fseek(rt_fshandle_t fp, long int offset, int whence)
{
    return fseek(fp, offset, whence);
}

long int rt_ftell(rt_fshandle_t fp)
{
    return ftell(fp);
}

size_t rt_fread(void *ptr, size_t size, size_t count, rt_fshandle_t fp)
{
    return fread(ptr, size, count, fp);
}

size_t rt_fwrite(const void *ptr, size_t size, size_t count, rt_fshandle_t fp)
{
    return fwrite(ptr, size, count, fp);
}

bool rt_dir_exist(const char *dir)
{
    if(access(dir, F_OK) == 0)
        return true;

    return false;
}

bool rt_file_exist(const char *file)
{
    if(access(file, F_OK) == 0)
        return true;

    return false;
}

int rt_create_dir(const char *dir)
{
    return mkdir(dir, S_IRWXU | S_IROTH | S_IXOTH);
}

int rt_delete_dir(const char *dir)
{
    char tmp[64] = "rm -rf ";

    strcat(tmp, dir);

    return system(tmp);
}

int rt_delete_file(const char *file)
{
    return remove(file);
}

rt_fshandle_t linux_fopen(const char *filename, rt_fsmode_t mode)
{
    rt_fshandle_t fp = NULL;
    MSG_PRINTF(LOG_INFO, "filename:%s\n", filename);
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

long int linux_ftell(rt_fshandle_t fp)
{
    return ftell(fp);
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
    if(access(dir, F_OK) == 0)
        return true;

    return false;
}

bool linux_file_exist(const char *file)
{
    if(access(file, F_OK) == 0)
        return true;

    return false;
}

int linux_create_dir(const char *dir)
{
    return mkdir(dir, S_IRWXU | S_IROTH | S_IXOTH);
}

int linux_delete_dir(const char *dir)
{
    char tmp[64] = "rm -rf ";

    strcat(tmp, dir);

    return system(tmp);
}

int linux_delete_file(const char *file)
{
    return remove(file);
}

