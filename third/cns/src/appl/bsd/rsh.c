/*
 *	rsh.c
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
static char sccsid[] = "@(#)rsh.c	5.7 (Berkeley) 9/20/88";
#endif /* not lint */

#include "conf.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

#ifdef KERBEROS
#include <krb.h>
#include <krbports.h>
#include <kstream.h>
#define EKSHELL
#endif /* KERBEROS */

#ifndef NO_SYSIO
#ifdef __svr4__
/* get FIONBIO from sys/filio.h, so what if it is a compatibility feature */
#include <sys/filio.h>
#endif
#endif

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
char	*malloc(), *getpass();

int	errno;
int	options;
int	rem, rfd2;
kstream krem, krfd2;
int	nflag;
sigtype	sendsig();		/* Sigtype is from krb.h, osconf.h, c-*.h */
#ifdef KERBEROS
char	krb_realm[REALM_SZ];
CREDENTIALS cred;
Key_schedule schedule;
#include "rpaths.h"
void	try_normal();
#endif

#ifndef sigmask
#define sigmask(x) (1<<(x))
#endif

main(argc, argv0)
	int argc;
	char **argv0;
{
	int pid;
	char *host, *cp, **ap, buf[BUFSIZ], *args, **argv = argv0, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
	int readfrom, ready;
	int one = 1;
	struct servent *sp;
	u_short sps_port;
	u_short alt_port = 0;
	sigmasktype omask;
#ifdef KERBEROS
	int xflag = 0;
	KTEXT_ST ticket;
	int sock;
	long authopts;
	MSG_DAT msg_dat;
	struct sockaddr_in faddr, laddr;
#endif

	host = strrchr(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rsh")) {
		host = NULL;
		asrsh = 1;
	}
another:
	if (argc > 0 && host == NULL && **argv != '-') {
		host = *argv++, --argc;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc > 0)
			user = *argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		nflag++;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
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
		  fprintf(stderr, "rsh(kerberos): -k flag must have a realm after it.\n");
		  exit (1);
		}
		strncpy(krb_realm,*argv,REALM_SZ);
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-x", 2)) {
		argv++, argc--;
		xflag++;
		goto another;
	}

#endif
	/*
	 * Ignore the -L, -w, -e and -8 flags to allow aliases with rlogin
	 * to work
	 *
	 * There must be a better way to do this! -jmb
	 */
	if (argc > 0 && !strncmp(*argv, "-L", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-w", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-8", 2)) {
		argv++, argc--;
		goto another;
	}
#ifdef ATHENA
	/* additional Athena flags to be ignored */
	if (argc > 0 && !strcmp(*argv, "-noflow")) {	/* No local flow control option for rlogin */
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-7")) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-c")) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-a")) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		goto another;
	}
		/*
		** Also ignore -t ttytype
		*/
	if (argc > 0 && !strcmp(*argv, "-t")) {
		argv++; argv++; argc--; argc--;
		goto another;
	}
#endif
	if (host == 0)
		goto usage;
	if (argv[0] == 0) {
		if (asrsh)
			*argv0 = "rlogin";
		execv(KRB_RLOGIN, argv0);
		perror(KRB_RLOGIN);
		exit(1);
	}

	/* If you want to experiment with the unsupported rsh -x code,
           change the 1 in the next line to a 0.  */
#if 1
	/* Unlike the other rlogin flags, we should warn if `-x' is
	   given and rlogin is not run, because the user could be
	   dangerously confused otherwise.  He might think he's got a
	   secure rsh channel, and there's no such thing yet.  */
	if (xflag)
	  {
	    fprintf (stderr, "%s: Encrypted rsh is not yet available.\n",
		     argv0[0]);
	    return 1;
	  }
#endif

	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, "who are you?\n");
		exit(1);
	}
	cc = 0;
	for (ap = argv; *ap; ap++)
		cc += strlen(*ap) + 1;
	cp = args = malloc(cc);
	for (ap = argv; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
#ifdef KERBEROS
	sp = getservbyname(xflag ? "ekshell" : "kshell", "tcp");
#else 
	sp = getservbyname("shell", "tcp");
#endif /* KERBEROS */
	if (sp == NULL) {
#ifdef KERBEROS
		int SHELL_PORT;
#ifdef KRB_CRYPT_SHELL_PORT
		SHELL_PORT = xflag ? KRB_CRYPT_SHELL_PORT : KRB_SHELL_PORT;
#else /* no KRB_CRYPT_SHELL_PORT */
		if (xflag && alt_port == 0)
		  {
		    fprintf (stderr, "rsh: ekshell/tcp: unknown service\n");
		    return 1;
		  }
		SHELL_PORT = KRB_SHELL_PORT;
#endif /* no KRB_CRYPT_SHELL_PORT */
#else
#define SHELL_PORT UCB_SHELL_PORT
#endif /* KERBEROS */
		sps_port = htons(SHELL_PORT);
	} else {
		sps_port = sp->s_port;
	}

	if (alt_port != 0)
		sps_port = htons(alt_port);

#ifdef KERBEROS
	rem=KSUCCESS;
#ifdef ATHENA_COMPAT
	authopts = KOPT_DO_OLDSTYLE;
#else
	authopts = 0L;
#endif
	if (xflag)
	  authopts |= KOPT_DO_MUTUAL;

	rem = kcmd(&sock, &host, sps_port,
		   pwd->pw_name,
		   user ? user : pwd->pw_name,
		   args, &rfd2, &ticket, "rcmd", krb_realm,
		   &cred, schedule, &msg_dat,
		   &laddr, &faddr,
		   authopts);
	if (rem != KSUCCESS) {
	    switch(rem) {
	    case KDC_PR_UNKNOWN: /* assume the foreign principal
					    isn't registered */
	      fprintf(stderr, "%s: Host %s isn't registered for Kerberos rsh service\n",
		      argv0[0], host);
	      break;
	    case NO_TKT_FIL:
	      fprintf(stderr, "%s: No tickets file found. You need to run \"kinit\".\n",
		      argv0[0]);
	      break;
	    default:
	      fprintf(stderr,
		      "%s: Kerberos rcmd failed: %s.\n",
		      argv0[0],
		      (rem == -1) ? "rcmd protocol failure" :
		      krb_get_err_text(rem));
	    }
	    rem = -1;
	}
	if (rem<0) {
		if (xflag) exit (1);
		try_normal(argv0);
	}
	rem = sock;
	if (xflag)
	  {
	    krem = kstream_create_rlogin_from_fd (rem, &schedule, &cred.session);
	    krfd2 = kstream_create_rlogin_from_fd (rfd2, &schedule, &cred.session);
	  }
	else
	  {
	    krem = kstream_create_from_fd (rem, 0, 0);
	    krfd2 = kstream_create_from_fd (rfd2, 0, 0);
	  }
	kstream_set_buffer_mode (krem, 0);
#else /* !KERBEROS */
        rem = rcmd(&host, sps_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
        if (rem < 0)
                exit(1);
	krem = kstream_create_from_fd (rem, 0, 0);
	krfd2 = kstream_create_from_fd (rfd2, 0, 0);
#endif /* KERBEROS */
	if (rfd2 < 0) {
		fprintf(stderr, "rsh: can't establish stderr\n");
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("setsockopt (stdin)");
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror("setsockopt (stderr)");
	}
	(void) setuid(getuid());
	SIGBLOCK3(omask, SIGINT, SIGQUIT, SIGTERM);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, sendsig);
	if (signal(SIGQUIT, SIG_IGN) != SIG_IGN)
		signal(SIGQUIT, sendsig);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, sendsig);
	if (nflag == 0) {
		pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		}
	}
#ifndef KERBEROS
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
#endif
        if (nflag == 0 && pid == 0) {
		char *bp; int rembits, wc;
		(void) close(rfd2);
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
		if (cc <= 0)
			goto done;
		bp = buf;
	rewrite:
		rembits = 1<<rem;
		if (select(16, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			goto rewrite;
		}
		if ((rembits & (1<<rem)) == 0)
			goto rewrite;
		wc = kstream_write(krem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
		(void) shutdown(rem, 1);
		exit(0);
	}
	SIGSETMASK(omask);
	readfrom = (1<<rfd2) | (1<<rem);
	do {
		ready = readfrom;
		if (select(16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror("select");
				exit(1);
			}
			continue;
		}
		if (ready & (1<<rfd2)) {
			errno = 0;
			cc = kstream_read(krfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rfd2);
			} else
				(void) write(2, buf, cc);
		}
		if (ready & (1<<rem)) {
			errno = 0;
			cc = kstream_read (krem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					readfrom &= ~(1<<rem);
			} else
				(void) write(1, buf, cc);
		}
        } while (readfrom);
	if (nflag == 0)
		(void) kill(pid, SIGKILL);
	exit(0);
usage:
	fprintf (stderr, "usage: rsh host [ -l login ] [ -n ]");
#ifdef KERBEROS
	fprintf (stderr, " [ -k realm ]");
#ifdef EKSHELL
	fprintf (stderr, " [ -x ]");
#endif
#endif
	fprintf (stderr, " command\n");
	exit(1);
}

sigtype
sendsig(signo)
	char signo;
{
	(void) kstream_write(krfd2, &signo, 1);
}
#ifdef KERBEROS
void
try_normal(argv)
char **argv;
{
	char *host;

	/*
	 * if we were invoked as 'rsh host mumble', strip off the rsh
	 * from arglist.
	 *
	 * We always want to call the Berkeley rsh as 'host mumble'
	 */

	host = strrchr(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];

	if (!strcmp(host, "rsh"))
		argv++;

	fprintf(stderr,"trying normal rsh (%s)\n",
		UCB_RSH);
	fflush(stderr);
	execv(UCB_RSH, argv);
	perror("exec");
	exit(1);
}
#endif
