/*
 * login.c
 */

/*
 * Copyright (c) 1980, 1987, 1988 The Regents of the University of California.
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
"@(#) Copyright (c) 1980, 1987, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)login.c	5.25 (Berkeley) 1/6/89";
#endif /* not lint */

/*
 * login [ name ]
 * login -r hostname	(for rlogind)
 * login -h hostname	(for telnetd, etc.)
 * login -f name	(for pre-authenticated login: datakit, xterm, etc.)
 * ifdef KERBEROS
 * login -e name	(for pre-authenticated encrypted, must do term
 *			 negotiation)
 * login -k hostname (for Kerberos rlogind with password access)
 * login -K hostname (for Kerberos rlogind with restricted access)
 *   either -k or -K may be preceded by -s srvtab to set the name of
 *   the srvtab to use; the default is KEYFILE, normally /etc/krb-srvtab
 * endif KERBEROS */

#include "conf.h"
#include <sys/types.h>
#include <sys/param.h>
#ifdef OQUOTA
#include <sys/quota.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#include <utmp.h>
#include <signal.h>

#ifndef NO_LASTLOG
#include <lastlog.h>
#endif

#ifdef linux
/* linux has V* but not C* in headers. Perhaps we shouldn't be
 * initializing these values anyway -- tcgetattr *should* give
 * them reasonable defaults... */
#define NO_INIT_CC
#endif

#ifdef POSIX
#define POSIX_TERMIOS
#endif

#include <errno.h>
#ifndef NOTTYENT
#include <ttyent.h>
#endif /* NOTTYENT */
#include <syslog.h>
/* Alpha OSF/1 2.0 wants stdio.h before grp.h.  */
#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <setjmp.h>
#include <string.h>
#ifdef KERBEROS
#include <krb.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef BIND_HACK
#include <arpa/nameser.h>
#include <arpa/resolv.h>
#endif /* BIND_HACK */
#ifdef KRB5
#include <krb5.h>
#endif
#endif /* KERBEROS */
#include "loginpaths.h"

#ifdef POSIX
#include <stdlib.h>
#endif
#ifdef POSIX_TERMIOS
#include <termios.h>
#ifdef _AIX
#include <termio.h>
#endif
#endif
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif
#ifdef __SCO__
#include <sys/unistd.h>
/* for struct winsize */
#include <sys/stream.h>
#include <sys/ptem.h>
#endif

#ifdef __386BSD__
#include <sys/ioctl_compat.h>
#endif

#ifdef sgi
#define _AIX
#endif

#ifdef hpux
#define _AIX
#define setreuid(r,e) setresuid(r,e,-1)
#endif
#ifdef __svr4__
#define setreuid(r,e) setuid(r)
#endif

#if defined(hpux) || defined(__svr4__)
#include <sys/resource.h>
int getdtablesize() {
  struct rlimit rl;
  getrlimit(RLIMIT_NOFILE, &rl);
  return rl.rlim_cur;
}
#endif

#if defined(_AIX)
#define PRIO_OFFSET 20
#else
#define PRIO_OFFSET 0
#endif

#ifdef UIDGID_T
uid_t getuid();
#define uid_type uid_t
#define gid_type gid_t
#else
#define uid_type int
#define gid_type int
#endif /* UIDGID_T */

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

#define SETPAG

#define	TTYGRPNAME	"tty"		/* name of group to own ttys */

#define	MOTDFILE	"/etc/motd"
#define	MAILDIR		"/usr/spool/mail"
#define	NOLOGIN		"/etc/nologin"
#define	HUSHLOGIN	".hushlogin"
#define	LASTLOG		"/usr/adm/lastlog"
#define	BSHELL		"/bin/sh"

#if !defined(OQUOTA) && !defined(QUOTAWARN)
#define QUOTAWARN	"/usr/ucb/quota" /* warn user about quotas */
#endif

#define PROTOTYPE_DIR	"/usr/athena/lib/prototype_tmpuser"
#define TEMP_DIR_PERM	0711

#define NOATTACH	"/etc/athena/noattach"
#define NOCREATE	"/etc/athena/nocreate"
#define NOREMOTE	"/etc/athena/noremote"
#define REGISTER	"/usr/etc/go_register"
#define GET_MOTD	"/bin/athena/get_message"

#ifndef NOUTHOST
#ifndef UT_HOSTSIZE
/* linux defines it directly in <utmp.h> */
#define	UT_HOSTSIZE	sizeof(((struct utmp *)0)->ut_host)
#endif
#endif
#ifndef UT_NAMESIZE
/* linux defines it directly in <utmp.h> */
#define	UT_NAMESIZE	sizeof(((struct utmp *)0)->ut_name)
#endif

#define MAXENVIRON	32

/*
 * This bounds the time given to login.  Not a define so it can
 * be patched on machines where it's too small.
 */
int	timeout = 300;

struct passwd *pwd;
char term[64], *hostname, *username;

#ifndef POSIX_TERMIOS
struct sgttyb sgttyb;
struct tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};
#endif

extern int errno;

#ifdef KERBEROS
#define KRB_ENVIRON	"KRBTKFILE"	/* Ticket file environment variable */
#define KRB_TK_DIR	"/tmp/tkt_"	/* Where to put the ticket */

#ifdef KRB5
#define KRB5_ENVIRON	"KRB5CCNAME"	/* Ticket file environment variable */
#define KRB5_TK_DIR	"/tmp/krb5cc_"	/* Where to put the ticket */
#endif

#define MAXPWSIZE	128		/* Biggest string accepted for Kerberos
					   passsword */

AUTH_DAT *kdata;
char tkfile[MAXPATHLEN];
#ifdef KRB5
char tk5file[MAXPATHLEN];
#endif
int krbflag;			/* Set if Kerberos tickets are
				   hanging around. */
#ifdef SETPAG
int pagflag;			/* true if setpag() has been called */
#endif
char *srvtab = "";
char *getenv();
#ifndef HAVE_STRSAVE
static char *strsave();
#endif
void dofork();
time_t time();
#endif /* KERBEROS */

#define EXCL_TEST if (rflag || kflag || Kflag || eflag || hflag) { \
				fprintf(stderr, \
				    "login: only one of -r, -k, -K, -e and -h allowed.\n"); \
				exit(1);\
			}

off_t lseek();

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	extern char *optarg, **environ;
	struct group *gr;
	register int ch, i;
	register char *p;
	int fflag, hflag, pflag, rflag, cnt;
	int kflag, Kflag, eflag;
	int quietlog, passwd_req, ioctlval;
	sigtype timedout();
	char *domain, *salt, **envinit, *ttyn, *tty, *ktty, *tz;
	char tbuf[MAXPATHLEN + 2];
	char *ttyname(), *stypeof(), *crypt(), *getpass();
	time_t login_time;
#ifdef POSIX_TERMIOS
	struct termios tc;
#endif

	(void)signal(SIGALRM, timedout);
	(void)alarm((u_int)timeout);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
#ifndef NO_SETPRIORITY
	(void)setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif
#ifdef OQUOTA
	(void)quota(Q_SETUID, 0, 0, 0);
#endif

	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
 	 * -f is used to skip a second login authentication 
	 * -e is used to skip a second login authentication, but allows
	 * 	login as root.
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 * -k is used by klogind to cause the Kerberos autologin protocol;
	 * -K is used by klogind to cause the Kerberos autologin protocol with
	 *    restricted access.;
	 *   either -k or -K may be preceded by -s srvtab to set the
	 *   name of the srvtab to use; the default is KEYFILE,
	 *   normally /etc/krb-srvtab
	 */
	(void)gethostname(tbuf, sizeof(tbuf));
	domain = strchr(tbuf, '.');

	fflag = hflag = pflag = rflag = kflag = Kflag = eflag = 0;
	passwd_req = 1;
	while ((ch = getopt(argc, argv, "fe:h:pr:k:K:s:")) != EOF)
		switch (ch) {
		case 'f':
			/* Do not disallow -f and other options (-h)! */
			fflag = 1;
			break;
		case 'h':
			EXCL_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -h for super-user only.\n");
				exit(1);
			}
			hflag = 1;
			if (domain && (p = strchr(optarg, '.')) &&
			    strcmp(p, domain) == 0)
				*p = 0;
			hostname = optarg;
			break;
		case 'p':
			pflag = 1;
			break;
		case 'r':
			EXCL_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -r for super-user only.\n");
				exit(1);
			}
			/* "-r hostname" must be last args */
			if (optind != argc) {
				fprintf(stderr, "Syntax error.\n");
				exit(1);
			}
			rflag = 1;
			passwd_req = (doremotelogin(optarg) == -1);
			if (domain && (p = strchr(optarg, '.')) &&
			    !strcmp(p, domain))
				*p = '\0';
			hostname = optarg;
			break;
#ifdef KERBEROS
		case 'k':
		case 'K':
			EXCL_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -%c for super-user only.\n", ch);
				exit(1);
			}
			/* "-k hostname" must be last args */
			if (optind != argc) {
				int ii;
				fprintf(stderr, "Syntax error: %d != %d\n\r",optind,argc);
				for(ii = 0;ii<argc;ii++) {
				  fprintf(stderr, "[%d: %s]\n\r",
					  ii, argv[ii]);
				}
				fflush(stderr);
				sleep(1);
				exit(1);
			}
			if (ch == 'K')
			    Kflag = 1;
			else
			    kflag = 1;
			passwd_req = (do_krb_login(optarg,
						   Kflag ? 1 : 0) == -1);
			if (domain && (p = strchr(optarg, '.')) &&
			    !strcmp(p, domain))
				*p = '\0';
			hostname = optarg;
			break;
		case 'e':
			EXCL_TEST;
			if (getuid()) {
			    fprintf(stderr,
				    "login: -e for super-user only.\n");
			    exit(1);
			}
			eflag = 1;
			passwd_req = 0;
			if (domain && (p = strchr(optarg, '.')) &&
			    strcmp(p, domain) == 0)
				*p = 0;
			hostname = optarg;
			break;
		case 's':
			srvtab = optarg;
			break;
#endif /* KERBEROS */
		case '?':
		default:
			fprintf(stderr, "usage: login [-fp] [username]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;
	if (*argv)
		username = *argv;

#if !defined(_AIX)
	ioctlval = 0;
#ifndef __SCO__
#ifndef linux
/* linux *has* line disciplines, but there's no user interface yet */
#ifdef TIOCLSET
	(void)ioctl(0, TIOCLSET, (char *)&ioctlval);
#endif
#endif
#endif
	(void)ioctl(0, TIOCNXCL, (char *)0);
	(void)fcntl(0, F_SETFL, ioctlval);
#endif
#ifdef POSIX_TERMIOS
	(void)tcgetattr(0, &tc);
#else
	(void)ioctl(0, TIOCGETP, (char *)&sgttyb);
#endif

	/*
	 * If talking to an rlogin process, propagate the terminal type and
	 * baud rate across the network.
	 */
#ifdef KERBEROS
	if (eflag)
	    	getstr(term, sizeof(term), "Terminal type");
#endif
#ifdef POSIX_TERMIOS
	doremoteterm(&tc);
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;
#ifndef NO_INIT_CC
	tc.c_cc[VERASE] = CERASE;
	tc.c_cc[VKILL] = CKILL;
	tc.c_cc[VEOF] = CEOF;
	tc.c_cc[VINTR] = CINTR;
	tc.c_cc[VQUIT] = CQUIT;
	tc.c_cc[VSTART] = CSTART;
	tc.c_cc[VSTOP] = CSTOP;
#ifndef CNUL
#define CNUL CEOL
#endif
	tc.c_cc[VEOL] = CNUL;
	/* The following are common extensions to POSIX */
#ifdef VEOL2
	tc.c_cc[VEOL2] = CNUL;
#endif
#ifdef VSUSP
#ifdef hpux
#ifndef CSUSP
#define CSUSP CSWTCH
#endif
#endif
	tc.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
	tc.c_cc[VDSUSP] = CDSUSP;
#endif
#ifdef VLNEXT
	tc.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VREPRINT
	tc.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VDISCRD
	tc.c_cc[VDISCRD] = CFLUSH;
#endif
#ifdef VWERSE
	tc.c_cc[VWERSE] = CWERASE;
#endif
#endif /* NO_INIT_CC */
	tcsetattr(0, TCSANOW, &tc);
#else
	doremoteterm(&sgttyb);
	sgttyb.sg_erase = CERASE;
	sgttyb.sg_kill = CKILL;
	(void)ioctl(0, TIOCSLTC, (char *)&ltc);
	(void)ioctl(0, TIOCSETC, (char *)&tc);
	(void)ioctl(0, TIOCSETP, (char *)&sgttyb);
#endif

	for (cnt = getdtablesize(); cnt > 2; cnt--)
		(void) close(cnt);

	ttyn = ttyname(0);
	if (ttyn == NULL || *ttyn == '\0')
		ttyn = "/dev/tty??";

	/* This allows for tty names of the form /dev/pts/4 as well */
	if ((tty = strchr(ttyn, '/')) && (tty = strchr(tty+1, '/')))
		++tty;
	else
		tty = ttyn;

	/* For kerberos tickets, extract only the last part of the ttyname */
	if (ktty = strchr(tty, '/'))
		++ktty;
	else
		ktty = tty;

#ifndef LOG_ODELAY /* 4.2 syslog ... */                      
	openlog("login", 0);
#else
	openlog("login", LOG_ODELAY, LOG_AUTH);
#endif /* 4.2 syslog */

	for (cnt = 0;; username = NULL) {
#ifdef KERBEROS
		char pp[9], pp2[MAXPWSIZE], *namep;
#if 0
		int krbval;
		char realm[REALM_SZ];
#endif
		int kpass_ok,lpass_ok;
#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else
#define read_long_pw_string des_read_pw_string
#endif
		int read_long_pw_string();
#endif
#if !defined(_AIX)
		ioctlval = 0;
		(void)ioctl(0, TIOCSETD, (char *)&ioctlval);
#endif

		if (username == NULL) {
			fflag = 0;
			getloginname();
		}

		if (pwd = getpwnam(username))
			salt = pwd->pw_passwd;
		else
			salt = "xx";
		
		/* if user not super-user, check for disabled logins */
		if (pwd == NULL || pwd->pw_uid)
			checknologin();

		/*
		 * Disallow automatic login to root; if not invoked by
		 * root, disallow if the uid's differ.
		 */
		if (fflag && pwd) {
			int uid = (int) getuid();

			passwd_req = pwd->pw_uid == 0 ||
			    (uid && uid != pwd->pw_uid);
		}

		/*
		 * If no remote login authentication and a password exists
		 * for this user, prompt for one and verify it.
		 */
		if (!passwd_req || pwd && !*pwd->pw_passwd)
			break;

#ifdef KERBEROS
		kpass_ok = 0;
		lpass_ok = 0;

#ifndef NO_SETPRIORITY
		(void) setpriority(PRIO_PROCESS, 0, -4 + PRIO_OFFSET);
#endif
		if (read_long_pw_string(pp2, sizeof(pp2)-1, "Password: ", 0)) {
		    /* reading password failed... */
#ifndef NO_SETPRIORITY
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif
		    goto bad_login;
		}
		if (!pwd)		/* avoid doing useless work */
		    goto bad_login;

#ifdef __SCO__
		cnt = 7;
		goto sco_lose;
#else
		/* Modifications for Kerberos authentication -- asp */
		(void) strncpy(pp, pp2, sizeof(pp));
		pp[8]='\0';
		namep = crypt(pp, pwd->pw_passwd);
		memset (pp, 0, sizeof(pp));	/* To the best of my recollection, Senator... */
		lpass_ok = !strcmp (namep, pwd->pw_passwd);
		
		/* This code used to get Kerberos tickets using the
		   login password.  However, this is no longer useful,
		   because when invoked by rlogin this code won't be
		   asking for a password anyhow.  Also, using the same
		   password for both Kerberos and Unix is risky when
		   non-authenticated logins are permitted to some
		   systems.  */
#if 0
		if (pwd->pw_uid != 0) { /* Don't get tickets for root */

		    if (krb_get_lrealm(realm, 1) != KSUCCESS) {
			(void) strncpy(realm, KRB_REALM, sizeof(realm));
		    }
#ifdef BIND_HACK
		    /* Set name server timeout to be reasonable,
		       so that people don't take 5 minutes to
		       log in.  Can you say abstraction violation? */
		    _res.retrans = 1;
#endif
		    /* Set up the ticket file environment variable */
		    strncpy(tkfile, KRB_TK_DIR, sizeof(tkfile));
		    strncat(tkfile, ktty,
			    sizeof(tkfile) - strlen(tkfile) - 1);
		    (void) setenv(KRB_ENVIRON, tkfile, 1);
		    krb_set_tkt_string(tkfile);
#ifdef KRB5
		    /* Set up the ticket file environment variable */
		    strncpy(tk5file, KRB5_TK_DIR, sizeof(tkfile));
		    strncat(tk5file, tty,
			    sizeof(tk5file) - strlen(tk5file) - 1);
		    while (p = strchr((char *)tk5file+strlen(KRB5_TK_DIR), '/'))
			*p = '_';
		    (void) setenv(KRB5_ENVIRON, tk5file, 1);
#endif /* KRB5 */
		    if (setreuid(pwd->pw_uid, -1) < 0) {
			/* can't set ruid to user! */
			krbval = -1;
			fprintf(stderr,
				"login: Can't set ruid for ticket file.\n");
		    } else
		    krbval = krb_get_pw_in_tkt(username, "",
					       realm, "krbtgt",
					       realm,
					       DEFAULT_TKT_LIFE, pp2);
#ifdef KRB5
		    {
			krb5_error_code krb5_ret;
			char *etext;
			
			krb5_ret = do_v5_kinit(username, "", realm,
					       TKT_LIFE,
					       pp2, 0, &etext);
			if (krb5_ret &&
			    krb5_ret != KRB5KRB_AP_ERR_BAD_INTEGRITY) {
			    com_err("login", krb5_ret, etext);
			}
		    }
#endif
		    memset (pp2, 0, sizeof(pp2));
#ifndef NO_SETPRIORITY
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif
		    switch (krbval) {
		    case INTK_OK:
			kpass_ok = 1;
			krbflag = 1;
			break;	

		    /* These errors should be silent */
		    /* So the Kerberos database can't be probed */
		    case KDC_NULL_KEY:
		    case KDC_PR_UNKNOWN:
		    case INTK_BADPW:
		    case KDC_PR_N_UNIQUE:
		    case -1:
			break;
		    /* These should be printed but are not fatal */
		    case INTK_W_NOTALL:
			krbflag = 1;
			kpass_ok = 1;
			fprintf(stderr, "Kerberos error: %s\n",
				krb_get_err_text(krbval));
			break;
		    default:
			fprintf(stderr, "Kerberos error: %s\n",
				krb_get_err_text(krbval));
			break;
		    }
		} else {
		    (void) memset (pp2, 0, sizeof(pp2));
#ifndef NO_SETPRIORITY
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif
		}
#endif /* 0 */

		(void) memset (pp2, 0, sizeof(pp2));
#ifndef NO_SETPRIORITY
		(void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif

#endif

		/* Policy: If local password is good, user is good.
		   We really can't trust the Kerberos password,
		   because somebody on the net could spoof the
		   Kerberos server (not easy, but possible).
		   Some sites might want to use it anyways, in
		   which case they should change this line
		   to:
		   if (kpass_ok)
		   */
		if (lpass_ok)
		    break;
bad_login:
		if (krbflag) {
		    dest_tkt();		/* clean up tickets if login fails */
#ifdef KRB5
		    do_v5_kdestroy(0);
#endif
		}
#else
#ifndef NO_SETPRIORITY
		(void) setpriority(PRIO_PROCESS, 0, -4 + PRIO_OFFSET);
#endif
		p = crypt(getpass("password:"), salt);
#ifndef NO_SETPRIORITY
		(void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#endif
		if (pwd && !strcmp(p, pwd->pw_passwd))
			break;
#endif /* KERBEROS */

sco_lose:
		printf("Login incorrect\n");
		if (++cnt >= 5) {
			if (hostname)
#ifdef UT_HOSTSIZE
			    syslog(LOG_ERR,
				"REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
				tty, UT_HOSTSIZE, hostname, UT_NAMESIZE,
				username);
#else
			    syslog(LOG_ERR,
				"REPEATED LOGIN FAILURES ON %s FROM %s, %.*s",
				tty, hostname, UT_NAMESIZE,
				username);
#endif
			else
			    syslog(LOG_ERR,
				"REPEATED LOGIN FAILURES ON %s, %.*s",
				tty, UT_NAMESIZE, username);
/* irix has no tichpcl */
#ifdef TIOCHPCL
			(void)ioctl(0, TIOCHPCL, (char *)0);
#endif
			sleepexit(1);
		}
	}

	/* committed to login -- turn off timeout */
	(void)alarm((u_int)0);

	/*
	 * If valid so far and root is logging in, see if root logins on
	 * this terminal are permitted.
	 *
	 * We allow authenticated remote root logins (except -r style)
	 */
	if (pwd->pw_uid == 0 && !rootterm(tty) && (passwd_req || rflag)) {
		if (hostname)
#ifdef UT_HOSTSIZE
			syslog(LOG_ERR, "ROOT LOGIN REFUSED ON %s FROM %.*s",
			    tty, UT_HOSTSIZE, hostname);
#else
			syslog(LOG_ERR, "ROOT LOGIN REFUSED ON %s FROM %s",
			    tty, hostname);
#endif
		else
			syslog(LOG_ERR, "ROOT LOGIN REFUSED ON %s", tty);
		printf("Login incorrect\n");
		sleepexit(1);
	}

#ifdef OQUOTA
	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		switch(errno) {
		case EUSERS:
			fprintf(stderr,
		"Too many users logged on already.\nTry again later.\n");
			break;
		case EPROCLIM:
			fprintf(stderr,
			    "You have too many processes running.\n");
			break;
		default:
			perror("quota (Q_SETUID)");
		}
		sleepexit(0);
	}
#endif

	if (chdir(pwd->pw_dir) < 0) {
		printf("No directory %s!\n", pwd->pw_dir);
		if (chdir("/"))
			exit(0);
		pwd->pw_dir = "/";
		printf("Logging in with home = \"/\".\n");
	}

	/* nothing else left to fail -- really log in */
	{
		struct utmp utmp;

		memset((char *)&utmp, 0, sizeof(utmp));
		login_time = time(&utmp.ut_time);
		(void) strncpy(utmp.ut_name, username, sizeof(utmp.ut_name));
#ifndef NOUTHOST
		if (hostname)
		    (void) strncpy(utmp.ut_host, hostname,
				   sizeof(utmp.ut_host));
		else
		    memset(utmp.ut_host, 0, sizeof(utmp.ut_host));
#endif
		/* Solaris 2.0, 2.1 used ttyn here. Never Again... */
		(void) strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
		login(&utmp);
	}

	quietlog = access(HUSHLOGIN, F_OK) == 0;
	dolastlog(quietlog, tty);

	if (!hflag && !rflag && !kflag && !Kflag && !eflag) {	/* XXX */
		static struct winsize win = { 0, 0, 0, 0 };

		(void)ioctl(0, TIOCSWINSZ, (char *)&win);
	}

	(void)chown(ttyn, pwd->pw_uid,
	    (gr = getgrnam(TTYGRPNAME)) ? gr->gr_gid : pwd->pw_gid);
#ifdef KERBEROS
	if(krbflag)
	    (void) chown(getenv(KRB_ENVIRON), pwd->pw_uid, pwd->pw_gid);
#ifdef KRB5
	    (void) chown(getenv(KRB5_ENVIRON), pwd->pw_uid, pwd->pw_gid);
#endif
#endif
	(void)chmod(ttyn, 0620);
#ifdef KERBEROS
#ifdef SETPAG
	if (pwd->pw_uid) {
	    /* Only reset the pag for non-root users. */
	    /* This allows root to become anything. */
	    pagflag = 1;
	    setpag();
	}
#endif
	/* This is often not desirable, and the old code didn't work
           anyhow.  */
	/* Fork so that we can call kdestroy */
	dofork();
#endif
	(void)setgid((gid_type) pwd->pw_gid);

	(void) initgroups(username, pwd->pw_gid);

#ifdef OQUOTA
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
#endif
#ifdef __SCO__
	/* this is necessary when C2 mode is enabled, but not otherwise */
	setluid((uid_type) pwd->pw_uid);
#endif
#ifdef KERBEROS
	/* This call MUST succeed */
	if(setuid((uid_type) pwd->pw_uid) < 0) {
	     perror("setuid");
	     sleepexit(1);
	}
#else
	(void)setuid((uid_type) pwd->pw_uid);
#endif /* KERBEROS */

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = BSHELL;
	/* turn on new line discipline for the csh */
	else if (!strcmp(pwd->pw_shell, "/bin/csh")) {
#ifndef __svr4__
#ifndef __SCO__
/* SCO has TIOCSETD but no *TTYDISC... */
#if !defined(_AIX)
#ifndef linux
/* linux also has TIOCSETD but not *TTYDISC */
#ifdef NTTYDISC
		ioctlval = NTTYDISC;
#else
		ioctlval = TTYDISC;
#endif
		(void)ioctl(0, TIOCSETD, (char *)&ioctlval);
#endif
#endif
#endif
#endif
	}

	/* destroy environment unless user has requested preservation */
	tz = getenv("TZ");
	envinit = (char **)malloc(MAXENVIRON * sizeof(char *));
	if (envinit == 0) {
		fprintf(stderr, "Can't malloc empty environment.\n");
		sleepexit(1);
	}
	if (!pflag)
		environ = envinit;

	i = 0;

#if defined(_AIX) && defined(_IBMR2)
	{
	    FILE *fp;
	    if ((fp = fopen("/etc/environment", "r")) != NULL) {
		while(fgets(tbuf, sizeof(tbuf), fp)) {
		    if ((tbuf[0] == '#') || (strchr(tbuf, '=') == 0))
			continue;
		    for (p = tbuf; *p; p++)
			if (*p == '\n') {
			    *p = '\0';
			    break;
			}
		    envinit[i++] = strsave(tbuf);
		}
		fclose(fp);
	    }
	}
#endif
	sprintf(tbuf,"LOGIN=%s",pwd->pw_name);
	envinit[i++] = strsave(tbuf);

#if defined(_AIX) && defined(i386)
	envinit[i++] = "hosttype=ps2";
#endif
#if defined(_AIX) && defined(_IBMR2)
	envinit[i++] = "hosttype=rsaix";
#endif
#if defined(mips) && defined(ultrix)
	envinit[i++] = "hosttype=decmips";
#endif
#if defined(_AUX_SOURCE)
	envinit[i++] = "hosttype=68kaux";
#endif

	envinit[i++] = NULL;

	setenv("HOME", pwd->pw_dir, 0);
	setenv("PATH", LPATH, 0);
	setenv("USER", pwd->pw_name, 0);
	setenv("LOGNAME", pwd->pw_name, 0);
	setenv("SHELL", pwd->pw_shell, 0);
	if (tz != NULL)
		setenv("TZ", tz, 0);

	if (term[0] != '\0' || !pflag || getenv("TERM") == NULL) {
		if (term[0] == '\0')
			(void) strncpy(term, stypeof(tty), sizeof(term));
		(void)setenv("TERM", term, 0);
	}
#ifdef KERBEROS
	/* tkfile[0] is only set if we got tickets above */
	if (tkfile[0])
	    (void) setenv(KRB_ENVIRON, tkfile, 1);
#endif /* KERBEROS */

#if 0
	strcpy(wgfile, "/tmp/wg.XXXXXX");
	mktemp(wgfile);
	setenv("WGFILE", wgfile, 0);
#endif

	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
	if (pwd->pw_uid == 0)
		if (hostname)
#ifdef KERBEROS
			if (kdata) {
			    /* @*$&@#*($)#@$ syslog doesn't handle very
			       many arguments */
			    char buf[BUFSIZ];
#ifdef UT_HOSTSIZE
			    (void) sprintf(buf,
				   "ROOT LOGIN (krb) %s from %.*s, %s.%s@%s",
				   tty, UT_HOSTSIZE, hostname,
				   kdata->pname, kdata->pinst,
				   kdata->prealm);
#else
			    (void) sprintf(buf,
				   "ROOT LOGIN (krb) %s from %s, %s.%s@%s",
				   tty, hostname,
				   kdata->pname, kdata->pinst,
				   kdata->prealm);
#endif
			    syslog(LOG_NOTICE, buf);
		        } else {
#endif /* KERBEROS */
#ifdef UT_HOSTSIZE
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
			    tty, UT_HOSTSIZE, hostname);
#else
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %s",
			    tty, hostname);
#endif
#ifdef KERBEROS
			}
  		else 
			if (kdata) {
			    syslog(LOG_NOTICE,
				   "ROOT LOGIN (krb) %s, %s.%s@%s",
				   tty,
				   kdata->pname, kdata->pinst,
				   kdata->prealm);
			} 
#endif /* KERBEROS */
		else
			syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);

	if (!quietlog) {
		struct stat st;

#ifdef KERBEROS
		if (!krbflag)
		    printf("\nWarning: No Kerberos tickets obtained.\n\n");
#endif /* KERBEROS */
#ifndef sgi /* SGI takes care of this stuff in /etc/profile and /etc/cshrc. */
		motd();
		(void)sprintf(tbuf, "%s/%s", MAILDIR, pwd->pw_name);
		if (stat(tbuf, &st) == 0 && st.st_size != 0)
			printf("You have %smail.\n",
			    (st.st_mtime > st.st_atime) ? "new " : "");
#endif
	}

#ifndef OQUOTA
	if (! access( QUOTAWARN, X_OK)) (void) system(QUOTAWARN);
#endif
	(void)signal(SIGALRM, SIG_DFL);
	(void)signal(SIGQUIT, SIG_DFL);
	(void)signal(SIGINT, SIG_DFL);
	(void)signal(SIGTSTP, SIG_IGN);

	tbuf[0] = '-';
	(void) strcpy(tbuf + 1, (p = strrchr(pwd->pw_shell, '/')) ?
	    p + 1 : pwd->pw_shell);
	execlp(pwd->pw_shell, tbuf, 0);
	fprintf(stderr, "login: no shell: ");
	perror(pwd->pw_shell);
	exit(0);
}

getloginname()
{
	register int ch;
	register char *p;
	static char nbuf[UT_NAMESIZE + 1];

	for (;;) {
		printf("login: ");
		for (p = nbuf; (ch = getchar()) != '\n'; ) {
			if (ch == EOF)
				exit(0);
			if (p < nbuf + UT_NAMESIZE)
				*p++ = ch;
		}
		if (p > nbuf)
			if (nbuf[0] == '-')
				fprintf(stderr,
				    "login names may not start with '-'.\n");
			else {
				*p = '\0';
				username = nbuf;
				break;
			}
	}
}

sigtype
timedout()
{
	fprintf(stderr, "Login timed out after %d seconds\n", timeout);
	exit(0);
}

#ifdef NOTTYENT
int root_tty_security = 1;
#endif
rootterm(tty)
	char *tty;
{
#ifdef NOTTYENT
	return(root_tty_security);
#else
	struct ttyent *t;

	return((t = getttynam(tty)) && t->ty_status&TTY_SECURE);
#endif /* NOTTYENT */
}

jmp_buf motdinterrupt;

motd()
{
	register int fd, nchars;
	sigtype (*oldint)(), sigint();
	char tbuf[8192];

	if ((fd = open(MOTDFILE, O_RDONLY, 0)) < 0)
		return;
	oldint = signal(SIGINT, sigint);
	if (setjmp(motdinterrupt) == 0)
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
	(void)signal(SIGINT, oldint);
	(void)close(fd);
}

sigtype
sigint()
{
	longjmp(motdinterrupt, 1);
}

checknologin()
{
	register int fd, nchars;
	char tbuf[8192];

	if ((fd = open(NOLOGIN, O_RDONLY, 0)) >= 0) {
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
		sleepexit(0);
	}
}

dolastlog(quiet, tty)
	int quiet;
	char *tty;
{
#ifndef NO_LASTLOG
	struct lastlog ll;
	int fd;

	if ((fd = open(LASTLOG, O_RDWR, 0)) >= 0) {
		(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		if (!quiet) {
			if (read(fd, (char *)&ll, sizeof(ll)) == sizeof(ll) &&
			    ll.ll_time != 0) {
				printf("Last login: %.*s ",
				    24-5, (char *)ctime(&ll.ll_time));
				if (*ll.ll_host != '\0')
					printf("from %.*s\n",
					    sizeof(ll.ll_host), ll.ll_host);
				else
					printf("on %.*s\n",
					    sizeof(ll.ll_line), ll.ll_line);
			}
			(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		}
		(void)time(&ll.ll_time);
		(void) strncpy(ll.ll_line, tty, sizeof(ll.ll_line));
		if (hostname)
		    (void) strncpy(ll.ll_host, hostname, sizeof(ll.ll_host));
		else
		    (void) memset(ll.ll_host, 0, sizeof(ll.ll_host));
		(void)write(fd, (char *)&ll, sizeof(ll));
		(void)close(fd);
	}
#endif /* NO_LASTLOG */
}

#undef	UNKNOWN
#define	UNKNOWN	"su"

char *
stypeof(ttyid)
	char *ttyid;
{
#ifdef NOTTYENT
	return(UNKNOWN);
#else
	struct ttyent *t;

	return(ttyid && (t = getttynam(ttyid)) ? t->ty_type : UNKNOWN);
#endif
}

doremotelogin(host)
	char *host;
{
	static char lusername[UT_NAMESIZE+1];
	char rusername[UT_NAMESIZE+1];

	getstr(rusername, sizeof(rusername), "Remote user");
	getstr(lusername, sizeof(lusername), "Local user");
	getstr(term, sizeof(term), "Terminal type");
	username = lusername;
	pwd = getpwnam(username);
	if (pwd == NULL)
		return(-1);
	return(ruserok(host, (pwd->pw_uid == 0), rusername, username));
}

#ifdef KERBEROS
do_krb_login(host, strict)
	char *host;
	int strict;
{
	int rc;
	struct sockaddr_in sin;
	char instance[INST_SZ], version[9];
	KTEXT ticket;
	long authoptions = 0L;
        struct hostent *hp = gethostbyname(host);
	static char lusername[UT_NAMESIZE+1];

	/*
	 * Kerberos autologin protocol.
	 */

	/* If the Kerberos authentication fails, and strict is 0, this
	   code will automatically fallback to the Berkeley rlogind
	   protocol.  However, it gets it wrong, because it does not
	   check the .rhost file or hosts.equiv.  Also, when it asks
	   for a password, it is not clear to the user what password
	   is meant or whether it is encrypted (it wants the password
	   from /etc/passwd, and it is not encrypted).  To remove this
	   confusion, we always set strict here.  Note that this means
	   that klogind and Klogind are now identical.  The old
	   behaviour can be restored by removing the `strict = 1;'
	   line.  */
	strict = 1;

	(void) memset((char *) &sin, 0, (int) sizeof(sin));

        if (hp)
                (void) memcpy((char *)&sin.sin_addr, hp->h_addr,
			      sizeof(sin.sin_addr));
	else
		sin.sin_addr.s_addr = inet_addr(host);

	if ((hp == NULL) && (sin.sin_addr.s_addr == -1)) {
		printf("Hostname did not resolve to an address, so Kerberos authentication failed\r\n");
                /*
		 * No host addr prevents auth, so
                 * punt krb and require password
		 */
                if (strict) {
                        goto paranoid;
                } else {
			pwd = NULL;
                        return(-1);
		}
	}

	kdata = (AUTH_DAT *)malloc( sizeof(AUTH_DAT) );
	ticket = (KTEXT) malloc(sizeof(KTEXT_ST));

	(void) strcpy(instance, "*");
	if (rc=krb_recvauth(authoptions, 0, ticket, "rcmd",
			    instance, &sin,
			    (struct sockaddr_in *)0,
			    kdata, srvtab, (bit_64 *) 0, version)) {
		printf("Kerberos rlogin failed: %s\r\n",krb_get_err_text(rc));
		if (strict) {
paranoid:
			/*
			 * Paranoid hosts, such as a Kerberos server,
			 * specify the Klogind daemon to disallow
			 * even password access here.
			 */
			printf("Sorry, you must have Kerberos authentication to access this host.\r\n");
			exit(1);
		}
	}
	(void) getstr(lusername, sizeof (lusername), "Local user");
	(void) getstr(term, sizeof(term), "Terminal type");
	username = lusername;
	if (getuid()) {
		pwd = NULL;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = NULL;
		return(-1);
	}

	/*
	 * if Kerberos login failed because of an error in krb_recvauth,
	 * return the indication of a bad attempt.  User will be prompted
	 * for a password.  We CAN'T check the .rhost file, because we need 
	 * the remote username to do that, and the remote username is in the 
	 * Kerberos ticket.  This affects ONLY the case where there is
	 * Kerberos on both ends, but Kerberos fails on the server end. 
	 */
	if (rc) {
		return(-1);
	}

	if (rc=kuserok(kdata,lusername)) {
		printf("login: %s has not given you permission to login without a password.\r\n",lusername);
		if (strict) {
		  exit(1);
		}
		return(-1);
	}
	return(0);
}
#endif /* KERBEROS */

getstr(buf, cnt, err)
	char *buf, *err;
	int cnt;
{
	int ocnt = cnt;
	char *obuf = buf;
	char ch;

	do {
		if (read(0, &ch, sizeof(ch)) != sizeof(ch))
			exit(1);
		if (--cnt < 0) {
			fprintf(stderr,"%s '%.*s' too long, %d characters maximum.\r\n",
			       err, ocnt, obuf, ocnt-1);
			sleepexit(1);
		}
		*buf++ = ch;
	} while (ch);
}

char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600",
	"1200", "1800", "2400", "4800", "9600", "19200", "38400",
};
#define	NSPEEDS	(sizeof(speeds) / sizeof(speeds[0]))

#ifdef POSIX_TERMIOS
#ifndef CBAUD
/* this must be in sync with the list above */
speed_t b_speeds[] = {
	B0, B50, B75, B110, B134, B150, B200, B300, B600,
	B1200, B1800, B2400, B4800, B9600, B19200, B38400,
};
#endif
#endif

doremoteterm(tp)
#ifdef POSIX_TERMIOS
	struct termios *tp;
#else
	struct sgttyb *tp;
#endif
{
	register char *cp = strchr(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = strchr(speed, '/');
		if (cp)
			*cp++ = '\0';
		/* DON'T set the baud rate to zero, it might hangup */
		if (strcmp(speed, "0")) {
		    for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
#ifdef POSIX_TERMIOS
#ifdef CBAUD
/* some otherwise-posix systems seem not to have cfset... for now, leave
   the old code for those who can use it */
				tp->c_cflag =
					(tp->c_cflag & ~CBAUD) | (cpp-speeds);
#else
				cfsetispeed(tp, b_speeds[cpp-speeds]);
				cfsetospeed(tp, b_speeds[cpp-speeds]);
#endif
#else
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
#endif
				break;
			}
	       }
	}
#ifdef POSIX_TERMIOS
 	/* set all standard echo, edit, and job control options */
	/* but leave any extensions */
 	tp->c_lflag |= ECHO|ECHOE|ECHOK|ICANON|ISIG;
	tp->c_lflag &= ~(NOFLSH|TOSTOP|IEXTEN);
#ifdef ECHOCTL
	/* Not POSIX, but if we have it, we probably want it */
 	tp->c_lflag |= ECHOCTL;
#endif
#ifdef ECHOKE
	/* Not POSIX, but if we have it, we probably want it */
 	tp->c_lflag |= ECHOKE;
#endif
 	tp->c_iflag |= ICRNL|BRKINT;
 	tp->c_oflag |= ONLCR|OPOST|TAB3;
#else /* !POSIX_TERMIOS */
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
#endif
}

sleepexit(eval)
	int eval;
{
#ifdef KERBEROS
	if (krbflag) {
	    (void) dest_tkt();
#ifdef KRB5
	    do_v5_kdestroy(0);
#endif
	}
#endif /* KERBEROS */
	sleep((u_int)5);
	exit(eval);
}

/*
 * This routine handles cleanup stuff, and the like.
 * It exits only in the child process.
 */
#include <sys/wait.h>
#ifdef WAIT_USES_INT
#define WAIT_TYPE int
#else
#define WAIT_TYPE union wait
#endif
void
dofork()
{
    int child;

    if(!(child=fork()))
	    return; /* Child process */

    /* Setup stuff?  This would be things we could do in parallel with login */
    (void) chdir("/");	/* Let's not keep the fs busy... */
    
    /* If we're the parent, watch the child until it dies */
    while(wait((WAIT_TYPE *)0) != child)
	    ;

    /* Cleanup stuff */

    /* Run dest_tkt to destroy tickets */
    (void) dest_tkt();		/* If this fails, we lose quietly */
#ifdef KRB5
    do_v5_kdestroy(0);
#endif
#ifdef SETPAG
    if (pagflag)
	ktc_ForgetAllTokens();
#endif

    /* Leave */
    exit(0);
}


#ifndef HAVE_STRSAVE
static char *strsave(s)
char *s;
{
    char *ret = (char *)malloc(strlen(s) + 1);
    strcpy(ret, s);
    return(ret);
}
#endif



#if defined(_AIX) && defined(_IBMR2)
#include <sys/id.h>

/*
 * AIX 3.1 has bizzarre ideas about changing uids around.  They are
 * such that the seteuid and setruid calls here fail.  For this reason
 * we are replacing the seteuid and setruid calls.
 * 
 * The bizzarre ideas are as follows:
 *
 * The effective ID may be changed only to the current real or
 * saved IDs.
 *
 * The saved uid may be set only if the real and effective
 * uids are being set to the same value.
 *
 * The real uid may be set only if the effective
 * uid is being set to the same value.
 */

#ifdef __STDC__
static int setruid(uid_t ruid)
#else
static int setruid(ruid)
  uid_t ruid;
#endif /* __STDC__ */
{
    uid_t euid;

    if (ruid == -1)
        return (0);

    euid = geteuid();

    if (setuidx(ID_REAL | ID_EFFECTIVE, ruid) == -1)
        return (-1);
    
    return (setuidx(ID_EFFECTIVE, euid));
}


#ifdef __STDC__
static int seteuid(uid_t euid)
#else
static int seteuid(euid)
  uid_t euid;
#endif /* __STDC__ */
{
    uid_t ruid;

    if (euid == -1)
        return (0);

    ruid = getuid();

    if (setuidx(ID_SAVED | ID_REAL | ID_EFFECTIVE, euid) == -1)
        return (-1);
    
    return (setruid(ruid));
}


#ifdef __STDC__
static int setreuid(uid_t ruid, uid_t euid)
#else
static int setreuid(ruid, euid)
  uid_t ruid;
  uid_t euid;
#endif /* __STDC__ */
{
    if (seteuid(euid) == -1)
        return (-1);

    return (setruid(ruid));
}


#ifdef __STDC__
static int setuid(uid_t uid)
#else
static int setuid(uid)
  uid_t uid;
#endif /* __STDC__ */
{
    return (setreuid(uid, uid));
}

#endif /* RIOS */

#ifdef KRB5
/*
 * This routine takes v4 kinit parameters and performs a V5 kinit.
 * 
 * name, instance, realm is the v4 principal information
 *
 * lifetime is the v4 lifetime (i.e., in units of 5 minutes)
 * 
 * password is the password
 *
 * ret_cache_name is an optional output argument in case the caller
 * wants to know the name of the actual V5 credentials cache (to put
 * into the KRB5_ENV_CCNAME environment variable)
 *
 * etext is a mandatory output variable which is filled in with
 * additional explanatory text in case of an error.
 * 
 */
krb5_error_code do_v5_kinit(name, instance, realm, lifetime, password,
			    ret_cache_name, etext)
	char	*name;
	char	*instance;
	char	*realm;
	int	lifetime;
	char	*password;
	char	**ret_cache_name;
	char	**etext;
{
	krb5_context context;
	krb5_error_code retval;
	krb5_principal	me = 0, server = 0;
	krb5_ccache	ccache = NULL;
	krb5_creds my_creds;
	krb5_timestamp now;
	krb5_address **my_addresses = 0;
	char *cache_name;

	*etext = 0;
	if (ret_cache_name)
		*ret_cache_name = 0;
	memset((char *)&my_creds, 0, sizeof(my_creds));

	retval = krb5_init_context(&context);
	if (retval)
		return retval;

	cache_name = krb5_cc_default_name(context);
	krb5_init_ets(context);
	
	retval = krb5_425_conv_principal(context, name, instance, realm, &me);
	if (retval) {
		*etext = "while converting V4 principal";
		goto cleanup;
	}
    
	retval = krb5_cc_resolve (context, cache_name, &ccache);
	if (retval) {
		*etext = "while resolving ccache";
		goto cleanup;
	}

	retval = krb5_cc_initialize (context, ccache, me);
	if (retval) {
		*etext = "while initializing cache";
		goto cleanup;
	}

	retval = krb5_build_principal_ext(context, &server,
					  krb5_princ_realm(context,
							   me)->length,
					  krb5_princ_realm(context, me)->data,
					  KRB5_TGS_NAME_SIZE, KRB5_TGS_NAME,
					  krb5_princ_realm(context,
							   me)->length,
					  krb5_princ_realm(context, me)->data,
					  0);
	if (retval)  {
		*etext = "while building server name";
		goto cleanup;
	}

	retval = krb5_os_localaddr(context, &my_addresses);
	if (retval) {
		*etext = "when getting my address";
		goto cleanup;
	}

	retval = krb5_timeofday(context, &now);
	if (retval) {
		*etext = "while getting time of day";
		goto cleanup;
	}
	
	my_creds.client = me;
	my_creds.server = server;
	my_creds.times.starttime = 0;
	my_creds.times.endtime = now + lifetime*5*60;
	my_creds.times.renew_till = 0;
	
	retval = krb5_get_in_tkt_with_password(context, 0, my_addresses,
					       NULL, NULL, password, ccache,
					       &my_creds, 0);
	if (retval) {
		*etext = "while calling krb5_get_in_tkt_with_password";
		goto cleanup;
	}

	if (ret_cache_name) {
		*ret_cache_name = (char *) malloc(strlen(cache_name)+1);
		if (!*ret_cache_name) {
			retval = ENOMEM;
			goto cleanup;
		}
		strcpy(*ret_cache_name, cache_name);
	}

cleanup:
	if (me)
		krb5_free_principal(context, me);
	if (server)
		krb5_free_principal(context, server);
	if (my_addresses)
		krb5_free_addresses(context, my_addresses);
	if (ccache)
		krb5_cc_close(context, ccache);
	my_creds.client = 0;
	my_creds.server = 0;
	krb5_free_cred_contents(context, &my_creds);
	krb5_free_context(context);
	return retval;
}

krb5_error_code do_v5_kdestroy(cachename)
	char	*cachename;
{
	krb5_context context;
	krb5_error_code retval;
	krb5_ccache cache;

	retval = krb5_init_context(&context);
	if (retval)
		return retval;

	if (!cachename)
		cachename = krb5_cc_default_name(context);

	krb5_init_ets(context);

	retval = krb5_cc_resolve (context, cachename, &cache);
	if (retval) {
		krb5_free_context(context);
		return retval;
	}

	retval = krb5_cc_destroy(context, cache);

	krb5_free_context(context);
	return retval;
}
#endif /* KRB5 */
