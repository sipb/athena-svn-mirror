/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
extern char *GetAppArchStr()
{
    extern PSI_t	       GetAppArchPSI[];
    static char		      *Str = NULL;

    if (!Str)
	Str = PSIquery(GetAppArchPSI);

    return Str;
}

/*
 * Get application architecture.
 */
extern int GetAppArch(Query)
     MCSIquery_t	      *Query;
{
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = GetAppArchStr()) {
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
extern char *GetKernArchStr()
{
    extern PSI_t	       GetKernArchPSI[];
    static char		      *Str = NULL;

    if (!Str)
	Str = PSIquery(GetKernArchPSI);

    return Str;
}

/*
 * Get kernel architecture
 */
extern int GetKernArch(Query)
     MCSIquery_t	      *Query;
{
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = GetKernArchStr()) {
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
