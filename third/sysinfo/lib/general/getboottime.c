/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Boot Time information
 */

#include <sys/types.h>	/* Must be before utmp.h */
#include <utmp.h>	/* Must be before defs.h */
#include "defs.h"
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
	if (Ptr->ut_type == BOOT_TIME)
	    DateStr = TimeToStr(Ptr->ut_time, NULL);

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
	    DateStr = TimeToStr(Ptr->ut_time, NULL);
	    break;
	}

    (void) free(UtmpBuff);
    return(DateStr);
}
#endif	/* BOOT_TIME */

#if	defined(HAVE_NLIST)
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
	    DateStr = TimeToStr(TimeVal, NULL);
	}
    }

    if (kd)
	KVMclose(kd);

    return(DateStr);
}
#endif	/* HAVE_NLIST */

/*
 * Get Boot Time
 */
extern int GetBootTime(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	        GetBootTimePSI[];
    char		       *Str = NULL;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetBootTimePSI)) {
	    Query->Out = (Opaque_t) strdup(Str);
	    Query->OutSize = strlen(Str);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}
