/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lp.h,v $
 *	$Author: probe $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lp.h,v 1.12 1993-10-09 18:14:33 probe Exp $
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)lp.h	5.1 (Berkeley) 6/6/85
 */

/*
 * Global definitions for the line printer system.
 */

#ifdef POSIX
#include <unistd.h>
#endif
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#ifdef POSIX
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pwd.h>
#include <syslog.h>
#include <signal.h>
#ifdef _AIX
#undef _BSD
#endif
#include <sys/wait.h>
#ifdef _AIX
#define _BSD 44
#include <sys/ioctl.h>
#endif
#ifdef POSIX
#include <termios.h>
#else
#include <sgtty.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <strings.h>
#include "lp.local.h"

#ifdef SOLARIS
/*
 * flock operations.
 */
#define LOCK_SH               1       /* shared lock */
#define LOCK_EX               2       /* exclusive lock */
#define LOCK_NB               4       /* don't block when locking */
#define LOCK_UN               8       /* unlock */
#endif

#ifdef KERBEROS
#include <krb.h>
#define KLPR_SERVICE "rcmd"
#endif KERBEROS

extern int	DU;		/* daeomon user-id */
extern int	MX;		/* maximum number of blocks to copy */
extern int	MC;		/* maximum number of copies allowed */
extern char	*LP;		/* line printer device name */
extern char	*RM;		/* remote machine name */
extern char	*RG;		/* restricted group */
extern char	*RP;		/* remote printer name */
extern char	*LO;		/* lock file name */
extern char	*ST;		/* status file name */
extern char	*SD;		/* spool directory */
extern char	*AF;		/* accounting file */
extern char	*LF;		/* log file for error messages */
extern char	*OF;		/* name of output filter (created once) */
extern char	*IF;		/* name of input filter (created per job) */
extern char	*RF;		/* name of fortran text filter (per job) */
extern char	*TF;		/* name of troff(1) filter (per job) */
extern char	*NF;		/* name of ditroff(1) filter (per job) */
extern char	*DF;		/* name of tex filter (per job) */
extern char	*GF;		/* name of graph(1G) filter (per job) */
extern char	*VF;		/* name of raster filter (per job) */
extern char	*CF;		/* name of cifplot filter (per job) */
extern char	*FF;		/* form feed string */
extern char	*TR;		/* trailer string to be output when Q empties */
extern short	SC;		/* suppress multiple copies */
extern short	SF;		/* suppress FF on each print job */
extern short	SH;		/* suppress header page */
extern short	SB;		/* short banner instead of normal header */
extern short	HL;		/* print header last */
extern short	RW;		/* open LP for reading and writing */
extern short	PW;		/* page width */
extern short	PX;		/* page width in pixels */
extern short	PY;		/* page length in pixels */
extern short	PL;		/* page length */
extern short	BR;		/* baud rate if lp is a tty */
extern int	FC;		/* flags to clear if lp is a tty */
extern int	FS;		/* flags to set if lp is a tty */
extern int	XC;		/* flags to clear for local mode */
extern int	XS;		/* flags to set for local mode */
extern short	RS;		/* restricted to those with local accounts */
#ifdef PQUOTA
extern char     *RQ;            /* Name of remote quota server */
extern int      CP;	        /* Cost per page */
extern char 	*QS;		/* Quota service for printer */
#endif /* PQUOTA */
#ifdef LACL
extern char	*AC;		/* Local ACL file to use */
extern short	PA;		/* ACL file used as positive ACL */
extern short	RA;		/* Restricted host access */
#endif /* LACL */

extern char	line[BUFSIZ];
extern char	pbuf[];		/* buffer for printcap entry */
extern char	*bp;		/* pointer into ebuf for pgetent() */
extern char	*name;		/* program name */
extern char	*printer;	/* printer name */
extern char	host[32];	/* host machine name */
extern char	*from;		/* client's machine name */
extern int	errno;
#ifdef HESIOD
extern char	alibuf[BUFSIZ/2]; /* buffer for printer alias */
#endif

#ifdef KERBEROS
extern int      use_kerberos;
extern int      kerberos_cf;
extern char     kprincipal[];
extern char     kinstance[];
extern char     krealm[];
#endif KERBEROS

/*
 * Structure used for building a sorted list of control files.
 */
struct queue_ {
	time_t	q_time;			/* modification time */
	char	q_name[MAXNAMLEN+1];	/* control file name */
};

char	*pgetstr();
char	*malloc();
char	*getenv();

#define UNLINK unlink
