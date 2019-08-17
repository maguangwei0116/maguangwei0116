
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
#include <sys/stat.h>
#include "rt_manage_data.h"

int32_t rt_create_file(uint8_t *file_name)
{
    int8_t status = RT_ERROR;
    FILE *fp = NULL;
    if ((fp = fopen(file_name, "w+")) == NULL) {
        MSG_WARN("open error \n");
    } else {
        status = RT_SUCCESS;
    }

    if(fp != NULL) {
        fclose(fp);
    }
    return status;
}

int32_t rt_write_data(uint8_t *addr, uint32_t offset, const uint8_t *data_buffer, uint32_t len)
{
    FILE *fp = NULL;
    int32_t status = RT_ERROR;
    fp = fopen(addr, "rb+");
    if (fp == NULL) {
        MSG_WARN("open config error\n");
    } else {
        fseek(fp,offset,SEEK_SET);
        if (fwrite(data_buffer,len, 1, fp) != 1) {
            MSG_WARN("write error\n");
        } else {
            status = 0;
        }
        fclose(fp);
    }
    rt_os_sync();
    return status;
}

int32_t rt_read_data(uint8_t *addr, uint32_t offset, uint8_t *data_buffer, uint32_t len)
{
    FILE *fp = NULL;
    int32_t status = RT_ERROR;

    fp = fopen(addr, "r");
    if (NULL == fp) {
        MSG_WARN("open config file error\n");
    } else {
        fseek(fp, offset, SEEK_SET);
        if (fread(data_buffer, len, 1, fp) != 1) {
        } else {
            status = RT_SUCCESS;
        }
        fclose(fp);
    }
    return status;
}
int32_t rt_truncate_data(uint8_t *filename, int32_t offset)
{
    if (filename == NULL) {
        return RT_ERROR;
    }
    return truncate(filename, offset);
}

int32_t rm_dir(const int8_t *dirpath)
{
    int8_t sub_path[100];
    DIR* dirp = NULL;
    struct dirent *dir;
    struct stat st;

    opendir(dirpath);
    if(!dirp) {
        return RT_ERROR;
    }

    while ((dir = readdir(dirp)) != NULL) {
        if (strcmp(dir->d_name,".") == 0
                || strcmp(dir->d_name,"..") == 0) {
            continue;
        }

        snprintf(sub_path, 100, "%s/%s", dirpath, dir->d_name);

        if (lstat(sub_path, &st) == -1) {
            MSG_WARN("rm_dir:lstat %s error\n", sub_path);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (rm_dir(sub_path) == -1) {  // 如果是目录文件，递归删除
                closedir(dirp);
                return -1;
            }
            rt_os_rmdir(sub_path);
        } else if(S_ISREG(st.st_mode)) {
            rt_os_unlink(sub_path);
        } else {
            MSG_WARN("rm_dir:lstat %s error\n", sub_path);
            continue;
        }
    }
    if (rt_os_rmdir(dirpath) == -1) {//delete dir itself.
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return RT_SUCCESS;
}

int32_t rm(const int8_t *file_name)
{
    struct stat st;

    if (lstat(file_name, &st) == -1) {
        return RT_ERROR;
    }

    if (S_ISREG(st.st_mode)) {
        if(rt_os_unlink(file_name) == -1) {
            return RT_ERROR;
        }
    } else if (S_ISDIR(st.st_mode)) {
        if (!strcmp(file_name, ".") || !strcmp(file_name, "..")) {
            return RT_ERROR;
        }

        if (rm_dir(file_name) == -1) {  // delete all the files in dir.
            return RT_ERROR;
        }
    }
    return RT_SUCCESS;
}
