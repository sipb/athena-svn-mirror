/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: getosver.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
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
    static char			Buf[BUFSIZ];
#if	defined(HAVE_UNAME)
    struct utsname 		un;

    if (uname(&un) >= 0) {
	/*
	 * Vendors don't all do the same thing for storing
	 * version numbers via uname().
	 */
#if	defined(UNAME_REL_VER_COMB)
	(void) sprintf(Buf, "%s.%s", un.version, un.release);
#else
	(void) sprintf(Buf, "%s", un.release);
#endif 	/* UNAME_REL_VER_COMB */
    }
#endif	/* HAVE_UNAME */

    return( (Buf[0]) ? Buf : (char *) NULL );
}

/*
 * Get OS version by reading an a specific argument out of
 * the kernel version string.
 */
extern char *GetOSVerKernVer()
{
    static char			Buf[BUFSIZ];
#if	defined(OSVERS_FROM_KERNVER)
    register char	       *cp;
    register int		i;

    if (!(cp = GetKernVer()))
	return(UnSupported);

    (void) strcpy(Buf, cp);
    for (cp = strtok(Buf, " "), i = 0; cp && i != OSVERS_FROM_KERNVER-1; 
	 cp = strtok((char *)NULL, " "), ++i);
    (void) strcpy(Buf, cp);
    if ((cp = strchr(Buf, ':')) != NULL)
	*cp = C_NULL;
#endif	/* OSVERS_FROM_KERNVER */

    return( (Buf[0]) ? Buf : (char *) NULL );
}

/*
 * Get OS version using sysinfo() system call.
 */
extern char *GetOSVerSysinfo()
{
    static char			buff[BUFSIZ];

#if	defined(HAVE_SYSINFO)
    if (buff[0])
	return(buff);

    if (sysinfo(SI_RELEASE, buff, sizeof(buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
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
    static char		      *Str = NULL;
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
