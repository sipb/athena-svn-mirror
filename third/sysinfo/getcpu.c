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
 * Get cpu type information
 */

#include "defs.h"

/*
 * Get cpu type using sysinfo(SI_ISALIST) system call.
 */
extern char *GetCpuTypeIsalist()
{
    static char			Buff[128];
    register char	       *cp;

#if	defined(SI_ISALIST)
    if (Buff[0])
	return(Buff);

    if (sysinfo(SI_ISALIST, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);

    /* We want only the first argument */
    if (cp = strchr(Buff, ' '))
	*cp = CNULL;
#endif	/* SI_ISALIST */

    return( (Buff[0]) ? Buff : (char *) NULL );
}

/*
 * Get cpu type using sysinfo() system call.
 */
extern char *GetCpuTypeSysinfo()
{
    static char			buff[128];

#if	defined(HAVE_SYSINFO)
    if (buff[0])
	return(buff);

    if (sysinfo(SI_ARCHITECTURE, buff, sizeof(buff)) < 0)
	return((char *) NULL);
#endif	/* HAVE_SYSINFO */

    return( (buff[0]) ? buff : (char *) NULL );
}

/*
 * Use predefined CPU_NAME.
 */
extern char *GetCpuTypeDef()
{
#if	defined(CPU_NAME)
    return(CPU_NAME);
#else	/* !CPU_NAME */
    return((char *)NULL);
#endif	/* CPU_NAME */
}

/*
 * Run Test Files
 */
extern char *GetCpuTypeTest()
{
    extern char		       *CPUFiles[];

    return(RunTestFiles(CPUFiles));
}

/*
 * Run Test Commands
 */
extern char *GetCpuTypeCmds()
{
    extern char		       *AppArchCmds[];

    return(RunCmds(AppArchCmds, FALSE));
}

/*
 * Get CPU type
 */
extern char *GetCpuType()
{
    extern PSI_t	       GetCpuTypePSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetCpuTypePSI));
}

/*
 * Use sysconf() to find number of CPU's.
 */
extern char *GetNumCpuSysconf()
{
    int				Num = -1;
    static char		       *NumStr = NULL;

    if (NumStr)
	return(NumStr);

#if	defined(_SC_NPROCESSORS_CONF)
    Num = (int) sysconf(_SC_NPROCESSORS_CONF);
    if (Num >= 0) {
	NumStr = itoa(Num);
	if (NumStr)
	    NumStr = strdup(NumStr);
    }
#endif	/* _SC_NPROCESSORS_CONF */

    return(NumStr);
}

/*
 * Get number of CPU's
 */
extern char *GetNumCpu()
{
    extern PSI_t	       GetNumCpuPSI[];
    static char		      *Str = NULL;

    if (Str)
	return(Str);

    return(Str = PSIquery(GetNumCpuPSI));
}
