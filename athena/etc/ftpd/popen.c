/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software written by Ken Arnold and
 * published in UNIX Review, Vol. 6, No. 8.
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
 * static char sccsid[] = "@(#)popen.c	5.7 (Berkeley) 9/1/88";
 */

#ifndef lint
static char sccsid[] = "@(#)popen.c	5.3 (Berkeley) 11/30/88";
#endif /* not lint */

#include <sys/types.h>
#include <sys/signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
/*
 * Special version of popen which avoids call to shell.  This insures noone
 * may create a pipe to a hidden program as a side effect of a list or dir
 * command.
 */
static unsigned int *pids;
static int fds;
#ifdef ATHENA
uid_t athena_setuid = 0;
gid_t athena_setgid = 0;
#endif

FILE *
ftpd_popen(program, type)
	char *program, *type;
{
	register char *cp;
	FILE *iop;
	int argc, gargc, pdes[2], pid;
	char **pop, *argv[100], *gargv[1000], *vv[2];
	extern char **glob(), **copyblk(), *strtok();

	if (*type != 'r' && *type != 'w' || type[1])
		return(NULL);

	if (!pids) {
#ifndef POSIX
		if ((fds = getdtablesize()) <= 0)
#else
		if ((fds = sysconf(_SC_OPEN_MAX)) <= 0)
#endif
			return(NULL);
		if (!(pids =
		    (unsigned int *)malloc((u_int)(fds * sizeof(unsigned int)))))
			return(NULL);
		bzero(pids, fds * sizeof(unsigned int));
	}
	if (pipe(pdes) < 0)
		return(NULL);

	/* break up string into pieces */
	for (argc = 0, cp = program;; cp = NULL)
		if (!(argv[argc++] = strtok(cp, " \t\n")))
			break;

	/* glob each piece */
	gargv[0] = argv[0];
	for (gargc = argc = 1; argv[argc]; argc++) {
		if (!(pop = glob(argv[argc]))) {	/* globbing failed */
			vv[0] = argv[argc];
			vv[1] = NULL;
			pop = copyblk(vv);
		}
		argv[argc] = (char *)pop;		/* save to free later */
		while (*pop && gargc < 1000)
			gargv[gargc++] = *pop++;
	}
	gargv[gargc] = NULL;

	iop = NULL;
	switch(pid = vfork()) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		goto free;
		/* NOTREACHED */
	case 0:				/* child */
#ifdef ATHENA
		if (athena_setgid)
#ifdef _IBMR2
		  setgid_rios(athena_setgid);
#else
		  setgid(athena_setgid);
#endif
		if (athena_setuid)
#ifdef _IBMR2
		  setuid_rios(athena_setuid);
#else
		  setuid(athena_setuid);
#endif
#endif
		if (*type == 'r') {
			if (pdes[1] != 1) {
				dup2(pdes[1], 1);
				(void)close(pdes[1]);
			}
			(void)close(pdes[0]);
		} else {
			if (pdes[0] != 0) {
				dup2(pdes[0], 0);
				(void)close(pdes[0]);
			}
			(void)close(pdes[1]);
		}
		execv(gargv[0], gargv);
		_exit(1);
	}
	/* parent; assume fdopen can't fail...  */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}
	pids[fileno(iop)] = pid;

free:	for (argc = 1; argv[argc] != NULL; argc++)
		blkfree((char **)argv[argc]);
	return(iop);
}

pclose(iop)
	FILE *iop;
{
	register int fdes;
#ifdef POSIX
	sigset_t omask, nmask;
#else
	long omask;
#endif
	int pid, stat_loc;
	u_int waitpid();
#ifdef POSIX
	struct sigaction act;
#endif
	/*
	 * pclose returns -1 if stream is not associated with a
	 * `popened' command, or, if already `pclosed'.
	 */
	if (pids[fdes = fileno(iop)] == 0)
		return(-1);
	(void)fclose(iop);
#ifdef POSIX
	sigemptyset(&nmask);
	sigaddset(&nmask, SIGINT);
	sigaddset(&nmask, SIGQUIT);
	sigaddset(&nmask, SIGHUP);
	sigprocmask(SIG_BLOCK, &nmask, &omask);
#else
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
#endif
#if defined(_IBMR2) || defined(SOLARIS)
#ifdef POSIX
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler= (void (*)()) SIG_DFL;
	(void) sigaction (SIGCHLD, &act, NULL);
#else
	signal(SIGCHLD, SIG_DFL);
#endif
	while ((pid = waitpid(pids[fdes], &stat_loc, 0))
	       != pids[fdes] && pid != -1);
#ifdef POSIX
	act.sa_handler= (void (*)()) SIG_IGN;
	(void) sigaction (SIGCHLD, &act, NULL);
#else
	signal(SIGCHLD, SIG_IGN);
#endif
#else
	while ((pid = wait(&stat_loc)) != pids[fdes] && pid != -1);
#endif
#ifdef POSIX
	(void)sigprocmask(SIG_SETMASK, &omask, NULL);
#else
	(void)sigsetmask(omask);
#endif
	pids[fdes] = 0;
	return(stat_loc);
}
