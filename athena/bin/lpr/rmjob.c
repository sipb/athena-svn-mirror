/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/rmjob.c,v $
 *	$Author: epeisach $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/rmjob.c,v 1.5 1991-06-28 13:35:42 epeisach Exp $
 */

#ifndef lint
static char *rcsid_rmjob_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/rmjob.c,v 1.5 1991-06-28 13:35:42 epeisach Exp $";
#endif lint

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
static char sccsid[] = "@(#)rmjob.c	5.1 (Berkeley) 6/6/85";
#endif not lint

/*
 * rmjob - remove the specified jobs from the queue.
 */

#include "lp.h"

/*
 * Stuff for handling lprm specifications
 */
extern char	*user[];		/* users to process */
extern int	users;			/* # of users in user array */
extern int	requ[];			/* job number of spool entries */
extern int	requests;		/* # of spool requests */
extern char	*person;		/* name of person doing lprm */

char	root[] = "root";
int	all = 0;		/* eliminate all files (root only) */
int	cur_daemon;		/* daemon's pid */
char	current[40];		/* active control file name */
int	assasinated = 0;	/* 1 means we've killed the lpd */

#if defined(KERBEROS) && !defined(SERVER)
extern int      use_kerberos;
extern int      kerberos_override;
short KA;
KTEXT_ST kticket;
long kerror;
#endif /* KERBEROS */

#ifdef SERVER
int	iscf();
#endif /* SERVER */

rmjob()
{
#ifndef SERVER
	register int i;
#else /* SERVER */
	register int i, nitems;
#ifdef POSIX
	struct dirent **files;
#else
	struct direct **files;
#endif
#endif /* SERVER */

	assasinated = 0;	/* Haven't killed it yet! */
#ifdef HESIOD
	if ((i = pgetent(line, printer)) <= 0) {
		if (pralias(alibuf, printer))
			printer = alibuf;
		if ((i = hpgetent(line, printer)) < 1)
			fatal("unknown printer");
	}
#else
	if ((i = pgetent(line, printer)) < 0) {
		fatal("cannot open printer description file");
	} else if (i == 0)
		fatal("unknown printer");
#endif /* HESIOD */
#ifdef SERVER
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;
#endif /* SERVER */
	if ((LP = pgetstr("lp", &bp)) == NULL)
		LP = DEFDEVLP;
	if ((RP = pgetstr("rp", &bp)) == NULL)
		RP = DEFLP;
	RM = pgetstr("rm", &bp);

#if defined(KERBEROS) && !defined(SERVER)
        KA = pgetnum("ka");
        if (KA > 0)
            use_kerberos = 1;
        else
            use_kerberos = 0;
        if (kerberos_override > -1)
            use_kerberos = kerberos_override;
#endif /* KERBEROS */

	/*
	 * If the format was `lprm -' and the user isn't the super-user,
	 *  then fake things to look like he said `lprm user'.
	 */
	if (users < 0) {
		if (getuid() == 0)
			all = 1;	/* all files in local queue */
		else {
			user[0] = person;
			users = 1;
		}
	}
	if (!strcmp(person, "-all")) {
		if (from == host)
			fatal("The login name \"-all\" is reserved");
		all = 1;	/* all those from 'from' */
		person = root;
	}
	
#ifdef SERVER
	if (chdir(SD) < 0)
		fatal("cannot chdir to spool directory");
	if ((nitems = scandir(".", &files, iscf, NULL)) < 0)
		fatal("cannot access spool directory");

	if (nitems) {
#ifdef notdef
		/*
		 * Check for an active printer daemon (in which case we
		 *  kill it if it is reading our file) then remove stuff
		 *  (after which we have to restart the daemon).
		 */
		if (lockchk(LO) && chk(current)) {
			assasinated = kill(cur_daemon, SIGINT) == 0;
			if (!assasinated)
				fatal("cannot kill printer daemon");
		}
#endif
		/*
		 * process the files
		 */
		for (i = 0; i < nitems; i++)
			process(files[i]->d_name);
	}
	chkremote();
	/*
	 * Restart the printer daemon if it was killed
	 */
	if (assasinated && !startdaemon(printer))
		fatal("cannot restart printer daemon\n");
#else /* SERVER */
	chkremote();
#endif /* SERVER */
	exit(0);
}

#ifdef SERVER
/*
 * Process a lock file: collect the pid of the active
 *  daemon and the file name of the active spool entry.
 * Return boolean indicating existence of a lock file.
 */
lockchk(s)
	char *s;
{
	register FILE *fp;
	register int i, n;

	cur_daemon = -1;	/* Initialize to no daemon */
	
	if ((fp = fopen(s, "r")) == NULL)
		if (errno == EACCES)
			fatal("can't access lock file");
		else
			return(0);
	if (!getline(fp)) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	cur_daemon = atoi(line);
	if (kill(cur_daemon, 0) < 0) {
		(void) fclose(fp);
		return(0);		/* no daemon present */
	}
	for (i = 1; (n = fread(current, sizeof(char), sizeof(current), fp)) <= 0; i++) {
		if (i > 5) {
			n = 1;
			break;
		}
		sleep(i);
	}
	current[n-1] = '\0';
	(void) fclose(fp);
	return(1);
}

/*
 * Process a control file.
 */
process(file)
	char *file;
{
	FILE *cfp;
	int	flock_retry = 0;

	if (!chk(file))
		return;
	if ((cfp = fopen(file, "r")) == NULL)
		fatal("cannot open %s", file);
	while (flock(fileno(cfp), LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK) {
			/*
			 * We couldn't get the lock; lpd must be
			 * using the control file.  So try to
			 * blow it away.
			 *
			 * Note: assasumes lockchk has already been
			 * run, so cur_daemon contains valid
			 * information. 
			 */
			if (!lockchk(LO))
				/* No daemon, must have just */
				/* exited.... */
				continue;
			syslog(LOG_DEBUG, "Killing printer daemon %d",
			       cur_daemon);
			if (assasinated = kill(cur_daemon, SIGINT) == 0)
				sleep(1);
			else {
				syslog(LOG_ERR, "kill %d: %m", cur_daemon);
#ifdef notdef
				fatal("cannot kill printer daemon");
#endif
			}
			flock_retry++;
			if (flock_retry > 2)
				fatal("cannot obtain lock on control file");
		} else {
			syslog(LOG_ERR, "%s: %s: %m", printer, file);
			return;
		}
	}
	while (getline(cfp)) {
		switch (line[0]) {
		case 'U':  /* unlink associated files */
			if (from != host)
				printf("%s: ", host);
			printf(UNLINK(line+1) ? "cannot dequeue %s\n" :
				"%s dequeued\n", line+1);
		}
	}
	(void) fclose(cfp);
	if (from != host)
		printf("%s: ", host);
	printf(UNLINK(file) ? "cannot dequeue %s\n" : "%s dequeued\n", file);
}

/*
 * Do the dirty work in checking
 */
chk(file)
	char *file;
{
	register int *r, n;
	register char **u, *cp;
	FILE *cfp;

	/*
	 * Check for valid cf file name (mostly checking current).
	 */
	if (strlen(file) < 7 || file[0] != 'c' || file[1] != 'f')
		return(0);

	if (all && (from == host || !strcmp(from, file+6)))
		return(1);

	/*
	 * get the owner's name from the control file.
	 */
	if ((cfp = fopen(file, "r")) == NULL)
		return(0);
	while (getline(cfp)) {
		if (line[0] == 'P')
			break;
	}
	(void) fclose(cfp);
	if (line[0] != 'P')
		return(0);

	if (users == 0 && requests == 0)
		return(!strcmp(file, current) && isowner(line+1, file));
	/*
	 * Check the request list
	 */
	for (n = 0, cp = file+3; isdigit(*cp) && cp != file+6; )
		n = n * 10 + (*cp++ - '0');
	for (r = requ; r < &requ[requests]; r++)
		if (*r == n && isowner(line+1, file))
			return(1);
	/*
	 * Check to see if it's in the user list
	 */
	for (u = user; u < &user[users]; u++)
		if (!strcmp(*u, line+1) && isowner(line+1, file))
			return(1);
	return(0);
}

/*
 * If root is removing a file on the local machine, allow it.
 * If root is removing a file from a remote machine, only allow
 * files sent from the remote machine to be removed.
 * Normal users can only remove the file from where it was sent.
 */
isowner(owner, file)
	char *owner, *file;
{
	if (!strcmp(person, root) && (from == host || !strcmp(from, file+6)))
		return(1);
#ifdef KERBEROS
	if (!strcmp(person, owner))
		return(1);
#else
	if (!strcmp(person, owner) && !strcmp(from, file+6))
		return(1);
#endif /* KERBEROS */
	if (from != host)
		printf("%s: ", host);
	printf("%s: Permission denied\n", file);
	return(0);
}

#endif SERVER
/*
 * Check to see if we are sending files to a remote machine. If we are,
 * then try removing files on the remote machine.
 */
chkremote()
{
	register char *cp;
	register int i, rem;
#ifndef SERVER
	register int resp;
	int n;
#endif
	char buf[BUFSIZ];


#ifndef SERVER
	char name[255];
	struct hostent *hp;
#else /* SERVER */
	/*
	 * Figure out whether the local machine is the same as the remote 
	 * machine entry (if it exists).  If not, then ignore the local
	 * queue information.
	 */
#endif /* SERVER */

#ifndef SERVER
	if (RM == NULL) {
		/* get the name of the local host */
#else /* SERVER */
	if (RM == (char *) NULL) return;
	else {
		char name[255];
		struct hostent *hp;

			/* get the name of the local host */
#endif /* SERVER */
		gethostname (name, sizeof(name) - 1);
		name[sizeof(name)-1] = '\0';
#ifndef SERVER
		/* get the network standard name of the local host */
		if (hp =gethostbyname(name))
			strcpy (name, hp->h_name);
		RM = name;
#else /* SERVER */
			/* get the network standard name of the local host */
		hp = gethostbyname (name);
		if (hp == (struct hostent *) NULL) {
		    printf ("unable to get hostname for local machine %s\n",
				name);
		    return;
		} else strcpy (name, hp->h_name);

			/* get the network standard name of RM */
		hp = gethostbyname (RM);
		if (hp == (struct hostent *) NULL) {
		    printf ("unable to get hostname for remote machine %s\n",
		    		RM);
		    return;
		}

			/* if printer is not on local machine, ignore LP */
		if (strcasecmp (name, hp->h_name) != 0) *LP = '\0';
		else return;	/* local machine */
#endif SERVER
	}

	/*
	 * Flush stdout so the user can see what has been deleted
	 * while we wait (possibly) for the connection.
	 */
	fflush(stdout);

	sprintf(buf, "\5%s %s", RP, all ? "-all" : person);
	cp = buf;
	for (i = 0; i < users; i++) {
		cp += strlen(cp);
		*cp++ = ' ';
		(void) strcpy(cp, user[i]);
	}
	for (i = 0; i < requests; i++) {
		cp += strlen(cp);
		(void) sprintf(cp, " %d", requ[i]);
	}
	(void) strcat(cp, "\n");
	rem = getport(RM);
	if (rem < 0) {
		if (from != host)
			printf("%s: ", host);
		printf("connection to %s is down\n", RM);
	} else {
#ifndef SERVER
#ifdef KERBEROS
		if (use_kerberos) {
                        /* If we require kerberos authentication,
                         * then send credentials
                         * over
                         */
                        (void) sprintf(line, "k%s\n", RP);
                        n = strlen(line);
                        if (write(rem, line, n) != n)
				fatal("Error sending kerberos opcode.\n");

			if ((resp = responser(rem)) != '\0') {
			    fprintf(stderr,
				    "Remote printer does not support kerberos authentication\n");
			    if(kerberos_override == 1) 
				fprintf(stderr, "Try again without the -k flag\n");
			    if(kerberos_override == -1) 
				fprintf(stderr,"Try again using the -u option\n");
			    exit(1);
			}
			
                        kerror = krb_sendauth(0L, rem, &kticket, KLPR_SERVICE,
                                              RM, (char *)krb_realmofhost(RM),
                                              0, (MSG_DAT *) 0,
                                              (CREDENTIALS *) 0,
                                              (bit_64 *) 0,
                                              (struct sockaddr_in *)0,
                                              (struct sockaddr_in *)0,
                                              "KLPRV0.1");
                        if (kerror != KSUCCESS)
                            fatal("Kerberos authentication failed. Use kinit and try again.\n");
			if ((resp = responser(rem)) != '\0') {
			    if (resp == '\3') 
				fatal("Authentication failed. Use kinit and then try again.\n");
			    else fatal("Syncronization error.\n");
			}
		    }
#endif /* KERBEROS */
#endif /* !(SERVER) */
		i = strlen(buf);
		if (write(rem, buf, i) != i)
			fatal("Lost connection");
		while ((i = read(rem, buf, sizeof(buf))) > 0)
			(void) fwrite(buf, 1, i, stdout);
		(void) close(rem);
	}
}

#ifdef SERVER
/*
 * Return 1 if the filename begins with 'cf'
 */
iscf(d)
#ifdef POSIX
	struct dirent *d;
#else
	struct direct *d;
#endif
{
	return(d->d_name[0] == 'c' && d->d_name[1] == 'f');
}
#endif SERVER

/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */

#if !defined(SERVER)
static responser(fd)
int fd;
{
	char resp;

	if (read(fd, &resp, 1) != 1) {
		fprintf(stderr,"Lost connection to printer....\n");
		return(-1);
	}
	return(resp);
}
#endif
