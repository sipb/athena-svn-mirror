/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: Macintosh, under MPW
 * (Cloned initially from conf-bsdsparc.h)
 */

#include "mit-copyright.h"
#include <Memory.h> 	/* needed in memcache.c for NewHandleSys and Size defs */
#include <Events.h>		/* Needed below for TickCount */

#define BITS32
#define BIG
#define MSBFIRST
#define	HOST_BYTE_ORDER	MSB_FIRST
#define BSDUNIX
#define MUSTALIGN
#define DES_SHIFT_SHIFT
#define	HAVE_GETENV

/* We'd like to 
#define MULTIDIMENSIONAL_ERR_TXT
 here.  We need it defined when building a Mac *driver* because Think
 C can't initialize a pointer to a string in that case.  But we don't
 have the room to use a big two-dimensional array when building a
 library that will be used to link lib/krb into a Mac *application*.
 So rather than define it here, it is set in the Think C "project
 file" (binary Makefile) KrbLib-project-A4, since that's the only
 thing that differs between those two cases.  */

/* Set the default ticket cache ``name'' string.  Pretty much unused
   for now, I think...  FIXME.  */
#define	TKT_FILE	"TKT_FILE"

/*
 * TIME_GMT_UNIXSEC returns the current time of day, in Greenwich Mean Time,
 * as its unsigned KRB_INT32 result.  The result is in seconds since 
 * January 1, 1970, as used in Unix.
 *
 * TIME_GMT_UNIXSEC_US does the above, and also returns the number of
 * microseconds that have passed in the current second, through 
 * the argument pointer *usecptr, which points to an unsigned KRB_INT32.
 */
 
#define	TIME_GMT_UNIXSEC	gettimeofdaynet_no_offset()
unsigned long gettimeofdaynet_no_offset();
#define	TIME_GMT_UNIXSEC_US(usecptr)  ((*(usecptr) = 0), gettimeofdaynet_no_offset())

/* On Unix these are from <sys/types.h>; we supply them by hand.  */
typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;

typedef void sigtype;	/* Signal handler functions are declared "noid".  */



/* 
 * The long value which is the difference between the current time
 * as handled by the system time conversion utilities, and the 
 * current time as transmitted by Kerberos (seconds since 1/1/70).
 *
 * E.g. the result of   (tickettime) - CONVERT_TIME_EPOCH   is suitable
 * to pass to localtime() or ctime().
 *
 * For Mac:  Change epoch from UNIX (1-1-1970) to Mac (1-1-1904),
 *	     offsetting by 66 years and 17 leap-days.
 */
#define	CONVERT_TIME_EPOCH	(-(long) \
	(66L * 365L * 24L * 60L * 60L  +  17L * 24L * 60L * 60L))



/* If this source file requires it, define struct sockaddr_in
   (and possibly other things related to network I/O).  */
#ifdef DEFINE_SOCKADDR
#include "macsock.h"		/* Our very own socket kludge implem. */

/* Some of our own infrastructure where the WinSock stuff was too hairy
   to dump into a clean Unix program...  */

#define	SOCKET_INITIALIZE()	(WSAStartup(0x0101, (WSADATA *)0))
#define	SOCKET_CLEANUP()	(WSACleanup())
#define	SOCKET_ERRNO		(WSAGetLastError())
#define	SOCKET_SET_ERRNO(x)	(WSASetLastError(x))
#define	SOCKET_NFDS(f)		(0)	/* select()'s first arg is ignored */
#define SOCKET_READ(fd, b, l)	(recv(fd, b, l, 0))
#define SOCKET_WRITE(fd, b, l)	(send(fd, b, l, 0))
#define SOCKET_EINTR		WSAEINTR
#endif

#ifdef NEED_TIME_H
#include <time.h>		/* For localtime, etc */
#endif


/* 
 * Return some "pretty" random values in KRB_INT32's.  This is not
 * very random -- FIXME install what old Mac Kerberos did.  Improving the
 * randomness would help a lot here.  Need to include something
 * that changes on each machine to avoid multiple machines using the
 * same random keystream.
 */
#include <Events.h>
#define	RANDOM_KRB_INT32_1	((KRB_INT32) (TickCount()))
#define	RANDOM_KRB_INT32_2	(0)  /* gethostid isn't that random anyway. */
/* Stub out gethostname()...   FIXME eventually by doing a DNS lookup on our
   IP address.  */
#define	GETHOSTNAME(h,l)	(-1)	/* Mac doesn't know its name */


/* 
 * Deal with FAR pointers needed in coping with Windows DLL's.
 * I really hate to add this crap Crap CRAP to a clean program!
 *
 * There ain't no such thing on real Unix machines, so this is easy.
 */
#define	FAR		/* no such thing in real machines */
#define	INTERFACE	/* No special declaration?? FIXME. */
#define	_fmemcpy	memcpy
#define	_fstrncpy	strncpy
#define	far_fputs	fputs
