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
 * S Y S 5
 *
 * ATT System V Unix compatibility routines.
 */

#include "std.h"

#include "nbase.h"
#include "pfm.h"

#ifndef SYS5_HAS_SELECT
#  undef select
#  undef recvfrom
#endif


#ifndef SYS5_HAS_GETTIMEOFDAY
/*
 * G E T T I M E O F D A Y
 *
 * Return system time is BSD format.  On return from this procedure the
 * timeval structure will have the seconds since 1970 filled in System
 * V does not track microseconds. The timezone structure is in from System
 * V global variables.
 */

void gettimeofday(tp, tzp)
struct timeval *tp;
struct timezone *tzp;
{
    extern long timezone;
    extern int daylight;

    tp->tv_sec = time((long) 0);
    tp->tv_usec = (long) 0;

    tzp->tz_minuteswest = timezone/60;
    tzp->tz_dsttime = daylight;
}
#endif


#ifndef SYS5_HAS_SELECT

/*
 * F F S
 *
 * Finds the first bit set in the bitmap and returns the index of the bit.
 * Bits are numbered starting at one from the right.  Returns 0 if the
 * bitmap is zero.
 */

int ffs(bm)
int bm;
{
    int n = 0;

    if (bm == 0)
        return(0);

    while ((bm & 1) == 0) {
        n++;
        bm >>= 1;
    }

    return(n + 1);
}


/*
 * A L A R M _ H A N D L E R
 *
 * SIGALRM handler.  Just arrange for another alarm to happen.
 */

static void alarm_handler()
{
    signal(SIGALRM, alarm_handler);
    alarm(1);
}


/*
 * S E L E C T _ N C K
 *
 * Less than completely faithful simulation of BSD select(2).
 */

static struct {
    u_char cached_read;         /* T => select rcvd & stored some data */
    u_char recv_in_progress;    /* T => someone already in select */
    int s;                      /* socket file descriptor that cached data was read from */
    int from_len;               /* length of above */
    struct sockaddr from;       /* sockaddr that cached data was from */
    int count;                  /* size of cached data */
    char buf[1024];             /* cached data */
} sel;

int select_nck(nfds, readfds, writefds, exceptfds, timeout)
int nfds;
struct fd_set *readfds, *writefds, *exceptfds;
struct timeval *timeout;
{
    int s, oflags, nflags, rv, set_alarm;
    long now;
    pfm_$cleanup_rec crec;
    status_$t fst, st;
    extern int errno;
    void (*old_alarm_handler)();
    int faulted;

    assert(writefds->fds_bits[0] == 0 && exceptfds->fds_bits[0] == 0);

    /*
     * Find socket to read on and make sure our caller isn't trying to
     * exceed our weak abilities.
     */

    if ((s = ffs(*readfds)) == 0)
        return (0);

    s--;

    FD_CLR(s, readfds);

    assert(readfds->fds_bits[0] == 0);

    /*
     * This code is not re-entrant.  As a result, if there's a 
     * recv in progress, then this better be a "polling select" (i.e.
     * one with a wait time of zero).  We long for "test and set" here
     * but our desire being unrequited, we leave a small window open.
     */

    if (sel.recv_in_progress) {
        assert(timeout->tv_sec == 0 && timeout->tv_usec == 0);
        return (0);
    }

    sel.recv_in_progress = 1;

    /*
     * Is there data already there?  If so it better be on the same socket
     * as the one we want now.
     */

    if (sel.cached_read) {
        assert(s == sel.s);
        sel.recv_in_progress = 0;
        return (1);
    }

    /*
     * Make sure we're not in no-delay mode on the file descriptor.
     */

    oflags = fcntl(s, F_GETFL);
    nflags = oflags & ~O_NDELAY;

    faulted = 0;

    fst = pfm_$cleanup(crec);
    if (fst.all != pfm_$cleanup_set) {
        faulted = 1;
        goto CLEANUP;
    }

    fcntl(s, F_SETFL, nflags);

    /*
     * If no alarm signal handler is set, set one now and arrange for there
     * to be an alarm every second.
     */

#ifdef SYS5_PRE_R3
    old_alarm_handler = signal(SIGALRM, SIG_IGN);
#else
    old_alarm_handler = signal(SIGALRM, SIG_HOLD);
#endif

    set_alarm = (int) old_alarm_handler < 10;

    if (set_alarm) {
        signal(SIGALRM, alarm_handler);
        alarm(1);
    }
    else 
        signal(SIGALRM, old_alarm_handler);

    now = time(0);   

    /*
     * Loop until we get data, or until we get an error.  If the error is
     * EINTR (interrupted system call) and we've waited long enough, then
     * return.       
     */ 

    while (1) {
        sel.from_len = sizeof sel.from;
        sel.count = recvfrom(s, sel.buf, sizeof sel.buf, 0, &sel.from, &sel.from_len);

        if (sel.count >= 0) {
            rv = 1;
            break;
        }

        if (errno != EINTR) {
            rv = -1;
            break;
        }

        if (timeout != 0 && time(0) - now > timeout->tv_sec) {
            rv = 0;
            break;
        }
    }

    if (rv == 1) {
        FD_SET(s, readfds);
        sel.s = s;
        sel.cached_read = 1;
#ifdef SYS5_SOCKETS_TYPE1
        sel.from.sa_family = 2;         /* !!! GET AROUND IP BUG !!! */
#endif
    }

CLEANUP:

    sel.recv_in_progress = 0;
    fcntl(s, F_SETFL, oflags);

    if (set_alarm) {
        alarm(0);
        signal(SIGALRM, old_alarm_handler);
    }

    if (faulted)
        pfm_$signal(fst);

    pfm_$rls_cleanup(crec, st);

    return (rv);
}


/*
 * R E C V F R O M _ N C K
 *
 * Simulation of BSD recvfrom(2) that works in "cooperation" with select(2)
 * simulation (above).
 */ 

int recvfrom_nck(s, buf, len, flags, from, fromlen)
int s;
char *buf;
int len, flags;
struct sockaddr *from;
int *fromlen;
{
    if (sel.cached_read) {
        sel.cached_read = 0;
        assert(*fromlen >= sel.from_len);
        memcpy(from, &sel.from, sel.from_len);
        *fromlen = sel.from_len;
        assert(len >= sel.count);
        memcpy(buf, sel.buf, sel.count);
        return (sel.count);
    }

    return(recvfrom(s, buf, len, flags, from, fromlen));
}

#endif


#ifdef SYS5_SOCKETS_TYPE2

/*
 * G E T H O S T N A M E
 *
 */

int gethostname(name, namelen)
char *name;
int namelen;
{
    strcpy(name, "ridge");
    return (0);
}

#endif
