/* $Id: log.c,v 1.1.1.1 2001-01-16 15:25:52 ghudson Exp $
 *
 * Logs the various messages which tinyproxy produces to either a log file or
 * the syslog daemon. Not much to it...
 *
 * Copyright (C) 1998  Steven Young
 * Copyright (C) 1999  Robert James Kaes (rjkaes@flarenet.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * log.c - For the manipulation of log files.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <glib.h>

#include "sock.h"
#include "proxy.h"
#include "log.h"

#define LENGTH 16

/*
 * This routine logs messages to either the log file or the syslog function.
 */
void log(char *fmt, ...)
{
	va_list args;
	time_t nowtime;
	FILE *cf;
	static char time_string[LENGTH];
	char *out, *p;

	g_assert (fmt);

	if (!(cf = config.logf)) {
		/* then just don't log anything */
		return;
	}

	va_start (args, fmt);

	nowtime = time (NULL);
	/* Format is month day hour:minute:second (24 time) */
	strftime (time_string, LENGTH, "%b %d %H:%M:%S", localtime (&nowtime));

	out = g_strdup_vprintf (fmt, args);
	/* weed out any questionable chars */
	for (p = out; *p; p++) {
		if ((*p < ' ') || (*p == 127)) {
			*p = '?';
		}
	}

	fprintf (cf, "%s [%d]: %s", time_string, getpid (), out);
	fprintf (cf, "\n");
	fflush (cf);

	g_free (out);
	va_end (args);
}
