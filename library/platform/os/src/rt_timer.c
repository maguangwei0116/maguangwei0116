
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
    sigset_t set, oldset;

    p = timer_list;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK, &set, &oldset);

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
        setitimer(ITIMER_REAL, &itimer, NULL);
    }
    sigprocmask(SIG_SETMASK,&oldset,NULL);
    p->func();
    rt_os_free(p);
}

int32_t register_timer(int sec, int usec, void (*action)())
{
    my_timer_t  *t, *p, *pre;
    struct itimerval itimer;
    struct sigaction sa;
    sigset_t set, oldset;

    t = (my_timer_t *) rt_os_malloc(sizeof(my_timer_t));
    t->next = t->prev = NULL;
    t->diff_sec = sec;
    t->diff_usec = usec;
    t->func = action;

    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigprocmask(SIG_SETMASK,&set,&oldset);

    if (timer_list == NULL) {
        timer_list = t;
    } else {
        for (pre = NULL, p = timer_list; p; pre = p, p = p->next) {
            if (p->diff_sec > t->diff_sec ) {
                t->next = p;
                p->prev = t;
                if (p->prev) {
                    p->prev->next = t;
                    t->prev = p->prev;
                }
                break;
            }
        }
        if (p == NULL) {
            t->prev = pre;
            pre->next = t;
        }
    }
    t = timer_list;
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 0;
    itimer.it_value.tv_sec = t->diff_sec;
    itimer.it_value.tv_usec = t->diff_usec;
    setitimer(ITIMER_REAL, &itimer, NULL);
    sigprocmask(SIG_SETMASK, &oldset, NULL);
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
    signal(SIGALRM, timer_handler);
}
