/* $Id: larvnetd.h,v 1.1 1998-09-01 20:57:45 ghudson Exp $ */

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

#ifndef LARVNETD__H
#define LARVNETD__H

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <ares.h>
#include "timer.h"

#define UNKNOWN_ARCH	-1
#define OTHER_ARCH	-2

enum busystate { BUSY, FREE, UNKNOWN_BUSYSTATE };

struct cluster {
  char *name;
  char *phone;
  int cgroup;			/* Index into config.cgroups, or -1 */
};

struct archname {
  char *netname;
  char *reportname;
};

struct machine {
  /* Configuration data */
  char *name;
  int cluster;			/* Index into config.clusters */

  /* State information */
  enum busystate busy;
  int arch;			/* Index into config.arches */
  time_t laststatus;		/* Time of last status report */
  time_t lastpoll;		/* Time of last poll we sent */
  int numpolls;			/* Number of polls since laststatus */
};

struct printer {
  /* Configuration data */
  char *name;
  int cluster;			/* Index into config.clusters */

  /* State information */
  int up;
  int jobs;
  int s;			/* Communications socket for polling */
  int to_send;			/* Non-zero if we want to send data */
  char buf[BUFSIZ];		/* Data to send or data received */
  int buflen;			/* Amount of data in buf */
  int jobs_counted;		/* Jobs counted from data received so far */
  int up_so_far;		/* 1 until we find out the printer is down */
  Timer *timer;			/* Timer for next poll (when s == -1) */
};

struct cgroup {
  char *name;
  int x;
  int y;
};

struct config {
  struct cluster *clusters;
  int nclusters;
  struct archname *arches;
  int narch;
  struct machine *machines;	/* Sorted by name */
  int nmachines;
  struct printer *printers;	
  int nprinters;
  struct cgroup *cgroups;
  int ncgroups;
  char *report_other;
  char *report_unknown;
};

struct serverstate {
  struct config config;
  ares_channel channel;
  void *hescontext;
  int server_socket;
  unsigned short poll_port;
  int startmachine;
  const char *configfile;
};

/* config.c */
void read_initial_config(struct serverstate *state);
void reread_config(struct serverstate *state);

/* printer.c */
void printer_start_polls(struct serverstate *state);
void printer_handle_output(struct serverstate *state, struct printer *printer);
void printer_handle_input(struct serverstate *state, struct printer *printer);

/* report.c */
void report(void *arg);

/* util.c */
void *emalloc(size_t size);
void *erealloc(void *ptr, size_t size);
char *estrdup(const char *s);
char *estrndup(const char *s, size_t n);
int read_line(FILE *fp, char **buf, int *bufsize);

/* ws.c */
void ws_poll(void *arg);
void ws_handle_status(int s, struct config *config);
struct machine *ws_find(struct config *config, const char *name);
void ws_sort(struct config *config);

#endif /* LARVNETD__H */
