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
 * functions to read and reread the configuration file.
 */

static const char rcsid[] = "$Id: config.c,v 1.4 1999-10-19 20:23:31 danw Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <hesiod.h>
#include "larvnetd.h"
#include "timer.h"

static int read_config(const char *configfile, struct config *config);
static int find_numeric_range(const char *name, const char **start,
			      const char **end, int *first, int *last);
static void add_machine(struct config *config, int *machsize, const char *name,
			int cluster_index);
static void freeconfig(struct config *config);
static char *skip_spaces(char *p);
static char *skip_nonspaces(char *p);

void read_initial_config(struct serverstate *state)
{
  if (read_config(state->configfile, &state->config) == -1)
    {
      syslog(LOG_ALERT, "read_initial_config: can't read initial "
	     "configuration, aborting");
      exit(1);
    }
}

void reread_config(struct serverstate *state)
{
  struct config newconf;
  struct machine *machine;
  struct printer *printer;
  int i, j, status;
  char *errmem;

  syslog(LOG_DEBUG, "reread_config");

  /* Read from configfile into a new config structure. */
  if (read_config(state->configfile, &newconf) == -1)
    return;

  /* Copy over workstation state to newconf. */
  for (i = 0; i < state->config.nmachines; i++)
    {
      machine = ws_find(&newconf, state->config.machines[i].name);
      if (machine)
	{
	  syslog(LOG_DEBUG, "reread_config: ws %s copying state",
		 machine->name);
	  machine->busy = state->config.machines[i].busy;
	  machine->laststatus = state->config.machines[i].laststatus;
	  machine->lastpoll = state->config.machines[i].lastpoll;
	  machine->numpolls = state->config.machines[i].numpolls;
	  if (state->config.machines[i].arch)
	    machine->arch = estrdup(state->config.machines[i].arch);
	  else
	    machine->arch = NULL;
	}
    }

  /* Copy over printer state to newconf. */
  for (i = 0; i < state->config.nprinters; i++)
    {
      for (j = 0; j < newconf.nprinters; j++)
	{
	  printer = &newconf.printers[j];
	  if (strcmp(state->config.printers[i].name, printer->name) == 0)
	    {
	      syslog(LOG_DEBUG, "reread_config: printer %s copying state",
		     printer->name);
	      printer->up = state->config.printers[i].up;
	      printer->jobs = state->config.printers[i].jobs;
	    }
	}
    }

  /* Recreate the resolver channel, since any pending queries have
   * invalid pointers as their arguments.
   */
  ares_destroy(state->channel);
  status = ares_init(&state->channel);
  if (status != ARES_SUCCESS)
    {
      syslog(LOG_ALERT, "reread_config: can't reinitialize resolver channel, "
	     "aborting: %s", ares_strerror(status, &errmem));
      ares_free_errmem(errmem);
      exit(1);
    }

  /* Now free the old config structure and copy the new one in. */
  freeconfig(&state->config);
  state->config = newconf;
}

static int read_config(const char *configfile, struct config *config)
{
  FILE *fp;
  char *line = NULL, *p, *q, *name;
  const char *range_start, *range_end;
  int i, linesize, status, clsize, archsize, machsize, prsize, cgsize;
  int cluster_index, retval = -1, range_first, range_last;
  struct archname *arch;
  struct cluster *cluster;
  struct printer *printer;
  struct cgroup *cgroup;

  fp = fopen(configfile, "r");
  if (!fp)
    {
      syslog(LOG_ERR, "read_config: can't open config file %s: %m",
	     configfile);
      return -1;
    }

  /* Initialize the configuration lists. */
  clsize = archsize = machsize = prsize = cgsize = 10;
  config->clusters = emalloc(clsize * sizeof(struct cluster));
  config->arches = emalloc(archsize * sizeof(struct archname));
  config->machines = emalloc(machsize * sizeof(struct machine));
  config->printers = emalloc(prsize * sizeof(struct printer));
  config->cgroups = emalloc(cgsize * sizeof(struct cgroup));
  config->nclusters = config->narch = config->nmachines = 0;
  config->nprinters = config->ncgroups = 0;
  config->report_other = NULL;
  config->report_unknown = NULL;

  /* Read in the architecture order.  Each line is an architecture
   * name followed by an optional report name; a line beginning with
   * a hyphen terminates the list.
   */
  while ((status = read_line(fp, &line, &linesize)) == 0)
    {
      p = skip_spaces(line);
      q = skip_nonspaces(p);
      if (*p == '#' || !*p)
	continue;
      if (*p == '-')
	break;
      if (config->narch == archsize)
	{
	  archsize *= 2;
	  config->arches = erealloc(config->arches,
				    archsize * sizeof(struct archname));
	}
      arch = &config->arches[config->narch];
      arch->reportname = estrndup(p, q - p);

      /* Read in the net names for this architecture type. */
      arch->netnames = NULL;
      arch->nnetnames = 0;
      p = skip_spaces(q);
      while (*p)
	{
	  q = skip_nonspaces(p);
	  arch->netnames = erealloc(arch->netnames,
				    (arch->nnetnames + 1) * sizeof(char *));
	  arch->netnames[arch->nnetnames] = estrndup(p, q - p);
	  arch->nnetnames++;
	  p = skip_spaces(q);
	}

      /* If no net names were specified, default to the report name. */
      if (!arch->nnetnames)
	{
	  arch->netnames = emalloc(sizeof(char *));
	  arch->netnames[0] = estrdup(arch->reportname);
	  arch->nnetnames = 1;
	}

      config->narch++;
    }
  if (status == -1)
    {
      syslog(LOG_ERR, "read_config: error reading config file %s: %m",
	     configfile);
      goto cleanup;
    }

  /* Read in the clusters.  Possible line formats are:
   *
   *	cluster <name> <phone>
   *	printer <name>
   *	ws <hostname>
   *	cgroup <name> <x> <y> <cluster> ...
   *    option report-other <name>
   *    option report-unknown <name>
   *
   * A ws or printer line must come after a cluster line.
   */
  cluster_index = -1;
  while ((status = read_line(fp, &line, &linesize)) == 0)
    {
      /* Chop off the line at the comment delimiter, if any, and
       * ignore leading whitespace.  If the line is empty except for
       * whitespace and/or comments, ignore it.  Also eliminate
       * trailing whitespace.
       */
      p = strchr(line, '#');
      if (p)
	*p = 0;
      p = skip_spaces(line);
      if (!*p)
	continue;
      q = p + strlen(p) - 1;
      while (q > p && isspace((unsigned char)*q))
	*q-- = 0;

      if (strncmp(p, "cluster", 7) == 0 && isspace((unsigned char)p[7]))
	{
	  if (config->nclusters == clsize)
	    {
	      clsize *= 2;
	      config->clusters = erealloc(config->clusters,
					  clsize * sizeof(struct cluster));
	    }
	  cluster = &config->clusters[config->nclusters];

	  /* Get the cluster name. */
	  p = skip_spaces(p + 8);
	  q = skip_nonspaces(p);
	  cluster->name = estrndup(p, q - p);

	  /* Read the cluster phone number. */
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cluster->phone = estrndup(p, q - p);

	  /* Put the cluster in no cgroup to start with. */
	  cluster->cgroup = -1;

	  cluster_index = config->nclusters;
	  config->nclusters++;
	}
      else if (strncmp(p, "printer", 7) == 0 && isspace((unsigned char)p[7]))
	{
	  if (cluster_index == -1)
	    {
	      syslog(LOG_ERR, "read_config: printer before cluster in %s: %s",
		     configfile, line);
	      goto cleanup;
	    }
	  if (config->nprinters == prsize)
	    {
	      prsize *= 2;
	      config->printers = erealloc(config->printers,
					  prsize * sizeof(struct printer));
	    }
	  printer = &config->printers[config->nprinters];
	  p = skip_spaces(p + 7);
	  q = skip_nonspaces(p);
	  printer->name = estrndup(p, q - p);
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  printer->location = (*p) ? estrndup(p, q - p)
	      : estrdup(config->clusters[cluster_index].name);
	  printer->cluster = cluster_index;

	  /* Initialize state variables. */
	  printer->up = 0;
	  printer->jobs = 0;
	  printer->s = -1;
	  printer->timer = NULL;

	  config->nprinters++;
	}
      else if (strncmp(p, "ws", 2) == 0 && isspace((unsigned char)p[2]))
	{
	  if (cluster_index == -1)
	    {
	      syslog(LOG_ERR, "read_config: workstation before cluster in "
		     "%s: %s", configfile, line);
	      goto cleanup;
	    }

	  p = skip_spaces(p + 2);
	  q = skip_nonspaces(p);
	  *q = 0;

	  if (find_numeric_range(p, &range_start, &range_end, &range_first,
				 &range_last))
	    {
	      name = emalloc(strlen(p) + 100);
	      memcpy(name, p, range_start - p);
	      q = name + (range_start - p);
	      for (i = range_first; i <= range_last; i++)
		{
		  sprintf(q, "%d%s", i, range_end);
		  add_machine(config, &machsize, name, cluster_index);
		}
	      free(name);
	    }
	  else
	    add_machine(config, &machsize, p, cluster_index);
	}
      else if (strncmp(p, "cgroup", 6) == 0 && isspace((unsigned char)p[6]))
	{
	  if (config->ncgroups == cgsize)
	    {
	      cgsize *= 2;
	      config->cgroups = erealloc(config->cgroups,
					 cgsize * sizeof(struct cgroup));
	    }
	  cgroup = &config->cgroups[config->ncgroups];
	  p = skip_spaces(p + 6);
	  q = skip_nonspaces(p);
	  cgroup->name = estrndup(p, q - p);
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cgroup->x = atoi(p);
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cgroup->y = atoi(p);
	  config->ncgroups++;
	  while (*q)
	    {
	      p = skip_spaces(q);
	      q = skip_nonspaces(p);
	      for (i = 0; i < config->nclusters; i++)
		{
		  if (strlen(config->clusters[i].name) == q - p
		      && strncmp(config->clusters[i].name, p, q - p) == 0)
		    break;
		}
	      if (i == config->nclusters)
		{
		  syslog(LOG_ERR, "read_config: unknown cluster name %.*s in "
			 "%s: %s", q - p, p, configfile, line);
		  goto cleanup;
		}
	      if (config->clusters[i].cgroup != -1)
		{
		  syslog(LOG_ERR, "read_config: cluster %.*s already in "
			 "cluster group in %s: %s", q - p, p, configfile,
			 line);
		  goto cleanup;
		}
	      config->clusters[i].cgroup = config->ncgroups - 1;
	    }
	}
      else if (strncmp(p, "option", 6) == 0 && isspace((unsigned char)p[6]))
	{
	  p = skip_spaces(p + 6);
	  if (strncmp(p, "report-other", 12) == 0 &&
	      isspace((unsigned char)p[12]))
	    config->report_other = estrdup(skip_spaces(p + 12));
	  else if (strncmp(p, "report-unknown", 14) == 0 &&
		   isspace((unsigned char)p[14]))
	    config->report_unknown = estrdup(skip_spaces(p + 14));
	  else
	    {
	      syslog(LOG_ERR, "read_config: unrecognized option %s", p);
	      goto cleanup;
	    }
	}
      else
	{
	  syslog(LOG_ERR, "read_config: unrecognized line %s", line);
	  goto cleanup;
	}
    }
  if (status == -1)
    {
      syslog(LOG_ERR, "read_config: error reading config file %s: %m",
	     configfile);
      goto cleanup;
    }

  ws_sort(config);
  retval = 0;

cleanup:
  if (retval == -1)
    freeconfig(config);
  fclose(fp);
  free(line);
  return retval;
}

static int find_numeric_range(const char *name, const char **start,
			      const char **end, int *first, int *last)
{
  const char *p;

  /* Find the opening bracket. */
  p = strchr(name, '[');
  if (!p)
    return 0;
  *start = p;

  /* Read the range beginning. */
  p++;
  if (!isdigit((unsigned char)*p))
    return 0;
  *first = strtol(p, (char **) &p, 0);

  /* Skip the dash in the middle. */
  if (*p != '-')
    return 0;
  p++;

  /* Read the range end. */
  if (!isdigit((unsigned char)*p))
    return 0;
  *last = strtol(p, (char **) &p, 0);

  /* Make sure we close with a square bracket. */
  if (*p != ']')
    return 0;
  *end = p + 1;

  return 1;
}

static void add_machine(struct config *config, int *machsize, const char *name,
			int cluster_index)
{
  struct machine *machine;

  if (config->nmachines == *machsize)
    {
      *machsize *= 2;
      config->machines = erealloc(config->machines,
				  *machsize * sizeof(struct machine));
    }

  machine = &config->machines[config->nmachines];
  machine->name = estrdup(name);
  machine->cluster = cluster_index;

  /* Initialize state variables. */
  machine->busy = UNKNOWN_BUSYSTATE;
  machine->arch = NULL;
  machine->laststatus = 0;
  machine->lastpoll = 0;
  machine->numpolls = 0;

  config->nmachines++;
}

static void freeconfig(struct config *config)
{
  int i, j;

  for (i = 0; i < config->nmachines; i++)
    {
      free(config->machines[i].name);
      free(config->machines[i].arch);
    }
  for (i = 0; i < config->nprinters; i++)
    {
      free(config->printers[i].name);
      free(config->printers[i].location);
      if (config->printers[i].s != -1)
	close(config->printers[i].s);
      if (config->printers[i].timer)
	free(timer_reset(config->printers[i].timer));
    }
  for (i = 0; i < config->nclusters; i++)
    {
      free(config->clusters[i].name);
      free(config->clusters[i].phone);
    }
  for (i = 0; i < config->narch; i++)
    {
      free(config->arches[i].reportname);
      for (j = 0; j < config->arches[i].nnetnames; j++)
	free(config->arches[i].netnames[j]);
      free(config->arches[i].netnames);
    }
  for (i = 0; i < config->ncgroups; i++)
    free(config->cgroups[i].name);
  free(config->clusters);
  free(config->arches);
  free(config->machines);
  free(config->printers);
  free(config->cgroups);
  free(config->report_other);
  free(config->report_unknown);
}

static char *skip_spaces(char *p)
{
  while (isspace((unsigned char)*p))
    p++;
  return p;
}

static char *skip_nonspaces(char *p)
{
  while (*p && !isspace((unsigned char)*p))
    p++;
  return p;
}
