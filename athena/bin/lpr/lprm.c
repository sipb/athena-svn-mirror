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
static char sccsid[] = "@(#)lprm.c	5.2 (Berkeley) 11/17/85";
#endif not lint

#ifdef OPERATOR
#define GETUID geteuid
#else 
#define GETUID getuid
#endif

/*
 * lprm - remove the current user's spool entry
 *
 * lprm [-] [[job #] [user] ...]
 *
 * Using information in the lock file, lprm will kill the
 * currently active daemon (if necessary), remove the associated files,
 * and startup a new daemon.  Priviledged users may remove anyone's spool
 * entries, otherwise one can only remove their own.
 * A special version of lprm can be compiled with OPERATOR #defined that will
 * allow anyone to access it as the superuser.  It also logs all such actions.
 * o_lprm should be installed suid root, group execute operator only.
 */

#include "lp.h"

#if defined(KERBEROS)
int use_kerberos;
int kerberos_override = -1;
#endif /* KERBEROS */

/*
 * Stuff for handling job specifications
 */
char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */
char	*person;		/* name of person doing lprm */

static char	luser[16];	/* buffer for person */

struct passwd *getpwuid();

main(argc, argv)
	char *argv[];
{
	register char *arg;
	struct passwd *p;
	struct hostent *hp;

#ifdef OPERATOR
#ifdef LOG_AUTH
	openlog("lprm", 0, LOG_AUTH);
#else
	openlog("lprm", 0);
#endif
#endif

	name = argv[0];
	gethostname(host, sizeof(host));
	hp = gethostbyname(host);
	if (hp) strcpy(host, hp -> h_name);

#ifndef OPERATOR
#ifdef LOG_LPR
	openlog("lprm", 0, LOG_LPR);
#else
	openlog("lprm", 0);
#endif
#endif

	if ((p = getpwuid(GETUID())) == NULL)
		fatal("Who are you?");
	if (strlen(p->pw_name) >= sizeof(luser))
		fatal("Your name is too long");
	strcpy(luser, p->pw_name);
	person = luser;
	while (--argc) {
		if ((arg = *++argv)[0] == '-')
			switch (arg[1]) {
			case 'P':
				if (arg[2])
					printer = &arg[2];
				else if (argc > 1) {
					argc--;
					printer = *++argv;
				}
				break;
#if defined(KERBEROS)
			case 'u':
				kerberos_override = 0;
				break;
			case 'k':
				kerberos_override = 1;
				break;
#endif /* KERBEROS */
			case '\0':
				if (!users) {
					users = -1;
					break;
				}
			default:
				usage();
			}
		else {
			if (users < 0)
				usage();
			if (isdigit(arg[0])) {
				if (requests >= MAXREQUESTS)
					fatal("Too many requests");
				requ[requests++] = atoi(arg);
			} else {
				if (users >= MAXUSERS)
					fatal("Too many users");
				user[users++] = arg;
			}
		}
	}
	if (printer == NULL && (printer = getenv("PRINTER")) == NULL)
		printer = DEFLP;
#ifdef OPERATOR
	log(requ, requests, user, users, printer);
#endif
	rmjob();
}

static
usage()
{
	printf("usage: lprm [-] [-Pprinter] [[job #] [user] ...]\n");
	exit(2);
}
#ifdef OPERATOR
log(jobs, jobnums, users, usernums, printer)
int jobs[], jobnums;
char *users[];
int usernums;
char *printer;
{
	struct passwd 	*pwentry;
	char		*name;
	char 		logbuf[512];
	char		scratch[512];
	register int	i;
					/* note getuid() returns real uid */
	pwentry = getpwuid(getuid()); /* get password entry for invoker */
	name = pwentry->pw_name; /* get his name */

	sprintf(logbuf,"%s LPRM'd ",name);
	if (jobnums > 0) {
		strcat(logbuf,"jobs ");
		for (i=0; i<jobnums; i++) {
			sprintf(scratch, "%d ",jobs[i]);
			strcat(logbuf, scratch);
		}
	}
	if (usernums > 0) {
		if (jobnums > 0) { 
			strcat(logbuf,"and ");
		}
		strcat(logbuf, "users ");
		for (i=0; i<usernums; i++) {
			sprintf(scratch, "%s ",users[i]);
			strcat(logbuf, scratch);
		}
	}
	sprintf(scratch, "on printer %s.",printer);
	strcat(logbuf, scratch);

	syslog(LOG_INFO, logbuf);
	return(0);
}
#endif
