/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: report.c,v 1.1.1.2 1998-02-12 21:32:00 ghudson Exp $";
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

    printf("%s%s%s%s%s", 
	   strlower(ClassType), RepSep, PS(InfoType), RepSep, PS(Key));

    for (i = 0; i < NumData; ++i) {
	printf("%s", RepSep);
	if (Data[i])
	    printf("%s", Data[i]);
    }

    printf("\n");
}
