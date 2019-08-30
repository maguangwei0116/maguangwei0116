
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_manage_data.h
 * Date        : 2018.08.08
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/
#ifndef __RT_OS_H__
#define __RT_OS_H__

#include <signal.h>
#include "rt_type.h"

#define  RT_FILE_HANDLE                     FILE *
#define  RT_SET                             SEEK_SET
#define  RT_CUR                             SEEK_CUR
#define  RT_END                             SEEK_END
#define  RT_SIGINT                          SIGINT
#define  RT_SIGTERM                         SIGTERM
#define  RT_SIGALRM                         SIGALRM

typedef unsigned long      rt_task;
typedef void * (* rt_taskfun) (void *para);
typedef pthread_mutex_t    rt_pthread_mutex_t;

int32_t rt_create_task(rt_task *task_id, rt_taskfun task_fun, void * args);

int32_t rt_creat_msg_queue(int8_t *pathname, int8_t proj_id);
int32_t rt_receive_queue_msg(int32_t msgid, void *buffer, int32_t len, int64_t msgtyp, int32_t msgflg);
int32_t rt_send_queue_msg(int32_t msgid, const void *buffer, int32_t len, int32_t msgflg);

int32_t rt_os_memcmp(void *mem_des, const void *mem_src, int32_t len);
int32_t rt_os_strcmp(void *mem_des, const void *mem_src);
int32_t rt_os_strncmp(void *mem_des, const void *mem_src, int32_t len);
uint32_t rt_os_strlen(void *string);
int8_t *rt_os_strchr(int8_t *str, const int8_t chr);
int8_t *rt_os_strstr(int8_t *str1, const int8_t *str2);
void *rt_os_malloc(uint32_t size);
void  rt_os_free(void *mem);
void *rt_os_memset(void *mem, int8_t value, int32_t len);
void *rt_os_memcpy(void *mem_des, const void *mem_src, int32_t len);
void *rt_os_strcpy(char* dest, const char *src);

int32_t rt_os_access(const char *filenpath, int32_t mode);
int32_t rt_os_unlink(const int8_t *pathname);
int32_t rt_os_rename(const int8_t *oldname, const int8_t *newname);

void  rt_os_reboot(void);
void  rt_os_sync(void);
void  rt_os_sleep(int32_t time);

void *rt_os_signal(int32_t signum, void *handler);

int32_t rt_mutex_init(pthread_mutex_t *mutex);
int32_t rt_mutex_lock(pthread_mutex_t *mutex);
int32_t rt_mutex_unlock(pthread_mutex_t *mutex);

#endif // __RT_OS_H__
