/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/mon.c,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/mon.c,v 1.1 1984-12-13 12:00:58 dgg Exp $
 */

#ifndef lint
static char *rcsid_mon_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/mon.c,v 1.1 1984-12-13 12:00:58 dgg Exp $";
#endif	lint

/*
 *			M O N . C
 *
 *  A program to moniter system activity.
 *   Rework of earlier mon program.  17 Aug 84 - Phillip Dykstra
 *
 */

#include "mon.h"
#include <curses.h>
#include <signal.h>

struct	sgttyb ttyb;
int done();

/*
 *  The namelist array.  Uses to get the kernal variables needed
 *  by all the MON routines.  The order of this list MUST correspond
 *  to the order of the definitions in mon.h
 */
struct nlist namelist[] = {
	{ "_dk_busy" },
	{ "_dk_mspw" },
	{ "_hz" },
	{ "_cp_time" },
	{ "_rate" },
	{ "_total" },
	{ "_deficit" },
	{ "_dk_xfer" },
	{ "_mbdinit" },
	{ "_ubdinit" },
	{ "_ifnet" },
	{ "_dk_time" },
	{ "_dk_wds" },
	{ "_dk_seek" },
	{ "_tk_nin" },
	{ "_tk_nout" },
	{ "_avenrun" },
	{ "_cp2_time" },	/* 2nd CPU stats */
	{ "_slavestart" },	/* Used to detect 2nd CPU */
	{ 0 }
};

main(argc,argv)
int argc;
char *argv[];
{
        register i;
        double f1, f2;
        extern char *ctime();

/*
 * fill the namelist and open /dev/kmem
 */
        nlist("/vmunix", namelist);
        if(namelist[X_DK_BUSY].n_type == 0) {
                printf("dk_busy not found in /vmunix namelist\n");
                exit(1);
        }
        kmem = open("/dev/kmem", 0);
        if(kmem < 0) {
                printf("cannot open /dev/kmem\n");
                exit(1);
        }
        if(argc > 1)
                intv = (atoi(argv[1]));
        else intv = 1;

/* 
 * do all things that need to be done only once
 */
        lseek(kmem, (long)namelist[X_DK_MSPW].n_value, 0);
        read(kmem, s.dk_mspw, sizeof s.dk_mspw);
        lseek(kmem, (long)namelist[X_HZ].n_value, 0);
        read(kmem, &hz, sizeof hz);
	read_names();
	dualcpu = 0;
	if (namelist[X_SLAVESTART].n_type)
		dualcpu++;        
/*
 * monitor parameters forever
 */
        worker();
        /* NOTREACHED */
}

char obuf[BUFSIZ];	/* Output buffer */

worker()
{
        long clock;
	struct timeval tintv;
	int i, tin;

	/* set up signals */
	signal(SIGINT, done);
	signal(SIGQUIT, done);

	/* set CBREAK mode with no buffering on stdin */
	setbuf(stdin, 0);
	setbuf(stdout, obuf);
	ioctl(0, TIOCGETP, &ttyb);
	ttyb.sg_flags |= CBREAK;
	ioctl(fileno(stdin), TIOCSETP, (char *)&ttyb);

	tintv.tv_sec = intv;
	tintv.tv_usec = 0;
	initscr();	/* init curses package */
	nifinit();	/* get initial net interfaces data */
        dispinit();     /* initialize display */
        for(;;){
                vm();
                io();
        	nifupdate();
        	/* get load average */
		lseek(kmem, (long)namelist[LOADAV].n_value, 0);
		read(kmem, &loadavg[0], sizeof loadavg);
        	mvprintw(0,13,"%4.2f %4.2f %4.2f %4.2f", loadavg[3], loadavg[0], loadavg[1], loadavg[2]);
                time(&clock);
                mvprintw(0,40,ctime(&clock));
        	dispupdate();
        	mvprintw(23, 0, "CMD> ");
                refresh();
        	tin = 1;
        	i = select(2, &tin, (int *)0, (int *)0, &tintv);
        	if (i && tin) {
        		i = getchar();
        		if (i == 12)
				dispinit();
        		else if (i == 'q')
        			done();
        	}
        }
}

/*
 * DONE - put the term back in non CBREAK mode and exit.
 */
done()
{
	ioctl(0, TIOCGETP, &ttyb);
	ttyb.sg_flags &= ~CBREAK;
	ioctl(fileno(stdin), TIOCSETP, (char *)&ttyb);
	mvprintw(23,0,"\n");
	refresh();
	exit(1);
}
