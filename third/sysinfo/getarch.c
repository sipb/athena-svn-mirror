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
 * Get architecture information
 */

#include "defs.h"

extern char		       *AppArchCmds[];
extern char		       *AppArchFiles[];
extern char		       *KernArchCmds[];
extern char		       *KernArchFiles[];

/*
 * Get kernel arch using sysinfo() system call.
 */
extern char *GetKernArchSysinfo()
{
    static char			Buff[128];

#if	defined(HAVE_SYSINFO)
    if (Buff[0])
	return(Buff);

    if (sysinfo(SI_MACHINE, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);

#endif	/* HAVE_SYSINFO */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get kernel arch using uname() system call.
 */
extern char *GetKernArchUname()
{
    static char			Buff[128];
#if	defined(HAVE_UNAME)
    static struct utsname	un;

    if (Buff[0])
	return(Buff);

    if (uname(&un) < 0)
	return((char *) NULL);

    strcpy(Buff, un.machine);
#endif	/* HAVE_UNAME */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get application arch using sysinfo() system call.
 */
extern char *GetAppArchSysinfo()
{
    static char			Buff[128];

#if	defined(HAVE_SYSINFO)
    if (Buff[0])
	return(Buff);

    if (sysinfo(SI_ARCHITECTURE, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Use predefined name.
 */
extern char *GetAppArchDef()
{
#if	defined(ARCH_TYPE)
    return(ARCH_TYPE);
#else
    return((char *)NULL);
#endif	/* ARCH_TYPE */
}

/*
 * Try running the App Arch test commands
 */
extern char *GetAppArchCmds()
{
    return(RunCmds(AppArchCmds, FALSE));
}

/*
 * Try testing Architecture files
 */
extern char *GetAppArchTest()
{
    return(RunTestFiles(AppArchFiles));
}

/*
 * Get application architecture.
 */
extern char *GetAppArch()
{
    extern PSI_t	       GetAppArchPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetAppArchPSI));
}

/*
 * Try running the KArch test commands
 */
extern char *GetKernArchCmds()
{
    return(RunCmds(KernArchCmds, FALSE));
}

/*
 * Try testing Kernel Architecture files
 */
extern char *GetKernArchTest()
{
    return(RunTestFiles(KernArchFiles));
}

/*
 * Use predefined name.
 */
extern char *GetKernArchDef()
{
#if	defined(KARCH_TYPE)
    return(KARCH_TYPE);
#else
    return((char *)NULL);
#endif	/* KARCH_TYPE */
}

/*
 * Get kernel architecture
 */
extern char *GetKernArch()
{
    extern PSI_t	       GetKernArchPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetKernArchPSI));
}
