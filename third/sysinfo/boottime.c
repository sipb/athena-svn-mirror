/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.3 $";
#endif

/*
 * Boot Time information
 */

#include "defs.h"
#include <utmp.h>
#include <sys/stat.h>
#include <sys/time.h>

#if	defined(BOOT_TIME) && defined(HAVE_GETUTID)
/*
 * Look in utmp for the system boot time using getutid()
 */
extern char *GetBootTimeGetutid()
{
    static struct utmp		Utmp;
    struct utmp		       *Ptr;
    char		       *DateStr = NULL;
    register char	       *cp;

    setutent();
    memset(&Utmp, 0, sizeof(Utmp));
    Utmp.ut_type = BOOT_TIME;

    if (Ptr = getutid(&Utmp))
	if (Ptr->ut_type == BOOT_TIME) {
	    DateStr = (char *) ctime(&(Ptr->ut_time));
	    if (cp = strchr(DateStr, '\n'))
		*cp = CNULL;
	}

    endutent();

    return(DateStr);
}
#endif	/* BOOT_TIME && HAVE_GETUTID */

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
	SImsg(SIM_GERR, "stat failed: %s: %s", UTMP_FILE, SYSERR);
	return((char *) NULL);
    }
    Amt = StatBuff.st_size;

    if ((fd = open(UTMP_FILE, O_RDONLY)) < 0) {
	SImsg(SIM_GERR, "open O_RDONLY failed: %s: %s", UTMP_FILE, SYSERR);
	return((char *) NULL);
    }

    UtmpBuff = (struct utmp *) xmalloc(Amt);
    if (read(fd, (char *)UtmpBuff, Amt) != Amt) {
	SImsg(SIM_GERR, "read failed: %s: %s", UTMP_FILE, SYSERR);
	(void) close(fd);
	return((char *) NULL);
    }
    (void) close(fd);

    for (Ptr = UtmpBuff; Ptr < UtmpBuff + Amt; ++Ptr) 
	if (Ptr->ut_type == BOOT_TIME) {
	    DateStr = (char *) ctime(&(Ptr->ut_time));
	    if (cp = strchr(DateStr, '\n'))
		*cp = CNULL;
	    break;
	}

    (void) free(UtmpBuff);
    return(DateStr);
}
#endif	/* BOOT_TIME */

#if	defined(HAVE_KVM) && defined(HAVE_NLIST)
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
	    SImsg(SIM_GERR, "Read of \"%s\" from kernel failed.", 
		  BOOTTIME_SYM);
	} else {
	    TimeVal = BootTime.tv_sec;
	    DateStr = (char *) ctime(&TimeVal);
	    if (cp = strchr(DateStr, '\n'))
		*cp = CNULL;
	}
    }

    if (kd)
	KVMclose(kd);

    return(DateStr);
}
#endif	/* HAVE_KVM && HAVE_NLIST */

/*
 * Get Boot Time
 */
extern char *GetBootTime()
{
    extern PSI_t	       GetBootTimePSI[];

    return(PSIquery(GetBootTimePSI));
}
