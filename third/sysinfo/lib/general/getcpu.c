/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
extern int GetCpuType(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	       GetCpuTypePSI[];
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetCpuTypePSI)) {
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
 * Use sysconf() to find number of CPU's.
 */
extern char *GetNumCpuSysconf()
{
    int				Num = -1;
    char		       *NumStr = NULL;

#if	defined(_SC_NPROCESSORS_CONF)
    Num = (int) sysconf(_SC_NPROCESSORS_CONF);
    if (Num >= 0)
	NumStr = itoa(Num);
#endif	/* _SC_NPROCESSORS_CONF */

    return(NumStr);
}

/*
 * Get number of CPU's
 */
extern int GetNumCpu(Query)
     MCSIquery_t	      *Query;
{
    extern PSI_t	       GetNumCpuPSI[];
    char		      *Str;

    if (Query->Op == MCSIOP_CREATE) {
	if (Str = PSIquery(GetNumCpuPSI)) {
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
