/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Software Information
 *
 * Obtain software (package, product) information from a system.
 */
#include "bjhashtab.h"
#include "defs.h"

static htab_t	 	      *TreeHashTabByName; 	/* By Name */
static htab_t	 	      *TreeHashTabByVersion; 	/* By Version */

/*
 * Create a new SoftInfo_t 
 */
extern SoftInfo_t *SoftInfoCreate(Old)
     SoftInfo_t		       *Old;
{
    SoftInfo_t		       *New;

    New = (SoftInfo_t *) xcalloc(1, sizeof(SoftInfo_t));

    return(New);
}

/*
 * Destroy a SoftInfo_t
 */
extern int SoftInfoDestroy(What)
     SoftInfo_t		       *What;
{
    if (!What)
	return -1;

    if (What->Name)		(void) free(What->Name);
    if (What->Version)		(void) free(What->Version);
    if (What->Revision)		(void) free(What->Revision);
    if (What->Desc)		(void) free(What->Desc);
    if (What->DescVerbose)	(void) free(What->DescVerbose);
    if (What->Category)		(void) free(What->Category);
    if (What->SubCategory)	(void) free(What->SubCategory);
    if (What->Arch)		(void) free(What->Arch);
    if (What->ISArch)		(void) free(What->ISArch);
    if (What->InstDate)		(void) free(What->InstDate);
    if (What->ProdStamp)	(void) free(What->ProdStamp);
    if (What->BaseDir)		(void) free(What->BaseDir);
    if (What->VendorName)	(void) free(What->VendorName);
    if (What->VendorEmail)	(void) free(What->VendorEmail);
    if (What->VendorPhone)	(void) free(What->VendorPhone);
    if (What->VendorStock)	(void) free(What->VendorStock);
    if (What->DescList)		(void) DestroyDesc(What->DescList);
    /* XXX Need to destroy FileData */

    (void) free(What);

    return 0;
}

/*
 * Get Version string for use as hash key
 */
static char *GetVers(Name, Version)
     char		       *Name;
     char		       *Version;
{
    static char			Buff[256];

    snprintf(Buff, sizeof(Buff), "%s-%s", 
	     (Name) ? Name : "",
	     (Version) ? Version : "");

    return Buff;
}

/*
 * Get the string name of the file Type.
 */
static char *FileTypes[] = { 
    NULL, SFT_FILE_S, SFT_HLINK_S, SFT_SLINK_S, SFT_DIR_S, 
    SFT_CDEV_S, SFT_BDEV_S
};

extern char *SoftInfoFileType(Type)
     int			Type;
{
    if (Type < 1 || Type > (sizeof(FileTypes)/sizeof(char *)))
	return (char *) NULL;

    return (char *) FileTypes[Type];
}

/*
 * Get Entry Type string values.
 * These values correspond to MC_SET_* in mcsysinfo.h
 */
static char *EntryTypes[] = {
    NULL, MC_SET_PKG_S, MC_SET_PROD_S
};
static char *SoftInfoEntryType(Type)
     int			Type;
{
    if (Type < 1 || Type > (sizeof(EntryTypes)/sizeof(char *)))
	return (char *) NULL;

    return (char *) EntryTypes[Type];
}

/*
 * Add a reason (What) to why Find matched
 */
static void SoftInfoFindAddMatch(Find, What)
     SoftInfoFind_t	       *Find;
     char		       *What;
{
    if (!Find || !What)
	return;

    if ((strlen(Find->Reason) + strlen(What) + 2) >= sizeof(Find->Reason)) {
#ifdef EXTRA_DEBUG
	SImsg(SIM_DBG, "SoftInfoFindAddMatch(..., %s): Find->Reason is full.", 
	      What);
#endif	/* EXTRA_DEBUG */
	return;
    }
	      
    if (Find->Reason && Find->Reason[0])
	(void) strcat(Find->Reason, ",");
    (void) strcat(Find->Reason, What);
}

/*
 * See if the pkg Name matches
 */
static int SoftInfoFindMatchName(Find)
     SoftInfoFind_t	       *Find;
{
    SoftInfo_t		       *Tree;
    SoftInfo_t		       *Found = NULL;

    if (!Find || !Find->Tree || !Find->Name)
	return FALSE;

    Tree = Find->Tree;

    if (EQ(Find->Name, Tree->Name))
	Found = Tree;

    if (Found) {
	SoftInfoFindAddMatch(Find, "Name");
	return TRUE;
    }

    return FALSE;
}

/*
 * See if the pkg Version matches
 */
static int SoftInfoFindMatchVersion(Find)
     SoftInfoFind_t	       *Find;
{
    SoftInfo_t		       *Tree;
    SoftInfo_t		       *Found = NULL;

    if (!Find || !Find->Tree || !Find->Version)
	return FALSE;

    Tree = Find->Tree;

    if (EQ(Find->Version, Tree->Version))
	Found = Tree;

    if (Found) {
	SoftInfoFindAddMatch(Find, "Version");
	return TRUE;
    }

    return FALSE;
}

/*
 * Check to see if SoftInfo's Soft1 and Soft2 are consistant.
 * If there is a discrepancy between the two due to one
 * device not having it's value set, then set it to the
 * other value.  Basically this "fills in the blanks".
 */
static void CheckSoftInfo(Soft1, Soft2)
     SoftInfo_t		       *Soft1;
     SoftInfo_t		       *Soft2;
{
#define CHECK(a,b) \
    if (a != b) { \
	if (a) \
	    b = a; \
	else if (b) \
	    a = b; \
    }

    CHECK(Soft1->EntryType, 	Soft2->EntryType);
    CHECK(Soft1->Version,	Soft2->Version);
    CHECK(Soft1->Desc,		Soft2->Desc);
    CHECK(Soft1->DescVerbose,	Soft2->DescVerbose);
    CHECK(Soft1->Category,	Soft2->Category);
    CHECK(Soft1->Arch, 		Soft2->Arch);
    CHECK(Soft1->InstDate,	Soft2->InstDate);
    CHECK(Soft1->ProdStamp,	Soft2->ProdStamp);
    CHECK(Soft1->VendorName, 	Soft2->VendorName);
    CHECK(Soft1->VendorEmail, 	Soft2->VendorEmail);
    CHECK(Soft1->VendorPhone, 	Soft2->VendorPhone);

#undef CHECK
}

/*
 * Find a SoftInfo entry using the Find query.
 * Normally we find what we want in one of the hash tables (TreeHashBy*)
 * in a single call.  If the hash table is not initialized, then we
 * do a slow, recursive search.
 * --RECURSE--
 */
extern SoftInfo_t *SoftInfoFind(Find)
     SoftInfoFind_t	       *Find;
{
    SoftInfo_t		       *Ptr;
    SoftInfo_t		       *Tree;
    SoftInfo_t		       *Found = NULL;
    char		       *cp;

    if (!Find || !Find->Tree)
	return (SoftInfo_t *) NULL;

    Tree = Find->Tree;

    /*
     * We currently don't have a hash table for Find->Addr lookups since
     * our hash functions don't support binary hash values right now.
     */
    if (Find->Addr) {
	if (Tree && Tree == Find->Addr)
	    return(Tree);
    }

    if (Find->Expr == DFE_OR) {
	/*
	 * Do a quick, onetime lookup in the hash table if we can
	 */
	if (TreeHashTabByName || TreeHashTabByVersion) {
	    if (TreeHashTabByName && Find->Name &&
		hfind(TreeHashTabByName, Find->Name, strlen(Find->Name)))
		Found = (SoftInfo_t *) hstuff(TreeHashTabByName);

	    if (!Found && TreeHashTabByVersion && Find->Version &&
		(cp = GetVers(Find->Name, Find->Version)) &&
		hfind(TreeHashTabByVersion, cp, strlen(cp)))
		Found = (SoftInfo_t *) hstuff(TreeHashTabByVersion);
	    return Found;
	}

	/*
	 * Do it the slow way (recursive)
	 */
	if (SoftInfoFindMatchName(Find) ||
	    SoftInfoFindMatchVersion(Find))
	    return Tree;
    } else if (Find->Expr == DFE_AND) {
	/*
	 * Do a quick, onetime lookup in the hash table if we can
	 */
	if (TreeHashTabByName && TreeHashTabByVersion) {
	    if (TreeHashTabByName && Find->Name && 
		TreeHashTabByVersion && Find->Version &&
		(cp = GetVers(Find->Name, Find->Version)) &&
		hfind(TreeHashTabByName, Find->Name, strlen(Find->Name)) &&
		hfind(TreeHashTabByVersion, cp, strlen(cp))) {

		Found = (SoftInfo_t *) hstuff(TreeHashTabByVersion);
		return Found;
	    }
	    return (SoftInfo_t *) NULL;
	}

	/*
	 * Do it the slow way (recursive)
	 */
	if (SoftInfoFindMatchName(Find) &&
	    SoftInfoFindMatchVersion(Find))
	    return Tree;
    } else {
	SImsg(SIM_DBG, "SoftInfoFind(): Expr %d unknown.", Find->Expr);
	return (SoftInfo_t *) NULL;
    }

    if (Tree->Slaves) {
	Find->Tree = Tree->Slaves;
	if (Ptr = SoftInfoFind(Find))
	    return(Ptr);
    }

    if (Tree->Next) {
	Find->Tree = Tree->Next;
	if (Ptr = SoftInfoFind(Find))
	    return(Ptr);
    }

    return((SoftInfo_t *) NULL);
}

/*
 * Sanitize Soft entry
 */
static int Sanitize(Soft)
     SoftInfo_t		       *Soft;
{
    if (!Soft)
	return -1;

    if (!Soft->EntryTypeNum)
	Soft->EntryTypeNum = MC_SET_PKG;

    Soft->EntryType = SoftInfoEntryType(Soft->EntryTypeNum);

    return 0;
}

/*
 * Add an entry (New) to the tree (SoftInfoTree)
 * --RECURSE--
 */
extern int SoftInfoAdd(New, SoftInfoTree, SearchExp)
     SoftInfo_t		       *New;
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    SoftInfo_t		       *Master;
    static SoftInfoFind_t	Find;
    SoftInfo_t		       *mp;
    char		       *Vers;

    if (!New || !SoftInfoTree) {
	SImsg(SIM_GERR, "Invalid parameters passed to AddSoftInfo()");
	return -1;
    }

    /*
     * Only add it if it's in our search list or no SearchExp was given
     */
    if (SearchExp && New->Name)
	if (SearchCheck(New->Name, SearchExp) != 0) {
	    SImsg(SIM_DBG, "AddSoftInfo: Package <%s> not in search list.",
		  New->Name);
	    return -1;
	}

    Sanitize(New);

    if (New->Master && !SearchExp) {
	Master = NULL;
	if (SoftInfoTree && *SoftInfoTree) {
	    (void) memset(&Find, 0, sizeof(Find));
	    Find.Tree = *SoftInfoTree;
	    Find.Name = New->Master->Name;
	    Find.Version = New->Master->Version;
	    Find.Expr = (Find.Version) ? DFE_AND : DFE_OR;
	    Master = SoftInfoFind(&Find);
	    if (Master)
		/* Check and adjust any diffs we find between entries */
		CheckSoftInfo(Master, New->Master);
	}

	/* No existing parent was found so create the Master now */
	if (!Master) {
	    if (SoftInfoAdd(New->Master, SoftInfoTree, SearchExp) != 0) {
		SImsg(SIM_GERR, "Add master for <%s> to softinfo tree failed.",
		      New->Name);
		return -1;
	    }
	    Master = New->Master;
	}
    } else {
	if (!*SoftInfoTree)
	    /* The tree is empty so create a new one */
	    *SoftInfoTree = SoftInfoCreate(NULL);
	/* The entry doesn't have a Master so make it a child of the root */
	Master = *SoftInfoTree;
    }

    if (Master->Slaves) {
	/* Add to existing list of slaves */
	for (mp = Master->Slaves; mp && mp->Next; mp = mp->Next)
	    /* Check to see if New is already in Master's Slaves list */
	    if (New == mp) {
		SImsg(SIM_DBG, "SoftInfoAdd: FOUND DUP <%s> <%s>",
		      mp->Name, mp->Version);
		break;
	    }
	/* Don't add duplicates */
	if (New != mp)
	    mp->Next = New;
    } else
	/* Create first slave */
	Master->Slaves = New;

    /*
     * Add it to the Hash Tables
     */
    if (!TreeHashTabByName)
	TreeHashTabByName = hcreate(128);
    hadd(TreeHashTabByName, New->Name, strlen(New->Name), New);

    if (!TreeHashTabByVersion)
	TreeHashTabByVersion = hcreate(128);
    if (Vers = GetVers(New->Name, New->Version))
	hadd(TreeHashTabByVersion, Vers, strlen(Vers), New);

    return(0);

}

/*
 * Add the list of software file data (SoftFileData) to the SoftInfo
 * tree (SoftInfoTree).
 */
extern int SoftInfoAddFileData(SoftFileData, SoftInfoTree)
     SoftFileData_t	       *SoftFileData;
     SoftInfo_t		      **SoftInfoTree;
{
    register SoftFileData_t    *Data;
    register char	      **Pkgs;
    SoftFileList_t	       *New;
    SoftInfo_t		       *SoftInfo;
    SoftInfoFind_t		Find;

    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *SoftInfoTree;
    Find.Expr = DFE_OR;

    /*
     * Each file data entry has a list of package names (PkgNames) that
     * we lookup in the SoftInfo table and add to the table entry we find.
     */
    for (Data = SoftFileData; Data; Data = Data->Next) {
        for (Pkgs = Data->PkgNames; Pkgs && *Pkgs; ++Pkgs) {
	    if (!*Pkgs)
	        continue;
	    Find.Name = *Pkgs;
	    if (SoftInfo = SoftInfoFind(&Find)) {
	        New = (SoftFileList_t *) xcalloc(1, sizeof(SoftFileList_t));
		New->FileData = Data;
		New->Next = SoftInfo->FileList;
		SoftInfo->FileList = New;
		SoftInfo->DiskUsage += Data->FileSize;
	    } else {
		SImsg(SIM_DBG, 
	    "AddFileData: Cannot locate pkg <%s> in SoftInfo table.", 
		      *Pkgs);
	    }
	}
    }

    return 0;
}

/*
 * Interface function between mcSysInfo() and underlying probes.
 */
extern int SoftInfoMCSI(Query)
     MCSIquery_t	       *Query;
{
#if	defined(HAVE_SOFTINFO_SUPPORT)
    static SoftInfo_t 	       *Root;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    switch (Query->Op) {
    case MCSIOP_CREATE:
	if (!Root) {
	    SImsg(SIM_DBG, "BUILDING Software Tree ...");
	    if (BuildSoftInfo(&Root, Query->SearchExp) != 0)
		return -1;
	}

	if (!Root) {
	    SImsg(SIM_DBG, "No software information was found.");
	    errno = ENOENT;
	    return -1;
	}

	Query->Out = (Opaque_t) Root;
	Query->OutSize = 1;
	break;
    case MCSIOP_DESTROY:
	if (Query->Out && Query->OutSize)
	    return SoftInfoDestroy((SoftInfo_t *) Query->Out);
	break;
    }
#else	/* !HAVE_SOFTINFO_SUPPORT */
    SImsg(SIM_DBG, 
"Support for `software' class information is not available on this platform.");
#endif	/* HAVE_SOFTINFO_SUPPORT */

    return 0;
}
