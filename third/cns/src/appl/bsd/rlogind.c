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
static char sccsid[] = "@(#)rlogind.c	5.17 (Berkeley) 8/31/88";
#endif /* not lint */

/*
 * remote login server:
 *	remuser\0
 *	locuser\0
 *	terminal info\0
 *	data
 */

#include "conf.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __SCO__
#include <sys/unistd.h>
#endif

#include <netinet/in.h>

#ifdef POSIX
#define POSIX_TERMIOS
#endif

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#ifndef POSIX_TERMIOS
/* or perhaps POSIX? given termios, we probably shouldn't use sgtty... */
#include <sgtty.h>
#endif
#include <netdb.h>
#include <syslog.h>
#include <string.h>

#ifdef POSIX
#include <stdlib.h>
#endif
#ifdef POSIX_TERMIOS
#include <termios.h>
#ifdef _AIX
#include <termio.h>
#endif
#endif
#ifdef __SCO__
/* for TIOCPKT_* */
#include <sys/spt.h>
/* for struct winsize */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef hpux
#include <sys/ptyio.h>
#endif

#ifdef __svr4__
#include <sys/tty.h>
#ifndef solaris20
/* These values are over-the-wire protocol, *not* local values */
#ifndef TIOCPKT_NOSTOP
#define TIOCPKT_NOSTOP          0x10
#endif
#ifndef TIOCPKT_DOSTOP
#define TIOCPKT_DOSTOP          0x20
#endif
#ifndef TIOCPKT_FLUSHWRITE
#define TIOCPKT_FLUSHWRITE      0x02
#endif
#else
/* but solaris actually uses packet mode, so the real macros are needed too */
#include <sys/ptyvar.h>
#endif

#ifndef NO_SYSIO
/* get FIONBIO from sys/filio.h, so what if it is a compatibility feature */
#include <sys/filio.h>
#endif
#define krb_setpgrp(a,b) setpgrp()
#ifndef NO_GETPGID
#define getpgrp(a) getpgid(a)
#endif
#ifndef NO_PTYEM
/* some svr4 systems (hpux 68k) don't have the streams interface... */
/* for I_PUSH */
#include <sys/stropts.h>
#define STREAMS_PTYEM
#endif
#ifdef __DGUX__
/* STREAMS_PTYEM definitions, like M_DATA, are here on DG/UX */
#include <sys/stream.h>
#endif
#endif

#if defined (linux) || defined (__osf__)
#define krb_setpgrp(a,b) setpgid(a,b) 
#define GETPGRP_ONEARG
#endif

#ifdef hpux
#include <sys/ptyio.h>
#define PFLAG "-p",
#else
#define PFLAG /**/
#endif

#ifdef sgi
/* for _getpty */
#include <unistd.h>
#define GETPGRP_ONEARG
#define krb_setpgrp(a,b) setsid()
#endif

#ifdef __alpha
#define GETPGRP_ONEARG
#endif

#ifndef krb_setpgrp
#define krb_setpgrp(a,b) setpgrp(a,b)
#endif

#ifdef KERBEROS
#include <sys/param.h>
#include <utmp.h>
#include <krb.h>
#include <krb_err.h>
#include <kstream.h>

#ifndef LOGIN_PROGRAM
#define LOGIN_PROGRAM "/usr/athena/etc/login.krb"
#endif
#define	SECURE_MESSAGE	"This rlogin session is using DES encryption for all data transmissions.\r\n"
#define NMAX	sizeof(((struct utmp *)0)->ut_name)
char		lusername[NMAX+1];
char		term[64];
int 		Klogin, klogin, eklogin;
AUTH_DAT	*kdata;
KTEXT		ticket;
Key_schedule	schedule;
void		do_krb_login();
kstream		kstr = 0;
char		*srvtab = "";
char		*login_program = LOGIN_PROGRAM;
#endif

# ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
# endif /* TIOCPKT_WINDOW */

extern	int errno;
int	reapchild();
struct	passwd *getpwnam();

extern char *error_message();

static	int Pfd;

#if (defined(_AIX) && defined(i386)) || defined(ibm032) || (defined(vax) && !defined(ultrix)) || (defined(SunOS) && SunOS > 40) || defined(solaris20)
#define VHANG_FIRST
#endif

#if defined(ultrix)
#define VHANG_LAST		/* vhangup must occur on close, not open */
#endif


/* ARGSUSED */
main(argc, argv)
	int argc;
	char **argv;
{
#ifdef KERBEROS
	int opt;
	extern char *optarg;
#endif
	int on = 1, fromlen;
	struct sockaddr_in from;
	int port = -1;

	/* Remove any controlling terminal.  */
	{
		int tt = open("/dev/tty", O_RDWR);
		if (tt > 0) {
			int gpid;

#ifdef GETPGRP_ONEARG
			gpid = getpgrp();
#else
			gpid = getpgrp(0);
#endif
			if (gpid == getpid()) {
				int pid;

				(void) signal(SIGHUP, SIG_IGN);
				pid = fork();
				if (pid < 0)
					fatalperror(2, "", errno);
				if (pid != 0)
					_exit(0);
				while (getppid() != 1)
					sleep(1);
				(void) signal(SIGHUP, SIG_DFL);
			}
#ifdef POSIX
			setsid();
#else
#ifdef TIOCNOTTY
			(void) ioctl(tt, TIOCNOTTY, 0);
#endif
#endif
			(void) close(tt);
		}
	}

#ifdef KERBEROS
	/*
	 * if the command name is klogind or Klogind or eklogind, the
	 * connection has 
	 * been made thru the Kerberos authentication port for rlogin.  The
	 * protocol is different, to provide the transmission for the
	 * ticket used for authentication.
	 *
	 * If the name of the program is "klogind", and the Kerberos 
	 * authentication fails, we will allow the user password access
	 * to this host.  If the program name is "Klogind", and the
	 * Kerberos authentication fails, we will NOT allow password access;
	 * this is for protection of the Kerberos server and other highly
	 * paranoid hosts.
	 *
	 * If the name of the program is "eklogind", all data passing over
	 * the network pipe are encrypted.
	 */
#ifdef NOENCRYPTION
	klogin = (!strcmp(*argv,"eklogind") ||
		  !strcmp(*argv,"klogind"));	/* pass -k flag to login */
#else
	eklogin = !strcmp(*argv,"eklogind");	/* pass -e flag to login */
	klogin = !strcmp(*argv,"klogind");	/* pass -k flag to login */
#endif
	Klogin = !strcmp(*argv,"Klogind");	/* pass -K flag to login */
	/* 
	 * if klogin, Klogin, and eklogin are zero (ie, the program name was
	 * probably rlogind instead), pass the -r flag to login
	 */

	/* Accept options.
	   -k
	   	Pass -k flag to login, as though invoked as klogind.
	   -K
	   	Pass -K flag to login, as though invoked as Klogind.
#ifndef NOENCRYPTION
	   -x
	   	Pass -e flag to login, as though invoked as eklogind.
#endif
	   -l login_program
	        Set the login program to use.  The default is
		LOGIN_PROGRAM, which Makefile.in normally sets to
		/usr/kerberos/etc/login.krb.
	   -p port
	        Accept a connection on the given port.  Normally
		rlogind assumes that it was invoked by inetd.
	   -r realm_filename
	        Set the realm file to use.  The default is whatever
		krb__get_cnffile uses, i.e. the file named in the
		environment variable KRB_CONF or the macro KRB_CONF,
		which is normally /usr/kerberos/lib/krb.conf.
	   -s srvtab_filename
	   	Set the srvtab file to use.  The default is whatever
		krb_rd_req uses, i.e., KEYFILE, which is normally
		/etc/krb-srvtab.  */
#ifdef NOENCRYPTION
	while ((opt = getopt(argc, argv, "kKl:p:r:s:")) != EOF) {
#else
	while ((opt = getopt(argc, argv, "kKxl:p:r:s:")) != EOF) {
#endif
		switch (opt) {
		case 'k':
			klogin = 1;
			Klogin = 0;
#ifndef NOENCRYPTION
			eklogin = 0;
#endif
			break;
		case 'K':
			Klogin = 1;
			klogin = 0;
#ifndef NOENCRYPTION
			eklogin = 0;
#endif
			break;
#ifndef NOENCRYPTION
		case 'x':
			eklogin = 1;
			klogin = Klogin = 0;
			break;
#endif
		case 'l':
			login_program = optarg;
			break;
		case 'p':
			port = atoi (optarg);
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
#endif /* KERBEROS */

#ifdef KERBEROS
#ifndef LOG_AUTH /* 4.2 syslog */
#ifdef NOENCRYPTION
	openlog(klogin ? "klogind" : (Klogin ? "Klogind" : "rlogind"),
		LOG_PID);
#else /* !NOENCRYPTION */
	openlog(eklogin ? "eklogind" : (klogin ? "klogind" :
					(Klogin ? "Klogind" : "rlogind")),
		LOG_PID);
#endif /* NOENCRYPTION */
#else
#ifdef NOENCRYPTION
	openlog(klogin ? "klogind" : (Klogin ? "Klogind" : "rlogind"),
		LOG_PID | LOG_AUTH, LOG_AUTH);
#else /* !NOENCRYPTION */
	openlog(eklogin ? "eklogind" : (klogin ? "klogind" :
					(Klogin ? "Klogind" : "rlogind")),
		LOG_PID | LOG_AUTH, LOG_AUTH);
#endif /* NOENCRYPTION */
#endif /* 4.2 syslog */
#else
#ifndef LOG_AUTH /* 4.2 syslog */
	openlog("rlogind", LOG_PID);
#else
	openlog("rlogind", LOG_PID | LOG_AUTH, LOG_AUTH);
#endif /* 4.2 syslog */
#endif /* KERBEROS */

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
		syslog(LOG_DEBUG, "getpeername failed: %m");
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
		_exit(1);
	}
	if (setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
		syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
	}
	doit(0, &from);
}

int	child;
sigtype	cleanup();
int	netf;
char	*line;
extern	char	*inet_ntoa();

#ifdef NO_WINSIZE
struct winsize {
	unsigned short ws_row, ws_col;
	unsigned short ws_xpixel, ws_ypixel;
};
#endif
struct winsize win = { 0, 0, 0, 0 };


doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	int i, p, t, vfd, pid, on = 1;
	register struct hostent *hp;
	struct hostent hostent;
	char c;

	alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);
	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr((const char *)&fromp->sin_addr,
			   sizeof (struct in_addr),
			   fromp->sin_family);
	if (hp == 0) {
		/*
		 * Only the name is used below.
		 */
		hp = &hostent;
		hp->h_name = inet_ntoa(fromp->sin_addr);
	}
#ifdef KERBEROS
	/* Don't care about reserved port for kerberos logins */
#ifdef NOENCRYPTION
	if (fromp->sin_family != AF_INET ||
	    (!klogin && !Klogin &&
	     (fromp->sin_port >= IPPORT_RESERVED ||
	     fromp->sin_port < IPPORT_RESERVED/2)))
#else /* !NOENCRYPTION */
	if (fromp->sin_family != AF_INET ||
	    (!klogin && !Klogin && !eklogin &&
	     (fromp->sin_port >= IPPORT_RESERVED ||
	     fromp->sin_port < IPPORT_RESERVED/2)))
#endif /* NOENCRYPTION */
#else /* !KERBEROS */
	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2)
#endif /* KERBEROS */
		fatal(f, "Permission denied");
#if defined(KERBEROS) && !defined(NOENCRYPTION)
	/*
	 * If encrypting, we need to respond here, since we have to send
	 * the mutual authentication stuff before the response
	 */
	if (eklogin)
	    do_krb_login(f, hp->h_name);
	else
	  {
	    kstr = kstream_create_from_fd (0, 0, 0);
	    kstream_set_buffer_mode (kstr, 0);
	  }
#endif /* KERBEROS */
	write(f, "", 1);
#if defined(_AIX) && defined(_IBMR2)
	if ((p = open("/dev/ptc", O_RDWR)) >= 0) {
		line = ttyname(p);
		goto gotpty;
	}
#else
#ifdef sgi
	line = _getpty(&p, O_RDWR, 0600, 1); /* mode 0600, don't fork */
	if (line) goto gotpty;
#else
#ifdef STREAMS_PTYEM
	/* from pts(7) */
	if ((p = open("/dev/ptmx", O_RDWR)) >= 0) {
		if (!grantpt(p)) {
			if (!unlockpt(p)) {
				extern char *ptsname ();
				if (line = ptsname(p)) {
					goto gotpty;
				}
			}
		}
	}
#else
	for (c = 'p'; c <= 'z'; c++) {
		struct stat stb;
		static char ptybuf[20] = "/dev/ptyXX";
		line = ptybuf;
#ifndef __SCO__
		line[sizeof("/dev/pty") - 1] = c;
		line[sizeof("/dev/ptyp") - 1] = '0';
		if (stat(line, &stb) < 0)
			break;
#else
		line[sizeof("/dev/pty") - 1] = 'p';
#endif
		for (i = 0; i < 16; i++) {
#ifndef __SCO__
 			line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
#else
			sprintf(line + sizeof("/dev/ptyp") - 1, "%d",
				(c - 'p') * 16 + i);
#endif
 			p = open(line, O_RDWR);
			if (p > 0)
				goto gotpty;
		}
	}
#endif /* !STREAMS_PTYEM */
#endif /* !sgi */
#endif /* !AIX3.1 */
	fatal(f, "Out of ptys");
	/*NOTREACHED*/
gotpty:
	Pfd = p;
#ifndef solaris20
	(void) ioctl(p, TIOCSWINSZ, &win);
#endif
	netf = f;
#if !defined(_AIX) && !defined(_IBMR2)
#ifndef STREAMS_PTYEM
	line[strlen("/dev/")] = 't';
#endif
#endif
#ifdef __alpha
	/* osf/1 method of losing controlling tty...*/
	setsid();
#endif
	
#ifdef VHANG_FIRST
	vfd = open(line, O_RDWR);
 	if (vfd < 0)
 		fatalperror(vfd, line);
 	if (fchmod(vfd, 0))
 		fatalperror(vfd, line);
	/*
	 * On some systems you have to revoke tty access on close or
	 * you may lose access yourself.
	 */
 	(void)signal(SIGHUP, SIG_IGN);
#if defined(_AIX) && defined(i386)
	_vhangup();
#else
 	vhangup();
#endif
#endif /* VHANG_FIRST */
 	(void)signal(SIGHUP, SIG_DFL);
 	t = open(line, O_RDWR);
#ifdef VHANG_FIRST
#ifndef VHANG_NO_CLOSE
	(void) close(vfd);
#endif
#endif /* VHANG_FIRST */
 	if (t < 0)
 		fatalperror(f, line);
#ifdef __alpha
	if(ioctl(t, TIOCSCTTY, 0) < 0) /* set controlling tty */
	  fatalperror(f, "setting controlling tty");
#endif
#ifdef STREAMS_PTYEM
	/* from pts(7) */
	if(ioctl(t, I_PUSH, "ptem") < 0)	/* push ptem */
		fatalperror(f, "pushing ptem streams module");
	if(ioctl(t, I_PUSH, "ldterm") < 0)	/* push ldterm */
		fatalperror(f, "pushing ldterm streams module");
#ifdef __svr4__
#ifndef solaris20
	if(ioctl(t, I_PUSH, "ttcompat") < 0)	/* push ttcompat */
		fatalperror(f, "pushing ldterm streams module");
#endif
#endif
	(void) ioctl(p, TIOCSWINSZ, &win);
#endif
#ifdef POSIX_TERMIOS
	{
		struct termios new_termio;

		tcgetattr(t,&new_termio);
		new_termio.c_lflag &=  ~(ICANON|ECHO|ISIG|IEXTEN);
		/* so that login can read the authenticator */
		new_termio.c_iflag &= ~(IXON|IXANY|BRKINT|INLCR|ICRNL|ISTRIP);
		/* new_termio.c_iflag = 0; */
		/* new_termio.c_oflag = 0; */
		new_termio.c_cc[VMIN] = 1;
		new_termio.c_cc[VTIME] = 0;
		tcsetattr(t,TCSANOW,&new_termio);
	}
#else
 	{
 		struct sgttyb b;
 
 		(void)ioctl(t, TIOCGETP, &b);
 		b.sg_flags = RAW|ANYP;
 		(void)ioctl(t, TIOCSETP, &b);
	}
#endif
#ifdef DEBUG
	{
		int tt = open("/dev/tty", O_RDWR);
		if (tt > 0) {
		(void) ioctl(tt, TIOCNOTTY, 0);
		(void) close(tt);
	  }
	}
#endif
#if defined(POSIX) && ! defined(__SCO__)
	/* On SCO, this must be done in the child, not the parent.  I
           don't know why this is not the case on other systems.  If
           the parent calls setsid, it will get the terminal as a
           controlling terminal, and it will then be vulnerable to
           signals generated by the terminal.  */
	setsid();
#endif
#if defined (__386BSD__) || defined (__NetBSD__)
        if (ioctl(t, TIOCSCTTY, (char *)NULL) == -1)
		fatalperror(f, "TIOCSCTTY", errno);
#endif
	t = open(line, 2);
	if (t < 0)
		fatalperror(f, line, errno);
#if defined(POSIX_TERMIOS) && !defined(ultrix)
	{
		struct termios new_termio;

		tcgetattr(t,&new_termio);
		new_termio.c_lflag &=  ~(ICANON|ECHO|ISIG|IEXTEN);
		/* so that login can read the authenticator */
		new_termio.c_iflag &= ~(IXON|IXANY|BRKINT|INLCR|ICRNL|ISTRIP);
		/* new_termio.c_iflag = 0; */
		/* new_termio.c_oflag = 0; */
		new_termio.c_cc[VMIN] = 1;
		new_termio.c_cc[VTIME] = 0;
		tcsetattr(t,TCSANOW,&new_termio);
	}
#else
	{
		struct sgttyb b;
		gtty(t, &b); b.sg_flags = RAW|ANYP; stty(t, &b);
	}
#endif
	pid = fork();
	if (pid < 0)
		fatalperror(f, "", errno);
	if (pid == 0) {
#ifdef __SCO__
		/* On SCO, we need to force the child to be in a
                   separate process group.  Otherwise, the parent will
                   catch terminal interrupts intended only for the
                   child.  I don't know why this isn't required on
                   other systems.  */
		setsid();
		close(t);
		t = open(line, 2);
#endif

		close(f), close(p);
		dup2(t, 0); dup2(t, 1); dup2(t, 2);
		if (t > 2)
		    close(t);

		/* Under Ultrix 3.0, the pgrp of the slave pty terminal
		 needs to be set explicitly.  Why rlogind works at all
		 without this on 4.3BSD is a mystery.
		 It seems to work fine on 4.3BSD with this code enabled.
		 */
	        /* On solaris, at least, the pgrp is already correct... */
#ifdef GETPGRP_ONEARG
		pid = getpgrp();
#else
		/* hmm. this could be getpgrp(0) on most platforms... */
		pid = getpgrp(getpid());
#endif
#ifdef POSIX_TERMIOS /* solaris */
		/* we've already done setsid above. Just do tcsetpgrp here. */
		tcsetpgrp(0, pid);
#else
#ifndef hpux
		ioctl(0, TIOCSPGRP, &pid);
#else
		/* we've already done setsid above. Just do tcsetpgrp here. */
		tcsetpgrp(0, pid);
#endif
#endif /* posix */
#ifdef KERBEROS
		if (klogin) {
			if (srvtab[0] == '\0')
				execl(login_program, "login", PFLAG "-k",
				      hp->h_name, 0);
			else
				execl(login_program, "login", "-s", srvtab,
				      PFLAG "-k", hp->h_name, 0);
		} else if (Klogin) {
			if (srvtab[0] == '\0')
				execl(login_program, "login", PFLAG "-K",
				      hp->h_name, 0);
			else
				execl(login_program, "login", "-s", srvtab,
				      PFLAG "-K", hp->h_name, 0);
#ifndef NOENCRYPTION
		} else if (eklogin) {
		    struct passwd *pw;
		    pw = getpwnam(lusername);
		    if (pw && !pw->pw_uid)
			syslog(LOG_INFO, "ROOT LOGIN (krb) from %s, %s.%s@%s.",
			       hp->h_name, kdata->pname, kdata->pinst,
			       kdata->prealm);
		    execl(login_program, "login", PFLAG "-e", hp->h_name,
			  lusername, 0);
#endif
		} else {
			execl(login_program, "login", PFLAG "-r", hp->h_name, 0);
		}
		fatalperror(2, login_program, errno);
#else
		execl("/bin/login", "login", PFLAG "-r", hp->h_name, 0);
		fatalperror(2, "/bin/login", errno);
#endif /* KERBEROS */
		/*NOTREACHED*/
	}
	close(t);

#if defined(KERBEROS) && !defined(NOENCRYPTION)
	if (eklogin)
	    kstream_write (kstr, SECURE_MESSAGE, sizeof(SECURE_MESSAGE)-1);
	else
	/*
	 * if encrypting, don't turn on NBIO, else the read/write routines
	 * will fail to work properly
	 */
#endif /* KERBEROS && !NOENCRYPTION */
#ifndef hpux
	ioctl(f, FIONBIO, &on);
	ioctl(p, FIONBIO, &on);
#else /* hpux */
	fcntl(f, F_SETFL, O_NONBLOCK | fcntl(f, F_GETFL, 0));
	fcntl(p, F_SETFL, O_NONBLOCK | fcntl(p, F_GETFL, 0));
#endif /* hpux */
#if defined(TIOCPKT) && !defined(__svr4__) || defined(solaris20)
	ioctl(p, TIOCPKT, &on);
#endif
#ifdef STREAMS_PTYEM
	ioctl(p, I_PUSH, "pckt");
#endif
	signal(SIGTSTP, SIG_IGN);
	signal(SIGCHLD, cleanup);
	krb_setpgrp(0, 0);
#ifdef KERBEROS
	if (eklogin)
	    (void) write(p, term, strlen(term)+1); /* stuff term info down
						      to login */
#endif /* KERBEROS */
	protocol(f, p);
	signal(SIGCHLD, SIG_IGN);
	cleanup();
}

char	magic[2] = { (char)0377, (char)0377 };
char	oobdata[] = {(char)TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
control(pty, cp, n)
	int pty;
	char *cp;
	int n;
{
	struct winsize w;

	if (n < 4+sizeof (w) || cp[2] != 's' || cp[3] != 's')
		return (0);
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	memcpy((char *)&w, cp+4, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);
	(void)ioctl(pty, TIOCSWINSZ, &w);
	return (4+sizeof (w));
}

/*
 * rlogin "protocol" machine.
 */
protocol(f, p)
	int f, p;
{
	char pibuf[1024], fibuf[1024], *pbp, *fbp;
	register pcc = 0, fcc = 0;
	int cc;
	char cntl;
	int fd_max = f;
	if (p > fd_max) fd_max = p;

	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	for (;;) {
		int n;
		fd_set ibits, obits, ebits;

		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		FD_ZERO(&ebits);
		if (fcc)
			FD_SET(p, &obits);
		else
			FD_SET(f, &ibits);
		if (pcc >= 0)
			if (pcc)
				FD_SET(f, &obits);
			else
				FD_SET(p, &ibits);
		FD_SET(p, &ebits);
		if ((n = select(fd_max+1, &ibits, &obits, &ebits, 0)) < 0) {
			if (errno == EINTR)
				continue;
			fatalperror(f, "select");
		}
		if (n == 0) {
			/* shouldn't happen, since we're not timing out */
			sleep(5);
			continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (FD_ISSET(p, &ebits)) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					FD_CLR(p, &ibits);
				}
			}
		}
		if (FD_ISSET(f, &ibits)) {
			fcc = kstream_read (kstr, fibuf, sizeof (fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, n;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						n = control(p, cp, left);
						if (n) {
							left -= n;
#ifdef MOVE_WITH_BCOPY
							if (left > 0)
								bcopy(cp+n, cp, left);
#else
							if (left > 0)
								memmove(cp, cp+n, left);
#endif
							fcc -= n;
							goto top; /* n^2 */
						}
					}
			}
		}

		if (FD_ISSET(p, &obits) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (FD_ISSET(p, &ibits)) {
#ifdef STREAMS_PTYEM
			struct strbuf databuf, ctlbuf;
			static char ctl_s[128];
			int flags = 0, st;
			databuf.buf=pibuf;
			databuf.maxlen=sizeof(pibuf);
			ctlbuf.buf=ctl_s;
			ctlbuf.maxlen=sizeof(ctl_s);

			st = getmsg(p, &ctlbuf, &databuf, &flags);
			/* since we're using 'pckt'... */
			if (st == (-1) && errno == EWOULDBLOCK)
				pcc = 0;
			else if (st == (-1))
				{ pcc = st; break; }
			if(ctlbuf.len != 1) {
			  /* then it's probably more data, if MOREDATA */
			  if (ctlbuf.len == (-1) && (databuf.len != (-1))) {
			    pbp = databuf.buf;
			    pcc = databuf.len;
			  } else {
			    /* the break causes an abort from here. */
			    pcc = 0; break;
			  }
			} else switch (ctlbuf.buf[0]) {
			case M_DATA:
			  pbp = databuf.buf;
			  pcc = databuf.len;
			  break;
			case M_PROTO:
			  /* hmm. */
			  pcc = 0;
			  break;
			}
#if 0
			if (ctlbuf.len != (-1)) {
				if (pkcontrol(ctl_s[0])) {
					ctl_s[0] |= oobdata[0];
					send(f, &ctl_s[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
#endif
#else
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0)
				pbp++, pcc--;
			else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
#endif
		}
		if (FD_ISSET(f, &obits) && pcc > 0) {
			cc = kstream_write(kstr, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/* also shouldn't happen */
				sleep(5);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
}

sigtype
cleanup()
{
	char *p;

#ifdef solaris20
	p = line;
#else
	p = line + sizeof("/dev/") - 1;
#endif
	if (!logout(p))
		logwtmp(p, "", "");
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
	*p = 'p';
	(void)chmod(line, 0666);
	(void)chown(line, 0, 0);
#ifdef VHANG_LAST
	close(Pfd);
	vhangup();
#endif
	shutdown(netf, 2);
	exit(1);
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, "rlogind: %s.", msg);
	syslog(LOG_DEBUG, "fatal: %s",buf+1);
	strcat (buf, "\r\n");
#ifdef KERBEROS
	if(kstr)
	  (void) kstream_write(kstr, buf, strlen(buf));
	else
#endif /* KERBEROS */
	(void) write(f, buf, strlen(buf));
	exit(1);
}

fatalperror(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];
	extern int sys_nerr;
#ifndef __NetBSD__
	extern char *sys_errlist[];
#endif

	if ((unsigned)errno < sys_nerr)
		(void) sprintf(buf, "%s: %s", msg, sys_errlist[errno]);
	else
		(void) sprintf(buf, "%s: Error %d", msg, errno);
	fatal(f, buf);
}

#ifdef KERBEROS
void
do_krb_login(f, host)
	int f;
	char *host;
{
	int rc;
	struct sockaddr_in sin, faddr;
	char instance[INST_SZ], version[9];
	long authoptions = KOPT_DO_MUTUAL;

	if (getuid()) {
		fatal (f, "server not running as root?!  Login fails");
	}
	rc = sizeof(sin);
	if (getpeername(0, (struct sockaddr *)&sin, &rc)) {
		fatalperror (f, "can't get peer name");
	}
	rc = sizeof(faddr);
	if (getsockname(0, (struct sockaddr *)&faddr, &rc)) {
		fatalperror (f, "can't get socket name");
	}

	kdata = (AUTH_DAT *)malloc( sizeof(AUTH_DAT) );
	ticket = (KTEXT) malloc(sizeof(KTEXT_ST));

	strcpy(instance, "*");
	if (rc=krb_recvauth(authoptions, 0, ticket, "rcmd",
			    instance, &sin,
			    &faddr,
			    kdata, srvtab, schedule, version)) {
		/* XXX Should spit back error msg from krb library.  */
	  char buf[1024];
	  initialize_krb_error_table();
	  sprintf(buf, "Error %s decoding authenticator",
		  error_message(rc+krb_err_base));
	  
	  fatal (f, buf);
	}
	getstr(lusername, sizeof (lusername), "Local username");
	getstr(term, sizeof(term), "Terminal type");

	if (rc=kuserok(kdata,lusername)) {
		char buf[1024];
		sprintf (buf, "You are not allowed to log in as `%s'",
			 lusername);
		fatal (f, buf);
	}
#ifdef NOENCRYPTION
	kstr = kstream_create_from_fd (0, 0, 0);
#else
	kstr = kstream_create_rlogin_from_fd (0, &schedule, &kdata->session);
#endif
	kstream_set_buffer_mode (kstr, 0);
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
		if (read(0, &c, 1) != 1) {
			exit(1);
		}
		if (--cnt < 0) {
			printf("%s '%.*s' too long, %d characters maximum.\r\n",
			       err, ocnt, obuf, ocnt-1);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}
#endif
