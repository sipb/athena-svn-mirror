/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: virtmem.c,v 1.1.1.2 1998-02-12 21:31:54 ghudson Exp $";
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
    off_t			Amount;
{
    static char			Buff[BUFSIZ];

    if (Amount == 0)
	return(UnSupported);

    (void) sprintf(Buff, "%s", GetSizeStr((u_long)Amount, KBYTES));

    return(Buff);
}

/*
 * Find amount of virtual memory using "anoninfo" symbol.
 */

#if	defined(ANONINFO_SYM) && !defined(HAVE_ANONINFO)
#	define			HAVE_ANONINFO
#endif	/* ANONINFO_SYM */
#if	defined(HAVE_ANONINFO)
#	include <vm/anon.h>
#if	!defined(ANONINFO_SYM)
#	define ANONINFO_SYM		"_anoninfo"
#endif
#endif	/* HAVE_ANONINFO */

extern char *GetVirtMemAnoninfo()
{
#if	defined(HAVE_ANONINFO)
    kvm_t		       *kd;
    static struct anoninfo	AnonInfo;
    off_t			Amount = 0;
    nlist_t		       *nlPtr;
    int				PageSize;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, ANONINFO_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return(0);

	if (CheckNlist(nlPtr))
	    return(0);

	if (KVMget(kd, nlPtr->n_value, (char *) &AnonInfo, 
		   sizeof(AnonInfo), KDT_DATA) >= 0)
	    Amount = (off_t) (AnonInfo.ani_free + AnonInfo.ani_resv);

	if (Amount) {
	    /*
	     * Try to avoid overflow
	     */
	    PageSize = getpagesize();
	    if (PageSize >= KBYTES)
		Amount = Amount * (PageSize / KBYTES);
	    else
		Amount = (Amount * PageSize) / KBYTES;
	}

	KVMclose(kd);
    }

    return(GetVirtMemStr(Amount));
#else	/* !HAVE_ANONINFO */
    return((char *) NULL);
#endif	/* HAVE_ANONINFO */
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
    off_t			Amount = 0;
    nlist_t		       *nlPtr;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, NSWAP_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return(0);

	if (CheckNlist(nlPtr))
	    return(0);

	if (KVMget(kd, nlPtr->n_value, (char *) &Nswap,
		   sizeof(Nswap), KDT_DATA) >= 0)
	    Amount = Nswap;

	Amount /= KBYTES / NSWAP_SIZE;

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
