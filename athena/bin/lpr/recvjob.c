/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/recvjob.c,v $
 *	$Author: epeisach $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/recvjob.c,v 1.9 1992-04-19 21:26:04 epeisach Exp $
 */

#ifndef lint
static char *rcsid_recvjob_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/recvjob.c,v 1.9 1992-04-19 21:26:04 epeisach Exp $";
#endif lint

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)recvjob.c	5.4 (Berkeley) 6/6/86";
#endif not lint

/*
 * Receive printer jobs from the network, queue them and
 * start the printer daemon.
 */

#include "lp.h"
#ifdef _AUX_SOURCE
#include <sys/sysmacros.h>
#include <ufs/ufsparam.h>
#endif

#if (!defined(AIX) || !defined(i386)) && (!defined(_IBMR2))
#ifdef VFS
#include <ufs/fs.h>
#else
#include <sys/fs.h>
#endif VFS
#endif

#ifdef PQUOTA
#include "quota.h"
#include <sys/time.h>
#endif

#ifdef _IBMR2
#include <sys/select.h>
#endif

#if BUFSIZ != 1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

char	*sp = "";
#define ack()	(void) write(1, sp, 1);

int 	lflag;			/* should we log a trace? */
char    tfname[40];		/* tmp copy of cf before linking */
char    dfname[40];		/* data files */
char    cfname[40];             /* control fle - fix compiler bug */
int	minfree;		/* keep at least minfree blocks available */
char	*ddev;			/* disk device (for checking free space) */
int	dfd;			/* file system device descriptor */
#ifdef KERBEROS
char    tempfile[40];           /* Same size as used for cfname and tfname */
extern int kflag;
#endif KERBEROS

#ifdef _AUX_SOURCE
/* They defined fds_bits correctly, but lose by not defining this */
#define FD_ZERO(p)  ((p)->fds_bits[0] = 0)
#define FD_SET(n, p)   ((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)   ((p)->fds_bits[0] & (1 << (n)))
#endif

char	*find_dev();

recvjob()
{
	struct stat stb;
	char *bp = pbuf;
	int status, rcleanup();

	/*
	 * Perform lookup for printer name or abbreviation
	 */
	if(lflag) syslog(LOG_INFO, "in recvjob");
#ifdef HESIOD
	if ((status = pgetent(line, printer)) <= 0) {
		if (pralias(alibuf, printer))
			printer = alibuf;
		if ((status = hpgetent(line, printer)) < 1)
			frecverr("unknown printer %s", printer);
	}
#else
	if ((status = pgetent(line, printer)) < 0) {
		frecverr("cannot open printer description file");
	}
 	else if (status == 0)
		frecverr("unknown printer %s", printer);
#endif HESIOD
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = DEFLOGF;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
#ifdef PQUOTA
	RQ = pgetstr("rq", &bp);
	QS = pgetstr("qs", &bp);
#endif PQUOTA	    
#ifdef LACL
	AC = pgetstr("ac", &bp);
	PA = pgetflag("pa");
	RA = pgetflag("ra");
#endif /* LACL */
	    
	(void) close(2);			/* set up log file */
	if (open(LF, O_WRONLY|O_APPEND, 0664) < 0) {
		syslog(LOG_ERR, "%s: %m", LF);
		(void) open("/dev/null", O_WRONLY);
	}

	if (chdir(SD) < 0)
		frecverr("%s: %s: %m", printer, SD);
	if (stat(LO, &stb) == 0) {
		if (stb.st_mode & 010) {
			/* queue is disabled */
			putchar('\1');		/* return error code */
			exit(1);
		}
	} else if (stat(SD, &stb) < 0)
		frecverr("%s: %s: %m", printer, SD);
	minfree = read_number("minfree");
	ddev = find_dev(stb.st_dev, S_IFBLK);
	if ((dfd = open(ddev, O_RDONLY)) < 0)
		syslog(LOG_WARNING, "%s: %s: %m", printer, ddev);

	signal(SIGTERM, rcleanup);
	signal(SIGPIPE, rcleanup);

	if(lflag) syslog(LOG_INFO, "Reading job");
	if (readjob())
	  {
	    if (lflag) syslog(LOG_INFO, "Printing job..");
	    printjob();
	  }
	
}

char *
find_dev(dev, type)
	register dev_t dev;
	register int type;
{
	register DIR *dfd = opendir("/dev");
#if defined(_IBMR2) || defined(POSIX)
	struct dirent *dir;
#else
	struct direct *dir;
#endif
	struct stat stb;
	char devname[MAXNAMLEN+6];
	char *dp;

	strcpy(devname, "/dev/");
	while ((dir = readdir(dfd))) {
		strcpy(devname + 5, dir->d_name);
		if (stat(devname, &stb))
			continue;
		if ((stb.st_mode & S_IFMT) != type)
			continue;
		if (dev == stb.st_rdev) {
			closedir(dfd);
			dp = (char *)malloc(strlen(devname)+1);
			strcpy(dp, devname);
			return(dp);
		}
	}
	closedir(dfd);
	frecverr("cannot find device %d, %d", major(dev), minor(dev));
	/*NOTREACHED*/
}

/*
 * Read printer jobs sent by lpd and copy them to the spooling directory.
 * Return the number of jobs successfully transfered.
 */
readjob()
{
	register int size, nfiles;
	register char *cp;
#if defined(PQUOTA) || defined(LACL)
	char *cret;
#endif
#ifdef PQUOTA
	char *check_quota();
#endif
#ifdef LACL
	char *check_lacl(), *check_remhost();
#endif

	if (lflag) syslog(LOG_INFO, "In readjob");
	ack();
	nfiles = 0;
	for (;;) {
		/*
		 * Read a command to tell us what to do
		 */
		cp = line;
		do {
			if ((size = read(1, cp, 1)) != 1) {
				if (size < 0)
					frecverr("%s: Lost connection",printer);
				if (lflag) syslog(LOG_INFO, "Returning from readjobs");
				return(nfiles);
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = line;
		switch (*cp++) {
		case '\1':	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case '\2':	/* read cf file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			/*
			 * host name has been authenticated, we use our
			 * view of the host name since we may be passed
			 * something different than what gethostbyaddr()
			 * returns
			 */
			strcpy(cp + 6, from);
			strcpy(cfname, cp);
			strcpy(tfname, cp);
			tfname[0] = 't';
#ifdef KERBEROS
			strcpy(tempfile, tfname);
			tempfile[0] = 'T';
#endif KERBEROS
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			    
			/* Don't send final acknowledge beacuse we may wish to 
			   send error below */
			if (!readfile(tfname, size, 0)) {
			    syslog(LOG_DEBUG, "Failed read");
				rcleanup();
				continue;
			}

#ifdef KERBEROS
			if (kerberos_cf && (!kerberize_cf(tfname, tempfile))) {
				rcleanup();
				continue;
			}
#endif KERBEROS

#ifdef LACL
			if(RA && ((cret = check_remhost()) != 0)) {
			    (void) write(1, cret, 1);
			    rcleanup();
			    continue;
			}

			if(AC && (cret = check_lacl(tfname)) != 0) {
			    /* We return !=0 for error. Old clients
			       stupidly don't print any error in this sit.
			       We do a cleanup cause we can't expect 
			       client to do so. */
			    (void) write(1, cret, 1);
#ifdef DEBUG
			    syslog(LOG_DEBUG, "Got %s", cret);
#endif DEBUG
			    rcleanup();
			    continue;
			}
#endif /*LACL*/

#ifdef PQUOTA
			if(kerberos_cf && (RQ != NULL) && 
			   (cret = check_quota(tfname)) != 0) {
			    /* We return !=0 for error. Old clients
			       stupidly don't print any error in this sit.
			       We do a cleanup cause we can't expect 
			       client to do so. */
			    (void) write(1, cret, 1);
#ifdef DEBUG
			    syslog(LOG_DEBUG, "Got %s", cret);
#endif DEBUG
			    rcleanup();
			    continue;
			}
#endif PQUOTA

			/* Send acknowldege, cause we didn't before */
			ack();

			if (link(tfname, cfname) < 0)
				frecverr("%s: %m", tfname);
			(void) UNLINK(tfname);
			tfname[0] = '\0';
			nfiles++;
			continue;

		case '\3':	/* read df file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}

			(void) strcpy(dfname, cp);
			if (index(dfname, '/'))
				frecverr("illegal path name");
			(void) readfile(dfname, size, 1);
			continue;
		}
		frecverr("protocol screwup");
	}
}

/*
 * Read files send by lpd and copy them to the spooling directory.
 */
readfile(file, size, acknowledge)
	char *file;
	int size;
        int acknowledge;
{
	register char *cp;
	char buf[BUFSIZ];
	register int i, j, amt;
	int fd, err;

	fd = open(file, O_WRONLY|O_CREAT, FILMOD);
	if (fd < 0)
		frecverr("%s: %m", file);
	ack();
	err = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		amt = BUFSIZ;
		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			j = read(1, cp, amt);
			if (j <= 0)
				frecverr("Lost connection");
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (write(fd, buf, amt) != amt) {
			err++;
			break;
		}
	}
	(void) close(fd);
	if (err)
		frecverr("%s: write error", file);
	if (noresponse()) {		/* file sent had bad data in it */
		(void) UNLINK(file);
		return(0);	
	    }
	if(acknowledge)
	    ack();
	return(1);
}

#ifdef KERBEROS
kerberize_cf(file, tfile)
char *file, *tfile;
{
	FILE *cfp, *tfp;
	char kname[ANAME_SZ + INST_SZ + REALM_SZ + 3];
	char oldname[ANAME_SZ + INST_SZ + REALM_SZ + 3];

	oldname[0] = '\0';

	/* Form a complete string name consisting of principal, 
	 * instance and realm
	 */
	make_kname(kprincipal, kinstance, krealm, kname);

	/* If we cannot open tf file, then return error */
	if ((cfp = fopen(file, "r")) == NULL)
		return (0);

	/* Read the control file for the person sending the job */
	while (getline(cfp)) {
		if (line[0] == 'P') {
			strncpy(oldname, line+1, sizeof(oldname)-1);
			break;
		}
	}
	fclose(cfp);

	/* Have we got a name in oldname, if not, then return error */
	if (oldname[0] == '\0')
		return(0);

	/* Does kname match oldname. If so do nothing */
	if (!strcmp(kname, oldname))
		return(1); /* all a-okay */

	/* hmm, doesnt match, guess we have to change the name in
	 * the control file by doing the following :
	 *
	 * (1) Move 'file' to 'tfile'
	 * (2) Copy all of 'tfile' back to 'file' but
	 *     changing the persons name
	 */
	if (link(file, tfile) < 0)
		return(0);
	(void) UNLINK(file);

	/* If we cannot open tf file, then return error */
	if ((tfp = fopen(tfile, "r")) == NULL)
		return (0);
	if ((cfp = fopen(file, "w")) == NULL) {
		(void) fclose(tfp);
		return (0);
	}

	while (getline(tfp)) {
		if (line[0] == 'P')
			strcpy(&line[1], kname);
		else if (line[0] == 'L')
		    strcpy(&line[1], kname);
		fprintf(cfp, "%s\n", line);
	}

	(void) fclose(cfp);
	(void) fclose(tfp);
	(void) UNLINK(tfile);

	return(1);
}
#endif KERBEROS

noresponse()
{
	char resp;

	if (read(1, &resp, 1) != 1)
		frecverr("Lost connection");
	if (resp == '\0')
		return(0);
	return(1);
}

/*
 * Check to see if there is enough space on the disk for size bytes.
 * 1 == OK, 0 == Not OK.
 */
chksize(size)
	int size;
{
#if (defined(AIX) && defined(i386)) || defined(_IBMR2)
	/* This is really not appropriate, but maybe someday XXX */
	return 1;
#else
	int spacefree;
	struct fs fs;

	if (dfd < 0 || lseek(dfd, (long)(SBLOCK * DEV_BSIZE), 0) < 0)
		return(1);
	if (read(dfd, (char *)&fs, sizeof fs) != sizeof fs)
		return(1);
	spacefree = (fs.fs_cstotal.cs_nbfree * fs.fs_frag +
		fs.fs_cstotal.cs_nffree - fs.fs_dsize * fs.fs_minfree / 100) *
			fs.fs_fsize / 1024;
	size = (size + 1023) / 1024;
	if (minfree + size > spacefree)
		return(0);
	return(1);
#endif /* AIX & i386 */
}

read_number(fn)
	char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

/*
 * Remove all the files associated with the current job being transfered.
 */
rcleanup()
{

	/* This was cretinous code.. which regularly walked off the end
	 * of the name space...  I changed the != to a >=..
	 */

	if (tfname[0])
		(void) UNLINK(tfname);
#ifdef KERBEROS
	if (tempfile[0])
		(void) UNLINK(tempfile);
#endif KERBEROS
	if (dfname[0])
		do {
			do
				(void) UNLINK(dfname);
			while (dfname[2]-- >= 'A');
			dfname[2] = 'z';
		} while (dfname[0]-- >= 'd');
	dfname[0] = '\0';
}

/* VARARGS1 */
frecverr(msg, a1, a2)
	char *msg;
{
	rcleanup();
	syslog(LOG_ERR, msg, a1, a2);
	putchar('\1');		/* return error code */
	exit(1);
}

#ifdef PQUOTA

char* check_quota(file)
char file[];
    {
        struct hostent *hp;
	char outbuf[BUFSIZ], inbuf[BUFSIZ];
	int t, act=0, s1;
	FILE *cfp;
	struct sockaddr_in sin_c;
	int fd, retry;
	struct servent *servname;
	struct timeval tp;
	fd_set set;

	if(RQ == NULL) 
	    return 0;
	if((hp = gethostbyname(RQ)) == NULL) {
	    syslog(LOG_WARNING, "Cannot resolve quota servername %s", RQ);
	    return 0;
	}

	/* Setup output buffer.... */
	outbuf[0] = (char) UDPPROTOCOL;

	/* Generate a sequence number... Since we fork the only realistic
	   thing to use is the time... */
	t = htonl(time((char *) 0));
	bcopy(&t, outbuf + 1, 4);
	strncpy(outbuf + 4, printer, 30);


	if(QS == NULL) 
	    outbuf[39] = '\0';
	else 
	    strncpy(outbuf + 39, QS, 20);
	/* If can't open the control file, then there is some error...
	   We'll return allowed to print, but somewhere else it will be caught.
	   Is this proper? XXX
	   */

	if ((cfp = fopen(file, "r")) == NULL)
		return 0;

	/* Read the control file for the person sending the job */
	while (getline(cfp)) {
		if (line[0] == 'Q' || line[0] == 'A') { /* 'A' for old clients */
		    if(sscanf(line + 1, "%d", &act) != 1) act=0;
		    break;
		}
	}
	fclose(cfp);

       	act = htonl(act);
	bcopy(&act, outbuf + 35, 4);

	strncpy(outbuf + 59, kprincipal, ANAME_SZ);
	strncpy(outbuf + 59 + ANAME_SZ, kinstance, INST_SZ);
	strncpy(outbuf + 59 + ANAME_SZ + INST_SZ, krealm, REALM_SZ);

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    syslog(LOG_WARNING, "Could not create UDP socket\n");
	    /* Allow print */
	    return 0;
	}

	bzero((char *)&sin_c, sizeof(sin_c));
	sin_c.sin_family = AF_INET;
	servname = getservbyname(QUOTASERVENTNAME,"udp");
	if(!servname) 
	    sin_c.sin_port = htons(QUOTASERVENT);
	else 
	    sin_c.sin_port = servname->s_port;

	bcopy(hp->h_addr_list[0], &sin_c.sin_addr,hp->h_length);

	if(connect(fd, &sin_c, sizeof(sin_c)) < 0) {
	    syslog(LOG_WARNING, "Could not connect with UDP - quota server down?");
	    /* This means that the quota serve is down */
	    /* Allow printing */
	    return 0;
	}

	for(retry = 0; retry < RETRY_COUNT; retry++) {
	    if(send(fd, outbuf, 59+ANAME_SZ+REALM_SZ+INST_SZ+1,0)<
	       59+ANAME_SZ+REALM_SZ+INST_SZ+1) {
		syslog(LOG_WARNING, "Send failed to quota");
		continue;
	    }

	    FD_ZERO(&set);
	    FD_SET(fd, &set);
	    tp.tv_sec = UDPTIMEOUT;
	    tp.tv_usec = 0;

	    /* So, select and wait for reply */
	    if((s1=select(fd+1, &set, 0, 0, &tp))==0) {
		/*Time out, retry */
		continue;
	    }

	    if(s1 < 0) {
		/* Error, which makes no sense. Oh well, display */
		syslog(LOG_WARNING, "Error in UDP return errno=%d", errno);
		/* Allow print */
		return 0;
	    }

	    if((s1=recv(fd, inbuf, 36)) != 36) {
		syslog(LOG_WARNING, "Receive error in UDP contacting quota");
		/* Retry */
		continue;
	    }

	    if(bcmp(inbuf, outbuf, 35)) {
		/* Wrong packet */
#ifdef DEBUG
		syslog(LOG_DEBUG, "Packet not for me on UDP");
#endif
		continue;
	    }

	    /* Packet good, send response */
	    switch ((int) inbuf[35]) {
	    case ALLOWEDTOPRINT:
#ifdef DEBUG
		syslog(LOG_DEBUG, "Allowed to print!!");
#endif
		return 0;
	    case NOALLOWEDTOPRINT:
		return "\4";
	    case UNKNOWNUSER:
		return "\3";
	    case UNKNOWNGROUP:
		return "\5";
	    case USERNOTINGROUP:
		return "\6";
	    case USERDELETED:
		return "\7";
	    case GROUPDELETED:
		return "\10";
	    default:
		break;
		/* Bogus, retry */
	    }
		    
	}

	if(retry == RETRY_COUNT) {
	    /* We timed out in contacting... Allow printing*/
	    return 0;
	}
	return 0;
    }

#endif

#ifdef LACL
char *check_lacl(file)
char *file;
    {
	FILE *cfp;
	char person[BUFSIZ];
#ifdef KERBEROS
	extern char local_realm[];
#endif
	person[0] = '\0';

	if(!AC) {
	    syslog("lpd: ACL file not set");
	    return NULL;
	}
	if(access(AC, R_OK)) {
	    syslog(LOG_ERR, "lpd: Could not find ACL file %s", AC);
	    return NULL;
	}
	if ((cfp = fopen(file, "r")) == NULL)
		return 0;

	/* Read the control file for the person sending the job */
	while (getline(cfp)) {
		if (line[0] == 'P' && line[1]) {
		    strcpy(person, line + 1);
		    break;
		}
	}
	fclose(cfp);

	if(!person[0]) {
#ifdef DEBUG
	    syslog(LOG_DEBUG, "Person not found :%s", line);
#endif
	    goto notfound;
	}
#ifdef DEBUG
	else 
	    syslog(LOG_DEBUG, "Found person :%s:%s", line, person);
#endif

#ifdef KERBEROS
	/* Now to tack the realm on */
	if(kerberos_cf && !index(person, '@')) {
	    strcat(person,"@");
	    strcat(person, local_realm);
	}

#endif /* KERBEROS */

#ifdef DEBUG
	syslog(LOG_DEBUG, "Checking on :%s: ", person);
#endif

	/* Now see if the person is in AC */

	if ((cfp = fopen(AC, "r")) == NULL)
		return 0;
	   
	while(getline(cfp)) {
	    if(!strcasecmp(person, line)) {
		fclose(cfp);
		goto found;
	    }
	}
	fclose(cfp);

    notfound:
	if(PA) return "\4"; /* NOALLOWEDTOPRINT */
	else return NULL;

    found:
	if(PA) return NULL;
	else return "\4"; /* NOALLOWEDTOPRINT */
    }

char *
check_remhost()
{
    register char *cp, *sp;
    extern char from_host[];
    register FILE *hostf;
    char ahost[MAXHOSTNAMELEN];
    int baselen = -1;

    if(!strcasecmp(from_host, host)) return NULL;
#if 0
    syslog(LOG_DEBUG, "About to check on %s\n", from_host);
#endif
    sp = from_host;
    cp = ahost;
    while (*sp) {
	if (*sp == '.') {
	    if (baselen == -1)
		baselen = sp - from_host;
	    *cp++ = *sp++;
	} else {
	    *cp++ = isupper(*sp) ? tolower(*sp++) : *sp++;
	}
    }
    *cp = '\0';
    hostf = fopen("/etc/hosts.lpd", "r");
#define DUMMY ":nobody::"
    if (hostf) {
	if (!_validuser(hostf, ahost, DUMMY, DUMMY, baselen)) {
	    (void) fclose(hostf);
	    return NULL;
	}
	(void) fclose(hostf);
	return "\4";
    } else {
	/* Could not open hosts.lpd file */
	return NULL;
    }
}
#endif /* LACL */
