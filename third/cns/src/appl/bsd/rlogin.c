/*
 *	rlogin.c
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
static char sccsid[] = "@(#)rlogin.c	5.12 (Berkeley) 9/19/88";
#endif /* not lint */

/*
 * rlogin - remote login
 */

#include "conf.h"
#ifdef _AIX
#undef _BSD
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <netdb.h>
#ifdef KERBEROS
#include <krb.h>
#include <krbports.h>
#include <kstream.h>
#endif

#ifdef POSIX
#include <termios.h>
#ifdef __svr4__
/* for struct ltchars */
#include <sys/tty.h>
#include <sys/ttold.h>
#include <sys/stropts.h>
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

#ifndef NO_SYSIO
/* for SIOCATMARK */
#include <sys/sockio.h>
#endif
/* for exit(), which is used as a signal handler */
#include <stdlib.h>
#endif

/* how do we tell apart irix 5 and irix 4? */
#if defined(__sgi) && defined(__mips)
/* for exit() */ 
#include <unistd.h>
/* IRIX 5: TIOCGLTC doesn't actually work */
#undef TIOCGLTC
#endif

#if defined (__386BSD__) || defined (__NetBSD__)
#include <sys/ioctl_compat.h>
#endif

struct termios deftty;

#else /* !POSIX */
#include <sgtty.h>
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

# ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
# endif /* TIOCPKT_WINDOW */

/* concession to sun */
# ifndef SIGUSR1
# define SIGUSR1 30
# endif /* SIGUSR1 */

#define MY_SIGUSR1 SIGUSR1

/* SCO sends SIGUSR1 rather than SIGURG, so we use SIGUSR2 to play the
   role of SIGUSR1.  */
#ifdef __SCO__
#undef MY_SIGUSR1
#define MY_SIGUSR1 SIGUSR2
#define SIGURG SIGUSR1
#endif

#ifdef USE_SIGPROCMASK
static struct sigaction sa_act, sa_oact;
#define signal(s,f)			\
  (sa_act.sa_handler = (f),		\
   sigemptyset(&sa_act.sa_mask),	\
   sa_act.sa_flags = 0,			\
   sigaction((s), &sa_act, &sa_oact),	\
   sa_oact.sa_handler)
#endif

extern char *getenv();

char	*name;
int	rem;
kstream krem;
char	cmdchar = '~';
int	eight = 1;		/* Default to 8 bit transmission */
int	no_local_escape = 0;
int	null_local_username = 0;
int	flow = 0;		/* by default, let server to flow control */
int	confirm = 0;			/* ask if ~. is given before dying. */
int	litout = 0;
int	alt_port = 0;		/* port to contact rlogind, for testing*/
#ifdef hpux
char	*speeds[] =
    { 	"0", "50", "75", "110", "134", "150", "200", "300",
	"600", "900", "1200", "1800", "2400", "3600", "4800", "7200", 
	"9600", "19200", "38400", "57600", "115200", "230400", "460800" };
#else
char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#endif
char	term[256] = "network";
extern	int errno;
sigtype	lostpeer();		/* Sigtype is from krb.h, osconf.h, c-*.h */
int	dosigwinch = 0;
#ifndef sigmask
#define sigmask(m)	(1 << ((m)-1))
#endif
#ifdef NO_WINSIZE
struct winsize {
	unsigned short ws_row, ws_col;
	unsigned short ws_xpixel, ws_ypixel;
};
#endif
struct	winsize winsize;

sigtype	sigwinch();
sigtype	oob();

char	*host;				/* external, so it can be
					   reached from confirm_death() */

/* so we don't need exit declared as a function: */
sigtype exit_rlogin()
{
  exit(1);
}

#ifdef KERBEROS
void try_normal();
char krb_realm[REALM_SZ];
#ifndef NOENCRYPTION
int encrypt_flag = 0;
#endif /* NOENCRYPTION */
CREDENTIALS cred;
Key_schedule schedule;
MSG_DAT msg_data;
struct sockaddr_in local, foreign;
#include "rpaths.h"
#endif /* KERBEROS */

/*
 * The following routine provides compatibility (such as it is)
 * between 4.2BSD Suns and others.  Suns have only a `ttysize',
 * so we convert it to a winsize.
 */
#ifdef TIOCGWINSZ
#define get_window_size(fd, wp)	ioctl(fd, TIOCGWINSZ, wp)
#else
int
get_window_size(fd, wp)
	int fd;
	struct winsize *wp;
{
	struct ttysize ts;
	int error;

	if ((error = ioctl(0, TIOCGSIZE, &ts)) != 0)
		return (error);
	wp->ws_row = ts.ts_lines;
	wp->ws_col = ts.ts_cols;
	wp->ws_xpixel = 0;
	wp->ws_ypixel = 0;
	return (0);
}
#endif /* TIOCGWINSZ */

char *krb_realmofhost();

main(argc, argv)
	int argc;
	char **argv;
{
  	char *cp = (char *) NULL;
#ifdef POSIX
	struct termios ttyb;
#else
	struct sgttyb ttyb;
#endif
	u_short sps_port;
	struct passwd *pwd;
	struct servent *sp;
	int uid, options = 0;
	sigmasktype oldmask;
	int on = 1;
#ifdef KERBEROS
	KTEXT_ST ticket;
	char password[BUFSIZ];
	char **orig_argv = argv;
	int sock;
	long authopts;
	int through_once = 0;
	int ok;
#endif

	host = strrchr(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
#ifdef KERBEROS
	krb_realm[0] = '\0';
#endif
	if (strstr(host, "rlogin") || !strcmp(host, "cygin"))
		host = NULL;
another:
	if (argc > 0 && host == NULL && **argv != '-') {
		host = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
#ifndef NOENCRYPTION
		extern int _kstream_des_debug_OOB;
		_kstream_des_debug_OOB = 1;
#endif
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-c")) {
		confirm = 1;
		argv++; argc--;
		goto another;
		}
	if (argc > 0 && !strcmp(*argv, "-C")) {
		confirm = 0;
		argv++; argc--;
		goto another;
		}
	if (argc > 0 && !strcmp(*argv, "-a")) {	   /* ask -- make remote */
		argv++; argc--;			/* machine ask for password */
		null_local_username = 1;	/* by giving null local user */
		goto another;			/* id */
	}
	if (argc > 0 && !strcmp(*argv, "-t")) {
		argv++; argc--;
		if (argc == 0) goto usage;
		cp = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		no_local_escape = 1;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-7")) {  /* Pass only 7 bits */
		eight = 0;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-noflow")) {
		flow = 0;		/* Turn off local flow control so
					   that ^S can be passed to emacs. */
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-flow")) {
		flow = 1;		/* Turn on local flow control */
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-L")) {
		litout = 1;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-p", 2)) {
		alt_port = atoi(argv[0]+2);
		argv++, argc--;
		goto another;
	}
#ifdef KERBEROS
	if (argc > 0 && !strcmp(*argv, "-k")) {
	        argv++, argc--;
		if (argc == 0) {
		  fprintf(stderr, "rlogin: -k flag must be followed with a realm name.\n");
		  exit (1);
		}
		strncpy(krb_realm, *argv, REALM_SZ);
		argv++, argc--;
		goto another;
	}
#ifndef NOENCRYPTION
	if (argc > 0 && !strcmp(*argv, "-x")) {
		encrypt_flag++;
		argv++, argc--;
		goto another;
	}
#endif
#endif /* KERBEROS */
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "Who are you?\n");
		exit(1);
	}
#ifdef KERBEROS
	/*
	 * if there is an entry in /etc/services for Kerberos login,
	 * attempt to login with Kerberos. 
	 * If we fail at any step,  use the standard rlogin
	 */
#ifndef NOENCRYPTION
	if (encrypt_flag)
		sp = getservbyname("eklogin","tcp");
	else
#endif /* NOENCRYPTION */
		sp = getservbyname("klogin","tcp");
	if (sp == 0) {
#ifdef NOENCRYPTION
		sps_port = htons(KLOGIN_PORT); /* klogin/tcp */
#else
		sps_port = htons(encrypt_flag ? EKLOGIN_PORT : KLOGIN_PORT);
		/* eklogin : klogin */
#endif /* NOENCRYPTION */
	} else {
		sps_port = sp->s_port;
	}

#else
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		sps_port = htons(UCB_LOGIN_PORT); /* login/tcp */
	}
#endif /* KERBEROS */
	/* If alt_port, override the value, to avoid making the ifdef's
	   above even harder to follow... */
	if (alt_port) {
		sps_port = htons(alt_port);
	}
	if (cp == (char *) NULL) cp = getenv("TERM");
	if (cp)
		(void) strcpy(term, cp);
#ifdef POSIX
	if (tcgetattr(0, &ttyb) == 0) {
		int ospeed = cfgetospeed (&ttyb);

		(void) strcat(term, "/");
		if (ospeed >= 50)
			/* On some systems, ospeed is the baud rate itself,
			   not a table index.  */
			sprintf (term + strlen (term), "%d", ospeed);
		else {
#ifdef CBAUD
/* some "posix" systems don't have cfget... so used CBAUD if it's there */
			(void) strcat(term, speeds[ttyb.c_cflag & CBAUD]);
#else
			(void) strcat(term, speeds[cfgetospeed(&ttyb)]);
#endif
		}
	}
#else
	if (ioctl(0, TIOCGETP, &ttyb) == 0) {
		(void) strcat(term, "/");
		(void) strcat(term, speeds[ttyb.sg_ospeed]);
	}
#endif
	(void) get_window_size(0, &winsize);
	(void) signal(SIGPIPE, lostpeer);
	/* will use MY_SIGUSR1 for window size hack, so hold it off */
#ifdef SIGURG
	SIGBLOCK2 (oldmask, SIGURG, MY_SIGUSR1);

	/* The default action for SIGURG is to ignore it.  When an
           ignored signal is received, it is discarded, even if it is
           blocked.  To avoid the race condition between discarding
           the signal and leaving it pending until it is caught, we
           set a signal handler for SIGURG now.  This signal handler
           will be set to the correct value before the signal is
           unblocked.  */
	(void) signal (SIGURG, exit_rlogin);
#else
	SIGBLOCK (oldmask, MY_SIGUSR1);
#endif

#ifdef KERBEROS
	rem=KSUCCESS;
#ifndef NOENCRYPTION
	if (encrypt_flag) {
		authopts = KOPT_DO_MUTUAL;
	} else
#endif /* NOENCRYPTION */
	{
		authopts = 0L;
	}

	/* default this now, once. */
	if (krb_realm[0] == '\0') {

	  
	  if (!(cp = krb_realmofhost (host))) {
	    fprintf(stderr, "%s: Can't devine realm for host %s.  Use -k to specify.\n",
		    orig_argv[0], host);
	    goto ulose;
	  }

	  strncpy(krb_realm, cp, REALM_SZ);
	}

	/* Before we call kcmd, see if we have an appropriate ticket
	   which has expired.  If we have no tickets, kcmd will simply
	   return NO_TKT_FIL.  If we don't have the right ticket, and
	   our TGT has expired, kcmd will return RD_AP_EXP.  However,
	   if we do have the right ticket, but it happens to have
	   expired, kcmd will blithely send it along to the remote
	   system and return KSUCCESS.  So, instead, we check for an
	   expired ticket now.  Perhaps this type of check should be
	   pushed into krb_sendauth.  On the other hand, this is one
	   of the few programs which currently takes any special
	   action for an expired ticket.  */
	rem = KSUCCESS;
	{
	  char inst[INST_SZ];
	  CREDENTIALS cr;

	  strncpy(inst, krb_get_phost(host), INST_SZ);
	  rem = krb_get_cred("rcmd", inst, krb_realm, &cr);
	  if (rem == KSUCCESS
	      && (cr.issue_date + ((unsigned char) cr.lifetime) * 5 * 60
		  < time (0)))
	    rem = RD_AP_EXP;
	  else
	    /* Call kcmd, which will try to use the TGT to get a new
	       ticket.  */
	    rem = KSUCCESS;
	}

      tryagain:
	if (rem == KSUCCESS)
	  rem = kcmd(&sock, &host, sps_port,
		     null_local_username ? NULL : pwd->pw_name,
		     name ? name : pwd->pw_name, term,
		     0, &ticket, "rcmd", krb_realm,
		     &cred, schedule, &msg_data, &local, &foreign,
		     authopts);
	if (rem != KSUCCESS) {
	    switch(rem) {
	    case KDC_PR_UNKNOWN: /* Assume the foreign principal isn't
				    registered.  */
	      fprintf(stderr, "%s: Host %s isn't registered for Kerberos rlogin service\n",
		      orig_argv[0], host);
	      break;
	    case NO_TKT_FIL:
	    case RD_AP_EXP:
	      if (through_once++) goto ulose;

	      if (rem == RD_AP_EXP)
		printf("%s: Your Kerberos tickets have expired\n",
		       orig_argv[0]);
	      else if (strcmp(orig_argv[0], "cygin") != 0)
		printf("%s: You have no Kerberos tickets\n", orig_argv[0]);

	      {
		char buf[MAXHOSTNAMELEN];

		buf[0] = '\0';
		if (GETHOSTNAME(buf, MAXHOSTNAMELEN))
		  buf[0] = '\0';
		if (buf[0] != '\0')
		  printf("Kerberos initialization on %s\n", buf);
		else
		  printf("Kerberos initialization\n");
	      }

	      printf("Caution: do not enter your Kerberos password if you are typing on an\n");
	      printf("         unencrypted connection to a remote machine.  Type ^C instead.\n");

	      ok = des_read_pw_string (password, sizeof(password),
				       "Kerberos password: ", 0);
	      if (ok != 0) {
		 memset(password, 0, sizeof (password));
		 fprintf(stderr, "%s: could not read password\n",
			 orig_argv[0]);
		 break;
	      }

	      rem = krb_get_pw_in_tkt(name ? name : pwd->pw_name, "",
				      krb_realm, "krbtgt", krb_realm,
				      DEFAULT_TKT_LIFE/5, password);
	      if (rem != KSUCCESS) {
		fprintf(stderr, "%s: Reading password failed: %s.\n",
			orig_argv[0], krb_get_err_text(rem));
		break;
	      }

	      goto tryagain;
	    ulose:
	    default:
	      fprintf(stderr,
		      "%s: Kerberos rcmd failed: %s.\n",
		      orig_argv[0],
		      (rem == -1) ? "rcmd protocol failure" :
		      krb_get_err_text(rem));
	    }
	    rem = -1;
	}
	if (rem == -1) {
		SIGSETMASK(oldmask);
		try_normal(orig_argv);
	}
	rem = sock;
#ifndef NOENCRYPTION
	if (encrypt_flag)
	  krem = kstream_create_rlogin_from_fd (rem, &schedule, &cred.session);
	else
#endif
	  krem = kstream_create_from_fd (rem, 0, 0);
	kstream_set_buffer_mode (krem, 0);
#else
        rem = rcmd(&host, sps_port,
		   null_local_username ? NULL : pwd->pw_name,
	    name ? name : pwd->pw_name, term, 0);
        if (rem < 0)
                exit(1);
	krem = kstream_create_from_fd (rem, 0, 0);
#endif /* KERBEROS */
	/* we need to do the SETOWN here so that we get the SIGURG
	   registered if the URG data come in early, before the reader() gets
	   to do this for real (otherwise, the signal is never generated
	   by the kernel).  We block it above, so when it gets unblocked
	   it will get processed by the reader().
	   There is a possibility that the signal will get delivered to both
	   writer and reader, but that is harmless, since the writer reflects
	   it to the reader, and the oob() processing code in the reader will
	   work properly even if it is called when no oob() data is present.
	 */
#ifndef hpux
#ifndef __SCO__
#ifndef __svr4__
	(void) fcntl(rem, F_SETOWN, getpid());
#else
	(void) ioctl(rem, I_SETSIG, S_RDBAND | S_BANDURG);
#endif
#else /* SCO */
	{
		int pid = getpid();
		ioctl(rem, SIOCSPGRP, &pid);
	}
#endif /* SCO */
#else /* hpux */
	/* hpux invention */
	{
	  int pid = getpid();
	  ioctl(rem, FIOSSAIOSTAT, &pid); /* trick: pid is non-zero */
	  ioctl(rem, FIOSSAIOOWN, &pid);
	}
#endif /* hpux */
	if (options & SO_DEBUG &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		perror("rlogin: setsockopt (SO_DEBUG)");
	uid = getuid();
	if (setuid(uid) < 0) {
		perror("rlogin: setuid");
		exit(1);
	}
	doit(oldmask);
	/*NOTREACHED*/
usage:
#ifdef KERBEROS
	fprintf (stderr,
"usage: rlogin host [-option] [-option...] [-k realm ] [-t ttytype] [-l username]\n");
#ifdef NOENCRYPTION
    	fprintf (stderr, "     where option is e, 7, 8, noflow, n, a, or c\n");
#else /* !NOENCRYPTION */
    	fprintf (stderr, "     where option is e, 7, 8, noflow, n, a, x, or c\n");
#endif /* NOENCRYPTION */
#else /* !KERBEROS */
	fprintf (stderr,
"usage: rlogin host [-option] [-option...] [-t ttytype] [-l username]\n");
    	fprintf (stderr, "     where option is e, 7, 8, noflow, n, a, or c\n");
#endif /* KERBEROS */
	exit(1);
}

int
confirm_death ()
{
	char hostname[33];
	char input;
	int answer;
	if (!confirm) return (1);	/* no confirm, just die */

	if (gethostname (hostname, sizeof(hostname)-1) != 0)
		strcpy (hostname, "???");
	else
		hostname[sizeof(hostname)-1] = '\0';

	fprintf (stderr, "\r\nKill session on %s from %s (y/n)?  ",
			 host, hostname);
	fflush (stderr);
	if (read(0, &input, 1) != 1)
		answer = EOF;	/* read from stdin */
	else
		answer = (int) input;
	fprintf (stderr, "%c\r\n", answer);
	fflush (stderr);
	return (answer == 'y' || answer == 'Y' || answer == EOF ||
		answer == 4);	/* control-D */
}

#define CRLF "\r\n"

int	child;
sigtype	catchild();
#ifdef SIGURG
sigtype	copytochild();
#endif
sigtype	writeroob();

#ifdef TIOCGLTC
/*
 * POSIX 1003.1-1988 does not define a 'suspend' character.
 * POSIX 1003.1-1990 does define an optional VSUSP but not a VDSUSP character.
 * Some termio implementations (A/UX, Ultrix 4.2) include both.
 *
 * However, since this is all derived from the BSD ioctl() and ltchars
 * concept, all these implementations generally also allow for the BSD-style
 * ioctl().  So we'll simplify the problem by only testing for the ioctl().
 */
struct	ltchars defltc;
struct	ltchars noltc =	{ -1, -1, -1, -1, -1, -1 };
#endif

#ifndef POSIX
int	defflags, tabflag;
int	deflflags;
char	deferase, defkill;
struct	tchars deftc;
struct	tchars notc =	{ -1, -1, -1, -1, -1, -1 };
#endif

doit(oldmask)
     sigmasktype oldmask;
{
#ifdef POSIX
	(void) tcgetattr(0, &deftty);
#ifdef __svr4__
#ifdef VLNEXT
	/* there's a POSIX way of doing this, but do we need it general? */
	/* well, hpux 9.0 doesn't have VLNEXT. */
	deftty.c_cc[VLNEXT] = 0;
#endif
#endif
#ifdef TIOCGLTC
	(void) ioctl(0, TIOCGLTC, (char *)&defltc);
#endif
#else
	struct sgttyb sb;

	(void) ioctl(0, TIOCGETP, (char *)&sb);
	defflags = sb.sg_flags;
	tabflag = defflags & TBDELAY;
	defflags &= ECHO | CRMOD;
	deferase = sb.sg_erase;
	defkill = sb.sg_kill;
	(void) ioctl(0, TIOCLGET, (char *)&deflflags);
	(void) ioctl(0, TIOCGETC, (char *)&deftc);
	notc.t_startc = deftc.t_startc;
	notc.t_stopc = deftc.t_stopc;
	(void) ioctl(0, TIOCGLTC, (char *)&defltc);
#endif
	(void) signal(SIGINT, SIG_IGN);
	setsignal(SIGHUP, exit_rlogin);
	setsignal(SIGQUIT, exit_rlogin);
	child = fork();
	if (child == -1) {
		perror("rlogin: fork");
		done(1);
	}
	if (child == 0) {
		mode(1);
		if (reader(oldmask) == 0) {
			prf("Connection closed.");
			exit(0);
		}
		sleep(1);
		prf("\007Connection closed.");
		exit(3);
	}

	/*
	 * We may still own the socket, and may have a pending SIGURG
	 * (or might receive one soon) that we really want to send to
	 * the reader.  Set a trap that simply copies such signals to
	 * the child.
	 */
#ifdef SIGURG
	(void) signal(SIGURG, copytochild);
#endif
	(void) signal(MY_SIGUSR1, writeroob);
	SIGSETMASK(oldmask);
	(void) signal(SIGCHLD, catchild);
	writer();
	prf("Closed connection.");
	done(0);
}

/*
 * Trap a signal, unless it is being ignored.
 */
setsignal(sig, act)
	int sig;
	sigtype (*act)();
{
	sigmasktype mask;

	SIGBLOCK (mask, sig);
	if (signal(sig, act) == SIG_IGN)
		(void) signal(sig, SIG_IGN);
	SIGSETMASK (mask);
}

done(status)
	int status;
{
	int w;

	mode(0);
	if (child > 0) {
		/* make sure catchild does not snap it up */
		(void) signal(SIGCHLD, SIG_DFL);
		if (kill(child, SIGKILL) >= 0) {
			while ((w = wait(0)) > 0 && w != child)
				/*void*/;
		}
	}
	exit(status);
}

#ifdef SIGURG
/*
 * Copy SIGURGs to the child process.
 */
sigtype
copytochild()
{
	(void) kill(child, SIGURG);
}
#endif

/*
 * This is called when the reader process gets the out-of-band (urgent)
 * request to turn on the window-changing protocol.
 */
sigtype
writeroob()
{

	if (dosigwinch == 0) {
		sendwindow();
		(void) signal(SIGWINCH, sigwinch);
	}
	dosigwinch = 1;

#ifdef __svr4__
	/* At this point, we know that the reader is alive and
           catching signals.  We can stop reflecting SIGURG signals to
           it.  */
	(void) ioctl(rem, I_SETSIG, 0);
#endif
}

sigtype
catchild()
{
	int pid;
#if defined (POSIX) || defined (WAIT_USES_INT)
	int status;
#else
	union wait status;
#endif

#ifdef POSIX
#define wait3(st,opt,usage) waitpid(-1,st,opt)
#endif
again:
	pid = wait3(&status, WNOHANG|WUNTRACED, (struct rusage *)0);
	if (pid == 0)
		return;
	/*
	 * if the child (reader) dies, just quit
	 */
#if defined (POSIX) || defined (WAIT_USES_INT)
	if (pid < 0 || (pid == child && !WIFSTOPPED(status)))
		done(status);
#else
	if (pid < 0 || (pid == child && !WIFSTOPPED(status)))
		done((int)(status.w_termsig | status.w_retcode));
#endif
	goto again;
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 * ~^Y  suspend rlogin process, but leave reader alone.
 */
writer()
{
	char c;
	register int n;
	register int bol = 1;			/* beginning of line */
	register int local = 0;

#ifdef ultrix
	fd_set waitread;

	/* we need to wait until the reader() has set up the terminal, else
	   the read() below may block and not unblock when the terminal
	   state is reset.
	   */
	for (;;) {
	    FD_ZERO(&waitread);
	    FD_SET(0, &waitread);
	    n = select(1, &waitread, 0, 0, 0, 0);
	    if (n < 0 && errno == EINTR)
		continue;
	    if (n > 0)
		break;
	    else
		if (n < 0) {
		    perror("select");
		    break;
		}
	}
#endif /* ultrix */
	for (;;) {
		n = read(0, &c, 1);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
		/*
		 * If we're at the beginning of the line
		 * and recognize a command character, then
		 * we echo locally.  Otherwise, characters
		 * are echo'd remotely.  If the command
		 * character is doubled, this acts as a 
		 * force and local echo is suppressed.
		 */
		if (bol) {
			bol = 0;
			/* Allow NULL to mean "none."  We can't distinguish
			   the two cases, and we need a way to say "none" */
			if (cmdchar && c == cmdchar) {
				bol = 0;
				local = 1;
				continue;
			}
		} else if (local) {
			local = 0;
#ifdef POSIX
			if (c == '.' || c == deftty.c_cc[VEOF])
#else
			if (c == '.' || c == deftc.t_eofc)
#endif
			{
			    if (confirm_death())
			    {
				echo(c);
				break;
			    }
			}
#ifdef TIOCGLTC
			if ((c == defltc.t_suspc || c == defltc.t_dsuspc)
  				&& !no_local_escape) {
				bol = 1;
				echo(c);
				stop(c);
				continue;
			}
#else
#ifdef POSIX
			if ( (
			      (c == deftty.c_cc[VSUSP]) 
#ifdef VDSUSP
			      || (c == deftty.c_cc[VDSUSP]) 
#endif
			      )
			    && !no_local_escape) {
				bol = 1;
				echo(c);
				stop(c);
				continue;
			}
#endif
#endif
			if (c != cmdchar)
				(void) kstream_write(krem, &cmdchar, 1);
		}
		if (kstream_write(krem, &c, 1) == 0) {
			prf("line gone");
			break;
		}
#ifdef POSIX
		bol = (c == deftty.c_cc[VKILL] ||
		       c == deftty.c_cc[VINTR] ||
		       c == '\r' || c == '\n');
#ifdef TIOCGLTC
		if (!bol)
			bol = (c == defltc.t_suspc);
#endif
#else /* !POSIX */
		bol = c == defkill || c == deftc.t_eofc ||
		    c == deftc.t_intrc || c == defltc.t_suspc ||
		    c == '\r' || c == '\n';
#endif
	}
}

echo(c)
register char c;
{
	char buf[8];
	register char *p = buf;

	c &= 0177;
	*p++ = cmdchar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if (c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	(void) write(1, buf, p - buf);
}

stop(cmdc)
	char cmdc;
{
	mode(0);
	(void) signal(SIGCHLD, SIG_IGN);
#ifdef TIOCGLTC
	(void) kill(cmdc == defltc.t_suspc ? 0 : getpid(), SIGTSTP);
#else
#ifdef POSIX
        (void) kill(cmdc == deftty.c_cc[VSUSP] ? 0 : getpid(), SIGTSTP);
#endif
#endif
	(void) signal(SIGCHLD, catchild);
	mode(1);
	sigwinch();			/* check for size changes */
}

sigtype
sigwinch()
{
	struct winsize ws;

	if (dosigwinch && get_window_size(0, &ws) == 0 &&
	    memcmp(&ws, &winsize, sizeof (ws))) {
		winsize = ws;
		sendwindow();
	}
}

/*
 * Send the window size to the server via the magic escape
 */
sendwindow()
{
	char obuf[4 + sizeof (struct winsize)];
	struct winsize *wp = (struct winsize *)(obuf+4);

	obuf[0] = (char)0377;
	obuf[1] = (char)0377;
	obuf[2] = 's';
	obuf[3] = 's';
	wp->ws_row = htons(winsize.ws_row);
	wp->ws_col = htons(winsize.ws_col);
	wp->ws_xpixel = htons(winsize.ws_xpixel);
	wp->ws_ypixel = htons(winsize.ws_ypixel);
	(void) kstream_write(krem, obuf, sizeof(obuf));
}

/*
 * reader: read from remote: line -> 1
 */
#define	READING	1
#define	WRITING	2

char	rcvbuf[8 * 1024];
int	rcvcnt;
int	rcvstate;
int	ppid;
jmp_buf	rcvtop;

sigtype
oob()
{
	int atmark, n;
	int rcvd = 0;
	char waste[BUFSIZ], mark;
#ifdef POSIX
	struct termios tty;
#else
	int out = FWRITE;
	struct sgttyb sb;
#endif

	while (recv(rem, &mark, 1, MSG_OOB) < 0)
		switch (errno) {
		
		  /* We don't try to handle EWOULDBLOCK on SVR4
                     because 1) it is not necessary, since we will
                     only get SIGURG when the message is at the head
                     of the queue, and 2) on SVR4, we often get
                     duplicate SIGURG signals, because both the parent
                     and the child have called I_SETSIG, and this code
                     can not handle encrypted data.  */
#ifndef __svr4__
		case EWOULDBLOCK:
			/*
			 * Urgent data not here yet.
			 * It may not be possible to send it yet
			 * if we are blocked for output
			 * and our input buffer is full.
			 * Don't do this when encrypting, because it just 
			 * leads to confusion.
			 */
		  	if (encrypt_flag)
				return;
			if (rcvcnt < sizeof(rcvbuf)) {
				n = read(rem, rcvbuf + rcvcnt,
					sizeof(rcvbuf) - rcvcnt);
				if (n <= 0)
					return;
				rcvd += n;
			} else {
				n = read(rem, waste, sizeof(waste));
				if (n <= 0)
					return;
			}
			continue;
#endif /* ! defined (__svr4__) */
				
		default:
			return;
	}
#ifndef NOENCRYPTION
	{
	  extern int _kstream_des_debug_OOB;
	  if (_kstream_des_debug_OOB)
	    /* spew debug messages here too */
	    {
	      char msgbuf[100];
	      char *p = msgbuf;
	      unsigned char x = mark;
	      sprintf (msgbuf, "\r\n [ oob %02x : ", x);
	      if (x & TIOCPKT_WINDOW)
		strcat (p, "WINDOW ");
	      if (x & TIOCPKT_NOSTOP)
		strcat (msgbuf, "NOSTOP ");
	      if (x & TIOCPKT_DOSTOP)
		strcat (msgbuf, "DOSTOP ");
	      if (x & TIOCPKT_FLUSHWRITE)
		strcat (msgbuf, "FLUSHWRITE ");
	      x &= ~ (TIOCPKT_WINDOW | TIOCPKT_NOSTOP | TIOCPKT_DOSTOP | TIOCPKT_FLUSHWRITE);
	      if (x)
		sprintf (msgbuf + strlen (msgbuf), "+ 0x%x", x);
	      strcat (msgbuf, "]\r\n");
	      fputs (msgbuf, stderr);
	      if (x & TIOCPKT_FLUSHWRITE)
		/* Try to let the message actually get printed out...  */
		sleep (2);
	    }
	}
#endif
	if (mark & TIOCPKT_WINDOW) {
		/*
		 * Let server know about window size changes
		 */
		(void) kill(ppid, MY_SIGUSR1);
	}
#ifdef POSIX
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		(void) tcgetattr(0, &tty);
		tty.c_iflag &= ~IXON;
		(void) tcsetattr(0, TCSADRAIN, &tty);
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		(void) tcgetattr(0, &tty);
		tty.c_iflag |= IXON;
		(void) tcsetattr(0, TCSADRAIN, &tty);
	}
#else
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		(void) ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~CBREAK;
		sb.sg_flags |= RAW;
		(void) ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = -1;
		notc.t_startc = -1;
		(void) ioctl(0, TIOCSETC, (char *)&notc);
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		(void) ioctl(0, TIOCGETP, (char *)&sb);
		sb.sg_flags &= ~RAW;
		sb.sg_flags |= CBREAK;
		(void) ioctl(0, TIOCSETN, (char *)&sb);
		notc.t_stopc = deftc.t_stopc;
		notc.t_startc = deftc.t_startc;
		(void) ioctl(0, TIOCSETC, (char *)&notc);
	}
#endif
	if (mark & TIOCPKT_FLUSHWRITE) {
#ifdef POSIX
		(void) tcflush(1, TCOFLUSH);
#else
		(void) ioctl(1, TIOCFLUSH, (char *)&out);
#endif
		if (encrypt_flag) {
			/* When encrypting, data must be read in
                           blocks.  If we read up to the mark, we will
                           most likely wind up in the middle of a
                           block, confusing the output.  Also, we
                           definitely don't want to longjmp out of the
                           decryption code.  */
			rcvcnt = 0;
			if (rcvstate == WRITING)
				longjmp(rcvtop, 1);
		} else {
			for (;;) {
				if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
					perror("ioctl");
					break;
				}
				if (atmark)
					break;
				n = read(rem, waste, sizeof (waste));
				if (n <= 0)
					break;
			}
			/*
			 * Don't want any pending data to be output,
			 * so clear the recv buffer.
			 * If we were hanging on a write when interrupted,
			 * don't want it to restart.  If we were reading,
			 * restart anyway.
			 */
			rcvcnt = 0;
			longjmp(rcvtop, 1);
		}
	}

	/*
	 * oob does not do FLUSHREAD (alas!)
	 */

	/*
	 * If we filled the receive buffer while a read was pending,
	 * longjmp to the top to restart appropriately.  Don't abort
	 * a pending write, however, or we won't know how much was written.
	 */
	if (rcvd && rcvstate == READING)
		longjmp(rcvtop, 1);
}

/*
 * reader: read from remote: line -> 1
 */
reader(oldmask)
	sigmasktype oldmask;
{
	int n, remaining;
	char *bufp = rcvbuf;
#ifdef __SCO__
	int zerocount = 0;
#endif

	(void) signal(SIGTTOU, SIG_IGN);
#ifdef SIGURG
	(void) signal(SIGURG, oob);
#endif
	ppid = getppid();
#ifndef hpux
#ifndef __SCO__
#ifndef __svr4__
	(void) fcntl(rem, F_SETOWN, getpid());
#else
	(void) ioctl(rem, I_SETSIG, S_RDBAND | S_BANDURG);
#endif /* ! __svr4__ */
#else /* __SCO__ */
	{
		int pid = getpid();
		ioctl(rem, SIOCSPGRP, &pid);
	}
#endif /* __SCO__ */
#else /* hpux */
	{
		int fioflag = 1;
		/* hpux invention */
		ioctl(rem, FIOSSAIOSTAT, &fioflag);
		ioctl(rem, FIOSSAIOOWN, &fioflag);
	}
#endif /* hpux */ 
	(void) setjmp(rcvtop);
	SIGSETMASK(oldmask);
	for (;;) {
		while ((remaining = rcvcnt - (bufp - rcvbuf)) > 0) {
			rcvstate = WRITING;
			n = write(1, bufp, remaining);
			if (n < 0) {
				if (errno != EINTR)
					return (-1);
				continue;
			}
			bufp += n;
		}
		bufp = rcvbuf;
		rcvcnt = 0;
		rcvstate = READING;
		rcvcnt = kstream_read(krem, rcvbuf, sizeof (rcvbuf));
		if (rcvcnt == 0) {
#ifdef __SCO__
			/* On SCO, read will return zero when it hits
                           OOB data.  We don't return unless we read
                           zero ten times in a row.  I'm not sure how
                           else to handle this; I tried checking
                           SIOCATMARK, but it did not work.  The
                           signal will sometimes arrive well after the
                           zero read, so we can't rely on that.  */
			if (zerocount < 10) {
				++zerocount;
				continue;
			}
#endif
			return (0);
		}
		if (rcvcnt < 0) {
			if (errno == EINTR)
				continue;
			perror("read");
			return (-1);
		}
#ifdef __SCO__
		zerocount = 0;
#endif
	}
}

mode(f)
	int f;
{
#ifdef POSIX
	struct termios newtty;

	switch(f) {
	case 0:
#ifdef TIOCGLTC
#ifndef solaris20
		(void) ioctl(0, TIOCSLTC, (char *)&defltc);
#endif
#endif
		(void) tcsetattr(0, TCSADRAIN, &deftty);
		break;
	case 1:
		(void) tcgetattr(0, &newtty);
#ifdef __svr4__
#ifdef VLNEXT
	/* there's a POSIX way of doing this, but do we need it general? */
	/* well, hpux 9.0 doesn't have VLNEXT. */
		newtty.c_cc[VLNEXT] = 0;
#endif
#endif
		
		newtty.c_lflag &= ~(ICANON|ISIG|ECHO);
		if (!flow)
		{
			newtty.c_lflag &= ~(ICANON|ISIG|ECHO);
			newtty.c_iflag &= ~(BRKINT|INLCR|ICRNL|ISTRIP);
			/* newtty.c_iflag |=  (IXON|IXANY); */
			newtty.c_iflag &= ~(IXON|IXANY);
			newtty.c_oflag &= ~(OPOST);
		} else {
			newtty.c_lflag &= ~(ICANON|ISIG|ECHO);
			newtty.c_iflag &= ~(INLCR|ICRNL);
			/* newtty.c_iflag |=  (BRKINT|ISTRIP|IXON|IXANY); */
			newtty.c_iflag &= ~(IXON|IXANY);
			newtty.c_iflag |=  (BRKINT|ISTRIP);
			newtty.c_oflag &= ~(ONLCR|ONOCR);
			newtty.c_oflag |=  (OPOST);
		}
		/* preserve tab delays, but turn off XTABS */
		if ((newtty.c_oflag & TABDLY) == TAB3)
			newtty.c_oflag &= ~TABDLY;

		if (litout)
			newtty.c_oflag &= ~OPOST;

		newtty.c_cc[VMIN] = 1;
		newtty.c_cc[VTIME] = 0;
		(void) tcsetattr(0, TCSADRAIN, &newtty);
#ifdef TIOCGLTC
		/* Do this after the tcsetattr() in case this version
		 * of termio supports the VSUSP or VDSUSP characters */
#ifndef solaris20
		/* this forces ICRNL under Solaris... */
		(void) ioctl(0, TIOCSLTC, (char *)&noltc);
#endif
#endif
		break;
	default:
		return;
		/* NOTREACHED */
	}
#else
	struct tchars *tc;
	struct ltchars *ltc;
	struct sgttyb sb;
	int	lflags;

	(void) ioctl(0, TIOCGETP, (char *)&sb);
	(void) ioctl(0, TIOCLGET, (char *)&lflags);
	switch (f) {

	case 0:
		sb.sg_flags &= ~(CBREAK|RAW|TBDELAY);
		sb.sg_flags |= defflags|tabflag;
		tc = &deftc;
		ltc = &defltc;
		sb.sg_kill = defkill;
		sb.sg_erase = deferase;
		lflags = deflflags;
		break;

	case 1:
  		sb.sg_flags &= ~(CBREAK|RAW);
  		sb.sg_flags |= (!flow ? RAW : CBREAK);
		sb.sg_flags &= ~defflags;
		/* preserve tab delays, but turn off XTABS */
		if ((sb.sg_flags & TBDELAY) == XTABS)
			sb.sg_flags &= ~TBDELAY;
		tc = &notc;
		ltc = &noltc;
		sb.sg_kill = sb.sg_erase = -1;
		if (litout)
			lflags |= LLITOUT;
#if defined(LPASS8)
		if (eight)
			lflags |= LPASS8;
#endif
		break;

	default:
		return;
	}
	(void) ioctl(0, TIOCSLTC, (char *)ltc);
	(void) ioctl(0, TIOCSETC, (char *)tc);
	(void) ioctl(0, TIOCSETN, (char *)&sb);
	(void) ioctl(0, TIOCLSET, (char *)&lflags);
#endif /* !POSIX */
}

/*VARARGS*/
prf(f, a1, a2, a3, a4, a5)
	char *f;
{

	fprintf(stderr, f, a1, a2, a3, a4, a5);
	fprintf(stderr, CRLF);
}

#ifdef KERBEROS
void
try_normal(argv)
char **argv;
{
	register char *host;

#ifndef NOENCRYPTION
	if (encrypt_flag)
		exit(1);
#endif
	fprintf(stderr,"trying normal rlogin (%s)\n",
		UCB_RLOGIN);
	fflush(stderr);

	host = strrchr(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	/* Rlogin is sensitive to the name under which it runs.
	   (If argv[0] != rlogin, it assumes that's the host name.)
	   So if argv[0] is not rlogin and not a host name, fake it.  */
	if (!strcmp(host,"cygin"))
		argv[0] = "rlogin";

	execv(UCB_RLOGIN, argv);
	perror("exec");
	exit(1);
}
#endif /* KERBEROS */

sigtype
lostpeer()
{

	(void) signal(SIGPIPE, SIG_IGN);
	prf("\007Connection closed.");
	done(1);
}
