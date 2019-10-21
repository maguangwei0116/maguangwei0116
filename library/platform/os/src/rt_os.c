
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
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/reboot.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "rt_os.h"

#ifdef CFG_ENABLE_LIBUNWIND
int32_t _rt_create_task(rt_task *task_id, rt_taskfun task_fun, void * args)
#else
int32_t rt_create_task(rt_task *task_id, rt_taskfun task_fun, void * args)
#endif
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
    static pthread_mutex_t mutex_t;
    mutex = &mutex_t;

    ret = pthread_mutex_init(mutex, NULL);
    if(ret != 0){
        MSG_PRINTF(LOG_WARN, "create mutex error!!\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

int32_t rt_mutex_lock(pthread_mutex_t *mutex)
{
    int32_t ret = 0;

    ret = pthread_mutex_lock(mutex);
    if(ret != 0){
        MSG_PRINTF(LOG_WARN, "lock mutex error!!\n");
        return RT_ERROR;
    }
    return RT_SUCCESS;
}

int32_t rt_mutex_unlock(pthread_mutex_t *mutex)
{
    int32_t ret = 0;

    ret = pthread_mutex_unlock(mutex);
    if(ret != 0){
        MSG_PRINTF(LOG_WARN, "unlock mutex error!!\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

int32_t rt_mutex_destroy(pthread_mutex_t *mutex)
{
    int32_t ret = 0;

    ret = pthread_mutex_destroy(mutex);
    if(ret != 0){
        MSG_PRINTF(LOG_WARN, "destroy mutex error!!\n");
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

pthread_mutex_t *linux_mutex_init(void)
{
    pthread_mutex_t *mutex = NULL;

    mutex = rt_os_malloc(sizeof(pthread_mutex_t));
    if (!mutex) {
        return NULL;
    }
    rt_mutex_init(mutex);

    return mutex;
}

int32_t linux_mutex_lock(pthread_mutex_t *mutex)
{
    if (!mutex) {
        return RT_ERROR;
    }
    return rt_mutex_lock(mutex);
}

int32_t linux_mutex_unlock(pthread_mutex_t *mutex)
{
    if (!mutex) {
        return RT_ERROR;
    }
    return rt_mutex_unlock(mutex);
}

int32_t linux_mutex_release(pthread_mutex_t *mutex)
{
    if (!mutex) {
        return RT_ERROR;
    }
    rt_mutex_destroy(mutex);
    rt_os_free(mutex);
}

//message queue
int32_t rt_creat_msg_queue(int8_t *pathname, int8_t proj_id)
{
    int32_t msgid = -1;
    key_t q_key;

    q_key = ftok(pathname, proj_id);
    if (msgctl(q_key, IPC_RMID, NULL) == -1) {  //remove msg
    }
    msgid = msgget(q_key, 0666 | IPC_CREAT);
    if (msgid == -1){
        MSG_PRINTF(LOG_WARN, "messge id \n");
    }

    return msgid;
}

int32_t rt_receive_queue_msg(int32_t msgid, void *buffer, int32_t len, int64_t msgtyp, int32_t msgflg)
{
    if (msgrcv(msgid, buffer, len, msgtyp, msgflg) == -1) {
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

int32_t rt_send_queue_msg(int32_t msgid, const void *buffer, int32_t len, int32_t msgflg)
{
    if (msgsnd(msgid, buffer, len, msgflg) == -1) {
        MSG_PRINTF(LOG_ERR, "send queue error, err(%d)=%s!!\n", errno, strerror(errno));
        return RT_ERROR;
    }

    return RT_SUCCESS;
}

#ifndef RT_OS_MEM_DEBUG
void *rt_os_malloc(uint32_t size)
{
    return malloc(size);
}

void *rt_os_realloc(void *mem, uint32_t size)
{
    return realloc(mem, size);
}

void *rt_os_calloc(uint32_t count, uint32_t size)
{
    return calloc(count, size);
}

void rt_os_free(void *mem)
{
    if (NULL == mem) {
        return;
    }
    free(mem);
    mem = NULL;
}
#else
static int g_mem_cnt = 0;
void *_rt_os_malloc(const char *file, uint32_t line, uint32_t size)
{
    void *ret = malloc(size);
    printf("[%-50s, %5d] ++++++++ (%5d) malloc : %p\r\n", file, line, ++g_mem_cnt, ret);
    return ret;
}

void _rt_os_free(const char *file, uint32_t line, void *mem)
{
    if (NULL == mem) {
        return;
    }
    printf("[%-50s, %5d] -------- (%5d) free   : %p\r\n", file, line, --g_mem_cnt, mem);
    free(mem);
    mem = NULL;
}

void *_rt_os_realloc(const char *file, uint32_t line, void *mem, uint32_t size)
{
    if (mem) {
        if (!size) {
            _rt_os_free(file, line, mem);
            return NULL;
        } else {
            _rt_os_free(file, line, mem);
            return _rt_os_malloc(file, line, size);
        }
    } else {
        return _rt_os_malloc(file, line, size);
    }
}

#endif

void *rt_os_memset(void *mem, int32_t value, uint32_t len)
{
    if (NULL == mem) {
        return NULL;
    }

    return memset(mem, value, len);
}

void *rt_os_memmove(void *dst, const void *src, uint32_t len)
{
    return memmove(dst, src, len);
}

int32_t rt_os_memcmp(const void *mem_des, const void *mem_src, uint32_t len)
{
    if ((NULL == mem_des) || (NULL == mem_src)) {
        MSG_PRINTF(LOG_WARN, "memory is empty!\n");
        return RT_ERROR;
    }

    return memcmp(mem_des, mem_src, len);
}

uint32_t rt_os_strlen(const char *string)
{
    if (NULL == string) {
        MSG_PRINTF(LOG_WARN, "memory is empty!\n");
        return 0;
    }

    return strlen(string);
}

void *rt_os_strcpy(char* dest, const char *src)
{
    if ((NULL == dest) || (NULL == src)) {
        MSG_PRINTF(LOG_WARN, "strcpy is empty!\n");
        return NULL;
    }

    return strcpy(dest, src);
}

void *rt_os_strncpy(char* dest, const char *src, uint32_t len)
{
    if ((NULL == dest) || (NULL == src)) {
        MSG_PRINTF(LOG_WARN, "strncpy is empty!\n");
        return NULL;
    }

    return strncpy(dest, src, len);
}

int32_t rt_os_strncmp(const char *mem_des, const char *mem_src, uint32_t len)
{
    if ((NULL == mem_des) || (NULL == mem_src)) {
        return RT_ERROR;
    }

    return strncmp(mem_des, mem_src, len);
}

int32_t rt_os_strcmp(const char *mem_des, const char *mem_src)
{
    if ((NULL == mem_des) || (NULL == mem_src)) {
        MSG_PRINTF(LOG_WARN, "memory is empty!\n");
        return RT_ERROR;
    }

    return strcmp(mem_des,mem_src);
}

void *rt_os_memcpy(void *mem_des, const void *mem_src, uint32_t len)
{
    if ((NULL == mem_des) || (NULL == mem_src)) {
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

char *rt_os_strstr(const char *str1, const char *str2)
{
    if ((NULL == str1) || (NULL == str2)) {
        MSG_PRINTF(LOG_WARN, "strstr is empty!\n");
        return NULL;
    }

    return strstr(str1, str2);
}

char *rt_os_strchr(const char *str, int32_t chr)
{
    if (NULL == str) {
        MSG_PRINTF(LOG_WARN, "strstr is empty!\n");
        return NULL;
    }

    return strchr(str, chr);
}

char *rt_os_strrchr(const char *str, int32_t chr)
{
    if (NULL == str) {
        MSG_PRINTF(LOG_WARN, "strstr is empty!\n");
        return NULL;
    }

    return strrchr(str, chr);
}

int32_t rt_os_unlink(const char *pathname)
{
    if (NULL == pathname) {
        return RT_ERROR;
    }

    return unlink(pathname);
}

int32_t rt_os_rename(const char *oldname, const char *newname)
{
    if (NULL == oldname || NULL == newname) {
        return RT_ERROR;
    }

    return rename(oldname, newname);
}

int32_t rt_os_chmod(const char *pathname, RT_MODE mode)
{
    return chmod(pathname, mode);
}

int32_t rt_os_rmdir(const char *pathname)
{
    if (NULL == pathname) {
        return RT_ERROR;
    }

    return rmdir(pathname);
}

int32_t rt_os_mkdir(const char *pathname)
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

void rt_os_exit(int status)
{
    exit(status);
}

void rt_os_sync(void)
{
    sync();
}

void rt_os_sleep(int32_t time)
{
    sleep(time);
}

void *rt_os_signal(int signum, void* handler)
{
    signal(signum, handler);
}
