/*
 *	rshd.c
 */

/*
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)rshd.c	5.12 (Berkeley) 9/12/88";
#endif /* not lint */

/*
 * remote shell server:
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 */
#include "conf.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif
#ifdef POSIX
#include <stdlib.h>
#else
extern char *malloc();
#endif
#ifdef __SCO__
#include <sys/unistd.h>
#endif

#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>

extern char *getenv ();

#include "loginpaths.h"

#ifdef hpux
/* has no killpg... */
#define killpg(pid, sig) kill(-(pid), (sig))
#endif

#ifdef __svr4__
#ifndef NO_SYSIO
/* get FIONBIO from sys/filio.h, so what if it is a compatibility feature */
#include <sys/filio.h>
#endif
#define krb_setpgrp(a,b) setpgrp()
#define getpgrp(a) getpgid(a)
/* has no killpg... */
#define killpg(pid, sig) kill(-(pid), (sig))
#endif

#if defined (linux) || defined (__osf__)
#define krb_setpgrp(a,b) setpgid(a,b) 
#endif

#ifndef krb_setpgrp
#define krb_setpgrp(a,b) setpgrp(a,b)
#endif

#ifdef __SCO__
/* sco has getgroups and setgroups but no initgroups */
int initgroups(char* name, gid_t basegid) {
  gid_t others[NGROUPS_MAX+1];
  int ngrps;

  others[0] = basegid;
  ngrps = getgroups(NGROUPS_MAX, others+1);
  return setgroups(ngrps+1, others);
}
#endif

#ifdef KERBEROS
#include <krb.h>
#include <kstream.h>

int ksh = 0, eksh = 0;
kstream kstr = 0, kstr2 = 0;
int anyport = 0;
Key_schedule schedule;
char *srvtab = "";
char *kprogdir = KPROGDIR;
#ifndef HAVE_STRSAVE
static char *strsave();
#endif
#endif /* KERBEROS */

#ifdef ATHENA
#define DEFAULT "default"     /* account name for default network usage */
#endif /* ATHENA */

#ifndef errno
extern int	errno;
#endif
/*VARARGS1*/
int	error();

/*ARGSUSED*/
main(argc, argv)
	int argc;
	char **argv;
{
#ifdef KERBEROS
	int opt;
	extern char *optarg;
#endif
#ifdef SO_LINGER
	struct linger linger;
#endif
	int on = 1, fromlen;
	struct sockaddr_in from;
	int port = -1;

#ifdef KERBEROS
	if (!strcmp (*argv, "kshd"))
	  ksh = 1;
	else if (!strcmp (*argv, "ekshd"))
	  ksh = eksh = 1;
#endif /* KERBEROS */
#ifndef LOG_ODELAY /* 4.2 syslog */
	openlog("rsh", LOG_PID);
#else
	openlog("rsh", LOG_PID | LOG_ODELAY, LOG_DAEMON);
#endif /* 4.2 syslog */

#ifdef KERBEROS
	/* Accept options.
	   -a
	   	Allocate any port for stderr, not a reserved port.
		This will cause old versions of rsh to fail, because
		they explicitly check that the port is reserved.
	   -k
	   	Do Kerberos authentication as though invoked as kshd.
#ifndef NOENCRYPTION
	   -x
	   	Do encryption as though invoked as ekshd.
#endif
	   -p port
	        Accept a connection on the given port.  Normally
		rshd assumes that it was invoked by inetd.
	   -P directory
	        Set the directory in which to find Kerberos programs.
		This directory is placed at the start of PATH.  The
		default is KPROGDIR from Makefile.in, normally
		/usr/kerberos/bin.
	   -r realm_filename
	        Set the realm file to use.  The default is whatever
		krb__get_cnffile uses, i.e. the file named in the
		environment variable KRB_CONF or the macro KRB_CONF,
		which is normally /usr/kerberos/lib/krb.conf.
	   -s filename
	   	Set the srvtab file to use.  The default is whatever
		krb_rd_req uses, i.e., KEYFILE, which is normally
		/etc/krb-srvtab.  */
#ifdef NOENCRYPTION
	while ((opt = getopt(argc, argv, "akp:P:r:s:")) != EOF) {
#else
	while ((opt = getopt(argc, argv, "akxp:P:r:s:")) != EOF) {
#endif
		switch (opt) {
		case 'a':
			anyport = 1;
			break;
		case 'k':
			ksh = 1;
			break;
#ifndef NOENCRYPTION
		case 'x':
			eksh = ksh = 1;
			break;
#endif
		case 'p':
			port = atoi (optarg);
			break;
		case 'P':
			kprogdir = optarg;
			break;
		case 'r':
			setenv ("KRB_CONF", optarg, 1);
			break;
		case 's':
			srvtab = optarg;
			break;
		default:
			/* Can't happen.  */
			_exit(1);
		}
	}

	/* Any non option arguments are ignored.  */
#endif

	if (port != -1) {
		struct sockaddr_in sin;
		int s, ns, sz;

		/* Accept an incoming connection on port.  */
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s < 0) {
			perror("socket");
			exit(1);
		}
		(void) setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
				  (char *)&on, sizeof(on));
		if (bind(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
			perror("bind");
			exit(1);
		}
		if (listen(s, 1) < 0) {
			perror("listen");
			exit(1);
		}
		sz = sizeof sin;
		ns = accept(s, (struct sockaddr *)&sin, &sz);
		if (ns < 0) {
			perror("accept");
			exit(1);
		}
		(void) dup2(ns, 0);
		(void) close(ns);
		(void) close(s);
	}

	fromlen = sizeof (from);
	if (getpeername(0, (struct sockaddr *)&from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof (on)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
#ifdef SO_LINGER
	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, "setsockopt (SO_LINGER): %m");
#endif
	doit(dup(0), &from);
}

extern char **environ;

#ifndef NCARGS
/* linux doesn't seem to have it... */
#define NCARGS 1024
#endif

static int
copyout (in, outfd, out, buf, size)
     int in, outfd;
     kstream out;
     char *buf;
     int size;
{
  int cc;
  errno = 0;
  cc = read (in, buf, size);
  if (cc <= 0) {
    shutdown (outfd, 1);
    return -1;
  }
  buf[cc+1] = 0;
  (void) kstream_write (out, buf, cc);
  return 0;
}

static int
copyin (in, out, buf, size)
     kstream in;
     int out;
     char *buf;
     int size;
{
  int cc;
  errno = 0;
  cc = kstream_read (in, buf, size);
  if (cc <= 0) {
    shutdown (out, 2);
    close (out);
    return -1;
  }
  (void) write (out, buf, cc);
  return 0;
}

static int
getport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s, sin_len;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);
	sin.sin_port = 0;
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		(void) close(s);
		return(-1);
	}
	sin_len = sizeof(sin);
	getsockname(s, (struct sockaddr *)&sin, &sin_len);
	*alport = ntohs(sin.sin_port);
	return(s);
}

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp;
	char locuser[16], remuser[16];
	struct passwd *pwd;
	int s;
	struct hostent *hp;
	char *hostname;
	short port;
	int pv[3][2], pid, cc;
	long ready, readfrom = 0;
	char buf[BUFSIZ], sig;
	int one = 1;
	char *path, *tz;
#ifdef ATHENA
	int def = 0;
#endif /* ATHENA */
#ifdef KERBEROS
	AUTH_DAT *kdata = (AUTH_DAT *)NULL;
	KTEXT ticket = (KTEXT)NULL;
	char instance[INST_SZ], version[9];
	/* need to save the sockaddr_in for krb_recvauth; *fromp is reused
	   before then */
	struct sockaddr_in fromaddr;
	int rc;
	long authoptions;

	fromaddr = *fromp; 
#endif /* KERBEROS */

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET) {
		syslog(LOG_ERR, "malformed from address\n");
		exit(1);
	}
	if ((fromp->sin_port >= IPPORT_RESERVED
	     || fromp->sin_port < IPPORT_RESERVED/2)
#ifdef KERBEROS
	    && !ksh
#endif
	    )
	  {
	    syslog(LOG_NOTICE, "connection from bad port\n");
	    exit(1);
	  }
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if ((cc = read(f, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_NOTICE, "read: %m");
			shutdown(f, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		if (ksh && anyport)
			s = getport(&lport);
		else
			s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, "can't get stderr port: %m");
			exit(1);
		}
		if (port >= IPPORT_RESERVED
#ifdef KERBEROS
		    && !ksh
#endif
		    ) {
			syslog(LOG_ERR, "2nd port not reserved\n");
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, (struct sockaddr *)fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, "connect second port: %m");
			exit(1);
		}
	}
	else
	  s = -1;
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp)
		hostname = strsave(hp->h_name);
	else
		hostname = strsave(inet_ntoa(fromp->sin_addr));
#ifdef KERBEROS
	if (ksh) {
		struct sockaddr_in myaddr;
		int myaddr_len = sizeof (myaddr);
		kdata = (AUTH_DAT *)malloc(sizeof(AUTH_DAT));
		ticket = (KTEXT) malloc(sizeof(KTEXT_ST));
		if (getsockname (f, (struct sockaddr *) &myaddr, &myaddr_len)) {
		  syslog (LOG_ERR, "can't check local address: %m");
		  if (eksh) {
		    error ("getsockname failed\n");
		    exit (1);
		  }
		}
		authoptions = eksh ? KOPT_DO_MUTUAL : 0L;
		strcpy(instance, "*");
		version[8] = '\0';
		if (rc=krb_recvauth(authoptions, f, ticket, "rcmd",
				    instance, &fromaddr, &myaddr,
				    kdata, srvtab, schedule, version)) {
			error("Kerberos rsh or rcp failed: %s\n",
				krb_get_err_text(rc));
			exit(1);
		}
	} else {
		getstr(remuser, sizeof(remuser), "remuser");
	}
	if (eksh)
	  {
	    kstr = kstream_create_rlogin_from_fd (0, &schedule, &kdata->session);
	    if (s >= 0)
	      kstr2 = kstream_create_rlogin_from_fd (s, &schedule, &kdata->session);
	  }
	else
	  {
	    kstr = kstream_create_from_fd (0, 0, 0);
	    if (s >= 0)
	      kstr2 = kstream_create_from_fd (s, 0, 0);
	  }
	kstream_set_buffer_mode (kstr, 0);
	if (kstr2)
	  kstream_set_buffer_mode (kstr2, 0);
#else
	getstr(remuser, sizeof(remuser), "remuser");
#endif /* KERBEROS */
	getstr(locuser, sizeof (locuser), "Local username");
	getstr(cmdbuf, sizeof(cmdbuf), "command");
	setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
#ifdef ATHENA
		strcpy (locuser, DEFAULT);
		pwd = getpwnam(locuser);
		if (pwd == NULL) {
			error("Login incorrect.\n");
			exit(1);
		} else {
			def = 1;
			sprintf(buf,"Remote user %s@%s logged in as default\n",
				remuser, hostname);
			syslog (LOG_INFO, buf);
		}
#else
		error("Login incorrect.\n");
		exit(1);
#endif /* ATHENA */
	}
	endpwent();
	if (chdir(pwd->pw_dir) < 0) {
		(void) chdir("/");
#ifdef notdef
		error("No remote directory.\n");
		exit(1);
#endif
	}
#ifdef KERBEROS
	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0') {
		if (ksh) {
			if ( kuserok(kdata,locuser) ) {
				error("Permission denied.\r\n");
				exit(1);
			}
		} else {
			if (ruserok(hostname, pwd->pw_uid == 0,
#ifdef ATHENA
				    def ? locuser : remuser,
#else
				    remuser,
#endif
				    locuser) < 0)
			  {
			    error("Permission denied.\n");
			    exit(1);
			  }
		}
	}
#else
	if (pwd->pw_passwd != 0 && *pwd->pw_passwd != '\0' &&
	    ruserok(hostname, pwd->pw_uid == 0, remuser, locuser) < 0) {
		error("Permission denied.\n");
		exit(1);
	}
#endif /* KERBEROS */
	if (pwd->pw_uid && !access("/etc/nologin", F_OK)) {
		error("Logins currently disabled.\n");
		exit(1);
	}
	(void) write(2, "\0", 1);
	if (port || eksh) {
		if (port && pipe(pv[2]) < 0) {
			perror("rlogind: pipe failed:");
			exit(1);
		}
		if (eksh
		    && (pipe (pv[0]) < 0
			|| pipe (pv[1]) < 0)) {
		  perror ("rlogind: pipe failed:");
		  exit (1);
		}
		pid = fork();
		if (pid == -1)  {
			perror("rlogind: fork failed:");
			exit(1);
		}
		if (pid) {
			(void) close(2);
			(void) close(f);
			if (port) {
			  (void) close(pv[2][1]);
			  readfrom = (1L<<s) | (1L<<pv[2][0]);
			}
			if (eksh) {
			  close (pv[1][1]);
			  readfrom |= (1L << pv[1][0]);
			  close (pv[0][0]);
			  readfrom |= (1L << 0);
			} else {
			  (void) close(0); (void) close(1);
			}
#if !defined(_IBMR2) && !defined(KERBEROS)
			if (port)
			  ioctl(pv[2][0], FIONBIO, (char *)&one);
			if (eksh)
			  ioctl(pv[1][0], FIONBIO, (char *)&one);
#endif
			/* should set s nbio! */
			do {
				ready = readfrom;
				if (select(16, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0) < 0)
					break;
#define COPYOUT(IN,OUT,KOUT) \
	if (ready & (1L << (IN))) { if (copyout (IN, OUT, KOUT, buf, (int) sizeof (buf)) < 0) readfrom &= ~(1L << (IN)); }
#define COPYIN(IN,OUT,KIN) \
	if (ready & (1L << (IN))) { if (copyin (KIN, OUT, buf, (int) sizeof (buf)) < 0) readfrom &= ~(1L << (IN)); }
				if (port) {
				  if (ready & (1L<<s)) {
				    if (kstream_read(kstr2, &sig, 1) <= 0)
				      readfrom &= ~(1L<<s);
				    else
				      killpg(pid, sig);
				  }
				  COPYOUT (pv[2][0], s, kstr2);
				}
				if (eksh) {
				  COPYOUT (pv[1][0], 1, kstr);
				  COPYIN (0, pv[0][1], kstr);
				}
			} while (readfrom);
			exit(0);
		}
		krb_setpgrp(0, getpid());
		if (port) {
		  (void) close(s); (void) close(pv[2][0]);
		  dup2(pv[2][1], 2);
		  (void) close(pv[2][1]);
		}
		if (eksh) {
		  close (1); close (pv[1][0]);
		  dup2 (pv[1][1], 1); close (pv[1][1]);
		  close (0); close (pv[0][1]);
		  dup2 (pv[0][0], 0); close (pv[0][0]);
		}
	}
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	(void) close(f);
	(void) setgid((gid_t)pwd->pw_gid);
	if (getuid() == 0 || getuid() != pwd->pw_uid) {
		/* For testing purposes, we don't call initgroups if
                   we already have the right uid, and it is not root.
                   This is because on some systems initgroups outputs
                   an error message if not called by root.  */
		initgroups(pwd->pw_name, pwd->pw_gid);
	}
	(void) setuid((uid_t)pwd->pw_uid);
	/* Preserve TZ if it is set.  */
	tz = getenv("TZ");
	environ[0] = NULL;
	setenv("HOME", pwd->pw_dir, 1);
	setenv("SHELL", pwd->pw_shell, 1);
	setenv("USER", pwd->pw_name, 1);
	path = (char *) malloc(strlen(kprogdir) + strlen(RPATH) + 2);
	if (path != NULL) {
		sprintf(path, "%s:%s", kprogdir, RPATH);
		setenv("PATH", path, 1);
	}
	if (tz != NULL)
		setenv("TZ", tz, 1);
#ifdef KERBEROS
	/* Pass the srvtab argument to the program via the KRB_SRVTAB
           environment variable.  This is for the benefit of rcp,
           which needs to authenticate again in order to handle
           encryption.  */
	if (srvtab[0] != '\0')
		setenv("KRB_SRVTAB", srvtab, 1);
	/* To make Kerberos rcp work correctly, we must ensure that we
           invoke Kerberos rcp on this end, not normal rcp, even if
           the shell startup files change PATH.  This is a hack.  */
	if (strncmp(cmdbuf, "rcp ", sizeof "rcp " - 1) == 0) {
		char *copy;
		struct stat s;

		copy = malloc(strlen(cmdbuf) + 1);
		strcpy(copy, cmdbuf);
		strcpy(cmdbuf, kprogdir);
		strcat(cmdbuf, "/rcp");
		if (stat(cmdbuf, &s) >= 0)
			strcat(cmdbuf, copy + 3);
		else
			strcpy(cmdbuf, copy);
		free(copy);
	}
#endif
	cp = strrchr(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);
	perror(pwd->pw_shell);
	exit(1);
}

/*VARARGS1*/
error(fmt, a1, a2, a3, a4)
	char *fmt;
	char *a1, *a2, *a3, *a4;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3, a4);
	(void) write(2, buf, strlen(buf));
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	int ocnt = cnt;
	char *obuf = buf;
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
		        error("%s '%.*s' too long, %d characters maximum.\n",
			      err, ocnt, obuf, ocnt-1);
			exit(1);
		}
	} while (c != 0);
}

#ifdef KERBEROS
#ifndef HAVE_STRSAVE
static char *strsave(s)
char *s;
{
    char *ret = (char *) malloc(strlen(s) + 1);
    strcpy(ret, s);
    return(ret);
}
#endif
#endif
