/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: boottime.c,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $";
#endif

/*
 * Boot Time information
 */

#include "defs.h"
#include <utmp.h>
#include <sys/stat.h>
#include <sys/time.h>

#if	defined(BOOT_TIME)

#ifndef UTMP_FILE
#define UTMP_FILE		"/etc/utmp"
#endif

/*
 * Look in utmp for the system boot time
 */
extern char *GetBootTimeUtmp()
{
    struct utmp		       *Ptr;
    struct utmp		       *UtmpBuff;
    struct stat			StatBuff;
    int				fd;
    size_t			Amt;
    char		       *DateStr = NULL;
    register char	       *cp;

    if (stat(UTMP_FILE, &StatBuff) < 0) {
	if (Debug) Error("stat failed: %s: %s", UTMP_FILE, SYSERR);
	return((char *) NULL);
    }
    Amt = StatBuff.st_size;

    if ((fd = open(UTMP_FILE, O_RDONLY)) < 0) {
	if (Debug) Error("open O_RDONLY failed: %s: %s", UTMP_FILE, SYSERR);
	return((char *) NULL);
    }

    UtmpBuff = (struct utmp *) xmalloc(Amt);
    if (read(fd, (char *)UtmpBuff, Amt) != Amt) {
	if (Debug) Error("read failed: %s: %s", UTMP_FILE, SYSERR);
	(void) close(fd);
	return((char *) NULL);
    }
    (void) close(fd);

    for (Ptr = UtmpBuff; Ptr < UtmpBuff + Amt; ++Ptr) 
	if (Ptr->ut_type == BOOT_TIME) {
	    DateStr = ctime(&(Ptr->ut_time));
	    if (cp = strchr(DateStr, '\n'))
		*cp = CNULL;
	    break;
	}

    (void) free(UtmpBuff);
    return(DateStr);
}
#endif	/* BOOT_TIME */

#if !defined(BOOTTIME_SYM)
#	define BOOTTIME_SYM 	"_boottime"
#endif

/*
 * Get kernel BOOTTIME string by reading the
 * symbol "_boottime" from the kernel.
 */
extern char *GetBootTimeSym()
{
    nlist_t	 	       *nlptr;
    static struct timeval	BootTime;
    time_t			TimeVal;
    register char	       *cp;
    char		       *DateStr = NULL;
    kvm_t		       *kd;

    if (kd = KVMopen()) {
	if ((nlptr = KVMnlist(kd, BOOTTIME_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return((char *) NULL);

	if (CheckNlist(nlptr))
	    return((char *) NULL);

	if (KVMget(kd, nlptr->n_value, (char *) &BootTime, 
		   sizeof(BootTime), KDT_DATA)){
	    if (Debug) Error("Read of \"%s\" from kernel failed.",
			     BOOTTIME_SYM);
	} else {
	    TimeVal = BootTime.tv_sec;
	    DateStr = ctime(&TimeVal);
	    if (cp = strchr(DateStr, '\n'))
		*cp = CNULL;
	}
    }

    if (kd)
	KVMclose(kd);

    return(DateStr);
}

/*
 * Get Boot Time
 */
extern char *GetBootTime()
{
    extern PSI_t	       GetBootTimePSI[];

    return(PSIquery(GetBootTimePSI));
}
