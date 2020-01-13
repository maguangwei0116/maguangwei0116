
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_manage_data.c
 * Date        : 2017.10.31
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include "rt_type.h"
#include "log.h"
#include "rt_manage_data.h"
#include "file.h"

int32_t rt_create_file(const char *file_name)
{
    int8_t status = RT_ERROR;
    rt_fshandle_t fp = NULL;

    if ((fp = linux_rt_fopen(file_name, "w+")) == NULL) {
        MSG_PRINTF(LOG_WARN, "open failed \n");
    } else {
        status = RT_SUCCESS;
    }
    if (fp != NULL) {
        linux_fclose(fp);
    }

    return status;
}

int32_t rt_write_data(const char *file_name, uint32_t offset, const uint8_t *data_buffer, uint32_t len)
{
    rt_fshandle_t fp = NULL;
    int32_t status = RT_ERROR;

    fp = linux_rt_fopen(file_name, "rb+");
    if (fp == NULL) {
        MSG_PRINTF(LOG_WARN, "open file failed\n");
    } else {
        linux_fseek(fp,offset, RT_FS_SEEK_SET);
        if (linux_fwrite(data_buffer,len, 1, fp) != 1) {
            MSG_PRINTF(LOG_WARN, "write failed\n");
        } else {
            status = 0;
        }
        linux_fclose(fp);
        rt_os_sync();
    }
    
    return status;
}

int32_t rt_read_data(const char *file_name, uint32_t offset, uint8_t *data_buffer, uint32_t len)
{
    rt_fshandle_t fp = NULL;
    int32_t status = RT_ERROR;

    fp = linux_rt_fopen(file_name, "r");
    if (NULL == fp) {
        MSG_PRINTF(LOG_WARN, "open file %s failed\n", file_name);
    } else {
        linux_fseek(fp, offset, RT_FS_SEEK_SET);
        if (linux_fread(data_buffer, len, 1, fp) != 1) {
        } else {
            status = RT_SUCCESS;
        }
        linux_fclose(fp);
    }

    return status;
}
int32_t rt_truncate_data(const char *file_name, uint32_t offset)
{
    if (!file_name) {
        return RT_ERROR;
    }
    return linux_rt_truncate(file_name, offset);
}

int32_t rt_rm_dir(const char *dir_path)
{
    char sub_path[128];
    rt_dir_t dirp = NULL;
    rt_dirent_t dir;
    rt_stat_t st;

    linux_opendir(dir_path);
    if (!dirp) {
        return RT_ERROR;
    }

    while ((dir = linux_readdir(dirp)) != NULL) {
        if (rt_os_strncmp(dir->d_name, ".", 1) == 0 || rt_os_strncmp(dir->d_name, "..", 2) == 0) {
            continue;
        }

        snprintf(sub_path, sizeof(sub_path), "%s/%s", dir_path, dir->d_name);

        if (linux_lstat(sub_path, &st) == -1) {
            MSG_PRINTF(LOG_WARN, "rm dir:l-stat %s error\n", sub_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (rt_rm_dir(sub_path) == -1) {  // 如果是目录文件，递归删除
                linux_closedir(dirp);
                return -1;
            }
            rt_os_rmdir(sub_path);
        } else if (S_ISREG(st.st_mode)) {  // 普通文件
            rt_os_unlink(sub_path);
        } else {
            MSG_PRINTF(LOG_WARN, "rm dir:l-stat %s error\n", sub_path);
            continue;
        }
    }
    if (rt_os_rmdir(dir_path) == -1) { // delete dir itself.
        linux_closedir(dirp);
        return -1;
    }
    linux_closedir(dirp);

    return RT_SUCCESS;
}

int32_t rt_rm(const char *file_name)
{
    rt_stat_t st;

    if (linux_lstat(file_name, &st) == -1) {
        return RT_ERROR;
    }

    if (S_ISREG(st.st_mode)) {
        if (rt_os_unlink(file_name) == -1) {
            return RT_ERROR;
        }
    } else if (S_ISDIR(st.st_mode)) {
        if (!rt_os_strncmp(file_name, ".", 1) || !rt_os_strncmp(file_name, "..", 2)) {
            return RT_ERROR;
        }

        if (rt_rm_dir(file_name) == -1) {  // delete all the files in dir.
            return RT_ERROR;
        }
    }

    return RT_SUCCESS;
}

