/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)common.c	5.2 (Berkeley) 5/6/86";
#endif not lint

/*
 * Routines and data common to all the line printer functions.
 */

#include "lp.h"

#ifdef SERVER
int	DU;		/* daeomon user-id */
#endif SERVER
int	MX;		/* maximum number of blocks to copy */
int	MC;		/* maximum number of copies allowed */
char	*LP;		/* line printer device name */
char	*RM;		/* remote machine name */
char	*RP;		/* remote printer name */
#ifdef SERVER
char	*LO;		/* lock file name */
char	*ST;		/* status file name */
char	*SD;		/* spool directory */
char	*AF;		/* accounting file */
char	*LF;		/* log file for error messages */
char	*OF;		/* name of output filter (created once) */
char	*IF;		/* name of input filter (created per job) */
char	*RF;		/* name of fortran text filter (per job) */
char	*TF;		/* name of troff filter (per job) */
char	*NF;		/* name of ditroff filter (per job) */
char	*DF;		/* name of tex filter (per job) */
char	*GF;		/* name of graph(1G) filter (per job) */
char	*VF;		/* name of vplot filter (per job) */
char	*CF;		/* name of cifplot filter (per job) */
char	*PF;		/* name of vrast filter (per job) */
char	*FF;		/* form feed string */
char	*TR;		/* trailer string to be output when Q empties */
#endif /* SERVER */
short	SC;		/* suppress multiple copies */
#ifdef SERVER
short	SF;		/* suppress FF on each print job */
short	SH;		/* suppress header page */
short	SB;		/* short banner instead of normal header */
short	HL;		/* print header last */
short	RW;		/* open LP for reading and writing */
short	PW;		/* page width */
short	PL;		/* page length */
short	PX;		/* page width in pixels */
short	PY;		/* page length in pixels */
short	BR;		/* baud rate if lp is a tty */
int	FC;		/* flags to clear if lp is a tty */
int	FS;		/* flags to set if lp is a tty */
int	XC;		/* flags to clear for local mode */
int	XS;		/* flags to set for local mode */
short	RS;		/* restricted to those with local accounts */
#ifdef PQUOTA
char    *RQ; 	        /* Name of remote quota server */
int     CP; 	  	/* Cost per page */
char	*QS;		/* Quota service for printer */
#endif /* PQUOTA */
#ifdef LACL
char	*AC;		/* Local ACL file to use */
short	PA;		/* ACL file used as positive ACL */
short	RA;		/* Restricted host access */
#endif /* LACL */
#endif SERVER

char	line[BUFSIZ];
char	pbuf[BUFSIZ/2];	/* buffer for printcap strings */
char	*bp = pbuf;	/* pointer into pbuf for pgetent() */
char	*name;		/* program name */
char	*printer;	/* printer name */
char	host[32]="";	/* host machine name */
char	*from = host;	/* client's machine name */
#ifdef HESIOD
char	alibuf[BUFSIZ/2];	/* buffer for printer alias */
#endif

/*
 * Create a connection to the remote printer server.
 * Most of this code comes from rcmd.c.
 */
getport(rhost)
	char *rhost;
{
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s, timo = 1, lport = IPPORT_RESERVED - 1;
	int err;

	/*
	 * Get the host address and port number to connect to.
	 */
	if (rhost == NULL)
		fatal("no remote host to connect to");
	hp = gethostbyname(rhost);
	if (hp == NULL)
		fatal("unknown host %s", rhost);
	sp = getservbyname("printer", "tcp");
	if (sp == NULL)
		fatal("printer/tcp: unknown service");
	bzero((char *)&sin, sizeof(sin));
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	sin.sin_family = hp->h_addrtype;
	sin.sin_port = sp->s_port;

	/*
	 * Try connecting to the server.
	 */
retry:
	s = rresvport(&lport);
	if (s < 0)
		return(-1);
	if (connect(s, (caddr_t)&sin, sizeof(sin)) < 0) {
		err = errno;
		(void) close(s);
		errno = err;
		if (errno == EADDRINUSE) {
			lport--;
			goto retry;
		}
		if (errno == ECONNREFUSED && timo <= 8) {
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		return(-1);
	}
	return(s);
}

/*
 * Getline reads a line from the control file cfp, removes tabs, converts
 *  new-line to null and leaves it in line.
 * Returns 0 at EOF or the number of characters read.
 */
getline(cfp)
	FILE *cfp;
{
	register int linel = 0;
	register char *lp = line;
	register c;

	while ((c = getc(cfp)) != '\n') {
		if (c == EOF)
			return(0);
		if (c == '\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0);
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = '\0';
	return(linel);
}

#ifdef SERVER
/*
 * Scan the current directory and make a list of daemon files sorted by
 * creation time.
 * Return the number of entries and a pointer to the list.
 */
getq(namelist)
	struct queue *(*namelist[]);
{
#ifdef POSIX
	register struct dirent *d;
#else
	register struct direct *d;
#endif
	register struct queue *q, **queue;
	register int nitems;
	struct stat stbuf;
	int arraysz, compar();
	DIR *dirp;

	if ((dirp = opendir(SD)) == NULL)
		return(-1);
	if (fstat(dirp->dd_fd, &stbuf) < 0)
		goto errdone;

	/*
	 * Estimate the array size by taking the size of the directory file
	 * and dividing it by a multiple of the minimum size entry. 
	 */
	arraysz = (stbuf.st_size / 24);
	queue = (struct queue **)malloc(arraysz * sizeof(struct queue *));
	if (queue == NULL)
		goto errdone;

	nitems = 0;
	while ((d = readdir(dirp)) != NULL) {
		if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
			continue;	/* daemon control files only */
		if (stat(d->d_name, &stbuf) < 0)
			continue;	/* Doesn't exist */
		q = (struct queue *)malloc(sizeof(time_t)+strlen(d->d_name)+1);
		if (q == NULL)
			goto errdone;
		q->q_time = stbuf.st_mtime;
		strcpy(q->q_name, d->d_name);
		/*
		 * Check to make sure the array has space left and
		 * realloc the maximum size.
		 */
		if (++nitems > arraysz) {
			queue = (struct queue **)realloc((char *)queue,
				(stbuf.st_size/12) * sizeof(struct queue *));
			if (queue == NULL)
				goto errdone;
		}
		queue[nitems-1] = q;
	}
	closedir(dirp);
	if (nitems)
		qsort(queue, nitems, sizeof(struct queue *), compar);
	*namelist = queue;
	return(nitems);

errdone:
	closedir(dirp);
	return(-1);
}

/*
 * Compare modification times.
 */
static
compar(p1, p2)
	register struct queue **p1, **p2;
{
	if ((*p1)->q_time < (*p2)->q_time)
		return(-1);
	if ((*p1)->q_time > (*p2)->q_time)
		return(1);
	return(0);
}
#endif SERVER

/*VARARGS1*/
fatal(msg, a1, a2, a3)
	char *msg;
{
	if (from != host)
		printf("%s: ", host);
	printf("%s: ", name);
	if (printer)
		printf("%s: ", printer);
	printf(msg, a1, a2, a3);
	syslog(LOG_ERR, msg, a1, a2, a3);
	putchar('\n');
	exit(1);
}

#ifdef KERBEROS
/* Form a complete string name consisting of principal, 
 * instance and realm 
 */
make_kname(principal, instance, realm, out_name)
char *principal, *instance, *realm, *out_name;
{
	if ((instance[0] == '\0') && (realm[0] == '\0'))
		strcpy(out_name, principal);
	else {
		if (realm[0] == '\0')
			sprintf(out_name, "%s.%s", principal, instance);
		else {
			if (instance[0] == '\0')
				sprintf(out_name, "%s@%s", principal, realm);
			else
				sprintf(out_name, "%s.%s@%s", principal, 
					instance, realm);
		}
	}
}
#endif KERBEROS
