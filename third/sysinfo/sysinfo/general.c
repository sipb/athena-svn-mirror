/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
    { "hostname",	"Host Name",		MCSI_HOSTNAME },
    { "hostaliases",	"Host Aliases",		MCSI_HOSTALIASES },
    { "hostaddrs",	"Host Address(es)",	MCSI_HOSTADDRS },
    { "hostid",		"Host ID",		MCSI_HOSTID },
    { "serial",		"Serial Number",	MCSI_SERIAL },
    { "man",		"Manufacturer",		MCSI_MAN },
    { "manshort",	"Manufacturer (Short)",	MCSI_MANSHORT },
    { "manlong",	"Manufacturer (Full)",	MCSI_MANLONG },
    { "model",		"System Model",		MCSI_MODEL },
    { "memory",		"Main Memory",		MCSI_PHYSMEM },
    { "virtmem",	"Virtual Memory",	MCSI_VIRTMEM },
    { "romver",		"ROM Version",		MCSI_ROMVER },
    { "numcpu",		"Number of CPUs",	MCSI_NUMCPU },
    { "cpu",		"CPU Type",		MCSI_CPUTYPE },
    { "arch",		"App Architecture",	MCSI_APPARCH },
    { "karch",		"Kernel Architecture",	MCSI_KERNARCH },
    { "osname",		"OS Name",		MCSI_OSNAME },
    { "osvers",		"OS Version",		MCSI_OSVER },
    { "osdist",		"OS Distribution",	MCSI_OSDIST },
    { "kernver",	"Kernel Version",	MCSI_KERNVER },
    { "boottime",	"Boot Time",		MCSI_BOOTTIME },
    { "currenttime",	"Current Time",		MCSI_CURRENTTIME },
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
    static MCSIquery_t		Query;
    static char		       *RptData[3];
    register char	      **cpp;
    char		       *Value;
    register ShowInfo_t	       *ShowPtr;
    int				Found = 0;
    int				Valid = 0;
    register int		Len = 0;
    register int		MaxLen = 0;

    if (Names) {
	/*
	 * Enable only the specific items specified by user
	 */
	for (cpp = Names; cpp && *cpp; ++cpp) {
	    Valid = FALSE;
	    for (ShowPtr = ShowInfo; ShowPtr->Name; ++ShowPtr)
		if (EQ(*cpp, ShowPtr->Name)) {
		    Valid = TRUE;
		    break;
		}
	    if (!Valid) {
		SImsg(SIM_CERR, 
		      "%s: Not a valid -show value.  Use `-list show` for a list of values.",
		      *cpp);
		exit(1);
	    }
	    if (ShowPtr->Name && ShowPtr->GetType) {
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
	if (ShowPtr->Enabled) {
	    (void) memset(&Query, CNULL, sizeof(Query));
	    Query.Op = MCSIOP_CREATE;
	    Query.Flags |= MCSIF_STRING;
	    Query.Cmd = ShowPtr->GetType;
	    if (mcSysInfo(&Query) == 0)
		Value = Query.Out;
	    else
		Value = NULL;
	    switch (FormatType) {
	    case FT_PRETTY:
		if (!Value)
		    Value = UnSupported;
		ClassShowValue(ShowPtr->Label, ShowPtr->Name, 
			       Value, MaxLen);
		break;
	    case FT_REPORT:
		RptData[0] = PRTS(ShowPtr->Label);
		RptData[1] = PRTS(Value);
		Report(CN_GENERAL, NULL, ShowPtr->Name, RptData, 2);
		break;
	    }
	}
}
