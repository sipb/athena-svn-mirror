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
 * Display System Information
 */

#include <stdio.h>
#include <sys/types.h>

#include "defs.h"

/*
 * Local declarations
 */
int				Danger = FALSE;
int 				DoPrintUnknown = FALSE;
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
char 			       *ShowStr = NULL;
char			       *ClassNames = NULL;
char			       *TypeNames = NULL;
char 			       *MsgLevelStr = NULL;
char 			       *ListStr = NULL;
char 			       *UnSupported = NULL;
char 			       *ConfFile = NULL;
char			       *ConfDir = CONFIG_DIR;
char			       *MCLprogramName = "sysinfo";
char			      **Environ = NULL;
extern char		       *RepSep;
int 				DoNothing = TRUE;

#if	defined(OPTION_COMPAT)
int 				Terse = FALSE;
#endif	/* OPTION_COMPAT */

/*
 * Command line options table.
 */
OptionDescRec Opts[] = {
    {"-cffile",	SepArg,		OptStr, (OptPtr_t) &ConfFile,		NULL,
	 "file","Sysinfo config file"},
    {"-cfdir",	SepArg,		OptStr, (OptPtr_t) &ConfDir,		NULL,
	 "directory","Sysinfo config directory"},
    {"-class",	SepArg,		OptStr, (OptPtr_t) &ClassNames,		NULL,
	 "<see \"-list class\">",	"Class name"},
    {"-danger",NoArg,		OptBool,(OptPtr_t) &Danger,		"1",
	 NULL,	"Disable runtime platform checks" },
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
    {"-type",	SepArg,		OptStr, (OptPtr_t) &TypeNames,		NULL,
	 "<see \"-list type\">",	"Type name"},
    {"+unknown",NoArg,		OptBool,(OptPtr_t) &DoPrintUnknown,	"1",
	 NULL,	"Print unknown devices"},
    {"-unknown",NoArg,		OptBool,(OptPtr_t) &DoPrintUnknown,	"0",
	 NULL,	"Don't print unknown devices"},
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
    {"-debug",ArgHidden|SepArg,	OptInt,	(OptPtr_t) &DebugFlag,		"1",
     	NULL,  "Enable debugging" },
    {"-level",ArgHidden|SepArg,	OptStr, (OptPtr_t) &MsgLevelStr,	NULL,
	 "<see \"-list msglevel\">",	"Message Levels to output"},
    {"+all", ArgHidden|NoArg,	OptBool,(OptPtr_t) &DoNothing,		"1",
	 NULL,	"Print all information"},
    {"-all", ArgHidden|NoArg,	OptBool,(OptPtr_t) &DoNothing,		"0",
	 NULL,	"Don't print all information"},
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
static void			ListShow();

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

    if (OptStr)
	SImsg(SIM_INFO, 
	"\nThe following values may be specified with the `%s' option:\n",
	OptStr);

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
 * Get argument number "arg" from "str".
 */
static char *GetArg(Str, ArgNum)
    char 		       *Str;
    int 			ArgNum;
{
    register char 	       *p, *start = NULL;
    register int 		c;

    for (c = 1, p = Str; p && *p; ++c) {
	/* set start of word */
	start = p; 

	/* skip over word */
	while (p && *p && *p != ' ' && *p != '\t' && *p != '\n')
	    ++p;

	/* is this what we want? */
	if (c == ArgNum) {
	    if (p) *p = C_NULL;
	    break;
	}

	/* skip white space */
	while (*p == ' ' || *p == '\t')
	    ++p;
    }

    return(start);
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
 *   	PLEASE DO NOT DISABLE.  DISABLING THIS CODE IS A VIOLATION OF 
 *	THE LICENSE.  AND IT'S NOT VERY NICE EITHER.
 */
static mcl_t *CheckMCL(Product)
     char		       *Product;
{
    static mcl_t		Query;
    static char			FileBuff[MAXPATHLEN];

    (void) snprintf(FileBuff, sizeof(FileBuff), "%s/license%s", 
		    ConfDir, MCL_FILE_EXT);
    (void) memset(&Query, 0, sizeof(Query));
    Query.File = FileBuff;
    Query.Product = Product;

    if (MCLcheck(&Query) < 0) {
	if (Query.Status == MCL_STAT_NOTFOUND)
	    SImsg(SIM_CERR|SIM_NOLBL, "\
You are running a free 60-day DEMO version of %s.  Please obtain a\n\
permanent license or cease using this program within 60 days.  Licenses\n\
may be obtained from http://www.MagniComp.com/sysinfo/#getlicense\n",
		  Product);
	SImsg(SIM_DBG, "Cannot obtain license: %s", 
	      (Query.Msg) ? Query.Msg : "UNKNOWN REASON");
	return((mcl_t *) NULL);
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
 * Check to see if we're running with the right Root Access.
 */
static void CheckRunTimeAccess()
{
#if	defined(RA_LEVEL) && RA_LEVEL == RA_ADVISED
    if (geteuid() != 0) {
	SImsg(SIM_WARN, 
"This program should be run as `root' (uid=0) to obtain\n\t\
all supported information.  Please either make setuid to\n\t\
root (chown root sysinfo;chmod u+s sysinfo) or invoke as a\n\t\
uid=0 user such as `root'.");
    }
#endif	/* RA_ADVISED */
#if	defined(RA_LEVEL) && RA_LEVEL == RA_NONE
    /* Do nothing */
    return;
#endif	/* RA_NONE */
}

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

    /*
     * Skip this check if the user specified the -danger flag
     */
    if (Danger)
	return;

    /* 
     * Yes, it is possible for the OS name to be different.
     * i.e. "IRIX" vs. "IRIX64" 
     */
    if (!EQ(GetOSName(), BuildOSname))
	(void) snprintf(What, sizeof(What),
		"This program was built on `%s' but is being run on `%s'.",
			BuildOSname, GetOSName());
    else if (!EQ(GetOSVer(), BuildOSver))
	(void) snprintf(What, sizeof(What),
"This program was built on `%s %s' \
but is being run on `%s %s'.",
			BuildOSname, BuildOSver, GetOSName(), GetOSVer());
#if	defined(sunos) && OSMVER == 4
    else if (!EQ(GetKernArch(), BuildCPUArch))
	(void) snprintf(What, sizeof(What),
"This program was built on a `%s' kernel arch machine and is being\
run on a `%s' machine.",
			BuildCPUArch, GetKernArch());
#endif

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
    int				MsgFlags = SIM_INFO|SIM_DBG|SIM_NONL;
    char		       *MCLinfo;

    if (!VL_TERSE)
	SImsg(MsgFlags, "SysInfo Version ");
    if (PATCHLEVEL)
	SImsg(MsgFlags, "%s.%d", VERSION_STR, PATCHLEVEL);
    else
	SImsg(MsgFlags, "%s", VERSION_STR);

    if (!VL_TERSE) {
	SImsg(MsgFlags, " (%s)\n", VERSION_STATUS);
	SImsg(MsgFlags, "Binary Build Information:\n");
#define bOFFSET 10
	SImsg(MsgFlags, "\t%*s: %s\n", bOFFSET, "Build Date", BuildDate);
	SImsg(MsgFlags, "\t%*s: %s on %s\n", 
	      bOFFSET, "Built By", BuildUser, BuildHost);
	SImsg(MsgFlags, "\t%*s: %s %s\n", 
	      bOFFSET, "OS", BuildOSname, BuildOSver);
	SImsg(MsgFlags, "\t%*s: %s\n", bOFFSET, "OS Type",  BuildOStype);
	SImsg(MsgFlags, "\t%*s: %s\n", bOFFSET, "App Arch", BuildAppArch);
	SImsg(MsgFlags, "\t%*s: %s\n", bOFFSET, "CPU Arch", BuildCPUArch);
#undef bOFFSET
	SImsg(MsgFlags, GENERAL_MSG);
	SImsg(MsgFlags, COPYRIGHT_MSG);
    }

    SImsg(MsgFlags, "\n");
}

/*
 * The beginning
 */
main(Argc, Argv, Envp)
    int 			Argc;
    char 		      **Argv;
    char 		      **Envp;
{
    char		       *cp;
    mcl_t		       *MClic;

    Environ = Envp;

    /*
     * Parse command line arguments.
     */
    if (ParseOptions(Opts, Num_Opts(Opts), Argc, Argv) < 0)
	exit(1);

    /*
     * Cleanup our program name used for error messages.
     */
    if (cp = strrchr(ProgramName, '/'))
	ProgramName = ++cp;

    /*
     * Set Message Class Flags
     */
    if (MsgClassStr && ParseMsgClass(MsgClassStr))
	exit(1);

    /*
     * For -debug backwards compatibile 
     */
    if (DebugFlag)
	MsgClassFlags |= SIM_DBG|SIM_GERR;

    /*
     * If we're setuid(root) and we're not being run by user "root",
     * don't let unpriv'ed users try anything tricky.
     */
    if (UseConfig && (getuid() != 0 && geteuid() == 0) && 
	(ConfFile || !EQ(ConfDir, CONFIG_DIR))) {
	SImsg(SIM_CERR, 
	"Only \"root\" may specify \"-cfdir\" when %s is \"setuid root\".", 
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
	if (CFparse(ConfFile, ConfDir) != 0) {
	    SImsg(SIM_CERR, "Errors occured while parsing config file(s).");
	    exit(1);
	}
    }

    /*
     * Check MCL
     */
    MClic = CheckMCL(MCLprogramName);

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
     * Set verbosity strings
     */
    if (MsgLevelStr && ParseMsgLevel(MsgLevelStr))
	exit(1);

    /*
     * Set format type
     */
    if (FormatStr && ParseFormat(FormatStr))
	exit(1);

    /*
     * Check to see if runtime access is ok.
     */
    CheckRunTimeAccess();

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

    exit( ClassCall(ShowStr) );
}
