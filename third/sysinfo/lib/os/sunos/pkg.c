/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Functions relating to the Solaris pkg (software package) format.
 */

#include "defs.h"
#include <dirent.h>

/* Standard message in most SUNW pkgs saying there's no phone number */
#define NO_PHONE_MSG "Please contact your local service provider"

/*
 * Get and then Set the Version and Revision from a Version string (String)
 */
static void SetVersion(SoftInfo, String)
     SoftInfo_t		       *SoftInfo;
     char		       *String;
{
    register char	       *cp;
    char		       *Version = NULL;
    char		       *Revision = NULL;

    if (!SoftInfo || !String)
	return;

    /*
     * String should be one of:
     *  X.X
     * 	X.X,REV=...
     */
    Version = String;
    if (cp = strchr(String, ',')) {
	*cp++ = CNULL;
	if (EQN(cp, "REV=", 4)) {
	    Revision = cp + 4;
	}
    }

    if (Version && *Version)
	SoftInfo->Version = strdup(Version);
    if (Revision && *Revision)
	SoftInfo->Revision = strdup(Revision);
}

/*
 * Extract software info from FileName/FilePtr and return results.
 * See pkginfo(4)
 */
static SoftInfo_t *PkgExtract(SoftInfoTree, PkgName, FileName, FilePtr)
     SoftInfo_t		      **SoftInfoTree;
     char		       *PkgName;
     char		       *FileName;
     FILE		       *FilePtr;
{
    static char			Buff[1024];
    SoftInfo_t		       *SoftInfo;
    SoftInfo_t		       *Product = NULL;
    char		       *Key;
    char		       *Value;
    char		       *ProdName = NULL;
    char		       *ProdVers = NULL;
    register char	       *cp;

    if (!FileName || !FilePtr)
	return (SoftInfo_t *) NULL;

    SoftInfo = SoftInfoCreate(NULL);

    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	if (Buff[0] == CNULL)
	    continue;

	/* Set Key */
	Key = Buff;
	/* Set Value */
	if (!(cp = strchr(Buff, '=')))
	    continue;
	Value = cp+1;
	*cp = CNULL;

	if (Value && (cp = strchr(Value, '\n')))
	    *cp = CNULL;
	if (!Value || *Value == CNULL)
	    continue;

	if (EQ(Key, "PKG")) {
	    SoftInfo->Name = strdup(Value);
	} else if (EQ(Key, "PKGINST")) {
	    /* PKGINST is the same as PKG but PKG is usually better */
	    if (!SoftInfo->Name)
		SoftInfo->Name = strdup(Value);
	} else if (EQ(Key, "VERSION")) {
	    SetVersion(SoftInfo, Value);
	} else if (EQ(Key, "NAME")) {
	    SoftInfo->Desc = strdup(Value);
	} else if (EQ(Key, "DESC")) {
	    SoftInfo->DescVerbose = strdup(Value);
	} else if (EQ(Key, "ARCH")) {
	    SoftInfo->Arch = strdup(Value);
	} else if (EQ(Key, "SUNW_ISA")) {
	    SoftInfo->ISArch = strdup(Value);
	} else if (EQ(Key, "CATEGORY")) {
	    SoftInfo->Category = strdup(Value);
	} else if (EQ(Key, "PKGTYPE") || EQ(Key, "SUNW_PKGTYPE")) {
	    SoftInfo->SubCategory = strdup(Value);
	} else if (EQ(Key, "INSTDATE")) {
	    SoftInfo->InstDate = strdup(Value);
	} else if (EQ(Key, "PSTAMP")) {
	    SoftInfo->ProdStamp = strdup(Value);
	} else if (EQ(Key, "BASEDIR")) {
	    SoftInfo->BaseDir = strdup(Value);
	} else if (EQ(Key, "VSTOCK")) {
	    SoftInfo->VendorStock = strdup(Value);
	} else if (EQ(Key, "VENDOR")) {
	    SoftInfo->VendorName = strdup(Value);
	} else if (EQ(Key, "EMAIL")) {
	    SoftInfo->VendorEmail = strdup(Value);
	} else if (EQ(Key, "HOTLINE")) {
	    if (!EQ(Value, NO_PHONE_MSG))
		SoftInfo->VendorPhone = strdup(Value);
	} else if (EQ(Key, "PRODNAME") || EQ(Key, "SUNW_PRODNAME")) {
	    ProdName = strdup(Value);
	} else if (EQ(Key, "PRODVERS") || EQ(Key, "SUNW_PRODVERS")) {
	    ProdVers = strdup(Value);
	} else
	    AddDesc(&(SoftInfo->DescList), ExpandKey(Key), 
		    strdup(Value), 0);
    }

    /* We're a package by default */
    SoftInfo->EntryTypeNum = MC_SET_PKG;

    /* Create a Product (our Master) if we found enough info */
    if (SoftInfo && ProdName) {
	Product = SoftInfoCreate(NULL);
	Product->EntryTypeNum = MC_SET_PROD;
	Product->Name = ProdName;
	Product->Version = ProdVers;
	SoftInfo->Master = Product;
    }

    if (SoftInfo && !SoftInfo->Name) {
	SoftInfoDestroy(SoftInfo);
	return (SoftInfo_t *) NULL;
    }

    return SoftInfo;
}

/*
 * Parse TypeStr and return the type of file this should be.
 */
static int GetFileType(TypeStr)
     char		       *TypeStr;
{
    if (!TypeStr)
	return 0;

    switch (tolower(*TypeStr)) {
    case 'v':	/* 'v' volatile file (contents change over time) */
    case 'i':	/* 'i' is installation file */
    case 'e':	/* 'e' means a regular file which needs to be edited */
    case 'f':	return SFT_FILE;
    case 'l':	return SFT_HLINK;
    case 's':	return SFT_SLINK;
    case 'x':	/* Exclusive Directory used by 1 pkg */
    case 'd':	return SFT_DIR;
    case 'b':	return SFT_BDEV;
    case 'c':	return SFT_CDEV;
    case 'p':	/* Named pipe - not sure what format of entry is */
    default:
	SImsg(SIM_DBG, "%s: Entry with unknown file type `%c' found.", 
	      _PATH_PKG_CONTENTS, *TypeStr);
    }

    return 0;
}

/*
 * Get a list of packages from String where String is a space seperate list
 * of package names.
 * Note: String may be NULL which causes strtok() to use the last s1 arg
 * it was passed.
 */
static char **GetPkgNames(String)
     char		       *String;
{
    char		      **List = NULL;
    char		       *cp;
    char		       *Name;
    int				Count = 0;

    while (cp = strtok(String, " ")) {
        if (Count == 0)
	    List = (char **) xcalloc(1, sizeof(char *));
	else
	    List = (char **) xrealloc(List, (Count+1) * sizeof(char *));

	Name = strdup(cp);
	/* Name might be "SUNWxx:none" and we want "SUNWxx" */
	if (cp = strchr(Name, ':'))
	    *cp = CNULL;

	List[Count++] = Name;
    }

    if (Count) {
        List = (char **) xrealloc(List, (Count+1) * sizeof(char *));
        List[Count] = NULL;
    }

    return List;
}

/*
 * Build a list of software file data by reading the 'contents' file.
 */
static int SoftInfoBuildFileData(SoftFileData)
     SoftFileData_t	      **SoftFileData;
{
    FILE		       *FilePtr;
    static char			Buff[2048];
    SoftFileData_t	       *New;
    char		       *cp;
    int				Type;
    char		       *Name;
    char		       *Path;
    char		       *LinkTo;
    char		       *TypeChar;
    char		       *SizeStr;
    char		      **PkgNames;
    ulong_t			Count = 0;			
    time_t			StartTime;
    time_t			EndTime;

    if (!(FilePtr = fopen(_PATH_PKG_CONTENTS, "r"))) {
	SImsg(SIM_GERR, "%s: Open for reading failed: %s",
	      _PATH_PKG_CONTENTS, SYSERR);
	return -1;
    }

    SImsg(SIM_DBG, "BuildFileData: Building file data from %s ...",
	  _PATH_PKG_CONTENTS);
    time(&StartTime);

    /*
     * The format of this file is not directly documented, but the
     * pkgmap(5) man page provides many hints.
     */
    while (fgets(Buff, sizeof(Buff), FilePtr)) {
	if (Buff[0] == '#')		/* Comment */
	    continue;
        if (cp = strchr(Buff, '\n'))	/* Zap newline */
	    *cp = CNULL;
	Name = Path = LinkTo = SizeStr = NULL;
	PkgNames = NULL;
	/* Field 0 */
	if (!(Name = strtok(Buff, " ")))	continue;
	/* Field 1 */
	if (!(TypeChar = strtok(NULL, " ")))	continue;
	/* Field 2 - Class*/
	if (!strtok(NULL, " "))	    		continue;

	switch (Type = GetFileType(TypeChar)) {
	case SFT_FILE:
	    Path = Name;
	    /* Field 3 - File Mode */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 4 - Owner */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 5 - Group */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 6 - File Size */
	    if (!(SizeStr = strtok(NULL, " ")))	continue;
	    /* Field 7 - ??? */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 8 - Create Time */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 9 ... - Pkg Names */
	    PkgNames = GetPkgNames(NULL);
	    break;
	case SFT_CDEV:
	case SFT_BDEV:
	    Path = Name;
	    /* Field 3 - Major # */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 4 - Minor # */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 5 - File Mode */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 6 - Owner */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 7 - Group */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 8 ... - Pkg Names */
	    PkgNames = GetPkgNames(NULL);
	    break;
	case SFT_DIR:
	    Path = Name;
	    /* Field 3 - File Mode */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 4 - Owner */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 5 - Group */
	    if (!strtok(NULL, " "))		continue;
	    /* Field 6 ... - Pkg Names */
	    PkgNames = GetPkgNames(NULL);
	    break;
	case SFT_HLINK:
	case SFT_SLINK:
	    /* Can't use strtok() here in order to preserve Buff */
	    Path = Name;
	    if (cp = strchr(Path, '='))
	        *cp++ = CNULL;
	    LinkTo = cp;
	    /* Field 7 ... - Pkg Names */
	    PkgNames = GetPkgNames(NULL);
	    break;
	}
	if (!Path)
	    continue;

	/* Create the new entry */
	New = (SoftFileData_t *) xcalloc(1, sizeof(SoftFileData_t));
	New->Type = Type;
	New->Path = strdup(Path);
	New->LinkTo = (LinkTo) ? strdup(LinkTo) : NULL;
	if (SizeStr)
	    New->FileSize = (Large_t) atol(SizeStr);
	New->PkgNames = PkgNames;
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

    fclose(FilePtr);

    time(&EndTime);
    SImsg(SIM_DBG, 
	  "BuildFileData: Finished building file data (Entries=%ld Time=%ds)",
	  Count, EndTime-StartTime);

    return 0;
}

/*
 * Build Software Information tree
 */
extern int BuildSoftInfo(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    SoftInfo_t		       *SoftInfo;
    SoftFileData_t	       *SoftFileData = NULL;
    static char			PkgFile[MAXPATHLEN];
    DIR			       *DirPtr;
    struct dirent	       *DirEnt;
    FILE		       *FilePtr;
    int				Found = 0;
    time_t			StartTime;
    time_t			EndTime;

    time(&StartTime);

    if (!(DirPtr = opendir(_PATH_PKG_DIR))) {
	SImsg(SIM_GERR, "%s: Open directory failed: %s", 
	      _PATH_PKG_DIR, SYSERR);
	return -1;
    }

    while (DirEnt = readdir(DirPtr)) {
	if (EQ(DirEnt->d_name, ".") || EQ(DirEnt->d_name, ".."))
	    continue;
	
	(void) snprintf(PkgFile, sizeof(PkgFile), "%s/%s/pkginfo", 
			_PATH_PKG_DIR, DirEnt->d_name);

	if (!(FilePtr = fopen(PkgFile, "r"))) {
	    SImsg(SIM_GERR, "%s: Open failed: %s", PkgFile, SYSERR);
	    continue;
	}

	SoftInfo = PkgExtract(SoftInfoTree, DirEnt->d_name, 
			      PkgFile, FilePtr);

	(void) fclose(FilePtr);

	if (SoftInfoAdd(SoftInfo, SoftInfoTree, SearchExp) == 0)
	    ++Found;
    }
    
    (void) closedir(DirPtr);

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
