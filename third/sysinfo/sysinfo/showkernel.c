/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Show Kernel related functions.
 */

#include "defs.h"

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
    register char	       *cp;

    cp = String;
    if (*cp == '_')
	++cp;

    for (cpp = Argv; cpp && *cpp; ++cpp)
	if (EQ(cp, *cpp))
	    return(1);

    return(0);
}

/*
 * Show kernel variables
 */
extern void KernelShow(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    Define_t		       *KernDef;
    Define_t		       *DefPtr;
    static char		       *RptData[3];
    int				Len;
    int				MaxLen = 0;
    static MCSIquery_t		Query;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_KERNELVAR;
    if (mcSysInfo(&Query) != 0)
	return;

    KernDef = (Define_t *) Query.Out;
    if (!KernDef) {
	SImsg(SIM_WARN, "No kernel variables are defined.");
	return;
    }

    ClassShowBanner(MyInfo);

    if (!VL_BRIEF || !VL_TERSE)
	/*
	 * Find the longest descriptive string for later use.
	 */
	for (DefPtr = KernDef; DefPtr; DefPtr = DefPtr->Next) {
	    if (!DefPtr->ValStr3 || (Names && HasName(DefPtr->KeyStr, Names)))
		continue;
	    Len = 0;
	    if (DefPtr->ValStr2)
		Len = (int) strlen(DefPtr->ValStr2);
	    if (VL_ALL) {
		Len += (int) strlen(DefPtr->KeyStr);
		Len += 2;	/* () */
	    }
	    Len += 4;		/* _is_ */
	    if (Len > MaxLen)
		MaxLen = Len;
	}

    /*
     * For each variable in the definetion list that we have an
     * address for, read the value from the kernel using a data type
     * specific function.
     */
    for (DefPtr = KernDef; DefPtr; DefPtr = DefPtr->Next) {
	if (!DefPtr->ValStr3 || (Names && HasName(DefPtr->KeyStr, Names)))
	    continue;

	if (FormatType == FT_PRETTY) {
	    ClassShowValue(DefPtr->ValStr2, DefPtr->KeyStr, 
			   DefPtr->ValStr3, MaxLen);
	} else if (FormatType == FT_REPORT) {
	    RptData[0] = DefPtr->ValStr2;
	    RptData[1] = DefPtr->ValStr3;
	    Report(CN_KERNEL, NULL, DefPtr->KeyStr, RptData, 2);
	}
    }
}

/*
 * List valid arguments for the Kernel class.
 * XXX Maybe we should nlist the variables first and only list
 * those variables we find?
 */
extern void KernelList()
{
    Define_t		       *KernDef;
    char		       *SymName;

    KernDef = DefGetList(DL_KERNEL); 
    if (!KernDef) {
	SImsg(SIM_WARN, "No kernel variables are defined.");
	return;
    }

    SImsg(SIM_INFO, "\n\nThe following are valid arguments for `-class Kernel -show Name1,Name2,...':\n\n");
    SImsg(SIM_INFO, "%-25s %s\n", "NAME", "DESCRIPTION");

    for ( ; KernDef; KernDef = KernDef->Next) {
	SymName = KernDef->KeyStr;
	if (*SymName == '_')
	    ++SymName;
	SImsg(SIM_INFO, "%-25s %s\n", SymName, KernDef->ValStr2);
    }
}
