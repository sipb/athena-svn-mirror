/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Show general functions
 */

#include "defs.h"

/*
 * Show Off Set space
 */
extern void ShowOffSet(cnt)
    int 			cnt;
{
    if (FormatType == FT_PRETTY)
	SImsg(SIM_INFO, "%*s", cnt, "");
}

/*
 * Show a label
 */
extern void ShowLabel(Name, OffSet)
    char 		       *Name;
    int 			OffSet;
{
    ShowOffSet(OffSet);
    if (VL_CONFIG)
	SImsg(SIM_INFO, "%*s%-22s:", OffSetAmt, "", Name);
    else
	SImsg(SIM_INFO, "%*s%22s:", OffSetAmt, "", Name);
}
