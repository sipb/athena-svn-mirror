/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.2 $";
#endif

/*
 * Virtual Memory related functions.
 */

#include "defs.h"

/*
 * Change memory Amount into a string.
 * Amount should always be in kilobytes.
 */
extern char *GetVirtMemStr(Amount)
    Large_t			Amount;
{
    static char			Buff[32];

    if (Amount == 0)
	return((char *) NULL);

    (void) snprintf(Buff, sizeof(Buff), "%s", GetSizeStr(Amount, KBYTES));

    return(Buff);
}

/*
 * Find amount of virtual memory using "anoninfo" symbol.
 */

#if	defined(HAVE_ANONINFO) && !defined(HAVE_SWAPCTL)
#	include <vm/anon.h>
#if	!defined(ANONINFO_SYM)
#	define ANONINFO_SYM		"_anoninfo"
#endif
#endif	/* HAVE_ANONINFO && !HAVE_SWAPCTL */

extern char *GetVirtMemAnoninfo()
{
#if	defined(HAVE_ANONINFO) && !defined(HAVE_SWAPCTL)
    kvm_t		       *kd;
    static struct anoninfo	AnonInfo;
    Large_t			Amount = 0;
    nlist_t		       *nlPtr;
    int				PageSize;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, ANONINFO_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return(0);

	if (CheckNlist(nlPtr))
	    return(0);

	if (KVMget(kd, nlPtr->n_value, (char *) &AnonInfo, 
		   sizeof(AnonInfo), KDT_DATA) >= 0)
	    Amount = (Large_t)AnonInfo.ani_max;

	if (Amount) {
	    /*
	     * Try to avoid overflow
	     */
	    PageSize = getpagesize();
	    if (PageSize >= KBYTES)
		Amount = Amount * ((Large_t)((PageSize / KBYTES)));
	    else
		Amount = (Amount * (Large_t)PageSize) / 
		    (Large_t)KBYTES;
	    SImsg(SIM_DBG, "GetVirtMemAnon: Amount=%.0f PageSize=%d",
		  (float) Amount, PageSize);
	}

	KVMclose(kd);
    }

    return(GetVirtMemStr(Amount));
#else	/* !HAVE_ANONINFO */
    return((char *) NULL);
#endif	/* HAVE_ANONINFO && !HAVE_SWAPCTL*/
}

/*
 * Find amount of virtual memory using swapctl()
 */

#if	defined(HAVE_SWAPCTL)
#	include <sys/stat.h>
#	include <sys/swap.h>
#endif	/* HAVE_SWAPCTL */

extern char *GetVirtMemSwapctl()
{
    char		       *ValStr = NULL;
    Large_t			Amount = 0;
#if	defined(HAVE_ANONINFO) && defined(HAVE_SWAPCTL)
    static struct anoninfo	AnonInfo;
    int				PageSize;

    if (swapctl(SC_AINFO, &AnonInfo) == -1) {
	SImsg(SIM_GERR, "swapctl(SC_AINFO) failed: %s", SYSERR);
	return((char *) NULL);
    }

    SImsg(SIM_DBG, "GetVirtMemSwapctl: max=%d free=%d resv=%d",
	  AnonInfo.ani_max, AnonInfo.ani_free, AnonInfo.ani_resv);

    Amount = (Large_t)AnonInfo.ani_max;

    if (Amount) {
	/*
	 * Try to avoid overflow
	 */
	PageSize = getpagesize();
	if (PageSize >= KBYTES)
	    Amount = Amount * ((Large_t)((PageSize / KBYTES)));
	else
	    Amount = (Amount * (Large_t)PageSize) / (Large_t)KBYTES;
	SImsg(SIM_DBG, "GetVirtMemAnon: Amount=%.0f PageSize=%d",
	      (float) Amount, PageSize);
    }

    ValStr = GetVirtMemStr(Amount);
#endif	/* HAVE_ANONINFO && HAVE_SWAPCTL */

#if	defined(HAVE_SWAPCTL) && defined(SC_GETLSWAPTOT)
    off_t			lswaptot;

    if (swapctl(SC_GETLSWAPTOT, &lswaptot) == -1) {
	SImsg(SIM_GERR, "swapctl(SC_GETLSWAPTOT) failed: %s", SYSERR);
	return((char *) NULL);
    }

    SImsg(SIM_DBG, "SC_GETLSWAPTOT = %.0f blocks", (float) lswaptot);

    Amount = (Large_t) ((lswaptot * 512) / 1024);

    ValStr = GetVirtMemStr(Amount);
#endif	/* HAVE_SWAPCTL && SC_GETLSWAPTOT */
    
    return(ValStr);
}

/*
 * Use the "nswap" symbol to determine amount of
 * virtual memory.
 */
#if	defined(NSWAP_SYM) && !defined(HAVE_NSWAP)
#	define HAVE_NSWAP
#endif	/* NSWAP_SYM */

#if	defined(HAVE_NSWAP)

#if	!defined(NSWAP_SIZE)
#	define NSWAP_SIZE	512
#endif	/* NSWAP_SIZE */
#if	!defined(NSWAP_SYM)
#	define NSWAP_SYM	"_nswap"
#endif	/* NSWAP_SYM */

#endif	/* HAVE_NSWAP */

extern char *GetVirtMemNswap()
{
#if	defined(HAVE_NSWAP)
    kvm_t		       *kd;
    int				Nswap;
    Large_t			Amount = 0;
    nlist_t		       *nlPtr;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, NSWAP_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return(0);

	if (CheckNlist(nlPtr))
	    return(0);

	if (KVMget(kd, nlPtr->n_value, (char *) &Nswap,
		   sizeof(Nswap), KDT_DATA) >= 0)
	    Amount = (Large_t)Nswap;

	Amount /= (Large_t)KBYTES / (Large_t)NSWAP_SIZE;
	SImsg(SIM_DBG, "GetVirtMemNswap: Amount=%.0f Nswap=%d",
	      (float) Amount, Nswap);

	KVMclose(kd);
    }

    return(GetVirtMemStr(Amount));
#else	/* !HAVE_NSWAP */
    return((char *) NULL);
#endif	/* HAVE_NSWAP */
}

/*
 * Get Virtual Memory
 */
extern char *
GetVirtMem()
{
    extern PSI_t	       GetVirtMemPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetVirtMemPSI));
}
