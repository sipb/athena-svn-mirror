#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * SGI IRIX functions for reading software information from "inst"
 */

#include "defs.h"
#include "cmd.h"

static char	       *ShowProdsPaths[] = { "/usr/sbin/showprods", NULL };
static char	       *ShowFilesPaths[] = { "/usr/sbin/showfiles", NULL };

/*
 * Extract inst info from Entry and place in SoftInfoTree
 */
static int InstExtract(Entry, SoftInfoTree)
     char		       *Entry;
     SoftInfo_t		      **SoftInfoTree;
{
    SoftInfo_t                 *SoftInfo;
    SoftInfo_t                 *Parent;
    static SoftInfoFind_t	Find;
    register char	       *cp;
    register char	       *Start;
    register char	       *End;
    char		       *Name;

    if (!Entry || !SoftInfoTree)
	return -1;

    SoftInfo = SoftInfoCreate(NULL);

    if (Start = SkipWhiteSpace(Entry+1)) {
	if ((End = strchr(Start, ' ')) || (End = strchr(Start, '\t')))
	    *End = CNULL;
	SoftInfo->Name = strdup(Start);
    } else {
	(void) SoftInfoDestroy(SoftInfo);
	return -1;
    }

    if (Start = SkipWhiteSpace(End+1)) {
	if ((End = strchr(Start, ' ')) || (End = strchr(Start, '\t')))
	    *End = CNULL;
	SoftInfo->InstDate = strdup(Start);
    }

    if (Start = SkipWhiteSpace(End+1)) {
	if (End = strchr(Start, '\n'))
	    *End = CNULL;
	SoftInfo->Desc = strdup(Start);
    }

    if (cp = strrchr(SoftInfo->Name, '.')) {
	Name = strdup(SoftInfo->Name);
	Name[cp - SoftInfo->Name] = CNULL;

	(void) memset(&Find, CNULL, sizeof(Find));
	Find.Name = Name;
	Find.Tree = *SoftInfoTree;
	Parent = SoftInfoFind(&Find);
	if (!Parent) {
	    Parent = SoftInfoCreate(NULL);
	    Parent->Name = SoftInfo->Category;
	    SoftInfoAdd(Parent, SoftInfoTree, NULL);
	}
	SoftInfo->Master = Parent;
    }

    SoftInfoAdd(SoftInfo, SoftInfoTree, NULL);
 
    return 0;
}

/*
 * Get file type for Type
 */
static int GetFileType(Type)
     char			Type;
{
    switch (Type) {
    case 'f':	return SFT_FILE;
    case 'd':	return SFT_DIR;
    case 'l':	return SFT_SLINK;
    default:
	SImsg(SIM_DBG, "showfiles: Entry with type `%c' is unknown.", Type);
    }

    return -1;
}

/*
 * Build a list of software file data by running showfiles.
 */
static int SoftInfoBuildFileData(SoftFileData)
     SoftFileData_t	      **SoftFileData;
{
    SoftFileData_t	       *New;
    char		       *cp;
    char		       *Start;
    char		       *End;
    int				Type;
    char		       *Name;
    char		       *Path;
    char		       *CheckSum;
    char		       *TypeChar;
    char		       *SizeStr;
    ulong_t			Count = 0;			
    time_t			StartTime;
    time_t			EndTime;
    static char		       *ShowFilesCmd[3];
    char		       *Entry;
    static Cmd_t		Cmd;
    FileIO_t		       *IO;

    time(&StartTime);

    /* Build the showprods(1m) command line */
    ShowFilesCmd[1] = "-A";	/* Show absolute pathnames */
    ShowFilesCmd[2] = NULL;

    (void) memset(&Cmd, CNULL, sizeof(Cmd));
    Cmd.FD = -1;
    Cmd.CmdPath = ShowFilesPaths;
    Cmd.Argv = ShowFilesCmd;
    Cmd.Argc = sizeof(ShowFilesCmd)/sizeof(char *);
    Cmd.Env = NULL;
    Cmd.Flags = CMF_READ;

    if (CmdOpen(&Cmd) < 0) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      Cmd.Program, SYSERR);
	return -1;
    }

    IO = FIOcreate();
    IO->FD = Cmd.FD;

    while (Entry = FIOread(IO)) {
	SImsg(SIM_DBG, "showfiles=<%s>", Entry);	

	Name = Path = SizeStr = NULL;
	Start = End = Entry;

	/* File Type */
	Type = GetFileType(Entry[0]);
	if (Type < 0)
	    continue;

	if (Start = SkipWhiteSpace(Entry + 1)) {
	    if ((End = strchr(Start, ' ')) || (End = strchr(Start, '\t')))
		*End = CNULL;
	    CheckSum = Start;
	}

	if (Start = SkipWhiteSpace(End + 1)) {
	    if ((End = strchr(Start, ' ')) || (End = strchr(Start, '\t')))
		*End = CNULL;
	    SizeStr = Start;
	}

	if (Start = SkipWhiteSpace(End + 1)) {
	    if ((End = strchr(Start, ' ')) || (End = strchr(Start, '\t')))
		*End = CNULL;
	    Name = Start;
	}

	/* Either a letter indicating 'm' or 'c' or the path starting with / */
	if (Start = SkipWhiteSpace(End + 1)) {
	    /* It's not the path so skip over it to the next field */
	    if (*Start != '/')
		Start = SkipWhiteSpace(Start + 1);

	    if (Start)
		Path = Start;
	}

	if (!Name || !Path)
	    continue;

	/* Create the new entry */
	New = (SoftFileData_t *) xcalloc(1, sizeof(SoftFileData_t));
	New->Type = Type;
	if (Path) 	New->Path = strdup(Path);
	if (Type == SFT_FILE) {
	    if (CheckSum) 	New->CheckSum = strdup(CheckSum);
	    if (SizeStr)	New->FileSize = (Large_t) atol(SizeStr);
	}
	New->PkgNames = xcalloc(2, sizeof(char *));
	New->PkgNames[0] = strdup(Name);

	/* 
	 * Add it to the list in reverse order since we reverse add it
	 * it later.
	 */
	if (!*SoftFileData)
	    *SoftFileData = New;
	else {
	    New->Next = *SoftFileData;
	    *SoftFileData = New;
	}
	++Count;
    }

    (void) FIOdestroy(IO);
    (void) CmdClose(&Cmd);

    time(&EndTime);
    SImsg(SIM_DBG, 
	  "BuildFileData: Finished building file data (Entries=%ld Time=%ds)",
	  Count, EndTime-StartTime);

    return 0;
}

/*
 * Build Software Information tree from inst
 */
extern int BuildSoftInfo(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    static char		       *ShowProdsCmd[3];
    char		       *Entry;
    static Cmd_t		Cmd;
    FileIO_t		       *IO;
    int				Line = 0;
    time_t			StartTime;
    time_t			EndTime;
    SoftFileData_t	       *SoftFileData = NULL;
    int				Found = 0;

    time(&StartTime);

    /* Build the showprods(1m) command line */
    ShowProdsCmd[1] = "-E";	/* Don't page output */
    ShowProdsCmd[2] = NULL;

    (void) memset(&Cmd, CNULL, sizeof(Cmd));
    Cmd.FD = -1;
    Cmd.CmdPath = ShowProdsPaths;
    Cmd.Argv = ShowProdsCmd;
    Cmd.Argc = sizeof(ShowProdsCmd)/sizeof(char *);
    Cmd.Env = NULL;
    Cmd.Flags = CMF_READ;

    if (CmdOpen(&Cmd) < 0) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      Cmd.Program, SYSERR);
	return -1;
    }

    IO = FIOcreate();
    IO->FD = Cmd.FD;

    while (Entry = FIOread(IO)) {
	SImsg(SIM_DBG, "showprods=<%s>", Entry);	
	++Line;
	if (Line == 1)
	    continue;

	if (Entry[0] == 'I')
	    /* Extract data */
	    if (InstExtract(Entry, SoftInfoTree) == 0)
		++Found;
    }

    (void) FIOdestroy(IO);
    (void) CmdClose(&Cmd);

    time(&EndTime);

    if (Found) {
	/*
	 * First build a list (SoftFileData) from the software file data
	 * file, then add what we find to the SoftInfo tree.
	 */
	if (SoftInfoBuildFileData(&SoftFileData) == 0)
	    SoftInfoAddFileData(SoftFileData, SoftInfoTree);
	else
	    SImsg(SIM_GERR, "Unable to build list of software file data.");

	SImsg(SIM_DBG, "Created %d software info entries in %ld seconds.", 
	      Found, EndTime - StartTime);
	return 0;
    } else {
	SImsg(SIM_DBG, "No software packages were found (%d seconds).",
	      EndTime - StartTime);
	return -1;
    }
}
