/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: sysconf.c,v 1.1.1.1 1996-10-07 20:16:52 ghudson Exp $";
#endif

/*
 * System Configuration
 *
 * Use the sysconf() routine to show system configuration info.
 * This routine is available on POSIX complient systems.
 */
#include "defs.h"

/*
 * List valid arguments for the SysConf class.
 */
extern void SysConfList()
{
#if	defined(HAVE_SYSCONF)
    register Define_t	       *DefPtr;

    DefPtr = DefGetList(DL_SYSCONF); 
    if (!DefPtr) {
	if (Debug) Error("No sysconf variables are defined.");
	return;
    }

    printf("\n\nThe following are valid arguments for `-class SysConf -show Name1,Name2,...':\n\n");
    printf("%-25s %s\n", "NAME", "DESCRIPTION");

    for ( ; DefPtr; DefPtr = DefPtr->Next)
	if (sysconf(DefPtr->KeyNum) > 0)
	    printf("%-25s %s\n", DefPtr->KeyStr, DefPtr->ValStr2);
#endif	/* HAVE_SYSCONF */
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
#if	defined(HAVE_SYSCONF)
    static char			Buff[BUFSIZ];
    static char		       *RptData[3];
    int				MaxDesc = 0;
    register int		Len;
    long			Value;
    char		       *ValuePtr;
    Define_t		       *SysConfDef;
    register Define_t	       *DefPtr;

    SysConfDef = DefGetList(DL_SYSCONF);
    if (!SysConfDef)
	return;

    for (DefPtr = SysConfDef; DefPtr; DefPtr = DefPtr->Next) {
	if (Names && !HasName(DefPtr->KeyStr, Names))
	    continue;

	/*
	 * Call sysconf().  If it succeeds, then store the return value
	 * in KeyNum.  If it fails, set KeyNum to -1 and continue on.
	 */
	Value = sysconf(DefPtr->KeyNum);
	if (Value < 0) {
	    if (Debug) Error("sysconf(%s) failed: %s", DefPtr->KeyStr, SYSERR);
	    DefPtr->KeyNum = -1;
	    continue;
	}
	DefPtr->KeyNum = Value;

	Len = strlen(DefPtr->ValStr2);
	if (VL_ALL)
	    Len += strlen(DefPtr->KeyStr) + 2;
	if (Len > MaxDesc)
	    MaxDesc = Len;
    }

    ClassShowLabel(MyInfo);

    for (DefPtr = SysConfDef; DefPtr; DefPtr = DefPtr->Next) {
	if (Names && !HasName(DefPtr->KeyStr, Names))
	    continue;

	/* Skip entries that are not available */
	if (DefPtr->KeyNum < 0)
	    continue;
	Value = DefPtr->KeyNum;

	if (strlen(DefPtr->ValStr2) + strlen(DefPtr->KeyStr) + 7 >= 
	    sizeof(Buff))
	    continue;

	if (EQ(DefPtr->ValStr1, "bool")) {
	    if (Value)
		ValuePtr = "TRUE";
	    else
		ValuePtr = "FALSE";
	} else
	    ValuePtr = itoa(Value);

	if (FormatType == FT_PRETTY) {
	    ClassShowValue(DefPtr->ValStr2, DefPtr->KeyStr, ValuePtr, MaxDesc);
	} else if (FormatType == FT_REPORT) {
	    RptData[0] = DefPtr->ValStr2;
	    RptData[1] = ValuePtr;
	    Report(CN_SYSCONF, NULL, DefPtr->KeyStr, RptData, 2);
	}
    }

#endif	/* HAVE_SYSCONF */
}
