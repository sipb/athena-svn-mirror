/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Partition Information
 *
 * Obtain information about what partitions (filesystem, swap, etc) that
 * exist on the local system.
 *
 * This module is unique in that it does nothing OS specific directly.
 * Instead, it gets a standard DevInfo_t device tree through the mcSysInfo()
 * interface and then traverses the DevInfo_t tree looking for partitioning
 * information which it then uses to "create" it's view of PartInfo.
 */

#include "defs.h"

/*
 * Scan through each DiskPart and add the PartInfo to our list.
 */
static int GetPartInfoDisk(PartInfoTree, DiskPart)
     PartInfo_t		      **PartInfoTree;
     DiskPart_t		       *DiskPart;
{
    DiskPart_t		       *dp;

    for (dp = DiskPart; dp; dp = dp->Next)
	PartInfoAdd(PartInfoTree, dp->PartInfo);

    return 0;
}

/*
 * Get partition information from DevInfo and add it to PartInfoTree.
 * --RECURSE--
 */
static int GetPartInfo(PartInfoTree, DevInfo, SearchExp)
     PartInfo_t		      **PartInfoTree;
     DevInfo_t		       *DevInfo;
     char		      **SearchExp;
{
    static DevInfo_t	       *DevPtr;
    static DiskDriveData_t     *DiskData;

    if (!PartInfoTree || !DevInfo)
	return -1;

    if (DevInfo->Type == DT_DISKDRIVE && DevInfo->DevSpec) {
	DiskData = (DiskDriveData_t *) DevInfo->DevSpec;
	if (DiskData->HWdata &&
	    DiskData->HWdata->DiskPart && 
	    DiskData->HWdata->DiskPart->PartInfo)
	    GetPartInfoDisk(PartInfoTree, DiskData->HWdata->DiskPart);
	if (DiskData->OSdata &&
	    DiskData->OSdata->DiskPart && 
	    DiskData->OSdata->DiskPart->PartInfo)
	    GetPartInfoDisk(PartInfoTree, DiskData->OSdata->DiskPart);
    }

    /*
     * Descend
     */
    (void) GetPartInfo(PartInfoTree, DevInfo->Slaves, SearchExp);

    /*
     * Traverse
     */
    if (DevInfo->Next)
	(void) GetPartInfo(PartInfoTree, DevInfo->Next, SearchExp);

    return 0;
}

/*
 * Build Partition information using device info and return it in PartInfoTree.
 */
extern int BuildPartInfoDevices(PartInfoTree, SearchExp)
     PartInfo_t		      **PartInfoTree;
     char		      **SearchExp;
{
    PartInfo_t		       *PartInfo;
    static DevInfo_t 	       *RootDev = NULL;
    static MCSIquery_t		Query;

    if (!PartInfoTree)
	return -1;

    /*
     * We depend on the information being in the device tree.
     * Once we have the device tree, we scan it looking for partition info
     * and add what we find to PartInfoTree.
     */

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_DEVTREE;

    if (mcSysInfo(&Query) != 0) {
	return -1;
    }

    RootDev = (DevInfo_t *) Query.Out;
    if (!RootDev) {
	SImsg(SIM_DBG, 
	      "No device information found, hence no partition info.");
	return -1;
    }

    GetPartInfo(PartInfoTree, RootDev, SearchExp);

    if (PartInfoTree && *PartInfoTree)
	return 0;
    else {
	SImsg(SIM_DBG, "No partition information was found on this system.");
	return -1;
    }
}

/*
 * Create a new PartInfo_t copying Old to the new if specified.
 */
extern PartInfo_t *PartInfoCreate(Old)
     PartInfo_t		       *Old;
{
    PartInfo_t		       *New;
    register char	      **cpp;
    register int		n;

    New = (PartInfo_t *) xcalloc(1, sizeof(PartInfo_t));

    if (Old) {
	if (Old->DevPath)	New->DevPath = strdup(Old->DevPath);
	if (Old->DevPathRaw)	New->DevPath = strdup(Old->DevPathRaw);
	if (Old->DevName)	New->DevName = strdup(Old->DevName);
	if (Old->Name)		New->Name = strdup(Old->Name);
	if (Old->MntName)	New->MntName = strdup(Old->MntName);
	if (Old->Type)		New->Type = strdup(Old->Type);
	New->TypeNum		= Old->TypeNum;
	New->Num		= Old->Num;
	New->Usage		= Old->Usage;
	New->SecSize		= Old->SecSize;
	New->NumSect		= Old->NumSect;
	if (Old->MntOpts) {
	    for (n = 0, cpp = Old->MntOpts; cpp && *cpp; ++cpp, ++n);
	    New->MntOpts = xcalloc(1, sizeof(char *)*(n+1));
	    for (n = 0, cpp = Old->MntOpts; cpp && *cpp; ++cpp, ++n)
		New->MntOpts[n] = strdup(Old->MntOpts[n]);
	}
    }

    return New;
}

/*
 * Destroy Part
 */
extern int PartInfoDestroy(Part)
     PartInfo_t		       *Part;
{
    char		     **cpp;
    char		     **Next = NULL;
    
    if (!Part)
	return -1;

    if (Part->DevPath)		(void) free(Part->DevPath);
    if (Part->DevPathRaw)	(void) free(Part->DevPathRaw);
    if (Part->DevName)		(void) free(Part->DevName);
    if (Part->Name)		(void) free(Part->Name);
    if (Part->MntName)		(void) free(Part->MntName);
    if (Part->Type)		(void) free(Part->Type);

    if (Part->MntOpts) {
	for (cpp = Part->MntOpts; cpp && *cpp; ) {
	    Next = cpp+1;
	    (void) free(*cpp);
	    cpp = Next;
	}
	(void) free(Part->MntOpts);
    }

    (void) free(Part->MntOpts);

    return 0;
}

/*
 * Add Part to PartInfoPtr
 */
extern int PartInfoAdd(PartInfoPtr, Part)
     PartInfo_t		      **PartInfoPtr;
     PartInfo_t		       *Part;
{
    register PartInfo_t	       *Ptr;
    register PartInfo_t	       *End = NULL;

    if (!PartInfoPtr)
	return -1;

    for (Ptr = *PartInfoPtr; Ptr; Ptr = Ptr->Next) {
	End = Ptr;
	if (EQ(Part->DevPath, Ptr->DevPath) ||
	    EQ(Part->DevName, Ptr->DevName))
	    return -1;
    }

    if (End)
	End->Next = Part;
    else
	*PartInfoPtr = Part;

    return 0;
}

/*
 * Get the string name for the Usage
 */
static char *PartInfoUsage(Part)
     PartInfo_t		       *Part;
{
    switch (Part->Usage) {
    case PIU_UNUSED:	return "UNUSED";
    case PIU_FILESYS:	return "FILESYSTEM";
    case PIU_SWAP:	return "SWAP";
    case PIU_UNKNOWN:	return "UNKNOWN";
    }

    return (char *) NULL;
}

/*
 * Sanitize Part to make sure all values are filled in as required.
 */
static int PartInfoSanitize(Part)
     PartInfo_t		       *Part;
{
    char		       *cp;

    if (!Part->BaseName && Part->DevPath) {
#if	defined(_PATH_DEV_DSK)
	if (eqn(Part->DevPath, _PATH_DEV_DSK, strlen(_PATH_DEV_DSK)))
	    Part->BaseName = Part->DevPath + strlen(_PATH_DEV_DSK) + 1;
#endif	/* _PATH_DEV_DSK */
	if (!Part->BaseName && (cp = strrchr(Part->DevPath, '/')))
	    Part->BaseName = cp + 1;
    }

    if (!Part->SecSize)
	Part->SecSize = 512;

    if (!Part->Size)
	Part->Size = (Large_t) nsect_to_bytes(Part->NumSect, Part->SecSize);

    if (!Part->EndSect && Part->StartSect && Part->NumSect)
	Part->EndSect = Part->StartSect + Part->NumSect - 1;

    Part->UsageStatus = PartInfoUsage(Part);

    if (!Part->Type && Part->Usage == PIU_SWAP)
	if (cp = PartInfoUsage(Part))
	    Part->Type = strdup(cp);	/* Need to be able to free */

    return 0;
}

/*
 * Interface function between mcSysInfo() and underlying probes.
 */
extern int PartInfoMCSI(Query)
     MCSIquery_t	       *Query;
{
    static PartInfo_t 	       *Root;
    PartInfo_t	 	       *Part;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    switch (Query->Op) {
    case MCSIOP_CREATE:
	if (!Root) {
	    SImsg(SIM_DBG, "BUILDING Partition Tree ...");
	    if (BuildPartInfo(&Root, Query->SearchExp) != 0)
		return -1;
	}

	if (!Root) {
	    SImsg(SIM_DBG, "No partition information was found.");
	    errno = ENOENT;
	    return -1;
	}

	for (Part = Root; Part; Part = Part->Next)
	    PartInfoSanitize(Part);

	Query->Out = (Opaque_t) Root;
	Query->OutSize = sizeof(PartInfo_t *);
	break;
    case  MCSIOP_DESTROY:
	if (Query->Out && Query->OutSize)
	    return PartInfoDestroy((PartInfo_t *) Query->Out);
	break;
    }

    return 0;
}
