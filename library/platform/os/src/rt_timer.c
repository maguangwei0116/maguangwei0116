
/*******************************************************************************
 * Copyright (c) redtea mobile.
 * File name   : rt_timer.c
 * Date        : 2019.08.27
 * Note        :
 * Description :
 * Contributors: RT - create the file
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Sublime text 2
 *******************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "rt_timer.h"

typedef struct my_timer_s my_timer_t;
pthread_mutex_t g_mutex;
struct my_timer_s {
    my_timer_t *prev, *next;
    int32_t diff_sec;
    int32_t diff_usec;
    void (*func)();
};

my_timer_t  *timer_list = NULL;

void callback_timeout(void)
{
    my_timer_t *p, *q;
    struct itimerval itimer;

    rt_mutex_lock(&g_mutex);
    p = timer_list;
    for (q = timer_list->next; q; q = q->next) {
        q->diff_sec -= p->diff_sec;
        q->diff_usec -= p->diff_usec;
    }

    if (timer_list->next != NULL) {
        timer_list = timer_list->next;
        itimer.it_interval.tv_sec = 0;
        itimer.it_interval.tv_usec = 0;
        itimer.it_value.tv_sec = timer_list->diff_sec;
        itimer.it_value.tv_usec = timer_list->diff_usec;
        if ((itimer.it_value.tv_sec == 0) && (itimer.it_value.tv_usec == 0)) {
            itimer.it_value.tv_usec = 10;
        }
        MSG_PRINTF(LOG_INFO, "time:%d\n", itimer.it_value.tv_sec);
        setitimer(ITIMER_REAL, &itimer, NULL);
        timer_list->prev = NULL;
    } else {
        timer_list = NULL;
    }
    p->func();
    rt_os_free(p);
    p = NULL;
    rt_mutex_unlock(&g_mutex);
}

int32_t register_timer(int sec, int usec, void (*action)())
{
    my_timer_t  *t, *p, *pre;
    struct itimerval itimer;

    if ((action == NULL) || ((sec == 0) && (usec == 0))) {
        return RT_ERROR;
    }

    rt_mutex_lock(&g_mutex);
    t = (my_timer_t *) rt_os_malloc(sizeof(my_timer_t));
    t->next = t->prev = NULL;
    t->diff_sec = sec;
    t->diff_usec = usec;
    t->func = action;

    if (timer_list == NULL) {
        timer_list = t;
    } else {
        for (pre = NULL, p = timer_list; p; pre = p, p = p->next) {
            MSG_PRINTF(LOG_INFO, "p->diff_sec:%d,t->diff_sec:%d\n", p->diff_sec, t->diff_sec);
            if (t->diff_sec < p->diff_sec) {
                t->next = p;
                if (p->prev) {
                    p->prev->next = t;
                    t->prev = p->prev;
                } else {
                    t->prev = NULL;
                    timer_list = t;
                }
                p->prev = t;
                break;
            }
        }
        if (p == NULL) {
            t->next = NULL;
            t->prev = pre;
            pre->next = t;
        }
    }
    t = timer_list;
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = t->diff_sec;
    itimer.it_value.tv_usec = t->diff_usec;
    if ((itimer.it_value.tv_sec == 0) && (itimer.it_value.tv_usec == 0)) {
        itimer.it_value.tv_usec = 10;
    }
    MSG_PRINTF(LOG_INFO, "time:%d\n", itimer.it_value.tv_sec);
    setitimer(ITIMER_REAL, &itimer, NULL);
    rt_mutex_unlock(&g_mutex);
    return 0;
}

void timer_handler (int signo)
{
    switch(signo) {
    case SIGALRM:
        callback_timeout();
        break;
    }
}

void init_timer(void)
{
    int32_t ret = RT_SUCCESS;
    ret = rt_mutex_init(&g_mutex);
    MSG_PRINTF(LOG_INFO, "ret:%d\n", ret);
    signal(SIGALRM, timer_handler);
}
