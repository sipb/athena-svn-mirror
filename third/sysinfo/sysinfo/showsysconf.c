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
 * Use the sysconf() routine to show system configuration info.
 * This routine is available on POSIX compliant systems.
 */
#include "defs.h"

/*
 * List valid arguments for the SysConf class.
 */
extern void SysConfList()
{
    register Define_t	       *DefPtr;

    DefPtr = (Define_t *) DefGetList(DL_SYSCONF); 
    if (!DefPtr) {
	SImsg(SIM_WARN, "No sysconf variables are defined.");
	return;
    }

    SImsg(SIM_INFO, "\n\nThe following are valid arguments for `-class SysConf -show Name1,Name2,...':\n\n");
    SImsg(SIM_INFO, "%-25s %s\n", "NAME", "DESCRIPTION");

    for ( ; DefPtr; DefPtr = DefPtr->Next)
	if (sysconf(DefPtr->KeyNum) > 0)
	    SImsg(SIM_INFO, "%-25s %s\n", DefPtr->KeyStr, DefPtr->ValStr2);
}

/*
 * Check to see if a symbol named String appears
 * in Argv.
 * Return 1 if found.
 * Return 0 if not found.
 */
static int HasName(String, Argv)
    char		       *String;
    char		      **Argv;
{
    register char	      **cpp;

    for (cpp = Argv; cpp && *cpp; ++cpp)
	if (EQ(String, *cpp))
	    return(1);

    return(0);
}

/*
 * Show System Configuration information utilizing the sysconf()
 * system call.
 */
extern void SysConfShow(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    static char		       *RptData[3];
    int				MaxDesc = 0;
    register int		Len;
    Define_t		       *SysConfDef;
    register Define_t	       *DefPtr;
    static MCSIquery_t		Query;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_SYSCONF;
    if (mcSysInfo(&Query) != 0)
	return;

    SysConfDef = (Define_t *) Query.Out;
    if (!SysConfDef) {
	SImsg(SIM_WARN, "No sysconf variables are available.");
	return;
    }

    ClassShowBanner(MyInfo);

    /*
     * Find the longest descriptive string
     */
    for (DefPtr = SysConfDef; DefPtr; DefPtr = DefPtr->Next) {
	if (Names && !HasName(DefPtr->KeyStr, Names))
	    continue;
	Len = strlen(DefPtr->ValStr2);
	if (VL_ALL)
	    Len += strlen(DefPtr->KeyStr) + 2;
	if (Len > MaxDesc)
	    MaxDesc = Len;
    }

    for (DefPtr = SysConfDef; DefPtr; DefPtr = DefPtr->Next) {
	if (Names && !HasName(DefPtr->KeyStr, Names))
	    continue;

	/* Skip entries that are not available */
	if (DefPtr->ValInt1 < 0)
	    continue;

	if (FormatType == FT_PRETTY) {
	    ClassShowValue(DefPtr->ValStr2, DefPtr->KeyStr, DefPtr->ValStr3, 
			   MaxDesc);
	} else if (FormatType == FT_REPORT) {
	    RptData[0] = DefPtr->ValStr2;
	    RptData[1] = DefPtr->ValStr3;
	    Report(CN_SYSCONF, NULL, DefPtr->KeyStr, RptData, 2);
	}
    }
}
