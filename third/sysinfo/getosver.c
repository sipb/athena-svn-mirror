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
 * Get OS version information
 */

#include "defs.h"

/*
 * Get OS version number using uname()
 */
extern char *GetOSVerUname()
{
    static char			Buff[64];
#if	defined(HAVE_UNAME)
    struct utsname 		un;

    if (uname(&un) >= 0) {
	/*
	 * Vendors don't all do the same thing for storing
	 * version numbers via uname().
	 */
#if	defined(UNAME_REL_VER_COMB)
	(void) snprintf(Buff, sizeof(Buff),  "%s.%s", un.version, un.release);
#else
	(void) snprintf(Buff, sizeof(Buff),  "%s", un.release);
#endif 	/* UNAME_REL_VER_COMB */
    }
#endif	/* HAVE_UNAME */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get OS version by reading an a specific argument out of
 * the kernel version string.
 */
extern char *GetOSVerKernVer()
{
    static char			Buff[256];
#if	defined(OSVERS_FROM_KERNVER)
    register char	       *cp;
    register int		i;

    if (!(cp = GetKernVer()))
	return((char *) NULL);

    (void) strcpy(Buff, cp);
    for (cp = strtok(Buff, " "), i = 0; cp && i != OSVERS_FROM_KERNVER-1; 
	 cp = strtok((char *)NULL, " "), ++i);
    (void) strcpy(Buff, cp);
    if ((cp = strchr(Buff, ':')) != NULL)
	*cp = C_NULL;
#endif	/* OSVERS_FROM_KERNVER */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get OS version using sysinfo() system call.
 */
extern char *GetOSVerSysinfo()
{
    static char			Buff[128];

#if	defined(HAVE_SYSINFO)
    if (Buff[0])
	return(Buff);

    if (sysinfo(SI_RELEASE, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Use predefined OS_NAME.
 */
extern char *GetOSVerDef()
{
#if	defined(OS_VERSION)
    return(OS_VERSION);
#else
    return((char *) NULL);
#endif	/* OS_VERSION */
}

/*
 * Get Operating System version
 */
extern char *GetOSVer()
{
    extern PSI_t	       GetOSVerPSI[];
    static char		      *Str;
    register char	      *cp;

    if (Str)
	return(Str);

    if (Str = PSIquery(GetOSVerPSI))
	/*
	 * Zap "*-PL*".
	 */
	if (*Str && ((cp = strrchr(Str, '-')) != NULL) && 
	    (strncmp(cp, "-PL", 3) == 0))
	    *cp = C_NULL;

    return(Str);
}
