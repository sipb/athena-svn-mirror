/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the routines used for recording OLC daemon log
 * messages.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: syslog.c,v 1.19 1999-06-28 22:52:43 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: syslog.c,v 1.19 1999-06-28 22:52:43 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olcd.h>

#include <stdio.h>
#include <errno.h>

#ifdef   HAVE_SYSLOG_H
#include   <syslog.h>
#ifndef    LOG_CONS
#define      LOG_CONS 0  /* if LOG_CONS isn't defined, just ignore it */
#endif     /* LOG_CONS */
#endif /* HAVE_SYSLOG_H */

#define MSG_SIZE 2048  /* maximum size of the error messages */

static FILE *open_log_file (const char *filename);
static void log_to_file (FILE *file , char *text );

/*** initialization stuff ***/

static FILE *status_log = NULL;
static FILE *error_log =  NULL;
static FILE *admin_log =  NULL;

/* Open a log file.  If the file cannot be opened, an attempt to send
 * off a message is made before exiting.
 *
 * Arguments:   filename -- name of the file to be opened
 * Returns:     FILE* pointer for the opened file.
 * Non-local returns:   If fopen() fails, exits with status 1.
 */
static FILE *open_log_file (const char *filename)
{
  char msgbuf[MSG_SIZE];
  FILE *file;

  file = fopen (filename, "a");
  if (file)
    return file;

  /* We lose.  */
  my_snprintf (msgbuf, MSG_SIZE,
	       "OLCD fatal error: unable to open log file %s: %m", filename);
  fprintf (stderr, "\a%s\n", msgbuf);
  fflush (stderr);
  olc_broadcast_message ("syserror", msgbuf, "system");
  exit (1);
}

/* Initialize whichever logging mechanism is being used.
 *
 * Non-local returns:   If opening logs fails, exits with status 1.
 * Note: may safely be called any number of times.
 */
void init_logs(void)
{
  static int done = 0;
  if (done)
    return;

#ifdef HAVE_SYSLOG
  openlog ("olcd", LOG_PID | LOG_CONS, SYSLOG_FACILITY);
#else /* not HAVE_SYSLOG */

  error_log = open_log_file (ERROR_LOG);
  status_log = open_log_file (STATUS_LOG);
  admin_log = open_log_file (ADMIN_LOG);

#endif /* not HAVE_SYSLOG */

  done = 1;
}

/*** logging functions ***/

/* Ship out a line to a logfile, preceded by a timestamp.
 *
 * Arguments:   file -- output file
 *              text -- message to write
 * Note: using write_line_to_log() ensures the line ends in a '\n'.
 */
static void log_to_file (FILE *file, char *text)
{
  char time_buf[32];

  time_now (time_buf);
  fprintf (file, "%s ", time_buf);
  write_line_to_log (file, text);
  fflush (file);
}

/* Log an error message to syslogd or the error log file, and to Zephyr.
 * Arguments:	all of them -- error message to be written, as for printf.
 */
void log_error (const char *fmt, ...)
{
  va_list ap;
  char buf[MSG_SIZE];

  init_logs();

  va_start(ap, fmt);
  my_vsnprintf(buf, MSG_SIZE, fmt, ap);
  va_end(ap);

#ifdef HAVE_SYSLOG
  syslog(LOG_ERR, buf);
#else /* not HAVE_SYSLOG */
  log_to_file(error_log, buf);
#endif /* not HAVE_SYSLOG */

  olc_broadcast_message("syserror", buf, "system");
}

/* A one-argument version of log_error, suitable for set_olc_perror.
 * Arguments:	msg -- error message to be written.
 */
void log_error_string (const char *msg)
{
  log_error("%s: %m", msg);
}

/* Log an error message to syslogd or the error log file, but *not* Zephyr.
 * Arguments:	all of them -- error message to be written, as for printf.
 * Notes:
 *	This could be merged with log_error with an additional argument, but
 *	it's not that big of a deal.
 */
void log_zephyr_error (const char *fmt, ...)
{
  va_list ap;
  char buf[MSG_SIZE];

  init_logs();

  va_start(ap,fmt);
  my_vsnprintf(buf, MSG_SIZE, fmt, ap);
  va_end(ap);

#ifdef HAVE_SYSLOG
  syslog(LOG_ERR, buf);
#else /* not HAVE_SYSLOG */
  log_to_file(error_log, buf);
#endif /* not HAVE_SYSLOG */
}

/* Log a status message to syslogd or the status log file, but not Zephyr.
 * Arguments:	all of them -- error message to be written, as for printf.
 */
void log_status (const char *fmt, ...)
{
  va_list ap;
  char buf[MSG_SIZE];

  init_logs();

  va_start(ap,fmt);
  my_vsnprintf(buf, MSG_SIZE, fmt, ap);
  va_end(ap);

#ifdef HAVE_SYSLOG
  syslog(LOG_INFO, buf);
#else /* not HAVE_SYSLOG */
  log_to_file(status_log, buf);
#endif /* not HAVE_SYSLOG */
}

/* Log a message to syslogd or the admin log file, but not Zephyr.
 * Arguments:	all of them -- error message to be written, as for printf.
 * Note: This is only used in one place.  Do we really need it?
 */
void log_admin (const char *fmt, ...)
{
  va_list ap;
  char buf[MSG_SIZE];

  init_logs();

  va_start(ap,fmt);
  my_vsnprintf(buf, MSG_SIZE, fmt, ap);
  va_end(ap);

#ifdef HAVE_SYSLOG
  syslog(LOG_NOTICE, buf);
#else /* not HAVE_SYSLOG */
  log_to_file(admin_log, buf);
#endif /* not HAVE_SYSLOG */
}

/* Log a debugging message to syslogd (or just discard it if no syslogging).
 * Arguments:	all of them -- error message to be written, as for printf.
 */
void log_debug (const char *fmt, ...)
{
#ifdef HAVE_SYSLOG
  va_list ap;
  char buf[MSG_SIZE];

  init_logs();

  va_start(ap,fmt);
  my_vsnprintf(buf, MSG_SIZE, fmt, ap);
  va_end(ap);

  syslog(LOG_DEBUG, buf);
#endif /* not HAVE_SYSLOG */
}
