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
 * Kernel related functions.
 */

#include "defs.h"

#if	defined(HAVE_KVM)
#if !defined(VERSION_SYM)
#	define VERSION_SYM 	"_version"
#endif

/*
 * Get kernel version string by reading the
 * symbol "version" from the kernel.
 */
extern char *GetKernVerSym()
{
    nlist_t	 	       *nlptr;
    static char			Buf[256];
    register char	       *p;
    kvm_t		       *kd;

    if (kd = KVMopen()) {
	if ((nlptr = KVMnlist(kd, VERSION_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return((char *) NULL);

	if (CheckNlist(nlptr))
	    return((char *) NULL);

	if (KVMget(kd, nlptr->n_value, (char *) Buf, sizeof(Buf), KDT_STRING)){
	    SImsg(SIM_GERR, "Read of \"%s\" from kernel failed.",
			     VERSION_SYM);
	    Buf[0] = C_NULL;
	}
    }

    if (kd)
	KVMclose(kd);

#if	defined(KERNSTR_END)
    /*
     * Truncate extraneous info
     */
    if (Buf[0])
	if ((p = strchr(Buf, KERNSTR_END)) != NULL)
	    *p = C_NULL;
#endif	/* KERNSTR_END */

    return( (Buf[0]) ? Buf : (char *) NULL);
}
#endif	/* HAVE_KVM */

/*
 * Get kernel version string
 */
extern char *GetKernVer()
{
    extern PSI_t	       GetKernVerPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetKernVerPSI));
}

