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

static const char rcsid[] = "$Id: desync.c,v 1.3 1997-03-14 21:21:06 ghudson Exp $";

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
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

#define CONF SYSCONFDIR "/rc.conf"

static char *progname;

static unsigned long get_addr(void);

int main(int argc, char **argv)
{
  const char *timefile = NULL;
  int range, interval, c;
  unsigned long tval, ip;
  FILE *fp;

  /* Save the program name. */
  progname = argv[0];

  /* Parse command-line flags. */
  while ((c = getopt(argc, argv, "t:")) != -1)
    {
      switch (c) {
      case 't':
	timefile = optarg;
	break;
      default:
	fprintf(stderr, "Usage: %s [-t timefile] [interval]\n");
	return 2;
      }
    }

  /* Get the time interval from the remaining argument, if there is one. */
  argc -= optind;
  argv += optind;
  range = (argc == 1) ? atoi(argv[0]) : 3600;

  ip = get_addr();
  if (ip == INADDR_NONE)
    return 2;

  srand(ntohl(ip));
  interval = rand() % range;

  if (timefile)
    {
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
	  if (time(NULL) >= tval)
	    {
	      unlink(timefile);
	      return 0;
	    }
	  else
	    return 1;
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
	  fprintf(fp, "%lu\n", (unsigned long)(time(NULL) + interval));
	  fclose(fp);
	  return 1;
	}
    }
  else
    sleep(interval);

  return 0;
}

static unsigned long get_addr()
{
  FILE *fp;
  char buf[128], *p, *q;
  unsigned long ip;

  /* Open rc.conf for reading. */
  fp = fopen(CONF, "r");
  if (!fp)
    {
      fprintf(stderr, "%s: Can't open %s for reading: %s\n", progname, CONF,
	      strerror(errno));
      return INADDR_NONE;
    }

  /* Look for the line setting ADDR. */
  while (fgets(buf, sizeof(buf), fp) != NULL)
    {
      if (strncmp(buf, "ADDR", 4) == 0)
	{
	  /* Find the value (or go on if ADDR isn't followed by '='). */
	  p = buf + 4;
	  while (isspace(*p))
	    p++;
	  if (*p != '=')
	    continue;
	  p++;
	  while (isspace(*p))
	    p++;

	  /* Truncate after the address. */
	  q = p;
	  while (isdigit(*q) || *q == '.')
	    q++;
	  *q = 0;

	  /* Parse the value into an IP address and test its validity. */
	  ip = inet_addr(p);
	  if (ip == INADDR_NONE)
	    fprintf(stderr, "%s: Invalid address: %s\n", progname, p);

	  fclose(fp);
	  return ip;
	}
    }

  /* We didn't find it. */
  fclose(fp);
  fprintf(stderr, "%s: Can't find ADDR line in %s\n", progname, CONF);
  return INADDR_NONE;
}
