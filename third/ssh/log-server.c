/*

log-server.c

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Mon Mar 20 21:19:30 1995 ylo

Server-side versions of debug(), log_msg(), etc.  These normally send the
output to the system log.

*/

/*
 * $Id: log-server.c,v 1.4 1999-03-08 18:20:06 danw Exp $
 * $Log: not supported by cvs2svn $
 * Revision 1.3  1998/01/24 01:47:25  danw
 * merge in changes for 1.2.22
 *
 * Revision 1.2  1997/11/12 21:16:16  danw
 * Athena-login changes (including some krb4 stuff)
 *
 * Revision 1.1.1.1  1997/10/17 22:26:02  danw
 * Import of ssh 1.2.21
 *
 * Revision 1.1.1.2  1998/01/24 01:25:21  danw
 * Import of ssh 1.2.22
 *
 * Revision 1.1.1.3  1999/03/08 17:43:06  danw
 * Import of ssh 1.2.26
 *
 * Revision 1.6  1998/05/23  20:21:43  kivinen
 * 	Changed () -> (void).
 *
 * Revision 1.5  1998/01/02  06:18:49  kivinen
 * 	Fixed kerberos ticket name handling.
 *
 * Revision 1.4  1997/04/17 04:05:51  kivinen
 * 	Added return to end of syslog_severity to remove warning about
 * 	it.
 *
 * Revision 1.3  1997/03/27 03:09:58  kivinen
 * 	Added kerberos patches from Glenn Machin.
 *
 * Revision 1.2  1996/10/29 22:38:23  kivinen
 * 	log -> log_msg.
 *
 * Revision 1.1.1.1  1996/02/18 21:38:12  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.5  1995/10/02  01:22:57  ylo
 * 	Include sys/syslog.h if needed.
 *
 * Revision 1.4  1995/09/09  21:26:42  ylo
 * /m/shadows/u2/users/ylo/ssh/README
 *
 * Revision 1.3  1995/08/21  23:25:00  ylo
 * 	Added support for syslog facility.
 *
 * Revision 1.2  1995/07/13  01:26:21  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#include "includes.h"
#include <syslog.h>
#ifdef NEED_SYS_SYSLOG_H
#include <sys/syslog.h>
#endif /* NEED_SYS_SYSLOG_H */
#include "packet.h"
#include "xmalloc.h"
#include "ssh.h"

static int log_debug = 0;
static int log_quiet = 0;
static int log_on_stderr = 0;

#ifdef KERBEROS
extern krb5_context ssh_context;
#endif

/* Initialize the log.
     av0	program name (should be argv[0])
     on_stderr	print also on stderr
     debug	send debugging messages to system log
     quiet	don\'t log anything
     */

void log_init(char *av0, int on_stderr, int debug, int quiet, 
	      SyslogFacility facility)
{
  int log_facility;
  
  switch (facility)
    {
    case SYSLOG_FACILITY_DAEMON:
      log_facility = LOG_DAEMON;
      break;
    case SYSLOG_FACILITY_USER:
      log_facility = LOG_USER;
      break;
    case SYSLOG_FACILITY_AUTH:
      log_facility = LOG_AUTH;
      break;
    case SYSLOG_FACILITY_LOCAL0:
      log_facility = LOG_LOCAL0;
      break;
    case SYSLOG_FACILITY_LOCAL1:
      log_facility = LOG_LOCAL1;
      break;
    case SYSLOG_FACILITY_LOCAL2:
      log_facility = LOG_LOCAL2;
      break;
    case SYSLOG_FACILITY_LOCAL3:
      log_facility = LOG_LOCAL3;
      break;
    case SYSLOG_FACILITY_LOCAL4:
      log_facility = LOG_LOCAL4;
      break;
    case SYSLOG_FACILITY_LOCAL5:
      log_facility = LOG_LOCAL5;
      break;
    case SYSLOG_FACILITY_LOCAL6:
      log_facility = LOG_LOCAL6;
      break;
    case SYSLOG_FACILITY_LOCAL7:
      log_facility = LOG_LOCAL7;
      break;
    default:
      fprintf(stderr, "Unrecognized internal syslog facility code %d\n",
	      (int)facility);
      exit(1);
    }

  log_debug = debug;
  log_quiet = quiet;
  log_on_stderr = on_stderr;
  closelog(); /* Close any previous log. */
  openlog(av0, LOG_PID, log_facility);
}

/* Log this message (information that usually should go to the log). */

void log_msg(const char *fmt, ...)
{
  char buf[1024];
  va_list args;
  if (log_quiet)
    return;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "log: %s\n", buf);
  syslog(LOG_INFO, "log: %.500s", buf);
}

/* Converts portable syslog severity to machine-specific syslog severity. */

static int syslog_severity(int severity)
{
  switch (severity)
    {
    case SYSLOG_SEVERITY_DEBUG:
      return LOG_DEBUG;
    case SYSLOG_SEVERITY_INFO:
      return LOG_INFO;
    case SYSLOG_SEVERITY_NOTICE:
      return LOG_NOTICE;
    case SYSLOG_SEVERITY_WARNING:
      return LOG_WARNING;
    case SYSLOG_SEVERITY_ERR:
      return LOG_ERR;
    case SYSLOG_SEVERITY_CRIT:
      return LOG_CRIT;
    default:
      fatal("syslog_severity: bad severity %d", severity);
    }
  return 0;
}

/* Log this message (information that usually should go to the log) at
   the given severity level. */

void log_severity(SyslogSeverity severity, const char *fmt, ...)
{
  char buf[1024];
  va_list args;
  if (log_quiet)
    return;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "log: %s\n", buf);
  syslog(syslog_severity(severity), "log: %.500s", buf);
}

/* Debugging messages that should not be logged during normal operation. */

void debug(const char *fmt, ...)
{
  char buf[1024];
  va_list args;
  if (!log_debug || log_quiet)
    return;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "debug: %s\n", buf);
  syslog(LOG_DEBUG, "debug: %.500s", buf);
}

/* Error messages that should be logged. */

void error(const char *fmt, ...)
{
  char buf[1024];
  va_list args;
  if (log_quiet)
    return;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "error: %s\n", buf);
  syslog(LOG_ERR, "error: %.500s", buf);
}

struct fatal_cleanup
{
  struct fatal_cleanup *next;
  void (*proc)(void *);
  void *context;
};

static struct fatal_cleanup *fatal_cleanups = NULL;

/* Registers a cleanup function to be called by fatal() before exiting. */

void fatal_add_cleanup(void (*proc)(void *), void *context)
{
  struct fatal_cleanup *cu;

  cu = xmalloc(sizeof(*cu));
  cu->proc = proc;
  cu->context = context;
  cu->next = fatal_cleanups;
  fatal_cleanups = cu;
}

/* Removes a cleanup frunction to be called at fatal(). */

void fatal_remove_cleanup(void (*proc)(void *context), void *context)
{
  struct fatal_cleanup **cup, *cu;
  
  for (cup = &fatal_cleanups; *cup; cup = &cu->next)
    {
      cu = *cup;
      if (cu->proc == proc && cu->context == context)
	{
	  *cup = cu->next;
	  xfree(cu);
	  return;
	}
    }
  fatal("fatal_remove_cleanup: no such cleanup function: 0x%lx 0x%lx\n",
	(unsigned long)proc, (unsigned long)context);
}

static void do_fatal_cleanups(void)
{
  struct fatal_cleanup *cu, *next_cu;
  static int fatal_called = 0;
#ifdef KERBEROS
  extern char *ticket;
#endif

  if (!fatal_called)
    {
      fatal_called = 1;

      /* Call cleanup functions. */
      for (cu = fatal_cleanups; cu; cu = next_cu)
	{
	  next_cu = cu->next;
	  debug("Calling cleanup 0x%lx(0x%lx)",
		(unsigned long)cu->proc, (unsigned long)cu->context);
	  (*cu->proc)(cu->context);
	}
#ifdef KERBEROS
      /* If you forwarded a ticket you get one shot for proper
	 authentication. */
      /* If tgt was passed, destroy it */
      if (ticket)
	{
	  if (strcmp(ticket,"none"))
	    {
	      krb5_ccache ccache;
	      if (!krb5_cc_resolve(ssh_context, ticket, &ccache))
		krb5_cc_destroy(ssh_context, ccache);
	      dest_tkt();
	    }
	  else
	    ticket = NULL;
	}
#endif /* KERBEROS */
    }
}

/* Fatal messages.  This function never returns. */

void fatal(const char *fmt, ...)
{
  char buf[1024];
  va_list args;

  if (log_quiet)
    exit(1);
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "fatal: %s\n", buf);
  syslog(LOG_ERR, "fatal: %.500s", buf);

  do_fatal_cleanups();

  exit(1);
}

void fatal_severity(SyslogSeverity severity, const char *fmt, ...)
{
  char buf[1024];
  va_list args;

  if (log_quiet)
    exit(1);
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  if (log_on_stderr)
    fprintf(stderr, "fatal: %s\n", buf);
  syslog(syslog_severity(severity), "fatal: %.500s", buf);

  do_fatal_cleanups();

  exit(1);
}
