/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/vm.c,v $
 *	$Author: epeisach $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/vm.c,v 1.4 1990-03-26 15:39:30 epeisach Exp $
 */

#ifndef lint
static char *rcsid_vm_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/vm.c,v 1.4 1990-03-26 15:39:30 epeisach Exp $";
#endif	lint

/*
 *      V M
 *
 * Purpose:  Read the system virtual memory status info from kernel
 *      space and write it onto the virtual screen used by curses(3).
 *
 * Bugs:  This routime (like the others) it does its own
 *      printing.  This makes it very difficult to change the screen
 *      format.  A better solution would be to move all the prints
 *      into a screen update function.
 */

#include "mon.h"
#ifdef ultrix
#include <machine/param.h>	/* defines bytes/page */
#else
#include <machine/machparam.h>	/* defines bytes/page */
#endif

/* Temporary defines */
#define	PROCS	2
#define	CPUY	5
#define	TIMEY	8
#define	PAGE	11

vm()
{
        register i,j;
	long	t;		/* temporary */

        lseek(kmem, (long)namelist[X_CP_TIME].n_value, 0);
        read(kmem, s.cp_time, sizeof s.cp_time);
	/* Check for 2nd CPU stats */
	if (dualcpu) {
	        lseek(kmem, (long)namelist[X_CP_TIME2].n_value, 0);
	        read(kmem, s.cp_time2, sizeof s.cp_time2);
	}
        lseek(kmem, (long)namelist[X_DK_XFER].n_value, 0);
        read(kmem, s.dk_xfer, sizeof s.dk_xfer);
        lseek(kmem, (long)namelist[X_RATE].n_value, 0);
        read(kmem, &rate, sizeof rate);
        lseek(kmem, (long)namelist[X_TOTAL].n_value, 0);
        read(kmem, &total, sizeof total);
        lseek(kmem, (long)namelist[X_DEFICIT].n_value, 0);
        read(kmem, &deficit, sizeof deficit);
        etime = 0;
        for (i=0; i < CPUSTATES; i++) {
                t = s.cp_time[i];
                s.cp_time[i] -= s1.cp_time[i];
                s1.cp_time[i] = t;
        	if (dualcpu) {
	                t = s.cp_time2[i];
        	        s.cp_time2[i] -= s1.cp_time2[i];
                	s1.cp_time2[i] = t;
        	}
                etime += s.cp_time[i];	/* interval must count 1 CPU only */
        }
        if(etime == 0.)
                etime = 1.;
        etime /= (float) hz;

	/* Display the procs line */
        mvprintw(PROCS+1,6,"%2d%2d%2d%2d %2d", total.t_rq, total.t_dw, total.t_pw, total.t_sw, total.t_sl);
#define pgtok(a) ((a)*NBPG/1024)
        mvprintw(PROCS+1,23,"%5d %5d", pgtok(total.t_rm), pgtok(total.t_arm) );
        mvprintw(PROCS+1,34,"%6d %5d", pgtok(total.t_vm), pgtok(total.t_avm) );
        mvprintw(PROCS+1,47,"%5d", pgtok(total.t_free));

	/* Display paging info */
        mvprintw(PAGE+1,6,"%4d %3d",
                (rate.v_pgrec - (rate.v_xsfrec+rate.v_xifrec)),
                (rate.v_xsfrec+rate.v_xifrec));
        mvprintw(PAGE+1,14,"%4d %4d", pgtok(rate.v_pgpgin),
                pgtok(rate.v_pgpgout));
	/* operations per time is (pgin + pgout)  */
        mvprintw(PAGE+1,24,"%4d", (pgtok(rate.v_pgin)+
                pgtok(rate.v_pgout)));
        mvprintw(PAGE+1,29,"%4d %4d %4d", pgtok(rate.v_dfree)
                , pgtok(deficit), rate.v_scan);

	/* Display CPU info */
        mvprintw(CPUY+1,4,"%4d  %4d", 
#if defined(vax) || defined(mips)
	(rate.v_intr) - hz, rate.v_syscall);
#endif
#if defined(sun) || defined(ibm032)
	rate.v_intr, rate.v_syscall);
#endif
/* if not vax, mips, sun or ibm, a syntax error will result */
        mvprintw(CPUY+1,17,"%4d", rate.v_swtch);
        cputime();

	/* Display additional stuff */
	mvprintw(PAGE+4,6,"%4d%4d %4d%4d %4d%4d %4d %4d%4d",
		rate.v_nexfod, rate.v_exfod,
		rate.v_nzfod, rate.v_zfod,
		rate.v_nvrfod, rate.v_vrfod,
		rate.v_pgfrec,
		rate.v_swpin, rate.v_swpout);
}


/*
 * Display cpu time info (%time in each state)
 */
cputime()
{
        int x;
        double t, t2;
        register i;

        t = t2 = 0;
        for(i=0; i<CPUSTATES; i++) {
                t += s.cp_time[i];
                t2 += s.cp_time2[i];
        }
	if(t == 0.)
		t = 1.;
	if (t2 == 0.)
		t2 = 1.;
        x = 6;
        for(i=0; i<CPUSTATES; i++){
                mvprintw(TIMEY+1,x,"%3.0f", 100 * s.cp_time[i]/t);
        	if (dualcpu)
	                mvprintw(TIMEY+1,x+27,"%3.0f", 100 * s.cp_time2[i]/t2);
                x += 5;
        }
}
