/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: report.c,v 1.1.1.1 1996-10-07 20:16:49 ghudson Exp $";
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
