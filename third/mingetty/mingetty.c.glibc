/*  mingetty.c
 *
 *  Copyright (C) 1996 Florian La Roche
 *  florian@jurix.jura.uni-sb.de florian@suse.de
 *
 *  Newer versions should be on susix.jura.uni-sb.de/pub/linux/source/system
 *  or sunsite.unc.edu/pub/Linux/system/Admin/login or /pub/Linux/system/
 *  Daemons/init/.
 *
 *  utmp-handling is from agetty in util-linux 2.5 (probably from
 *  Peter Orbaek <poe@daimi.aau.dk>)
 *
 *  S.u.S.E. - GmbH has paid some of the time that was needed to write
 *  this program. Thanks a lot for their support.
 *
 *  This getty can only be used as console getty. It is very small, but
 *  should be very reliable. For a modem getty, I'd also use nothing else
 *  but mgetty.
 *
 *  Usage: mingetty [--noclear] tty
 *  Example entry in /etc/inittab: 1:123:respawn:/sbin/mingetty tty1
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 */

#define DEBUG_THIS 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <utmp.h>
#include <getopt.h>

#ifdef linux
#include <sys/param.h>
#define USE_SYSLOG
#endif

 /* If USE_SYSLOG is undefined all diagnostics go directly to /dev/console. */
#ifdef	USE_SYSLOG
#include <syslog.h>
#endif

#define	ISSUE "/etc/issue"	/* displayed before the login prompt */
#include <sys/utsname.h>
#include <time.h>

#define LOGIN " login: "	/* login prompt */

/* name of this program (argv[0]) */
static char *progname;
/* on which tty line are we sitting? (e.g. tty1) */
static char *tty;
/* some information about this host */
static struct utsname uts;
/* the hostname */
static char hn[MAXHOSTNAMELEN + 1];
/* process ID of this program */
static pid_t pid;
/* current time */
static time_t cur_time;
/* do not send a reset string to the terminal ? */
static int noclear = 0;
/* Print the whole string of gethostname() instead of just until the next "." */
static int longhostname = 0;


/*
 * output error messages
 */
static void error (const char *fmt, ...)
{
	va_list va_alist;
	char buf[256], *bp;
#ifndef	USE_SYSLOG
	int fd;
#endif

#ifdef USE_SYSLOG
	buf[0] = '\0';
	bp = buf;
#else
	strcpy (buf, progname);
	strcat (buf, ": ");
	bp = buf + strlen (buf);
#endif

	va_start (va_alist, fmt);
	vsprintf (bp, fmt, va_alist);
	va_end (va_alist);

#ifdef	USE_SYSLOG
	openlog (progname, LOG_PID, LOG_AUTH);
	syslog (LOG_ERR, buf);
	closelog ();
#else
	strcat (bp, "\r\n");
	if ((fd = open ("/dev/console", 1)) >= 0) {
		write (fd, buf, strlen (buf));
		close (fd);
	}
#endif
	exit (1);
}

/*
 * update_utmp - update our utmp entry
 *
 * The utmp file holds miscellaneous information about things started by
 * /sbin/init and other system-related events. Our purpose is to update
 * the utmp entry for the current process, in particular the process
 * type and the tty line we are listening to. Return successfully only
 * if the utmp file can be opened for update, and if we are able to find
 * our entry in the utmp file.
 */
static void update_utmp (void)
{
	struct utmp ut;
	int ut_fd;
	struct utmp *utp;

	utmpname (_PATH_UTMP);
	setutent ();
	while ((utp = getutent ()))
		if (utp->ut_type == INIT_PROCESS && utp->ut_pid == pid)
			break;

	if (utp) {
		memcpy (&ut, utp, sizeof (ut));
	} else {
		/* some inits don't initialize utmp... */
		/* XXX we should print out a warning message */
		memset (&ut, 0, sizeof (ut));
		strncpy (ut.ut_id, tty + 3, sizeof (ut.ut_id));
	}
	endutent ();

	strncpy (ut.ut_user, "LOGIN", sizeof (ut.ut_user));
	strncpy (ut.ut_line, tty, sizeof (ut.ut_line));
	ut.ut_time = cur_time;
	ut.ut_type = LOGIN_PROCESS;
	ut.ut_pid = pid;

	pututline (&ut);
	endutent ();

	if ((ut_fd = open (_PATH_WTMP, O_APPEND | O_WRONLY)) >= 0) {
		flock (ut_fd, LOCK_EX);
		write (ut_fd, &ut, sizeof (ut));
		flock (ut_fd, LOCK_UN);
		close (ut_fd);
	}
}

/* open_tty - set up tty as standard { input, output, error } */
static void open_tty (void)
{
	struct sigaction sa;
	char buf[20];
	int fd;

	/* Set up new standard input. */
	strcpy (buf, "/dev/");
	strcat (buf, tty);
	if (chown (buf, 0, 0) || chmod (buf, 0600))
		error ("%s: %s", buf, sys_errlist[errno]);

	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGHUP, &sa, NULL);

	/* vhangup() will replace all open file descriptors that point to our
	   controlling tty by a dummy that will deny further reading/writing
	   to our device. It will also reset the tty to sane defaults, so we
	   don't have to modify the tty device for sane settings.
	   We also get a SIGHUP/SIGCONT.
	 */
	if ((fd = open (buf, O_RDWR, 0)) < 0
		|| ioctl (fd, TIOCSCTTY, (void *)1) == -1)
		error ("%s: cannot open tty: %s", buf, sys_errlist[errno]);
	if (!isatty (fd))
		error ("%s: not a tty", buf);

	vhangup ();
	/* Get rid of the present stdout/stderr. */
	close (2);
	close (1);
	close (0);
	close (fd);
	/* ioctl (0, TIOCNOTTY, (char *)1); */

	if (open (buf, O_RDWR, 0) != 0)
		error ("%s: cannot open as standard input: %s", buf,
				sys_errlist[errno]);

	/* Set up standard output and standard error file descriptors. */
	if (dup (0) != 1 || dup (0) != 2)
		error ("%s: dup problem: %s", buf, sys_errlist[errno]);

	/* Write a reset string to the terminal. This is very linux-specific
	   and should be checked for other systems. */
	if (! noclear)
		write (0, "\033c", 2);

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigemptyset (&sa.sa_mask);
	sigaction (SIGHUP, &sa, NULL);

#if	DEBUG_THIS
	printf ("session=%d, pid=%d, pgid=%d\n", getsid (0), getpid (),
			getpgid (0));
#endif
}

static void output_special_char (unsigned char c)
{
	switch (c) {
	case 's':
		printf ("%s", uts.sysname);
		break;
	case 'n':
		printf ("%s", uts.nodename);
		break;
	case 'r':
		printf ("%s", uts.release);
		break;
	case 'v':
		printf ("%s", uts.version);
		break;
	case 'm':
		printf ("%s", uts.machine);
		break;
	case 'o':
		printf ("%s", uts.domainname);
		break;
#if 0
	case 'd':
	case 't':
		{
			char *weekday[] =
			{"Sun", "Mon", "Tue", "Wed", "Thu",
			 "Fri", "Sat"};
			char *month[] =
			{"Jan", "Feb", "Mar", "Apr", "May",
			 "Jun", "Jul", "Aug", "Sep", "Oct",
			 "Nov", "Dec"};
			time_t now;
			struct tm *tm;

			time (&now);
			tm = localtime (&now);

			if (c == 'd')
				printf ("%s %s %d  %d",
				    weekday[tm->tm_wday], month[tm->tm_mon],
					tm->tm_mday,
				     tm->tm_year < 70 ? tm->tm_year + 2000 :
					tm->tm_year + 1900);
			else
				printf ("%02d:%02d:%02d",
					tm->tm_hour, tm->tm_min, tm->tm_sec);

			break;
		}
#else
	case 'd':
	case 't':
		{
			char buff[20];
			struct tm *tm = localtime (&cur_time);
			strftime (buff, sizeof (buff),
				c == 'd'? "%a %b %d %Y" : "%X", tm);
			fputs (buff, stdout);
			break;
		}
#endif

	case 'l':
		printf ("%s", tty);
		break;
	case 'u':
	case 'U':
		{
			int users = 0;
			struct utmp *ut;
			setutent ();
			while ((ut = getutent ()))
				if (ut->ut_type == USER_PROCESS)
					users++;
			endutent ();
			printf ("%d", users);
			if (c == 'U')
				printf (" user%s", users == 1 ? "" : "s");
			break;
		}
	default:
		putchar (c);
	}
}

/* do_prompt - show login prompt, optionally preceded by /etc/issue contents */
static void do_prompt (void)
{
#if	! OLD
	FILE *fd;
#else
	int fd;
#endif
	char c;

	write (1, "\n", 1);	/* start a new line */
#if	! OLD
	if ((fd = fopen (ISSUE, "r"))) {
		while ((c = getc (fd)) != EOF) {
			if (c == '\\')
				output_special_char (getc(fd));
			else
				putchar (c);
		}
		fflush (stdout);
		fclose (fd);
	}
#else
	if ((fd = open (ISSUE, O_RDONLY)) >= 0) {
		close (fd);
	}
#endif
	write (1, hn, strlen (hn));
	write (1, LOGIN, sizeof (LOGIN) - 1);
}

/* get_logname - get user name, establish speed, erase, kill, eol */
static char *get_logname (void)
{
	static char logname[40];
	char *bp;
	unsigned char c;

	ioctl (0, TCFLSH, 0);	/* flush pending input */

	for (*logname = 0; *logname == 0;) {
		do_prompt ();
		for (bp = logname;;) {
			if (read (0, &c, 1) < 1) {
				if (errno == EINTR || errno == EIO
							|| errno == ENOENT)
					exit (0);
				error ("%s: read: %s", tty, sys_errlist[errno]);
			}
			if (c == '\n' || c == '\r') {
				*bp = 0;
				break;
			} else if (!isalnum (c) && c != '_')
				error ("%s: invalid character for login name",
								tty);
			else if (bp - logname >= sizeof (logname) - 1)
				error ("%s: too long login name", tty);
			else
				*bp++ = c;
		}
	}
	return logname;
}

static void usage (void)
{
	error ("usage: '%s tty' with e.g. tty=tty1", progname);
}

static struct option const long_options[] = {
	{ "noclear", no_argument, &noclear, 1},
	{ "long-hostname", no_argument, &longhostname, 1},
	{ 0, 0, 0, 0 }
};

/*
 * main program
 */
int main (int argc, char **argv)
{
	char *logname, *s;
	int c;

	progname = argv[0];
	uname (&uts);
	gethostname (hn, MAXHOSTNAMELEN);
	pid = getpid ();
	time (&cur_time);
#if 1
	putenv ("TERM=linux");
#endif

	while ((c = getopt_long (argc, argv, "", long_options, (int *) 0))
			!= EOF) {
		switch (c) {
			case 0:
				break;
			default:
				usage ();
		}
	}
	if (!longhostname && (s = strchr(hn, '.')))
		*s = '\0';
	tty = argv[optind];
	if (! tty)
		usage ();

	update_utmp ();
	open_tty ();
#if 0
#ifdef linux
	ioctl (0, TIOCSPGRP, &pid);
#endif
#endif
	/* flush input and output queues, important for modems */
	ioctl (0, TCFLSH, 2);

	while ((logname = get_logname ()) == 0);

	execl (_PATH_LOGIN, _PATH_LOGIN, "--", logname, NULL);
	error ("%s: can't exec " _PATH_LOGIN ": %s", tty, sys_errlist[errno]);
	exit (0);
}

