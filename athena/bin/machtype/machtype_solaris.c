/*
 *  Machtype: determine machine type & display type
 *
 * RCS Info
 *    $Id: machtype_solaris.c,v 1.5 2002-03-21 04:35:51 ghudson Exp $
 */

#define volatile 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kvm.h>
#include <nlist.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/cpu.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/vtoc.h>
/* OpenPROM stuff */
#include <sys/promif.h>
#include <sys/ddi.h>
#include <sys/sunddi.h>
#include <sys/ddi_impldefs.h>
#include <sys/systeminfo.h>
/* Frame buffer stuff */
#include <sys/fbio.h>
#include <errno.h>
#include "machtype.h"

void do_machtype(void)
{
    puts("sun4");
}

void do_cpu(int verbose)
{
  char            buf[BUFSIZ];

  char*           cpustr;

  /* read device name of the top node of the OpenPROM */

  if (sysinfo(SI_PLATFORM, buf, BUFSIZ) < 0) {
    fprintf(stderr, "Can't get CPU information\n");
    exit(2);
  }
  buf[BUFSIZ-1] = '\0';

  /* now, return a string identifying the CPU */

  if (verbose) {
    /* "verbose" returns the kernel information directly */
    puts(buf);

  } else {

    /* skip the initial "SUNW," */
    cpustr = strchr(buf, ',');
    if (cpustr)
      cpustr++;
    else
      cpustr = buf;

    /* reformat the result to look like "SPARC/Classic" or "SPARC/5" */
    if (! strncmp(cpustr, "SPARC", sizeof("SPARC")-1)) {
      cpustr += sizeof("SPARC")-1;

      if (! strncmp(cpustr, "station-", sizeof("station-")-1))
	cpustr += sizeof("station-")-1;
      else
	if (! strcmp(cpustr, "classic")) /* backwards compat - cap classic */
	  (*cpustr) = toupper(*cpustr);

      printf("SPARC/%s\n", cpustr);

    } else {
      /* if it didn't start with "SPARC", just leave it be... */
      puts(cpustr);
    }
  }

  return;
}

void do_dpy(int verbose)
{
  int count;
  char buf[1024], *p;

  count = readlink("/dev/fb", buf, sizeof(buf) - 1);
  if (count == -1) {
    puts(verbose ? "unknown frame buffer" : "unknown");
    return;
  }
  buf[count] = 0;
  p = buf + count;
  while (p > buf && isdigit((unsigned char)*(p - 1)))
    *--p = 0;
  p = strrchr(buf, ':');
  if (!p) {
    puts(verbose ? "unknown frame buffer" : "unknown");
    return;
  }
  printf("%s%s\n", p + 1, verbose ? " frame buffer" : "");
}

void do_disk(int verbose)
{
  DIR *dp;
  struct dirent *de;
  char path[MAXPATHLEN];
  const char *devdir = "/dev/rdsk";
  char *cp;
  int fd;
  int devlen;			/* Length of device name, w/o partition */
  struct vtoc vtoc;
  
  dp = opendir(devdir);
  if (dp == NULL)
    {
      fprintf(stderr, "Cannot open %s: %s\n", devdir, strerror(errno));
      exit(1);
    }

  while ((de = readdir(dp)) != NULL)
    {
      if ((!strcmp(de->d_name, ".")) || (!strcmp(de->d_name, "..")))
	continue;

      /* By convention partition (slice) 2 is the whole disk. */
      cp = strrchr(de->d_name, 's');
      if ((cp == NULL) || (strcmp(cp, "s2") != 0))
	continue;
      devlen = cp - de->d_name;		/* Get name length w/o partition */
      sprintf(path, "%s/%s", devdir, de->d_name);
      fd = open(path, O_RDONLY);
      if (fd == -1)
	continue;

      if ((read_vtoc(fd, &vtoc) < 0) || (vtoc.v_sanity != VTOC_SANE))
	{
	  close(fd);
	  continue;
	}
      close(fd);

      if (!verbose)
	{
	  /* Strip geometry info from the label text. */
	  cp = strchr(vtoc.v_asciilabel, ' ');
	  if (cp)
	    *cp = '\0';
	}

      printf("%.*s: %.*s\n",
	     devlen, de->d_name,
	     LEN_DKL_ASCII, vtoc.v_asciilabel);
    }

  closedir(dp);
  return;
}

#define MEG (1024*1024)

void do_memory(int verbose)
{
   int mem, nbpp;

   nbpp = getpagesize() / 1024;
   mem = sysconf(_SC_PHYS_PAGES);
   if(verbose)
      printf("%d (%d M) total\n", mem * nbpp, (mem * nbpp + 916) / 1024);
   else
      printf("%d\n", mem * nbpp + 916);
   return;
}

