/* Copyright 1996, 1997 by the Massachusetts Institute of Technology.
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

static const char rcsid[] = "$Id: desync.c,v 1.9 2000-09-30 21:08:32 rbasch Exp $";

/*
 * desync - desynchronize cron jobs on networks
 *
 * This program is a tool which sleeps an ip-address dependent period
 * of time in order to skew over the course of a set time cron jobs
 * that would otherwise be synchronized. It should reside on local disk
 * so as not to cause a fileserver load at its own invocation.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

extern int optind;
extern char *optarg;

static char *progname;

static unsigned long get_hash(const char *str);

static void usage(void);

int main(int argc, char **argv)
{
  const char *timefile = NULL, *hostname = NULL;
  char buf[128];
  int range, interval, c, noop = 0;
  unsigned long tval;
  time_t timenow;
  FILE *fp;

  /* Save the program name. */
  progname = argv[0];

  /* Parse command-line flags. */
  while ((c = getopt(argc, argv, "h:nt:")) != -1)
    {
      switch (c) {
      case 'h':
	hostname = optarg;
	break;
      case 'n':
	noop = 1;
	break;
      case 't':
	timefile = optarg;
	break;
      default:
	usage();
	return 2;
      }
    }

  /* Get the time interval from the remaining argument, if there is one. */
  argc -= optind;
  argv += optind;
  range = (argc == 1) ? atoi(argv[0]) : 3600;
  if (range == 0)
    {
      fprintf(stderr, "%s: Invalid range value\n", progname);
      usage();
      return 2;
    }

  /* Get a random number in the given range as the interval.  Seed the
   * random number generator with a hash of the current host name, or
   * the name given via the -h option.
   */
  if (hostname == NULL)
    {
      if (gethostname(buf, sizeof(buf)) != 0)
	{
	  fprintf(stderr, "%s: Unable to obtain hostname: %s\n",
		  progname, strerror(errno));
	  return 2;
	}
      buf[sizeof(buf) - 1] = '\0';
      hostname = buf;
    }
  srand(get_hash(hostname));
  interval = rand() % range;

  if (timefile)
    {
      time(&timenow);
      fp = fopen(timefile, "r");
      if (fp)
	{
	  if (fscanf(fp, "%lu", &tval) != 1)
	    {
	      fprintf(stderr, "%s: Invalid time file %s\n", progname,
		      timefile);
	      return 2;
	    }
	  fclose(fp);
	  if (timenow >= tval)
	    {
	      if (noop)
		puts("0");
	      else
		unlink(timefile);
	      return 0;
	    }
	  else
	    {
	      if (noop)
		printf("%lu\n", (unsigned long) (tval - timenow));
	      return 1;
	    }
	}
      else if (noop)
	{
	  fprintf(stderr, "%s: Warning: Cannot open %s (%s)\n", progname,
		  timefile, strerror(errno));
	  printf("%lu\n", (unsigned long) (interval));
	  return (interval == 0 ? 0 : 1);
	}
      else
	{
	  if (interval == 0)
	    return 0;
	  fp = fopen(timefile, "w");
	  if (fp == NULL)
	    {
	      fprintf(stderr, "%s: Couldn't open %s for writing: %s\n",
		      progname, timefile, strerror(errno));
	      return 2;
	    }
	  fprintf(fp, "%lu\n", (unsigned long)(timenow + interval));
	  fclose(fp);
	  return 1;
	}
    }
  else if (noop)
    printf("%lu\n", (unsigned long) interval);
  else
    sleep(interval);

  return 0;
}

static unsigned long get_hash(const char *str)
{
  const char *p;
  unsigned long g, hashval = 0;

  for (p = str; *p; p++)
    {
      hashval = (hashval << 4) + *p;
      g = hashval & 0xf0000000;
      if (g != 0) {
	hashval ^= g >> 24;
	hashval ^= g;
      }
    }

  if (hashval == 0)
    hashval = 1;

  return hashval;
}

static void usage()
{
  fprintf(stderr,
	  "Usage: %s [-h name] [-n] [-t timefile] [range]\n",
	  progname);
}
