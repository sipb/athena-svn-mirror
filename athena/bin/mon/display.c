/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/display.c,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/display.c,v 1.1 1984-12-13 12:00:27 dgg Exp $
 */

#ifndef lint
static char *rcsid_display_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/display.c,v 1.1 1984-12-13 12:00:27 dgg Exp $";
#endif	lint

/*
 *			D I S P L A Y . C
 *
 *  This section handles the display initialization and updates for "mon"
 */
#include "mon.h"
#include <curses.h>

#define	HOSTLEN	40		/* Length of hostname */

/* Screen Positions */
#define	LEFT	0
#define	PROCY	2
#define	CPUY	5
#define	TIMEY	8
#define	PAGEY	11
#define	CHARY	5
#define	CHARX	37
#define	NETIFY	17
#define	DISKY	2
#define	DISKX	55

/*
 * DISPINIT - clears the screen, puts up the info labels, and
 *   displays the initial information (device names).
 */
dispinit()
{
	char	hname[HOSTLEN];
	register int i;

	clear();			/* clear screen */
	gethostname(hname, HOSTLEN);	/* host name in upper left */
	printw(hname);

	/* Put up the labels */
        mvprintw(PROCY,LEFT,"Procs: r d p s sl  Mem: real  ract  virt  vact  free");
        mvprintw(DISKY,DISKX,"Disks: Kbps tps msps");
        mvprintw(CPUY,LEFT,"Cpu: ints  scall  csw");
	if (dualcpu)
	        mvprintw(CPUY,LEFT+25,"Cpu2: csw");
        mvprintw(TIMEY,LEFT,"Time: user nice sys idle");
	if (dualcpu)
	        mvprintw(TIMEY,LEFT+26,"Time2: user nice sys idle");
        mvprintw(PAGEY,LEFT,"Paging: re  at pin pout  oprs  fr  def   sr");
	mvprintw(PAGEY+3,LEFT,"       nxf  xf  nzf  zf  nrf  rf  prf  swi swo");
        mvprintw(CHARY,CHARX,"Char: in   out");
        mvprintw(NETIFY,LEFT,"Name   Ipkts  Ierrs  Opkts  Oerrs  Collis Oqueue");

	/* add the disk drive names to the screen */
        for(i = 0; i < DK_NDRIVE; i++) {
		if (*dr_name[i])
	                mvprintw(DISKY+1+i,DISKX,dr_name[i]);
		else
			break;
        }
	mvprintw(DISKY+1+i,DISKX,"--------------------");
	mvprintw(DISKY+2+i,DISKX,"Total:");

	/* put up the network interface names */
	for (i = 0; i < numif; i++)
		mvprintw(NETIFY+1+i,LEFT,nifinfo[i].name);
}

/*
 * DISPUPDATE - updates the dynamic data on the screen.
 */
dispupdate()
{
	int	i;

	for (i = 0; i < numif; i++)
		mvprintw(NETIFY+1+i,LEFT+7,"%5d  %5d  %5d  %5d  %5d  %5d",
			nifdat[i].ipackets, nifdat[i].ierrors,
			nifdat[i].opackets, nifdat[i].oerrors,
			nifdat[i].collisions, nifinfo[i].outqlen);
}
