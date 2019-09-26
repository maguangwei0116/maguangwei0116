
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

typedef struct RT_TIMER {
    struct RT_TIMER *prev, *next;
    int32_t diff_sec;
    int32_t diff_usec;
    void (*func)();
} rt_timer_t;

static pthread_mutex_t g_mutex;
static rt_timer_t *g_timer_list = NULL;

static void callback_timeout(void)
{
    rt_timer_t *p, *q;
    struct itimerval itimer;

    rt_mutex_lock(&g_mutex);
    p = g_timer_list;
    for (q = g_timer_list->next; q; q = q->next) {
        q->diff_sec -= p->diff_sec;
        q->diff_usec -= p->diff_usec;
    }

    if (g_timer_list->next != NULL) {
        g_timer_list = g_timer_list->next;
        itimer.it_interval.tv_sec = 0;
        itimer.it_interval.tv_usec = 0;
        itimer.it_value.tv_sec = g_timer_list->diff_sec;
        itimer.it_value.tv_usec = g_timer_list->diff_usec;
        if ((itimer.it_value.tv_sec == 0) && (itimer.it_value.tv_usec == 0)) {
            itimer.it_value.tv_usec = 10;
        }
        MSG_PRINTF(LOG_INFO, "time:%d\n", itimer.it_value.tv_sec);
        setitimer(ITIMER_REAL, &itimer, NULL);
        g_timer_list->prev = NULL;
    } else {
        g_timer_list = NULL;
    }
    p->func();
    rt_os_free(p);
    p = NULL;
    rt_mutex_unlock(&g_mutex);
}

// timer callbach
static void timer_handler (int signo)
{
    switch(signo) {
    case SIGALRM:
        callback_timeout();
        break;
    }
}

// A timer list from small to large
int32_t register_timer(int sec, int usec, void (*action)())
{
    rt_timer_t *t, *p, *pre;
    struct itimerval itimer;

    if ((action == NULL) || ((sec == 0) && (usec == 0))) {
        return RT_ERROR;
    }

    rt_mutex_lock(&g_mutex);
    t = (rt_timer_t *) rt_os_malloc(sizeof(rt_timer_t));
    t->next = t->prev = NULL;
    t->diff_sec = sec;
    t->diff_usec = usec;
    t->func = action;

    if (g_timer_list == NULL) {
        g_timer_list = t;
    } else {
        for (pre = NULL, p = g_timer_list; p; pre = p, p = p->next) {
            MSG_PRINTF(LOG_INFO, "p->diff_sec:%d,t->diff_sec:%d\n", p->diff_sec, t->diff_sec);
            if (t->diff_sec < p->diff_sec) {
                t->next = p;
                if (p->prev) {
                    p->prev->next = t;
                    t->prev = p->prev;
                } else {
                    t->prev = NULL;
                    g_timer_list = t;
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
    t = g_timer_list;
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

int32_t init_timer(void *arg)
{
    int32_t ret = RT_SUCCESS;
    ret = rt_mutex_init(&g_mutex);
    signal(SIGALRM, timer_handler);
    return ret;
}

uint32_t rt_os_alarm(uint32_t seconds)
{
    return alarm(seconds);
}
