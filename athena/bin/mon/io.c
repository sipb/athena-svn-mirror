/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/mon/io.c,v $
 *	$Author: dgg $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/io.c,v 1.1 1984-12-13 12:00:44 dgg Exp $
 */

#ifndef lint
static char *rcsid_io_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/mon/io.c,v 1.1 1984-12-13 12:00:44 dgg Exp $";
#endif	lint

/*
 *      I O
 *
 * Purpose: Read the kernel's I/O statistics and display selected
 *      parameters about terminals and disks.
 *
 */

#include "mon.h"

double	kbps, tps;	/* Kbytes/sec and xfers/sec */
int	lasty;		/* Last disk y location */

/* Temporary Defines - All output should be moved to dispinfo */
#include <curses.h>
#define CHARX   37
#define	CHARY	5
#define	DISKX	55
#define	DISKY	2
#define	DISKOFF	6
#define	DISKWDS	DISKOFF
#define	DISKXFER DISKOFF+4
#define	DISKSEEK DISKOFF+8

io()
{
        int y;
        register i;
	long	t;
	double	tkbps, ttps;	/* Totals for all drives */

        lseek(kmem, (long)namelist[X_DK_TIME].n_value, 0);
        read(kmem, s.dk_time, sizeof s.dk_time);
        lseek(kmem, (long)namelist[X_DK_XFER].n_value, 0);
        read(kmem, s.dk_xfer, sizeof s.dk_xfer);
        lseek(kmem, (long)namelist[X_DK_WDS].n_value, 0);
        read(kmem, s.dk_wds, sizeof s.dk_wds);
        lseek(kmem, (long)namelist[X_TK_NIN].n_value, 0);
        read(kmem, &s.tk_nin, sizeof s.tk_nin);
        lseek(kmem, (long)namelist[X_TK_NOUT].n_value, 0);
        read(kmem, &s.tk_nout, sizeof s.tk_nout);
        lseek(kmem, (long)namelist[X_DK_SEEK].n_value, 0);
        read(kmem, s.dk_seek, sizeof s.dk_seek);
        lseek(kmem, (long)namelist[X_DK_MSPW].n_value, 0);
        read(kmem, s.dk_mspw, sizeof s.dk_mspw);
        lseek(kmem, (long)namelist[X_HZ].n_value, 0);
        read(kmem, &hz, sizeof hz);
        for (i = 0; i < DK_NDRIVE; i++) {
#define X(fld)  t = s.fld[i]; s.fld[i] -= s1.fld[i]; s1.fld[i] = t
                X(dk_xfer); X(dk_seek); X(dk_wds); X(dk_time);
        }
        t = s.tk_nin; s.tk_nin -= s1.tk_nin; s1.tk_nin = t;
        t = s.tk_nout; s.tk_nout -= s1.tk_nout; s1.tk_nout = t;

	/* output character I/O counts */
        mvprintw(CHARY+1,CHARX+4,"%4.0f %5.0f", s.tk_nin/etime, s.tk_nout/etime);

	/* Display Drive statistics */
	tkbps = ttps = 0;
        for (i=0; i<DK_NDRIVE; i++) {
                if (stats(i)) {
	        	tkbps += kbps;
        		ttps += tps;
                }
        }
	/* Display Totals */
	mvprintw(lasty+2, DISKX+DISKOFF, "%4.0f%4.0f", tkbps, ttps);
}

/*
 * Display statistics for dk drive number dn.
 *  Returns 1 if totals should be updated.
 */
stats(dn)
{
        int y;
        register i;
        double atime, words, xtime, itime;

	/* only display info for named (real) devices */
	if (!*dr_name[dn])
		return(0);

        y = dn + DISKY+1;
	lasty = y;
        if (s.dk_mspw[dn] == 0.0) {
                mvprintw(y,DISKX+DISKOFF,"%4.0f%4.0f%6.1f ", 0.0, 0.0, 0.0);
                return(0);
        }
        atime = s.dk_time[dn];
        atime /= (float) hz;
        words = s.dk_wds[dn]*32.0;      /* number of 16 bit words transferred */
        xtime = s.dk_mspw[dn]*words;    /* transfer time */
        itime = atime - xtime;          /* time not transferring (seek time) */

        if (xtime < 0)
                itime += xtime, xtime = 0;
        if (itime < 0)
                xtime += itime, itime = 0;

	/* Avg Kbps, transfers/sec, avg seek time (in msec) */
	kbps = words/512/etime;
	tps = s.dk_xfer[dn]/etime;
        mvprintw(y,DISKX+DISKWDS,"%4.0f", kbps);
        mvprintw(y,DISKX+DISKXFER,"%4.0f", tps);
        mvprintw(y,DISKX+DISKSEEK,"%6.1f ",
            s.dk_seek[dn] ? itime*1000./s.dk_seek[dn] : 0.0);

	return(1);	/* Update totals */
}


