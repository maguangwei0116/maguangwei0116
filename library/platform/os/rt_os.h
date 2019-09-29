
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
#include <sys/stat.h>
#include <sys/ipc.h>
#include "rt_type.h"

#define  RT_FILE_HANDLE                     FILE *
#define  RT_SET                             SEEK_SET
#define  RT_CUR                             SEEK_CUR
#define  RT_END                             SEEK_END
#define  RT_SIGINT                          SIGINT
#define  RT_SIGTERM                         SIGTERM
#define  RT_SIGALRM                         SIGALRM
#define  RT_SIGUSR1                         SIGUSR1
#define  RT_SIGUSR2                         SIGUSR2
#define  RT_MODE                            mode_t
#define  RT_S_ISUID                         S_ISUID  // (04000)  set-user-ID (set process effective user ID on execve(2))
#define  RT_S_ISGID                         S_ISGID  // (02000)
#define  RT_S_ISVTX                         S_ISVTX  // (01000)  sticky bit (restricted deletion flag, as described in unlink(2))
#define  RT_S_IRUSR                         S_IRUSR  // (00400)  read by owner
#define  RT_S_IWUSR                         S_IWUSR  // (00200)  write by owner
#define  RT_S_IXUSR                         S_IXUSR  // (00100)
#define  RT_S_IRGRP                         S_IRGRP  // (00040)  read by group
#define  RT_S_IWGRP                         S_IWGRP  // (00020)  write by group
#define  RT_S_IXGRP                         S_IXGRP  // (00010)  execute/search by group
#define  RT_S_IROTH                         S_IROTH  // (00004)  read by others
#define  RT_S_IWOTH                         S_IWOTH  // (00002)  write by others
#define  RT_S_IXOTH                         S_IXOTH  // (00001)  execu
#define  RT_S_IRWXU                         (RT_S_IRUSR | RT_S_IWUSR | RT_S_IXUSR)
#define  RT_S_IRWXG                         (RT_S_IRGRP | RT_S_IWGRP | RT_S_IXGRP)
#define  RT_S_IRWXO                         (RT_S_IROTH | RT_S_IWOTH | RT_S_IXOTH)
#define  RT_IPC_NOWAIT                      IPC_NOWAIT

typedef unsigned long      rt_task;
typedef void * (* rt_taskfun) (void *para);
typedef pthread_mutex_t    rt_pthread_mutex_t;

int32_t rt_create_task(rt_task *task_id, rt_taskfun task_fun, void * args);
int32_t rt_creat_msg_queue(int8_t *pathname, int8_t proj_id);
int32_t rt_receive_queue_msg(int32_t msgid, void *buffer, int32_t len, int64_t msgtyp, int32_t msgflg);
int32_t rt_send_queue_msg(int32_t msgid, const void *buffer, int32_t len, int32_t msgflg);

int32_t rt_os_memcmp(const void *mem_des, const void *mem_src, uint32_t len);
int32_t rt_os_strcmp(const char *mem_des, const char *mem_src);
int32_t rt_os_strncmp(const char *mem_des, const char *mem_src, int32_t len);
uint32_t rt_os_strlen(const char *string);
char *rt_os_strchr(const char *str, int32_t chr);
char *rt_os_strrchr(const char *str, int32_t chr);
char *rt_os_strstr(const char *str1, const char *str2);
void *rt_os_memmove(void *dst, const void *src, size_t len);
void *rt_os_memset(void *mem, int32_t value, uint32_t len);
void *rt_os_memcpy(void *mem_des, const void *mem_src, int32_t len);
void *rt_os_strcpy(char* dest, const char *src);

//#define RT_OS_MEM_DEBUG 1

#ifndef RT_OS_MEM_DEBUG
void *rt_os_malloc(uint32_t size);
void *rt_os_realloc(void *mem, uint32_t size);
void  rt_os_free(void *mem);
void *rt_os_calloc(size_t count, size_t size);
#else
void *_rt_os_malloc(const char *file, uint32_t line, uint32_t size);
void *_rt_os_realloc(const char *file, uint32_t line, void *mem, uint32_t size);
void  _rt_os_free(const char *file, uint32_t line, void *mem);

#define rt_os_malloc(size)          _rt_os_malloc(__FILE__, __LINE__, size)
#define rt_os_realloc(mem, size)    _rt_os_realloc(__FILE__, __LINE__, mem, size)
#define rt_os_free(mem)             _rt_os_free(__FILE__, __LINE__, mem)
#endif

int32_t rt_os_access(const char *filenpath, int32_t mode);
int32_t rt_os_unlink(const char *pathname);
int32_t rt_os_rename(const char *oldname, const char *newname);
int32_t rt_os_chmod(const char *pathname, RT_MODE mode);

void  rt_os_reboot(void);
void rt_os_exit(int status);
void  rt_os_sync(void);
void  rt_os_sleep(int32_t time);

void *rt_os_signal(int32_t signum, void *handler);

RT_FILE_HANDLE rt_os_open(const char *filename, const char *flags);
uint32_t rt_os_alarm(uint32_t seconds);

int32_t rt_mutex_init(pthread_mutex_t *mutex);
int32_t rt_mutex_lock(pthread_mutex_t *mutex);
int32_t rt_mutex_unlock(pthread_mutex_t *mutex);

rt_pthread_mutex_t *linux_mutex_init(void);
int32_t linux_mutex_lock(pthread_mutex_t *mutex);
int32_t linux_mutex_unlock(pthread_mutex_t *mutex);
int32_t linux_mutex_release(pthread_mutex_t *mutex);

#endif // __RT_OS_H__
