/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Kernel related functions.
 */

#include "defs.h"

#if !defined(VERSION_SYM)
#	define VERSION_SYM 	"_version"
#endif

/*
 * Get kernel version string by reading the
 * symbol "version" from the kernel.
 */
extern char *GetKernVerSym()
{
    static char			Buf[256];
#if	defined(HAVE_NLIST)
    nlist_t	 	       *nlptr;
    register char	       *p;
    kvm_t		       *kd;

    if (kd = KVMopen()) {
	if ((nlptr = KVMnlist(kd, VERSION_SYM, (nlist_t *)NULL, 0)) == NULL) {
	    KVMclose(kd);
	    return((char *) NULL);
	}

	if (CheckNlist(nlptr)) {
	    KVMclose(kd);
	    return((char *) NULL);
	}

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
#endif	/* HAVE_NLIST */
    return( (Buf[0]) ? Buf : (char *) NULL);
}

/*
 * Get kernel version string
 */
extern int GetKernVer(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	       GetKernVerPSI[];
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetKernVerPSI)) {
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
