/*
 * Configuration file for VMS Kerberos hosts.
 *
 * Handles definitions specific to Berkeley Sockets as provided by 
 * MULTINET under VMS.
 *
 * Created by John Gilmore, Cygnus Support.
 * Modified by Mark Eichin, Cygnus Support.
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
/* It appears that the multinet include path won't let us include
   netinet/in.h directly, though we never figured out why. --eichin */
#include "multinet_root:[multinet.include.netinet]in.h"	/* For struct sockaddr_in and in_addr */
#include "multinet_root:[multinet.include.arpa]inet.h" /* For inet_ntoa */
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

extern long gethostid();

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
#define SOCKET_READ		socket_read  /* this assumes Multinet, as do other things */
#define SOCKET_WRITE		socket_write
#define SOCKET_EINTR		EINTR


/* 
 * Deal with FAR pointers needed in coping with Windows DLL's.
 * I really hate to add this crap Crap CRAP to a clean program!
 *
 * There ain't no such thing on real Unix machines, so this is easy.
 */
#define	FAR		/* no such thing in real machines */
#define	_fmemcpy	memcpy
#define	_fstrncpy	strncpy
#define	far_fputs	fputs
