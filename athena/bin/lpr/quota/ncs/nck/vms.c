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
 * V M S 
 *
 * These functions are simulated BSD library and system calls which are not
 * supported in VMS C. 
 */

#include "std.h"

/*------------------------------------------------------------*/

int ffs(bitmap)
    int bitmap;
{
    int temp, n, bm;

    bm = bitmap;
    if (bm == (int) 0)
        return (-1);

    for (n = 0; n >= 31 || !(bm & 1); n++)
        bm >>= 1;

    return (n);
}

/*------------------------------------------------------------*/

int _findchan(fds)
    fd_set *fds;
{
    int i;
    int chan;

    for (i = 0; i < 64; i++) {
        if (fds->fds_bits[i]) {
            chan = i * NFDBITS + ffs(fds->fds_bits[i]);
            return (chan);
        }
    }
    return 0;
}

/*------------------------------------------------------------*/

void gettimeofday(tp, tzp)
    struct timeval *tp;
    struct timezone *tzp;
{
    extern long timezone;
    extern int daylight;

    struct {
        time_t time;
        unsigned short millitm;
        short timezone;
        short dstflg;
    } timeb;

    ftime(&timeb);
    tp->tv_sec = timeb.time;
    tp->tv_usec = timeb.millitm;

    tzp->tz_minuteswest = timezone / 60;
    tzp->tz_dsttime = daylight;
}
