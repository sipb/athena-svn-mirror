/*
 *	kcmd.c
 */

#define LIBC_SCCS

/*
 * Copyright (c) 1983 Regents of the University of California.
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "derived from @(#)rcmd.c	5.17 (Berkeley) 6/27/88";
#endif /* LIBC_SCCS and not lint */

#include "conf.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/file.h>
#include <signal.h>
#ifndef sigmask
#define sigmask(m)	(1 << ((m)-1))
#endif
#ifdef HIDE_RUSEROK
/* sys/socket.h has a conflicting definition which doesn't really.. */
#define ruserok __notreally_ruserok
#endif /* HIDE_RUSEROK */
#include <sys/socket.h>
#ifdef HIDE_RUSEROK
#undef ruserok
#endif /* HIDE_RUSEROK */
#include <sys/stat.h>

#include <netinet/in.h>

#ifdef HIDE_RUSEROK
/* netdb.h (at least under Solaris 2.4) has a conflicting definition. */
#define ruserok __notreally_ruserok
#endif /* HIDE_RUSEROK */
#include <netdb.h>
#ifdef HIDE_RUSEROK
#undef ruserok
#endif /* HIDE_RUSEROK */
#include <errno.h>
#include <krb.h>
#include <kparse.h>

#ifndef MAXHOSTNAMELEN 
#define MAXHOSTNAMELEN 64
#endif

extern	errno;
char *malloc(), *krb_realmofhost();
extern	char	*inet_ntoa();

#define	START_PORT	5120	 /* arbitrary */

kcmd(sock, ahost, rport, locuser, remuser, cmd, fd2p, ticket, service, realm,
      cred, schedule, msg_data, laddr, faddr, authopts)
int *sock;
char **ahost;
u_short rport;
char *locuser, *remuser, *cmd;
int *fd2p;
KTEXT ticket;
char *service;
char *realm;
CREDENTIALS *cred;
Key_schedule schedule;
MSG_DAT *msg_data;
struct sockaddr_in *laddr, *faddr;
long authopts;
{
	int s, pid;
#ifdef SIGURG
	sigmasktype oldmask;
#endif
	struct sockaddr_in sin, from;
	char c;
#ifdef ATHENA_COMPAT
	int lport = IPPORT_RESERVED - 1;
#else
	int lport = START_PORT;
#endif
	struct hostent *hp;
	int rc;
	char *host_save;
	int status;
#ifdef DO_REVERSE_RESOLVE
	char *rev_addr; int rev_type, rev_len;
#endif

	pid = getpid();
	hp = gethostbyname(*ahost);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", *ahost);
		return (-1);
	}
#ifdef DO_REVERSE_RESOLVE
	if (! hp->h_addr_list ||! hp->h_addr_list[0]) {
		fprintf(stderr, "%s: no addresses for host\n", *ahost);
		return(-1);
	}
	rev_type = hp->h_addrtype;
	rev_len = hp->h_length;
	rev_addr = malloc(rev_len);
	memcpy(rev_addr, hp->h_addr_list[0], rev_len);
	hp = gethostbyaddr(rev_addr, rev_len, rev_type);
	free(rev_addr);
	if (hp == 0) {
		fprintf(stderr, "%s: couldn't find name from address\n", *ahost);
		return (-1);
	}
#endif
	host_save = malloc(strlen(hp->h_name) + 1);
	strcpy(host_save, hp->h_name);
	*ahost = host_save;

	/* If realm is null, look up from table */
	if ((realm == NULL) || (realm[0] == '\0')) {
		realm = krb_realmofhost(host_save);
	}

#ifdef SIGURG
	SIGBLOCK(oldmask, SIGURG);
#endif
	for (;;) {
		s = getport(&lport);
		if (s < 0) {
			if (errno == EAGAIN)
				fprintf(stderr, "socket: All ports in use\n");
			else
				perror("rcmd: socket");
#ifdef SIGURG
			SIGSETMASK(oldmask);
#endif
			return (-1);
		}
#ifndef hpux
#ifndef __svr4__
#ifndef __SCO__
		fcntl(s, F_SETOWN, pid);
#endif
#endif
#else
		/* hpux invention */
		ioctl(s, FIOSSAIOSTAT, &pid); /* trick: pid is non-zero */
		ioctl(s, FIOSSAIOOWN, &pid);
#endif
		sin.sin_family = hp->h_addrtype;
#if defined(ultrix) || defined(sun)
		memcpy((caddr_t)&sin.sin_addr, hp->h_addr, hp->h_length);
#else
		memcpy((caddr_t)&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
#endif						/* defined(ultrix) || defined(sun) */
		sin.sin_port = rport;
		if (connect(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		(void) close(s);
#ifdef ATHENA_COMPAT
		if (errno == EADDRINUSE) {
			lport--;
			continue;
		}
#endif
#if !(defined(ultrix) || defined(sun))
		if (hp->h_addr_list[1] != NULL) {
			int oerrno = errno;

			fprintf(stderr,
				"connect to address %s: ", inet_ntoa(sin.sin_addr));
			errno = oerrno;
			perror(0);
			hp->h_addr_list++;
			memcpy((caddr_t)&sin.sin_addr, hp->h_addr_list[0],
			       hp->h_length);
			fprintf(stderr, "Trying %s...\n",
				inet_ntoa(sin.sin_addr));
			continue;
		}
#endif						/* !(defined(ultrix) || defined(sun)) */
		perror(hp->h_name);
#ifdef SIGURG
		SIGSETMASK(oldmask);
#endif
		return (-1);
	}
#ifndef ATHENA_COMPAT
	lport--;
#endif
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = getport(&lport), s3;
		int len = sizeof (from);

		if (s2 < 0) {
			status = -1;
			goto bad;
		}
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			perror("write: setting up stderr");
			(void) close(s2);
			status = -1;
			goto bad;
		}
		s3 = accept(s2, (struct sockaddr *)&from, &len);
		(void) close(s2);
		if (s3 < 0) {
			perror("accept");
			lport = 0;
			status = -1;
			goto bad;
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
#if 0
		/* This check adds nothing when using Kerberos.  */
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
			fprintf(stderr,
				"socket: protocol failure in circuit setup.\n");
			status = -1;
			goto bad2;
		}
#endif
	}
	/*
	 * Kerberos-authenticated service.  Don't have to send locuser,
	 * since its already in the ticket, and we'll extract it on
	 * the other side.
	 */
	/* (void) write(s, locuser, strlen(locuser)+1); */

	/* set up the needed stuff for mutual auth, but only if necessary */
	if (authopts & KOPT_DO_MUTUAL) {
		int sin_len;
		*faddr = sin;

		sin_len = sizeof (struct sockaddr_in);
		if (getsockname(s, (struct sockaddr *)laddr, &sin_len) < 0) {
			perror("getsockname");
			status = -1;
			goto bad2;
		}
	}
	if ((status = krb_sendauth(authopts, s, ticket, service, *ahost,
				   realm, (unsigned long) getpid(), msg_data,
				   cred, schedule,
				   laddr,
				   faddr,
				   "KCMDV0.1")) != KSUCCESS) {
	  /* this part involves some very intimate knowledge of a 
	     particular sendauth implementation to pry out the old bits.
	     This only catches the case of total failure -- but that's 
	     the one where we get useful data from the remote end. If
	     we even get an authenticator back, then the problem gets 
	     diagnosed locally anyhow. */	     
	  extern KRB_INT32 __krb_sendauth_hidden_tkt_len;
	  char *old_data = (char*)&__krb_sendauth_hidden_tkt_len;
	  if ((status == KFAILURE) && (*old_data == 1)) {
	    write(2, old_data+1, 3);
	    *old_data = (-1);
	  }
	  if ((status == KFAILURE) && (*old_data == (char)-1)) {
	    while (read(s, &c, 1) == 1) {
	      (void) write(2, &c, 1);
	      if (c == '\n')
		break;
	    }
	    status = -1;
	  }
	  goto bad2;
	}
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);

      reread:
	if ((rc=read(s, &c, 1)) != 1) {
		if (rc==-1) {
			perror(*ahost);
		} else {
			fprintf(stderr,"rcmd: bad connection with remote host\n");
		}
		status = -1;
		goto bad2;
	}
	if (c != 0) {
		/* If rlogind was compiled on SunOS4, and it somehow
                   got the shared library version numbers wrong, it
                   may give an ld.so warning about an old version of a
                   shared library.  Just ignore any such warning.
                   Note that the warning is a characteristic of the
                   server; we may not ourselves be running under
                   SunOS4.  */
		if (c == 'l') {
			char *check = "d.so: warning:";
			char *p;
			char cc;

			p = check;
			while (read(s, &c, 1) == 1) {
				if (*p == '\0') {
					if (c == '\n')
						break;
				} else {
					if (c != *p)
						break;
					++p;
				}
			}

			if (*p == '\0')
				goto reread;

			cc = 'l';
			(void) write(2, &cc, 1);
			if (p != check)
				(void) write(2, check, p - check);
		}

		(void) write(2, &c, 1);
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		status = -1;
		goto bad2;
	}
#ifdef SIGURG
	SIGSETMASK(oldmask);
#endif
	*sock = s;
	return (KSUCCESS);
 bad2:
	if (lport)
		(void) close(*fd2p);
 bad:
	(void) close(s);
#ifdef SIGURG
	SIGSETMASK(oldmask);
#endif
	return (status);
}

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
#ifdef ATHENA_COMPAT
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void) close(s);
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			(void) close(s);
			errno = EAGAIN;		/* close */
			return (-1);
		}
	}
#else
	sin.sin_port = 0;
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		(void) close(s);
		return(-1);
	}
	sin_len = sizeof(sin);
	getsockname(s, (struct sockaddr *)&sin, &sin_len);
	*alport = ntohs(sin.sin_port);
	return(s);
#endif
}

ruserok(rhost, superuser, ruser, luser)
	char *rhost;
	int superuser;
	char *ruser, *luser;
{
	FILE *hostf;
	char fhost[MAXHOSTNAMELEN];
	int first = 1;
	register char *sp, *p;
	int baselen = -1;

	sp = rhost;
	p = fhost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - rhost;
			*p++ = *sp++;
		} else {
			*p++ = islower(*sp) ? toupper(*sp++) : *sp++;
		}
	}
	*p = '\0';
	hostf = superuser ? (FILE *)0 : fopen("/etc/hosts.equiv", "r");
again:
	if (hostf) {
		if (!_validuser(hostf, fhost, luser, ruser, baselen)) {
			(void) fclose(hostf);
			return(0);
		}
		(void) fclose(hostf);
	}
	if (first == 1) {
		struct stat sbuf;
		struct passwd *pwd;
		char pbuf[MAXPATHLEN];

		first = 0;
		if ((pwd = getpwnam(luser)) == NULL)
			return(-1);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");
		if ((hostf = fopen(pbuf, "r")) == NULL)
			return(-1);
		(void)fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			fclose(hostf);
			return(-1);
		}
		goto again;
	}
	return (-1);
}

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
			*p = islower(*p) ? toupper(*p) : *p;
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
		if ((domainp = strchr(ldomain, '.')) == (char *)NULL) {
			nodomain = 1;
			return(0);
		}
		for (cp = ++domainp; *cp; ++cp)
			if (islower(*cp))
				*cp = toupper(*cp);
	}
	return(!strcmp(domainp, rhost + len +1));

}
