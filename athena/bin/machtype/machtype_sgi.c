/*
 *  Machtype: determine machine type & display type
 *
 * RCS Info
 *	$Id: machtype_sgi.c,v 1.16 1999-05-15 19:51:39 jweiss Exp $
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include "machtype.h"

#ifdef sgi
#include <invent.h>
#include <sys/sbd.h>
void do_INV_SCSI(inventory_t *, int);
void do_INV_SCSICONTROL(inventory_t *, int);
void do_INV_DISK(inventory_t *, int);
void do_INV_PROCESSOR(inventory_t *,int);
void do_INV_GRAPHICS(inventory_t *, int);
void do_INV_CAM(inventory_t *, int);
void do_INV_CPUCHIP(inventory_t *, int);
#endif

void do_machtype(void)
{
  puts("sgi");
}

void do_cpu(int verbose)
{
  inventory_t *inv;
  setinvent();
  inv = getinvent();
  while (inv) {
    if (inv->inv_class == INV_PROCESSOR) {
      do_INV_PROCESSOR(inv, verbose);
    }
    inv = getinvent();
  }
}

void do_INV_CPUCHIP(inventory_t* i, int verbose)
{
  char* pu_short = (i->inv_type == INV_CPUCHIP)?"CPU":"FPU";
  char* pu_long = (i->inv_type == INV_CPUCHIP)?
    "Processor Chip":"Floating Point Coprocessor";
  int chip = (i->inv_state & C0_IMPMASK) >> C0_IMPSHIFT;
  int major = (i->inv_state & C0_MAJREVMASK) >> C0_MAJREVSHIFT;
  int minor = (i->inv_state & C0_MINREVMASK) >> C0_MINREVSHIFT;
  char* s_chip = 0;
  switch(chip)
    {
    case C0_IMP_R2000:
      if (major >= C0_MAJREVMIN_R2000A)
	s_chip = "R2000A";
      else
	s_chip = "R2000";
      break;
    case C0_IMP_R2000A:
      if (major >= C0_MAJREVMIN_R3000A)
	s_chip = "R3000A";
      else if (major >= C0_MAJREVMIN_R3000)
	s_chip = "R3000";
      else
	s_chip = "R2000A";
      break;
    case C0_IMP_R6000:
      s_chip = "R6000";
      break;
    case C0_IMP_R4000:
      if (major >= C0_MAJREVMIN_R4400)
	s_chip = "R4400";
      else
	s_chip = "R4000";
      break;
    case C0_IMP_R6000A:
      s_chip = "R6000A";
      break;
    case C0_IMP_R10000:
      s_chip = "R10000";
      break;
    case C0_IMP_R8000:
      s_chip = "R8000";
      break;
    case C0_IMP_R4600:
      s_chip = "R4600";
      break;
    case C0_IMP_R4700:
      s_chip = "R4700";
      break;
    case C0_IMP_R4650:
      s_chip = "R4650";
      break;
    case C0_IMP_R5000:
      s_chip = "R5000";
      break;
    default:
      s_chip = 0;
      break;
    }
  if (s_chip)
    printf("%s: MIPS %s %s Revision: %d.%d\n", 
	   pu_short, s_chip, pu_long, major, minor);
  else
    printf("%s: unknown (0x%x) %s Revision: %d.%d\n", 
	   pu_short, chip, pu_long, major, minor);
}

void do_INV_PROCESSOR(inventory_t *i,int verbose)
{
  if (i->inv_type == INV_CPUBOARD )  {
    switch (i->inv_state) {
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
    case INV_IP25BOARD:
      puts(verbose ? "SGI IP25": "IP25");
      break;
    case INV_IP26BOARD:
      puts(verbose ? "SGI IP26": "IP26");
      break;
#ifdef INV_IP27BOARD
    case INV_IP27BOARD:
      puts(verbose ? "SGI IP27": "IP27");
      break;
#endif
    case INV_IP28BOARD:
      puts(verbose ? "SGI IP28": "IP28");
      break;
    case INV_IP30BOARD:
      puts(verbose ? "SGI IP30": "IP30");
      break;
    case INV_IP32BOARD:
      puts(verbose ? "SGI IP32": "IP32");
      break;
#ifdef INV_IP33BOARD
    case INV_IP33BOARD:
      puts(verbose ? "SGI IP33": "IP33");
      break;
#endif
    case INV_IPMHSIMBOARD:
      puts(verbose ? "SGI IPMHSIM": "IPMHSIM");
      break;
    default:
      if(verbose)
	printf("Unknown SGI type %d\n", i->inv_state);
      else
	puts("SGI???");
    }
  } else if (verbose) {
    switch(i->inv_type) {
    case INV_CPUCHIP:
    case INV_FPUCHIP:
      do_INV_CPUCHIP(i, verbose);
    }
  }
}

void do_dpy(int verbose)
{
  int status;
  inventory_t *inv;
  int done = 0;

  if (verbose) {
    fflush(stdout);
    switch(fork()) {
    case -1:
      fprintf (stderr,
	       "Don't know how to determine display type for this machine.\n");
      return;
    case 0:
      if ((execlp("/usr/gfx/gfxinfo","gfxinfo",NULL) ) == -1 ) {
	fprintf (stderr, 
		 "Don't know how to determine display "
		 "type for this machine.\n");
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
	do_INV_CAM(inv, verbose);
      }
      inv = getinvent();
    }
  } else { /* not verbose */
    (void) setinvent();
    inv = getinvent();
    while ((inv != NULL) && !done) {
      if ( inv->inv_class == INV_GRAPHICS) {
	do_INV_GRAPHICS(inv, verbose);
      }
      inv = getinvent();
    }
  } /* verbose */
}

void do_INV_GRAPHICS(inventory_t *i, int verbose)
{
  switch(i->inv_type)
    {
    case INV_NEWPORT:
      switch(i->inv_state)
	{
	case INV_NEWPORT_24:
	  fprintf(stdout,"XL-24\n");
	  break;
	case INV_NEWPORT_XL:
	  fprintf(stdout,"XL\n");
	  break;
	default:
	  fprintf(stdout,"NG1\n");
	  break;
	}
      break;
    case INV_GR2:
      /* an EXPRESS is an EXPRESS of course of course
	 except when you are a GR3-XZ */
      if ((i->inv_state & ~INV_GR2_INDY) == INV_GR2_ELAN )
	fprintf(stdout,"GR3-XZ\n");
      else
	fprintf(stdout,"UNKNOWN video\n");
      break;
#ifdef INV_CRIME
    case INV_CRIME:
      fprintf(stdout, "CRM\n");
      break;
#endif
    default:
      fprintf(stdout,"UNKNOWN video\n");
    }
}

void do_INV_CAM(inventory_t *i, int verbose)
{
  if (i->inv_type == INV_VIDEO_VINO ) {
    if (i->inv_state == INV_VINO_INDY_CAM ) {
      fprintf(stdout,"\tIndy cam connected\n");
    }
  }
}



void do_disk(int verbose)
{
  inventory_t *inv;
  int t;
  int done=0;
  (void) setinvent();
  inv = getinvent();
  t = 0;
  while ((inv != NULL) && !done) {
    if (inv->inv_class == INV_DISK) 
      do_INV_DISK(inv, verbose);
    else if (inv->inv_class == INV_SCSI)
      do_INV_SCSI(inv, verbose);
    inv = getinvent();
  }
}

void do_INV_SCSI(inventory_t *i, int verbose)
{
  if (i->inv_type == INV_CDROM) {
    fprintf(stdout, "CDROM: unit %i, on SCSI controller %i\n",
	    i->inv_unit,i->inv_controller);
  } else {
    fprintf(stdout, "Unknown type %i:unit %i, on SCSI controller %i\n",
	    i->inv_type,i->inv_unit,i->inv_controller);
  }
}

void do_INV_DISK(inventory_t *i, int verbose)
{
  switch (i->inv_type)
    {
    case INV_SCSIDRIVE:
      printf("Disk drive: unit %u, on SCSI controller %u\n",
	     i->inv_unit, i->inv_controller);
      break;

    case INV_SCSIFLOPPY:
      printf("Floppy drive: unit %u, on SCSI controller %u\n",
	     i->inv_unit, i->inv_controller);
      break;

    case INV_SCSICONTROL:
    case INV_GIO_SCSICONTROL:
#ifdef INV_PCI_SCSICONTROL
    case INV_PCI_SCSICONTROL:
#endif
      do_INV_SCSICONTROL(i, verbose);
      break;

    default:
      printf("Unknown type %u: unit %u, on SCSI controller %u\n",
	     i->inv_type, i->inv_unit, i->inv_controller);
      break;
    }
}

void do_INV_SCSICONTROL(inventory_t *i, int verbose)
{
  /* Only display SCSI controller info when verbose */
  if (!verbose)
    return;

  switch (i->inv_type)
    {
    case INV_SCSICONTROL:
      printf("Integral");
      break;
    case INV_GIO_SCSICONTROL:
      printf("GIO");
      break;
#ifdef INV_PCI_SCSICONTROL
    case INV_PCI_SCSICONTROL:
      printf("PCI");
      break;
#endif
    default:
      printf("Unknown");
      break;
    }

  printf(" SCSI controller %u: Version ", i->inv_controller);

  switch (i->inv_state)
    {
    case INV_WD93:
      printf("WD 33C93");
      break;

    case INV_WD93A:
      printf("WD 33C93A");
      break;

    case INV_WD93B:
      printf("WD 33C93B");
      break;

    case INV_WD95A:
      printf("WD 33C95A");
      break;

    case INV_SCIP95:
      printf("SCIP w/WD 33C95A");
      break;

#ifdef INV_ADP7880
    case INV_ADP7880:
      printf("ADAPTEC 7880");
      break;
#endif

    default:
      printf("Unknown");
      break;
    }

  if (i->inv_unit)
    printf(", rev. %X", i->inv_unit);

  putchar('\n');
}


#define MEG (1024*1024)

void do_memory(int verbose)
{
  inventory_t *inv;
  int mem = 0;
  int mem_mb = 0;
  setinvent();
  inv = getinvent();
  while (inv) {
    if (inv->inv_class == INV_MEMORY) {
      switch(inv->inv_type) {
      case INV_MAIN:
	mem = inv->inv_state;
	break;
      case INV_MAIN_MB:
	mem_mb = inv->inv_state;
	break;
      }
    }
    inv = getinvent();
  }
  if (!mem && !mem_mb) {
    fprintf(stderr, "can't find main memory in inventory\n");
    exit(2);
  }
  if (verbose)
    printf("%d (%d M) total\n", mem?mem:(mem_mb * MEG), 
	   mem_mb?mem_mb:(mem/MEG));
  else
    printf("%d\n", mem);
}
