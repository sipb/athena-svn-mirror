/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Get OS name information
 */

#include "defs.h"

/*
 * Get OS name using sysinfo() system call.
 */
extern char *GetOSNameSysinfo()
{
    static char			Buff[128];

#if	defined(HAVE_SYSINFO)
    if (Buff[0])
	return(Buff);

    if (sysinfo(SI_SYSNAME, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get name of OS using uname()
 */
extern char *GetOSNameUname()
{
#if	defined(HAVE_UNAME)
    static struct utsname 	un;

    if (uname(&un) >= 0)
	return(un.sysname);
#endif	/* HAVE_UNAME */

    return((char *)NULL);
}

/*
 * Use predefined value.
 */
extern char *GetOSNameDef()
{
#if	defined(OS_NAME)
    return(OS_NAME);
#else
    return((char *)NULL);
#endif	/* OS_NAME */
}

/*
 * Get Operating System name.
 */
extern int GetOSName(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	       GetOSNamePSI[];
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetOSNamePSI)) {
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

