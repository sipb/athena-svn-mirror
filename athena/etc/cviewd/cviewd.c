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

/* This program presents reports generated by larvnetd to stdout
 * according to a command line from stdin.
 */

static const char rcsid[] = "$Id: cviewd.c,v 1.6 2001-01-22 21:21:05 ghudson Exp $";

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "larvnet.h"

struct outof {
  int nfree;
  int total;
};

struct cluster {
  char *name;
  char *phone;
  int public;
  struct outof *stats;
};

struct clusterfile {
  char **archnames;
  int narch;
  struct cluster *clusters;
  int nclusters;
  time_t mtime;
};

struct printer {
  char *name;
  char *cluster;
  int up;
  int jobs;
};

struct printerfile {
  struct printer *printers;
  int nprinters;
  time_t mtime;
};

struct cgcluster {
  char *name;
  char *phone;
};

struct cgroup {
  char *name;
  int x;
  int y;
  struct cgcluster *clusters;
  int nclusters;
  char **printers;
  int nprinters;
};

struct cgroupfile {
  struct cgroup *cgroups;
  int ncgroups;
  time_t mtime;
};

static void display_printers(void);
static void display_phones(void);
static void display_cluster_ints(void);
static void display_ints(int limit);
static void display_cgroups(void);
static void display_clusters(const char *s);
static void display_help(void);
static void read_clusters(struct clusterfile *file);
static void read_printers(struct printerfile *file);
static void read_cgroups(struct cgroupfile *file);
static int first_field_matches(const char *s, const char *word);
static int cluster_matches(const char *s, struct cluster *cluster);
static int read_line(FILE *fp, char **buf, int *bufsize);
static const char *skip_spaces(const char *p);
static const char *skip_nonspaces(const char *p);
static void *emalloc(size_t size);
static void *erealloc(void *ptr, size_t size);
static char *estrndup(const char *s, size_t n);

int main(int argc, char **argv)
{
  char *line = NULL;
  const char *p;
  int linesize;

  if (read_line(stdin, &line, &linesize) != 0)
    return 1;

  p = skip_spaces(line);

  if (first_field_matches(p, "printers"))
    display_printers();
  else if (first_field_matches(p, "phones"))
    display_phones();
  else if (first_field_matches(p, "intsonlyplease"))
    display_cluster_ints();
  else if (first_field_matches(p, "intsonlyplease2"))
    display_ints(1);
  else if (first_field_matches(p, "intsonlyplease3"))
    display_ints(0);
  else if (first_field_matches(p, "configplease2")
	   || first_field_matches(p, "configplease3"))
    display_cgroups();
  else if (first_field_matches(p, "help")
	   || first_field_matches(p, "-h")
	   || first_field_matches(p, "--help"))
    display_help();
  else
    display_clusters(p);

  return 0;
}

/* Display printer status in a user-readable format. */
static void display_printers(void)
{
  struct printerfile file;
  struct printer *printer;
  int i;
  char *ct;

  read_printers(&file);
  ct = ctime(&file.mtime);
  ct[strlen(ct) - 1] = 0;

  printf("            --  Printer status as of %s:  --\n", ct);
  printf("PRINTER     CLUSTER  STATUS   JOBS          "
	 "PRINTER     CLUSTER  STATUS   JOBS\n");
  printf("--------------------------------------------"
	 "----------------------------------\n");
  for (i = 0; i < file.nprinters; i++)
    {
      printer = &file.printers[i];
      printf("%-11.11s %7.7s   %-7.7s %3d", printer->name, printer->cluster,
	     (printer->up) ? "up" : "down", printer->jobs);

      /* Drop to the next line or display separating spaces as appropriate. */
      if (i % 2)
	printf("\n");
      else
	printf("           ");
    }
  if (i % 2)
    printf("\n");
}

/* Display cluster phone numbers in a user-readable format. */
static void display_phones(void)
{
  struct clusterfile file;
  int i;

  read_clusters(&file);
  printf("CLUSTER   PHONE NUMBER\n");
  printf("----------------------\n");
  for (i = 0; i < file.nclusters; i++)
    {
      if (file.clusters[i].public)
	printf("%-8.8s  %s\n", file.clusters[i].name, file.clusters[i].phone);
    }
}

/* Display a list of free cluster machines in the format:
 *	<clustername> {<free> <busy>} ...
 * At most five columns of free and busy numbers are given.
 */
static void display_cluster_ints(void)
{
  struct clusterfile file;
  struct cluster *cluster;
  int i, j;

  read_clusters(&file);
  for (i = 0; i < file.nclusters; i++)
    {
      cluster = &file.clusters[i];
      printf("%-8.8s", cluster->name);
      for (j = 0; j < file.narch && j < 5; j++)
	{
	  printf((j == 0) ? " " : "   ");
	  printf("%3d %3d", cluster->stats[j].nfree,
		 cluster->stats[j].total - cluster->stats[j].nfree);
	}
      printf("\n");
    }
}

/* Display a list of free cluster machines and printer statuses
 * in the formats:
 *	cluster <clustername> {<free> <busy>} ...
 *	printer <printername> {up|down} <jobs>
 * If limit is set, display at most five column-pairs of free and
 * busy numbers.
 */
static void display_ints(int limit)
{
  struct clusterfile cfile;
  struct printerfile pfile;
  struct cluster *cluster;
  struct printer *printer;
  int i, j;

  read_clusters(&cfile);
  read_printers(&pfile);
  for (i = 0; i < cfile.nclusters; i++)
    {
      cluster = &cfile.clusters[i];
      printf("cluster %-8.8s", cluster->name);
      for (j = 0; j < cfile.narch && (!limit || j < 5); j++)
	{
	  printf((j == 0) ? " " : "   ");
	  printf("%3d %3d", cluster->stats[j].nfree,
		 cluster->stats[j].total - cluster->stats[j].nfree);
	}
      printf("\n");
    }
  for (i = 0; i < pfile.nprinters; i++)
    {
      printer = &pfile.printers[i];
      printf("printer %-11.11s    %-7.7s %3d\n", printer->name,
	     (printer->up) ? "up" : "down", printer->jobs);
    }
}

/* Display configuration information.  The first line returned
 * has the form:
 *	<number of architectures> <archname> ...
 * The remaining lines come in groups of four, with the formats:
 *	<number> <cluster group name>
 *	{<cluster> <phone number>} ... XXXXX
 *	<x coordinate> <y coordinate>
 *	<printer> ... XXXXX
 */
static void display_cgroups(void)
{
  struct clusterfile cfile;
  struct cgroupfile cgfile;
  struct cgroup *cgroup;
  int i, j;

  read_clusters(&cfile);
  read_cgroups(&cgfile);

  printf("%d", cfile.narch);
  for (i = 0; i < cfile.narch; i++)
    printf(" %s", cfile.archnames[i]);
  printf("\n");

  for (i = 0; i < cgfile.ncgroups; i++)
    {
      /* Line 1: cluster group number and name */
      cgroup = &cgfile.cgroups[i];
      printf("%d\t%s\n", i + 1, cgroup->name);

      /* Line 2: cluster names */
      for (j = 0; j < cgroup->nclusters; j++)
	printf("%s %s ", cgroup->clusters[j].name, cgroup->clusters[j].phone);
      printf("XXXXX\n");

      /* Line 3: X and Y coordinates */
      printf("%d\t%d\n", cgroup->x, cgroup->y);

      /* Line 4: printer names */
      for (j = 0; j < cgroup->nprinters; j++)
	printf("%s ", cgroup->printers[j]);
      printf("XXXXX\n");
    }
}

static void display_clusters(const char *s)
{
  struct clusterfile file;
  struct cluster *cluster;
  struct outof coltotals[6], rowtotal, *entry;
  int i, j;
  char *ct;

  read_clusters(&file);
  ct = ctime(&file.mtime);
  ct[strlen(ct) - 1] = 0;

  /* We can handle at most six architecture columns in 80 columns. */
  if (file.narch > 6)
    file.narch = 6;

  /* Display the first header line. */
  printf("             --  Cluster status as of %s:  --\n", ct);

  /* Display the architecture names. */
  printf("          ");
  for (i = 0; i < file.narch; i++)
    {
      printf("%-9.9s ", file.archnames[i]);
      coltotals[i].nfree = 0;
      coltotals[i].total = 0;
    }
  printf("TOTAL\n");

  /* Display the fre/tot column headers. */
  printf("CLUSTER");
  for (i = 0; i < file.narch + 1; i++)
    printf("   fre/tot");
  printf("\n");

  /* Display the divider line. */
  printf("--------");
  for (i = 0; i < file.narch + 1; i++)
    printf("----------");
  printf("\n");

  /* Display the stats. */
  for (i = 0; i < file.nclusters; i++)
    {
      cluster = &file.clusters[i];
      if (!cluster_matches(s, cluster))
	continue;
      printf("%-8.8s ", cluster->name);
      rowtotal.nfree = 0;
      rowtotal.total = 0;
      for (j = 0; j < file.narch; j++)
	{
	  entry = &cluster->stats[j];
	  if (entry->total == 0)
	    printf("    -     ");
	  else
	    printf("%3d / %-3d ", entry->nfree, entry->total);
	  coltotals[j].nfree += entry->nfree;
	  coltotals[j].total += entry->total;
	  rowtotal.nfree += entry->nfree;
	  rowtotal.total += entry->total;
	}
      printf("%3d / %-3d\n", rowtotal.nfree, rowtotal.total);
    }

  /* Display the column totals. */
  rowtotal.nfree = 0;
  rowtotal.total = 0;
  printf("TOTALS   ");
  for (i = 0; i < file.narch; i++)
    {
      printf("%3d / %-3d ", coltotals[i].nfree, coltotals[i].total);
      rowtotal.nfree += coltotals[i].nfree;
      rowtotal.total += coltotals[i].total;
    }
  printf("%3d / %-3d\n", rowtotal.nfree, rowtotal.total);
}

static void display_help(void)
{
  printf("Usage:\tcview [cluster ...]\n");
  printf("\tcview all\n");
  printf("\tcview public\n");
  printf("\tcview printers\n");
  printf("\tcview phones\n");
}

static void read_clusters(struct clusterfile *file)
{
  FILE *fp;
  char *line = NULL;
  int linesize, i;
  struct cluster *cluster;
  const char *p, *q;
  struct stat statbuf;

  fp = fopen(LARVNET_PATH_CLUSTERS, "r");
  if (!fp)
    {
      fprintf(stderr, "Sorry, no cluster status information available.\n");
      exit(1);
    }

  fstat(fileno(fp), &statbuf);
  file->mtime = statbuf.st_mtime;

  /* Read in the architecture names. */
  file->archnames = NULL;
  file->narch = 0;
  while (read_line(fp, &line, &linesize) == 0)
    {
      if (*line == '-')
	break;

      /* Add an architecture name. */
      file->narch++;
      file->archnames = erealloc(file->archnames,
				 file->narch * sizeof(char *));
      p = skip_spaces(line);
      q = skip_nonspaces(p);
      file->archnames[file->narch - 1] = estrndup(p, q - p);
    }

  /* Read in the cluster information. */
  file->clusters = NULL;
  file->nclusters = 0;
  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Make a new cluster entry. */
      file->nclusters++;
      file->clusters = erealloc(file->clusters,
				file->nclusters * sizeof(struct cluster));
      cluster = &file->clusters[file->nclusters - 1];

      /* Read the name, public/private field, and phone. */
      p = skip_spaces(line);
      q = skip_nonspaces(p);
      cluster->name = estrndup(p, q - p);
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      cluster->public = first_field_matches(p, "public");
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      cluster->phone = estrndup(p, q - p);

      /* Read the stats. */
      cluster->stats = emalloc(file->narch * sizeof(struct outof));
      for (i = 0; i < file->narch; i++)
	{
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cluster->stats[i].nfree = atoi(p);
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cluster->stats[i].total = atoi(p);
	}
    }

  fclose(fp);
}

static void read_printers(struct printerfile *file)
{
  FILE *fp;
  char *line = NULL;
  int linesize;
  struct printer *printer;
  const char *p, *q;
  struct stat statbuf;

  fp = fopen(LARVNET_PATH_PRINTERS, "r");
  if (!fp)
    {
      fprintf(stderr, "Sorry, no printer status information available.\n");
      exit(1);
    }

  fstat(fileno(fp), &statbuf);
  file->mtime = statbuf.st_mtime;

  file->printers = NULL;
  file->nprinters = 0;
  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Make a new printer entry. */
      file->nprinters++;
      file->printers = erealloc(file->printers,
				file->nprinters * sizeof(struct printer));
      printer = &file->printers[file->nprinters - 1];

      /* Read in the name, cluster name, status, and jobs. */
      p = skip_spaces(line);
      q = skip_nonspaces(p);
      printer->name = estrndup(p, q - p);
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      printer->cluster = estrndup(p, q - p);
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      printer->up = first_field_matches(p, "up");
      p = skip_spaces(q);
      printer->jobs = atoi(p);
    }
}

static void read_cgroups(struct cgroupfile *file)
{
  FILE *fp;
  char *line = NULL;
  int linesize;
  struct cgroup *cgroup;
  struct cgcluster *cgcluster;
  const char *p, *q;
  struct stat statbuf;

  fp = fopen(LARVNET_PATH_CGROUPS, "r");
  if (!fp)
    {
      fprintf(stderr, "No cluster group information available.\n");
      exit(1);
    }

  fstat(fileno(fp), &statbuf);
  file->mtime = statbuf.st_mtime;

  file->cgroups = NULL;
  file->ncgroups = 0;
  while (read_line(fp, &line, &linesize) == 0)
    {
      /* Make a new cluster group entry. */
      file->ncgroups++;
      file->cgroups = erealloc(file->cgroups,
			       file->ncgroups * sizeof(struct cgroup));
      cgroup = &file->cgroups[file->ncgroups - 1];

      /* Read in the cluster group name and coordinates. */
      p = skip_spaces(line);
      q = skip_nonspaces(p);
      cgroup->name = estrndup(p, q - p);
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      cgroup->x = atoi(p);
      p = skip_spaces(q);
      q = skip_nonspaces(p);
      cgroup->y = atoi(p);

      /* Read in the cluster names. */
      cgroup->clusters = NULL;
      cgroup->nclusters = 0;
      while (1)
	{
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  if (!*p || *p == '-')
	    break;
	  cgroup->nclusters++;
	  cgroup->clusters = erealloc(cgroup->clusters, cgroup->nclusters
				      * sizeof(struct cgcluster));
	  cgcluster = &cgroup->clusters[cgroup->nclusters - 1];
	  cgcluster->name = estrndup(p, q - p);
	  p = skip_spaces(q);
	  q = skip_nonspaces(p);
	  cgcluster->phone = estrndup(p, q - p);
	}

      /* Read in the printer names. */
      cgroup->printers = NULL;
      cgroup->nprinters = 0;
      p = skip_spaces(q);
      while (*p)
	{
	  q = skip_nonspaces(p);
	  cgroup->nprinters++;
	  cgroup->printers = erealloc(cgroup->printers,
				      cgroup->nprinters * sizeof(char *));
	  cgroup->printers[cgroup->nprinters - 1] = estrndup(p, q - p);
	  p = skip_spaces(q);
	}
    }
}

static int first_field_matches(const char *s, const char *word)
{
  int len = strlen(word);

  return (strncasecmp(s, word, len) == 0 &&
	  (isspace((unsigned char)s[len]) || !s[len]));
}

static int cluster_matches(const char *s, struct cluster *cluster)
{
  const char *p, *q;

  /* If no cluster names specified, match public clusters. */
  if (!*s)
    return cluster->public;

  p = skip_spaces(s);
  while (*p)
    {
      if (first_field_matches(p, "public") && cluster->public)
	return 1;
      if (first_field_matches(p, "all")
	  || first_field_matches(p, cluster->name))
	return 1;
      q = skip_nonspaces(p);
      p = skip_spaces(q);
    }

  return 0;
}

/* Read a line from a file into a dynamically allocated buffer,
 * zeroing the trailing newline if there is one.  The calling routine
 * may call read_line multiple times with the same buf and bufsize
 * pointers; *buf will be reallocated and *bufsize adjusted as
 * appropriate.  The initial value of *buf should be NULL.  After the
 * calling routine is done reading lines, it should free *buf.  This
 * function returns 0 if a line was successfully read, 1 if the file
 * ended, and -1 if there was an I/O error.
 */

static int read_line(FILE *fp, char **buf, int *bufsize)
{
  char *newbuf;
  int offset = 0, len;

  if (*buf == NULL)
    {
      *buf = emalloc(128);
      *bufsize = 128;
    }

  while (1)
    {
      if (!fgets(*buf + offset, *bufsize - offset, fp))
	return (offset != 0) ? 0 : (ferror(fp)) ? -1 : 1;
      len = offset + strlen(*buf + offset);
      if ((*buf)[len - 1] == '\n')
	{
	  (*buf)[len - 1] = 0;
	  return 0;
	}
      offset = len;

      /* Allocate more space. */
      newbuf = erealloc(*buf, *bufsize * 2);
      *buf = newbuf;
      *bufsize *= 2;
    }
}

static const char *skip_spaces(const char *p)
{
  while (isspace((unsigned char)*p))
    p++;
  return p;
}

static const char *skip_nonspaces(const char *p)
{
  while (*p && !isspace((unsigned char)*p))
    p++;
  return p;
}

static void *emalloc(size_t size)
{
  void *ptr;

  ptr = malloc(size);
  if (!ptr)
    {
      fprintf(stderr, "Ran out of memory!\n");
      exit(1);
    }
  return ptr;
}

static void *erealloc(void *ptr, size_t size)
{
  ptr = realloc(ptr, size);
  if (!ptr)
    {
      fprintf(stderr, "Ran out of memory!\n");
      exit(1);
    }
  return ptr;
}

static char *estrndup(const char *s, size_t n)
{
  char *new_s;

  new_s = emalloc(n + 1);
  memcpy(new_s, s, n);
  new_s[n] = 0;
  return new_s;
}
