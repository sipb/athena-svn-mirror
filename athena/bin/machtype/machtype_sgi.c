/*
 *  Machtype: determine machine type & display type
 *
 * RCS Info
 *	$Id: machtype_sgi.c,v 1.5 1996-08-20 19:26:07 cfields Exp $
 *	$Locker:  $
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#if defined(_AIX) && defined(_BSD)
#undef _BSD
#endif
#include <nlist.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>

#include <sys/sysinfo.h>
#include <sys/ioctl.h>

#include <ctype.h>
#ifdef sgi
#include <sys/cpu.h>
#include <invent.h>
void do_INV_SCSI(inventory_t *);
void do_INV_DISK(inventory_t *);
void do_INV_PROCESSOR(inventory_t *,int);
void do_INV_GRAPHICS(inventory_t *);
void do_INV_CAM(inventory_t *);
#endif



#define NL_NF(nl) ((nl).n_type == 0)

#define KERNEL "/unix"
#define MEMORY "/dev/kmem"

int verbose = 0;

struct nlist nl[] = {
#define X_cpu 0
	{ "cputype" },
#define X_maxmem 1
	{ "maxmem" },
#define X_physmem 2
	{ "physmem" },
#define X_nscsi 3
	{ "_nscsi_devices" },
	{ "" },
};

main(argc, argv)
int	argc;
char	**argv;
{
    int i;
    int cpuflg = 0, dpyflg = 0, raflg = 0, memflg = 0;
    int doathenaV = 0;
    int dosyspV = 0;
    int dolocalV = 0;
    int dobosN = 0;
    int dobosV = 0;
    int dosysnam = 0;
    char *kernel = KERNEL,  *memory = MEMORY;
    FILE *f;
    int memfd=0;

    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-')
	  usage(argv[0]);

	switch (argv[i][1]) {
	case 'c':
	    cpuflg++;
	    break;
	case 'd':
	    dpyflg++;
	    break;
	case 'r':
	    raflg++;
	    break;
	case 'M':
	    memflg++;
	    break;
	case 'k':
	    kernel = argv[i+1];
	    i++;
	    break;
	case 'm':
	    memory = argv[i+1];
	    i++;
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
	case 'v':
	    verbose++;
	    break;
	default:
	    usage(argv[0]);
	}
    }


    if ((argc == 1) || ((argc == 2) && verbose)) {
#if defined(vax)
      puts("vax");
#else
#if defined(ibm032)
      puts("rt");
#else
#if defined(ultrix) && defined(mips)
      puts("decmips");
#else
#if defined(i386) && defined(_AIX)
      puts("ps2");
#else /* ! ps2 */
#if defined(sgi)
	puts("sgi");
#else
#if defined(sun) && defined(sparc)
      puts("sun4");
#else /* ! sun sparc */
#if defined(sun) 
      puts("sun3");
#else
      puts("???");
#endif
#endif
#endif
#endif
#endif
#endif
#endif
      exit(0);
    }

	/* Print out version of Athena machtype compiled against */
    if (doathenaV) {
      if (verbose)
	printf("Machtype version: %s.%s\n",ATHMAJV,ATHMINV);
      else
	printf("%s.%s\n",ATHMAJV,ATHMINV);
    }

    /* Print out version of attached packs */
    if (dosyspV) {
      char buf[256],rvd_version[256], *p;
      if ((f = fopen("/srvd/.rvdinfo","r")) == NULL) {
	printf("Syspack information unavailable\n");
      } else {
	fgets(buf,256,f);
	fclose(f);
	
	/* If it is verbose, give the whole line, else just the vers # */
	if (verbose) {
	  printf(buf);
	} else {
	  p = index(buf,' '); /* skip "Athena" */
	  p = index(p+1,' '); /* skip "RVD" */
	  p = index(p+1,' '); /* Skip "RSAIX" */
	  p = index(p+1,' '); /* skip "version" */
	  strncpy(rvd_version,p+1,256);
	  p = index(rvd_version,' ');
	  *p = '\0';
	  printf("%s\n",rvd_version);
	}
      }
    }
	
    /* Print out local version from /etc/athena/version */
    if (dolocalV) {
      char buf[256],loc_version[256], *p;
      if ((f = fopen("/etc/athena/version","r")) == NULL) {
	printf("Local version information unavailable\n");
      } else {
	fseek(f,-100,2);
	while (fgets(buf,256,f) != NULL)
	  ;
	fclose(f);
	
	if (verbose) {
	  printf(buf);
	} else {
	  p = index(buf,' '); /* skip "Athena" */
	  p = index(p+1,' '); /* skip "Workstation/Server" */
	  p = index(p+1,' '); /* Skip "RSAIX" */
	  p = index(p+1,' '); /* skip "version" */
	  strncpy(loc_version,p+1,256);
	  p = index(loc_version,' ');
	  *p = '\0';
	  printf("%s\n",loc_version);
	}
      }
    }

    /* Print out vendor OS name */
    if (dobosN) {
      if (verbose) {
	printf(OSNAME " " OSVERS "\n");
      } else {
	printf(OSNAME "\n");
      }
    }

    /* Print out vendor OS version */
    if (dobosV) {
	printf(OSVERS "\n");
    }
    if (dosysnam)
        printf("%s\n", ATHSYS);
    if (cpuflg)
	do_cpu(kernel, memfd);
    if (dpyflg)
	do_dpy(kernel, memfd);
    if (raflg)
	do_disk(kernel, memfd);

    if (memflg)
      {

	if (nlist(kernel, nl) < 0) {
	  fprintf(stderr,"%s: can't get namelist\n", argv[0]);
	  exit(2);
	}
        if ((memfd = open (memory, O_RDONLY)) == -1) {
          perror ("machtype: can't open memory");
          exit(2);
	}
	if (memflg)
	  do_memory(kernel, memfd);
      }
    exit(0);
}



usage(name)
char *name;
{
    fprintf(stderr, "usage: %s [-v] [-c] [-d] [-r] [-E] [-N] [-M]\n",name);
    fprintf(stderr, "             [-k kernel] [-m memory] [-A] [-L] [-P] [-S]\n");
    exit(1);
}


do_cpu(kernel, mf)
char *kernel;
int mf;
{
inventory_t *inv;
int done=0;
	(void)setinvent();
        inv = getinvent();
        while ((inv != NULL) && !done) {
		if ( inv->inv_class == 1) {
			do_INV_PROCESSOR(inv,verbose);
		}
                inv = getinvent();
        }
}

void do_INV_PROCESSOR(inventory_t *i,int verbose)
{
if (i->inv_type == INV_CPUBOARD )  {
	switch (i->inv_state) {
        case INV_R2300BOARD: 
                puts(verbose ? "SGI R2300": "R2300");
                break;
	case INV_IP4BOARD:
                puts(verbose ? "SGI IP4": "IP4");
                break;
	case INV_IP5BOARD:
                puts(verbose ? "SGI IP5": "IP5");
                break;
	case INV_IP6BOARD:
                puts(verbose ? "SGI IP6": "IP6");
                break;
	case INV_IP7BOARD:
                puts(verbose ? "SGI IP7": "IP7");
                break;
	case INV_IP9BOARD:
                puts(verbose ? "SGI IP9": "IP9");
                break;
	case INV_IP12BOARD:
                puts(verbose ? "SGI IP12": "IP12");
                break;
	case INV_IP15BOARD:
                puts(verbose ? "SGI IP15": "IP15");
                break;
	case INV_IP17BOARD:
                puts(verbose ? "SGI IP17": "IP17");
                break;
	case INV_IP19BOARD:
                puts(verbose ? "SGI IP19": "IP19");
                break;
	case INV_IP20BOARD:
                puts(verbose ? "SGI IP20": "IP20");
                break;
	case INV_IP21BOARD:
                puts(verbose ? "SGI IP21": "IP21");
                break;
	case INV_IP22BOARD:
                puts(verbose ? "SGI IP22": "IP22");
                break;
	case INV_IP26BOARD:
                puts(verbose ? "SGI IP26": "IP26");
                break;
         default:
           if(verbose)
                printf("Unknown SGI type %d\n", i->inv_state);
           else
                puts("SGI???");
	}
	} else if (verbose) {
		if (i->inv_type == INV_CPUCHIP) {
			fprintf(stdout,"CPU: MIPS R4600\n");
	} else {
			fprintf(stdout,"FPU:MIPS R4610\n");
	}
}
}


#ifdef vax
int ka420model()
{
    unsigned long cacr;
    int kUmf;
    if (nl[X_nexus].n_type == 0)
	return -1;
    kUmf = open("/dev/kUmem", O_RDONLY);
    if (kUmf == -1)
	return -1;
    lseek(kUmf, nl[X_nexus].n_value + (int)&((struct nb_regs *)0)->nb_cacr, L_SET);
    if(read(kUmf, &cacr, sizeof(cacr)) != sizeof(cacr))
	return -1;
    close (kUmf);
    return (cacr >> 6) & 3;
}
#endif /* vax */

do_dpy(kernel, mf)
char *kernel;
int mf;
{
int status;
inventory_t *inv;
int done=0;
#ifdef sgi
    if (verbose) {
	switch(fork()) {
		case -1:
			fprintf (stderr,
			"Don't know how to determine display type for this machine.\n");
			return;
		case 0:
			if ((execlp("/usr/gfx/gfxinfo","gfxinfo",NULL) ) == -1 ) {
				fprintf (stderr,
				"Don't know how to determine display type for this machine.\n");
				return;
			}	
		default:
			wait(&status);
		break;
	} /* switch */
	(void) setinvent();
	inv = getinvent();
        while ((inv != NULL) && !done) {
		if ( inv->inv_class == INV_VIDEO) {
			do_INV_CAM(inv);
		}
                inv = getinvent();
        }
    } else { /* not verbose */
	(void) setinvent();
	inv = getinvent();
        while ((inv != NULL) && !done) {
		if ( inv->inv_class == INV_GRAPHICS) {
			do_INV_GRAPHICS(inv);
		}
                inv = getinvent();
        }
     } /* verbose */

#else
    fprintf (stderr,
	     "Don't know how to determine display type for this machine.\n");
    return;
#endif
}

void do_INV_GRAPHICS(inventory_t *i)
{
        if (i->inv_type == INV_NEWPORT ) {
                if (i->inv_state == INV_NEWPORT_24 ) {
                        fprintf(stdout,"XL-24\n");
                } else if (i->inv_state == INV_NEWPORT_XL ) {
                        fprintf(stdout,"XL\n");
                } else {
                        fprintf(stdout,"NG1\n");
                }
        } else if (i->inv_type == INV_GR2 ) {
		if ((i->inv_state & ~INV_GR2_INDY) == INV_GR2_ELAN ) {
			/* an EXPRESS is an EXPRESS of course of course
			   except when you are a GR3-XZ */
			fprintf(stdout,"GR3-XZ\n");
		} else 
			fprintf(stdout,"UNKNOWN video\n");
	} else {
                fprintf(stdout,"UNKNOWN video\n");
        }
}

void do_INV_CAM(inventory_t *i)
{
	if (i->inv_type == INV_VIDEO_VINO ) {
		if (i->inv_state == INV_VINO_INDY_CAM ) {
			fprintf(stdout,"\tIndy cam connected\n");
		}
	}
}



do_disk(kernel, mf)
char *kernel;
int mf;
{
inventory_t *inv;
int t;
int done=0;
	(void) setinvent();
        inv = getinvent();
        t = 0;
        while ((inv != NULL) && !done) {
                if ((inv->inv_class == 2) || (inv->inv_class ==9) ) {
                                if ( inv->inv_class == 9) {
                                do_INV_SCSI(inv);
                                } else {
                                        do_INV_DISK(inv);
                                }
                }
                inv = getinvent();
        }
}
void do_INV_SCSI(inventory_t *i)
{
if (i->inv_type == INV_CDROM) {
        fprintf(stdout,"CDROM : unit %i, on SCSI controller %i\n",i->inv_unit,i->inv_controller);
} else {
        fprintf(stdout,"Unknown type %i:unit %i, on SCSI controller %i\n",i->inv_type,i->inv_unit,i->inv_controller);
}
}

void do_INV_DISK(inventory_t *i)
{
if (i->inv_type == INV_SCSIDRIVE) {
         fprintf(stdout,"Disk drive: unit %i, on SCSI controller %i\n",i->inv_unit,i->inv_controller);
} else if (i->inv_type == INV_SCSIFLOPPY) {
        fprintf(stdout,"Floppy drive: unit %i, on SCSI controller %i\n",i->inv_unit,i->inv_controller);
} else if (i->inv_type == INV_SCSICONTROL) {
        if (i->inv_type == INV_WD93) {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %c\n",i->inv_controller,"WD 33C93",i->inv_state);
        } else if (i->inv_type == INV_WD93A) {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %d\n",i->inv_controller,"WD 33C93A",i->inv_state);
        } else if (i->inv_type == INV_WD93B) {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %d\n",i->inv_controller,"WD 33C93B",i->inv_state);
        } else if (i->inv_type == INV_WD95A) {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %d\n",i->inv_controller,"WD 33C95A",i->inv_state);
        } else if (i->inv_type == INV_SCIP95) {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %d\n",i->inv_controller,"SCIP w/WD 33C95A",i->inv_state);
        } else {
                fprintf(stdout,"SCSI controller %i: Version %s, rev. %d\n",i->inv_controller,"Unkown",i->inv_state);
        }
} else {
        fprintf(stdout,"Unknown type %i:unit %i, on SCSI controller %i\n",i->inv_type,i->inv_unit,i->inv_controller);
}

}



#define MEG (1024*1024)

do_memory (kernel, mf)
char *kernel;
int mf;
{
  int pos, mem;
  pos = nl[X_maxmem].n_value;
  if(pos == 0) {
      fprintf(stderr, "can't find maxmem\n");
      exit(3);
  }
  lseek(mf,pos,L_SET);	/* Error checking ? */
  if(read(mf,&mem,4) == -1) {
      perror("read (kmem)");
      exit(4);
  } else {
    if(verbose)
      printf("%#06x user, ",mem * getpagesize());
  }
  pos = nl[X_physmem].n_value;
  if(pos == 0) {
      fprintf(stderr, "can't find physmem\n");
      exit(3);
  }
  lseek(mf,pos,L_SET);
  if(read(mf,&mem,4) == -1) {
      perror("read (kmem)");
      exit(4);
  } else {
    if(verbose)
      printf("%#06x (%d M) total\n",mem * getpagesize(),(mem * getpagesize() + MEG/2)/MEG);
    else
      printf("%d\n", mem * getpagesize());
  }
  return; 
}

