/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/mon.h,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/mon.h,v 1.1 1984-12-13 12:01:51 dgg Exp $
 */

/*
 *		M O N . H
 *
 *  Contains all includes and global defines/data needed by "mon"
 */
#include <stdio.h>
#include <ctype.h>
#include <nlist.h>
#include <sys/types.h>
#include <sys/vm.h>		/* virtual memory info struct */
#include <sys/dk.h>		/* disk info struct */
#include <sys/time.h>

/*
 * The main namelist entries.  This order MUST correspond to the
 *  definition of namelist in main.
 */
#define	X_DK_BUSY	0
#define	X_DK_MSPW	1
#define	X_HZ		2
#define	X_CP_TIME	3
#define	X_RATE		4
#define	X_TOTAL		5
#define	X_DEFICIT	6
#define	X_DK_XFER	7
#define X_MBDINIT	8
#define	X_UBDINIT	9
#define	N_IFNET		10
#define X_DK_TIME	11
#define	X_DK_WDS	12
#define	X_DK_SEEK	13
#define	X_TK_NIN	14
#define	X_TK_NOUT	15
#define	LOADAV		16
#define X_CP_TIME2	17	/* 2nd CPU stats */
#define	X_SLAVESTART	18	/* 2nd cpu existance test */

extern struct nlist namelist[];
int intv;			/* interval time */
int numif;			/* number of interfaces */
int dualcpu;			/* flag to indicate dual CPU */
int kmem, hz;
int deficit;
double etime;
double loadavg[4];

/* drive names and numbers */
char dr_name[DK_NDRIVE][10];
char dr_unit[DK_NDRIVE];

/*
 *  Net Interfaces Data - consists of two structures:
 *
 *	nifinfo		contains the "static" data (names, addresses)
 *			and iterative data which does not consist
 *			of "running totals".
 *
 *	nifdata		The "dynamic" running total data.  Two copies,
 *			a "total" and "interval" are kept.
 *
 */
#define	MAXIF	10		/* ten interfaces maximum */

struct	nifinfo {
	char	name[16];	/* interface name */
	int	outqlen;	/* output queue length */
	/* add address, netname, etc. eventually */
	/* also non totaling data */
} nifinfo[MAXIF];

struct	nifdata	{
	int	ipackets;	/* input packets */
	int	ierrors;	/* input errors */
	int	opackets;	/* output packets */
	int	oerrors;	/* output errors */
	int	collisions;
} nifdat[MAXIF], niftot[MAXIF];

/* state info */
struct state {
        long    cp_time[CPUSTATES];
	long	cp_time2[CPUSTATES];	/* 2nd CPU */
        long    dk_time[DK_NDRIVE];
        long    dk_wds[DK_NDRIVE];
        long    dk_seek[DK_NDRIVE];
        long    dk_xfer[DK_NDRIVE];
        float   dk_mspw[DK_NDRIVE];
        long    tk_nin;
        long    tk_nout;
        struct  vmmeter Rate;
        struct  vmtotal Total;
} s, s1;

#define rate            s.Rate
#define total           s.Total
