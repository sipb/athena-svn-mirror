/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/include/olc/os.h,v $
 *	$Id: os.h,v 1.6 1991-04-08 21:01:33 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef HAS_ANSI_INCLUDES
#include <mit-copyright.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#ifndef DIRSIZ
#include <sys/dir.h>
#endif
#include <sys/resource.h>

#ifndef __olc_os_h
#define __olc_os_h

#ifdef __STDC__
# define	P(s) s
#else
# define P(s) ()
#endif

#ifdef S_IFMT
int fstat P((int, struct stat *));
#endif

extern int errno;
int open P((const char *, int, ...));
int unlink P((const char *));
int rename P((const char *, const char *));
int close P((int));
void free P((void *));

#ifdef FILE
int fclose P((FILE *));
int fflush P((FILE *));
int fputs P((const char *, FILE *));
#endif

char *ttyname P((int filedes ));
char *getenv P((char *name ));
void *malloc P((unsigned int));
void *realloc P((void *, unsigned int));
void *calloc P((unsigned int, unsigned int));
int socket P((int, int, int));

#ifdef SOCK_STREAM
int connect P((int, struct sockaddr *, int));
int bind P((int, /* struct sockaddr * */ void *, int));
int accept P((int, struct sockaddr *, int *));
#endif /* SOCK_STREAM */

#ifdef UIO_USERSPACE
int writev P((int, struct iovec *, int));
#endif /* UIO_USERSPACE */

#ifndef mips
void setlinebuf P((FILE *));
#else /* sigh */
int setlinebuf P((FILE *));
#endif /* mips */

int setsockopt P((int, int, int, void *, int));
int setenv P((const char *, const char *, int));
int listen P((int, int));
int select P((int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfs,
	      struct timeval *timeout));
int gettimeofday P((struct timeval *, struct timezone *));

int fflush P((FILE *stream));

/* man page uses varargs.h, but ... */
/*#include <stdarg.h>*/
int _doprnt P((const char *, /*va_list*/void *, FILE *));

#undef P
#endif /* __olc_os_h */

#endif
