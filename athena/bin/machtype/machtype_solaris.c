/*
 *  Machtype: determine machine type & display type
 *
 * RCS Info
 *    $Id: machtype_solaris.c,v 1.1 1999-09-22 00:26:50 danw Exp $
 */

#define volatile 
#include <stdio.h>
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
/* Frame buffer stuff */
#include <sys/fbio.h>
#include <errno.h>
#include "machtype.h"

struct nlist nl[] = {
#define X_cpu 0
      { "cputype" },
#define X_maxmem 1
      { "maxmem" },
#define X_physmem 2
      { "physmem" },
#define X_topnode 3
      { "top_devinfo" },
      { "" }
};

static kvm_t *do_init(void);
static void do_cleanup(kvm_t *kernel);
static void do_cpu_prom(kvm_t *kernel, int verbose);

static kvm_t *do_init(void)
{
    kvm_t *kernel;

    kernel = kvm_open(NULL,NULL,NULL,O_RDONLY,NULL);
    if (!kernel) {
        fprintf(stderr,"unable to examine the kernel\n");
        exit(2);
    }
    if (kvm_nlist(kernel, nl) < 0) {
        fprintf(stderr,"can't get namelist\n");
        exit(2);
    }
    return kernel;
}

static void do_cleanup(kvm_t *kernel)
{
    kvm_close(kernel);
}

void do_machtype(void)
{
    puts("sun4");
}

static void do_cpu_prom(kvm_t *kernel, int verbose)
{
  unsigned long   ptop;
  struct dev_info top;
  char            buf[BUFSIZ];

  char*           cpustr;

  /* read device name of the top node of the OpenPROM */

  if (   (! nl[X_topnode].n_value)
      || (kvm_read(kernel, (unsigned long) nl[X_topnode].n_value,
		            (char*) &ptop, sizeof(ptop)) != sizeof(ptop))
      || (! ptop)
      || (kvm_read(kernel, (unsigned long) ptop,
		            (char*) &top, sizeof(top)) != sizeof(top))
      || (! top.devi_name)
      || (kvm_read(kernel, (unsigned long) top.devi_name,
		            (char*) &buf, sizeof(buf)) != sizeof(buf))
      || (! buf[0]) ) {
    fprintf(stderr, "Can't get CPU information from the kernel\n");
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

void do_cpu(int verbose)
{
    kvm_t *kernel;
    short cpu, cpu_type;

    kernel = do_init();
    cpu_type = kvm_read(kernel,nl[X_cpu].n_value,(char*)&cpu, sizeof(cpu));

        switch(cpu) {
          case CPU_SUN4C_60:
            puts(verbose ? "SPARCstation 1": "SPARC/1");
            break;
          case CPU_SUN4C_40:
            puts(verbose ? "SPARCstation IPC" : "SPARC/IPC");
            break;
          case CPU_SUN4C_65:
            puts(verbose ? "SPARCstation 1+" : "SPARC/1+");
            break;
          case CPU_SUN4C_20:
            puts(verbose ? "SPARCstation SLC" : "SPARC/SLC");
            break;
          case CPU_SUN4C_75:
            puts(verbose ? "SPARCstation 2" : "SPARC/2");
            break;
          case CPU_SUN4C_25:
            puts(verbose ? "SPARCstation ELC" : "SPARC/ELC");
            break;
          case CPU_SUN4C_50:
            puts(verbose ? "SPARCstation IPX" : "SPARC/IPX");
            break;
	  case CPU_SUN4M_50:	/* 114... Sparc20 */
	  case OBP_ARCH:	/* 128 */
	    do_cpu_prom(kernel, verbose);
		break;

         default:
           if(verbose)
                printf("Unknown SUN type %d\n", cpu);
           else
              puts("SUN???");
         }

    do_cleanup(kernel);
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
  while (p > buf && isdigit((int)*(p - 1)))
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
   kvm_t *kernel;
   int mem, nbpp;

   kernel = do_init();
   nbpp = getpagesize() / 1024;
   kvm_read(kernel, nl[X_maxmem].n_value, (char *)&mem, sizeof(mem));
   if(verbose)
      printf("%d user, ", mem * nbpp);
   kvm_read(kernel, nl[X_physmem].n_value, (char *)&mem, sizeof(mem));
   if(verbose)
      printf("%d (%d M) total\n", mem * nbpp, (mem * nbpp + 916) / 1024);
   else
      printf("%d\n", mem * nbpp + 916);
   do_cleanup(kernel);
   return;
}

