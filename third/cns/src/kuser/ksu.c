/*
 * ksu.c
 *
 * Copyright (c) 1988 The Regents of the University of California.
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
 *
 * Kerberos additions Copyright 1987, 1988 by the Massachusetts Institute
 * of Technology. For copying and distribution information, please see
 * the file <mit-copyright.h>.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)su.c	5.11 (Berkeley) 12/7/88";
#endif /* not lint */

#include "mit-copyright.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include "krb.h"
#include <netdb.h>
#include <sys/ioctl.h>

#ifdef USE_UNISTD_H
#include <unistd.h>
#endif

#ifndef LOG_AUTH
#define LOG_AUTH 0
#endif /* LOG_AUTH */

/* for Ultrix and friends ... */
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

int     kerno;
char    lrealm[REALM_SZ];
char    krbtkfile[128];

#define MAXPWSIZE	128	/* Biggest string we accept for a password
				   (includes space for null terminator) */

#ifdef __svr4__
#include <time.h>
#endif

void svr4_log (from, to, tty, ok)
	char *from, *to, *tty;
	int ok;
{
#ifdef __svr4__
	time_t now;
	struct tm *tm;
	FILE *f;

	setuid (0);		/* we're only called just before exiting, or on success */
	f = fopen ("/var/adm/sulog", "a");
	if (f == NULL) {
		perror ("/var/adm/sulog");
		return;
	}
	time (&now);
	tm = localtime (&now);
	if (!strncmp ("/dev/", tty, 5))
		tty += 5;
	if (tm)
		fprintf (f, "KSU %02d/%02d %02d:%02d %c %s %s-%s\n",
			 tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
			 ok ? '+' : '-',
			 tty, from, to);
	else
		perror ("localtime");
	fclose (f);
#endif
}

int main(argc, argv)
	int argc;
	char **argv;
{
	extern char **environ;
	extern int errno, optind;
	register struct passwd *pwd;
	register char *p, **g;
	struct group *gr;
	uid_t ruid, getuid();
#ifdef NO_GETUSERSHELL
	int ch, fulllogin, fastlogin, prio;
#else
	int asme, ch, fulllogin, fastlogin, prio;
#endif /* NO_GETUSERSHELL */
	int use_preauth = 0;
	enum { UNSET, YES, NO } iscsh = UNSET;
	char *user, *shell, *username, *cleanenv[2], *nargv[4], **np;
	char namebuf[50], shellbuf[MAXPATHLEN];
	char *crypt(), *getpass(), *getenv(), *getlogin();
#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else
#define read_long_pw_string des_read_pw_string
#endif
	int read_long_pw_string();
	char pw_buf[MAXPWSIZE];
	char   *mytty;
  
	mytty = isatty(2) ? (char *) ttyname(2) : "(no tty)";

	np = &nargv[3];
	*np-- = NULL;
#ifdef NO_GETUSERSHELL
	fulllogin = fastlogin = 0;
#define	GETOPTARG "-flp"
#else
	asme = fulllogin = fastlogin = 0;
#define GETOPTARG "-flmp"
#endif
	while ((ch = getopt(argc, argv, GETOPTARG)) != EOF)
		switch((char)ch) {
		case 'f':
			fastlogin = 1;
			break;
		case 'p':
			use_preauth = 1;
			break;
		case '-':
		case 'l':
			fulllogin = 1;
			break;
#ifndef NO_GETUSERSHELL
		case 'm':
			asme = 1;
			break;
#endif
		case '?':
		default:
#ifdef NO_GETUSERSHELL
			fprintf(stderr, "usage: ksu [-flp] [login]\n");
#else
			fprintf(stderr, "usage: ksu [-flmp] [login]\n");
#endif /* NO_GETUSERSHELL */
			exit(1);
		}
	argv += optind;

	errno = 0;
#ifndef NO_SETPRIORITY
	prio = getpriority(PRIO_PROCESS, 0);
	if (errno)
		prio = 0;
	(void)setpriority(PRIO_PROCESS, 0, -2);
#endif

	/* get current login name and shell */
	if ((pwd = getpwuid(ruid = getuid())) == NULL) {
		fprintf(stderr, "ksu: who are you?\n");
		exit(1);
	}
	username = strcpy(namebuf, pwd->pw_name);
#ifndef NO_GETUSERSHELL
	if (asme)
		if (pwd->pw_shell && *pwd->pw_shell)
			shell = strcpy(shellbuf,  pwd->pw_shell);
		else {
			shell = "/bin/sh";
			iscsh = NO;
		}
#endif

	/* get target login information */
	user = *argv ? *argv : "root";
	if ((pwd = getpwnam(user)) == NULL) {
		fprintf(stderr, "ksu: unknown login %s\n", user);
		exit(1);
	}

	/*
	 * Only allow those with Kerberos "foo" instances in the ~foo/.klogin
	 * file to su to "foo".
	 */
	{
	    KTEXT_ST ticket;
	    AUTH_DAT authdata;
	    char hostname[MAXHOSTNAMELEN], savehost[MAXHOSTNAMELEN];
	    char promptbuf[100];
	    unsigned long faddr;
	    struct hostent *hp;

	    /* First lets see if he has a chance! */
	    if (krb_get_lrealm(lrealm, 1) != KSUCCESS) {
		fprintf(stderr,"Unable to get local realm\n");
		svr4_log (username, user, mytty, 0);
		exit(1);
	    }
	    if (koktologin(username, lrealm, user)) {
		fprintf(stderr,"You are not allowed to ksu to %s\n", user);
		svr4_log (username, user, mytty, 0);
		exit(1);
	    }
	    sprintf(krbtkfile, "/tmp/tkt_%s_%d", user, getuid());
	    setuid(0);		/* so ticket file has good protection */
	    sprintf (promptbuf, "Your `%s' instance password: ", user);
	    if (read_long_pw_string(pw_buf, sizeof(pw_buf)-1, promptbuf, 0)) {
		fprintf(stderr,"Error reading password.\n");
		svr4_log (username, user, mytty, 0);
		exit(1);
	    }
	    p = pw_buf;
	    setenv("KRBTKFILE", krbtkfile, 1);
	    if (use_preauth)
		kerno = krb_get_pw_in_tkt_preauth(username, user, lrealm,
						  "krbtgt", lrealm, 2, p);
	    else
		kerno = krb_get_pw_in_tkt(username, user, lrealm, "krbtgt",
				 	  lrealm, 2, p);
	    memset(p, 0, strlen(p));
	    if (kerno != KSUCCESS) {
		printf("Unable to ksu: %s\n", krb_get_err_text (kerno));
		syslog(LOG_NOTICE|LOG_AUTH, "ksu: BAD SU %s on %s: %s",
		       username, mytty, krb_get_err_text (kerno));
		svr4_log (username, user, mytty, 0);
		exit(1);
	    }
#ifndef NO_SETPRIORITY
	    setpriority(PRIO_PROCESS, 0, -2);
#endif
	    /*
	     * Now use the ticket for something useful, to make sure
	     * it is valid.
	     */
	    if (gethostname(hostname, sizeof(hostname)) == -1) {
		perror("cannot retrieve hostname");
		dest_tkt();
		svr4_log (username, user, mytty, 0);
		exit(1);
	    }
	    (void) strncpy(savehost, krb_get_phost(hostname),
			   sizeof(savehost));
	    savehost[sizeof(savehost)-1] = 0;

	    kerno = krb_mk_req(&ticket, "rcmd", savehost, lrealm, 33);
	    if (kerno == KDC_PR_UNKNOWN) {
		printf("Warning: tgt not verified\n");
		syslog(LOG_NOTICE|LOG_AUTH, "ksu: %s on %s: tgt not verified",
		       username, mytty);
	    } else if (kerno != KSUCCESS) {
		printf("Unable to use tgt: %s\n", krb_get_err_text (kerno));
		syslog(LOG_NOTICE|LOG_AUTH, "ksu: failed su: %s on %s: %s",
		       username, mytty, krb_get_err_text (kerno));
		dest_tkt();
		svr4_log (username, user, mytty, 0);
		exit(1);
	    } else {
		if (!(hp = gethostbyname(hostname))) {
		    printf("Unable to get address of %s\n",hostname);
		    dest_tkt();
		    svr4_log (username, user, mytty, 0);
		    exit(1);
		}
		memcpy((char *) &faddr, (char *)hp->h_addr, sizeof(faddr));
		if ((kerno = krb_rd_req(&ticket, "rcmd", savehost,
					faddr, &authdata, "")) != KSUCCESS) {
		    printf("Unable to verify rcmd ticket: %s\n",
			   krb_get_err_text (kerno));
		    syslog(LOG_NOTICE|LOG_AUTH, "ksu: failed su: %s on %s: %s",
			   username, mytty, krb_get_err_text (kerno));
		    dest_tkt();
		    svr4_log (username, user, mytty, 0);
		    exit(1);
		}		
	    }
	    printf("Don't forget to kdestroy before exiting the new shell.\n");
	    svr4_log (username, user, mytty, 1);
	}

#ifndef NO_GETUSERSHELL
	if (asme) {
		/* if asme and non-standard target shell, must be root */
		if (!chshell(pwd->pw_shell) && ruid) {
			fprintf(stderr, "ksu: Permission denied.\n");
			dest_tkt();
			exit(1);
		}
	}
	else
#endif
	if (pwd->pw_shell && *pwd->pw_shell) {
		shell = pwd->pw_shell;
		iscsh = UNSET;
	} else {
		shell = "/bin/sh";
		iscsh = NO;
	}

	/* if we're forking a csh, we want to slightly muck the args */
	if (iscsh == UNSET) {
		if (p = strrchr(shell, '/'))
			++p;
		else
			p = shell;
		iscsh = strcmp(p, "csh") ? NO : YES;
	}

	/* set permissions */
	if (setgid(pwd->pw_gid) < 0) {
		perror("ksu: setgid");
		dest_tkt();
		exit(1);
	}
#ifndef __SCO__
	if (initgroups(user, pwd->pw_gid)) {
		fprintf(stderr, "ksu: initgroups failed\n");
		dest_tkt();
		exit(1);
	}
#endif
	if (setuid(pwd->pw_uid) < 0) {
		perror("ksu: setuid");
		dest_tkt();
		exit(1);
	}

#ifndef NO_GETUSERSHELL
	if (!asme) {
#endif
		if (fulllogin) {
			p = getenv("TERM");
			cleanenv[0] = "PATH=:/usr/ucb:/bin:/usr/bin";
			cleanenv[1] = NULL;
			environ = cleanenv;
			(void)setenv("TERM", p, 1);
			if (chdir(pwd->pw_dir) < 0) {
				fprintf(stderr, "ksu: no directory\n");
				dest_tkt();
				exit(1);
			}
		}
		if (fulllogin || pwd->pw_uid)
			(void)setenv("USER", pwd->pw_name, 1);
		(void)setenv("HOME", pwd->pw_dir, 1);
		(void)setenv("SHELL", shell, 1);
#ifndef NO_GETUSERSHELL
	}
#endif

	if (iscsh == YES) {
		if (fastlogin)
			*np-- = "-f";
#ifndef NO_GETUSERSHELL
		if (asme)
			*np-- = "-m";
#endif
	}

	/* csh strips the first character... */
	*np = fulllogin ? "-ksu" : iscsh == YES ? "_ksu" : "ksu";

	if (pwd->pw_uid == 0)
		syslog(LOG_NOTICE|LOG_AUTH, "ksu: %s on %s",
		    username, mytty);
#ifndef NO_SETPRIORITY
	(void)setpriority(PRIO_PROCESS, 0, prio);
#endif
	execv(shell, np);
	fprintf(stderr, "ksu: no shell.\n");
	dest_tkt();
	exit(1);
}

#ifndef NO_GETUSERSHELL
chshell(sh)
	char *sh;
{
	char *cp, *getusershell();

	while ((cp = getusershell()) != NULL)
		if (!strcmp(cp, sh))
			return(1);
	return(0);
}
#endif /* NO_GETUSERSHELL */

koktologin(name, realm, user)
    char   *name;
    char   *realm;
    char   *user;
{
    struct auth_dat kdata_st;
    AUTH_DAT *kdata = &kdata_st;
    /* Cons together an AUTH_DAT structure for kuserok */
    memset((caddr_t) kdata, 0, sizeof(*kdata));
    strcpy(kdata->pname, name);
    strcpy(kdata->pinst, user);
    strcpy(kdata->prealm, realm);
    return (kuserok(kdata, user));
}
