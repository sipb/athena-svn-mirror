/*
 *	rcp.c
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
static char sccsid[] = "@(#)rcp.c	5.10 (Berkeley) 9/20/88";
#endif /* not lint */

/*
 * rcp
 */
#include "conf.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <signal.h>
#ifdef POSIX
#include <stdlib.h>
#endif
#include <pwd.h>
#include <ctype.h>
#include <netdb.h>
#include <errno.h>
#ifdef KERBEROS
#include <krb.h>
#include <krbports.h>
#include <kstream.h>

#ifdef _AUX_SOURCE
#define vfork fork
#endif
#ifdef NOVFORK
#define vfork fork
#endif

#ifdef hpux
#define setreuid(r,e) setresuid(r,e,-1)
#endif
#ifdef __svr4__
#define setreuid(r,e) setuid(r)
#endif
#ifndef roundup
#define roundup(x,y) ((((x)+(y)-1)/(y))*(y))
#endif

#if defined(hpux) || defined(__svr4__)
#include <sys/resource.h>
int getdtablesize() {
  struct rlimit rl;
  getrlimit(RLIMIT_NOFILE, &rl);
  return rl.rlim_cur;
}
#endif

int	sock;
CREDENTIALS cred;
MSG_DAT msg_data;
struct sockaddr_in foreign, local;
Key_schedule schedule;

KTEXT_ST ticket;
AUTH_DAT kdata;
static des_cblock crypt_session_key;
char	krb_realm[REALM_SZ];
void	try_normal();
char	**save_argv(), *krb_realmofhost();
#ifndef HAVE_STRSAVE
static char *strsave();
#endif
#ifdef NOENCRYPTION
#define	des_read	read
#define	des_write	write
#else /* !NOENCRYPTION */
void	send_auth(), answer_auth();
int	encryptflag = 0;
#endif /* NOENCRYPTION */
#include "rpaths.h"
#else /* !KERBEROS */
#define	des_read	read
#define	des_write	write
#endif /* KERBEROS */

int	rem;
kstream krem;
char	*colon();
int	errs;
#ifdef POSIX
void	lostconn();
#else
int	lostconn();
#endif
int	errno;
#ifndef __NetBSD__
extern char	*sys_errlist[];
#endif
int	iamremote, targetshouldbedirectory;
int	iamrecursive;
int	pflag;
int	force_net;
struct	passwd *pwd;
int	userid;
int	port;

struct buffer {
	int	cnt;
	char	*buf;
} *allocbuf();

#define	NULLBUF	(struct buffer *) 0

/*VARARGS*/
int	error();

#define	ga()		(void) kstream_write (krem, "", 1)

main(argc, argv)
	int argc;
	char **argv;
{
	char *targ, *host, *src;
	char *suser, *tuser, *thost;
	int i;
	char buf[BUFSIZ], cmd[50 + REALM_SZ];
	char portarg[20], rcpportarg[20];
	struct servent *sp;
#ifdef ATHENA
	static char curhost[256];
#endif /* ATHENA */
#ifdef KERBEROS
	char realmarg[REALM_SZ + 5];
	long authopts;
	char **orig_argv = save_argv(argc, argv);

	sp = getservbyname("kshell", "tcp");
#else
	sp = getservbyname("shell", "tcp");
#endif /* KERBEROS */
	if (sp == NULL) {
#ifdef KERBEROS
#define SHELL_PORT KRB_SHELL_PORT
#else
#define SHELL_PORT UCB_SHELL_PORT
#endif /* KERBEROS */
		port = htons(SHELL_PORT);
	} else {
		port = sp->s_port;
	}

	portarg[0] = '\0';
	rcpportarg[0] = '\0';
	realmarg[0] = '\0';

	pwd = getpwuid(userid = getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}

#ifdef KERBEROS
	krb_realm[0] = '\0';		/* Initially no kerberos realm set */
#endif /* KERBEROS */
	for (argc--, argv++; argc > 0 && **argv == '-'; argc--, argv++) {
		(*argv)++;
		while (**argv) switch (*(*argv)++) {

		    case 'r':
			iamrecursive++;
			break;

		    case 'p':		/* preserve mtimes and atimes */
			pflag++;
			break;

		    case 'P':		/* Set port to use.  */
			port = atoi(*argv);
			sprintf(portarg, " -p%d", port);
			sprintf(rcpportarg, " -P%d", port);
			port = htons(port);
			goto next_arg;

		    case 'N':
			/* Force use of network even on local machine.  */
			force_net++;
			break;

#ifdef KERBEROS
#ifndef NOENCRYPTION
		    case 'x':
			encryptflag++;
			break;
#endif
		    case 'k':		/* Change kerberos realm */
			argc--, argv++;
			if (argc == 0) 
			  usage();
			strncpy(krb_realm,*argv,REALM_SZ);
			sprintf(realmarg, " -k %s", krb_realm);
			goto next_arg;
#endif /* KERBEROS */
		    /* The rest of these are not for users. */
		    case 'd':
			targetshouldbedirectory = 1;
			break;

		    case 'f':		/* "from" */
			iamremote = 1;
#if defined(KERBEROS) && !defined(NOENCRYPTION)
			if (encryptflag) {
				answer_auth();
				krem = kstream_create_rcp_from_fd (rem,
								   &schedule,
								   &crypt_session_key);
			} else
				krem = kstream_create_from_fd (rem, 0, 0);
			kstream_set_buffer_mode (krem, 0);
#endif /* KERBEROS && !NOENCRYPTION */
			(void) response();
			(void) setuid(userid);
			source(--argc, ++argv);
			exit(errs);

		    case 't':		/* "to" */
			iamremote = 1;
#if defined(KERBEROS) && !defined(NOENCRYPTION)
			if (encryptflag) {
				answer_auth();
				krem = kstream_create_rcp_from_fd (rem,
								   &schedule,
								   &crypt_session_key);
			} else
				krem = kstream_create_from_fd (rem, 0, 0);
			kstream_set_buffer_mode (krem, 0);
#endif /* KERBEROS && !NOENCRYPTION */
			(void) setuid(userid);
			sink(--argc, ++argv);
			exit(errs);

		    default:
			usage();
		}
#ifdef KERBEROS
	      next_arg: ;
#endif /* KERBEROS */
	}
	if (argc < 2)
		usage();
	if (argc > 2)
		targetshouldbedirectory = 1;
	rem = -1;
#ifdef KERBEROS
	(void) sprintf(cmd, "rcp%s%s%s%s%s%s", realmarg, rcpportarg,
		       iamrecursive ? " -r" : "", pflag ? " -p" : "", 
#ifdef NOENCRYPTION
		       "",
#else
		       encryptflag ? " -x" : "",
#endif /* NOENCRYPTION */
		       targetshouldbedirectory ? " -d" : "");
#else /* !KERBEROS */
	(void) sprintf(cmd, "rcp%s%s%s",
	    iamrecursive ? " -r" : "", pflag ? " -p" : "", 
	    targetshouldbedirectory ? " -d" : "");
#endif /* KERBEROS */
	(void) signal(SIGPIPE, lostconn);
	targ = colon(argv[argc - 1]);

#ifdef ATHENA
	/* Check if target machine is the current machine. */

	gethostname(curhost, sizeof(curhost));
#endif /* ATHENA */
	if (targ) {				/* ... to remote */
		*targ++ = 0;
#ifdef ATHENA
		if (hosteq(argv[argc - 1], curhost)) {

			/* If so, pretend there wasn't even one given
			 * check for an argument of just "host:", it
			 * should become "."
			 */
			
			if (*targ == 0) {
				targ = ".";
				argv[argc - 1] = targ;
			}
			else
				argv[argc - 1] = targ;
			targ = 0;
		}
	}
	if (targ) {
	    /* Target machine is some remote machine */
#endif /* ATHENA */
		if (*targ == 0)
			targ = ".";
		thost = strchr(argv[argc - 1], '@');
		if (thost) {
			*thost++ = 0;
			tuser = argv[argc - 1];
			if (*tuser == '\0')
				tuser = NULL;
			else if (!okname(tuser))
				exit(1);
		} else {
			thost = argv[argc - 1];
			tuser = NULL;
		}
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
			if (src) {		/* remote to remote */
				*src++ = 0;
				if (*src == 0)
					src = ".";
				host = strchr(argv[i], '@');
				if (host) {
					*host++ = 0;
					suser = argv[i];
					if (*suser == '\0')
						suser = pwd->pw_name;
					else if (!okname(suser))
						continue;
		(void) sprintf(buf, "rsh %s -l %s -n%s%s %s %s '%s%s%s:%s'",
					    host, suser, portarg, realmarg,
					    cmd, src,
					    tuser ? tuser : "",
					    tuser ? "@" : "",
					    thost, targ);
				} else
		(void) sprintf(buf, "rsh %s -n%s%s %s %s '%s%s%s:%s'",
					    argv[i], portarg, realmarg,
					    cmd, src,
					    tuser ? tuser : "",
					    tuser ? "@" : "",
					    thost, targ);
				(void) susystem(buf);
			} else {		/* local to remote */
				if (rem == -1) {
					(void) sprintf(buf, "%s -t %s",
					    cmd, targ);
					host = thost;
#ifdef KERBEROS
					rem = KSUCCESS;
#ifdef ATHENA_COMPAT
					authopts = KOPT_DO_OLDSTYLE;
#else
					authopts = 0L;
#endif /* ATHENA_COMPAT */
					rem = kcmd(&sock, &host,
						   port,
						   pwd->pw_name,
						   tuser ? tuser :
						   pwd->pw_name,
						   buf,
						   0,
						   &ticket,
						   "rcmd",
						   krb_realm,
						   (CREDENTIALS *)0,
						   (bit_64 *)0,
						   (MSG_DAT *)0,
						   (struct sockaddr_in *) 0,
						   (struct sockaddr_in *) 0,
						   authopts);
					if (rem != KSUCCESS) {
					  switch(rem) {
					  case KDC_PR_UNKNOWN:
					    /* assume the foreign principal
					       isn't registered */
					    fprintf(stderr,
		"Host %s isn't registered for Kerberos rcp service\n",
						    host);
					    break;
					  case NO_TKT_FIL:
					    fprintf(stderr, 
		"%s: No tickets file found. You need to run \"kinit\".\n",
						    orig_argv[0]);
					    break;
					  default:
					    fprintf(stderr,
		"Kerberos rcp failed: %s.\n",
		(rem == -1) ? "rcmd protocol failure" : krb_get_err_text(rem));
					  }
					  rem = -1;
					} else {
#ifndef NOENCRYPTION
						if (encryptflag)
							send_auth(host,krb_realm);
#endif
					}
					if (rem<0)
						try_normal(orig_argv);
					rem = sock; /* needed for source() */
#ifndef NOENCRYPTION
					if (encryptflag)
					  krem = kstream_create_rcp_from_fd (rem,
									     &schedule,
									     &crypt_session_key);
					else
#endif
					  krem = kstream_create_from_fd (rem, 0, 0);
					kstream_set_buffer_mode (krem, 0);
#else
					rem = rcmd(&host, port, pwd->pw_name,
					    tuser ? tuser : pwd->pw_name,
					    buf, 0);
					if (rem < 0)
						exit(1);
					krem = kstream_create_from_fd (rem,
								       0, 0);
#endif /* KERBEROS */
					if (response() < 0)
						exit(1);
					if (krem == 0)
					  perror ("couldn't create stream");
					(void) setuid(userid);
				}
				source(1, argv+i);
			}
		}
	} else {				/* ... to local */
		if (targetshouldbedirectory)
			verifydir(argv[argc - 1]);
		for (i = 0; i < argc - 1; i++) {
			src = colon(argv[i]);
#ifdef ATHENA			
			/* Check if source machine is current machine */
			
			if (src) {
				*src++ = 0;
				if (hosteq(argv[i], curhost)) {

				/* If so, pretend src machine never given */

					if (*src == 0) {
						error("rcp: no path given in arg: %s:\n",
						      argv[i]);
						errs++;
						continue;
					}
					argv[i] = src;
					src = 0;
				} else {
					/* not equiv, return colon */
					*(--src) = ':';
				}
			}
#endif /* ATHENA */
			if (src == 0) {		/* local to local */
				(void) sprintf(buf, "/bin/cp%s%s %s %s",
				    iamrecursive ? " -r" : "",
				    pflag ? " -p" : "",
				    argv[i], argv[argc - 1]);
				(void) susystem(buf);
			} else {		/* remote to local */
				*src++ = 0;
				if (*src == 0)
					src = ".";
				host = strchr(argv[i], '@');
				if (host) {
					*host++ = 0;
					suser = argv[i];
					if (*suser == '\0')
						suser = pwd->pw_name;
					else if (!okname(suser))
						continue;
				} else {
					host = argv[i];
					suser = pwd->pw_name;
				}
				(void) sprintf(buf, "%s -f %s", cmd, src);
#ifdef KERBEROS
				rem=KSUCCESS;
#ifdef ATHENA_COMPAT
				authopts = KOPT_DO_OLDSTYLE;
#else
				authopts = 0L;
#endif /* ATHENA_COMPAT */
				rem = kcmd(&sock, &host, port,
					   pwd->pw_name, suser,
					   buf, 0, &ticket,
					   "rcmd", krb_realm,
					   (CREDENTIALS *)0,
					   (bit_64 *)0,
					   (MSG_DAT *)0,
					   (struct sockaddr_in *) 0,
					   (struct sockaddr_in *) 0,
					   authopts);
				if (rem != KSUCCESS) {
				  switch(rem) {
				  case KDC_PR_UNKNOWN:
				    /* assume the foreign principal
				       isn't registered */
				    fprintf(stderr,
		"Host %s isn't registered for Kerberos rcp service\n",
					    host);
				    break;
				  case NO_TKT_FIL:
				    fprintf(stderr, 
		"%s: No tickets file found. You need to run \"kinit\".\n",
					    orig_argv[0]);
				    break;
				  default:
				    fprintf(stderr,
		"Kerberos rcp failed: %s.\n",
		(rem == -1) ? "rcmd protocol failure" : krb_get_err_text(rem));
				  }
				  rem = -1;
				} else {
#ifndef NOENCRYPTION
					if (encryptflag)
						send_auth(host,krb_realm);
#endif
				}
				if (rem<0)
					try_normal(orig_argv);

				rem = sock; /* needed for sink() below */
#ifndef NOENCRYPTION
				if (encryptflag)
				  krem = kstream_create_rcp_from_fd (rem,
								     &schedule,
								     &crypt_session_key);
				else
#endif
				  krem = kstream_create_from_fd (rem, 0, 0);
				kstream_set_buffer_mode (krem, 0);
#else
				rem = rcmd(&host, port, pwd->pw_name, suser,
				    buf, 0);
				if (rem < 0)
					continue;
#endif /* KERBEROS */
#ifndef solaris20
				(void) setreuid(0, userid);
				sink(1, argv+argc-1);
				(void) setreuid(userid, 0);
#else
/* special solaris hack */
				(void) setuid(0);
				if(seteuid(userid)) {
				  perror("rcp seteuid user"); errs++; exit(errs);
				}
				sink(1, argv+argc-1);
				(void) seteuid(0);
#endif
				(void) close(rem);
				rem = -1;
			}
		}
	}
	exit(errs);
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	if (stat(cp, &stb) >= 0) {
		if ((stb.st_mode & S_IFMT) == S_IFDIR)
			return;
		errno = ENOTDIR;
	}
	error("rcp: %s: %s.\n", cp, sys_errlist[errno]);
	exit(1);
}

char *
colon(cp)
	char *cp;
{

	while (*cp) {
		if (*cp == ':')
			return (cp);
		if (*cp == '/')
			return (0);
		cp++;
	}
	return (0);
}

okname(cp0)
	char *cp0;
{
	register char *cp = cp0;
	register int c;

	do {
		c = *cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
		cp++;
	} while (*cp);
	return (1);
bad:
	fprintf(stderr, "rcp: invalid user name %s\n", cp0);
	return (0);
}

susystem(s)
	char *s;
{
	int status, pid, w;
#if defined (POSIX) || defined (sun)
	register void (*istat)(), (*qstat)();
#else
	register int (*istat)(), (*qstat)();
#endif

	if ((pid = vfork()) == 0) {
		(void) setuid(userid);
		execl("/bin/sh", "sh", "-c", s, (char *)0);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != -1)
		;
	if (w == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return (status);
}

source(argc, argv)
	int argc;
	char **argv;
{
	char *last, *name;
	struct stat stb;
	static struct buffer buffer;
	struct buffer *bp;
	int x, readerr, f, amt;
	off_t i;
	char buf[BUFSIZ];

	for (x = 0; x < argc; x++) {
		name = argv[x];
		if ((f = open(name, 0)) < 0) {
			error("rcp: %s: %s\n", name, sys_errlist[errno]);
			continue;
		}
		if (fstat(f, &stb) < 0)
			goto notreg;
		switch (stb.st_mode&S_IFMT) {

		case S_IFREG:
			break;

		case S_IFDIR:
			if (iamrecursive) {
				(void) close(f);
				rsource(name, &stb);
				continue;
			}
			/* fall into ... */
		default:
notreg:
			(void) close(f);
			error("rcp: %s: not a plain file\n", name);
			continue;
		}
		last = strrchr(name, '/');
		if (last == 0)
			last = name;
		else
			last++;
		if (pflag) {
			/*
			 * Make it compatible with possible future
			 * versions expecting microseconds.
			 */
			(void) sprintf(buf, "T%ld 0 %ld 0\n",
			    stb.st_mtime, stb.st_atime);
			kstream_write (krem, buf, strlen (buf));
			if (response() < 0) {
				(void) close(f);
				continue;
			}
		}
		(void) sprintf(buf, "C%04o %ld %s\n",
		    stb.st_mode&07777, stb.st_size, last);
		kstream_write (krem, buf, strlen (buf));
		if (response() < 0) {
			(void) close(f);
			continue;
		}
		if ((bp = allocbuf(&buffer, f, BUFSIZ)) == NULLBUF) {
			(void) close(f);
			continue;
		}
		readerr = 0;
		for (i = 0; i < stb.st_size; i += bp->cnt) {
			amt = bp->cnt;
			if (i + amt > stb.st_size)
				amt = stb.st_size - i;
			if (readerr == 0 && read(f, bp->buf, amt) != amt)
				readerr = errno;
			kstream_write (krem, bp->buf, amt);
		}
		(void) close(f);
		if (readerr == 0)
			ga();
		else
			error("rcp: %s: %s\n", name, sys_errlist[readerr]);
		(void) response();
	}
}

#ifndef USE_DIRENT_H
#include <sys/dir.h>
#else
#include <dirent.h>
#endif

rsource(name, statp)
	char *name;
	struct stat *statp;
{
	DIR *d = opendir(name);
	char *last;
	char buf[BUFSIZ];
	char *bufv[1];
#ifdef HAS_DIRENT
	struct dirent *dp;
#else
	struct direct *dp;
#endif

	if (d == 0) {
		error("rcp: %s: %s\n", name, sys_errlist[errno]);
		return;
	}
	last = strrchr(name, '/');
	if (last == 0)
		last = name;
	else
		last++;
	if (pflag) {
		(void) sprintf(buf, "T%ld 0 %ld 0\n",
		    statp->st_mtime, statp->st_atime);
		kstream_write (krem, buf, strlen (buf));
		if (response() < 0) {
			closedir(d);
			return;
		}
	}
	(void) sprintf(buf, "D%04o %d %s\n", statp->st_mode&07777, 0, last);
	kstream_write (krem, buf, strlen (buf));
	if (response() < 0) {
		closedir(d);
		return;
	}
	while (dp = readdir(d)) {
		if (dp->d_ino == 0)
			continue;
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		if (strlen(name) + 1 + strlen(dp->d_name) >= BUFSIZ - 1) {
			error("%s/%s: Name too long.\n", name, dp->d_name);
			continue;
		}
		(void) sprintf(buf, "%s/%s", name, dp->d_name);
		bufv[0] = buf;
		source(1, bufv);
	}
	closedir(d);
	kstream_write (krem, "E\n", 2);
	(void) response();
}

response()
{
	char resp, c, rbuf[BUFSIZ], *cp = rbuf;

	if (kstream_read (krem, &resp, 1) != 1)
		lostconn();
	switch (resp) {

	case 0:				/* ok */
		return (0);

	default:
		*cp++ = resp;
		/* fall into... */
	case 1:				/* error, followed by err msg */
	case 2:				/* fatal error, "" */
		do {
			if (kstream_read (krem, &c, 1) != 1)
				lostconn();
			*cp++ = c;
		} while (cp < &rbuf[BUFSIZ] && c != '\n');
		if (iamremote == 0)
			(void) write(2, rbuf, cp - rbuf);
		errs++;
		if (resp == 1)
			return (-1);
		exit(1);
	}
	/*NOTREACHED*/
}

#ifdef POSIX
void
#else
int
#endif
lostconn()
{

	if (iamremote == 0)
		fprintf(stderr, "rcp: lost connection\n");
	exit(1);
}

sink(argc, argv)
	int argc;
	char **argv;
{
	off_t i, j;
	char *targ, *whopp, *cp;
	int of, mode, wrerr, exists, first, count, amt;
	off_t size;
	struct buffer *bp;
	static struct buffer buffer;
	struct stat stb;
	int targisdir = 0;
	int mask = umask(0);
	char *myargv[1];
	char cmdbuf[BUFSIZ], nambuf[BUFSIZ];
	int setimes = 0;
	struct timeval tv[2];
#define atime	tv[0]
#define mtime	tv[1]
#define	SCREWUP(str)	{ whopp = str; goto screwup; }

	if (!pflag)
		(void) umask(mask);
	if (argc != 1) {
		error("rcp: ambiguous target\n");
		exit(1);
	}
	targ = *argv;
	if (targetshouldbedirectory)
		verifydir(targ);
	ga();
	if (stat(targ, &stb) == 0 && (stb.st_mode & S_IFMT) == S_IFDIR)
		targisdir = 1;
	for (first = 1; ; first = 0) {
		cp = cmdbuf;
		if (kstream_read (krem, cp, 1) <= 0)
			return;
		if (*cp++ == '\n')
			SCREWUP("unexpected '\\n'");
		do {
			if (kstream_read(krem, cp, 1) != 1)
				SCREWUP("lost connection");
		} while (*cp++ != '\n');
		*cp = 0;
		if (cmdbuf[0] == '\01' || cmdbuf[0] == '\02') {
			if (iamremote == 0)
				(void) write(2, cmdbuf+1, strlen(cmdbuf+1));
			if (cmdbuf[0] == '\02')
				exit(1);
			errs++;
			continue;
		}
		*--cp = 0;
		cp = cmdbuf;
		if (*cp == 'E') {
			ga();
			return;
		}

#define getnum(t) (t) = 0; while (isdigit(*cp)) (t) = (t) * 10 + (*cp++ - '0');
		if (*cp == 'T') {
			setimes++;
			cp++;
			getnum(mtime.tv_sec);
			if (*cp++ != ' ')
				SCREWUP("mtime.sec not delimited");
			getnum(mtime.tv_usec);
			if (*cp++ != ' ')
				SCREWUP("mtime.usec not delimited");
			getnum(atime.tv_sec);
			if (*cp++ != ' ')
				SCREWUP("atime.sec not delimited");
			getnum(atime.tv_usec);
			if (*cp++ != '\0')
				SCREWUP("atime.usec not delimited");
			ga();
			continue;
		}
		if (*cp != 'C' && *cp != 'D') {
			/*
			 * Check for the case "rcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the rcp command on the remote is
			 * executed so the ^Aerror_message convention isn't
			 * followed.
			 */
			if (first) {
				error("%s\n", cp);
				exit(1);
			}
			SCREWUP("expected control record");
		}
		cp++;
		mode = 0;
		for (; cp < cmdbuf+5; cp++) {
			if (*cp < '0' || *cp > '7')
				SCREWUP("bad mode");
			mode = (mode << 3) | (*cp - '0');
		}
		if (*cp++ != ' ')
			SCREWUP("mode not delimited");
		size = 0;
		while (isdigit(*cp))
			size = size * 10 + (*cp++ - '0');
		if (*cp++ != ' ')
			SCREWUP("size not delimited");
		if (targisdir)
			(void) sprintf(nambuf, "%s%s%s", targ,
			    *targ ? "/" : "", cp);
		else
			(void) strcpy(nambuf, targ);
		exists = stat(nambuf, &stb) == 0;
		if (cmdbuf[0] == 'D') {
			if (exists) {
				if ((stb.st_mode&S_IFMT) != S_IFDIR) {
					errno = ENOTDIR;
					goto bad;
				}
				if (pflag)
					(void) chmod(nambuf, mode);
			} else if (mkdir(nambuf, mode) < 0)
				goto bad;
			myargv[0] = nambuf;
			sink(1, myargv);
			if (setimes) {
				setimes = 0;
				if (utimes(nambuf, tv) < 0)
					error("rcp: can't set times on %s: %s\n",
					    nambuf, sys_errlist[errno]);
			}
			continue;
		}
		if ((of = open(nambuf, O_WRONLY|O_CREAT|O_TRUNC, mode)) < 0) {
	bad:
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
			continue;
		}
#ifdef NO_FCHMOD
		if (exists && pflag)
			(void) chmod(nambuf, mode);
#else
		if (exists && pflag)
			(void) fchmod(of, mode);
#endif
		ga();
		if ((bp = allocbuf(&buffer, of, BUFSIZ)) == NULLBUF) {
			(void) close(of);
			continue;
		}
		cp = bp->buf;
		count = 0;
		wrerr = 0;
		for (i = 0; i < size; i += BUFSIZ) {
			amt = BUFSIZ;
			if (i + amt > size)
				amt = size - i;
			count += amt;
			do {
				j = kstream_read(krem, cp, amt);
				if (j <= 0) {
					if (j == 0)
					    error("rcp: dropped connection");
					else
					    error("rcp: %s\n",
						sys_errlist[errno]);
					exit(1);
				}
				amt -= j;
				cp += j;
			} while (amt > 0);
			if (count == bp->cnt) {
				if (wrerr == 0 &&
				    write(of, bp->buf, count) != count)
					wrerr++;
				count = 0;
				cp = bp->buf;
			}
		}
		if (count != 0 && wrerr == 0 &&
		    write(of, bp->buf, count) != count)
			wrerr++;
#ifndef __SCO__
		if (ftruncate(of, size))
			error("rcp: can't truncate %s: %s\n",
			    nambuf, sys_errlist[errno]);
#endif
		(void) close(of);
		(void) response();
		if (setimes) {
			setimes = 0;
			if (utimes(nambuf, tv) < 0)
				error("rcp: can't set times on %s: %s\n",
				    nambuf, sys_errlist[errno]);
		}				   
		if (wrerr)
			error("rcp: %s: %s\n", nambuf, sys_errlist[errno]);
		else
			ga();
	}
screwup:
	error("rcp: protocol screwup: %s\n", whopp);
	exit(1);
}

struct buffer *
allocbuf(bp, fd, blksize)
	struct buffer *bp;
	int fd, blksize;
{
	int size;
#ifndef NOSTBLKSIZE
	struct stat stb;

	if (fstat(fd, &stb) < 0) {
		error("rcp: fstat: %s\n", sys_errlist[errno]);
		return (NULLBUF);
	}
	size = roundup(stb.st_blksize, blksize);
	if (size == 0)
#endif
		size = blksize;
	if (bp->cnt < size) {
		if (bp->buf != 0)
			free(bp->buf);
		bp->buf = (char *)malloc((unsigned) size);
		if (bp->buf == 0) {
			error("rcp: malloc: out of memory\n");
			return (NULLBUF);
		}
	}
	bp->cnt = size;
	return (bp);
}

/*VARARGS1*/
error(fmt, a1, a2, a3, a4, a5)
	char *fmt;
	int a1, a2, a3, a4, a5;
{
	char buf[BUFSIZ], *cp = buf;

	errs++;
	*cp++ = 1;
	(void) sprintf(cp, fmt, a1, a2, a3, a4, a5);
	if (krem)
	  (void) kstream_write(krem, buf, strlen(buf));
	if (iamremote == 0)
		(void) write(2, buf+1, strlen(buf+1));
}

usage()
{
#ifdef KERBEROS
#ifdef NOENCRYPTION
		fprintf(stderr,
"Usage: rcp [-p] [-k realm] f1 f2; or:\n\trcp [-r] [-p] [-k realm] f1 ... fn d2\n");
#else
		fprintf(stderr,
"Usage: rcp [-p] [-x] [-k realm] f1 f2; or:\n\trcp [-r] [-p] [-x] [-k realm] f1 ... fn d2\n");
#endif /* NOENCRYPTION */
#else /* !KERBEROS */
	fputs("usage: rcp [-p] f1 f2; or: rcp [-rp] f1 ... fn d2\n", stderr);
#endif
	exit(1);
}

#ifdef ATHENA
hosteq(h1, h2)
char *h1, *h2;
{
	struct hostent *h_ptr;
	char hname1[256];
	
	/* If -N used to force use of network, always assume the hosts
           are not the same.  */
	if (force_net)
		return 0;

	/* get the official names for the two hosts */
	
	if ((h_ptr = gethostbyname(h1)) == NULL)
	return(0);
	strcpy(hname1, h_ptr->h_name);
	if ((h_ptr = gethostbyname(h2)) == NULL)
	return(0);
	
	/*return if they are equal (strcmp returns 0 for equal - I return 1) */
	
	return(!strcmp(hname1, h_ptr->h_name));
}
#endif /* ATHENA */
#ifdef KERBEROS
void
try_normal(argv)
char **argv;
{
	register int i;

#ifndef NOENCRYPTION
	if (!encryptflag) {
#endif
		fprintf(stderr,"trying normal rcp (%s)\n", UCB_RCP);
		fflush(stderr);
		/* close all but stdin, stdout, stderr */
		for (i = getdtablesize(); i > 2; i--)
			(void) close(i);
		execv(UCB_RCP, argv);
		perror("exec");
#ifndef NOENCRYPTION
	}
#endif
	exit(1);
}

char **
save_argv(argc, argv)
int argc;
char **argv;
{
	register int i;

	char **local_argv = (char **)calloc((unsigned) argc+1,
					    (unsigned) sizeof(char *));
	/* allocate an extra pointer, so that it is initialized to NULL
	   and execv() will work */
	for (i = 0; i < argc; i++)
		local_argv[i] = strsave(argv[i]);
	return(local_argv);
}

#ifndef HAVE_STRSAVE
static char *
strsave(sp)
char *sp;
{
	register char *ret;
	
	if((ret = (char *)malloc((unsigned) strlen(sp)+1)) == NULL) {
		fprintf(stderr, "rcp: no memory for saving args\n");
		exit(1);
	}
	(void) strcpy(ret,sp);
	return(ret);
}
#endif

#ifndef NOENCRYPTION
void
send_auth(host,realm)
char *host;
char *realm;
{
	int sin_len;
	long authopts;

	sin_len = sizeof (struct sockaddr_in);
	if (getpeername(sock, &foreign, &sin_len) < 0) {
		perror("getpeername");
		exit(1);
	}

	sin_len = sizeof (struct sockaddr_in);
	if (getsockname(sock, &local, &sin_len) < 0) {
		perror("getsockname");
		exit(1);
	}

	if ((realm == NULL) || (realm[0] == '\0'))
	     realm = krb_realmofhost(host);
	/* this needs to be sent again, because the
	   rcp process needs the key.  the rshd has
	   grabbed the first one. */
	authopts = KOPT_DO_MUTUAL;
	if ((rem = krb_sendauth(authopts, sock, &ticket,
				"rcmd", host,
				realm, (unsigned long) getpid(),
				&msg_data,
				&cred, schedule,
				&local,
				&foreign,
				"KCMDV0.1")) != KSUCCESS) {
		fprintf(stderr,
			"krb_sendauth mutual fail: %s\n",
			krb_get_err_text(rem));
		exit(1);
	}
	memcpy(&crypt_session_key, &cred.session, sizeof (crypt_session_key));
}

void
answer_auth()
{
	int sin_len, status;
	long authopts = KOPT_DO_MUTUAL;
	char instance[INST_SZ];
	char version[9];
	char *srvtab;

	sin_len = sizeof (struct sockaddr_in);
	if (getpeername(rem, &foreign, &sin_len) < 0) {
		perror("getpeername");
		exit(1);
	}

	sin_len = sizeof (struct sockaddr_in);
	if (getsockname(rem, &local, &sin_len) < 0) {
		perror("getsockname");
		exit(1);
	}
	strcpy(instance, "*");

	/* If rshd was invoked with the -s argument, it will set the
           environment variable KRB_SRVTAB.  We use that to get the
           srvtab file to use.  If we do use the environment variable,
           we reset to our real user ID (which will already have been
           set up by rsh).  Since rcp is setuid root, we would
           otherwise have a security hole.  If we are using the normal
           srvtab (KEYFILE in krb.h, normally set to /etc/krb-srvtab),
           we must keep our effective uid of root, because that file
           can only be read by root.  */
	srvtab = (char *) getenv("KRB_SRVTAB");
	if (srvtab == NULL)
		srvtab = "";
	if (*srvtab != '\0')
		(void) setuid (userid);

	if ((status = krb_recvauth(authopts, rem, &ticket, "rcmd", instance,
				   &foreign,
				   &local,
				   &kdata,
				   srvtab,
				   schedule,
				   version)) != KSUCCESS) {
		fprintf(stderr, "krb_recvauth mutual fail: %s\n",
			krb_get_err_text(status));
		exit(1);
	}
	memcpy(&crypt_session_key, &kdata.session, sizeof (crypt_session_key));
	return;
}
#endif /* !NOENCRYPTION */

#endif /* KERBEROS */
