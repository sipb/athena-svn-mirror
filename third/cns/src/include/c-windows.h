/*
 * c-windows.h
 *
 * Machine-type definitions: PC running Windows.
 */

#include "mit-copyright.h"
#include <time.h>

#undef BSDUNIX
#undef DES_SHIFT_SHIFT

/* Set default ticket file ``name'' for interim support.  FIXME!  */

#define TKT_FILE        "c:\\kerberos\\ticket.ses"

typedef void sigtype;	/* Signal handler functions are declared "void".  */

/*
 * conf-pc.h
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Machine-type definitions: IBM PC 8086
 */

#define BITS16
#undef CROSSMSDOS	/* read_password.c bites without this FIXME */
#define LSBFIRST
#define	HOST_BYTE_ORDER	LSB_FIRST

#if defined(__WINDOWS__) && !defined(WINDOWS)
#define WINDOWS
#endif

/* Force prototypes even if __STDC__ not defined */
#define PROTOTYPE(p) p

/* These used to be from <sys/types.h> but now osconf.h supplies them (thru us). */
typedef unsigned char	u_char;
typedef unsigned short	u_short;
typedef unsigned int	u_int;
typedef unsigned long	u_long;

/* 
 * Windows Sockets definitions, as well as a few other include files
 * that only a few modules want included.
 * Also, our own infrastructure for accessing sockets.
 */
#ifdef	DEFINE_SOCKADDR
#include "winsock.h"
#define	MAXHOSTNAMELEN	512	/* Why be stingy? */
#define	GETHOSTNAME(h,l)	(-1)	/* No hostname for now FIXME?
		    (We *could* get our internet addr and gethostbyaddr.) */
#include <time.h>

/* Some of our own infrastructure where the WinSock stuff was too hairy
   to dump into a clean Unix program...  */

#define	SOCKET_INITIALIZE()	win_socket_initialize()
#define	SOCKET_CLEANUP()	WSACleanup()
#define	SOCKET_ERRNO		(WSAGetLastError())
#define	SOCKET_SET_ERRNO(x)	(WSASetLastError (x))
#define	SOCKET_NFDS(f)		(0)	/* select()'s first arg is ignored */
#define SOCKET_READ(fd, b, l)	(recv(fd, b, l, 0))
#define SOCKET_WRITE(fd, b, l)	(send(fd, b, l, 0))
#define SOCKET_EINTR		WSAEINTR

int win_socket_initialize();
#endif	/* DEFINE_SOCKADDR */


typedef void sigtype;	/* Signal handler functions are declared "noid".  */

/* Used by email/POP/pop_log.c and pop_msg.c */
#define	HAVE_VSPRINTF	1

/* Used in various places, appl/bsd, email/POP, kadmin, lib/krb */
#define	USE_UNISTD_H	1

/*
 * TIME_GMT_UNIXSEC returns the current time of day, in Greenwich Mean Time,
 * as its unsigned KRB_INT32 result.  The result is in seconds since 
 * January 1, 1970, as used in Unix.
 *
 * TIME_GMT_UNIXSEC_US does the above, and also returns the number of
 * microseconds that have passed in the current second, through 
 * the argument pointer *usecptr, which points to an unsigned KRB_INT32.
 */
 
/* Unfortunately, KRB_INT32 isn't defined here yet.  Push the declaration
   into a macro, which krb.h will expand as needed at an appropriate point.  */
#define	DECL_THAT_NEEDS_KRB_INT32	\
	extern	unsigned KRB_INT32	win_time_gmt_unixsec \
					PROTOTYPE ((unsigned KRB_INT32 *));

#define	TIME_GMT_UNIXSEC	win_time_gmt_unixsec((unsigned KRB_INT32 *)0)
#define	TIME_GMT_UNIXSEC_US(us)	win_time_gmt_unixsec((us))


/* 
 * Return some "pretty" random values in KRB_INT32's.  This is not
 * very random!  Improving the
 * randomness would help a lot here.  Need to include something
 * that changes on each machine to avoid multiple machines using the
 * same random keystream.
 */
#define	RANDOM_KRB_INT32_1	((KRB_INT32) 0) 	/* FIXME!! */
#define	RANDOM_KRB_INT32_2	((KRB_INT32) 0)		/* FIXME !! */


/* 
 * The long value which is the difference between the current time
 * as handled by the system time conversion utilities, and the 
 * current time as transmitted by Kerberos (seconds since 1/1/70).
 *
 * E.g. the result of   (tickettime) - CONVERT_TIME_EPOCH   is suitable
 * to pass to localtime() or ctime().
 */
#define	CONVERT_TIME_EPOCH	win_time_get_epoch()
extern long win_time_get_epoch();


/* 
 * Deal with FUCKED pointers (aka FAR pointers).
 * We keep a list here of what FAR functions we use; since real machines
 * have to #define them away in THEIR config files, we have them here too.
 *
 * Unbelievably, Microloss never released a way to print FAR strings,
 * except a qualifier for printf (and you can't change every printf format
 * string with a macro!).  So we supply our own such function, far_fputs.
 * It doesn't declare its prototype since FILE may not be defined yet.
 *
 * Windows DLL interface routines are not *documented* to require Pascal
 * calling sequences.  But they will not link properly unless you actually
 * *use* Pascal naming conventions.  Without, you must link by numeric
 * entry point numbers in kerberos.def, and even so, Visual Basic will not
 * be able to find the names of routines in the DLL.  My guess is that
 * they are unintentionally assuming that all names in a DLL are uppercase.
 */
#ifndef FAR
#define	FAR	__far
#endif
#define	INTERFACE	FAR __export __pascal	/* Windows DLL interface */
/* #define	_fmemcpy		 */
/* #define	_fstrncpy		*/
int	far_fputs (/* char FAR *string, FILE *stream */);


/* 
 * Fake up a few values from the Macintosh world since lib/krb/memcache.h is
 * still written in Mac fashion.
 */
#define	OSErr int
#define	noErr 0
#define memFullErr -108


/* Kerberos changed window message */
#define WM_KERBEROS_CHANGED "Kerberos Changed"

/* Kerberos Windows initialization file */
#define KERBEROS_INI "kerberos.ini"
#define KERBEROS_HLP "kerberos.hlp"
#define INI_DEFAULTS "Defaults"
	#define INI_USER "User"			/* Default user */
	#define INI_INSTANCE "Instance"		/* Default instance */
	#define INI_REALM "Realm"		/* Default realm */
	#define INI_POSITION "Position"
#define INI_OPTIONS "Options"
	#define INI_DURATION "Duration"		/* Ticket duration in minutes */
#define INI_EXPIRATION "Expiration"		/* Action on expiration (alert or beep) */
	#define INI_ALERT "Alert"
	#define INI_BEEP "Beep"
#define INI_FILES "Files"
	#define INI_KRB_CONF "krb.conf"		/* Location of krb.conf file */
	#define DEF_KRB_CONF "krb.con"		/* Default name for krb.conf file */
	#define INI_KRB_REALMS "krb.realms"	/* Location of krb.realms file */
	#define DEF_KRB_REALMS "krb.rea"	/* Default name for krb.realms file */
#define INI_RECENT_LOGINS "Recent Logins"
	#define INI_LOGIN "Login"
