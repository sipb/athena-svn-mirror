/*
 * Configuration file for Unix Kerberos hosts.
 *
 * Handles definitions specific to Berkeley Sockets on Unix.
 *
 * Created by John Gilmore, Cygnus Support.
 * Copyright 1994 Cygnus Support.
 * 
 * Permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Cygnus Support makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

/* Define u_char, u_short, u_int, and u_long. */
#include <sys/types.h>

/* If this source file requires it, define struct sockaddr_in
   (and possibly other things related to network I/O).  FIXME.  */
#ifdef DEFINE_SOCKADDR
#include <netinet/in.h>		/* For struct sockaddr_in and in_addr */
#include <arpa/inet.h>		/* For inet_ntoa */
#include <netdb.h>		/* For struct hostent, gethostbyname, etc */
#include <sys/param.h>		/* For MAXHOSTNAMELEN */
#include <sys/socket.h>		/* For SOCK_*, AF_*, etc */
#include <sys/time.h>		/* For struct timeval */
#ifdef NEED_TIME_H
#include <time.h>		/* For localtime, etc */
#endif
#endif

/*
 * TIME_GMT_UNIXSEC returns the current time of day, in Greenwich Mean Time,
 * as its unsigned KRB_INT32 result.  The result is in seconds since 
 * January 1, 1970, as used in Unix.
 *
 * TIME_GMT_UNIXSEC_US does the above, and also returns the number of
 * microseconds that have passed in the current second, through 
 * the argument pointer *us, which points to an unsigned KRB_INT32.
 */
 
/* Unfortunately, KRB_INT32 isn't defined here yet.  Push the declaration
   into a macro, which krb.h will expand as needed at an appropriate point.  */
#define	DECL_THAT_NEEDS_KRB_INT32	\
	extern	unsigned KRB_INT32	unix_time_gmt_unixsec \
					PROTOTYPE ((unsigned KRB_INT32 *));

#define	TIME_GMT_UNIXSEC	unix_time_gmt_unixsec((unsigned KRB_INT32 *)0)
#define	TIME_GMT_UNIXSEC_US(us)	unix_time_gmt_unixsec((us))
#define	CONVERT_TIME_EPOCH	((long)0)	/* Unix epoch is Krb epoch */



/* 
 * Return some "pretty" random values in KRB_INT32's.  This is not
 * very random but is what the old Kerberos did.  Improving the
 * randomness would help a lot here.  Need to include something
 * that changes on each server to avoid multiple servers using the
 * same random keystream.
 */
#define	RANDOM_KRB_INT32_1	((KRB_INT32) getpid())

#ifdef	NO_GETHOSTID
#define	RANDOM_KRB_INT32_2	(0)  /* gethostid isn't that random anyway. */
#else
#define	RANDOM_KRB_INT32_2	((KRB_INT32) gethostid())
#endif

/* extern int getpid();	 -- this will default, and some systems explicitly
				declare pid_t rather than int (sigh).  */

#if 0 /* OSF/1 3.0 provides prototype that conflicts; do any machines
	 actually lose by omitting this?  */
extern long gethostid();
#endif

/*
 * Compatability with WinSock calls on MS-Windows...
 */
#define	SOCKET		unsigned int
#define	INVALID_SOCKET	((SOCKET)~0)
#define	closesocket	close
#define	ioctlsocket	ioctl
#define	SOCKET_ERROR	(-1)

/* Some of our own infrastructure where the WinSock stuff was too hairy
   to dump into a clean Unix program...  */

#define	SOCKET_INITIALIZE()	(0)	/* No error (or anything else) */
#define	SOCKET_CLEANUP()	/* nothing */
#define	SOCKET_ERRNO		errno
#define	SOCKET_SET_ERRNO(x)	(errno = (x))
#define SOCKET_NFDS(f)		((f)+1)	/* select() arg for a single fd */
#define SOCKET_READ		read
#define SOCKET_WRITE		write
#define SOCKET_EINTR		EINTR

/* 
 * Deal with FAR pointers needed in coping with Windows DLL's.
 * I really hate to add this crap Crap CRAP to a clean program!
 *
 * There ain't no such thing on real Unix machines, so this is easy.
 */
#define	FAR		/* no such thing in real machines */
#define	INTERFACE	/* No special declaration needed */
#define	_fmemcpy	memcpy
#define	_fstrncpy	strncpy
#define	far_fputs	fputs

/*
 * Defined to use the global error handling scheme of util/et
 */
#define GLOBAL_ERROR_HANDLING

/* Grumble. */
#define GETHOSTNAME(h,l)	gethostname(h,l)
/*
 * Some Unixes don't declare errno in <errno.h>...
 * Move this out to individual c-*.h files if it becomes troublesome.
 */
extern int errno;
