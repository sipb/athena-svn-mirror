/*
 * This is the MIT supplement to the PSI/NYSERNet implementation of SNMP.
 * This file describes the io portion of the mib.
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Tom Coppeto
 * MIT Network Services
 * 15 April 1990
 *
 *    $Source: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/io_grp.c,v $
 *    $Author: tom $
 *    $Locker:  $
 *    $Log: not supported by cvs2svn $
 */

#ifndef lint
static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/io_grp.c,v 1.1 1993-06-18 14:32:59 tom Exp $";
#endif

#include "include.h"
#include <sys/buf.h>
#include <sys/dk.h>

#include <mit-copyright.h>

#ifdef MIT

struct 
{
  int	dk_busy;
  long	*dk_time;
  long	*dk_wds;
  long	*dk_seek;
  long	*dk_xfer;
  long	tk_nin;
  long	tk_nout;
} s, s1;

int	mf;
int	hz;
int	phz;
double	etime;
int	tohdr = 1;


	{ "_dk_busy" },
#define	X_DK_BUSY	0
	{ "_dk_time" },
#define	X_DK_TIME	1
	{ "_dk_wds" },
#define	X_DK_WDS	3
	{ "_tk_nin" },
#define	X_TK_NIN	4
	{ "_tk_nout" },
#define	X_TK_NOUT	5
	{ "_dk_seek" },
#define	X_DK_SEEK	6
	{ "_dk_mspw" },
#define	X_DK_MSPW	8

char	**dr_name;
int	*dr_select;
float	*dk_mspw;

#ifdef vax
char	*defdrives[] = { "hp0", "hp1", "hp2",  0 };
#endif
#ifdef ibm032
char	*defdrives[] = { "hd0", "hd1", "hd2",  0 };
#endif



/*
 * Function:    lu_kerberos()
 * Description: Top level callback for kerberos. The realm and key version
 *              variables should be split into two functions.
 */
 
int
lu_kerberos(varnode, repl, instptr, reqflg)
     struct snmp_tree_node *varnode;
     varbind *repl;
     objident *instptr;
     int reqflg;
{
  int num;

  bzero(lbuf, sizeof(lbuf));

  if (varnode->flags & NOT_AVAIL || varnode->offset <= 0)
    return (BUILD_ERR);

  if((instptr == (objident *) NULL) || (instptr->ncmp == 0))
    num = 1;
  else
    num = instptr->cmp[0];

  if((reqflg == NXT) && (instptr != (objident *) NULL) &&
     (instptr->ncmp != 0))
    num++;

  /*
   * Build reply
   */

  bcopy ((char *)varnode->var_code, (char *) &repl->name, sizeof(repl->name));
  
}


main(argc, argv)
	char *argv[];
{
  extern char *ctime();
  register  i;
  int iter, ndrives;
  double f1, f2;
  long t;
  char *arg, **cp, name[6], buf[BUFSIZ];
  
  if (nl[X_DK_NDRIVE].n_value == 0) 
    {
      printf("dk_ndrive undefined in system\n");
      exit(1);
    }
  
  lseek(mf, nl[X_DK_NDRIVE].n_value, L_SET);
  read(mf, &dk_ndrive, sizeof (dk_ndrive));
  if (dk_ndrive <= 0) 
    {
      syslog(LOG_ERR, "dk_ndrive %d\n", dk_ndrive);
      return(BUILD_ERR);
    }
  
  dr_select = (int *)calloc(dk_ndrive, sizeof (int));
  dr_name = (char **)calloc(dk_ndrive, sizeof (char *));
  dk_mspw = (float *)calloc(dk_ndrive, sizeof (float));

#define	allocate(e, t) \
  s./**/e = (t *)calloc(dk_ndrive, sizeof (t)); \
    s1./**/e = (t *)calloc(dk_ndrive, sizeof (t));
  allocate(dk_time, long);
  allocate(dk_wds, long);
  allocate(dk_seek, long);
  allocate(dk_xfer, long);

  for (arg = buf, i = 0; i < dk_ndrive; i++) 
    {
      dr_name[i] = arg;
      sprintf(dr_name[i], "dk%d", i);
      arg += strlen(dr_name[i]) + 1;
    }
  
  read_names();
  lseek(mf, (long)nl[X_HZ].n_value, L_SET);
  read(mf, &hz, sizeof hz);
  lseek(mf, (long)nl[X_PHZ].n_value, L_SET);
  read(mf, &phz, sizeof phz);
  if (phz)
    hz = phz;
  lseek(mf, (long)nl[X_DK_MSPW].n_value, L_SET);
  read(mf, dk_mspw, dk_ndrive*sizeof (dk_mspw));

  lseek(mf, (long)nl[X_DK_BUSY].n_value, L_SET);
  read(mf, &s.dk_busy, sizeof s.dk_busy);
  lseek(mf, (long)nl[X_DK_TIME].n_value, L_SET);
  read(mf, s.dk_time, dk_ndrive*sizeof (long));
  lseek(mf, (long)nl[X_DK_XFER].n_value, L_SET);
  read(mf, s.dk_xfer, dk_ndrive*sizeof (long));
  lseek(mf, (long)nl[X_DK_WDS].n_value, L_SET);
  read(mf, s.dk_wds, dk_ndrive*sizeof (long));
  lseek(mf, (long)nl[X_DK_SEEK].n_value, L_SET);
  read(mf, s.dk_seek, dk_ndrive*sizeof (long));
  lseek(mf, (long)nl[X_TK_NIN].n_value, L_SET);
  read(mf, &s.tk_nin, sizeof s.tk_nin);
  lseek(mf, (long)nl[X_TK_NOUT].n_value, L_SET);
  read(mf, &s.tk_nout, sizeof s.tk_nout);
  lseek(mf, (long)nl[X_CP_TIME].n_value, L_SET);
  read(mf, s.cp_time, sizeof s.cp_time);
  for (i = 0; i < dk_ndrive; i++) 
    {
      if (!dr_select[i])
	continue;
#define X(fld)	t = s.fld[i]; s.fld[i] -= s1.fld[i]; s1.fld[i] = t
      X(dk_xfer); X(dk_seek); X(dk_wds); X(dk_time);
    }
	
  t = s.tk_nin; s.tk_nin -= s1.tk_nin; s1.tk_nin = t;
  t = s.tk_nout; s.tk_nout -= s1.tk_nout; s1.tk_nout = t;
  etime = 0;
  for(i=0; i<CPUSTATES; i++) 
    {
      X(cp_time);
      etime += s.cp_time[i];
    }
  if (etime == 0.0)
    etime = 1.0;
  etime /= (float) hz;
  
  for (i=0; i<dk_ndrive; i++)
    if (dr_select[i])
      stats(i);	
}

stats(dn)
{
	register i;
	double atime, words, xtime, itime;

	if (dk_mspw[dn] == 0.0) {
		printf("%4.0f%4.0f%5.1f ", 0.0, 0.0, 0.0);
		return;
	}
	atime = s.dk_time[dn];
	atime /= (float) hz;
	words = s.dk_wds[dn]*32.0;	/* number of words transferred */
	xtime = dk_mspw[dn]*words;	/* transfer time */
	itime = atime - xtime;		/* time not transferring */
	if (xtime < 0)
		itime += xtime, xtime = 0;
	if (itime < 0)
		xtime += itime, itime = 0;
	printf("%4.0f", words/512/etime);
	printf("%4.0f", s.dk_xfer[dn]/etime);
	printf("%5.1f ",
	    s.dk_seek[dn] ? itime*1000./s.dk_seek[dn] : 0.0);
}

#define steal(where, var) \
    lseek(mf, where, L_SET); read(mf, &var, sizeof var);

#ifdef vax
#include <vaxuba/ubavar.h>
#include <vaxmba/mbavar.h>

read_names()
{
	struct mba_device mdev;
	register struct mba_device *mp;
	struct mba_driver mdrv;
	short two_char;
	char *cp = (char *) &two_char;
	struct uba_device udev, *up;
	struct uba_driver udrv;

	mp = (struct mba_device *) nl[X_MBDINIT].n_value;
	up = (struct uba_device *) nl[X_UBDINIT].n_value;
	if (up == 0) {
		fprintf(stderr, "iostat: Disk init info not in namelist\n");
		exit(1);
	}
	if (mp) for (;;) {
		steal(mp++, mdev);
		if (mdev.mi_driver == 0)
			break;
		if (mdev.mi_dk < 0 || mdev.mi_alive == 0)
			continue;
		steal(mdev.mi_driver, mdrv);
		steal(mdrv.md_dname, two_char);
		sprintf(dr_name[mdev.mi_dk], "%c%c%d",
		    cp[0], cp[1], mdev.mi_unit);
	}
	if (up) for (;;) {
		steal(up++, udev);
		if (udev.ui_driver == 0)
			break;
		if (udev.ui_dk < 0 || udev.ui_alive == 0)
			continue;
		steal(udev.ui_driver, udrv);
		steal(udrv.ud_dname, two_char);
		sprintf(dr_name[udev.ui_dk], "%c%c%d",
		    cp[0], cp[1], udev.ui_unit);
	}
}
#endif

#ifdef ibm032
#include <caio/ioccvar.h>
read_names()
{
	struct iocc_device iod;
	register struct iocc_device *mp;
	struct iocc_driver mdrv;
	union {
	short two_short;
	char two_char[2];
	} two;
	char *cp =  two.two_char;

	mp = (struct iocc_device *) nl[X_MBDINIT].n_value;
	if (mp == 0) {
		fprintf(stderr, "iostat: Disk init info not in namelist\n");
		exit(1);
	}
	for (;;) {
		steal(mp++, iod);
		if (iod.iod_driver == 0)
			break;
		if (debug)
			printf("dk=%d alive=%d ctlr=%d\n",iod.iod_dk,
				iod.iod_alive,iod.iod_ctlr);
		if (iod.iod_dk < 0 || iod.iod_alive == 0)
			continue;
		if (iod.iod_ctlr < 0)
			continue;
		steal(iod.iod_driver, mdrv);
		steal(mdrv.idr_dname, two.two_short);
		sprintf(dr_name[iod.iod_dk], "%c%c%d", cp[0], cp[1],
			 iod.iod_unit);
	}
}
#endif

#endif MIT
