/*
 * Netsend - sends a file to a remote lpd.  Code is taken from
 * printjob.c, with demon code references taken out.
 *
 * 	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/netsend.c,v $
 * 	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/netsend.c,v 1.6 1992-11-09 00:51:18 probe Exp $
 */

#ifndef lint
static char *rcsid_netsend_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/netsend.c,v 1.6 1992-11-09 00:51:18 probe Exp $";
#endif lint

#define TMPDIR "/tmp"

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


/*
 * printjob -- print jobs in the queue.
 *
 *	NOTE: the lock file is used to pass information to lpq and lprm.
 *	it does not need to be removed because file locks are dynamic.
 */

#include "lp.h"

#define DORETURN	0	/* absorb fork error */
#define DOABORT		1	/* abort if dofork fails */
/*
 * Error tokens
 */
#define REPRINT		-2
#define ERROR		-1
#define	OK		0
#define	FATALERR	1
#define	NOACCT		2
#define	FILTERERR	3
#define	ACCESS		4

FILE	*cfp;			/* control file */
int	pfd;			/* printer file descriptor */
int	ofd;			/* output filter file descriptor */
int	ofilter;		/* id of output filter, if any */
int	remote;			/* true if sending files to remote */
dev_t	fdev;			/* device of file pointed to by symlink */
ino_t	fino;			/* inode of file pointed to by symlink */

#if BUFSIZ != 1024
#undef BUFSIZ
#define BUFSIZ 1024
#endif

#ifdef KERBEROS
KTEXT_ST kticket;
long kerror;
extern int kerberos_override;
#endif KERBEROS

/*
 * Send the daemon control file (cf) and any data files.
 * Return -1 if a non-recoverable error occured, 1 if a recoverable error and
 * 0 if all is well.
 */
sendit(file)
	char *file;
{
	register int i, err = OK;
	char *cp, last[BUFSIZ];

	if (chdir(TMPDIR)) {
		perror("cannot chdir to TMPDIR");
		cleanup();
	}
	/*
	 * open control file
	 */
	if ((cfp = fopen(file, "r")) == NULL) {
		perror(file);
		return(ERROR);
	}
	/*
	 *      read the control file for work to do
	 *
	 *      file format -- first character in the line is a command
	 *      rest of the line is the argument.
	 *      commands of interest are:
	 *
	 *            a-z -- "file name" name of file to print
	 *              U -- "unlink" name of file to remove
	 *                    (after we print it. (Pass 2 only)).
	 */

	/*
	 * pass 1
	 */
	while (getline(cfp)) {
	again:
		if (line[0] == 'S') {
			cp = line+1;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fdev = i;
			cp++;
			i = 0;
			while (*cp >= '0' && *cp <= '9')
				i = i * 10 + (*cp++ - '0');
			fino = i;
			continue;
		}
		if (line[0] >= 'a' && line[0] <= 'z') {
			(void) strcpy(last, line);
			while (i = getline(cfp))
				if (strcmp(last, line))
					break;
			switch (sendfile('\3', last+1)) {
			case OK:
				if (i)
					goto again;
				break;
			case REPRINT:
				(void) fclose(cfp);
				return(REPRINT);
			case ERROR:
				err = ERROR;
			}
			break;
		}
	}
	if (err == OK && sendfile('\2', file) > 0) {
		(void) fclose(cfp);
		return(REPRINT);
	}
	/*
	 * pass 2
	 */
	fseek(cfp, 0L, 0);
	while (getline(cfp))
		if (line[0] == 'U')
			(void) UNLINK(line+1);
	/*
	 * clean-up in case another control file exists
	 */
	(void) fclose(cfp);
	(void) UNLINK(file);
	return(err);
}

/*
 * Send a data file to the remote machine and spool it.
 * Return positive if we should try resending.
 */
sendfile(type, file)
	char type, *file;
{
	register int f, i, amt;
	struct stat stb;
	char buf[BUFSIZ];
	int sizerr, resp;

	if (stat(file, &stb) < 0 || (f = open(file, O_RDONLY)) < 0)
		return(ERROR);
	(void) sprintf(buf, "%c%d %s\n", type, stb.st_size, file);
	amt = strlen(buf);
	for (i = 0;  ; i++) {
		if (write(pfd, buf, amt) != amt ||
 		    (resp = response()) < 0 || resp == '\1') {
			(void) close(f);
			return(REPRINT);
		} else if (resp == '\0')
			break;
		if (i>6) {
			fprintf(stderr,"The printer queue is full.  Please ");
			fprintf(stderr,"try again in a few minutes.\n");
			cleanup();
		}
		sleep(5);
	}
	sizerr = 0;
	for (i = 0; i < stb.st_size; i += BUFSIZ) {
		amt = BUFSIZ;
		if (i + amt > stb.st_size)
			amt = stb.st_size - i;
		if (sizerr == 0 && read(f, buf, amt) != amt)
			sizerr = 1;
		if (write(pfd, buf, amt) != amt) {
			(void) close(f);
			return(REPRINT);
		}
	}
	(void) close(f);
	if (sizerr) {
		fprintf(stderr,
			"Warning: %s was not printed - size sync problem\n",
			file);
		/* tell recvjob to ignore this file */
		(void) write(pfd, "\1", 1);
		return(ERROR);
	}
	if (write(pfd, "", 1) != 1 || (resp=response())) {
#ifdef PQUOTA
	    /* Gross but lpr didnt have any error returns. Something
	     * is better than nothing :)
	     */
	    if (resp == '\3') {
		fprintf(stderr, "You are not known by the quota server and ");
		fprintf(stderr, "are not allowed to print. \n");
		fprintf(stderr, "See User Accounts to be added.\n");
		cleanup();	/* Never returns */
	    } else if (resp == '\4') {
		fprintf(stderr, 
			"You are not allowed to print on this printer. Please contact Athena\n");
		/* You cannot be over quota, because of policy... */
		fprintf(stderr, 
			"User Accounts (x3-1325) or your Cluster Manager for more information.\n");
		cleanup();	/* Never returns */
	    } else if (resp == '\5') {
		fprintf(stderr, 
			"The account number is not known by the quota server.\n");
		cleanup();	/* Never returns */
	    } else if (resp == '\6') {
		fprintf(stderr, "You are not a member of the group account.\n");
		fprintf(stderr, 
			"See one of the group's administrator to be added.\n");
		cleanup();
	    } else if (resp == '\7') {
		fprintf(stderr, 
			"You are marked for deletion on the quota server.");
		fprintf(stderr, 
			"\nContact User Accounts if you should not be.\n");
		cleanup();	/* Never returns */
	    } else if (resp == '\10') {
		fprintf(stderr, 
			"The account number is marked as deleted on the quota server.");
		fprintf(stderr, 
			"\nContact User Accounts if it should not be.\n");
		cleanup();	/* Never returns */
	    }
#endif PQUOTA
	    return(REPRINT);
	}
	return(OK);
}

/*
 * Check to make sure there have been no errors and that both programs
 * are in sync with eachother.
 * Return non-zero if the connection was lost.
 */
response()
{
	char resp;

	if (read(pfd, &resp, 1) != 1) {
		fprintf(stderr,"Lost connection to printer....\n");
		return(-1);
	}
	return(resp);
}

/*
 * Acquire line printer or remote connection.
 */
openpr()
{
    register int i, n;
    int resp;

    for (i = 0; ; i++) {
	resp = -1;
	pfd = getport(RM);
	if (pfd >= 0) {
#ifdef KERBEROS
	    if (use_kerberos) {
		/* If we require kerberos authentication, 
		 * then send credentials
		 * over
		 */
		(void) sprintf(line, "k%s\n", RP);
		n = strlen(line);
		if (write(pfd, line, n) != n) {
		    fprintf(stderr, 
			    "Error sending kerberos opcode.\n");
		    cleanup();
		}
		if ((resp = response()) != '\0') {
		    fprintf(stderr,
			    "Remote printer does not support kerberos authentication\n");
		    if(kerberos_override == 1) 
			fprintf(stderr, "Try again without the -k flag\n");
		    if(kerberos_override == -1) 
			fprintf(stderr, "Try again using the -u option\n");
		    cleanup();
		}
			

		kerror = krb_sendauth(0L, pfd, &kticket, KLPR_SERVICE,
				      RM, (char *)krb_realmofhost(RM),
				      0, (MSG_DAT *) 0, 
				      (CREDENTIALS *) 0,
				      (bit_64 *) 0, 
				      (struct sockaddr_in *)0,
				      (struct sockaddr_in *)0,
				      "KLPRV0.1");
		if (kerror != KSUCCESS) {
		    fprintf(stderr, "Kerberos authentication failed. Use kinit and try again.\n");
		    cleanup();
		}
		if ((resp = response()) != '\0') {
		    if (resp == '\3') 
			fprintf(stderr, "Authentication failed. Use kinit and then try again.\n");
		    else fprintf(stderr, "Syncronization error.\n");
		    cleanup();
		}
	    }
#endif KERBEROS
	    (void) sprintf(line, "\2%s\n", RP);
	    n = strlen(line);

	    if (write(pfd, line, n) == n &&
		(resp = response()) == '\0')
		break;
	    (void) close(pfd);
	}

#ifdef KERBEROS
	if (resp == '\2') {
	    /* Should provide better error XXX */
	    fprintf(stderr, "Printer requires kerberos authentication\n");
	    cleanup();
	}

#endif						/* KERBEROS */
		    
	if (resp > 0) {
	    fprintf(stderr,	"Printer queue is disabled.\n");
	    cleanup();
	}
	if (i>6) {
	    fprintf(stderr, "Unable to contact printer server.\n");
	    cleanup();
	}
	sleep(5);
    }
    remote = 1;
    ofd = pfd;
    ofilter = 0;
}
