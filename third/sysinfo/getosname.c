/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: getosname.c,v 1.1.1.2 1998-02-12 21:32:03 ghudson Exp $";
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
    static char			buff[BUFSIZ];

#if	defined(HAVE_SYSINFO)
    if (buff[0])
	return(buff);

    if (sysinfo(SI_SYSNAME, buff, sizeof(buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
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

