/*
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
 */

#include "conf.h"
#include <sys/types.h>
#include <sys/file.h>
#include <utmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef NEED_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif
#include <osconf.h>
#ifdef USE_UNISTD_H
#include <unistd.h>
#endif

/* solaris uses these names */
#ifdef UTMP_FILE
#define UTMPFILE UTMP_FILE
#endif
#ifdef WTMP_FILE
#define WTMPFILE WTMP_FILE
#endif

#ifndef UTMPFILE
#define	UTMPFILE	"/etc/utmp"
#endif
#ifndef WTMPFILE
#define	WTMPFILE	"/usr/adm/wtmp"
#endif

#ifdef sgi
#define _AIX
#endif

#if defined(_AIX) || defined(linux) || defined(solaris20) || (defined(__alpha) && defined(__osf__)) || (defined(__sgi) && defined(__SYSTYPE_SVR4)) || defined(__SCO__)
#define HAVE_GETUTENT
#endif
#ifndef EMPTY
/* linux has UT_UNKNOWN but not EMPTY */
#define EMPTY UT_UNKNOWN
#endif

void
login(ut)
	struct utmp *ut;
{
	register int fd;
	struct utmp utmp;
	int tty;

#ifdef HAVE_GETUTENT
	if (!ut->ut_pid)
		ut->ut_pid = getppid();
	ut->ut_type = USER_PROCESS;

	if (sizeof ut->ut_id > 4)
	    (void) strncpy(ut->ut_id, ut->ut_line, sizeof(ut->ut_id));
	else if (!strncmp (ut->ut_line, "pts/", 4)) {
	    ut->ut_id[0] = 'k';
	    ut->ut_id[1] = 'l';
	    strncpy(ut->ut_id + 2, ut->ut_line + 4, sizeof(ut->ut_id) - 2);
	}
	else if (!strncmp (ut->ut_line, "tty", 3))
	    (void) strncpy(ut->ut_id, ut->ut_line + 3, sizeof(ut->ut_id));
	else
	    (void) strncpy(ut->ut_id, ut->ut_line, sizeof(ut->ut_id));

	(void) setutent();
	(void) memset((char *)&utmp, 0, sizeof(utmp));
	(void) strncpy(utmp.ut_id, ut->ut_id, sizeof(utmp.ut_id));
	utmp.ut_type = DEAD_PROCESS;
	(void) getutid(&utmp);

	(void) pututline(ut);
	(void) endutent();
#else
	tty = ttyslot();
	if (tty > 0 && (fd = open(UTMPFILE, O_WRONLY, 0)) >= 0) {
		(void)lseek(fd, (long)(tty * sizeof(struct utmp)), SEEK_SET);
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
#endif
	if ((fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) >= 0) {
		(void)write(fd, (char *)ut, sizeof(struct utmp));
		(void)close(fd);
	}
}

logout(line)
	register char *line;
{
	register FILE *fp;
	struct utmp ut;
	int rval;

	if (!(fp = fopen(UTMPFILE, "r+")))
		return(0);
	rval = 1;
	while (fread((char *)&ut, sizeof(struct utmp), 1, fp) == 1) {
		if (!ut.ut_name[0] ||
		    strncmp(ut.ut_line, line, sizeof(ut.ut_line)))
			continue;
		(void)memset(ut.ut_name, 0, sizeof(ut.ut_name));
#ifndef NOUTHOST
		(void)memset(ut.ut_host, 0, sizeof(ut.ut_host));
#endif
		(void)time(&ut.ut_time);
#ifdef HAVE_GETUTENT
		(void)memset(ut.ut_id, 0, sizeof(ut.ut_id));
		ut.ut_pid = 0;
#ifndef linux
		ut.ut_exit.e_exit = 0;
#endif
		ut.ut_type = EMPTY;
#endif
		(void)fseek(fp, (long)-sizeof(struct utmp), SEEK_CUR);
		(void)fwrite((char *)&ut, sizeof(struct utmp), 1, fp);
		(void)fseek(fp, (long)0, SEEK_CUR);
		rval = 0;
	}
	(void)fclose(fp);
	return(rval);
}

logwtmp(line, name, host)
	char *line, *name, *host;
{
	struct utmp ut;
	struct stat buf;
	int fd;

	if ((fd = open(WTMPFILE, O_WRONLY|O_APPEND, 0)) < 0)
		return;
	if (!fstat(fd, &buf)) {
		(void)memset((char *)&ut, 0, sizeof(ut));
		(void)strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		(void)strncpy(ut.ut_name, name, sizeof(ut.ut_name));
#ifndef NOUTHOST
		(void)strncpy(ut.ut_host, host, sizeof(ut.ut_host));
#endif
		(void)time(&ut.ut_time);
#ifdef HAVE_GETUTENT
		if (*name) {
			if (!ut.ut_pid)
				ut.ut_pid = getpid();
			ut.ut_type = USER_PROCESS;
		} else {
			ut.ut_type = EMPTY;
		}
#endif
		if (write(fd, (char *)&ut, sizeof(struct utmp)) !=
		    sizeof(struct utmp))
			(void)ftruncate(fd, buf.st_size);
	}
	(void)close(fd);
}
