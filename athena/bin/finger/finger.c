/*
 * Copyright 1987 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file
 * "mit-copyright.h".
 *
 * $Id: finger.c,v 1.30 1997-01-24 08:05:56 jweiss Exp $
 */

#ifndef lint
static char *rcsid_finger_c = "$Id: finger.c,v 1.30 1997-01-24 08:05:56 jweiss Exp $";
#endif /*lint*/

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";

#endif /*not lint*/

#ifndef lint
static char sccsid[] = "@(#)finger.c	5.8 (Berkeley) 3/13/86";

#endif /*not lint */

/*
 * This is a finger program.  It prints out useful information about
 * users by digging it up from various system files.  It is not very
 * portable because the most useful parts of the information (the full
 * user name, office, and phone numbers) are all stored in the
 * VAX-unused gecos field of /etc/passwd, which, unfortunately, other
 * UNIXes use for other things.
 *
 * There are three output formats, all of which give login name,
 * teletype line number, and login time.  The short output format is
 * reminiscent of finger on ITS, and gives one line of information per
 * user containing in addition to the minimum basic requirements
 * (MBR), the full name of the user, his idle time and office location
 * and phone number.  The quick style output is UNIX who-like, giving
 * only name, teletype and login time.  Finally, the long style output
 * give the same information as the short (in more legible format),
 * the home directory and shell of the user, and, if it exists, copies
 * of the files .plan and .project in the user's home directory.
 * Finger may be called with or without a list of people to finger --
 * if no list is given, all the people currently logged in are
 * fingered.
 *
 * The program is validly called by one of the following:
 *
 *	finger			{short form list of users}
 *	finger -l		{long form list of users}
 *	finger -b		{briefer long form list of users}
 *	finger -q		{quick list of users}
 *	finger -i		{quick list of users with idle times}
 *	finger namelist		{long format list of specified users}
 *	finger -s namelist	{short format list of specified users}
 *	finger -w namelist	{narrow short format list of specified users}
 *
 * where 'namelist' is a list of users login names.
 * The other options can all be given after one '-', or each can have its
 * own '-'.  The -f option disables the printing of headers for short and
 * quick outputs.  The -b option briefens long format outputs.  The -p
 * option turns off plans for long format outputs.
 */

#ifdef POSIX
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#ifdef SYSV
#include <utmpx.h>
#endif
#include <string.h>
#include <utmp.h>
#include <sys/signal.h>
#include <pwd.h>
#include <stdio.h>
#if !defined(_AIX) || (AIXV < 20)
/* AIX 3.1 does not have <lastlog.h> */
#include <lastlog.h>
#else
#define NO_LASTLOG
#endif
#include <ctype.h>
#ifdef _AUX_SOURCE
#include <time.h>
#endif
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>
#include <zephyr/zephyr.h>
#include "mit-copyright.h"

#define MAXTTYS 256
#define MAXSEARCH 750

#define NOIDLE -1		/* Magic number for unavailable idle time */
#define CAPITALIZE	0137&	/* capitalize character macro */
#define ASTERISK	'*'	/* ignore this in real name */
#define COMMA		','	/* separator in pw_gecos field */
#define COMMAND		'-'	/* command line flag char */
#define TECHSQ		'T'	/* tech square office */
#define MIT		'M'	/* MIT office */
#define SAMENAME	'&'	/* repeat login name in real name */
#define TALKABLE	0220	/* tty is writable if 220 mode */
#define MAILDIR 	"/usr/spool/mail/"	/* default mail directory */

#ifdef SYSV
struct utmpx user;
#else
struct utmp user;
#endif

#define NMAX sizeof(user.ut_name)
#define LMAX sizeof(user.ut_line)
#define HMAX sizeof(user.ut_host)
#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) ((a) < (b) ? (a) : (b))

struct person {			/* one for each person fingered */
	char *name;		/* name */
	char tty[BUFSIZ];	/* null terminated tty line */
	char host[BUFSIZ];	/* null terminated remote host name */
	int loginout;		/* 0 means login time, 1 logout */
#ifdef POSIX
	time_t loginat;
	time_t idletime;
#else
	long loginat;		/* time of (last) login/out */
	long idletime;		/* how long idle (if logged in) */
#endif
	char *logintime;	/* pointer to string showing logintime */
	char *realname;		/* pointer to full name */
	char *nickname;		/* pointer to nickname */
	char *office;		/* pointer to office name */
	char *officephone;	/* pointer to office phone no. */
	char *homephone;	/* pointer to home phone no. */
	char *random;		/* for any random stuff in pw_gecos */
	struct passwd *pwd;	/* structure of /etc/passwd stuff */
	int loggedin;		/* person is logged in */
	char writable;		/* tty is writable */
	int original;		/* this is not a duplicate entry */
	struct person *link;	/* link to next person */
	int zlocation;		/* found via Zephyr */
};

char LASTLOG[] = "/usr/adm/lastlog";	/* last login info */
#ifdef SYSV
char USERLOG[] = "/etc/utmpx";	/* who is logged in */
char ACCTLOG[] = "/usr/adm/wtmpx";	/* Accounting file */
#else
char USERLOG[] = "/etc/utmp";	/* who is logged in */
char ACCTLOG[] = "/usr/adm/wtmp";	/* Accounting file */
#endif

char PLAN[] = "/.plan";		/* what plan file is */
char PROJ[] = "/.project";	/* what project file */
char MM[] = "%MENTION-MAIL%\n";	/* if present, check for mail */
int MMLEN = 15;

int unbrief = 1;		/* -b option default */
int header = 1;			/* -f option default */
int hack = 1;			/* -h option default */
int idle = 0;			/* -i option default */
int large = 0;			/* -l option default */
int match = 1;			/* -m option default */
int plan = 1;			/* -p option default */
int unquick = 1;		/* -q option default */
int small = 0;			/* -s option default */
int wide = 1;			/* -w option default */

int numloc = 1;			/* number of locations from zgetlocations */
ZLocations_t location;		/* holds Zephyr locations */
int znloc = NULL;		/* number of locations returned via a Zephyr
				 * lookup */
int unshort;
int lf = -1;			/* LASTLOG file descriptor */
int lw = -1;			/* ACCTLOG file descriptor */

/* !#$%!@#$! Bezerkeley non-initializations !!! */
struct person *person1 = (struct person *) NULL;	/* list of people */
struct person *person2 = (struct person *) NULL;	/* 2nd list of people */
#ifdef POSIX
time_t tloc;
#else
long tloc;
#endif

char ttnames[MAXTTYS][LMAX];	/* TTY names */
long logouts[MAXTTYS];		/* Logout times */

struct passwd *pwdcopy();

#ifdef NOTDEF
/* I think these are now all declared by headers. Remove this in a couple
   of months. This works on AIX and Ultrix... It's up to the Solaris
   build now. */
char *strcpy();
char *strcat();
char *strncpy();
char *malloc();
char *ctime();
#endif

#ifndef POSIX
long time();
#endif

/*ARGSUSED*/
main(argc, argv)
	int argc;
	register char **argv;
{
	register char *s;

	/* parse command line for (optional) arguments */
	while (*++argv && **argv == COMMAND)
		for (s = *argv + 1; *s; s++)
			switch (*s) {
			case 'b':
				unbrief = 0;
				break;
			case 'f':
				header = 0;
				break;
			case 'h':
				hack = 0;
				break;
			case 'i':
				idle = 1;
				unquick = 0;
				break;
			case 'l':
				large = 1;
				break;
			case 'm':
				match = 0;
				break;
			case 'p':
				plan = 0;
				break;
			case 'q':
				unquick = 0;
				break;
			case 's':
				small = 1;
				break;
			case 'w':
				wide = 0;
				break;
			default:
				fprintf(stderr, "Usage: finger [-bfhilmpqsw] [login1 [login2 ...] ]\n");
				exit(1);
			}
	if (unquick || idle)
		(void) time(&tloc);
	/*
	 * *argv == 0 means no names given 
	 *
	 * this procedure should not (and doesn't) use zephyr at all, because
	 * finger [no args] should NOT give you every logged-in person... 
	 */
	if (*argv == 0)
		doall();
	else
		donames(argv);
	if (person1) {
		printf("\nLocal:\n");
		print(person1);
	}
	if (person2) {
		printf("\nAthena-wide:\n");
		print(person2);
	}
	exit(0);
}

doall()
{
	register struct person *p;
	register struct passwd *pw;
	int uf;
	char name[NMAX + 1];

	unshort = large;
	if ((uf = open(USERLOG, 0)) < 0) {
		fprintf(stderr, "finger: error opening %s\n", USERLOG);
		exit(2);
	}
	if (unquick) {
		setpwent();
		fwopen();
	}
	while (read(uf, (char *) &user, sizeof user) == sizeof user) {
		if (user.ut_name[0] == 0)
			continue;
#if defined(USER_PROCESS)
		if (user.ut_type != USER_PROCESS)
			continue;
#endif
		if (person1 == 0)
			p = person1 = (struct person *) malloc(sizeof *p);
		else {
			p->link = (struct person *) malloc(sizeof *p);
			p = p->link;
		}
		memcpy(name, user.ut_name, NMAX);
		name[NMAX] = 0;
		memcpy(p->tty, user.ut_line, LMAX);
		p->tty[LMAX] = 0;
		memcpy(p->host, user.ut_host, HMAX);
		p->host[HMAX] = 0;
		p->loginout = 0;
#ifndef SYSV
		p->loginat = user.ut_time;
#else
		p->loginat = user.ut_tv.tv_sec;
#endif
		p->pwd = 0;
		p->loggedin = 1;
		p->zlocation = 0;
		if (unquick && (pw = getpwnam(name))) {
			p->pwd = pwdcopy(pw);
			decode(p);
			p->name = p->pwd->pw_name;
		}
		else
			p->name = strcpy(malloc((unsigned) (strlen(name) + 1)),
					 name);
	}
	if (unquick) {
		fwclose();
		endpwent();
	}
	(void) close(uf);
	if (person1 == 0) {
		printf("\nNo one logged on\n");
		return;
	}
	p->link = 0;
}

donames(argv)
	char **argv;
{
	register struct person *p;
	register struct person *q;	/* A second chain for athena-wide
					 * finger */
	register struct passwd *pw;
	struct passwd *hes_getpwnam();
	int uf, i;
	Code_t state;

	/*
	 * get names from command line and check to see if they're logged in 
	 */
	unshort = !small;
	for (; *argv != 0; argv++) {
		if (netfinger(*argv))
			continue;
		if (person1 == 0) {
			p = person1 = (struct person *) malloc(sizeof *p);
			q = person2 = (struct person *) malloc(sizeof *q);
		}
		else {
			p->link = (struct person *) malloc(sizeof *p);
			p = p->link;
			q->link = (struct person *) malloc(sizeof *q);
			q = q->link;
		}
		p->name = q->name = *argv;
		p->tty[0] = q->tty[0] = '\0';
		p->host[0] = q->host[0] = '\0';
		p->loginout = q->loginout = 0;
		p->loginat = q->loginat = 0;
		p->logintime = q->logintime = (char *) NULL;
		p->idletime = q->idletime = 0;
		p->realname = q->realname = (char *) NULL;
		p->nickname = q->nickname = (char *) NULL;
		p->office = q->office = (char *) NULL;
		p->officephone = q->officephone = (char *) NULL;
		p->homephone = q->homephone = (char *) NULL;
		p->random = q->random = (char *) NULL;
		p->pwd = q->pwd = (struct passwd *) NULL;
		p->loggedin = q->loggedin = 0;
		p->writable = q->writable = '\0';
		p->original = q->original = 1;
		p->zlocation = q->zlocation = 0;
	}
	if (person1 == 0)
		return;
	p->link = q->link = 0;
	/*
	 * if we are doing it, read /etc/passwd for the useful info for p* --
	 * ask hesiod for the entries for q*. 
	 */
	if (unquick) {
		setpwent();
		if (!match) {
#if !defined(ultrix) && !defined(_AIX) && !defined(SOLARIS) || defined(_AUX_SOURCE) 
			extern _pw_stayopen;

			_pw_stayopen = 1;
#endif
			for (p = person1; p != 0; p = p->link)
				if (pw = getpwnam(p->name))
					p->pwd = pwdcopy(pw);

		}
		else
			while ((pw = getpwent()) != 0) {
				for (p = person1; p != 0; p = p->link) {
					if (!p->original)
						continue;
					if (strcmp(p->name, pw->pw_name) != 0 &&
					    !matchcmp(pw->pw_gecos, pw->pw_name, p->name))
						continue;
					if (p->pwd == 0) {
						p->pwd = pwdcopy(pw);
					}
					else {
						struct person *new;

						/*
						 * handle multiple login
						 * names, insert new
						 * "duplicate" entry behind 
						 */
						new = (struct person *)
							malloc(sizeof *new);
						new->pwd = pwdcopy(pw);
						new->name = p->name;
						new->original = 1;
						new->zlocation = 0;
						new->loggedin = 0;
						new->link = p->link;
						p->original = 0;
						p->link = new;
						p = new;
					}
				}
			}
		/* now do the hesiod chain */
		for (q = person2; q != 0; q = q->link)
			if (pw = hes_getpwnam(q->name))
				q->pwd = pwdcopy(pw);
		endpwent();
	}
	/* Now get login information */
	if ((uf = open(USERLOG, 0)) < 0) {
		fprintf(stderr, "finger: error opening %s\n", USERLOG);
		exit(2);
	}
	while (read(uf, (char *) &user, sizeof user) == sizeof user) {
		if (*user.ut_name == 0)
			continue;
#if defined(USER_PROCESS)
		if (user.ut_type != USER_PROCESS)
			continue;
#endif
		for (p = person1; p != 0; p = p->link) {
			if (p->loggedin == 2)
				continue;
			if (strncmp(p->pwd ? p->pwd->pw_name : p->name,
				    user.ut_name, NMAX) != 0)
				continue;
			if (p->loggedin == 0) {
				memcpy(p->tty, user.ut_line, LMAX);
				p->tty[LMAX] = 0;
				memcpy(p->host, user.ut_host, HMAX);
				p->host[HMAX] = 0;
#ifndef SYSV
				p->loginat = user.ut_time;
#else
				p->loginat = user.ut_tv.tv_sec;
#endif
				p->loggedin = 1;
			}
			else {	/* p->loggedin == 1 */
				struct person *new;

				new = (struct person *) malloc(sizeof *new);
				new->name = p->name;
				memcpy(new->tty, user.ut_line, LMAX);
				new->tty[LMAX] = 0;
				memcpy(new->host, user.ut_host, HMAX);
				new->host[HMAX] = 0;
#ifndef SYSV
				new->loginat = user.ut_time;
#else
				p->loginat = user.ut_tv.tv_sec;
#endif
				new->pwd = p->pwd;
				new->loggedin = 1;
				new->original = 0;
				new->zlocation = 0;
				new->link = p->link;
				p->loggedin = 2;
				p->link = new;
				p = new;
			}
		}
	}
	/* Ask Zephyr if someone is logged in. */
	if ((state = ZInitialize()) != ZERR_NONE) {
		com_err("finger", state, "\nFailure in Zephyr \
initialization");
	}
	else {
		for (q = person2; q != (struct person *) NULL; q = q->link) {
			char curname[BUFSIZ];

			(void) strcpy(curname, q->name);
			(void) strcat(curname, "@");
			(void) strcat(curname, ZGetRealm());

			if ((state = ZLocateUser(curname, &znloc,ZAUTH)) != ZERR_NONE) {
				com_err("finger", state, "\nFailure in \
ZLocateUser");
				break;
			}
			q->zlocation = 1;
			q->loggedin = 0;
			for (i = 1; i <= znloc; i++) {
				if ((state = ZGetLocations(&location, &numloc))
				    != 0)
					break;
				else {
					(void) strcpy(q->host, location.host);
					q->logintime = location.time;
					(void) strcpy(q->tty,
						      location.tty);
					q->loggedin = 1;
					/* if we can zlocate them, we can
					 * zwrite them -- if they're
					 * subscribing. */
					q->writable = 1;

				}
			}
		}
	}
	(void) close(uf);
	if (unquick) {
		fwopen();
		for (p = person1, q = person2; p != 0 && q != 0;
		     p = p->link, q = q->link) {
			decode(p);
			decode(q);
		}
		fwclose();
	}
}

print(personn)
	register struct person *personn;
{
	register struct person *p;
	register FILE *fp;
	register char *s;
	register c;

	/*
	 * print out what we got 
	 */
	if (header) {
		if (unquick) {
			if (!unshort)
				if (wide)
#if defined(SOLARIS) || defined(sgi)
					printf("Login       Name                TTY  Idle    When     Office\n");
#else
					printf("Login       Name              TTY Idle    When            Office\n");
#endif
				else
#if defined(SOLARIS) || defined(sgi)
					printf("Login      TTY Idle    When     Office\n");
#else
					printf("Login    TTY Idle    When            Office\n");
#endif
		}
		else {
			printf("Login      TTY            When");
			if (idle)
				printf("             Idle");
			(void) putchar('\n');
		}
	}
	for (p = personn; p != 0; p = p->link) {
		if (!unquick) {
			quickprint(p);
			continue;
		}
		else
			decode(p);
		if (!unshort) {
			shortprint(p);
			continue;
		}
		personprint(p);
		if (p->pwd != 0) {
			if (hack) {
				s = malloc(strlen(p->pwd->pw_dir) +
					   (unsigned) sizeof PROJ);
				(void) strcpy(s, p->pwd->pw_dir);
				(void) strcat(s, PROJ);
				if ((fp = fopen(s, "r")) != 0) {
					printf("Project: ");
					while ((c = getc(fp)) != EOF) {
						if (c == '\n')
							break;
						if (isprint(c) || isspace(c))
							(void) putchar(c);
						else
							(void) putchar(c ^ 100);
					}
					(void) fclose(fp);
					(void) putchar('\n');
				}
				free(s);
			}
			if (plan) {
				register int i = 0, j;
				register int okay = 1;
				char mailbox[100];	/* mailbox name */

				s = malloc(strlen(p->pwd->pw_dir) +
					   (unsigned) sizeof PLAN);
				(void) strcpy(s, p->pwd->pw_dir);
				(void) strcat(s, PLAN);
				if ((fp = fopen(s, "r")) == 0)
					printf("No Plan.\n");
				else {
					printf("Plan:\n");
					while ((c = getc(fp)) != EOF) {
						if (i < MMLEN && okay) {
							if (c != MM[i]) {
								for (j = 0; j < i; j++)
									(void) putchar(MM[j]);
								if (isprint(c) || isspace(c))
									(void) putchar(c);
								else
									(void) putchar(c ^ 100);
								okay = 0;
							}
						}
						else if (isprint(c) || isspace(c))
							(void) putchar(c);
						else
							(void) putchar(c ^ 100);
						i++;
					}
					(void) fclose(fp);
					if (okay) {
						(void) strcpy(mailbox, MAILDIR);
						/* start with the directory */
						(void) strcat(mailbox,
							 (p->pwd)->pw_name);
						if (access(mailbox, F_OK) == 0) {
							struct stat statb;

							(void) stat(mailbox, &statb);
							if (statb.st_size) {
								printf("User %s has new mail.\n", (p->pwd)->pw_name);
							};
						}
					}
				}
				free(s);
			}
		}
		if (p->link != 0)
			(void) putchar('\n');
	}
}

/*
 * Duplicate a pwd entry.
 * Note: Only the useful things (what the program currently uses) are copied.
 */
struct passwd *
pwdcopy(pfrom)
	register struct passwd *pfrom;
{
	register struct passwd *pto;

	pto = (struct passwd *) malloc((unsigned) (sizeof *pto));
#define savestr(s) strcpy(malloc((unsigned)(strlen(s) + 1)), s)
	pto->pw_name = savestr(pfrom->pw_name);
	pto->pw_uid = pfrom->pw_uid;
	pto->pw_gecos = savestr(pfrom->pw_gecos);
	pto->pw_dir = savestr(pfrom->pw_dir);
	pto->pw_shell = savestr(pfrom->pw_shell);
#undef savestr
	return pto;
}

/*
 * print out information on quick format giving just name, tty, login time
 * and idle time if idle is set.
 */
quickprint(pers)
	register struct person *pers;
{
	printf("%-*.*s  ", NMAX, NMAX, pers->name);
	if (pers->loggedin) {
		if (idle) {
			findidle(pers);
#ifndef SYSV
			printf("%c%-*s %-16.16s", pers->writable ? ' ' : '*',
			       LMAX, pers->tty, ctime(&pers->loginat));
#else
			printf("%c%-*s", pers->writable ? ' ' : '*',
			       LMAX, pers->tty);
#endif
			(void) ltimeprint("   ", &pers->idletime, "");
		}
		else
			printf(" %-*s %-16.16s", LMAX,
			       pers->tty, ctime(&pers->loginat));
		(void) putchar('\n');
	}
	else
		printf("          Not Logged In\n");
}

/*
 * print out information in short format, giving login name, full name,
 * tty, idle time, login time, office location and phone.
 */
shortprint(pers)
	register struct person *pers;
{
	char *p;
	char dialup;

	if (pers->pwd == 0) {
#if defined(SOLARIS) || defined(sgi)
		printf("%-8s       ???\n", pers->name);
#else
		printf("%-15s       ???\n", pers->name);
#endif
		return;
	}
#if defined(SOLARIS) || defined(sgi)
	printf("%-8s",  pers->pwd->pw_name);
#else
	printf("%-*s", NMAX, pers->pwd->pw_name);
#endif
	dialup = 0;
	if (wide) {
		if (pers->realname)
			printf(" %-20.20s", pers->realname); 

		else
			printf("        ???          ");
	}
	(void) putchar(' ');  
	if (pers->loggedin && !pers->writable)
		(void) putchar('*');
	else
		(void) putchar(' '); 
	if (*pers->tty) {
		if (!strncmp(pers->tty, "tty", 3)) {
			if (pers->tty[3] == 'd' && pers->loggedin)
				dialup = 1;
#ifdef SOLARIS
			printf("%-5.5s ", pers->tty );
#else
			printf("%-2.2s ", pers->tty + 3);
#endif
		} else
		if (!strncmp(pers->tty, "pts/", 4)) {
#ifdef SOLARIS
			printf("%-5.5s ", pers->tty );
#else
			printf("p%-1.1s ", pers->tty + 4);
#endif
		} else
#ifdef SOLARIS
			printf("%-5.5s ", pers->tty);
#else
			printf("%-2.2s ", pers->tty);
#endif
	}
	else
		printf("   ");
	p = ctime(&pers->loginat);
	if (pers->loggedin) {
		stimeprint(&pers->idletime);
		printf(" %3.3s %-5.5s ", p, p + 11);
	}
	else if (pers->loginat == 0)
		printf(" < .  .  .  . >");
	else if (tloc - pers->loginat >= 180 * 24 * 60 * 60)
		printf(" <%-6.6s, %-4.4s>", p + 4, p + 20);
	else
		printf(" <%-12.12s>", p + 4);
	if (dialup && pers->homephone)
		printf(" %20s", pers->homephone);
	else {
#ifdef SOLARIS
		(void) putchar(' '); 
#endif
		if (pers->office)
			printf(" %-12.12s", pers->office);
		else if (pers->officephone || pers->homephone)
			printf("            ");
		if (pers->officephone)
			printf(" %s", pers->officephone);
		else if (pers->homephone)
			printf(" %s", pers->homephone);
	}
	(void) putchar('\n');
}

/*
 * print out a person in long format giving all possible information.
 * directory and shell are inhibited if unbrief is clear.
 */
personprint(pers)
	register struct person *pers;
{
	if (pers->pwd == (struct passwd *) NULL) {
		printf("Login name: %-10s\t\t\tIn real life: ???\n",
		       pers->name);
		return;
	}
	printf("Login name: %-10s", pers->pwd->pw_name);
	if (pers->loggedin && !pers->writable)
		printf("	(messages off)	");
	else
		printf("			");
	if (pers->realname)
		printf("In real life: %s", pers->realname);
	if (pers->nickname)
		printf("\nNickname: %-s", pers->nickname);
	if (pers->office) {
		printf("\nOffice: %-.12s", pers->office);
		if (pers->officephone) {
			printf(", %s", pers->officephone);
			if (pers->homephone)
				printf("\t\tHome phone: %s", pers->homephone);
			else if (pers->random)
				printf("\t\t%s", pers->random);
		}
		else if (pers->homephone)
			printf("\t\t\tHome phone: %s", pers->homephone);
		else if (pers->random)
			printf("\t\t\t%s", pers->random);
	}
	else if (pers->officephone) {
		printf("\nPhone: %s", pers->officephone);
		if (pers->homephone)
			printf(", %s", pers->homephone);
		if (pers->random)
			printf(", %s", pers->random);
	}
	else if (pers->homephone) {
		printf("\nPhone: %s", pers->homephone);
		if (pers->random)
			printf(", %s", pers->random);
	}
	else if (pers->random)
		printf("\n%s", pers->random);
	if (unbrief) {
		printf("\nDirectory: %-25s", pers->pwd->pw_dir);
		if (*pers->pwd->pw_shell)
			printf("\tShell: %-s", pers->pwd->pw_shell);
	}
	if (pers->zlocation) {
		if (pers->loggedin)
			printf("\nOn since %s on %s on host %s",
			       pers->logintime, pers->tty, pers->host);
		else
			printf("\nNot currently locatable.");
	}
	else if (pers->loggedin) {
		if (*pers->host) {
			if (pers->logintime) {
				printf("\nOn since %s on %s from %s",
				    pers->logintime, pers->tty, pers->host);
			}
			else {
				register char *ep = ctime(&pers->loginat);

				printf("\nOn since %15.15s on %s from %s",
				       &ep[4], pers->tty, pers->host);
			(void) ltimeprint("\n", &pers->idletime, " Idle Time");
			}
		}
		else {
			register char *ep = ctime(&pers->loginat);

			printf("\nOn since %15.15s on %-*s",
			       &ep[4], LMAX, pers->tty);
			(void) ltimeprint("\t", &pers->idletime, " Idle Time");
		}
	}
	else if (pers->loginat == 0)
		printf("\nNever logged in.");
	else if (tloc - pers->loginat > 180 * 24 * 60 * 60) {
		register char *ep = ctime(&pers->loginat);

		printf("\nLast %s %10.10s, %4.4s on %s",
		       pers->loginout ? "logout" : "login",
		       ep, ep + 20, pers->tty);
		if (*pers->host)
			printf(" from %s", pers->host);
	}
	else {
		register char *ep = ctime(&pers->loginat);

		printf("\nLast %s %16.16s on %s",
		       pers->loginout ? "logout" : "login",
		       ep, pers->tty);
		if (*pers->host)
			printf(" from %s", pers->host);
	}
	(void) putchar('\n');
}

/*
 *  very hacky section of code to format phone numbers.  filled with
 *  magic constants like 4, 7 and 10.
 */
char *
phone(s, len, alldigits)
	register char *s;
	int len;
	char alldigits;
{
	char fonebuf[15];
	register char *p = fonebuf;
	register i;

	if (!alldigits)
		return (strcpy(malloc((unsigned) (len + 1)), s));
	switch (len) {
	case 4:
		*p++ = ' ';
		*p++ = 'x';
		*p++ = '3';
		*p++ = '-';
		for (i = 0; i < 4; i++)
			*p++ = *s++;
		break;
	case 5:
		*p++ = ' ';
		*p++ = 'x';
		*p++ = *s++;
		*p++ = '-';
		for (i = 0; i < 4; i++)
			*p++ = *s++;
		break;
	case 7:
		for (i = 0; i < 3; i++)
			*p++ = *s++;
		*p++ = '-';
		for (i = 0; i < 4; i++)
			*p++ = *s++;
		break;
	case 10:
		for (i = 0; i < 3; i++)
			*p++ = *s++;
		*p++ = '-';
		for (i = 0; i < 3; i++)
			*p++ = *s++;
		*p++ = '-';
		for (i = 0; i < 4; i++)
			*p++ = *s++;
		break;
	case 0:
		return 0;
	default:
		return (strcpy(malloc((unsigned) (len + 1)), s));
	}
	*p++ = 0;
	return (strcpy(malloc((unsigned) (p - fonebuf)), fonebuf));
}

/*
 * decode the information in the gecos field of /etc/passwd
 */
decode(pers)
	register struct person *pers;
{
	char buffer[256];
	register char *bp, *gp, *lp;
	int alldigits;
	int hasspace;
	int len;

	pers->realname = 0;
	pers->nickname = 0;
	pers->office = 0;
	pers->officephone = 0;
	pers->homephone = 0;
	pers->random = 0;
	if (pers->pwd == 0)
		return;
	gp = pers->pwd->pw_gecos;
	bp = buffer;
	if (*gp == ASTERISK)
		gp++;
	while (*gp && *gp != COMMA)	/* name */
		if (*gp == SAMENAME) {
			lp = pers->pwd->pw_name;
			if (islower(*lp))
				*bp++ = toupper(*lp++);
			while (*bp++ = *lp++);
			bp--;
			gp++;
		}
		else
			*bp++ = *gp++;
	*bp++ = 0;
	if ((len = bp - buffer) > 1)
		pers->realname = strcpy(malloc((unsigned) len), buffer);
	if (*gp == COMMA) {	/* nickname */
		gp++;
		bp = buffer;
		while ((*gp != NULL) && (*gp != COMMA)) {
			if (*gp == SAMENAME) {
				lp = pers->pwd->pw_name;
				*bp++ = CAPITALIZE(*lp++);
				while (*lp != NULL) {
					*bp++ = *lp++;
				}
			}
			else {
				*bp++ = *gp;
			}
			gp++;
		}
		*bp = NULL;
		if (strlen(buffer) > 0) {
			pers->nickname = malloc((unsigned) (strlen(&buffer[0])
							    + 1));
			(void) strcpy(pers->nickname, &buffer[0]);
		}
	}
	if (*gp == COMMA) {	/* office, supposedly */
		gp++;
		hasspace = 0;
		bp = buffer;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (*bp == ' ')
				hasspace = 1;
			/* leave 9 for Tech Sq. and MIT expansion */
			if (bp < buffer + sizeof buffer - 9)
				bp++;
		}
		*bp = 0;
		len = bp - buffer;
		bp--;		/* point to last character */
		if (hasspace || len == 0)
			len++;
		else if (*bp == TECHSQ) {
			(void) strcpy(bp, " Tech Sq.");
			len += 9;
		}
		else if (*bp == MIT) {
			(void) strcpy(bp, " MIT");
			len += 4;
		}
		else
			len++;
		if (len > 1)
			pers->office = strcpy(malloc((unsigned) len), buffer);
	}
	if (*gp == COMMA) {	/* office phone */
		gp++;
		bp = buffer;
		alldigits = 1;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (!isdigit(*bp))
				alldigits = 0;
			if (bp < buffer + sizeof buffer - 1)
				bp++;
		}
		*bp = 0;
		pers->officephone = phone(buffer, bp - buffer, alldigits);
	}
	if (*gp == COMMA) {	/* home phone */
		gp++;
		bp = buffer;
		alldigits = 1;
		while (*gp && *gp != COMMA) {
			*bp = *gp++;
			if (!isdigit(*bp))
				alldigits = 0;
			if (bp < buffer + sizeof buffer - 1)
				bp++;
		}
		*bp = 0;
		pers->homephone = phone(buffer, bp - buffer, alldigits);
	}
	if (pers->loggedin)
		findidle(pers);
	else if (!pers->zlocation)
		findwhen(pers);
}

/*
 * find the last log in of a user by checking the LASTLOG file.
 * the entry is indexed by the uid, so this can only be done if
 * the uid is known (which it isn't in quick mode)
 */

fwopen()
{
#ifndef NO_LASTLOG
	if ((lf = open(LASTLOG, 0)) < 0) {
#ifndef _AIX
		fprintf(stderr, "finger: %s open error\n", LASTLOG);
#endif
	}
#endif
	if ((lw = open(ACCTLOG, 0)) < 0)
		fprintf(stderr, "finger: %s open error\n", ACCTLOG);
}

findwhen(pers)
	register struct person *pers;
{
#ifndef NO_LASTLOG
	struct lastlog ll;
#endif
	struct stat stb;
#ifndef SYSV
	struct utmp *bp;
	struct utmp buf[128];
#else
	struct utmpx *bp;
	struct utmpx buf[128];
#endif
	int i, bl, count;
	off_t lseek();

	if (lw >= 0) {
		(void) fstat(lw, &stb);
		bl = (stb.st_size + sizeof(buf) - 1) / sizeof(buf);
		count = 0;
		for (bl--; bl >= 0; bl--) {
			(void) lseek(lw, (off_t) (bl * sizeof(buf)), 0);
			bp = &buf[read(lw, (char *) buf, sizeof(buf))
				  / sizeof(buf[0]) - 1];
			for (; bp >= buf; bp--) {
				if (count++ == MAXSEARCH)
					goto fudged;
				if (!strncmp(bp->ut_name, pers->name,
					     MIN(strlen(pers->name)+1, NMAX))) {
					(void) strncpy(pers->tty,
						       bp->ut_line, LMAX);
					(void) strncpy(pers->host,
						       bp->ut_host, HMAX);
					pers->loginout = 1;
					for (i = 0; i < MAXTTYS; i++) {
						if (!strncmp(ttnames[i],
							     bp->ut_line,
						     sizeof(bp->ut_line))) {
							pers->loginat = logouts[i];
							return;
						}
					}
					goto fudged;
				}
				else {
					for (i = 0; i < MAXTTYS; i++) {
						if (ttnames[i][0] == 0) {
							(void) strncpy(ttnames[i],
								bp->ut_line,
							sizeof(bp->ut_line));
#ifndef SYSV
							logouts[i] = bp->ut_time;
#else
							logouts[i] = bp->ut_tv.tv_sec;
#endif
							break;
						}
						if (!strncmp(ttnames[i],
							     bp->ut_line,
						     sizeof(bp->ut_line))) {
#ifndef SYSV
							logouts[i] = bp->ut_time;
#else
							logouts[i] = bp->ut_tv.tv_sec;
#endif
							break;
						}
					}
				}
			}
		}
	}
fudged:
#ifndef NO_LASTLOG
	if (lf >= 0) {
		(void) lseek(lf, (long) pers->pwd->pw_uid * sizeof ll, 0);
		if ((i = read(lf, (char *) &ll, sizeof ll)) == sizeof ll) {
			memcpy(pers->tty, ll.ll_line, LMAX);
			pers->tty[LMAX] = 0;
			memcpy(pers->host, ll.ll_host, HMAX);
			pers->host[HMAX] = 0;
			pers->loginat = ll.ll_time;
			pers->loginout = 0;
		}
		else {
			if (i != 0)
				fprintf(stderr, "finger: %s read error\n",
					LASTLOG);
			pers->tty[0] = 0;
			pers->host[0] = 0;
			pers->loginat = 0L;
		}
	} else
#endif
	{
		pers->tty[0] = 0;
		pers->host[0] = 0;
		pers->loginat = 0L;
	}
}

fwclose()
{
#ifndef NO_LASTLOG
	if (lf >= 0) {
		(void) close(lf);
		lf = -1;
	}
#endif
	if (lw >= 0) {
		(void) close(lw);
		lw = -1;
	}
}

/*
 * find the idle time of a user by doing a stat on /dev/tty??,
 * where tty?? has been gotten from USERLOG, supposedly.
 */
findidle(pers)
	register struct person *pers;
{
	struct stat ttystatus;
	static char buffer[20] = "/dev/";
#ifdef POSIX
	time_t t;
#else
	long t;
#endif

#define TTYLEN 5

	if (!pers->zlocation) {
		(void) strcpy(buffer + TTYLEN, pers->tty);
		buffer[TTYLEN + LMAX] = 0;
		if (stat(buffer, &ttystatus) < 0) {
			pers->idletime = NOIDLE;
			pers->writable = 0;
			return;
/*
			fprintf(stderr, "finger: Can't stat %s\n", buffer);
			exit(4);
*/
		}
		(void) time(&t);
		if (t < ttystatus.st_atime)
			pers->idletime = 0L;
		else
			pers->idletime = t - ttystatus.st_atime;
		pers->writable = (ttystatus.st_mode & TALKABLE) == TALKABLE;
	} else {
		pers->idletime = 0L;
		pers->writable = 1;
	}
}

/*
 * print idle time in short format; this program always prints 4 characters;
 * if the idle time is zero, it prints 4 blanks.
 */
stimeprint(dt)
#ifdef POSIX
	time_t *dt;
#else
	long *dt;
#endif
{
	register struct tm *delta;

	if (*dt == NOIDLE) {
		printf("   -");
		return;
	}

	delta = gmtime(dt);
	if (delta->tm_yday == 0)
		if (delta->tm_hour == 0)
			if (delta->tm_min == 0)
				printf("    ");
			else
				printf("  %2d", delta->tm_min);
		else if (delta->tm_hour >= 10)
			printf("%3d:", delta->tm_hour);
		else
			printf("%1d:%02d",
			       delta->tm_hour, delta->tm_min);
	else
		printf("%3dd", delta->tm_yday);
}

/*
 * print idle time in long format with care being taken not to pluralize
 * 1 minutes or 1 hours or 1 days.
 * print "prefix" first.
 */
ltimeprint(before, dt, after)
	long *dt;
	char *before, *after;
{
	register struct tm *delta;

	if (*dt == NOIDLE) {
		printf("%sUnavailable%s", before, after);
		return (0);
	}

	delta = gmtime(dt);
	if (delta->tm_yday == 0 && delta->tm_hour == 0 && delta->tm_min == 0 &&
	    delta->tm_sec <= 10)
		return (0);
	printf("%s", before);
	if (delta->tm_yday >= 10)
		printf("%d days", delta->tm_yday);
	else if (delta->tm_yday > 0)
		printf("%d day%s %d hour%s",
		       delta->tm_yday, delta->tm_yday == 1 ? "" : "s",
		       delta->tm_hour, delta->tm_hour == 1 ? "" : "s");
	else if (delta->tm_hour >= 10)
		printf("%d hours", delta->tm_hour);
	else if (delta->tm_hour > 0)
		printf("%d hour%s %d minute%s",
		       delta->tm_hour, delta->tm_hour == 1 ? "" : "s",
		       delta->tm_min, delta->tm_min == 1 ? "" : "s");
	else if (delta->tm_min >= 10)
		printf("%2d minutes", delta->tm_min);
	else if (delta->tm_min == 0)
		printf("%2d seconds", delta->tm_sec);
	else
		printf("%d minute%s %d second%s",
		       delta->tm_min,
		       delta->tm_min == 1 ? "" : "s",
		       delta->tm_sec,
		       delta->tm_sec == 1 ? "" : "s");
	printf("%s", after);
	return (0);
}

matchcmp(gname, login, given)
	register char *gname;
	char *login;
	char *given;
{
	char buffer[100];
	register char *bp, *lp;
	register c;

	if (*gname == ASTERISK)
		gname++;
	lp = 0;
	bp = buffer;
	for (;;)
		switch (c = *gname++) {
		case SAMENAME:
			for (lp = login; bp < buffer + sizeof buffer
			     && (*bp++ = *lp++););
			bp--;
			break;
		case ' ':
		case COMMA:
		case '\0':
			*bp = 0;
			if (namecmp(buffer, given))
				return (1);
			if (c == COMMA || c == 0)
				return (0);
			bp = buffer;
			break;
		default:
			if (bp < buffer + sizeof buffer)
				*bp++ = c;
		}
	/* NOTREACHED */
}

namecmp(name1, name2)
	register char *name1, *name2;
{
	register int c1, c2;

	/* Do not permit matching on empty strings */
	if (*name1 == 0 || *name2 == 0)
		return (0);
	
	for (;;) {
		c1 = *name1++;
		if (islower(c1))
			c1 = toupper(c1);
		c2 = *name2++;
		if (islower(c2))
			c2 = toupper(c2);
		if (c1 != c2)
			break;
		if (c1 == 0)
			return (1);
	}
	if (!c1) {
		for (name2--; isdigit(*name2); name2++);
		if (*name2 == 0)
			return (1);
	}
	if (!c2) {
		for (name1--; isdigit(*name1); name1++);
		if (*name1 == 0)
			return (1);
	}
	return (0);
}

netfinger(name)
	char *name;
{
	char *host;
	struct hostent *hp;
	struct servent *sp;
	struct sockaddr_in sin;
	int s;

	register FILE *f;
	register int c;
	register int lastc;

	if (name == NULL)
		return (0);
	host = strrchr(name, '@');
	if (host == NULL) {
		host = strrchr(name, '%');
		if (host != NULL)
			*host = '@';
		else
			return (0);
	}
	*host++ = 0;
	hp = gethostbyname(host);
	if (hp == NULL) {
		static struct hostent def;
		static struct in_addr defaddr;
		static char *alist[1];
		static char namebuf[128];
		u_long inet_addr();

		defaddr.s_addr = inet_addr(host);
		if (defaddr.s_addr == -1) {
			printf("unknown host: %s\n", host);
			return (1);
		}
		(void) strcpy(namebuf, host);
		def.h_name = namebuf;
		def.h_addr_list = alist, def.h_addr = (char *) &defaddr;
		def.h_length = sizeof(struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}
	printf("[%s]", hp->h_name);
	(void) fflush(stdout);
	sp = getservbyname("finger", "tcp");
	if (sp == 0) {
		printf("tcp/finger: unknown service\n");
		return (1);
	}
	sin.sin_family = hp->h_addrtype;
	memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
	sin.sin_port = sp->s_port;
	s = socket(hp->h_addrtype, SOCK_STREAM, 0);
	if (s < 0) {
		(void) fflush(stdout);
		perror("socket");
		return (1);
	}
	if (connect(s, (struct sockaddr *) & sin, sizeof(sin)) < 0) {
		(void) fflush(stdout);
		perror("connect");
		(void) close(s);
		return (1);
	}
	printf("\n");
	if (large)
		(void) write(s, "/W ", 3);
	(void) write(s, name, strlen(name));
	(void) write(s, "\r\n", 2);
	f = fdopen(s, "r");
	while ((c = getc(f)) != EOF) {
		switch (c) {
		case 0210:
		case 0211:
		case 0212:
		case 0214:
			c -= 0200;
			break;
		case 0215:
			c = '\n';
			break;
		}
		lastc = c;
		if (isprint(c) || isspace(c))
			(void) putchar(c);
		else
			(void) putchar(c ^ 100);
	}
	if (lastc != '\n')
		(void) putchar('\n');
	(void) fclose(f);
	return (1);
}

/*
 * Local Variables:
 * mode: c
 * c-indent-level: 8
 * c-continued-statement-offset: 8
 * c-brace-offset: -8
 * c-argdecl-indent: 8
 * c-label-offset: -8
 * End:
 */
