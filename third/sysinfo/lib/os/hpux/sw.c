/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * HP-UX interface to the "sw" (software depot) system.
 * These functions gather/build SoftInfo
 */

#include "defs.h"
#if	defined(HAVE_STRING_H)
#include <string.h>
#endif
#if	defined(HAVE_STRINGS_H)
#include <strings.h>
#endif
#if	defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

static char	       *SwListPaths[] = { "/usr/sbin/swlist", NULL };

/* List of attribute keys we should ignore */
static char	       *IgnoreKeys[] = {
    "product", "subproduct", NULL
};
/* Lines to ignore in "description" strings */
static char	       *IgnoreDesc[] = {
    "Vendor Name", "Product Name", "Subproduct Name", "Fileset Name", NULL
};

/*
 * Chop off the end char of String if it's newline
 */
static void Chop(String)
     char		       *String;
{
    int				Len;

    Len = strlen(String);
    if (Len > 0 && String[Len-1] == '\n')
	String[Len-1] = CNULL;
}

/*
 * Should we ignore String by checking for a match in List?
 */
static int Ignore(String, List, MatchLen)
     char		       *String;
     char		      **List;
     int			MatchLen;
{
    register char	      **cpp;

    for (cpp = List; cpp && *cpp; ++cpp)
	if (MatchLen) {
	    if (EQN(*cpp, String, strlen(*cpp)))
		return TRUE;
	} else
	    if (EQ(*cpp, String))
		return TRUE;

    return FALSE;
}

/*
 * Get and return a string which starts with and ends with a double quote (").
 * The string is likely to contain newlines.
 */
static char *GetQuoted(Key, Start, IO, SoftInfo)
     char		       *Key;
     char		       *Start;
     FILE		       *IO;
     SoftInfo_t		       *SoftInfo;
{
    static char			LineBuff[8192];
    char		       *String = NULL;
    char		       *Line;
    size_t			StrLen = 0;
    register char	       *cp;
    int				IsStart = TRUE;
    register int		i;

    if (!Key || !Start || !IO || !SoftInfo)
	return (char *) NULL;

    for (Line = Start + 1; Line; 
	 Line = fgets(LineBuff, sizeof(LineBuff), IO)) {
	Chop(Line);
	if (Line != Start + 1)
	    SImsg(SIM_DBG, "SWList=<%s>", Line);	

	/* A quote on a line by itself is the end of this quoted string */
	if (Line[0] == '"' && strlen(Line) == 1)
	    break;

	/* Special handling for the starting header */
	if (IsStart) {
	    /* Skip empty lines at start */
	    if (Line[0] == CNULL)
		continue;
	    if (EQ(Key, "description")) {
		if (EQN("Text:", Line, 5)) {
		    if (cp = SkipWhiteSpace(Line + 5))
			Line = cp;
		    else
			Line += 5;
		    if (strlen(Line) == 0)
			continue;
		}
		/* See if this is the vendor's name */
		if (EQN("Vendor Name", Line, 11)) {
		    if (cp = SkipWhiteSpace(Line + 12))
			if (*cp == ':')
			    cp = SkipWhiteSpace(cp+1);
		    /* Is it better than what we have already, if anything? */
		    if (cp && (!SoftInfo->VendorName ||
			       strlen(cp) > strlen(SoftInfo->VendorName))) {
			SoftInfo->VendorName = strdup(cp);
		    }
		    continue;
		}
		/* Skip lines in our ignore list */
		if (Ignore(Line, IgnoreDesc, TRUE))
		    continue;
	    }
	}

	IsStart = FALSE;

	if (!String) {
	    /* Special handling for the first line using Start */
	    if (!(cp = SkipWhiteSpace(Line)))
		cp = Line;
	    StrLen = strlen(cp) + 1;
	    String = (char *) xmalloc(StrLen + 1);
	    (void) snprintf(String, StrLen, "%s", cp);
	} else {
	    cp = &String[StrLen-1];
	    StrLen += strlen(Line) + 1;
	    String = (char *) xrealloc(String, StrLen + 2);
	    (void) snprintf(cp, StrLen, "\n%s", Line);
	}
    }

    /* Zap all trailing newlines */
    if (String && StrLen > 2) {
	for (i = StrLen-1; i > 0 && !isprint(String[i]); --i)
	    if (String[i] == '\n')
		String[i] = CNULL;
    }

    return String;
}

/*
 * Extract software spec data from String and put into SoftInfo.
 */
static int ExtractSoftwareSpec(SoftInfo, String)
     SoftInfo_t		       *SoftInfo;
     char		       *String;
{
    char		      **Argv;
    int				Argc;
    register int		i;

    if (!SoftInfo || !String)
	return -1;

    if ((Argc = StrToArgv(String, ",", &Argv, NULL, 0)) <= 0)
	return -1;

    for (i = 0; i < Argc; ++i) {
	if (!SoftInfo->BaseDir && EQN("l=", Argv[i], 2))
	    SoftInfo->BaseDir = Argv[i] + 2;
	else if (!SoftInfo->Revision && EQN("r=", Argv[i], 2))
	    SoftInfo->Revision = Argv[i] + 2;
	else if (!SoftInfo->Arch && EQN("a=", Argv[i], 2))
	    SoftInfo->Arch = Argv[i] + 2;
	else if (!SoftInfo->VendorName && EQN("v=", Argv[i], 2))
	    SoftInfo->VendorName = Argv[i] + 2;
    }

    return 0;
}

/*
 * Extract a new softinfo entry starting with line Entry from IO
 */
static int ExtractProductEntry(Entry, IO, SoftInfoTree)
     char		       *Entry;
     FILE		       *IO;
     SoftInfo_t		      **SoftInfoTree;
{
    static char			LineBuff[8192];
    SoftInfo_t		       *SoftInfo = NULL;
    static SoftInfoFind_t	Find;
    char		       *Name = NULL;
    char		       *ParName;
    char		       *Line;
    char		       *Key;
    char		       *Value;
    char		       *cp;
    int				Len;

    if (!Entry || !*Entry || !IO || !SoftInfoTree)
	return -1;

    if (Entry[0] == '#')
	cp = SkipWhiteSpace(Entry + 1);
    else
	cp = SkipWhiteSpace(Entry);
    if (cp)
	Name = strdup(cp);

    if (!Name || !strlen(Name))
	return -1;

    /* XXX Special debug */
    if (EQ(Name, "DCE-Core.Runtime.DCE-CORE-DTS"))
      SImsg(SIM_DBG, "Found DTS");

    while (Line = fgets(LineBuff, sizeof(LineBuff), IO)) {
	Chop(Line);
	SImsg(SIM_DBG, "SWlist=<%s>", Line);	
	if (!Line || !*Line) 
	    /* Done with this entry */
	    break;

	Value = NULL;
	Key = Line;
	Len = 0;
	/* This entry could be a "key value" line or a simple "boolean_key" */
	if ((cp = strchr(Key, ' ')) || (cp = strchr(Key, '\t'))) {
	    *cp = CNULL;
	    if (Value = SkipWhiteSpace(cp+1))
		Len = strlen(Value);

	    /* Remove double quotes around whole Value */
	    if (Value && Value[0] == '"' && Len > 1 && Value[Len-1] == '"') {
		Value[Len-1] = CNULL;
		++Value;
	    } else if (Value && Value[0] == '"') {
		/* 
		 * Do full quoting if Value starts with, but does not end with
		 * a double quote.
		 */
		Value = GetQuoted(Key, Value, IO, SoftInfo);
	    }
	}

	if (!Value || !Value[0])
	    continue;

	if (!SoftInfo) {
	    SoftInfo = SoftInfoCreate(NULL);
	    SoftInfo->Name = Name;
	}

	/* Skip the babble */
	if (Value && EQ(Value, Name))
	    continue;

	if (EQ(Key, "software_spec") && Value) {
	    ExtractSoftwareSpec(SoftInfo, Value);
	} else if (Value && EQ(Key, "revision")) {
	    if (!SoftInfo->Revision) SoftInfo->Revision = strdup(Value);
	} else if (Value && EQ(Key, "directory") || EQ(Key, "location")) {
	    if (!SoftInfo->BaseDir) SoftInfo->BaseDir = strdup(Value);
	} else if (Value && EQ(Key, "architecture")) {
	    if (!SoftInfo->Arch) SoftInfo->Arch = strdup(Value);
	} else if (Value && EQ(Key, "description")) {
	    if (!SoftInfo->DescVerbose) SoftInfo->DescVerbose = strdup(Value);
	} else if (Value && EQ(Key, "copyright")) {
	    if (!SoftInfo->Copyright) SoftInfo->Copyright = strdup(Value);
	} else if (Value && EQ(Key, "os_name")) {
	    if (!SoftInfo->OSname) SoftInfo->OSname = strdup(Value);
	} else if (Value && EQ(Key, "os_release")) {
	    if (!SoftInfo->OSversion) SoftInfo->OSversion = strdup(Value);
	} else if (Value && EQ(Key, "date")) {
	    if (!SoftInfo->BuildDate) SoftInfo->BuildDate = strdup(Value);
	} else if (Value && EQ(Key, "install_date")) {
	    if (!SoftInfo->InstDate) SoftInfo->InstDate = strdup(Value);
	} else if (Value && EQ(Key, "size")) {
	    if (!SoftInfo->DiskUsage) 
		SoftInfo->DiskUsage = (Large_t) strtol(Value, NULL, 0);
	} else if (Value && EQ(Key, "title")) {
	    if (!SoftInfo->Desc && (!SoftInfo->VendorName ||
				    !EQ(SoftInfo->VendorName, Value)))
		SoftInfo->Desc = strdup(Value);
	} else {
	    if (!Ignore(Key, IgnoreKeys, FALSE)) {
		AddDesc(&(SoftInfo->DescList), Capitalize(Key), Value, 0);
	    }
	}
    }

    if (SoftInfo) {
	/* See if we can find the parent's entry */
	if (cp = strrchr(SoftInfo->Name, '.')) {
	    ParName = strdup(SoftInfo->Name);
	    ParName[cp - SoftInfo->Name] = CNULL;
	    Find.Tree = *SoftInfoTree;
	    Find.Name = ParName;
	    Find.Expr = (Find.Version) ? DFE_AND : DFE_OR;
	    if (SoftInfo->Master = SoftInfoFind(&Find)) {
		(void) free(ParName);
	    } else {
		SoftInfo->Master = SoftInfoCreate(NULL);
		SoftInfo->Master->Name = ParName;
	    }
	}
	SoftInfoAdd(SoftInfo, SoftInfoTree, NULL);
    }

    return 0;
}

/*
 * Extract a new file entry starting with line Entry from IO
 */
static int ExtractFileEntry(Entry, IO, SoftFileData)
     char		       *Entry;
     FILE		       *IO;
     SoftFileData_t	      **SoftFileData;
{
    static char			LineBuff[8192];
    SoftFileData_t	       *FileData = NULL;
    char		       *ProdName = NULL;
    char		       *Path = NULL;
    char		       *Line;
    char		       *Key;
    char		       *Value;
    char		       *cp;
    char		      **PkgNames = NULL;
    int				Len;

    if (!Entry || !*Entry || !IO || !SoftFileData)
	return -1;

    if (Entry[0] == '#')
	cp = SkipWhiteSpace(Entry + 1);
    else
	cp = SkipWhiteSpace(Entry);
    if (cp)
	ProdName = strdup(cp);

    if (!ProdName || !strlen(ProdName))
	return -1;

    if (cp = strchr(ProdName, ':')) {
	*cp = CNULL;
	Path = SkipWhiteSpace(cp + 1);
    } else
	return -1;

    while (Line = fgets(LineBuff, sizeof(LineBuff), IO)) {
	Chop(Line);
	SImsg(SIM_DBG, "SWList=<%s>", Line);	
	if (!Line || !*Line) 
	    /* Done with this entry */
	    break;

	Value = NULL;
	Key = Line;
	Len = 0;
	/* This entry could be a "key value" line or a simple "boolean_key" */
	if ((cp = strchr(Key, ' ')) || (cp = strchr(Key, '\t'))) {
	    *cp = CNULL;
	    if (Value = SkipWhiteSpace(cp+1))
		Len = strlen(Value);

	    /* Remove double quotes around whole Value */
	    if (Value && Value[0] == '"' && Len > 1 && Value[Len-1] == '"') {
		Value[Len-1] = CNULL;
		++Value;
	    }
	}

	if (!Value || !Value[0])
	    continue;

	if (!FileData) {
	    FileData = (SoftFileData_t *) xcalloc(1, sizeof(SoftFileData_t));
	    FileData->Path = Path;
	}

	if (Value && EQ(Key, "type")) {
	    switch (Value[0]) {
	    case 'd': FileData->Type = SFT_DIR; break;
	    case 'f': FileData->Type = SFT_FILE; break;
	    case 'h': FileData->Type = SFT_HLINK; break;
	    case 's': FileData->Type = SFT_SLINK; break;
	    default:
		SImsg(SIM_DBG, "Unknown file type <%s> for <%s>", 
		       Value, Path);
	    }
	} else if (Value && EQ(Key, "size")) {
	    if (!FileData->FileSize) 
		FileData->FileSize = (Large_t) strtol(Value, NULL, 0);
	} else if (Value && EQ(Key, "cksum")) {
	    if (!FileData->CheckSum) FileData->CheckSum = strdup(Value);
	} else if (Value && EQ(Key, "link_source")) {
	    if (!FileData->LinkTo) FileData->LinkTo = strdup(Value);
	}
    }

    if (FileData) {
	PkgNames = (char **) xcalloc(2, sizeof(char *));
	PkgNames[0] = ProdName;
	FileData->PkgNames = PkgNames;
	if (!*SoftFileData)
	    *SoftFileData = FileData;
	else {
	    FileData->Next = *SoftFileData;
	    *SoftFileData = FileData;
	}
    }

    return 0;
}

/*
 * Build Software Information tree
 */
extern int BuildSoftInfo(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    static char			LineBuff[8192];
    static char		        SwListCmd[128];
    char		       *Entry;
    char		       *CmdPath;
    FILE 		       *IO;
    int				Header = TRUE;
    time_t			StartTime;
    time_t			EndTime;
    SoftFileData_t	       *SoftFileData = NULL;
    int				FoundProds = 0;
    int				FoundFiles = 0;

    time(&StartTime);

    /* 
     * Build the swlist(1m) command line 
     */
    CmdPath = (char *) CmdFind(SwListPaths);
    if (!CmdPath) {
	SImsg(SIM_GERR, "Cannot find swlist command.");
	return -1;
    }
    (void) snprintf(SwListCmd, sizeof(SwListCmd), 
		    "%s -v -l file -l subproduct",
		    CmdPath);

    IO = popen(SwListCmd, "r");
    if (!IO) {
        SImsg(SIM_GERR, "%s: Start command failed: %s", 
	      SwListCmd, SYSERR);
	return -1;
    }

    while (Entry = fgets(LineBuff, sizeof(LineBuff), IO)) {
	Chop(Entry);
	SImsg(SIM_DBG, "Swlist=<%s>", Entry);	
	/* XXX Special debuging */
	if (EQ(Entry, "# DCE-Core"))
	    SImsg(SIM_DBG, "Found DCE-Core");
	/* Skip over the header which ends with the first empty line */
	if (Header && Entry[0] == '#')
	    continue;
	Header = FALSE;

	/* Start of new entry */
	if (Entry[0] == '#' || Entry[0] == ' ') {
	    if (strchr(Entry, ':')) {
		if (ExtractFileEntry(Entry, IO, &SoftFileData) == 0)
		    ++FoundFiles;
	    } else {
		if (ExtractProductEntry(Entry, IO, SoftInfoTree) == 0)
		    ++FoundProds;
	    }
	}
    }

    (void) pclose(IO);

    time(&EndTime);

    if (FoundProds) {
	/*
	 * Add a list of files we found during the extraction to their
	 * proper parents
	 */
	SoftInfoAddFileData(SoftFileData, SoftInfoTree);
	SImsg(SIM_DBG, "Created %d software info entries in %ld seconds.", 
	      FoundProds, EndTime - StartTime);
	return 0;
    } else {
	SImsg(SIM_DBG, "No software packages were found (%d seconds).",
	      EndTime - StartTime);
	return -1;
    }
}
