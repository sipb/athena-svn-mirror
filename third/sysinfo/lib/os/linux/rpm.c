/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Linux functions for RPM
 *
 * Once upon a time we used the nice C API to quickly and easily get RPM
 * data.  Then it was discovered that many Linux distributions varied in
 * the version of RPM data they used which resulted in the C API not being
 * very portable (yeeks!).  So now we run the rpm command and parse it's
 * output.  Not ideal, but far more portable.
 */

#include "defs.h"

#include "cmd.h"
#include <sys/stat.h>

/*
 * Possible paths to the RPM command
 */
static char *RPMpaths[]	= { "/bin/rpm", "/usr/bin/rpm", NULL };

#define RPMCMD_FIELD_DELIM	'|'		/* Field deliminator */
#define RPMCMD_ENTRY_DELIM	'#'		/* End Of Entry */

/*
 * Environment vars for executing the RPM command with.
 */
static char *RPMenv[] = { 
    "HOME=/", 	/* rpm fails if $HOME is not set to *something* */
    NULL 
};

/*
 * RPM Tag table
 */
typedef struct {
    char		       *RPMtag;		/* RPMTAG_* value */
    int				Enabled;	/* This tag is available */
} RPMtag_t;

/*
 * Table of RPM Tags that we are interested in.
 * The TAGPOS_* values indicate the numeric index order that they appear in.
 */
static RPMtag_t RPMtags[] = {
#define TAGPOS_NAME		0
    { "NAME" },
#define TAGPOS_VERSION		1
    { "VERSION" },
#define TAGPOS_RELEASE		2
    { "RELEASE" },
#define TAGPOS_VENDOR		3
    { "VENDOR" },
#define TAGPOS_GROUP		4
    { "GROUP" },
#define TAGPOS_OS		5
    { "OS" },
#define TAGPOS_ARCH		6
    { "ARCH" },
#define TAGPOS_BUILDHOST	7
    { "BUILDHOST" },
#define TAGPOS_ROOT		8
    { "ROOT" },
#define TAGPOS_INSTALLTIME	9
    { "INSTALLTIME" },
#define TAGPOS_BUILDTIME	10
    { "BUILDTIME" },
#define TAGPOS_SIZE		11
    { "SIZE" },
#define TAGPOS_DISTRIBUTION	12
    { "DISTRIBUTION" },
#define TAGPOS_SUMMARY		13
    { "SUMMARY" },
#define TAGPOS_DESCRIPTION	14
    { "DESCRIPTION" },
    { NULL },
};

/*
 * Get the file data for the package named in SoftInfo
 */
static SoftFileList_t *RPMgetEntryFiles(SoftInfo)
     SoftInfo_t                *SoftInfo;
{
    SoftFileList_t             *SoftFileList = NULL;
    SoftFileList_t             *LastList = NULL;
    SoftFileList_t             *NewList = NULL;
    SoftFileData_t             *Data = NULL;
    static char			Buff[8192];
    static char		       *RPMcmd[6];
    static Cmd_t		Cmd;
    FILE		       *FilePtr;
    char		      **Argv = NULL;
    char		       *cp;
    int				Argc;
    mode_t			Mode;

    if (!SoftInfo || !SoftInfo->Name)
	return (SoftFileList_t *) NULL;

    /* RPMcmd[0] reserved */
    RPMcmd[1] = "-q";
    RPMcmd[2] = "-l";
    RPMcmd[3] = "--dump";
    RPMcmd[4] = SoftInfo->Name;
    RPMcmd[5] = NULL;

    (void) memset(&Cmd, CNULL, sizeof(Cmd));
    Cmd.FD = -1;
    Cmd.CmdPath = RPMpaths;
    Cmd.Argv = RPMcmd;
    Cmd.Argc = sizeof(RPMcmd)/sizeof(char *);
    Cmd.Env = RPMenv;
    Cmd.Flags = CMF_READ;

    if (CmdOpen(&Cmd) < 0) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      Cmd.Program, SYSERR);
	return (SoftFileList_t *) NULL;
    }

    FilePtr = fdopen(Cmd.FD, "r");
    if (!FilePtr) {
	SImsg(SIM_GERR, "%s: fdopen failed: %s", Cmd.Program, SYSERR);
	(void) CmdClose(&Cmd);
	return (SoftFileList_t *) NULL;
    }

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	if (cp = strchr(Buff, '\n'))
	    *cp = CNULL;

	Argc = StrToArgv(Buff, " ", &Argv, NULL, 0);
	if (Argc != 11) {	/* Argc = what --dump provides */
	    SImsg(SIM_GERR, 
	    "Wrong arg count (got %d/%d) while reading rpm input: %.30s...",
		  Argc, 11, Buff);
	    continue;
	}
	
	Data = (SoftFileData_t *) xcalloc(1, sizeof(SoftFileData_t));
	Data->Path = Argv[0];
	Data->FileSize = (Large_t) strtol(Argv[1], NULL, 0);
	Data->MD5 = Argv[3];
	Mode = (mode_t) strtol(Argv[4], NULL, 0);

	switch ( Mode & __S_IFMT ) {
	    case __S_IFREG:	Data->Type = SFT_FILE;	break;
	    case __S_IFDIR:	Data->Type = SFT_DIR;	break;
	    case __S_IFCHR:	Data->Type = SFT_CDEV;	break;
	    case __S_IFBLK:	Data->Type = SFT_BDEV;	break;
	    case __S_IFLNK:	
		Data->Type = SFT_SLINK;
		Data->LinkTo = Argv[10];
		break;
	}

	NewList = (SoftFileList_t *) xcalloc(1, sizeof(SoftFileList_t));
	NewList->FileData = Data;
	if (!SoftFileList) {
	    LastList = SoftFileList = NewList;
	} else {
	    LastList->Next = NewList;
	    LastList = NewList;
	}
    }

    (void) fclose(FilePtr);
    (void) CmdClose(&Cmd);	/* End command */

    return SoftFileList;
}

/*
 * Extract RPM info contain in Argv.
 */
static int RPMextract(Argv, Argc, SoftInfoTree)
     char		      **Argv;
     int			Argc;
     SoftInfo_t               **SoftInfoTree;
{
    SoftInfo_t                 *SoftInfo;
    SoftInfo_t                 *Parent;
    static SoftInfoFind_t	Find;
    char                       *cp;

    if (!Argv || Argc <= 0 || !SoftInfoTree)
	return -1;

    SoftInfo = SoftInfoCreate(NULL);

#define Arg(x)	( (Argv[x] && strcasecmp(Argv[x], "(none)")) ? Argv[x] : NULL )
    SoftInfo->Name = Arg(TAGPOS_NAME);
    SoftInfo->Version = Arg(TAGPOS_VERSION);
    SoftInfo->Revision = Arg(TAGPOS_RELEASE);
    SoftInfo->VendorName = Arg(TAGPOS_VENDOR);
    SoftInfo->Category = Arg(TAGPOS_GROUP);
    SoftInfo->OSname = Arg(TAGPOS_OS);
    SoftInfo->Arch = Arg(TAGPOS_ARCH);
    SoftInfo->ProdStamp = Arg(TAGPOS_BUILDHOST);
    SoftInfo->BaseDir = Arg(TAGPOS_ROOT);
    SoftInfo->InstDate = Arg(TAGPOS_INSTALLTIME);
    SoftInfo->BuildDate = Arg(TAGPOS_BUILDTIME);
    SoftInfo->DiskUsage = atoL(Argv[TAGPOS_SIZE]);
    if ((cp = Arg(TAGPOS_DISTRIBUTION)) && strlen(cp))
	AddDesc(&SoftInfo->DescList, "Distribution", cp, 0);
    SoftInfo->Desc = Arg(TAGPOS_SUMMARY);
    SoftInfo->DescVerbose = Arg(TAGPOS_DESCRIPTION);
#undef	Arg

    /*
     * Now get the file data
     */
    SoftInfo->FileList = RPMgetEntryFiles(SoftInfo);

    if (SoftInfo->Category) {
	/*
	 * Use the Category as this entry's Parent
	 */
	(void) memset(&Find, CNULL, sizeof(Find));
	Find.Name = SoftInfo->Category;
	Find.Tree = *SoftInfoTree;
	Parent = SoftInfoFind(&Find);
	if (!Parent) {
	    Parent = SoftInfoCreate(NULL);
	    Parent->Name = SoftInfo->Category;
	    Parent->EntryTypeNum = MC_SET_PROD;
	    SoftInfoAdd(Parent, SoftInfoTree, NULL);
	}
	SoftInfo->Master = Parent;
    }

    SoftInfoAdd(SoftInfo, SoftInfoTree, NULL);

    return 0;
}

/*
 * Run RPM to find out what tags it knows about.
 */
static char **RPMgetTagList()
{
    static char		       *RPMcmd[3];
    static char			Buff[128];
    static Cmd_t		Cmd;
    char		      **List = NULL;
    char		       *cp;
    size_t			ListCount = 0;
    int				ListIdx = 0;
    FILE		       *FilePtr;

    /* RPMcmd[0] reserved */
    RPMcmd[1] = "--querytags";
    RPMcmd[2] = NULL;

    (void) memset(&Cmd, CNULL, sizeof(Cmd));
    Cmd.FD = -1;
    Cmd.CmdPath = RPMpaths;
    Cmd.Argv = RPMcmd;
    Cmd.Env = RPMenv;
    Cmd.Argc = sizeof(RPMcmd)/sizeof(char *);
    Cmd.Flags = CMF_READ;

#define LIST_INITIAL	8
#define LIST_INC	32
    ListCount = LIST_INITIAL;
    List = (char **) xcalloc(ListCount, sizeof(char *));

    if (CmdOpen(&Cmd) < 0) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      Cmd.Program, SYSERR);
	return (char **) NULL;
    }

    FilePtr = fdopen(Cmd.FD, "r");
    if (!FilePtr) {
	SImsg(SIM_GERR, "%s: fdopen failed: %s", Cmd.Program, SYSERR);
	(void) CmdClose(&Cmd);
	return (char **) NULL;
    }

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	if (cp = strchr(Buff, '\n'))
	    *cp = CNULL;

	if (ListIdx >= ListCount) {
	    /* Increase List size */
	    List = (char **) xrealloc(List, (ListCount + LIST_INC) * 
				      sizeof(char *));
	    ListCount += LIST_INC;
	}

	List[ListIdx++] = strdup(Buff);
	List[ListIdx] = NULL;
    }

#undef	LIST_INITIAL
#undef	LIST_INC
    (void) fclose(FilePtr);
    (void) CmdClose(&Cmd);

    return List;
}

/*
 * Find Tag in List
 * Returns TRUE if found.
 * Returns FALSE otherwise.
 */
static int FindTag(Tag, List)
     char		       *Tag;
     char		      **List;
{
    register char	      **Ptr;

    if (!Tag || !List)
	return FALSE;

    for (Ptr = List; Ptr && *Ptr; ++Ptr)
	if (EQ(Tag, *Ptr))
	    return TRUE;

    return FALSE;
}

/*
 * See what tags are supported by RPM on this system.
 */
static void RPMcheckTags()
{
    char		      **SupportedList;
    RPMtag_t		       *Tags;
    register char	      **Ptr;

    SupportedList = RPMgetTagList();

    if (Debug) {
	SImsg(SIM_INFO, "RPM TAGS Supported:");
	for (Ptr = SupportedList; Ptr && *Ptr; ++Ptr)
	    SImsg(SIM_INFO, " %s", *Ptr);
	SImsg(SIM_INFO, "\n");
    }

    for (Tags = RPMtags; Tags && Tags && Tags->RPMtag; ++Tags) {
	if (FindTag(Tags->RPMtag, SupportedList))
	    Tags->Enabled = TRUE;
	else
	    SImsg(SIM_DBG, "RPM doesn't support tag <%s>", Tags->RPMtag);
    }
}

/*
 * Build up the list of query tags to use and return a --queryfmt
 * compatable string.
 */
static int RPMgetQuery(QueryFmt, QueryArgc)
     char		      **QueryFmt;
     int		       *QueryArgc;
{
    RPMtag_t		       *Tags;
    char		       *Ptr = NULL;
    size_t			QueryLen = 0;
    static char		       *Fmt;
    int				Argc = 0;

    if (!QueryFmt || !QueryArgc)
	return -1;

    /*
     * See what tags are supported since this can vary from Linux version
     * and Linux distribution.
     */
    RPMcheckTags();

    if (Fmt)
	(void) free(Fmt);

    /*
     * Calculate how much memory we need
     */
    for (Tags = RPMtags; Tags && Tags && Tags->RPMtag; ++Tags)
	if (Tags->Enabled) {
	    QueryLen += strlen(Tags->RPMtag) + 7;	/* Inc %{} | */
	    Argc++;
	}

    if (!QueryLen) {
	SImsg(SIM_GERR, "No RPM tags are available on this system.");
	return -1;
    }

    Fmt = (char *) xcalloc(1, QueryLen + 3);	/* Includes DELIM + \n */

    /*
     * Build QueryFmt
     */
    Ptr = Fmt;
    for (Tags = RPMtags; Tags && Tags && Tags->RPMtag; ++Tags) {
	if (Ptr != Fmt) {
	    (void) sprintf(Ptr," %c ", RPMCMD_FIELD_DELIM);
	    Ptr += 3;
	}
        (void) sprintf(Ptr, "%%{%s}", Tags->RPMtag);
	Ptr += strlen(Tags->RPMtag) + 3;
    }

    /*
     * Add the end of entry deliminator
     */
    (void) sprintf(Ptr, "%c\n", RPMCMD_ENTRY_DELIM);

    *QueryFmt = Fmt;
    *QueryArgc = Argc;

    return 0;
}

/*
 * Build Software Information tree
 */
extern int BuildSoftInfoRPM(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    static char		       *RPMcmd[6];
    char		       *QueryFmt = NULL;
    int				QueryArgc = 0;
    char		       *Entry;
    char		      **Argv = NULL;
    int				Argc;
    static Cmd_t		Cmd;
    FileIO_t		       *IO;

    if (RPMgetQuery(&QueryFmt, &QueryArgc) < 0) {
	SImsg(SIM_DBG, "Build RPM query format failed.");
	return -1;
    }

    /* RPMcmd[0] reserved */
    RPMcmd[1] = "-q";
    RPMcmd[2] = "-a";
    RPMcmd[3] = "--queryformat";
    RPMcmd[4] = QueryFmt;
    RPMcmd[5] = NULL;

    (void) memset(&Cmd, CNULL, sizeof(Cmd));
    Cmd.FD = -1;
    Cmd.CmdPath = RPMpaths;
    Cmd.Argv = RPMcmd;
    Cmd.Argc = sizeof(RPMcmd)/sizeof(char *);
    Cmd.Env = RPMenv;
    Cmd.Flags = CMF_READ;

    if (CmdOpen(&Cmd) < 0) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      Cmd.Program, SYSERR);
	return -1;
    }

    IO = FIOcreate();
    IO->FD = Cmd.FD;
    IO->Delim = RPMCMD_ENTRY_DELIM;

    while (Entry = FIOread(IO)) {
	Argc = StrToArgv(Entry, "|", &Argv, NULL, 0);
	if (Argc != QueryArgc) {
	    SImsg(SIM_GERR, 
	    "Wrong arg count (got %d/%d) while reading rpm input: %.30s...",
		  Argc, QueryArgc, Entry);
	    continue;
	}
	
	/* Extract data */
	(void) RPMextract(Argv, Argc, SoftInfoTree);
    }

    (void) FIOdestroy(IO);	/* Free memory */
    (void) CmdClose(&Cmd);	/* End command */

    return 0;
}
