/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 * 
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ========================================================================== 
 *
 * M S D O S
 *
 * MS/DOS-specific utilities.  (MS/DOS)
 *
 * Written by Kevin Ackley of Systems Guild Inc.
 */

#include "std.h"
#include "msdos.h"
#include <dos.h>

/*
 * Flag to enable internal socket debugging.
 * Not for application developers.
 */
unsigned char socket_$debug = 0;
unsigned char socket_$max_debug = 0;

extern unsigned char rpc_$debug;

/*** Want to use the real time() from now on */
#undef time
extern long time(long *);

/*===========================================================================
xlong (64-bit) math routines.  (also see msdosa.asm)
===========================================================================*/

void xlong_$set_long(xl, l)
xlong xl;
u_long l;
{
    XLONG_$LONG(xl, 0) = l;
    XLONG_$LONG(xl, 4) = 0L;
}

void xlong_$set_xlong(xl, high, low)
xlong xl;
u_long high, low;
{
    XLONG_$LONG(xl, 0) = low;
    XLONG_$LONG(xl, 4) = high;
}

/*===========================================================================
Time, alarm functions
===========================================================================*/
#define ALARM_STACK_SIZE (4*1024)  /* The size of stack used in interrupt */

#ifdef USE_INTERRUPT
short NEAR alarm_$interrupt_on=0;              /* is handler in int. chain  */

/*** Note alarm stack is in DGROUP segment so calls to msc-rtl funcs ok */
internal char NEAR alarm_stack[ALARM_STACK_SIZE] = {0};

internal long NEAR alarm_interval = 182;       /* # of ticks between alarms */
internal volatile long NEAR alarm_count = 0;   /* # of ticks to next alarm  */
internal long NEAR start_time = 0L;            /* time alarm_$on was called */
internal volatile short NEAR alarm_postponed=0;/* int when couldn't call handler   */
/*
 * The following is a pointer to a flag inside of the MS/DOS "operating system" 
 * which indicates that DOS has been called (via int21).  We test it in the 
 * sysclock interrupt to see whether the interrupted thread was inside DOS. 
 * Since DOS is not reentrant and the NCK may call it in the interrupt (e.g.
 * to print debugging info), it is safer to defer any processing to a later
 * time (none of the interrupt driven processing is super-critical).
 */
internal char far* NEAR in_dos;                /* T => inside of DOS         */
#ifdef MAX_DEBUG
long alarm_$postponed_cnt = 0L;
long alarm_$not_postponed_cnt = 0L;
extern long far alarm_$reenter_cnt;   /* **not** in DGROUP */
#endif
#endif
/*???internal*/ volatile short NEAR alarm_$disabled=0; /* is nck handler callable (counter)*/
internal void (* NEAR alarm_func)() = NULL;      /* nck alarm handler              */
internal volatile char NEAR in_alarm_handler=0;/* T => inside of alarm handler     */

void alarm_$start(handler, interval)
void (*handler)();
int interval;
{
#ifdef USE_INTERRUPT
    alarm_$disable();
    alarm_func = handler;
    alarm_$enable();

    alarm_interval = SEC_TO_TICK(interval);
    if (alarm_interval <= 0)
        alarm_interval = 1;
    alarm_count = alarm_interval;

    alarm_$on();
#else
    alarm_func = handler;
#endif
}

#ifdef USE_INTERRUPT

/*
 * Note: alarm interrupt can be turned on before alarm_func is set (if
 * any of the time functions are used before an alarm handler is set in
 * rpc_$periodically).
 */

int alarm_$on()
{
    internal int alarm_off();
    union REGS inregs, outregs;
    struct SREGS segregs;

    if (alarm_$interrupt_on)
        return(1);

    /*
     * Get address of DOS's "in DOS" flag.
     */

    inregs.x.ax = 0x3400;
    intdosx(&inregs, &outregs, &segregs);
    SEGMENT_OF(in_dos) = segregs.es;
    OFFSET_OF(in_dos)  = outregs.x.bx;

    /*
     * Note the current time as a base for the tick time.
     */

    if (start_time == 0L)
        start_time = time(NULL);

    /*
     * Turn on our interrupt routine.
     */

    if (atexit(alarm_off) != 0) {
        fprintf(stderr, "Warning (alarm): can't set exit() function for alarm\n");
        return(0);
    }

    alarm_postponed = 0;
    alarm_$interrupt_on = 1;

    alarm_$open(&alarm_stack[ALARM_STACK_SIZE], alarm_stack);

    return(1);
}

/*
 * alarm_off() is called by exit().
 */

internal int alarm_off()
{
    if (alarm_$interrupt_on) {
        alarm_$close();
        alarm_$interrupt_on = 0;
    }
#ifdef MAX_DEBUG
    if (rpc_$debug) {
        printf("alarm_$postponed_cnt = %ld\n", alarm_$postponed_cnt);
        printf("alarm_$not_postponed_cnt = %ld\n", alarm_$not_postponed_cnt);
        printf("alarm_$reenter_cnt = %ld\n", alarm_$reenter_cnt);
    }
#endif
}

#endif

void alarm_$enable()
{
    alarm_$disabled--;  /* should be an atomic decrement */

#ifdef MAX_DEBUG
    if (alarm_$disabled < 0)
        fprintf(stderr, "(alarm_$enable): count < 0\n");
#endif

    /*
     * Don't try to deliver any alarms if disabled or already
     * in an alarm call.
     */
    if (alarm_$disabled || in_alarm_handler)
        return;

#ifdef USE_INTERRUPT
    /*
     * Check if an alarm was postponed, if so do it now (using
     * the current stack).  Don't deliver alarm if disabled or
     * already in alarm call.
     */
    if (alarm_postponed) {
        alarm_postponed = 0;
        if (alarm_func != NULL) {
            in_alarm_handler = 1; /*???*/
            (*alarm_func)();
            in_alarm_handler = 0; /*???*/
        }
    }
#else
    check_sync_alarm();
#endif
}

int alarm_$in_alarm()
{
    return (in_alarm_handler);
}

void alarm_$disable()
{
    alarm_$disabled++;  /* should be an atomic increment */
}

/*
 * alarm_$handler() is called from assembly language at interrupt
 * time, after a stack is set up.  There is only one interrupt stack, 
 * so assembly code guarantees that this function won't reenter.
 */

void alarm_$handler()
{
#ifdef USE_INTERRUPT
    if (--alarm_count > 0)
        return;
    alarm_count = alarm_interval;

    /*** If alarms are currently disabled or if interrupted thread is inside DOS, postpone */
    if (alarm_$disabled > 0 || *in_dos != 0) {
#ifdef MAX_DEBUG
        alarm_$postponed_cnt++;
#endif
        alarm_postponed = 1;
    } else {
        alarm_postponed = 0;
#ifdef MAX_DEBUG
        alarm_$not_postponed_cnt++;
#endif
        if (alarm_func != NULL) {
            in_alarm_handler = 1;
            (*alarm_func)();
            in_alarm_handler = 0;
        }
    }
#endif
}

#ifndef USE_INTERRUPT

/*??? explain... */
internal check_sync_alarm()
{
    long now;
    static long last_alarm_time = 0;

    if (alarm_$disabled || in_alarm_handler)
        return;

    now = time(NULL);
    if (now > last_alarm_time) {
        last_alarm_time = now;
        if (alarm_func != NULL) {
            in_alarm_handler = 1;
            (*alarm_func)();
            in_alarm_handler = 0;
        }
    }
}

#endif

/*
 * time_nck() returns APPROXIMATE Unix time in seconds -- less than 
 * 10% error from time at program start.  NOT to be used where time 
 * must be exactly correct (i.e. generating uuids), see alarm_$4usec_time()
 * below.
 */

long time_nck()
{
#ifdef USE_INTERRUPT
    alarm_$on();

    /*
     * See msdos.h for discussion of accuracy of the following statement.
     */
    return(start_time + TICK_TO_SEC(alarm_$time()));
#else
    check_sync_alarm();
    return(time(NULL));
#endif
}

void sleep(secs)
int secs;
{
    long end;

#ifdef USE_INTERRUPT
    if (!alarm_$on())
        return;

    end = alarm_$time() + SEC_TO_TICK(secs);
    while (alarm_$time() < end)
        ;
#else
    end = secs + time(NULL);
    while (time(NULL) < end)
        check_sync_alarm();
#endif
}

/*
 * Read time in best resolution possible and convert to Apollo's
 * time standard: 48 bit number of 4usec counts from 0:00 1/1/80.
 */

void alarm_$4usec_time(high, low)
u_long* high;
u_short* low;
{
    u_long t;
    xlong xl, xl_start;
#   define decade_1970  (3652L * 24 * 60 * 60)

#if defined(USE_INTERRUPT)
    /*
     * If interrupt is used, time is measured to ~55 msec resolution.
     */
    alarm_$on();
    t = alarm_$time();

    /*
     * ticks from start are multiplied by the following constant:
     *    (0.054925416/4E-6) << 16
     * resulting in time (4usec) stored in high 48 bits of xlong
     */
    xlong_$set_long(xl, 899898024L);
    xlong_$mult_long(xl, t);

    /*
     * starting time is adjusted to 1/1/80 (from 1/1/70) and
     * multiplied by the following constant:
     *    (1.0/4E-6) << 16
     */
    xlong_$set_xlong(xl_start, 0x3L, 0xD0900000L);
    xlong_$mult_long(xl_start, start_time - decade_1970);

    /*
     * Add starting time plus time from start
     */
    xlong_$add_xlong(xl, xl_start);
#else
    /*
     * If not using interrupt, get time from MSDOS.
     * Time measured to 1 sec resolution.
     */
    t = time(NULL) - decade_1970;

    /*
     * seconds are multiplied by the following constant:
     *    (1.0/4E-6) << 16
     * resulting in time (4usec) stored in high 48 bits of xlong
     */
    xlong_$set_xlong(xl, 0x3L, 0xD0900000L);
    xlong_$mult_long(xl, t);
#endif
    *high = XLONG_$LONG(xl, 4);
    *low = XLONG_$SHORT(xl, 2);
}

/*===========================================================================
Misc utilities
===========================================================================*/

#if defined(MAX_DEBUG) || defined(SOCKET_DEBUG)

void util_$tab(level, fp)
register int level;
FILE* fp;
{
    level <<= 2;
    while (--level >= 0)
        putc(' ', fp);
}

#endif
