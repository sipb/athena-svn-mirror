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
 * functions to poll and receive notifications of workstation status.
 */

static const char rcsid[] = "$Id: ws.c,v 1.2 1998-10-13 17:12:59 ghudson Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <netdb.h>
#include "larvnetd.h"
#include "larvnet.h"
#include "timer.h"

/* Begin polling this many seconds after the last status notification. */
#define POLLINT		30 * 60

/* After this many polls have gone unanswered, decide that we don't know
 * what a machine's status is.
 */
#define POLLTRIES	8

/* After sending this many polls, reschedule for one second later and
 * stop scanning machines.
 */
#define POLLMAX		20

/* Retry schedule, in seconds between tries */
static int schedule[] = { 0, 5, 5, 10, 10, 30, 30, 60, 60, 5 * 60, 5 * 60,
			  10 * 60, 10 * 60, 30 * 60, 30 * 60, 60 * 60,
			  60 * 60 };

struct wsarg {
  struct serverstate *state;
  struct machine *machine;
};

static void poll_callback(void *arg, int status, struct hostent *host);
static int ws_searchcomp(const void *key, const void *elem);
static int ws_sortcomp(const void *elem1, const void *elem2);

void ws_poll(void *arg)
{
    struct serverstate *state = (struct serverstate *) arg;
    struct config *config = &state->config;
    struct machine *machine;
    struct wsarg *wsarg;
    int i, polls_sent = 0, nexttimeout = POLLINT, nextpoll;
    time_t now;

    syslog(LOG_DEBUG, "ws_poll: startmachine %d", state->startmachine);

    time(&now);
    for (i = state->startmachine; i < config->nmachines; i++)
      {
	machine = &config->machines[i];
	if (now - machine->laststatus >= POLLINT
	    && now - machine->lastpoll >= schedule[machine->numpolls])
	  {
	    syslog(LOG_DEBUG, "ws_poll: machine %s laststatus %d lastpoll "
		   "%d numpolls %d", machine->name, machine->laststatus,
		   machine->lastpoll, machine->numpolls);

	    /* Resolve the machine name so that we can send a poll. */
	    wsarg = (struct wsarg *) emalloc(sizeof(struct wsarg));
	    wsarg->state = state;
	    wsarg->machine = machine;
	    ares_gethostbyname(state->channel, machine->name, AF_INET,
			       poll_callback, wsarg);

	    /* If we've already had POLLTRIES polls go unanswered, decide
	     * we don't know if the machine is free or not any more.
	     */
	    if (machine->numpolls == POLLTRIES)
	      {
		syslog(LOG_DEBUG, "ws_poll: machine %s busy state set to "
		       "unknown", machine->name);
		machine->busy = UNKNOWN_BUSYSTATE;
	      }

	    /* Update the number of polls sent and the time of the last
	     * poll. */
	    if (machine->numpolls < (sizeof(schedule) / sizeof(int)) - 1)
	      machine->numpolls++;
	    machine->lastpoll = now;

	    polls_sent++;
	    if (polls_sent >= POLLMAX)
	      {
		syslog(LOG_DEBUG, "ws_poll: hit POLLMAX, stalling");
		nexttimeout = 1;
		state->startmachine = i + 1;
		break;
	      }
	  }

	/* Compute the time to the next poll. */
	if (now - machine->laststatus < POLLINT)
	  nextpoll = machine->laststatus + POLLINT - now;
	else
	  nextpoll = machine->lastpoll + schedule[machine->numpolls] - now;

	if (nexttimeout > nextpoll)
	  nexttimeout = nextpoll;
      }

    if (i == config->nmachines)
      state->startmachine = 0;
    syslog(LOG_DEBUG, "ws_poll: rescheduling for %d seconds with start "
	   "machine %d", nexttimeout, state->startmachine);
    timer_set_rel(nexttimeout, ws_poll, state);
}

void ws_handle_status(int s, struct config *config)
{
  struct sockaddr_in sin;
  int sz = sizeof(sin), count;
  char buf[LARVNET_MAX_PACKET + 1];
  const char *name, *arch;
  struct machine *machine;
  enum busystate busystate;

  /* Read a packet from the server socket. */
  count = recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr *) &sin, &sz);
  if (count == -1)
    {
      syslog(LOG_ERR, "ws_handle_status: recvfrom: %m");
      return;
    }
  if (count == 0)
    {
      syslog(LOG_NOTICE, "ws_handle_status: empty packet from %s",
	     inet_ntoa(sin.sin_addr));
      return;
    }

  /* Pull the busy state, name, and arch name out of the packet. */
  buf[count] = 0;
  busystate = (buf[0] == '1') ? BUSY : (buf[0] == '0') ? FREE
    : UNKNOWN_BUSYSTATE;
  name = buf + 1;
  arch = name + strlen(name) + 1;
  if (arch >= buf + count)
    {
      syslog(LOG_NOTICE, "ws_handle_status: invalid packet from %s",
	     inet_ntoa(sin.sin_addr));
      return;
    }

  syslog(LOG_DEBUG, "ws_handle_status: addr %s busy %c name %s arch %s",
	 inet_ntoa(sin.sin_addr), buf[0], name, arch);

  /* We could, at this point, resolve the given name and see if
   * sin.sin_addr is one of its interface addresses.  That would make
   * it a little harder to spoof, but not much, so we won't bother.
   */

  machine = ws_find(config, name);
  if (machine)
    {
      syslog(LOG_DEBUG, "ws_handle_status: machine %s set to state %s arch %s",
	     machine->name, (busystate == BUSY) ? "busy"
	     : (busystate == FREE) ? "free" : "unknown", arch);
      machine->busy = busystate;
      if (machine->arch)
	free(machine->arch);
      machine->arch = estrdup(arch);
      time(&machine->laststatus);
      machine->numpolls = 0;
    }
}

struct machine *ws_find(struct config *config, const char *name)
{
  return bsearch(name, config->machines, config->nmachines,
		 sizeof(struct machine), ws_searchcomp);
}

void ws_sort(struct config *config)
{
  qsort(config->machines, config->nmachines, sizeof(struct machine),
	ws_sortcomp);
}

static void poll_callback(void *arg, int status, struct hostent *host)
{
  struct wsarg *wsarg = (struct wsarg *) arg;
  struct serverstate *state = wsarg->state;
  struct machine *machine = wsarg->machine;
  struct sockaddr_in sin;
  char dummy = 0, *errmem;

  free(wsarg);
  if (status != ARES_SUCCESS)
    {
      if (status == ARES_EDESTRUCTION)
	{
	  syslog(LOG_DEBUG, "poll_callback: query for %s halted for channel "
		 "destruction", machine->name);
	}
      else
	{
	  syslog(LOG_ERR, "poll_callback: could not resolve ws name %s: %s",
		 machine->name, ares_strerror(status, &errmem));
	  ares_free_errmem(errmem);
	}
      return;
    }
  /* Send a poll. */
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  memcpy(&sin.sin_addr, host->h_addr, sizeof(sin.sin_addr));
  sin.sin_port = state->poll_port;
  sendto(state->server_socket, &dummy, 1, 0,
	 (struct sockaddr *) &sin, sizeof(sin));

  syslog(LOG_DEBUG, "poll_callback: query for ws name %s yielded %s; poll "
	 "sent", machine->name, inet_ntoa(sin.sin_addr));
}

static int ws_searchcomp(const void *key, const void *elem)
{
  const char *s = (const char *) key;
  const struct machine *m = (const struct machine *) elem;

  return strcmp(s, m->name);
}

static int ws_sortcomp(const void *elem1, const void *elem2)
{
  const struct machine *m1 = (const struct machine *) elem1;
  const struct machine *m2 = (const struct machine *) elem2;

  return strcmp(m1->name, m2->name);
}
