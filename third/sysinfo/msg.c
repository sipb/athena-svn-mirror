/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Miscellaneous functions
 */

#include "defs.h"
#include <sys/stat.h>

/*
 * SysInfo Message functions
 *
 * SIM_INFO messages will include \n (newline) when needed by calling
 * function.  All other SIM_* will have newline provided by SImsg().
 */
#if	ARG_TYPE == ARG_STDARG
/*
 * StdArg version of SImsg()
 */
extern void SImsg(int MsgType, char *fmt, ...)
{
    va_list 			args;

    if (MsgType == 0 || MsgClassFlags == 0)
	/* No output */
	return;

    va_start(args, fmt);

    if (FLAGS_ON(MsgType, SIM_INFO) && FLAGS_ON(MsgClassFlags, SIM_INFO)) {
	/* Normal output */
	(void) vfprintf(stdout, fmt, args);
    } else if (FLAGS_ON(MsgType, SIM_WARN) &&
	       FLAGS_ON(MsgClassFlags, SIM_WARN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: WARNING: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_UNKN) &&
	       FLAGS_ON(MsgClassFlags, SIM_UNKN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: UNKNOWN: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_GERR) &&
	       FLAGS_ON(MsgClassFlags, SIM_GERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: GERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_CERR) &&
	       FLAGS_ON(MsgClassFlags, SIM_CERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: CERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_DBG) &&
	       FLAGS_ON(MsgClassFlags, SIM_DBG)) {
	(void) fflush(stdout);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    }

    va_end(args);
}
#endif	/* ARG_STDARG */
#if	ARG_TYPE == ARG_VARARGS
/*
 * Varargs version of SImsg()
 */
extern void SImsg(MsgType, fmt, va_alist)
     int			MsgType;
     char		       *fmt;
     va_dcl
{
    va_list 			args;

    if (MsgType == 0 || MsgClassFlags == 0)
	/* No output */
	return;

    va_start(args);

    if (FLAGS_ON(MsgType, SIM_INFO) && 
	FLAGS_ON(MsgClassFlags, SIM_INFO)) {
	(void) vfprintf(stdout, fmt, args);
    } else if (FLAGS_ON(MsgType, SIM_WARN) &&
	       FLAGS_ON(MsgClassFlags, SIM_WARN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: WARNING: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_UNKN) &&
	       FLAGS_ON(MsgClassFlags, SIM_UNKN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: UNKNOWN: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_GERR) &&
	       FLAGS_ON(MsgClassFlags, SIM_GERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: GERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_CERR) && 
	       FLAGS_ON(MsgClassFlags, SIM_CERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: CERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_DBG) &&
	       FLAGS_ON(MsgClassFlags, SIM_DBG)) {
	(void) fflush(stdout);
	(void) vfprintf(stderr, fmt, args);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    }
    va_end(args);
}
#endif	/* ARG_VARARGS */
#if	!defined(ARG_TYPE)
/*
 * Non-Varargs version of SImsg()
 */
extern void SImsg(MsgType, fmt, a1, a2, a3, a4, a5, a6)
     int			MsgType;
     char 		       *fmt;
{
    if (MsgType == 0 || MsgClassFlags == 0)
	/* No output */
	return;

    if (FLAGS_ON(MsgType, SIM_INFO) &&
	FLAGS_ON(MsgClassFlags, SIM_INFO)) {
	(void) vfprintf(stdout, fmt, a1, a2, a3, a4, a5, a6);
    } else if (FLAGS_ON(MsgType, SIM_WARN) &&
	       FLAGS_ON(MsgClassFlags, SIM_WARN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: WARNING: ", ProgramName);
	(void) vfprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_UNKN) &&
	       FLAGS_ON(MsgClassFlags, SIM_UNKN)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: UNKNOWN: ", ProgramName);
	(void) vfprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_GERR) &&
	       FLAGS_ON(MsgClassFlags, SIM_GERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: GERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_CERR) &&
	       FLAGS_ON(MsgClassFlags, SIM_CERR)) {
	(void) fflush(stdout);
	if (FLAGS_OFF(MsgType, SIM_NOLBL))
	    (void) fprintf(stderr, "%s: CERROR: ", ProgramName);
	(void) vfprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    } else if (FLAGS_ON(MsgType, SIM_DBG) &&
	       FLAGS_ON(MsgClassFlags, SIM_DBG)) {
	(void) fflush(stdout);
	(void) vfprintf(stderr, fmt, a1, a2, a3, a4, a5, a6);
	if (FLAGS_OFF(MsgType, SIM_NONL))
	    (void) fprintf(stderr, "\n");
    }
}
#endif 	/* !ARG_TYPE */
