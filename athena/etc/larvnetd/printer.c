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
 * functions for querying the printers for queue status information.
 */

static const char rcsid[] = "$Id: printer.c,v 1.6 2000-01-05 16:29:15 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <hesiod.h>
#include "larvnetd.h"
#include "timer.h"

#define PRINTER_FALLBACK_PORT 515

struct printer_poll_args {
  struct serverstate *state;
  struct printer *printer;
};

static void printer_poll(void *arg);
static void printer_hes_callback(void *arg, int status, unsigned char *abuf,
				 int alen);
static void printer_host_callback(void *arg, int status, struct hostent *host);
static void *make_pargs(struct serverstate *state, struct printer *printer);

void printer_start_polls(struct serverstate *state)
{
  int i;

  syslog(LOG_DEBUG, "printer_start_polls");
  for (i = 0; i < state->config.nprinters; i++)
    printer_poll(make_pargs(state, &state->config.printers[i]));
}

static void printer_poll(void *arg)
{
  struct printer_poll_args *pargs = (struct printer_poll_args *) arg;
  struct serverstate *state = pargs->state;
  struct printer *printer = pargs->printer;
  char *hesname;

  /* Null out the timer, since it may have just gone off. */
  printer->timer = NULL;
  hesname = hesiod_to_bind(state->hescontext, printer->name, "pcap");
  if (hesname == NULL)
    {
      syslog(LOG_ERR, "printer_poll: can't convert printer name %s to "
	     "hesiod name: %m", printer->name);
      printer->timer = timer_set_rel(60, printer_poll, pargs);
      return;
    }

  syslog(LOG_DEBUG, "printer_poll: printer %s starting query for %s",
	 printer->name, hesname);
  ares_query(state->channel, hesname, C_IN, T_TXT, printer_hes_callback,
	     pargs);
  hesiod_free_string(hesname);
}

static void printer_hes_callback(void *arg, int status, unsigned char *abuf,
				 int alen)
{
  struct printer_poll_args *pargs = (struct printer_poll_args *) arg;
  struct serverstate *state = pargs->state;
  struct printer *printer = pargs->printer;
  char **vec = NULL, *p, *q, *errmem;

  if (status == ARES_EDESTRUCTION)
    {
      syslog(LOG_DEBUG, "printer_hes_callback: printer %s hesiod query "
	     "halted for channel destruction", printer->name);
      free(pargs);
      return;
    }

  if (status != ARES_SUCCESS)
    {
      syslog(LOG_ERR, "printer_hes_callback: could not resolve Hesiod pcap "
	     "for printer %s: %s", printer->name,
	     ares_strerror(status, &errmem));
      ares_free_errmem(errmem);
      goto failure;
    }

  /* Parse the result buffer into text records. */
  vec = hesiod_parse_result(state->hescontext, abuf, alen);
  if (!vec || !*vec)
    {
      syslog(LOG_ERR, "printer_hes_callback: could not parse Hesiod pcap "
	     "result for printer %s: %m", printer->name);
      goto failure;
    }

  /* Look for the print server name. */
  p = strchr(*vec, ':');
  while (p)
    {
      if (strncmp(p + 1, "rm=", 3) == 0)
	break;
      p = strchr(p + 1, ':');
    }
  if (!p)
    {
      syslog(LOG_ERR, "printer_hes_callback: can't find print server name in "
	     "Hesiod pcap result for printer %s", printer->name);
      goto failure;
    }
  p += 4;
  q = p;
  while (*q && *q != ':')
    q++;
  *q = 0;

  syslog(LOG_DEBUG, "printer_hes_callback: printer %s starting query for %s",
	 printer->name, p);
  ares_gethostbyname(state->channel, p, AF_INET, printer_host_callback, pargs);
  hesiod_free_list(state->hescontext, vec);
  return;

failure:
  if (vec)
    hesiod_free_list(state->hescontext, vec);
  printer->timer = timer_set_rel(60, printer_poll, pargs);
}

static void printer_host_callback(void *arg, int status, struct hostent *host)
{
  struct printer_poll_args *pargs = (struct printer_poll_args *) arg;
  struct printer *printer = pargs->printer;
  int s = -1, lport = IPPORT_RESERVED - 1, flags;
  unsigned short port;
  struct servent *servent;
  struct sockaddr_in sin;
  char *errmem;

  if (status == ARES_EDESTRUCTION)
    {
      syslog(LOG_DEBUG, "printer_host_callback: printer %s hostname query "
	     "halted for channel destruction", printer->name);
      free(pargs);
      return;
    }

  if (status != ARES_SUCCESS)
    {
      syslog(LOG_ERR, "printer_host_callback: printer %s can't resolve print "
	     "server name: %s", printer->name, ares_strerror(status, &errmem));
      ares_free_errmem(errmem);
      goto failure;
    }

  s = rresvport(&lport);
  if (s < 0)
    {
      syslog(LOG_ERR, "printer_host_callback: printer %s can't get reserved "
	     "port", printer->name);
      goto failure;
    }

  /* Set s non-blocking so we can do a non-blocking connect. */
  flags = fcntl(s, F_GETFL);
  fcntl(s, F_SETFL, flags | O_NONBLOCK);

  servent = getservbyname("printer", "tcp");
  port = (servent) ? servent->s_port : htons(PRINTER_FALLBACK_PORT);

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, host->h_addr, sizeof(sin.sin_addr));
  sin.sin_port = port;
  if (connect(s, (struct sockaddr *) &sin, sizeof(sin)) == -1
      && errno != EINPROGRESS)
    {
      syslog(LOG_ERR, "printer_host_callback: printer %s can't connect to "
	     "print server %s: %m", printer->name, host->h_name);
      goto failure;
    }

  /* Set up the request we want to send; the main loop will call
   * printer_handle_output() when the socket selects true for
   * writing.
   */
  printer->s = s;
  printer->to_send = 1;
  sprintf(printer->buf, "\3%.*s\n", (int)(sizeof(printer->buf) - 3),
	  printer->name);
  printer->buflen = strlen(printer->buf);
  printer->jobs_counted = 0;
  printer->up_so_far = 1;

  syslog(LOG_DEBUG, "printer_host_callback: printer %s queued %d-byte query",
	 printer->name, printer->buflen);

  free(pargs);
  return;

failure:
  if (s >= 0)
    close(s);
  printer->timer = timer_set_rel(60, printer_poll, pargs);
  return;
}

void printer_handle_output(struct serverstate *state, struct printer *printer)
{
  int count;

  count = write(printer->s, printer->buf, printer->buflen);
  if (count < 0)
    {
      syslog(LOG_ERR, "printer_handle_output: printer %s error writing to "
	     "print server: %m", printer->name);
      close(printer->s);
      printer->s = -1;
      printer->timer = timer_set_rel(60, printer_poll,
				     make_pargs(state, printer));
      return;
    }
  syslog(LOG_DEBUG, "printer_handle_output: printer %s wrote %d bytes",
	 printer->name, count);

  /* Slide the data to send back in the buffer. */
  printer->buflen -= count;
  memmove(printer->buf, printer->buf + count, printer->buflen);

  /* If we emptied the buffer, we're ready to receive. */
  if (printer->buflen == 0)
    {
      syslog(LOG_DEBUG, "printer_handle_output: printer %s receiving",
	     printer->name);
      printer->to_send = 0;
    }
}

void printer_handle_input(struct serverstate *state, struct printer *printer)
{
  int count;
  char *q;
  const char *p;

  /* Read in a chunk of data from the printer server. */
  count = read(printer->s, printer->buf + printer->buflen,
	       sizeof(printer->buf) - printer->buflen);
  if (count < 0)
    {
      syslog(LOG_ERR, "printer_handle_input: printer %s error reading from "
	     "print server: %m", printer->name);
    }
  if (count <= 0)
    {
      printer->jobs = printer->jobs_counted;
      printer->up = printer->up_so_far;
      close(printer->s);
      printer->s = -1;
      printer->timer = timer_set_rel(60, printer_poll,
				     make_pargs(state, printer));
      syslog(LOG_DEBUG, "printer_handle_input: printer %s ending query, "
	     "jobs %d up %d", printer->name, printer->jobs, printer->up);
      return;
    }
  printer->buflen += count;
  syslog(LOG_DEBUG, "printer_handle_input: printer %s read %d bytes",
	 printer->name, count);

  /* Now look over any complete lines we have. */
  p = printer->buf;
  q = memchr(p, '\n', printer->buflen);
  while (q)
    {
      *q = 0;
      syslog(LOG_DEBUG, "printer_handle_input: printer %s line: %s",
	     printer->name, p);
      if (strncmp(p, "active ", 7) == 0 || isdigit((unsigned char)*p))
	printer->jobs_counted++;
      else if (strstr(p, "is down") || strstr(p, "Printer Error") ||
	       strstr(p, "ing disabled"))
	printer->up_so_far = 0;
      p = q + 1;
      q = memchr(p, '\n', printer->buf + printer->buflen - p);
    }

  /* Copy any leftover partial line back into printer->buf. */
  printer->buflen -= (p - printer->buf);
  memmove(printer->buf, p, printer->buflen);

  /* If the buffer is full and we don't have a complete line, toss the
   * buffer so we can make some forward progress.  We don't expect
   * this to happen when the print server is functioning correctly.
   */
  if (printer->buflen == sizeof(printer->buf))
    {
      syslog(LOG_NOTICE, "printer_handle_input: printer %s input buffer "
	     "full, flushing", printer->name);
      printer->buflen = 0;
    }
}

static void *make_pargs(struct serverstate *state, struct printer *printer)
{
  struct printer_poll_args *pargs;

  pargs = emalloc(sizeof(struct printer_poll_args));
  pargs->state = state;
  pargs->printer = printer;
  return pargs;
}
