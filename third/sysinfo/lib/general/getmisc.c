/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Miscellaneous Get*() functions
 */

#include "defs.h"

/*
 * Get host ID
 */
extern int GetHostID(Query)
     MCSIquery_t	      *Query;
{
    static char 		Buff[100];

    if (Query->Op == MCSIOP_CREATE) {
#if	defined(HAVE_GETHOSTID)
	(void) snprintf(Buff, sizeof(Buff),  "%08x", gethostid());
#endif	/* HAVE_GETHOSTID */

	if (Buff[0]) {
	    Query->Out = (Opaque_t) strdup(Buff);
	    Query->OutSize = strlen(Buff);
	    return 0;
	}
    } else if (Query->Op == MCSIOP_DESTROY) {
	if (Query->Out && Query->OutSize)
	    (void) free(Query->Out);
	return 0;
    }

    return -1;
}

#if	defined(HAVE_SYSINFO) && defined(SI_HW_SERIAL)
/*
 * Get Serial Number using sysinfo()
 */
extern char *GetSerialSysinfo()
{
    static char 		Buff[128];

    if (!Buff[0])
	if (sysinfo(SI_HW_SERIAL, Buff, sizeof(Buff)) < 0)
	    Buff[0] = CNULL;

    if (Buff[0]) {
	return(Buff);
    }

    return (char *)NULL;
}
#endif	/* HAVE_SYSINFO ... */

/*
 * Get serial number
 */
extern int GetSerial(Query)
     MCSIquery_t	       *Query;
{
    extern PSI_t	        GetSerialPSI[];
    char		       *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetSerialPSI)) {
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

/*
 * Get ROM Version
 */
extern int GetRomVer(Query)
     MCSIquery_t	       *Query;
{
    extern PSI_t	        GetRomVerPSI[];
    char		       *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetRomVerPSI)) {
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
