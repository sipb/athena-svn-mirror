/***************************************************************************
 * LPRng - An Extended Print Spooler System
 *
 * Copyright 1988-2000, Patrick Powell, San Diego, CA
 *     papowell@astart.com
 * See LICENSE for conditions of use.
 * $Id: proctitle.h,v 1.1.1.3 2000-03-31 15:48:06 mwhitson Exp $
 ***************************************************************************/



#ifndef _PROCTITLE_H_
#define _PROCTITLE_H_ 1


void initsetproctitle(int argc, char *argv[], char *envp[]);
/* VARARGS3 */
#if !defined(HAVE_SETPROCTITLE) || !defined(HAVE_SETPROCTITLE_DEF)
#  ifdef HAVE_STDARGS
void setproctitle( const char *fmt, ... );
void proctitle( const char *fmt, ... );
#  else
void setproctitle();
void proctitle();
#  endif
#endif

/* PROTOTYPES */

#endif
