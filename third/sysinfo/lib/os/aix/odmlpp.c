/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * AIX interface to building SoftInfo by extracting lpp info from ODM.
 */

#include "defs.h"
#include "product.h"
#include "lpp.h"
#include "inventory.h"

static char    *ODMpaths[] = { "/etc/objrepos", "/usr/lib/objrepos",
			       "/usr/share/lib/objrepos", NULL };

/*
 * Extract all the files we can find from ODM matching LppId
 */
static int ExtractFiles(LppId, SoftInfo)
     short			LppId;
     SoftInfo_t		       *SoftInfo;
{
    static struct inventory     Inv;
    struct stat			Stat;
    SoftFileData_t	       *FileData;
    SoftFileList_t	       *List;
    SoftFileList_t	       *LastList = NULL;
    char			Buff[512];
    int				Op;
    int				Ret;

    SImsg(SIM_DBG, "ODMLPP:           Gathering file info . . .");

    (void) snprintf(Buff, sizeof(Buff), "lpp_id=%d", LppId);

    for (Op = ODM_FIRST; ; Op = ODM_NEXT) {
	/*
	 * Retrieve the object from ODM.
	 */
	Ret = (int) odm_get_obj(inventory_CLASS, Buff, &Inv, Op);
	if (Ret == -1) {
	    SImsg(SIM_GERR, "ODM get lppid=%d from object <%s> failed: %s", 
		  LppId, inventory_CLASS[0].classname, odmerror());
	    return -1;
	} else if (Ret == 0)
	    /*
	     * We're done
	     */
	    break;

	if (!Inv.loc0[0])
	    continue;

	FileData = (SoftFileData_t *) xcalloc(1, sizeof(SoftFileData_t));
	FileData->Path = strdup(Inv.loc0);

	/*
	 * The lstat() call is really slow, but we don't have a choice
	 * since the file data is not kept in ODM.
	 */
	if (lstat(FileData->Path, &Stat) == 0) {
	    switch (Stat.st_mode & S_IFMT) {
	    case S_IFREG: FileData->Type = SFT_FILE; break;
	    case S_IFDIR: FileData->Type = SFT_DIR; break;
	    case S_IFBLK: FileData->Type = SFT_BDEV; break;
	    case S_IFCHR: FileData->Type = SFT_CDEV; break;
	    case S_IFLNK: FileData->Type = SFT_SLINK; break;
	    }
	    FileData->FileSize = (Large_t) Stat.st_size;
	} else
	    SImsg(SIM_DBG, "ODMLPP: %s: stat failed: %s", 
		  FileData->Path, SYSERR);

	if (FileData->Type == SFT_SLINK && Inv.loc2 && Inv.loc2[0])
	    FileData->LinkTo = strdup(Inv.loc2);

	if (Inv.checksum)
	    FileData->CheckSum = strdup(Ltoa((Large_t)Inv.checksum));

	SoftInfo->DiskUsage += FileData->FileSize;

	/*
	 * Add the FileData
	 */
	List = (SoftFileList_t *) xcalloc(1, sizeof(SoftFileList_t));
	List->FileData = FileData;
	if (!LastList)
	    SoftInfo->FileList = LastList = List;
	else {
	    LastList->Next = List;
	    LastList = List;
	}
    }

    return 0;
}

/*
 * Extract all softinfo entries from the product repository located in ODMpath
 */
static int ExtractProduct(ODMpath, SoftInfoTree)
     char		       *ODMpath;
     SoftInfo_t		      **SoftInfoTree;
{
    SoftInfo_t		       *SoftInfo = NULL;
    static char			Buff[512];
    static char			Version[512];
    static struct product	Product;
    static struct lpp		Lpp;
    static SoftInfoFind_t	Find;
    int				Op;
    int				Ret;
    int				Count = 0;
    char		       *OdmMem;
    char		       *cp;
    char		       *ParName;

    if (!ODMpath || !SoftInfoTree)
	return -1;

    odmerrno = 0;
    OdmMem = odm_set_path(ODMpath);
    if (!OdmMem || odmerrno) {
	SImsg(SIM_GERR, "Failed to set ODM path to %s: %s", 
	      ODMpath, odmerror());
	return -1;
    }

    for (Op = ODM_FIRST; ; Op = ODM_NEXT) {
	/*
	 * Retrieve the object from ODM.
	 */
	Ret = (int) odm_get_obj(product_CLASS, (char *)NULL, &Product, Op);
	if (Ret == -1) {
	    SImsg(SIM_GERR, "ODM get object <%s> failed: %s", 
		  product_CLASS[0].classname, odmerror());
	    return -1;
	} else if (Ret == 0)
	    /*
	     * We're done
	     */
	    break;

	if (Product.ver)
	    (void) snprintf(Version, sizeof(Version), "%d.%d.%d.%d",
			    Product.ver, Product.rel, 
			    Product.mod, Product.fix);
	else
	    Version[0] = CNULL;

	SImsg(SIM_DBG, "ODMLPP:     FOUND Product=<%s> Version=<%s>",
	      Product.lpp_name, Version);

	/*
	 * See if we can find an existing entry from a previous pass
	 * through another objrepos, otherwise create a new instance.
	 */
	(void) memset(&Find, 0, sizeof(Find));
	Find.Tree = *SoftInfoTree;
	Find.Name = Product.lpp_name;
	if (Version[0])
	    Find.Version = Version;
	Find.Expr = (Find.Version) ? DFE_AND : DFE_OR;
	if (!(SoftInfo = SoftInfoFind(&Find)))
	    SoftInfo = SoftInfoCreate(NULL);

#define Set(k,v) 	if ( v && v[0] && !k ) k = strdup(v)
	Set(SoftInfo->Name, Product.lpp_name);
	Set(SoftInfo->Name, Product.name);
	Set(SoftInfo->Category, Product.name);
	Set(SoftInfo->Desc, Product.description);
	Set(SoftInfo->VendorStock, Product.comp_id);
	if (Version[0])
	    Set(SoftInfo->Version, Version);

	/*
	 * Now lookup the LPP class info for this object
	 */
	if (Product.lpp_name) {
	    (void) snprintf(Buff, sizeof(Buff), "name=%s", Product.lpp_name);
	    Ret = (int) odm_get_obj(lpp_CLASS, Buff, &Lpp, ODM_FIRST);
	    if (Ret == -1)
		SImsg(SIM_DBG, "ODM get <%s> from object <%s> failed: %s", 
		      Product.lpp_name, lpp_CLASS[0].classname, odmerror());
	    else {
		Set(SoftInfo->Desc, Lpp.description);
		if (!SoftInfo->Version && Lpp.ver) {
		    (void) snprintf(Buff, sizeof(Buff), "%d.%d.%d.%d",
				    Lpp.ver, Lpp.rel, 
				    Lpp.mod, Lpp.fix);
		    Set(SoftInfo->Version, Buff);
		}
		if (Lpp.lpp_id)
		    ExtractFiles(Lpp.lpp_id, SoftInfo);
	    }
	}
#undef	Set

	/*
	 * Add SoftInfo to the tree
	 */
	if (SoftInfo->Name) {
	    /* See if we can figure out the parent's name and find it */
	    if (cp = strrchr(SoftInfo->Name, '.')) {
		ParName = strdup(SoftInfo->Name);
		ParName[cp - SoftInfo->Name] = CNULL;
		(void) memset(&Find, 0, sizeof(Find));
		Find.Tree = *SoftInfoTree;
		Find.Name = ParName;
		if (SoftInfo->Master = SoftInfoFind(&Find)) {
		    (void) free(ParName);
		} else {
		    SoftInfo->Master = SoftInfoCreate(NULL);
		    SoftInfo->Master->Name = ParName;
		}
	    }
	    SoftInfoAdd(SoftInfo, SoftInfoTree, NULL);
	    ++Count;
	} else if (SoftInfo)
	    SoftInfoDestroy(SoftInfo);
    }

    if (OdmMem)
	(void) free(OdmMem);

    return Count;
}

/*
 * Build Software Information tree
 */
extern int BuildSoftInfo(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    time_t			StartTime;
    time_t			EndTime;
    int				Found = 0;
    register int		i;

    time(&StartTime);

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return -1;
    }

    for (i = 0; ODMpaths[i]; ++i) {
	SImsg(SIM_DBG, "ODMLPP: Extracting Products from %s . . .", 
	      ODMpaths[i]);
	Found += ExtractProduct(ODMpaths[i], SoftInfoTree);
    }

    if (odm_terminate() != 0)
	SImsg(SIM_WARN, "ODM Terminate did not succeed.");

    time(&EndTime);

    if (Found) {
	SImsg(SIM_DBG, "Created %d software info entries in %ld seconds.", 
	      Found, EndTime - StartTime);
	return 0;
    } else {
	SImsg(SIM_DBG, "No software packages were found (%d seconds).",
	      EndTime - StartTime);
	return -1;
    }
}

/*
 * Get OS version from ODM
 */
extern char *GetOSVerODM()
{
    static char			Version[32];
    register int		i;
    char		       *OdmMem = NULL;
    static struct product	Product;

    if (Version[0])
	return Version;

    if (odm_initialize() == -1) {
	SImsg(SIM_GERR, "ODM initialize failed: %s", odmerror());
	return (char *) NULL;
    }

    Version[0] = CNULL;

    for (i = 0; ODMpaths[i] && !Version[0]; ++i) {
	SImsg(SIM_DBG, "GetOSVerODM: Opening %s . . .", 
	      ODMpaths[i]);

	odmerrno = 0;
	OdmMem = odm_set_path(ODMpaths[i]);
	if (!OdmMem || odmerrno) {
	    SImsg(SIM_GERR, "Failed to set ODM path to %s: %s", 
		  ODMpaths[i], odmerror());
	    continue;
	}

	if (odm_get_obj(product_CLASS, "lpp_name=bos.rte", &Product, 
			ODM_FIRST)) {
	    if (Product.ver) {
		(void) snprintf(Version, sizeof(Version), "%d.%d.%d.%d",
				Product.ver, Product.rel, 
				Product.mod, Product.fix);
		SImsg(SIM_DBG, "GetOSVerODM: Found <%s>", Version);
	    }
	}

	if (OdmMem)
	    (void) free(OdmMem);
    }

    if (odm_terminate() != 0)
	SImsg(SIM_WARN, "ODM Terminate did not succeed.");

    return (Version[0]) ? Version : NULL;
}
