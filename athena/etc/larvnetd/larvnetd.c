/* Copyright 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file is part of larvnetd, a monitoring server.  It implements
 * the main loop.
 */

static const char rcsid[] = "$Id: larvnetd.c,v 1.3 1998-10-13 17:12:58 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <hesiod.h>
#include "larvnetd.h"
#include "larvnet.h"
#include "timer.h"

extern char *optarg;
extern int optind;

static volatile int reload;

static void usage(const char *progname);
static void hup_handler();

int main(int argc, char **argv)
{
  struct timeval tv, *tvp;
  struct sockaddr_in sin;
  struct sigaction action;
  struct servent *servent;
  struct serverstate state;
  struct printer *printer;
  unsigned short port;
  pid_t pid;
  fd_set readers, writers;
  int c, nofork = 0, fd, i, nfds, count, status;
  const char *progname;
  char *errmem;
  FILE *fp;

  progname = strrchr(argv[0], '/');
  progname = (progname) ? progname + 1 : argv[0];

  state.configfile = LARVNET_PATH_CONFIG;
  while ((c = getopt(argc, argv, "nf:")) != -1)
    {
      switch (c)
	{
	case 'n':
	  nofork = 1;
	  break;
	case 'f':
	  state.configfile = optarg;
	  break;
	default:
	  usage(progname);
	}
    }
  if (argc != optind)
    usage(progname);

  /* Bind to the server socket. */
  servent = getservbyname("larvnet", "udp");
  port = (servent) ? servent->s_port : htons(LARVNET_FALLBACK_PORT);
  state.server_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (state.server_socket == -1)
    {
      fprintf(stderr, "%s: socket: %s\n", progname, strerror(errno));
      return 1;
    }
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = port;
  if (bind(state.server_socket, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
      fprintf(stderr, "%s: bind: %s\n", progname, strerror(errno));
      return 1;
    }

  /* Initialize a resolver channel. */
  status = ares_init(&state.channel);
  if (status != ARES_SUCCESS)
    {
      fprintf(stderr, "Can't initialize resolver: %s\n",
	      ares_strerror(status, &errmem));
      ares_free_errmem(errmem);
      return 1;
    }

  /* Initialize a Hesiod context. */
  if (hesiod_init(&state.hescontext) == -1)
    {
      fprintf(stderr, "Can't initialize Hesiod context: %s\n",
	      strerror(errno));
      return 1;
    }

  /* Determine the port for busy status polls. */
  servent = getservbyname("busypoll", "udp");
  state.poll_port = (servent) ? servent->s_port
      : htons(BUSYPOLL_FALLBACK_PORT);

  if (!nofork)
    {
      /* fork, dissociate from our tty, chdir to /, repoint
       * stdin/stdout/stderr at /dev/null.  Standard daemon stuff.
       */
      pid = fork();
      if (pid == -1)
	{
	  fprintf(stderr, "%s: fork: %s", progname, strerror(errno));
	  return 1;
	}
      if (pid != 0)
	_exit(0);
      if (setsid() == -1)
	{
	  fprintf(stderr, "%s: setsid: %s", progname, strerror(errno));
	  return 1;
	}
      chdir("/");
      fd = open("/dev/null", O_RDWR, 0);
      if (fd != -1)
	{
	  dup2(fd, STDIN_FILENO);
	  dup2(fd, STDOUT_FILENO);
	  dup2(fd, STDERR_FILENO);
	  if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
	    close(fd);
	}

      /* Write a pid file. */
      fp = fopen(LARVNET_PATH_PIDFILE, "w");
      if (fp)
	{
	  fprintf(fp, "%lu\n", (unsigned long) getpid());
	  fclose(fp);
	}
    }

  /* Set signal handlers: reread the configuration file on HUP, and
   * ignore SIGPIPE because every robust network program's gotta.
   */
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  action.sa_handler = hup_handler;
  sigaction(SIGHUP, &action, NULL);
  action.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &action, NULL);

  /* Start syslogging. */
  openlog(progname, LOG_PID, LOG_LOCAL5);

  read_initial_config(&state);

  /* Start initial polls of workstations and printers, and start the
   * report generation timer. */
  state.startmachine = 0;
  ws_poll(&state);
  printer_start_polls(&state);
  timer_set_rel(60, report, &state.config);

  while (1)
    {
      /* See if we got a SIGHUP and need to reread the config file. */
      if (reload)
	{
	  reload = 0;
	  reread_config(&state);
	  printer_start_polls(&state);
	}

      /* Set up fd sets for select. */
      FD_ZERO(&readers);
      FD_ZERO(&writers);
      nfds = ares_fds(state.channel, &readers, &writers);
      FD_SET(state.server_socket, &readers);
      if (state.server_socket >= nfds)
	nfds = state.server_socket + 1;
      for (i = 0; i < state.config.nprinters; i++)
	{
	  printer = &state.config.printers[i];
	  if (printer->s == -1)
	    continue;
	  if (printer->to_send)
	    FD_SET(printer->s, &writers);
	  else
	    FD_SET(printer->s, &readers);
	  if (printer->s >= nfds)
	    nfds = printer->s + 1;
	}

      /* Find the minimum of the ares and timer timeouts. */
      tvp = timer_timeout(&tv);
      tvp = ares_timeout(state.channel, tvp, &tv);

      if (tvp)
	{
	  syslog(LOG_DEBUG, "main: select timeout %lu %lu",
		 (unsigned long) tvp->tv_sec, (unsigned long) tvp->tv_usec);
	}
      else
	syslog(LOG_DEBUG, "main: no select timeout");

      /* Wait for network input or a timer event. */
      count = select(nfds, &readers, &writers, NULL, tvp);
      if (count < 0 && errno != EINTR)
	{
	  syslog(LOG_ALERT, "main: aborting on select error: %m");
	  return 1;
	}

      syslog(LOG_DEBUG, "main: select count %d", count);

      ares_process(state.channel, &readers, &writers);
      timer_process();

      /* Don't bother scanning fd sets if nothing happened. */
      if (count <= 0)
	continue;

      if (FD_ISSET(state.server_socket, &readers))
	ws_handle_status(state.server_socket, &state.config);
      for (i = 0; i < state.config.nprinters; i++)
	{
	  printer = &state.config.printers[i];
	  if (printer->s == -1)
	    continue;
	  if (FD_ISSET(printer->s, &readers))
	    printer_handle_input(&state, printer);
	  else if (FD_ISSET(printer->s, &writers))
	    printer_handle_output(&state, printer);
	}
    }
}

static void usage(const char *progname)
{
  fprintf(stderr, "Usage: %s [-n] [-f configfile]\n", progname);
  exit(1);
}

static void hup_handler(void)
{
  reload = 1;
}
