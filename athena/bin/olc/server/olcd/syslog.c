/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains the routines used for recording OLC daemon log
 * messages.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/syslog.c,v $
 *	$Id: syslog.c,v 1.13 1991-01-06 02:40:32 lwvanels Exp $
 *	$Author: lwvanels $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/olcd/syslog.c,v 1.13 1991-01-06 02:40:32 lwvanels Exp $";
#endif
#endif

#include <mit-copyright.h>

/*
 * If we're compiling with TEST, we want to use the stderr log, so
 * punt syslog for now.
 */
#ifdef TEST
#undef SYSLOG
#endif

#include <stdio.h>
#ifdef SYSLOG
#include <syslog.h>
#endif
#include <errno.h>
#include <com_err.h>

#include <olcd.h>

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

#ifndef SYSLOG
static FILE *open_log P((char *filename ));
static void log_to_file P((FILE *file , char *text ));
#else
static void log_to_syslogd P((int level , char *text ));
#endif /* SYSLOG */

#undef P

#ifndef SYSLOG

static FILE *status_log = (FILE *)NULL;
static FILE *error_log = (FILE *)NULL;
static FILE *admin_log = (FILE *) NULL;

/*
 * Function:	open_log
 * Purpose:	Opens a log file.  If the file cannot be opened, an
 *		attempt to send off a message to this effect is made,
 *		and exit is called.
 * Arguments:	const char *filename: Name of the file to be opened.
 * Returns:	FILE *: The opened file.
 * Visibility:	static
 * Notes:
 */

static FILE *
open_log (filename)
     char *filename;
{
    char msgbuf[BUFSIZ];
    FILE *file;

#ifdef TEST

    return stderr;

#else
    
    file = fopen (filename, "a");
    if (file)
	return file;
    /* We lose.  */
    sprintf (msgbuf, "OLCD fatal error: unable to open log file %s: %s",
	     filename, error_message (errno));
    fprintf (stderr,
#if __STDC__
	     "\a%s\a\n",
#else
	     "%s\n",
#endif
	     msgbuf);
    fflush (stderr);
    olc_broadcast_message ("syserror", msgbuf, "system");
    exit (1);

#endif /* TEST */
}

static void
log_to_file (file, text)
     FILE *file;
     char *text;
{
    char time_buf[32];
    time_now (time_buf);
    fprintf (file, "%s ", time_buf);
    write_line_to_log (file, text);
    (void) fflush (file);
}

#else /* use syslog */

static void
log_to_syslogd (level, text)
     int level;
     char *text;
{
    static int initialized = 0;
    if (!initialized) {
#if defined(ultrix)
#ifdef LOG_CONS
	openlog ("olc", LOG_CONS | LOG_PID);
#else
	openlog ("olc", LOG_PID);
#endif /* LOG_CONS */
#else
#ifdef LOG_CONS
	openlog ("olc", LOG_CONS | LOG_PID,SYSLOG_LEVEL);
#else
	openlog ("olc", LOG_PID, SYSLOG_LEVEL);
#endif /* LOG_CONS */
#endif /* ultrix */
	initialized = 1;
      }
    syslog (level, text);
}

#endif /* ! SYSLOG */
    
/*
 * Function:	log_error() writes an error message into the daemon's error
 *			log file.
 * Arguments:	message:	Error message to be written.
 * Returns:	nothing
 * Notes:
 *	First, open the error log file, printing an error message and dying 
 *	if an error occurs here.  Then, get the system time and print a
 *	formatted version in the error log, followed by the error message.  
 *	Then, send a broadcase system error message.
 *	Finally, close the file and return.
 */

void
log_error (message)
     char *message;
{
#ifdef SYSLOG

  log_to_syslogd (LOG_ERR, message);

#else

  if (error_log == (FILE *)NULL) 
      error_log = open_log (ERROR_LOG);

  log_to_file (error_log, message);
#endif
  olc_broadcast_message("syserror",message, "system");
}

/*
 * Function:	log_zephyr_error() writes an error message into the daemon's
 *              error log file, but doesn't try to send out a broadcast
 *		zephyrgram.
 * Arguments:	message:	Error message to be written.
 * Returns:	nothing
 * Notes:
 *	First, open the error log file, printing an error message and dying 
 *	if an error occurs here.  Then, get the system time and print a
 *	formatted version in the error log, followed by the error message.  
 *	Finally, close the file and return.
 *	This could be merged with log_error with an additional argument, but
 *	it's not that big of a deal.
 */


void
log_zephyr_error (message)
     char *message;
{
#ifdef SYSLOG

  log_to_syslogd (LOG_ERR, message);

#else

  if (error_log == (FILE *)NULL) 
      error_log = open_log (ERROR_LOG);

  log_to_file (error_log, message);
#endif
}


/*
 * Function:	log_status() writes a message to the olcd status log.
 * Arguments:	message:	Message to be written.
 * Returns:	nothing
 * Notes:
 *	First, open the status log file, recording an error and exiting 
 *	if one occurs.  Then, write the formatted current time and the
 *	status message into the status log.  Finally, close the log file
 *	and return.
 */

void
log_status(message)
     char *message;
{
#ifdef SYSLOG

  log_to_syslogd (LOG_INFO,message);

#else

  if (!status_log)
      status_log = open_log (STATUS_LOG);

  log_to_file (status_log, message);

#endif
}



void
log_admin(message)
     char *message;
{
#ifdef SYSLOG

  log_to_syslogd (LOG_NOTICE, message);

#else

  if (admin_log == (FILE *) NULL)
      admin_log = open_log (ADMIN_LOG);

  log_to_file (admin_log, message);

#endif
}

void
log_debug (message)
     char *message;
{
#ifdef SYSLOG
    log_to_syslogd (LOG_DEBUG, message);
#else
    /* ? new log file? */
#endif
}
