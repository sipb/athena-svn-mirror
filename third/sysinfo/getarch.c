/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: getarch.c,v 1.1.1.1 1996-10-07 20:16:51 ghudson Exp $";
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
    static char			buff[BUFSIZ];

#if	defined(HAVE_SYSINFO)
    if (buff[0])
	return(buff);

    if (sysinfo(SI_MACHINE, buff, sizeof(buff)) < 0)
	return((char *) NULL);

#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
}

/*
 * Get kernel arch using uname() system call.
 */
extern char *GetKernArchUname()
{
    static char			buff[BUFSIZ];
#if	defined(HAVE_UNAME)
    static struct utsname	un;

    if (buff[0])
	return(buff);

    if (uname(&un) < 0)
	return((char *) NULL);

    strcpy(buff, un.machine);
#endif	/* HAVE_UNAME */

    return( (buff[0]) ? buff : (char *) NULL );
}

/*
 * Get application arch using sysinfo() system call.
 */
extern char *GetAppArchSysinfo()
{
    static char			buff[BUFSIZ];

#if	defined(HAVE_SYSINFO)
    if (buff[0])
	return(buff);

    if (sysinfo(SI_ARCHITECTURE, buff, sizeof(buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
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
