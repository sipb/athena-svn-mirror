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
extern char *GetOSName()
{
    extern PSI_t	       GetOSNamePSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetOSNamePSI));
}

