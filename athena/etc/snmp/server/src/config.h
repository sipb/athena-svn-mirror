
/*
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/config.h,v 1.5 1990-07-15 17:55:44 tom Exp $
 *  $Author: tom $
 *  $Locker:  $
 *  $Log: not supported by cvs2svn $
 * Revision 1.4  90/05/26  13:42:02  tom
 * athena release 7.0e + patch 15
 * 
 * Revision 1.3  90/04/26  15:45:25  tom
 * defined SYS_DESCR to slinky- I'd rather have it be totally bogus
 * than subtly wrong.
 * 
 * disabled built in defines for platforms- makefile or compiler should
 * do this
 * 
 * 
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988
 */
/*
 *  this file contains all of the miscellaneous compile-time definitions
 *  for snmpd.
 */
/*
 *  OS related definitions.  Please pick the appropriate one for
 *  your system.
 */
/*
 *  If you defined "GATEWAY" when config'ing your UNIX kernel, then
 *  define this!  This way when SNMPD has to read the routing tables,
 *  it uses the right size!  Also, if you didn't use this option
 *  when config'ing your kernel, may I suggest you go back and
 *  reconfig with this option.  SNMPD uses this define for other
 *  gateway related stuff and so does unix.
 */

#ifdef MIT
/*
 * This is a bogus way to do it.
 */
#endif MIT

/*#define GATEWAY		/* gives larger routing tables */
/*#define SUN3_3PLUS		/* Sun OS 3.3 or greater, but < 4.0 */
/*#define BSD43			/* 4.3 BSD */
/*#define ULTRIX2_2		/* Ultrix 2.2 or 2.0 */
/*#define ULTRIX3               /* Ultrix 3.X */
/*#define SUN40			/* Sun OS 4.0 */

/*
 *  If you are running Van Jacobson's new TCP code, there are a lot
 *  more TCP stats kept in the kernel.  Certain TCP variables will
 *  be available with these mods, but NOT with the stock BSD kernel.
 *  #define VANJ_XTCP if you run with Van's stuff.
 */
#define VANJ_XTCP		/* we use Van Jacobsons new TCP code */

/*
 *  This define is for the maximum number of TCP connections this
 *  entity can handle.  The MIB states that if this number is dynamic,
 *  use a -1.  Since UNIX is dynamic, -1 will usually be used here.
 */
#define TCPMAXCONN	-1

/*
 *  standard definitions of where we want things to be.
 */
#define SNMPINITFILE	"/etc/snmpd.conf"
#define PIDFILE		"/etc/snmpd.pid"
#define VERSION		"3.4"

#ifdef  MIT
#ifdef  LOGIN
#define LOGIN_FILE      "/etc/utmp"
#endif  LOGIN
#endif  MIT

/*
 *  Well known port that snmpd will listen on to find out
 *  about various agent/daemon variables dynamically.
 *  Right now it is only gated.
 */
#define AGENT_PORT	167

/*
 *  maximum number of interfaces SNMPD can handle.
 */
#define MAXIFS		15

/*
 *  Version and Rev of this gateway.
 *  This will be used for the _mib_system_sysDescr variable if
 *  nothing is specified in the SNMPINITFILE.
 */

#define SYS_DESCR "Slinky"

/* #define SYS_DESCR "VAX 11/750" */
/* #define SYS_DESCR "VAX 3600"   */

/*
 *  This is the system Object identifier.  It is used with the
 *  mib_system_sysObjectID variable
 *  The first number is the length of your object ID.
 */
#define SYS_OBJ_ID {8, 1, 3, 6, 1, 4, 1, 20, 1 }

/*
 *  With SunOS 4.0, the BSD43 flag works fine, so make sure it is
 *  turned on.  Also, SunOS 4.0 includes Van J's TCP improvements.
 */
#ifdef MIT
/* 
 * Deal with suns at a later time
 */
#endif MIT

#if defined(SUN40) || defined(ULTRIX3)
#ifndef BSD43
#define BSD43
#endif
#ifndef VANJ_XTCP
#define VAXJ_XTCP
#endif
#endif

