/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.2 $";
#endif

/*
 * Display System Information
 */

#include <stdio.h>
#include <sys/types.h>

#include "defs.h"
#include "options.h"
#include "mcl.h"

/*
 * Local declarations
 */
int				Danger = TRUE;
int 				DoPrintUnknown = FALSE;
int 				DoPrintUnused = FALSE;
int 				DoPrintVersion = FALSE;
int 				DoPrintLicense = FALSE;
int 				MsgLevel = L_GENERAL;
char			       *FormatStr = NULL;
FormatType_t			FormatType = FT_PRETTY;
char			       *MsgClassStr = NULL;
int				MsgClassFlags = SIM_DEFAULT;
int 				UseProm = 0;
int				DebugFlag = 0;
int 				OffSetAmt = 4;
int				UseConfig = 1;
int				SwFiles = 0;
char 			       *ShowStr = NULL;
char			       *ClassNames = NULL;
char			       *TypeNames = NULL;
char 			       *MsgLevelStr = NULL;
char 			       *ListStr = NULL;
char 			       *UnSupported = NULL;
char 			       *ConfFile = NULL;
char			       *ConfDir;
char			       *MCLprogramName = "sysinfo";
extern char		       *RepSep;
int 				DoNothing = TRUE;

#if	defined(OPTION_COMPAT)
int 				Terse = FALSE;
#endif	/* OPTION_COMPAT */

/*
 * Command line options table.
 */
OptionDescRec Opts[] = {
    {"-class",	SepArg,		OptStr, (OptPtr_t) &ClassNames,		NULL,
	 "<see \"-list class\">",	"Class name"},
    {"-configfile",	SepArg,	OptStr, (OptPtr_t) &ConfFile,		NULL,
	 "file",	"SysInfo master config file"},
    {"-configdir",	SepArg,	OptStr, (OptPtr_t) &ConfDir,		NULL,
	 "directory"	,"SysInfo config directory"},
    {"-danger",NoArg,		OptBool,(OptPtr_t) &Danger,		"1",
	 NULL,	"Disable runtime platform checks" },
    {"-debug", NoArg,	OptBool,	(OptPtr_t) &DebugFlag,		"1",
     	NULL,  "Enable debugging" },
    {"-license",NoArg,		OptBool,(OptPtr_t) &DoPrintLicense,	"1",
	 NULL,	"Print license information" },
    {"-list",	SepArg,		OptStr, (OptPtr_t) &ListStr,		"-",
	 "<what>",	"List information about <what>"},
    {"-format",	SepArg,		OptStr, (OptPtr_t) &FormatStr,		NULL,
	 "<see \"-list format\">",	"Format Type"},
    {"-msgclass",SepArg,	OptStr, (OptPtr_t) &MsgClassStr,	NULL,
	 "<see \"-list msgclass\">", 	"Message Classes to output"},
    {"-msglevel",SepArg,	OptStr, (OptPtr_t) &MsgLevelStr,	NULL,
	 "<see \"-list msglevel\">",	"Message Levels to output"},
    {"-offset",	SepArg,		OptInt, (OptPtr_t) &OffSetAmt,		NULL,
	 "<amount>",	"Number of spaces to offset device info"},
    {"-repsep",	SepArg,		OptStr, (OptPtr_t) &RepSep,		NULL,
	 "<string>","Report field seperator"},
    {"-show",	SepArg,		OptStr, (OptPtr_t) &ShowStr,		NULL,
	 "<see \"-list show\">",	"Show names"},
    {"+swfiles",NoArg,		OptBool,(OptPtr_t) &SwFiles,		"1",
	 NULL,	"Show files in software packages" },
    {"-swfiles",NoArg,		OptBool,(OptPtr_t) &SwFiles,		"0",
	 NULL,	"Don't show files in software packages" },
    {"-type",	SepArg,		OptStr, (OptPtr_t) &TypeNames,		NULL,
	 "<see \"-list type\">",	"Type name"},
    {"+unknown",NoArg,		OptBool,(OptPtr_t) &DoPrintUnknown,	"1",
	 NULL,	"Print unknown devices"},
    {"-unknown",NoArg,		OptBool,(OptPtr_t) &DoPrintUnknown,	"0",
	 NULL,	"Don't print unknown devices"},
    {"+unused",NoArg,		OptBool,(OptPtr_t) &DoPrintUnused,	"1",
	 NULL,	"Print unused partitions or other items"},
    {"-unused",NoArg,		OptBool,(OptPtr_t) &DoPrintUnused,	"0",
	 NULL,	"Don't print unused partititions or other items"},
    {"+useconfig",NoArg,	OptBool,(OptPtr_t) &UseConfig,		"1",
	 NULL,	"Use config (-cffile,-cfdir) files"},
    {"-useconfig",NoArg,	OptBool,(OptPtr_t) &UseConfig,		"0",
	 NULL,	"Don't use config (-cffile,-cfdir) files"},
    {"+useprom",NoArg,		OptBool,(OptPtr_t) &UseProm,		"1",
	 NULL,	"Use PROM values"},
    {"-useprom",NoArg,		OptBool,(OptPtr_t) &UseProm,		"0",
	 NULL,	"Don't use PROM values"},
    {"-version",NoArg,		OptBool,(OptPtr_t) &DoPrintVersion,	"1",
	 NULL,	"Print version of this program" },
    /*
     * These options are obsolete, but are present for backwards compatibility
     * Some of the options still do things and some don't (see DoNothing).
     */
    {"-level",ArgHidden|SepArg,	OptStr, (OptPtr_t) &MsgLevelStr,	NULL,
	 "<see \"-list msglevel\">",	"Message Levels to output"},
    {"+all", ArgHidden|NoArg,	OptBool,(OptPtr_t) &DoNothing,		"1",
	 NULL,	"Print all information"},
    {"-all", ArgHidden|NoArg,	OptBool,(OptPtr_t) &DoNothing,		"0",
	 NULL,	"Don't print all information"},
    /*
     * These options are deprecated starting in SysInfo 4.0
     */
    {"-cffile",	SepArg,		OptStr, (OptPtr_t) &ConfFile,		NULL,
	 "file","SysInfo config file"},
    {"-cfdir",	SepArg,		OptStr, (OptPtr_t) &ConfDir,		NULL,
	 "directory","SysInfo config directory"},
#if	defined(OPTION_COMPAT)
    /*
     * Old options from version 2.x
     * that can be enabled for backwards compatibility
     */
    {"+terse", 	NoArg,OptBool,(OptPtr_t) &Terse,"1",NULL,"Print info in terse format"},
    {"-terse", 	NoArg,OptBool,(OptPtr_t) &Terse,"0",NULL,"Don't print info in terse format"},
#endif	/* OPTION_COMPAT */
};

/*
 * Values and names of levels
 */
ListInfo_t MsgLevelNames[] = {
    { L_TERSE,		"terse",	"Values only (for parsing)" },
    { L_BRIEF,		"brief",	"Brief, but human readable" },
    { L_GENERAL,	"general",	"General info" },
    { L_DESC,		"descriptions",	"Show description info" },
    { L_CONFIG,		"config",	"Show configuration info" },
    { L_ALL,		"all",		"All information available" },
    { 0 },
};

/*
 * MsgClass options
 */
ListInfo_t MsgClassNames[] = {
    { SIM_INFO,		"info",		"Normal information" },
    { SIM_WARN,		"warn",		"Warnings" },
    { SIM_GERR,		"gerror",	"General errors" },
    { SIM_CERR,		"cerror",	"Critical errors" },
    { SIM_UNKN,		"unknown",	"Messages about unknown values" },
    { SIM_DBG,		"debug",	"Debugging information" },
    { SIM_ALL,		"all",		"All message classes" },
    { 0 },
};

/*
 * Format options
 */
ListInfo_t FormatNames[] = {
    { FT_PRETTY,	"pretty",	"Format output for humans" },
    { FT_REPORT,	"report",	"Format output for programs" },
    { 0 },
};

/*
 * List options
 */
static void			ListMsgLevel();
static void			ListFormat();
static void			ListMsgClass();

ListInfo_t ListInfo[] = {
    { 0, "class",	"Values for `-class' option",	ClassList },
    { 0, "format",	"Values for `-format' option",	ListFormat },
    { 0, "msgclass",	"Values for `-msgclass' option",ListMsgClass },
    { 0, "msglevel",	"Values for `-msglevel' option",ListMsgLevel },
    { 0, "show",	"Values for `-show' option",	ClassCallList },
    { 0, "type",	"Values for `-type' option",	TypeList },
    { 0 },
};

/*
 * List possible values for List
 */
static void ListInfoDesc(OptStr, List)
    char		       *OptStr;
    ListInfo_t		       *List;
{
    register ListInfo_t	       *ListPtr;

    if (OptStr && !VL_TERSE)
	SImsg(SIM_INFO, 
	"\nThe following values may be specified with the `%s' option:\n",
	OptStr);

    if (!VL_TERSE)
        SImsg(SIM_INFO, "%-20s %s\n", "VALUE", "DESCRIPTION");

    for (ListPtr = List; ListPtr->Name; ++ListPtr)
	SImsg(SIM_INFO, "%-20s %s\n", ListPtr->Name,
	       (ListPtr->Desc) ? ListPtr->Desc : "");
}

/*
 * List Format values
 */
static void ListFormat()
{
    ListInfoDesc("-format", FormatNames);
}

/*
 * List MsgClass values
 */
static void ListMsgClass()
{
    ListInfoDesc("-msgclass", MsgClassNames);
}

/*
 * List MsgLevel values
 */
static void ListMsgLevel()
{
    ListInfoDesc("-msglevel", MsgLevelNames);
}

/*
 * List list values
 */
static void ListList()
{
    ListInfoDesc("-list", ListInfo);
}

/*
 * List information about each word found in Str.
 */
static void List(Str)
    char		       *Str;
{
    register ListInfo_t	       *ListPtr;
    char		       *Word;
    int				Found;

    if (EQ(Str, "-")) {
	ListList();
	return;
    }

    for (Word = strtok(Str, ","); Word; Word = strtok((char *)NULL, ",")) {
	for (ListPtr = ListInfo, Found = FALSE; 
	     ListPtr->Name && !Found; ++ListPtr)
	    if (EQ(Word, ListPtr->Name)) {
		Found = TRUE;
		(*ListPtr->List)();
	    }

	if (!Found) {
	    SImsg(SIM_CERR, "The word \"%s\" is invalid.", Word);
	    ListList();
	    return;
	}
    }
}

/*
 * Parse and set the level keyword list
 */
static int ParseMsgLevel(Str)
    char		       *Str;
{
    ListInfo_t		       *ListPtr;
    char		       *Word;
    int				Found;

    /*
     * Check each word in the MsgLevelNames table
     */
    for (Word = strtok(Str, ","); Word; Word = strtok((char *)NULL, ",")) {
	for (ListPtr = MsgLevelNames, Found = FALSE; 
	     ListPtr && ListPtr->Name && !Found; ListPtr++) {
	    if (strncasecmp(Word, ListPtr->Name, strlen(Word)) == 0) {
		MsgLevel |= ListPtr->IntKey;
		Found = TRUE;
	    }
	}
	if (!Found) {
	    SImsg(SIM_CERR, "The word \"%s\" is not a valid message level.",
		  Word);
	    return(-1);
	}
    }

    return(0);
}

/*
 * Parse and set the Message Class Flags
 */
static int ParseMsgClass(Str)
    char		       *Str;
{
    ListInfo_t		       *ListPtr;
    char		       *Word;
    int				Found;
    int				NewFlags = 0;

    /*
     * Check each word in the MsgClassNames table
     */
    for (Word = strtok(Str, ","); Word; Word = strtok((char *)NULL, ",")) {
	for (ListPtr = MsgClassNames, Found = FALSE; 
	     ListPtr && ListPtr->Name && !Found; ListPtr++) {
	    if (EQN(Word, ListPtr->Name, strlen(Word))) {
		NewFlags |= ListPtr->IntKey;
		Found = TRUE;
	    }
	}
	if (!Found) {
	    SImsg(SIM_CERR, "The word \"%s\" is not a valid MsgClass flag.",
		  Word);
	    return(-1);
	}
    }

    if (NewFlags)
	MsgClassFlags = NewFlags;

    return(0);
}

/*
 * Parse and set the format type
 */
static int ParseFormat(Str)
    char		       *Str;
{
    register ListInfo_t	       *ListPtr;
    char		       *Word;
    int				Found;

    /*
     * Check each word in the FormatNames table
     */
    for (Word = strtok(Str, ","); Word; Word = strtok((char *)NULL, ",")) {
	for (ListPtr = FormatNames, Found = FALSE; ListPtr->Name && !Found; 
	     ListPtr++) {
	    if (strncasecmp(Word, ListPtr->Name, strlen(Word)) == 0) {
		FormatType = ListPtr->IntKey;
		Found = TRUE;
	    }
	}
	if (!Found) {
	    SImsg(SIM_CERR, "The word \"%s\" is not a valid format type.", 
		  Word);
	    return(-1);
	}
    }

    return(0);
}

/*
 * Check for license.
 *
 * You may disable this code if you meet the terms of the license
 * which qualifies you for a free license.  If this is not the case,
 * YOU MAY NOT CHANGE, DISABLE, OR ALTER THE LICENSE MECHANISM IN
 * ANY MANNER.  DOING SO IS A VIOLATION OF THE LICENSE.
 * And it's not very nice either.
 */
static mcl_t *CheckMCL(Product, ProgramPath)
     char		       *Product;
     char		       *ProgramPath;
{
    static mcl_t		Query;
    static char			FileBuff[MAXPATHLEN];
    int				DemoDays = 60;
    int				Status;
    char		       *What;

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/license%s", 
		    ConfDir, MCL_FILE_EXT);
    (void) memset(&Query, 0, sizeof(Query));
    Query.File = FileBuff;
    Query.Product = Product;
    Query.ProdVers = VERSION_STR;
    Query.TimeStampFile = ProgramPath;
    Query.DemoPeriod = DemoDays * 24 * 60 * 60; /* 60 Days */
    if (Debug)
	Query.Flags = MCLF_DEBUG;

    Status = MCLcheck(&Query);
    if (Status < 0 || Query.Type == MCL_DEMO)
	SImsg(SIM_CERR|SIM_NOLBL, "\
You are running a free %d-day DEMO version of %s.  Please obtain a\n\
permanent license or cease using this program within %d days.  Licenses\n\
may be obtained from http://www.MagniComp.com/sysinfo/#getlicense\n",
	      DemoDays, Product, DemoDays);

    if (Status < 0) {
	if (Query.Status == MCL_STAT_EXPIRED) {
	    What = (Query.Type == MCL_DEMO) ? " DEMO" : " FULL";
	    if (Query.Expires)
		SImsg(SIM_CERR, "Your%s license expired %s.", 
		      What, TimeStr(Query.Expires, "%c"));
	    else
		SImsg(SIM_CERR, "Your%s license has expired.", What);
	}
	SImsg(SIM_DBG, "Cannot obtain license: %s", 
	      (Query.Msg) ? Query.Msg : "UNKNOWN REASON");
	return((mcl_t *) NULL);
    } else if (Query.Type == MCL_DEMO) {
	if (Query.Expires) {
	    if (Query.DaysLeft) 
		SImsg(SIM_CERR|SIM_NOLBL|SIM_NONL, 
		      "This DEMO license expires in %d days",
		      Query.DaysLeft);
	    else if (Query.DaysLeft == 0) {
		SImsg(SIM_CERR|SIM_NOLBL|SIM_NONL, 
		      "This DEMO license expires TODAY");
	    }
	    SImsg(SIM_CERR|SIM_NOLBL|SIM_NONL, " on %s", 
		  TimeStr(Query.Expires, "%c"));
	    SImsg(SIM_CERR|SIM_NOLBL, ".\n");
	}
    }

    return(&Query);
}

#if	defined(OPTION_COMPAT)
/*
 * Set option compatibility
 */
static void SetOptionCompat()
{
    /*
     * For backwards compatibility
     */
    if (Terse)
	MsgLevel |= L_TERSE;
}
#endif	/* OPTION_COMPAT */

/*
 * Check to see if we're running on the same platform as we were
 * compiled on.
 */
static void CheckRunTimePlatform()
{
    extern char			BuildCPUType[];
    extern char			BuildCPUArch[];
    extern char			BuildOSname[];
    extern char			BuildOSver[];
    static char			What[512];
    MCSIquery_t			Query;
    char		       *OSname = NULL;
    char		       *OSver = NULL;
    char		       *OSmajver = NULL;
    char		       *KernArch = NULL;	/*VARUSED*/
    char		       *cp;

    /*
     * This environment variable is usually set by sysinfowrapper
     */
    if (getenv("SYSINFO_NO_RUNTIME_PLATFORM"))
	return;

    /*
     * Skip this check if the user specified the -danger flag
     */
    if (Danger)
	return;

    What[0] = CNULL;
    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_OSNAME;
    if (mcSysInfo(&Query) == 0)
	OSname = (char *) Query.Out;
    else
	(void) snprintf(What, sizeof(What),
			"Could not determine what the OSname is.");
	
    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_OSVER;
    if (mcSysInfo(&Query) == 0)
	OSver = (char *) Query.Out;
    else
	(void) snprintf(What, sizeof(What),
			"Could not determine what the OS version is.");
    if (OSver) {
	OSmajver = strdup(OSver);
	if (cp = strchr(OSmajver, '.'))
	    *cp = CNULL;
    }

    if (EQ(OSname, "SunOS") && EQ(OSmajver, "4")) {
	(void) memset(&Query, CNULL, sizeof(Query));
	Query.Op = MCSIOP_CREATE;
	Query.Cmd = MCSI_KERNARCH;
	if (mcSysInfo(&Query) == 0)
	    KernArch = (char *) Query.Out;
	else
	    (void) snprintf(What, sizeof(What),
			    "Could not determine what the Kernel Version is.");
    }
    /*
     * Fix the OS version contain a single dot.
     */
    if (EQ(OSname, "Linux") || EQ(OSname, "AIX")) {
	if (cp = strchr(OSver, '.'))
	    if (cp = strchr(cp+1, '.'))
		*cp = CNULL;
    }

    if (What[0] == CNULL) {
	/* 
	 * Yes, it is possible for the OS name to be different.
	 * i.e. "IRIX" vs. "IRIX64" 
	 */
	if (!EQ(OSname, BuildOSname))
	    (void) snprintf(What, sizeof(What),
		"This program was built on `%s' but is being run on `%s'.",
			    BuildOSname, OSname);
	else if (!EQ(OSver, BuildOSver))
	    (void) snprintf(What, sizeof(What),
			    "This program was built on `%s %s' \
but is being run on `%s %s'.",
			    BuildOSname, BuildOSver, OSname, OSver);
#if	defined(sunos) && OSMVER == 4
	else if (!EQ(KernArch, BuildCPUArch))
	    (void) snprintf(What, sizeof(What),
"This program was built on a `%s' kernel arch machine and is being\
run on a `%s' machine.",
			    BuildCPUArch, KernArch);
#endif
    }

    if (What[0]) {
	SImsg(SIM_CERR, "%s", What);
	SImsg(SIM_CERR, 
	"%s may not be able to provide consistant information.", 
	      ProgramName);
	SImsg(SIM_CERR,
	"If you wish to override and run anyway, use the `-danger'");
	SImsg(SIM_CERR, "option.  This is generally not advised.");
	exit(1);
    }
}

/*
 * Print version information
 */
void PrintVersion()
{
    extern char			BuildDate[];
    extern char			BuildHost[];
    extern char			BuildUser[];
    extern char			BuildOSname[];
    extern char			BuildOSver[];
    extern char			BuildOStype[];
    extern char			BuildAppArch[];
    extern char			BuildCPUArch[];
    extern char			BuildCC[];
    extern char			BuildCCver[];
    extern char			BuildCFLAGS[];
    extern char			BuildLIBS[];
    int				MsgFlags = SIM_INFO|SIM_DBG|SIM_NONL;

    if (!VL_TERSE)
	SImsg(MsgFlags, "SysInfo Version ");
    if (PATCHLEVEL)
	SImsg(MsgFlags, "%s.%d", VERSION_STR, PATCHLEVEL);
    else
	SImsg(MsgFlags, "%s", VERSION_STR);

    if (VL_TERSE)
	SImsg(MsgFlags, " %s", VERSION_STATUS);
    else {
	SImsg(MsgFlags, " (%s)\n", VERSION_STATUS);
	SImsg(MsgFlags, "Binary Build Information:\n");
#define bOFFSET 18
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "Build Date", BuildDate);
	SImsg(MsgFlags, "  %*s: %s on %s\n", 
	      bOFFSET, "Built By", BuildUser, BuildHost);
	SImsg(MsgFlags, "  %*s: %s %s\n", 
	      bOFFSET, "OS", BuildOSname, BuildOSver);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "OS Type",  BuildOStype);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "App Arch", BuildAppArch);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "CPU Arch", BuildCPUArch);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "Config Dir", CFDIR);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "C Compiler", BuildCC);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "C Compiler Version", 
	      BuildCCver);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "CFLAGS", BuildCFLAGS);
	SImsg(MsgFlags, "  %*s: %s\n", bOFFSET, "Libraries", BuildLIBS);
#undef bOFFSET
	SImsg(MsgFlags, GENERAL_MSG);
	SImsg(MsgFlags, COPYRIGHT_MSG);
    }

    SImsg(MsgFlags, "\n");
}

/*
 * The beginning
 */
main(Argc, Argv)
    int 			Argc;
    char 		      **Argv;
{
    char		       *cp;
    mcl_t		       *MClic;
    MCSIquery_t			MCSIquery;

    /*
     * Parse command line arguments.
     */
    if (ParseOptions(Opts, Num_Opts(Opts), Argc, Argv) < 0)
	exit(1);

    /*
     * Do this soon after start.
     * Set verbosity strings.
     */
    if (MsgLevelStr && ParseMsgLevel(MsgLevelStr))
	exit(1);

    /*
     * Set Message Class Flags
     */
    if (MsgClassStr && ParseMsgClass(MsgClassStr))
	exit(1);

    if (DebugFlag) {
	/*
	 * Turn on all message types
	 */
	MsgClassFlags = SIM_DBG|SIM_ALL;
	MsgLevel = L_ALL;
    }

    /*
     * If we're setuid(root) and we're not being run by user "root",
     * don't let unpriv'ed users try anything tricky.
     */
    if (UseConfig && (getuid() != 0 && geteuid() == 0) && 
	(ConfFile || (ConfDir && !EQ(ConfDir, CFDIR)))) {
	SImsg(SIM_CERR, 
	"Only \"root\" may specify \"-cfdir\" when %s is setuid root.", 
	      ProgramName);
	exit(1);
    }

    UnSupported = (Debug) ? "**UNSUPPORTED**" : (char *)NULL;

#if	defined(OPTION_COMPAT)
    SetOptionCompat();
#endif

    /*
     * Show version info
     */
    if (DoPrintVersion || Debug) {
	PrintVersion();
	if (!Debug)
	    exit(0);
    }
 
    /*
     * Parse config files
     */
    if (UseConfig) {
	if (!ConfDir)
	    ConfDir = CFchooseConfDir(Argv[0]);

	if (CFparse(ConfFile, ConfDir) != 0) {
	    SImsg(SIM_CERR, "Errors occured while parsing config file(s).");
	    exit(1);
	}
    }

    /*
     * Check MCL
     */
    MClic = CheckMCL(MCLprogramName, Argv[0]);
    if (!MClic) {
	SImsg(SIM_CERR, "No license was found.");
	exit(1);
    }

    if (DoPrintLicense) {
	if (cp = MCLgetInfo(MClic))
	    SImsg(SIM_INFO|SIM_NONL|SIM_DBG, cp);
	exit(0);
    }

    /*
     * Do any list commands and exit
     */
    if (ListStr) {
	List(ListStr);
	exit(0);
    }

    /*
     * Set format type
     */
    if (FormatStr && ParseFormat(FormatStr))
	exit(1);

    /*
     * Check to see if runtime access is ok.
     */
    CheckRunTimeAccess(MsgLevel);

    /*
     * Check to see if we're running on the right platform
     */
    CheckRunTimePlatform();

    /*
     * If a -show was specified but not -class, then
     * default to "General".
     */
    if (ShowStr && !ClassNames)
	ClassNames = "General";

    /*
     * Set class info
     */
    ClassSetInfo(ClassNames);

    /*
     * Set type info
     */
    TypeSetInfo(TypeNames);

    SImsg(SIM_DBG, "Verbosity level = 0x%x\n", MsgLevel);

    /*
     * Set our program name for mcSysInfo() calls.
     */
    memset(&MCSIquery, 0, sizeof(MCSIquery));
    MCSIquery.Op = MCSIOP_PROGRAM;
    MCSIquery.In = ProgramName;
    MCSIquery.InSize = strlen(ProgramName);
    mcSysInfo(&MCSIquery);

    exit( ClassCall(ShowStr) );
}
