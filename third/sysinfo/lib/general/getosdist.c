/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Get OS Distribution information
 */

#include "defs.h"

/*
 * Use predefined value.
 */
extern char *GetOSDistDef()
{
#if	defined(OS_RELEASE)
    return(OS_RELEASE);
#else
    return((char *)NULL);
#endif	/* OS_RELEASE */
}

/*
 * Get Operating System Dist string
 */
extern int GetOSDist(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	       GetOSDistPSI[];
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetOSDistPSI)) {
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
