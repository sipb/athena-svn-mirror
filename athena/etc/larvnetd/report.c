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
 * the generation of reports.
 */

static const char rcsid[] = "$Id: report.c,v 1.2 1998-10-13 17:12:59 ghudson Exp $";

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include "larvnetd.h"
#include "larvnet.h"
#include "timer.h"

#define PATH_CGROUPS_NEW	LARVNET_PATH_CGROUPS ".new"
#define PATH_CLUSTERS_NEW	LARVNET_PATH_CLUSTERS ".new"
#define PATH_PRINTERS_NEW	LARVNET_PATH_PRINTERS ".new"

struct outof {
  int nfree;
  int total;
};

static void report_cgroups(struct config *config);
static void report_clusters(struct config *config);
static void report_printers(struct config *config);
static int find_arch(struct config *config, const char *archname);

void report(void *arg)
{
  struct config *config = (struct config *) arg;

  timer_set_rel(60, report, config);
  syslog(LOG_DEBUG, "report");

  report_cgroups(config);
  report_clusters(config);
  report_printers(config);
}

static void report_cgroups(struct config *config)
{
  FILE *fp;
  int i, j;
  struct cgroup *cgroup;
  struct cluster *cluster;
  struct printer *printer;

  fp = fopen(PATH_CGROUPS_NEW, "w");
  if (fp)
    {
      for (i = 0 ; i < config->ncgroups; i++)
	{
	  cgroup = &config->cgroups[i];
	  fprintf(fp, "%s %d %d", cgroup->name, cgroup->x, cgroup->y);
	  for (j = 0; j < config->nclusters; j++)
	    {
	      cluster = &config->clusters[j];
	      if (cluster->cgroup == i)
		fprintf(fp, " %s %s", cluster->name, cluster->phone);
	    }
	  fprintf(fp, " ---");
	  for (j = 0; j < config->nprinters; j++)
	    {
	      printer = &config->printers[j];
	      cluster = &config->clusters[printer->cluster];
	      if (cluster->cgroup == i)
		fprintf(fp, " %s", printer->name);
	    }
	  fprintf(fp, "\n");
	}
      fclose(fp);
      if (rename(PATH_CGROUPS_NEW, LARVNET_PATH_CGROUPS) == -1)
	{
	  syslog(LOG_ERR, "report: can't replace %s with %s: %m",
		 LARVNET_PATH_CGROUPS, PATH_CGROUPS_NEW);
	}
    }
  else
    {
      syslog(LOG_ERR, "report: can't open %s for writing: %m",
	     PATH_CGROUPS_NEW);
    }
}

static void report_clusters(struct config *config)
{
  FILE *fp;
  int i, j;
  struct cluster *cluster;
  struct machine *machine;
  struct outof **entries, *ent;

  /* For each cluster and architecture, we need to compute the number
   * of free machines and total number of machines.  Allocate and
   * initialize a matrix of outof structures to store the data.
   * Leave an extra two entries at the end of each row for other and
   * unknown architectures.
   */
  entries = emalloc(config->nclusters * sizeof(struct outof *));
  for (i = 0; i < config->nclusters; i++)
    {
      entries[i] = emalloc((config->narch + 2) * sizeof(struct outof));
      for (j = 0; j < config->narch + 2; j++)
	{
	  entries[i][j].nfree = 0;
	  entries[i][j].total = 0;
	}
    }

  /* Now compute the free machines and total machines for each cluster
   * and architecture.
   */
  for (i = 0; i < config->nmachines; i++)
    {
      machine = &config->machines[i];
      ent = &entries[machine->cluster][find_arch(config, machine->arch)];
      if (machine->busy == FREE)
	ent->nfree++;
      ent->total++;
    }

  /* Write out the cluster status file. */
  fp = fopen(PATH_CLUSTERS_NEW, "w");
  if (fp)
    {
      /* Write out the architecture names and a divider. */
      for (i = 0; i < config->narch; i++)
	fprintf(fp, "%s\n", config->arches[i].reportname);
      if (config->report_other)
	fprintf(fp, "%s\n", config->report_other);
      if (config->report_unknown)
	fprintf(fp, "%s\n", config->report_unknown);
      fprintf(fp, "---\n");

      /* Write out the data for each cluster. */
      for (i = 0; i < config->nclusters; i++)
	{
	  /* Start with the cluster configuration information. */
	  cluster = &config->clusters[i];
	  fprintf(fp, "%s %s %s", cluster->name,
		  (cluster->cgroup == -1) ? "private" : "public",
		  cluster->phone);

	  /* Write out the totals for each defined architecture. */
	  for (j = 0; j < config->narch; j++)
	    fprintf(fp, " %d %d", entries[i][j].nfree, entries[i][j].total);
	  if (config->report_other)
	    {
	      fprintf(fp, " %d %d", entries[i][config->narch].nfree,
		      entries[i][config->narch].total);
	    }
	  if (config->report_unknown)
	    {
	      fprintf(fp, " %d %d", entries[i][config->narch + 1].nfree,
		      entries[i][config->narch + 1].total);
	    }
	  fprintf(fp, "\n");
	}
      fclose(fp);
      if (rename(PATH_CLUSTERS_NEW, LARVNET_PATH_CLUSTERS) == -1)
	{
	  syslog(LOG_ERR, "report: can't replace %s with %s: %m",
		 LARVNET_PATH_CLUSTERS, PATH_CLUSTERS_NEW);
	}
    }
  else
    {
      syslog(LOG_ERR, "report: can't open %s for writing: %m",
	     PATH_CLUSTERS_NEW);
    }

  /* Free the matrix we allocated above. */
  for (i = 0; i < config->nclusters; i++)
    free(entries[i]);
  free(entries);
}

static void report_printers(struct config *config)
{
  FILE *fp;
  int i;
  struct printer *printer;

  fp = fopen(PATH_PRINTERS_NEW, "w");
  if (fp)
    {
      for (i = 0; i < config->nprinters; i++)
	{
	  printer = &config->printers[i];
	  fprintf(fp, "%s %s %s %d\n", printer->name,
		  config->clusters[printer->cluster].name,
		  (printer->up) ? "up" : "down", printer->jobs);
	}
      fclose(fp);
      if (rename(PATH_PRINTERS_NEW, LARVNET_PATH_PRINTERS) == -1)
	{
	  syslog(LOG_ERR, "report: can't replace %s with %s: %m",
		 LARVNET_PATH_PRINTERS, PATH_PRINTERS_NEW);
	}
    }
  else
    {
      syslog(LOG_ERR, "report: can't open %s for writing: %m",
	     PATH_PRINTERS_NEW);
    }
}

static int find_arch(struct config *config, const char *archname)
{
  int i, j;
  struct archname *arch;

  /* If the workstation has not reported an architecture or reported
   * an empty string, return config->narch + 1 for "unknown".
   */
  if (!archname || !*archname)
    return config->narch + 1;

  /* Look for the name in the list of defined architectures. */
  for (i = 0; i < config->narch; i++)
    {
      arch = &config->arches[i];
      for (j = 0; j < arch->nnetnames; j++)
	{
	  if (strcmp(arch->netnames[j], archname) == 0)
	    return i;
	}
    }

  /* Return config->narch for "other". */
  return config->narch;
}
