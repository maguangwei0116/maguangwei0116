
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

#define MAX_RT_FILE_PATH        128
#define MAX_BUFFER_SIZE         2048

static char g_rt_file_path[MAX_RT_FILE_PATH];

rt_dir_t linux_opendir(const char *dir_name)
{
    return opendir(dir_name);
}

int linux_closedir(rt_dir_t dir)
{
    return closedir(dir);
}

rt_dirent_t linux_readdir(rt_dir_t dir)
{
    return readdir(dir);
}

rt_fshandle_t linux_fopen(const char *file_name, rt_fsmode_t mode)
{
    rt_fshandle_t fp = NULL;

    fp = fopen(file_name, mode);
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

rt_bool linux_dir_exist(const char *dir_name)
{
    if(access(dir_name, RT_FS_F_OK) == 0)
        return RT_TRUE;

    return RT_FALSE;
}

rt_bool linux_file_exist(const char *file_name)
{
    if(access(file_name, RT_FS_F_OK) == 0)
        return RT_TRUE;

    return RT_FALSE;
}

int linux_create_dir(const char *dir_name)
{
    return mkdir(dir_name, RT_S_IRWXU | RT_S_IROTH | RT_S_IXOTH);
}

int linux_delete_dir(const char *dir_name)
{
    char tmp[64];

    snprintf(tmp, sizeof(tmp), "rm -rf %s", dir_name);

    return system(tmp);
}

int linux_delete_file(const char *file_name)
{
    return remove(file_name);
}

int linux_truncate(const char *file_name, off_t offset)
{
    if (file_name == NULL) {
        return RT_ERROR;
    }
    return truncate(file_name, offset);
}

int linux_file_size(const char *file_name)
{
    struct stat statbuf;
    int32_t size;

    if (rt_os_access(file_name, RT_FS_F_OK)) { /* file isn't exist */
        return 0;
    }

    stat(file_name, &statbuf);
    size = statbuf.st_size;

    return size;
}

int linux_stat(const char *path_name, rt_stat_t *buf)
{
    return stat(path_name, buf);
}

int linux_fstat(int fd, rt_stat_t *buf)
{
    return fstat(fd, buf);
}

int linux_lstat(const char *path_name, rt_stat_t *buf)
{
    return lstat(path_name, buf);
}

int shell_cmd(const int8_t *cmd, uint8_t *buf, int size)
{
    rt_fshandle_t fp;
    int num = 0;

    fp = popen(cmd, RT_FS_READ);
    if (fp == NULL) {
        MSG_PRINTF(LOG_ERR, "Popen failed!!\n");
        return num;
    }
    num = linux_fread(buf, 1, size, fp);
    pclose(fp);

    return num;
}

int init_rt_file_path(void *arg)
{
    const char *redtea_path = (const char *)arg;

    if (redtea_path) {
        snprintf(g_rt_file_path, sizeof(g_rt_file_path), "%s", redtea_path);
        if (rt_os_access(redtea_path, RT_FS_F_OK)) {
            rt_os_mkdir(redtea_path);
        }
    } else {
        g_rt_file_path[0] = '\0';
    }
    
    return 0;
}

rt_fshandle_t linux_rt_fopen(const char *file_name, rt_fsmode_t mode)
{
    char rt_file_name[MAX_RT_FILE_PATH];

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, file_name);

    return linux_fopen(rt_file_name, mode);
}

int linux_rt_file_size(const char *file_name)
{
    char rt_file_name[MAX_RT_FILE_PATH];

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, file_name);

    return linux_file_size(rt_file_name);   
}

rt_bool linux_rt_file_exist(const char *file_name)
{
    char rt_file_name[MAX_RT_FILE_PATH];

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, file_name);

    return linux_file_exist(rt_file_name);
}

int linux_rt_truncate(const char *file_name, off_t offset)
{
    char rt_file_name[MAX_RT_FILE_PATH];
    
    if (!file_name) {
        return RT_ERROR;
    }

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, file_name);
    
    return linux_truncate(rt_file_name, offset);
}

int linux_rt_mkdir(const char *path_name)
{
    char rt_path_name[MAX_RT_FILE_PATH];
    
    if (!path_name) {
        return RT_ERROR;
    }

    snprintf(rt_path_name, sizeof(rt_path_name), "%s/%s", g_rt_file_path, path_name);

    return rt_os_mkdir(rt_path_name);
}

int linux_rt_delete_file(const char *file_name)
{
    char rt_file_name[MAX_RT_FILE_PATH];

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, file_name);

    return linux_delete_file(rt_file_name);
}

int linux_rt_rename_file(const char *relative_src_file_path, const char *absolute_dst_file_path)
{
    char rt_file_name[MAX_RT_FILE_PATH];

    snprintf(rt_file_name, sizeof(rt_file_name), "%s/%s", g_rt_file_path, relative_src_file_path);

    return rt_os_rename(rt_file_name, absolute_dst_file_path);
}

int linux_rt_file_abs_path(const char *file_name, char *file_abs_path, int len)
{
    snprintf(file_abs_path, len, "%s/%s", g_rt_file_path, file_name);

    return RT_SUCCESS;
}

int linux_rt_file_copy(const char *src, const char *dst)
{
    uint8_t buffer[MAX_BUFFER_SIZE];
    rt_fshandle_t pin;
    rt_fshandle_t pout;
    int ret = RT_ERROR;
    int len;

    pin = linux_rt_fopen(src, "rb");
    pout = linux_rt_fopen(dst, "wb");

    if (!pin || !pout) {
        MSG_PRINTF(LOG_ERR, "fopen failed\n");
        goto exit_entry;
    }

    while (!feof(pin)) {
        len = linux_fread(buffer, 1, sizeof(buffer), pin);
        if (len <= 0) {
            MSG_PRINTF(LOG_ERR, "fread failed!!\n");
            break;
        }
        linux_fwrite(buffer, 1, len, pout);
    }

    ret = RT_SUCCESS;

exit_entry:
    
    if (pin) {
        linux_fclose(pin);
        pin = NULL;
    }

    if (pout) {
        linux_fclose(pout);
        pout = NULL;
    }

    return RT_SUCCESS;
}

