/*
 *  Machtype: determine machine type & display type
 *
 * RCS Info
 *    $Id: machtype.c,v 1.1 1998-09-30 21:07:11 ghudson Exp $
 *    $Locker:  $
 */

#include <stdio.h>
#include <string.h>
#include "machtype.h"

extern char *optarg;
extern int optind;

void usage(char *name);

int main(int argc, char *argv[])
{
  int i;
  int cpuflg = 0, dpyflg = 0, raflg = 0, memflg = 0;
  int doathenaV = 0;
  int dosyspV = 0;
  int dolocalV = 0;
  int dobosN = 0;
  int dobosV = 0;
  int dosysnam = 0;
  int dosyscompatnam = 0;
  int verbose = 0;
  FILE *f;
  char *progname, *cp;

  progname = argv[0];
  cp = strrchr(progname, '/');
  if (cp != NULL)
    progname = cp+1;

  while ((i = getopt(argc, argv, "cdrMALPNESCv")) != EOF)
    {
      switch (i)
	{
	case 'c':
	  cpuflg = 1;
	  break;
	case 'd':
	  dpyflg = 1;
	  break;
	case 'r':
	  raflg = 1;
	  break;
	case 'M':
	  memflg = 1;
	  break;
	case 'A':
	  doathenaV = 1;
	  break;
	case 'L':
	  dolocalV = 1;
	  break;
	case 'P':
	  dosyspV = 1;
	  break;
	case 'N':
	  dobosN = 1;
	  break;
	case 'E':
	  dobosV = 1;
	  break;
	case 'S':
	  dosysnam = 1;
	  break;
	case 'C':
	  dosyscompatnam = 1;
	  break;
	case 'v':
	  verbose++;
	  break;
	default:
	  usage(progname);
	}
    }
  if (argv[optind] != NULL)
    usage(progname);

  if (!(cpuflg || dpyflg || raflg || memflg || doathenaV || dolocalV ||
	dosyspV || dobosN || dobosV || dosysnam || dosyscompatnam))
    {
      do_machtype();
      exit(0);
    }

  /* Print out version of Athena machtype compiled against */
  if (doathenaV)
    {
      if (verbose)
	fputs("Machtype version: ", stdout);
      printf("%s.%s\n", ATHMAJV, ATHMINV);
    }

    /* Print out version of attached packs */
    if (dosyspV)
      {
	char buf[256], *rvd_version, *p;
	if ((f = fopen("/srvd/.rvdinfo","r")) == NULL)
	    puts("Syspack information unavailable.");
	else
	  {
	    fgets(buf, sizeof(buf), f);
	    fclose(f);
	    /* If it is verbose, give the whole line, else just the vers # */
	    if (verbose)
		fputs(buf, stdout);
	    else
	      {
		p = strchr(buf,' '); /* skip "Athena" */
		p = strchr(p+1,' '); /* skip "RVD" */
		p = strchr(p+1,' '); /* Skip machtype */
		p = strchr(p+1,' '); /* skip "version" */
		rvd_version = p+1;
		p = strchr(rvd_version,' ');
		*p = '\0';
		puts(rvd_version);
	      }
	  }
      }

  /* Print out local version from /etc/athena/version */
  if (dolocalV)
    {
      char buf[256], *loc_version, *p;
      if ((f = fopen("/etc/athena/version","r")) == NULL)
	  puts("Local version information unavailable.");
      else
	{
	  while (fgets(buf, sizeof(buf), f) != NULL)
	    ;
	  fclose(f);
	  if (verbose)
	      fputs(buf, stdout);
	  else
	    {
	      p = strchr(buf,' '); /* skip "Athena" */
	      p = strchr(p+1,' '); /* skip "Workstation/Server" */
	      p = strchr(p+1,' '); /* Skip machtype */
	      p = strchr(p+1,' '); /* skip "version" */
	      loc_version = p+1;
	      p = strchr(loc_version,' ');
	      *p = '\0';
	      puts(loc_version);
	    }
	}
    }

  /* Print out vendor OS name */
  if (dobosN)
    if (verbose)
      puts(OSNAME " " OSVERS);
    else
      puts(OSNAME);

  /* Print out vendor OS version */
  if (dobosV)
    puts(OSVERS);

  /* Print out Athena System name */
  if (dosysnam)
    puts(ATHSYS);

  /* Print out compatible Athena System names */
  if (dosyscompatnam)
    puts(ATHSYSCOMPAT);

  if (cpuflg)
    do_cpu(verbose);
  if (dpyflg)
    do_dpy(verbose);
  if (raflg)
    do_disk(verbose);
  if (memflg)
    do_memory(verbose);
  exit(0);
}

void usage(char *name)
{
  fprintf(stderr, "usage: %s [-v] [-c] [-d] [-r] [-E] [-N] [-M]\n", name);
  fprintf(stderr, "             [-A] [-L] [-P] [-S] [-C]\n");
  exit(1);
}
