/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Reporting
 */
#include "defs.h"

char			       *RepSep = " | ";

extern void Report(ClassType, InfoType, Key, Data, NumData)
char			       *ClassType;
char			       *InfoType;
char			       *Key;
char			      **Data;
int				NumData;
{
    register int		i;

    if (!ClassType)
	return;

    SImsg(SIM_INFO, "%s%s%s%s%s", 
	   strlower(ClassType), RepSep, PRTS(InfoType), RepSep, PRTS(Key));

    for (i = 0; i < NumData; ++i) {
	SImsg(SIM_INFO, "%s", RepSep);
	if (Data[i])
	    SImsg(SIM_INFO, "%s", Data[i]);
    }

    SImsg(SIM_INFO, "\n");
}
