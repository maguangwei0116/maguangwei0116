
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_mem.c
 * Date        : 2017.10.30
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <memory.h>
#include <pthread.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/reboot.h>
#include "rt_os.h"

int32_t rt_create_task(rt_task *task_id, rt_taskfun task_fun,void * args)
{
    int32_t ret = RT_ERROR;
    ret = pthread_create(task_id, NULL, task_fun, args);
    if (ret != 0) {
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_mutex_init(pthread_mutex_t *mutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_init(mutex, NULL);
    if(ret != 0){
        MSG_WARN("create mutex error!!\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_mutex_lock(pthread_mutex_t *mutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_lock(mutex);
    if(ret != 0){
        MSG_WARN("lock mutex error!!\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_mutex_unlock(pthread_mutex_t *mutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_unlock(mutex);
    if(ret != 0){
        MSG_WARN("unlock mutex error!!\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_mutex_destroy(pthread_mutex_t *mutex)
{
    int32_t ret = 0;
    ret = pthread_mutex_destroy(mutex);
    if(ret != 0){
        MSG_WARN("destroy mutex error!!\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

//message queue
int32_t rt_creat_msg_queue(char *pathname, char proj_id)
{
    int32_t msgid = -1;
    key_t q_key;

    q_key = ftok(pathname, proj_id);
    if (msgctl(q_key, IPC_RMID, NULL) == -1) {  //remove msg
    }
    msgid = msgget(q_key, 0666 | IPC_CREAT);
    if (msgid == -1){
        MSG_WARN("messge id \n");
    }
    return msgid;
}

int32_t rt_receive_queue_msg(int32_t msgid, void *buffer, int32_t len)
{
    if (msgrcv(msgid, buffer, len, 0, 0) == -1) {
        MSG_WARN("no message data\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_send_queue_msg(int32_t msgid, void *buffer, int32_t len)
{
    if (msgsnd(msgid, buffer, len, 0) == -1) {
        MSG_WARN("send message data error\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

void *rt_os_malloc(uint32_t size)
{
    return malloc(size);
}

void rt_os_free(void *mem)
{
    if (NULL == mem) {
        return;
    }
    free(mem);
    mem = NULL;
}

void *rt_os_memset(void *mem, int8_t value, int32_t len)
{
    if (NULL == mem) {
        return NULL;
    }
    return memset(mem, value, len);
}

int32_t rt_os_memcmp(void *mem_des,void *mem_src,int32_t len)
{
    if ((NULL == mem_des)||(NULL == mem_src)) {
        MSG_WARN("memory is empty!\n");
        return RT_ERROR;
    }
    return memcmp(mem_des, mem_src, len);
}

uint32_t rt_os_strlen(void *string)
{
    if (NULL == string) {
        MSG_WARN("memory is empty!\n");
        return 0;
    }
    return strlen(string);
}

void *rt_os_strcpy(char* dest, const char *src)
{
    if ((NULL == dest)||(NULL == src)) {
        MSG_WARN("strcpy is empty!\n");
        return NULL;
    }
    return strcpy(dest,src);
}

int32_t rt_os_strncmp(void *mem_des,void *mem_src,int32_t len)
{
    if ((NULL == mem_des)||(NULL == mem_src)) {
        MSG_WARN("memory is empty!\n");
        return RT_ERROR;
    }
    return strncmp(mem_des, mem_src, len);
}

int32_t rt_os_strcmp(void *mem_des,void *mem_src)
{
    if ((NULL == mem_des)||(NULL == mem_src)) {
        MSG_WARN("memory is empty!\n");
        return RT_ERROR;
    }
    return strcmp(mem_des,mem_src);
}

void *rt_os_memcpy(void *mem_des, void *mem_src, int32_t len)
{
    if ((NULL == mem_des)||(NULL == mem_src)) {
        MSG_WARN("memory is empty!\n");
        return NULL;
    }
    if (len <= 0) {
        return NULL;
    }
    return memcpy(mem_des, mem_src, len);
}

int32_t rt_os_access(const char *filenpath, int32_t mode)
{
    if (filenpath == NULL) {
        return RT_ERROR;
    }
    return access(filenpath, mode);
}

int8_t *rt_os_strstr(int8_t *str1, const int8_t *str2)
{
    if ((NULL == str1)||(NULL == str2)) {
        MSG_WARN("strstr is empty!\n");
        return NULL;
    }
    return strstr(str1, str2);
}

int8_t *rt_os_strchr(int8_t *str, const int8_t chr)
{
    if (NULL == str) {
        MSG_WARN("strstr is empty!\n");
        return NULL;
    }
    return strchr(str, chr);
}

RT_FILE_HANDLE rt_os_open(const int8_t *filename, const int8_t *flags)
{
    if (NULL == filename) {
        return NULL;
    }

    return fopen(filename, flags);

}

int32_t rt_os_read(void *ptr, int32_t len, int32_t num, RT_FILE_HANDLE handle)
{
    if (NULL == ptr || NULL == handle) {
        return RT_ERROR;
    }

    return fread(ptr, (size_t)len, num, handle);
}

int32_t rt_os_write(const void *ptr, int32_t len, int32_t num, RT_FILE_HANDLE handle)
{
    if (NULL == ptr || NULL == handle) {
        return RT_ERROR;
    }

    return fwrite(ptr, (size_t)len, num, handle);
}

int32_t rt_os_seek(RT_FILE_HANDLE handle, int32_t offset, int32_t whence)
{
    if (NULL == handle) {
        return RT_ERROR;
    }

    return fseek(handle, offset, whence);
}

int32_t rt_os_close(RT_FILE_HANDLE handle)
{
    if (NULL == handle) {
        return RT_ERROR;
    }

    return fclose(handle);
}

int32_t rt_os_unlink(const int8_t *pathname)
{
    if (NULL == pathname) {
        return RT_ERROR;
    }
    return unlink(pathname);
}

int32_t rt_os_rename(const int8_t *oldname, const int8_t *newname)
{
    if(NULL == oldname || NULL == newname) {
        return RT_ERROR;
    }
    return rename(oldname, newname);
}

int32_t rt_os_rmdir(const int8_t *pathname)
{
    if (NULL == pathname) {
        return RT_ERROR;
    }
    return rmdir(pathname);
}

int32_t rt_os_mkdir(const int8_t *pathname)
{
    if (NULL == pathname) {
        return RT_ERROR;
    }
    return mkdir(pathname,  S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void rt_os_reboot(void)
{
    sync();
    sync();
    reboot(LINUX_REBOOT_CMD_POWER_OFF);
}

void rt_os_sync(void)
{
    sync();
}

void rt_os_sleep(int32_t time)
{
    sleep(time);
}
