/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: sysinfo.c,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $";
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
int 				DoPrintAll = TRUE;
int 				DoPrintUnknown = FALSE;
int 				DoPrintVersion = FALSE;
int 				Level = L_GENERAL;
char			       *FormatStr = NULL;
FormatType_t			FormatType = FT_PRETTY;
int 				Debug = 0;
int 				UseProm = 0;
int 				OffSetAmt = 4;
char 			       *ShowStr = NULL;
char			       *ClassNames = NULL;
char			       *TypeNames = NULL;
char 			       *LevelStr = NULL;
char 			       *ListStr = NULL;
char 			       *UnSupported = NULL;
char 			       *ConfFile = NULL;
char			       *ConfDir = CONFIG_DIR;
char			      **Environ = NULL;
extern char		       *RepSep;

#if	defined(OPTION_COMPAT)
int 				Terse = FALSE;
#endif	/* OPTION_COMPAT */

/*
 * Command line options table.
 */
OptionDescRec Opts[] = {
    {"+all", 	NoArg,		OptBool, 	__ &DoPrintAll,		"1",
	 NULL,	"Print all information"},
    {"-all", 	NoArg,		OptBool, 	__ &DoPrintAll,		"0",
	 NULL,	"Don't print all information"},
    {"-cffile",	SepArg,		OptStr, 	__ &ConfFile,		NULL,
	 "file","Sysinfo config file"},
    {"-cfdir",	SepArg,		OptStr, 	__ &ConfDir,		NULL,
	 "directory","Sysinfo config directory"},
    {"-class",	SepArg,		OptStr, 	__ &ClassNames,		NULL,
	 "<see \"-list class\">",	"Class name"},
    {"-level",	SepArg,		OptStr, 	__ &LevelStr,		NULL,
	 "<see \"-list level\">",	"Level names"},
    {"-list",	SepArg,		OptStr, 	__ &ListStr,		"-",
	 "<what>",	"List information about <what>"},
    {"-format",	SepArg,		OptStr, 	__ &FormatStr,		NULL,
	 "<see \"-list format\">",	"Format Type"},
    {"-offset",	SepArg,		OptInt, 	__ &OffSetAmt,		NULL,
	 "<amount>",	"Number of spaces to offset device info"},
    {"-repsep",	SepArg,		OptStr, 	__ &RepSep,		NULL,
	 "<string>","Report field seperator"},
    {"-show",	SepArg,		OptStr, 	__ &ShowStr,		NULL,
	 "<see \"-list show\">",	"Show names"},
    {"-type",	SepArg,		OptStr, 	__ &TypeNames,		NULL,
	 "<see \"-list type\">",	"Type name"},
    {"+unknown",NoArg,		OptBool, 	__ &DoPrintUnknown,	"1",
	 NULL,	"Print unknown devices"},
    {"-unknown",NoArg,		OptBool, 	__ &DoPrintUnknown,	"0",
	 NULL,	"Don't print unknown devices"},
    {"+useprom",NoArg,		OptBool, 	__ &UseProm,		"1",
	 NULL,	"Use PROM values"},
    {"-useprom",NoArg,		OptBool, 	__ &UseProm,		"0",
	 NULL,	"Don't use PROM values"},
    {"-version",NoArg,		OptBool, 	__ &DoPrintVersion,	"1",
	 NULL,	"Print version of this program" },
    {"-debug",ArgHidden|SepArg,	OptInt, 	__ &Debug,		"1",
	 NULL,	"Enable debugging"},
#if	defined(OPTION_COMPAT)
    /*
     * Old options from version 2.x
     * that can be enabled for backwards compatibility
     */
    {"+terse", 	NoArg,OptBool,__ &Terse,"1",NULL,"Print info in terse format"},
    {"-terse", 	NoArg,OptBool,__ &Terse,"0",NULL,"Don't print info in terse format"},
#endif	/* OPTION_COMPAT */
};

/*
 * Values and names of levels
 */
ListInfo_t LevelNames[] = {
    { L_TERSE,		"terse",	"Values only (for parsing)" },
    { L_BRIEF,		"brief",	"Brief, but human readable" },
    { L_GENERAL,	"general",	"General info" },
    { L_DESC,		"descriptions",	"Show description info" },
    { L_CONFIG,		"config",	"Show configuration info" },
    { L_ALL,		"all",		"All information available" },
    { L_DEBUG,		"debug",	"Debugging info" },
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
static void			ListLevel();
static void			ListFormat();
static void			ListShow();

ListInfo_t ListInfo[] = {
    { 0, "class",	"Values for `-class' option",	ClassList },
    { 0, "level",	"Values for `-level' option",	ListLevel },
    { 0, "format",	"Values for `-format' option",	ListFormat },
    { 0, "show",	"Values for `-show' option",	ClassCallList },
    { 0, "type",	"Values for `-type' option",	TypeList },
    { 0 },
};

/*
 * List Level values
 */
static void ListInfoDesc(OptStr, List)
    char		       *OptStr;
    ListInfo_t		       *List;
{
    register ListInfo_t	       *ListPtr;

    if (OptStr)
	printf(
	"\nThe following values may be specified with the `%s' option:\n",
	OptStr);

    printf("%-20s %s\n", "VALUE", "DESCRIPTION");

    for (ListPtr = List; ListPtr->Name; ++ListPtr)
	printf("%-20s %s\n", ListPtr->Name,
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
 * List Level values
 */
static void ListLevel()
{
    ListInfoDesc("-level", LevelNames);
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
	    Error("The word \"%s\" is invalid.", Word);
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
static int ParseLevel(Str)
    char		       *Str;
{
    ListInfo_t		       *ListPtr;
    char		       *Word;
    int				Found;

    /*
     * Check each word in the LevelNames table
     */
    for (Word = strtok(Str, ","); Word; Word = strtok((char *)NULL, ",")) {
	for (ListPtr = LevelNames, Found = FALSE; 
	     ListPtr && ListPtr->Name && !Found; ListPtr++) {
	    if (strncasecmp(Word, ListPtr->Name, strlen(Word)) == 0) {
		Level |= ListPtr->IntKey;
		Found = TRUE;
	    }
	}
	if (!Found) {
	    Error("The word \"%s\" is not a valid verbosity level.", Word);
	    return(-1);
	}
    }

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
	    Error("The word \"%s\" is not a valid format type.", Word);
	    return(-1);
	}
    }

    return(0);
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
	Level |= L_TERSE;
}
#endif	/* OPTION_COMPAT */

/*
 * Print version information
 */
void PrintVersion()
{
    extern char			BuildDate[];
    extern char			BuildHost[];
    extern char			BuildUser[];

    if (!VL_TERSE)
	printf("Sysinfo version ");
    if (PATCHLEVEL)
	printf("%s.%d", VERSION_STR, PATCHLEVEL);
    else
	printf("%s", VERSION_STR, PATCHLEVEL);


    if (!VL_TERSE) {
	printf(" (%s)\n", VERSION_STATUS);
	printf("Built %s on %s", BuildDate, BuildHost);
	if (BuildUser[0])
	    printf(" by %s", BuildUser);
	printf(COPYRIGHT_MSG);
    }

    printf("\n");
}

/*
 * The beginning
 */
main(Argc, Argv, Envp)
    int 			Argc;
    char 		      **Argv;
    char 		      **Envp;
{
    Environ = Envp;

    if (ParseOptions(Opts, Num_Opts(Opts), Argc, Argv) < 0)
	exit(1);
    
    /*
     * Don't let unpriv'ed users try anything tricky.
     */
    if (geteuid() != 0 && (ConfFile || !EQ(ConfDir, CONFIG_DIR))) {
	Error("Only ``root'' may use alternative configuration files.");
	exit(1);
    }

    UnSupported = (Debug) ? "**UNSUPPORTED**" : (char *)NULL;

#if	defined(OPTION_COMPAT)
    SetOptionCompat();
#endif

    /*
     * Show version info
     */
    if (DoPrintVersion) {
	PrintVersion();
	exit(0);
    }
 
    /*
     * Parse config files
     */
    if (CFparse(ConfFile, ConfDir) != 0) {
	Error("Error(s) occured while parsing config file(s).");
	exit(1);
    }

    /*
     * Do any list commands and exit
     */
    if (ListStr) {
	List(ListStr);
	exit(0);
    }

    /*
     * If a -show was specified but not -class, then
     * default to "General".
     */
    if (ShowStr && !ClassNames)
	ClassNames = "General";

    /*
     * Set verbosity strings
     */
    if (LevelStr && ParseLevel(LevelStr))
	exit(1);

    /*
     * Set format type
     */
    if (FormatStr && ParseFormat(FormatStr))
	exit(1);

    /*
     * Set class info
     */
    ClassSetInfo(ClassNames);

    /*
     * Set type info
     */
    TypeSetInfo(TypeNames);

    if (Debug)
	printf("Verbosity level = 0x%x\n", Level);

    exit( ClassCall(ShowStr) );
}
