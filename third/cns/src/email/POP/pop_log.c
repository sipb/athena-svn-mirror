/*
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char copyright[] = "Copyright (c) 1990 Regents of the University of California.\nAll rights reserved.\n";
static char SccsId[] = "@(#)pop_log.c   1.7 7/13/90";
#endif /* not lint */

#include <stdio.h>
#include "popper.h"

/* see util/et/com_err.c for the code this is based on -- use stdarg
 * by default, varargs only if we have to.
 */
#ifdef __GNUC__
#undef VARARGS
#endif

#include <sys/types.h>
#if defined(__STDC__) && !defined(VARARGS)
#include <stdarg.h>
#else
#ifndef VARARGS
#define VARARGS
#endif
#include <varargs.h>
#endif

/* 
 *  log:    Make a log entry
 */

static char msgbuf[MAXLINELEN];

#ifndef VARARGS
pop_log(POP* p, int stat, char *format, ...)
{
#else
pop_log(va_alist)
va_dcl
{
    POP     *   p;
    int         stat;
    char    *   format;
#endif
    va_list     ap;

#ifdef VARARGS
    va_start(ap);
    p = va_arg(ap,POP *);
    stat = va_arg(ap,int);
    format = va_arg(ap,char *);
#else
    va_start(ap, format);
#endif
#ifdef HAVE_VSPRINTF
        vsprintf(msgbuf,format,ap);
#else
        (void)sprintf (msgbuf,format,((int *)ap)[0],((int *)ap)[1],((int *)ap)[2],
                ((int *)ap)[3],((int *)ap)[4],((int *)ap)[5]);
#endif /* HAVE_VSPRINTF */
    va_end(ap);

    if (p->debug && p->trace) {
        (void)fprintf(p->trace,"%s\n",msgbuf);
        (void)fflush(p->trace);
    }
    else {
        syslog (stat,"%s",msgbuf);
    }

    return(stat);
}
