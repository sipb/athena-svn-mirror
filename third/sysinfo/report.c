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
