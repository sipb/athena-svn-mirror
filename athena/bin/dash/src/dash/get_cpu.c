/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/get_cpu.c,v $
 * $Author: vanharen $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#ifndef	lint
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/bin/dash/src/dash/get_cpu.c,v 1.1 1991-09-03 11:12:51 vanharen Exp $";
#endif	lint

#include "mit-copyright.h"
#include <stdio.h> 
#include <nlist.h>
#include <sys/param.h>
#include <sys/file.h>

extern void exit();

#define KERNEL_FILE "/vmunix"
#define KMEM_FILE "/dev/kmem"

static struct nlist namelist[] = {	    /* namelist for vmunix grubbing */
#define LOADAV 0
    {"_avenrun"},
#define X_CP_TIME 1
    {"_cp_time"},
    {0}
};

long cp_time[4];
long cp_old[4];
long cp_change[4];

static xload_error(str1, str2)
char *str1, *str2;
{
    (void) fprintf(stderr,"xload: %s %s\n", str1, str2);
    exit(-1);
}

/* ARGSUSED */
void getcpu( w, closure, call_data )
     caddr_t	w;		/* unused */
     caddr_t	closure;	/* unused */
     caddr_t	call_data;	/* pointer to (double) return value */
{
  	double *loadavg = (double *)call_data;
	static int init = 0;
	static kmem;
	static long loadavg_seek;
	long change, total_change;
	register int i;
	extern void nlist();
	
	if(!init)   {
	    nlist( KERNEL_FILE, namelist);
	    if (namelist[X_CP_TIME].n_type == 0){
		xload_error("cannot get name list from", KERNEL_FILE);
		exit(-1);
	    }
	    loadavg_seek = namelist[X_CP_TIME].n_value;
	    kmem = open(KMEM_FILE, O_RDONLY);
	    if (kmem < 0) xload_error("cannot open", KMEM_FILE);
	    init = 1;
	}
	

	(void) lseek(kmem, loadavg_seek, 0);
	(void) read(kmem, (char *)cp_time, sizeof(cp_time));
	total_change = 0;
	for (i = 0; i < 4; i++) {
	    if (cp_time[i] < cp_old[i])
	      change = (int)
		((unsigned long)cp_time[i]-(unsigned long)cp_old[i]);
	    else
	      change = cp_time[i] - cp_old[i];
	    total_change += (cp_change[i] = change);
	    cp_old[i] = cp_time[i];
	}
	i = 3;
	*loadavg = (double)
	  0.999999 - ((float)cp_change[i] / (float)total_change);
	return;
}
