/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * System Configuration
 *
 * Use the sysconf() routine to retrieve system configuration info.
 * This routine is available on POSIX compliant systems.
 */
#include "defs.h"

/*
 * Show System Configuration information utilizing the sysconf()
 * system call.
 *
 * Returns ptr to Define_t in Query->Out:
 * 	KeyStr		SysConf Variable (string)
 *	ValStr1		Type of variable (Boolean, etc)
 *	ValStr2		Description of Variable
 *	ValStr3		Value (string)
 *	ValInt1		Value (integer)
 */
extern int GetSysConf(Query)
     MCSIquery_t	       *Query;
{
#if	defined(HAVE_SYSCONF)
    long			Value;
    int				NumValid = 0;
    char		       *ValuePtr;
    Define_t		       *SysConfDef;
    register Define_t	       *DefPtr;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    if (Query->Op == MCSIOP_DESTROY)
	return DefDestroy((Define_t *) Query->Out);
    /* Else do MCSIOP_CREATE */

    SysConfDef = DefGetList(DL_SYSCONF);
    if (!SysConfDef) {
	SImsg(SIM_DBG, "No sysconf variables are defined.");
	return -1;
    }

    for (DefPtr = SysConfDef; DefPtr; DefPtr = DefPtr->Next) {
	/*
	 * Call sysconf().  If it succeeds, then store the return value
	 * in ValInt1.  If it fails, set ValInt1 to -1 and continue on.
	 */
	Value = sysconf(DefPtr->KeyNum);
	if (Value < 0) {
	    SImsg(SIM_GERR, "sysconf(%s) failed: %s", DefPtr->KeyStr, SYSERR);
	    DefPtr->ValInt1 = -1;
	    continue;
	}
	DefPtr->ValInt1 = Value;

	/* Skip entries that are not available */
	if (DefPtr->ValInt1 < 0)
	    continue;

	if (EQ(DefPtr->ValStr1, "bool")) {
	    if (Value)
		ValuePtr = "TRUE";
	    else
		ValuePtr = "FALSE";
	} else
	    ValuePtr = strdup(itoa(Value));

	/* Store string value here */
	DefPtr->ValStr3 = ValuePtr;
	++NumValid;
    }

    Query->Out = (Opaque_t) SysConfDef;
    Query->OutSize = NumValid;
#endif	/* HAVE_SYSCONF */

    return 0;
}
