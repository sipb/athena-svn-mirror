/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpd.c,v $
 *	$Author: epeisach $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpd.c,v 1.12 1991-06-28 13:35:07 epeisach Exp $
 */

#ifndef lint
static char *rcsid_lpd_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/lpd.c,v 1.12 1991-06-28 13:35:07 epeisach Exp $";
#endif lint

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)lpd.c	5.4 (Berkeley) 5/6/86";
#endif not lint

/*
 * lpd -- line printer daemon.
 *
 * Listen for a connection and perform the requested operation.
 * Operations are:
 *	\1printer\n
 *		check the queue for jobs and print any found.
 *	\2printer\n
 *		receive a job from another machine and queue it.
 *	\3printer [users ...] [jobs ...]\n
 *		return the current state of the queue (short form).
 *	\4printer [users ...] [jobs ...]\n
 *		return the current state of the queue (long form).
 *	\5printer person [users ...] [jobs ...]\n
 *		remove jobs from the queue.
 *      kprinter\nkerberos credentials
 *              Uses kerberos authentication
 *
 * Strategy to maintain protected spooling area:
 *	1. Spooling area is writable only by daemon and spooling group
 *	2. lpr runs setuid root and setgrp spooling group; it uses
 *	   root to access any file it wants (verifying things before
 *	   with an access call) and group id to know how it should
 *	   set up ownership of files in the spooling area.
 *	3. Files in spooling area are owned by root, group spooling
 *	   group, with mode 660.
 *	4. lpd, lpq and lprm run setuid daemon and setgrp spooling group to
 *	   access files and printer.  Users can't get to anything
 *	   w/o help of lpq and lprm programs.
 */

#include "lp.h"

#ifdef ZEPHYR
#include <zephyr/zephyr.h>
#endif

int	lflag;				/* log requests flag */

int	reapchild();
int	mcleanup();

#ifdef KERBEROS
KTEXT_ST kticket;
AUTH_DAT kdata;
int kauth;
int sin_len;
struct sockaddr_in faddr;
char krbname[ANAME_SZ + INST_SZ + REALM_SZ + 3];
char kprincipal[ANAME_SZ];
char kinstance[INST_SZ];
char krealm[REALM_SZ];
char local_realm[REALM_SZ];
char kversion[9];
int kflag;                     /* Is the current job authentc */
int kerror;		       /* They tried sending auth, but it failed */
int kerberos_override = -1;    /* Does command option override KA in printcap? */
int kerberos_cf = 0;           /* Are we using a kerberized cf file? */
int use_kerberos;
#endif KERBEROS

#ifdef LACL
char from_host[MAXHOSTNAMELEN];
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	int f, funix, finet, options=0, defreadfds, fromlen;
	struct sockaddr_un sun, fromunix;
	struct sockaddr_in sin, frominet;
	struct hostent *hp;
	int omask, lfd;

#ifdef ZEPHYR
        ZInitialize();
#endif ZEPHYR

	gethostname(host, sizeof(host));
	if(hp = gethostbyname(host)) strcpy(host, hp -> h_name);

	name = argv[0];

	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-')
			switch (argv[0][1]) {
			case 'd':
				options |= SO_DEBUG;
				break;
			case 'l':
				lflag++;
				break;
#ifdef KERBEROS
			case 'u':
				kerberos_override = 0;
				break;
			case 'k':
				kerberos_override = 1;
				break;
#endif KERBEROS
			}
	}

#ifndef DEBUG
	/*
	 * Set up standard environment by detaching from the parent.
	 */
	if (fork())
		exit(0);
	for (f = 0; f < 5; f++)
		(void) close(f);
	(void) open("/dev/null", O_RDONLY);
	(void) open("/dev/null", O_WRONLY);
	(void) dup(1);
	f = open("/dev/tty", O_RDWR);
	if (f > 0) {
		ioctl(f, TIOCNOTTY, 0);
		(void) close(f);
	}
#endif

#ifdef LOG_LPR
	openlog("lpd", LOG_PID, LOG_LPR);
#else
	openlog("lpd", LOG_PID);
#endif
	if (lflag) syslog(LOG_INFO, "daemon started");
	(void) umask(0);
	lfd = open(MASTERLOCK, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		syslog(LOG_ERR, "%s: %m", MASTERLOCK);
		exit(1);
	}
	if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK)	/* active deamon present */
			exit(0);
		syslog(LOG_ERR, "%s: %m", MASTERLOCK);
		exit(1);
	}
	ftruncate(lfd, 0);
	/*
	 * write process id for others to know
	 */
	sprintf(line, "%u\n", getpid());
	f = strlen(line);
	if (write(lfd, line, f) != f) {
		syslog(LOG_ERR, "%s: %m", MASTERLOCK);
		exit(1);
	}
	signal(SIGCHLD, reapchild);
	/*
	 * Restart all the printers.
	 */
	startup();
	(void) UNLINK(SOCKETNAME);
	funix = socket(AF_UNIX, SOCK_STREAM, 0);
	if (funix < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
#define	mask(s)	(1 << ((s) - 1))
	omask = sigblock(mask(SIGHUP)|mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	signal(SIGHUP, mcleanup);
	signal(SIGINT, mcleanup);
	signal(SIGQUIT, mcleanup);
	signal(SIGTERM, mcleanup);
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, SOCKETNAME);
	if (bind(funix, &sun, strlen(sun.sun_path) + 2) < 0) {
		syslog(LOG_ERR, "ubind: %m");
		exit(1);
	}
	sigsetmask(omask);
	defreadfds = 1 << funix;
	listen(funix, 5);
	finet = socket(AF_INET, SOCK_STREAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		if (options & SO_DEBUG)
			if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, 0, 0) < 0) {
				syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
				mcleanup();
			}
		sp = getservbyname("printer", "tcp");
		if (sp == NULL) {
			syslog(LOG_ERR, "printer/tcp: unknown service");
			mcleanup();
		}
		sin.sin_family = AF_INET;
		sin.sin_port = sp->s_port;
		if (bind(finet, &sin, sizeof(sin), 0) < 0) {
			syslog(LOG_ERR, "bind: %m");
			mcleanup();
		}
		defreadfds |= 1 << finet;
		listen(finet, 5);
	}
	/*
	 * Main loop: accept, do a request, continue.
	 */
	for (;;) {
		int domain, nfds, s, readfds = defreadfds;

		nfds = select(20, &readfds, 0, 0, 0);
		if (nfds <= 0) {
			if (nfds < 0 && errno != EINTR)
				syslog(LOG_WARNING, "select: %m");
			continue;
		}
		if (readfds & (1 << funix)) {
			domain = AF_UNIX, fromlen = sizeof(fromunix);
			s = accept(funix, &fromunix, &fromlen);
		} else if (readfds & (1 << finet)) {
			domain = AF_INET, fromlen = sizeof(frominet);
			s = accept(finet, &frominet, &fromlen);
		}
		if (s < 0) {
			if (errno != EINTR)
				syslog(LOG_WARNING, "accept: %m");
			continue;
		}
		if (fork() == 0) {
			signal(SIGCHLD, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
			(void) close(funix);
			(void) close(finet);
			dup2(s, 1);
			(void) close(s);
			if (domain == AF_INET)
				chkhost(&frominet);
			doit();
			exit(0);
		}
		(void) close(s);
	}
}

reapchild()
{
#if !defined(_IBMR2)
	union wait status;
#else
	int status;
#endif

	while (wait3(&status, WNOHANG, 0) > 0)
		;
}

mcleanup()
{
	if (lflag)
		syslog(LOG_INFO, "exiting");
	UNLINK(SOCKETNAME);
	exit(0);
}

/*
 * Stuff for handling job specifications
 */
char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */
char	*person;		/* name of person doing lprm */

char	fromb[32];	/* buffer for client's machine name */
char	cbuf[BUFSIZ];	/* command line buffer */
char	*cmdnames[] = {
	"null",
	"printjob",
	"recvjob",
	"displayq short",
	"displayq long",
	"rmjob"
};


#ifdef KERBEROS
require_kerberos(printer)
char *printer;
{
	int status;
	short KA;
	int use_kerberos;
	
#ifdef HESIOD
	if ((status = pgetent(line, printer)) <= 0) {
		if (pralias(alibuf, printer))
			printer = alibuf;
		if ((status = hpgetent(line, printer)) < 1)
			fatal("unknown printer");
	}
#else
	if ((status = pgetent(line, printer)) < 0) {
		fatal("can't open printer description file");
	} else if (status == 0)
		fatal("unknown printer");
#endif HESIOD			
	KA = pgetnum("ka");
	if (KA > 0)
		use_kerberos = 1;
	else
		use_kerberos = 0;
	if (kerberos_override > -1)
		use_kerberos = kerberos_override;
	
	return(use_kerberos);
}
#endif KERBEROS


doit()
{
	register char *cp;
	register int n;
	
#ifdef KERBEROS
	kflag = 0;
	kerberos_cf = 0;
	kerror = 0;
#endif KERBEROS
	
	for (;;) {
		cp = cbuf;
		do {
			if (cp >= &cbuf[sizeof(cbuf) - 1])
				fatal("Command line too long");
			if ((n = read(1, cp, 1)) != 1) {
				if (n < 0)
					fatal("Lost connection");
				return;
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = cbuf;
		if (lflag) {
			if (*cp >= '\1' && *cp <= '\5')
				syslog(LOG_INFO, "%s requests %s %s",
					from, cmdnames[*cp], cp+1);
#ifdef KERBEROS
			else if (*cp == 'k')
				syslog(LOG_INFO, "%s sent kerberos credentials",
				       from);
#endif KERBEROS
			else
				syslog(LOG_INFO, "bad request (%d) from %s",
					*cp, from);
		}
		switch (*cp++) {
		case '\1':	/* check the queue and print any jobs there */
			printer = cp;
			printjob();
			break;
		case '\2':	/* receive files to be queued */
			printer = cp;
#ifdef KERBEROS
			if (require_kerberos(printer)) {
			    if (kflag)
				kerberos_cf = 1;
			    else {
				/* Return an error and abort */
				syslog(LOG_DEBUG,"%s: Cannot receive job before authentication",printer);
				putchar('\2');
				exit(1);
			    }
			}
#endif KERBEROS
			recvjob();
			break;
		case '\3':	/* display the queue (short form) */
		case '\4':	/* display the queue (long form) */
			printer = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal("Too many requests");
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal("Too many users");
					user[users++] = cp;
				}
			}
			displayq(cbuf[0] - '\3');
			exit(0);
		case '\5':	/* remove a job from the queue */
			printer = cp;
			while (*cp && *cp != ' ')
				cp++;
			if (!*cp)
				break;
			*cp++ = '\0';
			person = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal("Too many requests");
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal("Too many users");
					user[users++] = cp;
				}
			}
#ifdef KERBEROS
			if (require_kerberos(printer)) {
			    if (kflag) {
				kerberos_cf = 1;
				make_kname(kprincipal, kinstance,
					   krealm, krbname);
				person = krbname;
			    }
			    else
				/* This message gets sent to the user */
				    {
					printf("Kerberos authentication required to remove job.\n");
					exit(1);
				    }
			}
#endif KERBEROS
			rmjob();
			break;
#ifdef KERBEROS
		case 'k':	/* Parse kerberos credentials */
			printer = cp;
			kprincipal[0] = krealm[0] = '\0';
			bzero(&kticket, sizeof(KTEXT_ST));
			bzero(&kdata,   sizeof(AUTH_DAT));
			sin_len = sizeof (struct sockaddr_in);
			if (getpeername(1, &faddr, &sin_len) < 0) {
				/* return error and exit */
				fatal("Could not get peername");
			}
			/* Tell remote side that kerberos is accepted here! */
			putchar('\0');
			fflush(stdout);
			strcpy(kinstance, "*");
			kauth = krb_recvauth(0L, 1, &kticket, KLPR_SERVICE,
					     kinstance, 
					     &faddr,
					     (struct sockaddr_in *)NULL,
					     &kdata, "", NULL,
					     kversion);
			if (kauth != KSUCCESS) {
				/* return error and exit */
			        /* We cannot call fatal - not really
				   in protocol yet. We will set error
				   for return later. */
			    putchar('\3');
			    syslog(LOG_DEBUG,"%s: Sending back auth failed", printer);
			    exit(1);
			    break;
			}
			strncpy(kprincipal, kdata.pname,  ANAME_SZ);
			strncpy(kinstance,  kdata.pinst,  INST_SZ);
			krb_get_lrealm(local_realm, 1);
			if (strncmp(kdata.prealm, local_realm, REALM_SZ))
			    strncpy(krealm, kdata.prealm, REALM_SZ);
#ifdef DEBUG
			if (krealm[0] == '\0')
				syslog(LOG_DEBUG,"Authentication for %s.%s",
				       kprincipal, kinstance);
			else
				syslog(LOG_DEBUG,"Authentication for %s.%s@%s",
				       kprincipal, kinstance, krealm);
#endif DEBUG
		        /* Ackknowledge accepted */
			kflag = 1;
			putchar('\0');
			fflush(stdout);
			break;
#endif KERBEROS
		default:
			fatal("Illegal service request");
			break;
		}
	}
}

/*
 * Make a pass through the printcap database and start printing any
 * files left from the last time the machine went down.
 */
startup()
{
	char buf[BUFSIZ];
	register char *cp;
	int pid;

	printer = buf;

	/*
	 * Restart the daemons.
	 */
	while (getprent(buf) > 0) {
		for (cp = buf; *cp; cp++)
			if (*cp == '|' || *cp == ':') {
				*cp = '\0';
				break;
			}
		if ((pid = fork()) < 0) {
			syslog(LOG_WARNING, "startup: cannot fork");
			mcleanup();
		}
		if (!pid) {
			endprent();
			printjob();
		}
	}
}

#define DUMMY ":nobody::"

/*
 * Check to see if the from host has access to the line printer.
 */
chkhost(f)
	struct sockaddr_in *f;
{
  /* The following definitions define what consititutes an "athena machine":
   */
#ifdef   ws
#ifdef NET
#undef NET
#endif
#define NET(x)  (((x) >> 24) & 0xff)
#define SUBNET(x) (((x) >> 16) & 0xff)
#define HHOST(x) (((x) >> 8) & 0xff)
#define LHOST ((x) & 0xff)
#define ATHENA_NETWORK 18
#endif

	register struct hostent *hp;
	register FILE *hostf;
	register char *cp, *sp;
	unsigned long hold_net;
	char ahost[50];
	int first = 1;
	extern char *inet_ntoa();
	int baselen = -1;

	f->sin_port = ntohs(f->sin_port);
	if (f->sin_family != AF_INET || f->sin_port >= IPPORT_RESERVED)
		fatal("Malformed from address");
	hp = gethostbyaddr(&f->sin_addr, sizeof(struct in_addr), f->sin_family);
	if (hp == 0)
		fatal("Host name for your address (%s) unknown",
			inet_ntoa(f->sin_addr));

	strcpy(fromb, hp->h_name);
	from = fromb;
#ifdef LACL
	strcpy(from_host, fromb);
#endif
	if (!strcasecmp(from, host))
		return;
#ifdef	ws
	/* Code for workstation printing only which permits any machine on the 
	   Athena network, and in the namespace, to print, even if not
	   in /etc/hosts.equiv or /etc/hosts.lpd */

	hold_net = ntohl(f->sin_addr.s_addr);
	if (NET(hold_net) == ATHENA_NETWORK)
	    return;
#endif
	sp = fromb;
	cp = ahost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - fromb;
			*cp++ = *sp++;
		} else {
			*cp++ = isupper(*sp) ? tolower(*sp++) : *sp++;
		}
	}
	*cp = '\0';

	hostf = fopen("/etc/hosts.equiv", "r");
again:
	if (hostf) {
		if (!_validuser(hostf, ahost, DUMMY, DUMMY, baselen)) {
			(void) fclose(hostf);
			return;
		}
		(void) fclose(hostf);
	}
	if (first == 1) {
		first = 0;
		hostf = fopen("/etc/hosts.lpd", "r");
		goto again;
	}
	printer = (char *) NULL;
	fatal("Your host does not have line printer access");
}

/*
 * A version of startdaemon for routines within lpd.
 * We're already here.... why open a connection to ourselves?
 */

startdaemon(pr)
	char	*pr;
{
	int	pid;
	
	if ((pid = fork()) == 0) {
		printer = malloc(strlen(pr) + 1);
		strcpy(printer, pr);
		if (lflag)
			syslog(LOG_INFO, "startdaemon(%s) succeeded", printer);
		printjob();
	} else if (pid < 0) {
		perr("fork");
		return(0);
	} else
		return(1);
}

static
perr(msg)
	char *msg;
{
	extern char *name;
	extern int sys_nerr;
	extern char *sys_errlist[];
	extern int errno;

	if (lflag)
		syslog(LOG_INFO, "%s: %s: %m", name, msg);
	printf("%s: %s: ", name, msg);
	fputs(errno < sys_nerr ? sys_errlist[errno] : "Unknown error" , stdout);
	putchar('\n');
}

#if defined(_AUX_SOURCE)
_validuser(hostf, rhost, luser, ruser, baselen)
char *rhost, *luser, *ruser;
FILE *hostf;
int baselen;
{
	char *user;
	char ahost[MAXHOSTNAMELEN];
	register char *p;

	while (fgets(ahost, sizeof (ahost), hostf)) {
		p = ahost;
		while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0') {
			*p = isupper(*p) ? tolower(*p) : *p;
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			*p++ = '\0';
			while (*p == ' ' || *p == '\t')
				p++;
			user = p;
			while (*p != '\n' && *p != ' ' && *p != '\t' && *p != '\0')
				p++;
		} else
			user = p;
		*p = '\0';
		if (_checkhost(rhost, ahost, baselen) &&
		    !strcmp(ruser, *user ? user : luser)) {
			return (0);
		}
	}
	return (-1);
}

_checkhost(rhost, lhost, len)
char *rhost, *lhost;
int len;
{
	static char ldomain[MAXHOSTNAMELEN + 1];
	static char *domainp = NULL;
	static int nodomain = 0;
	register char *cp;

	if (len == -1)
		return(!strcmp(rhost, lhost));
	if (strncmp(rhost, lhost, len))
		return(0);
	if (!strcmp(rhost, lhost))
		return(1);
	if (*(lhost + len) != '\0')
		return(0);
	if (nodomain)
		return(0);
	if (!domainp) {
		if (gethostname(ldomain, sizeof(ldomain)) == -1) {
			nodomain = 1;
			return(0);
		}
		ldomain[MAXHOSTNAMELEN] = NULL;
		if ((domainp = index(ldomain, '.')) == (char *)NULL) {
			nodomain = 1;
			return(0);
		}
		for (cp = ++domainp; *cp; ++cp)
			if (islower(*cp))
				*cp = toupper(*cp);
	}
	return(!strcmp(domainp, rhost + len +1));

}
#endif
