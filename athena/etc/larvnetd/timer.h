/* $Id: timer.h,v 1.1 1998-09-01 20:57:47 ghudson Exp $ */

/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file declares the interface for a mini-library of functions
 * for setting up and processing timers.
 */

#ifndef TIMER__H
#define TIMER__H

#include <sys/types.h>
#include <time.h>

typedef void (*Timer_proc)(void *);

typedef struct _Timer {
  int heap_pos;
  time_t abstime;
  Timer_proc func;
  void *arg;
} Timer;

Timer *timer_set_rel(int reltime, Timer_proc proc, void *arg);
Timer *timer_set_abs(time_t abstime, Timer_proc proc, void *arg);
void *timer_reset(Timer *timer);
void timer_process(void);
struct timeval *timer_timeout(struct timeval *tvbuf);

#endif /* TIMER__H */
