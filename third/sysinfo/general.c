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
 * General Information routines
 */

#include "defs.h"

/*
 * Option compatibility support
 */
#if	defined(OLD_OPTION_COMPAT)
#define OptCom(v) v
#else
#define OptCom(v) 0
#endif

static ShowInfo_t ShowInfo[] = {
    { "hostname",	"Host Name",		GetHostName },
    { "hostaliases",	"Host Aliases",		GetHostAliases },
    { "hostaddrs",	"Host Address(es)",	GetHostAddrs },
    { "hostid",		"Host ID",		GetHostID },
    { "serial",		"Serial Number",	GetSerial },
    { "man",		"Manufacturer",		GetMan },
    { "model",		"System Model",		GetModel },
    { "memory",		"Main Memory",		GetMemory },
    { "virtmem",	"Virtual Memory",	GetVirtMem },
    { "romver",		"ROM Version",		GetRomVer },
    { "numcpu",		"Number of CPUs",	GetNumCpu },
    { "cpu",		"CPU Type",		GetCpuType },
    { "arch",		"App Architecture",	GetAppArch },
    { "karch",		"Kernel Architecture",	GetKernArch },
    { "osname",		"OS Name",		GetOSName },
    { "osvers",		"OS Version",		GetOSVer },
    { "osdist",		"OS Distribution",	GetOSDist },
    { "kernver",	"Kernel Version",	GetKernVer },
    { "boottime",	"Boot Time",		GetBootTime },
    { 0 },
};

/*
 * List valid arguments for the General class.
 */
extern void GeneralList()
{
    register ShowInfo_t	       *ShowPtr;

    SImsg(SIM_INFO, "\n\nThe following are valid arguments for `-class General -show Name1,Name2,...':\n\n");
    SImsg(SIM_INFO, "%-25s %s\n", "NAME", "DESCRIPTION");

    for (ShowPtr = &ShowInfo[0]; ShowPtr->Name; ++ShowPtr)
	SImsg(SIM_INFO, "%-25s %s\n", ShowPtr->Name, ShowPtr->Label);
}

/*
 * Show General class information.
 */
extern void GeneralShow(MyClass, Names)
    ClassInfo_t		       *MyClass;
    char		      **Names;
{
    static char		       *RptData[3];
    register char	      **cpp;
    char		       *Value;
    register ShowInfo_t	       *ShowPtr;
    int				Found = 0;
    register int		Len = 0;
    register int		MaxLen = 0;

    if (Names) {
	/*
	 * Enable only the specific items specified by user
	 */
	for (cpp = Names; cpp && *cpp; ++cpp) {
	    for (ShowPtr = ShowInfo; ShowPtr->Name; ++ShowPtr)
		if (EQ(*cpp, ShowPtr->Name))
		    break;
	    if (ShowPtr->Name && ShowPtr->Get) {
		ShowPtr->Enabled = TRUE;
		Found = TRUE;
	    }
	}
    } else {
	Found = TRUE;
	/*
	 * No items specified so enable everything
	 */
	for (ShowPtr = ShowInfo; ShowPtr->Name; ++ShowPtr)
	    ShowPtr->Enabled = TRUE;
    }

    if (Found)
	ClassShowBanner(MyClass);

    for (ShowPtr = ShowInfo; ShowPtr->Name; ++ShowPtr) {
	Len = strlen(ShowPtr->Label);
	if (VL_ALL)
	    Len += strlen(ShowPtr->Name) + 2;
	if (Len > MaxLen)
	    MaxLen = Len;
    }

    for (ShowPtr = ShowInfo; ShowPtr->Name; ++ShowPtr)
	if (ShowPtr->Enabled) 
	    switch (FormatType) {
	    case FT_PRETTY:
		Value = (*ShowPtr->Get)();
		if (!Value)
		    Value = UnSupported;
		ClassShowValue(ShowPtr->Label, ShowPtr->Name, 
			       Value, MaxLen);
		break;
	    case FT_REPORT:
		RptData[0] = PRTS(ShowPtr->Label);
		RptData[1] = PRTS((*ShowPtr->Get)());
		Report(CN_GENERAL, NULL, ShowPtr->Name, RptData, 2);
		break;
	    }
}
