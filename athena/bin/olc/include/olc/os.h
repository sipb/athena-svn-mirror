/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/os.h,v $
 * $Id: os.h,v 1.1 1990-04-27 12:03:28 vanharen Exp $
 */

#ifndef __olc_os_h
#define __olc_os_h

#if is_cplusplus
#include <std.h>
#else
/* Not C++ */

#if __STDC__

#ifdef S_IFMT
int fstat (int, struct stat *);
#endif

extern int errno;
int open (const char *, int, ...);
int unlink (const char *);
void perror (const char *);
int rename (const char *, const char *);
int close (int);
void free (void *);
/* etc */

#ifdef FILE
int fclose (FILE *);
int fflush (FILE *);
int fputs (const char *, FILE *);
/* _flsbuf */
#endif

#else  /* __STDC__ */

void perror();

#endif /* __STDC__ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#ifndef DIRSIZ
#include <sys/dir.h>
#endif
/***  This is causing problems at compile time...
      We need to either dump it from here, or re-arrange a large
      number of source files so that it won't clash...
  #ifndef S_IFMT
  #include <sys/stat.h>
  #endif
***/
#include <sys/resource.h>
/*#include "/mit/gnu/vaxlib/syscalls.c.P"*/

#endif /* not C++ */

#if is_cplusplus || __STDC__
#if is_cplusplus
extern "C" {
#endif
    /* C library stuff */
    extern void *malloc (unsigned int), *realloc (void *, unsigned int);
    extern int	socket (int, int, int);
#ifdef SOCK_STREAM
    extern int	connect (int, struct sockaddr *, int);
    extern int	bind (int, /* struct sockaddr * */ void *, int);
    extern int	accept (int, struct sockaddr *, int *);
#endif
#ifdef UIO_USERSPACE
    extern int	writev (int, struct iovec *, int);
#endif
#ifndef mips
    extern void	setlinebuf (FILE *);
#else /* sigh */
    extern int setlinebuf (FILE *);
#endif
    extern int	setsockopt (int, int, int, void *, int);
    extern int	setenv (const char *, const char *, int);
    extern int	listen (int, int);
    extern /*int*/ gettimeofday (struct timeval *, struct timezone *);

    /* man page uses varargs.h, but ... */
    /*#include <stdarg.h>*/
    extern int	_doprnt (const char *, /*va_list*/void *, FILE *);
#if is_cplusplus
};
#endif
#endif

#endif /* __olc_os_h */
