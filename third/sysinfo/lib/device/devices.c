/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Device routines
 */

#include <stdio.h>
#include "defs.h"

static void DevTreeVerify();

static DevInfo_t	       *OrphanList = NULL;
#if	defined(IGNORE_SERIAL_LIST)
static char		       *IgnoreSerialList[] = IGNORE_SERIAL_LIST;
#else
static char		       *IgnoreSerialList[] = { NULL };
#endif	/* IGNORE_SERIAL_LIST */

/*
 * For AssignDevUnits
 */
struct DevUnits {
    char		       *Name;
    int				LastUnit;
    struct DevUnits	       *Next;
};
static struct DevUnits	       *DevUnits;

/*
 * Add a device description to a device.
 */
extern int AddDevDesc(DevInfo, Desc, Label, Action)
    DevInfo_t		       *DevInfo;
    char		       *Desc;
    char		       *Label;
    int				Action;
{
    return AddDesc(&(DevInfo->DescList), Label, Desc, Action);
}

/*
 * Add Resolution to the list of Resolutions in DevInfo.
 * Resolutions is assumed to already be a permament buffer.
 */
extern void DevAddRes(DevInfo, Resolution)
     DevInfo_t		       *DevInfo;
     char		       *Resolution;
{
    register char	      **cpp;
    register int		i;
    Monitor_t		       *Mon = NULL;

    if (!DevInfo || !Resolution)
	return;

    if (!DevInfo->DevSpec) {
	Mon = (Monitor_t *) xcalloc(1, sizeof(Monitor_t *));
	DevInfo->DevSpec = (void *) Mon;
    } else
	Mon = (Monitor_t *) DevInfo->DevSpec;

    if (!Mon->Resolutions) {
	Mon->Resolutions = (char **) xcalloc(2, sizeof(char *));
	Mon->Resolutions[0] = Resolution;
	return;
    }

    /*
     * Get count of current number of resolutions in list
     * while checking to make sure there is no matching entry.
     */
    for (i = 0; Mon->Resolutions[i]; ++i)
	/* Do case sensitive match check */
	if (eq(Mon->Resolutions[i], Resolution))
	    return;

    /*
     * Re alloc buffer and add File
     */
    Mon->Resolutions = (char **) xrealloc(Mon->Resolutions, 
					  (i+2)*sizeof(char *));
    Mon->Resolutions[i] = strdup(Resolution);
    Mon->Resolutions[i+1] = NULL;
}

/*
 * Add File to the list of Files in DevInfo.
 * File is assumed to already be a permament buffer.
 */
extern void DevAddFile(DevInfo, File)
     DevInfo_t		       *DevInfo;
     char		       *File;
{
    register char	      **cpp;
    register int		i;

    if (!DevInfo || !File)
	return;

    if (!DevInfo->Files) {
	DevInfo->Files = (char **) xcalloc(2, sizeof(char *));
	DevInfo->Files[0] = File;
	return;
    }

    /*
     * Get count of current number of files in list
     * while checking to make sure there is no matching entry.
     */
    for (i = 0, cpp = DevInfo->Files; cpp && cpp[i]; ++i)
	/* Do case sensitive match check */
	if (strcmp(cpp[i], File) == 0)
	    return;

    /*
     * Re alloc buffer and add File
     */
    DevInfo->Files = (char **) xrealloc(DevInfo->Files, (i+1)*sizeof(char *));
    DevInfo->Files[i] = File;
    DevInfo->Files[i+1] = NULL;
}

/*
 * Interface function between mcSysInfo() and underlying device probes.
 */
extern int DeviceInfoMCSI(Query)
     MCSIquery_t	       *Query;
{
#if	defined(HAVE_DEVICE_SUPPORT)
    static DevInfo_t 	       *Root;

    if (!Query) {
	errno = ENXIO;
	return -1;
    }

    switch (Query->Op) {
    case MCSIOP_CREATE:
	if (!Root) {
	    SImsg(SIM_DBG, "BUILDING Device Tree ...");
	    if (BuildDevices(&Root, Query->SearchExp) != 0)
		return -1;
	}

	if (!Root) {
	    SImsg(SIM_DBG, "No devices were found.");
	    errno = ENOENT;
	    return -1;
	}

	DevTreeVerify(&Root);

	Query->Out = (Opaque_t) Root;
	Query->OutSize = sizeof(DevInfo_t *);
	break;
    case MCSIOP_DESTROY:
	if (Query->Out && Query->OutSize)
	    return DestroyDeviceTree((DevInfo_t *) Query->Out);
	break; 
   }

#else	/* HAVE_DEVICE_SUPPORT */
    SImsg(SIM_DBG, 
"Support for `Device' class information is not available on this platform.");
#endif	/* HAVE_DEVICE_SUPPORT */

    return 0;
}

/*
 * --RECURSE--
 * Create a new DevInfo_t and optionally copy an old DevInfo_t.
 */
extern DevInfo_t *NewDevInfo(Old)
    DevInfo_t 		       *Old;
{
    register DevInfo_t 	       *SlavePtr, *Slave;
    DevInfo_t 		       *New = NULL;

    New = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));

    /* Set int's to -1 */
    New->Type = New->Unit = New->Addr = New->Prio = 
	New->Vec = -1;

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(DevInfo_t));

    New->Next = NULL;

    /* Copy contents of pointers */
    if (Old->Name)	New->Name = strdup(Old->Name);
    if (Old->Vendor)	New->Vendor = strdup(Old->Vendor);
    if (Old->Serial)	New->Serial = strdup(Old->Serial);
    if (Old->Revision)	New->Revision = strdup(Old->Revision);
    if (Old->Model)	New->Model = strdup(Old->Model);
    if (Old->ModelDesc) New->ModelDesc = strdup(Old->ModelDesc);
    if (Old->DescList) 	New->DescList = Old->DescList;
    if (Old->Ident)	New->Ident = IdentCreate(Old->Ident);

    /* Copy Slave info */
    for (Slave = Old->Slaves; Slave; Slave = Slave->Next) {
	/* Find last slave */
	for (SlavePtr = New->Slaves; SlavePtr && SlavePtr->Next; 
	     SlavePtr = SlavePtr->Next);
	/* Copy Old slave to last new slave device */
	SlavePtr = NewDevInfo(Slave);
    }

    return(New);
}

/*
 * Create a new DiskPart_t and optionally copy an old DiskPart_t.
 */
extern DiskPart_t *NewDiskPart(Old)
    DiskPart_t 		       *Old;
{
    DiskPart_t 		       *New = NULL;

    New = (DiskPart_t *) xcalloc(1, sizeof(DiskPart_t));

    if (Old) {
	/* Copy old values to new */
	if (Old->PartInfo)	New->PartInfo = PartInfoCreate(Old->PartInfo);
    }

    return(New);
}

/*
 * Create a new DiskDrive_t and optionally copy an old DiskDrive_t.
 */
extern DiskDrive_t *NewDiskDrive(Old)
    DiskDrive_t 	       *Old;
{
    register DiskPart_t        *dp, *pdp;
    DiskDrive_t 	       *New = NULL;

    New = (DiskDrive_t *) xcalloc(1, sizeof(DiskDrive_t));

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(DiskDrive_t));

    /* Copy contents of pointers */
    if (Old->Label)	New->Label = strdup(Old->Label);
    if (Old->Ctlr) 	New->Ctlr = NewDevInfo(Old->Ctlr);

    /* Copy partition info */
    for (dp = Old->DiskPart; dp; dp = dp->Next) {
	/* Find last DiskPart_t */
	for (pdp = New->DiskPart; pdp && pdp->Next; pdp = pdp->Next);
	/* Copy old DiskPart_t to last New DiskPart_t */
	pdp = NewDiskPart(dp);
    }

    return(New);
}

/*
 * Create a new DiskDriveData_t and optionally copy an old DiskDriveData_t.
 */
extern DiskDriveData_t *NewDiskDriveData(Old)
    DiskDriveData_t 	       *Old;
{
    DiskDriveData_t 	       *New = NULL;

    New = (DiskDriveData_t *) xcalloc(1, sizeof(DiskDriveData_t));

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(DiskDriveData_t));

    return(New);
}

/*
 * Create a new FrameBuffer_t and optionally copy an old FrameBuffer_t.
 */
extern FrameBuffer_t *NewFrameBuffer(Old)
    FrameBuffer_t 	       *Old;
{
    FrameBuffer_t 	       *New = NULL;

    New = (FrameBuffer_t *) xcalloc(1, sizeof(FrameBuffer_t));

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(FrameBuffer_t));

    return(New);
}

/*
 * Create a new Monitor_t and optionally copy an old Monitor_t.
 */
extern Monitor_t *NewMonitor(Old)
    Monitor_t 	       *Old;
{
    Monitor_t 	       *New = NULL;

    New = (Monitor_t *) xcalloc(1, sizeof(Monitor_t));

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(Monitor_t));

    return(New);
}

/*
 * Create a new NetIF_t and optionally copy an old NetIF_t.
 */
extern NetIF_t *NewNetif(Old)
    NetIF_t 		       *Old;
{
    NetIF_t 		       *New = NULL;

    New = (NetIF_t *) xcalloc(1, sizeof(NetIF_t));

    if (!Old)
	return(New);

    /* Copy */
    New->HostAddr = strdup(Old->HostAddr);
    New->HostName = strdup(Old->HostName);
    New->MACaddr = strdup(Old->MACaddr);
    New->MACname = strdup(Old->MACname);
    New->NetAddr = strdup(Old->NetAddr);
    New->NetName = strdup(Old->NetName);

    return(New);
}

/*
 * Add a reason (What) to why Find matched
 */
static void DevFindAddMatch(Find, What)
     DevFind_t		       *Find;
     char		       *What;
{
    if (!Find || !What)
	return;

    if ((strlen(Find->Reason) + strlen(What) + 2) >= sizeof(Find->Reason)) {
	SImsg(SIM_DBG, "DevFindAddMatch(..., %s): Find->Reason is full.", 
	      What);
	return;
    }
	      
    if (Find->Reason && Find->Reason[0])
	(void) strcat(Find->Reason, ",");
    (void) strcat(Find->Reason, What);
}

/*
 * Does NodeName match Tree?
 */
static int DevFindMatchName(Find)
     DevFind_t		       *Find;
{
    register char	      **cpp;
    char		       *NodeName;
    DevInfo_t		       *Tree;

    if (!Find || !Find->Tree)
	return(FALSE);

    Tree = Find->Tree;
    NodeName = Find->NodeName;

    if (NodeName && EQ(NodeName, Tree->Name)) {
	DevFindAddMatch(Find, "NodeName");
	return(TRUE);
    }

    if (NodeName && Tree->Aliases)
	for (cpp = Tree->Aliases; cpp && *cpp; ++cpp) {
	    /* Skip alias if it's the same as the canonical name */
	    if (EQ(Tree->Name, *cpp))
		continue;
	    if (EQ(*cpp, NodeName)) {
		DevFindAddMatch(Find, "NodeNameAlias");
		return(TRUE);
	    }
	}

    return(FALSE);
}

/*
 * Does Serial match Tree?
 */
static int DevFindMatchSerial(Find)
     DevFind_t		       *Find;
{
    register char	      **cpp;

    if (!Find || !Find->Tree)
	return(FALSE);

    /*
     * Is Driver in the list of driver or serial names which we should ignore
     * Serial?
     */
    if (Find->Tree->Driver)
	for (cpp = IgnoreSerialList; cpp && *cpp; ++cpp)
	    if (EQ(Find->Tree->Driver, *cpp))
		return(FALSE);
    if (Find->Tree->Serial)
	for (cpp = IgnoreSerialList; cpp && *cpp; ++cpp)
	    if (EQ(Find->Tree->Serial, *cpp))
		return(FALSE);

    if (Find->Serial && EQ(Find->Serial, Find->Tree->Serial)) {
	DevFindAddMatch(Find, "Serial");
	return(TRUE);
    } else
	return(FALSE);
}

/*
 * Does Ident match Tree?
 */
static int DevFindMatchIdent(Find)
     DevFind_t		       *Find;
{
    register char	      **cpp;

    if (!Find || !Find->Tree)
	return(FALSE);

    if (Find->Ident && IdentMatch(Find->Ident, Find->Tree->Ident)) {
	DevFindAddMatch(Find, "Ident");
	return(TRUE);
    } else
	return(FALSE);
}

/*
 * Does NodeID match Tree?
 */
static int DevFindMatchID(Find)
     DevFind_t		       *Find;
{
    if (!Find || !Find->Tree)
	return(FALSE);

    if (Find->NodeID && Find->NodeID != -1 && 
	Find->NodeID == Find->Tree->NodeID) {
	DevFindAddMatch(Find, "NodeID");
	return(TRUE);
    } else
	return(FALSE);
}

/*
 * Does ClassType, DevType, and Unit match Tree?
 */
static int DevFindMatchType(Find)
     DevFind_t		       *Find;
{
    if (!Find || !Find->Tree)
	return FALSE;

    if (FLAGS_ON(Find->Flags, DFF_NOTYPE))
	return FALSE;

    if (Find->Unit >= 0 && Find->Unit == Find->Tree->Unit &&
	(Find->DevType < 0 || Find->DevType == Find->Tree->Type) &&
	(Find->ClassType < 0 || Find->ClassType == Find->Tree->ClassType)) {
	SImsg(SIM_DBG, 
	      "DevFindMatchType: Dev=<%s> MATCHED Unit %d=%d Type %d=%d Class %d=%d",
	      PRTS(Find->NodeName),
	      Find->Unit, Find->Tree->Unit,
	      Find->DevType, Find->Tree->Type,
	      Find->ClassType, Find->Tree->ClassType);
	DevFindAddMatch(Find, "Type");
	return(TRUE);
    } else
	return(FALSE);
}

/*
 * --RECURSE--
 * Find a device in the device tree.  Find->Tree is the tree entry to search.
 * Find->Flags determines whether all conditions are OR'ed or AND'ed together. 
 * This function recursively calls itself.
 */
extern DevInfo_t *DevFind(Find)
     DevFind_t		       *Find;
{
    DevInfo_t 		       *Ptr;
    DevInfo_t 		       *Tree;

    if (!Find || !Find->Tree)
	return((DevInfo_t *) NULL);

    Tree = Find->Tree;

    if (Find->Expr == DFE_OR) {
	if (DevFindMatchName(Find) ||
	    DevFindMatchID(Find) ||
	    DevFindMatchIdent(Find) ||
	    DevFindMatchSerial(Find) ||
	    DevFindMatchType(Find))
	    return(Tree);
    } else if (Find->Expr == DFE_AND) {
	if (DevFindMatchName(Find) &&
	    DevFindMatchID(Find) &&
	    DevFindMatchIdent(Find) &&
	    DevFindMatchSerial(Find) &&
	    DevFindMatchType(Find))
	    return(Tree);
    } else {
	SImsg(SIM_DBG, "DevFind(): Expr %d unknown.", Find->Expr);
	return((DevInfo_t *) NULL);
    }

    if (Tree->Slaves) {
	Find->Tree = Tree->Slaves;
	if (Ptr = DevFind(Find))
	    return(Ptr);
    }

    if (Tree->Next) {
	Find->Tree = Tree->Next;
	if (Ptr = DevFind(Find))
	    return(Ptr);
    }

    return((DevInfo_t *) NULL);
}

/*
 * --RECURSE--
 * Search for device by address (DevInfo) and return that
 * device's parent.
 */
extern DevInfo_t *FindDeviceParent(DevInfo, TreePtr, Parent)
    DevInfo_t		       *DevInfo;
    DevInfo_t 		       *TreePtr;
    DevInfo_t		       *Parent;
{
    DevInfo_t 		       *Ptr;

    if (DevInfo && DevInfo == TreePtr)
	return(Parent);

    if (TreePtr->Slaves)
	if (Ptr = FindDeviceParent(DevInfo, TreePtr->Slaves, TreePtr))
	    return(Ptr);

    if (TreePtr->Next)
	if (Ptr = FindDeviceParent(DevInfo, TreePtr->Next, TreePtr))
	    return(Ptr);

    return((DevInfo_t *) NULL);
}

/*
 * Check to see if device's dev1 and dev2 are consistant.
 * If there is a discrepancy between the two due to one
 * device not having it's value set, then set it to the
 * other device's value.  Basically this "fills in the blanks".
 */
static void CheckDevice(Dev1, Dev2)
     DevInfo_t 		       *Dev1;
     DevInfo_t 		       *Dev2;
{
#define CHECK(a,b) \
    if (a != b) { \
	if (a) \
	    b = a; \
	else if (b) \
	    a = b; \
    }

    CHECK(Dev1->Type, 		Dev2->Type);
    CHECK(Dev1->ClassType,	Dev2->ClassType);
    CHECK(Dev1->Aliases,	Dev2->Aliases);
    CHECK(Dev1->Vendor,		Dev2->Vendor);
    CHECK(Dev1->Model, 		Dev2->Model);
    CHECK(Dev1->ModelDesc, 	Dev2->ModelDesc);
    CHECK(Dev1->Ident,		Dev2->Ident);
    CHECK(Dev1->Serial,		Dev2->Serial);
    CHECK(Dev1->Revision,	Dev2->Revision);
    CHECK(Dev1->DescList, 	Dev2->DescList);
    CHECK(Dev1->Unit, 		Dev2->Unit);
    CHECK(Dev1->Addr, 		Dev2->Addr);
    CHECK(Dev1->Prio, 		Dev2->Prio);
    CHECK(Dev1->Vec, 		Dev2->Vec);
    CHECK(Dev1->DevSpec, 	Dev2->DevSpec);
    CHECK(Dev1->Master, 	Dev2->Master);
    CHECK(Dev1->NodeID, 	Dev2->NodeID);

#undef CHECK
}

/*
 * Is Name in Argv?
 */
extern int SearchCheck(Name, Argv)
    char		       *Name;
    char		      **Argv;
{
    register char	      **cpp;

    for (cpp = Argv; cpp && *cpp; ++cpp) {
	if (EQ(Name, *cpp))
	    return(1);
    }

    return(0);
}

/*
 * Remove DevInfo from the Parent's list of children.
 */
extern int DeviceRemoveLinks(DevInfo, Parent)
     DevInfo_t		       *DevInfo;
     DevInfo_t		       *Parent;
{
    register DevInfo_t	       *Ptr;
    register DevInfo_t	       *Last = NULL;

    if (!DevInfo || !Parent)
	return(-1);

    SImsg(SIM_DBG, "RemoveLinks   DevName=<%s> Parent=<%s>", 
	  PRTS(DevInfo->Name), PRTS(Parent->Name));

    if (Parent->Slaves) {
	for (Ptr = Parent->Slaves; Ptr && Ptr->Next; ) {
	    if (Ptr == DevInfo) {
		if (Last) 
		    /* Delete from previous slave */
		    Last->Next = DevInfo->Next;
		else
		    /* There are no other slaves */
		    Parent->Slaves = NULL;
		DevInfo->Next = NULL;
	    }
	    Last = Ptr;
	    Ptr = Ptr->Next;
	}
    } else
	Parent->Slaves = DevInfo->Next;

    return(0);
}

/*
 * Add DevInfo to a linked list of orphaned devices for later
 * re-attachment.
 */
static void AddOrphanList(DevInfo)
     DevInfo_t		      *DevInfo;
{
    register DevInfo_t	       *Ptr;
    static DevInfo_t	       *Last;

    SImsg(SIM_DBG, "AddOrphanList DevName=<%s>", PRTS(DevInfo->Name));

    if (Last) {
	Last->Next = DevInfo;
	Last = DevInfo;
    } else {
	Last = OrphanList = DevInfo;
    }
}

/*
 * Check to see if the child (DevInfo) matches the Parent (Master).
 * If not, detach and add to orphan list for later re-attachment
 * with the right Parent.
 */
static int DevTreeCheck(Parent, DevInfo)
     DevInfo_t		      *Parent;
     DevInfo_t		      *DevInfo;
{
    if (!Parent || !DevInfo)
	return(-1);

    if (DevInfo->Master && DevInfo->MasterName && 
	(DevInfo->Master != Parent || 
	 !EQ(DevInfo->MasterName, Parent->Name))) {
	DeviceRemoveLinks(DevInfo, Parent);
	AddOrphanList(DevInfo);
	return(0);
    } else
	return(1);
}

/*
 * -RECURSE-
 * Scan the device tree.
 */
static void DevTreeScan(Parent, DevInfo)
     DevInfo_t		      *Parent;
     DevInfo_t		      *DevInfo;
{
    register DevInfo_t	       *Ptr;

    (void) DevTreeCheck(Parent, DevInfo);

    for (Ptr = DevInfo->Slaves; Ptr; Ptr = Ptr->Next) {
	if (DevTreeCheck(DevInfo, Ptr) == 1)
	    DevTreeScan(DevInfo, Ptr);
    }

    if (DevInfo->Next)
	DevTreeScan(Parent, DevInfo->Next);
}

/*
 * Check the entire device tree (recursively) to see if the
 * Master identified by the device and the Master's list of
 * child devices agree.  Believe the master indicated by the
 * device over the Master's list.
 */
static void DevTreeVerify(TreePtr)
     DevInfo_t		     **TreePtr;
{
    register DevInfo_t	       *Ptr;

    OrphanList = NULL;

    /* Find and detach orphans */
    DevTreeScan(NULL, *TreePtr);

    /* Re-attach each orphan */
    for (Ptr = OrphanList; Ptr; Ptr = Ptr->Next)
	AddDevice(Ptr, TreePtr, NULL);
}

/*
 * --RECURSE--
 * Add a device to a device list.
 */
extern int AddDevice(DevInfo, TreePtr, SearchNames)
    DevInfo_t 		       *DevInfo;
    DevInfo_t 		      **TreePtr;
    char		      **SearchNames;
{
    register DevInfo_t 	       *Master = NULL, *mp = NULL;
    static DevFind_t		Find;
    DevInfo_t		       *Found;
    int				Okay;

    if (!DevInfo || !TreePtr) {
	SImsg(SIM_GERR, "Invalid parameter passed to AddDevice()");
	return(-1);
    }

    /*
     * Only add the device if it's in our search list
     */
    if (SearchNames) {
	Okay = 0;
	if (DevInfo->Name && SearchCheck(DevInfo->Name, SearchNames))
	    Okay = 1;
	if (DevInfo->AltName && SearchCheck(DevInfo->AltName, SearchNames))
	    Okay = 1;
	if (!Okay) {
	    SImsg(SIM_DBG, "AddDevice: Device <%s> not in search list.",
		  DevInfo->Name);
	    return(0);	/* Lie */
	}
    }

    /*
     * Make sure device hasn't already been added
     */
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *TreePtr;
    Find.NodeName = DevInfo->Name;
    Find.NodeID = DevInfo->NodeID;
#if	defined(NO_DUP_DEV_BY_TYPE)
    Find.Flags |= DFF_NOTYPE;
#endif
#if	!defined(DONT_FIND_ON_SERIAL)
    Find.Serial = DevInfo->Serial;
#endif	/* DONT_FIND_ON_SERIAL */
    if (Found = DevFind(&Find)) {
	SImsg(SIM_DBG, 
    "AddDevice: <%s> device with the same `%s' already exists; Master = <%s>",
	      DevInfo->Name, Find.Reason,
	      (DevInfo->Master && DevInfo->Master->Name) 
	      ? DevInfo->Master->Name : "?");

	/*
	 * Assume that the new device has more/better info than the
	 * existing device.
	 */
	if (DevInfo->Vendor)	    Found->Vendor = DevInfo->Vendor;
	if (DevInfo->Model)	    Found->Model = DevInfo->Model;
	if (DevInfo->ModelDesc)	    Found->ModelDesc = DevInfo->ModelDesc;
	if (DevInfo->Ident)	    Found->Ident = DevInfo->Ident;
	if (DevInfo->Serial)	    Found->Serial = DevInfo->Serial;
	if (DevInfo->Revision)	    Found->Revision = DevInfo->Revision;
	if (DevInfo->DescList)	    Found->DescList = DevInfo->DescList;

	return(-1);
    }

    if (DevInfo->Name)
	DevInfo->Name = strdup(DevInfo->Name);

    /*
     * If the device has a master, find the master device.
     * If one doesn't exist in the tree, then add it by recursively
     * calling this function.
     */
    if (DevInfo->Master && !SearchNames) {
	Master = NULL;
	if (*TreePtr) {
	    (void) memset(&Find, 0, sizeof(Find));
	    Find.Tree = *TreePtr;
	    Find.NodeName = DevInfo->Master->Name;
	    Find.NodeID = DevInfo->Master->NodeID;
	    Master = DevFind(&Find);
	    if (Master && EQ(Master->Name, DevInfo->Master->Name))
		/* Check and fix any differences in info between master's */
		CheckDevice(Master, DevInfo->Master);
	}

	if (!Master) {
	    if (AddDevice(DevInfo->Master, TreePtr, SearchNames) != 0) {
		SImsg(SIM_GERR, "Add master <%s> to device tree failed.", 
		      DevInfo->Name);
		return(-1);
	    }
	    Master = DevInfo->Master;
	}
    } else {
	if (!*TreePtr)
	    *TreePtr = NewDevInfo((DevInfo_t *)NULL);
	/* 
	 * The device doesn't have a master, so make it a child of the
	 * top most level.
	 */
	Master = *TreePtr;
    }

    if (Master->Name)
	Master->Name = strdup(Master->Name);

    if (Master->Slaves) {
	/* Add to existing list of slaves */
	for (mp = Master->Slaves; mp && mp->Next; mp = mp->Next);
	mp->Next = DevInfo;
    } else
	/* Create first slave */
	Master->Slaves = DevInfo;

    return(0);
}

/*
 * Create a device entry.
 * All device Probes should call DeviceCreate() to create and return
 * the base DevInfo from which they can modify as needed.
 */
extern DevInfo_t *DeviceCreate(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    char 		       *Name;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
    register char	      **cpp;
    register int		Count;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    Name = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    /*
     * DT_GENERIC devices MUST be marked alive to proceed
     */
    if (DevDefine && DevDefine->Type == DT_GENERIC && 
	!(FLAGS_ON(DevData->Flags, DD_IS_ALIVE) ||
	  FLAGS_ON(DevDefine->Flags, DDT_ZOMBIE) ||
	  FLAGS_ON(DevData->Flags, DD_MAYBE_ALIVE)))
	return((DevInfo_t *) NULL);

    /*
     * Select new DevInfo.  Use UseDevInfo if passed in, or create a new one.
     */
    if (ProbeData->UseDevInfo)
	DevInfo = ProbeData->UseDevInfo;
    else
	DevInfo = NewDevInfo((DevInfo_t *) NULL);

    /*
     * Set our DevInfo (Device) Type
     */
    if (DevInfo->Type <= 0) {
	if (DevData && DevData->DevType)
	    DevInfo->Type = DevData->DevType;
	else if (DevDefine && DevDefine->Type)
	    DevInfo->Type = DevDefine->Type;
    }

    /*
     * Decide on/create our device name (with unit number) i.e. 'vx0'
     */
    if (!DevInfo->Name) {
	if (Name)
	    DevInfo->Name = strdup(Name);
	else if (DevDefine)
	    DevInfo->Name = MkDevName(DevData->DevName, DevData->DevUnit,
				      DevInfo->Type, DevDefine->Flags);
	else if (DevData)
	    DevInfo->Name = MkDevName(DevData->DevName, DevData->DevUnit,
				      DevInfo->Type, 0);
	else 
	    DevInfo->Name = DevData->DevName;
    }

    /*
     * Set what we know from ProbeData
     */
    if (!DevInfo->Master)
	DevInfo->Master = ProbeData->CtlrDevInfo;
    if (ProbeData->DevFile)
	DevAddFile(DevInfo, strdup(ProbeData->DevFile));

    /*
     * Set what we know from DevDefine
     */
    if (DevDefine) {
	DevInfo->Model = DevDefine->Model;
	if (!DevInfo->Vendor)
	    DevInfo->Vendor = DevDefine->Vendor;
	if (DevDefine->Desc)
	    DevInfo->ModelDesc = DevDefine->Desc;
	if (!DevInfo->ClassType)
	    DevInfo->ClassType = DevDefine->ClassType;
    }

    /*
     * Set what we want from DevData
     */
    if (DevData) {
	if (!DevInfo->Master)
	    DevInfo->Master = DevData->CtlrDevInfo;
	if (!DevInfo->Driver)
	    DevInfo->Driver = DevData->DevName;
	if (DevInfo->Unit < 0)
	    DevInfo->Unit = DevData->DevUnit;
	if (DevInfo->NodeID == 0 || DevInfo->NodeID == -1 || 
	    DevInfo->NodeID == -2)
	    DevInfo->NodeID = DevData->NodeID;
	if (!DevInfo->Master)
	    DevInfo->Master = MkMasterFromDevData(DevData);
    }

    /*
     * If we have OS provided info, use it for whatever was not already set
     */
    if (DevData && DevData->OSDevInfo) {
	if (!DevInfo->Vendor)
	    DevInfo->Vendor = DevData->OSDevInfo->Vendor;
	if (!DevInfo->Model)
	    DevInfo->Model = DevData->OSDevInfo->Model;
	if (!DevInfo->ModelDesc)
	    DevInfo->ModelDesc = DevData->OSDevInfo->ModelDesc;
	if (!DevInfo->Ident)
	    DevInfo->Ident = DevData->OSDevInfo->Ident;
	if (!DevInfo->Serial)
	    DevInfo->Serial = DevData->OSDevInfo->Serial;
	if (!DevInfo->Revision)
	    DevInfo->Revision = DevData->OSDevInfo->Revision;
	if (!DevInfo->DescList)
	    DevInfo->DescList = DevData->OSDevInfo->DescList;
	if (DevInfo->ClassType <= 0)
	    DevInfo->ClassType = DevData->OSDevInfo->ClassType;
	if (!DevInfo->Master)
	    DevInfo->Master = DevData->OSDevInfo->Master;
    }

    if (ProbeData->AliasNames) {
	/*
	 * Copy all aliases that are not the same as the DevInfo Name
	 */
	for (Count = 0, cpp = ProbeData->AliasNames; cpp && *cpp; ++cpp)
	    if (!EQ(*cpp, DevInfo->Name))
		++Count;
	if (Count) {
	    DevInfo->Aliases = (char **) xcalloc(Count+1, sizeof(char *));
	    for (Count = 0, cpp = ProbeData->AliasNames; cpp && *cpp; ++cpp)
		if (!EQ(*cpp, DevInfo->Name))
		    DevInfo->Aliases[Count++] = *cpp;
	}
    }

    return(DevInfo);
}

/*
 * Create a device type based on DevInfo provided from
 * some type of OS source.
 */
extern DevInfo_t *ProbeOSDevInfo(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    DevType_t		       *DevType;
    static DevDefine_t		DevDef;
    char		       *DevName;
    DevData_t		       *DevData;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevData = ProbeData->DevData;
    if (!(DevInfo = DevData->OSDevInfo))
	return((DevInfo_t *) NULL);

    if (DevInfo->Name)
	/* Use the provided DevName if set */
	DevName = DevInfo->Name;
    else
	DevName = MkDevName(
		    (DevInfo->Name) ? DevInfo->Name : DevData->DevName, 
		    DevData->DevUnit, -1, 0);

    /*
     * See if we can find a Device Type and if so, call it's 
     * Probe function.  Otherwise, create an entry ourselves.
     */
    if ((DevType = TypeGetByType(DevInfo->Type)) && DevType->Probe) {
	DevDef.Name 		= DevInfo->Name;
	DevDef.Model 		= DevInfo->Model;
	DevDef.Type 		= DevInfo->Type;
	if (!ProbeData->DevName)
	    ProbeData->DevName	= DevName;
	ProbeData->DevDefine	= &DevDef;
	if (!ProbeData->UseDevInfo)
	    ProbeData->UseDevInfo = DevInfo;
	return((*DevType->Probe)(ProbeData));
    } else {
	if (!DevInfo->Name)
	    DevInfo->Name = DevName;
	if (!DevInfo->Master)
	    DevInfo->Master = MkMasterFromDevData(DevData);
	if (DevData) {
	    if (DevInfo->Unit < 0)
		DevInfo->Unit = DevData->DevUnit;
	    if (DevInfo->NodeID == 0 || DevInfo->NodeID == -1 ||
		DevInfo->NodeID == -2)
		DevInfo->NodeID = DevData->NodeID;
	    if (!DevInfo->Driver)
		DevInfo->Driver = DevData->DevName;
	}
	return(ProbeData->RetDevInfo = DevInfo);
    }
}

/*
 * Set (add) alias Name to a list pointed to by LocPtr.
 */
static void SetAlias(LocPtr, Name)
     char		     ***LocPtr;
     char		       *Name;
{
    char		      **List;
    register char	      **cpp;
    int				Count;

    if (!LocPtr || !Name)
	return;

    List = *LocPtr;

    if (List) {
	for (cpp = List, Count = 0; cpp && *cpp; ++cpp) {
	    if (EQ(List[Count], Name))
		/* We already have it, so skip adding it */
		return;
	    ++Count;
	}
	List = (char **) xrealloc(List, (Count + 2) * sizeof(char *));
	List[Count] = Name;
	List[Count+1] = NULL;
    } else {
	List = (char **) xcalloc(2, sizeof(char *));
	List[0] = Name;
    }

    *LocPtr = List;
}

/*
 * Search for and call an appropriate probe function for this 
 * device
 */
extern DevInfo_t *ProbeDevice(DevData, TreePtr, Search, OfferDevDef)
    DevData_t 		       *DevData;
    DevInfo_t 		      **TreePtr;
    char		      **Search;
    DevDefine_t		       *OfferDevDef;
{
    register DevDefine_t       *DevDefine = NULL;
    int				DevTypeNum = 0;
    DevType_t		       *DevType = NULL;
    register char 	       *Name = NULL;
    register char	      **cpp;
    static DevFind_t		Find;
    static ProbeData_t		ProbeData;
    DevInfo_t 		       *ProbeUnknown();
    DevInfo_t		     *(*ProbeFunc)() = NULL;
    char		       *DevName = NULL;

    /* Set DevName (no unit) e.g. 'vx' */
    DevName = DevData->DevName;

    /* Get the Device Definetion to use (if any) */
    if (OfferDevDef)
	DevDefine = OfferDevDef;
    else if (DevData->DevName)
	DevDefine = DevDefGet(DevData->DevName, 0, 0);

    /*
     * See if we can find DevType and lookup an appropriate probe function
     */
    if (DevData->DevType)
	DevTypeNum = DevData->DevType;
    else if (!DevDefine && DevData->OSDevInfo && DevData->OSDevInfo->Type)
	DevTypeNum = DevData->OSDevInfo->Type;
    if (DevTypeNum && (DevType = TypeGetByType(DevTypeNum)))
	ProbeFunc = DevType->Probe;

    /*
     * Try to assign a Probe Function if we didn't manage to set one 
     * from DevType above.
     */
    if (!ProbeFunc && DevDefine) {
	if ((*DevDefine->Probe) == NULL)
	    ProbeFunc = DeviceCreate;
	else
	    ProbeFunc = DevDefine->Probe;
    }

    (void) memset(&ProbeData, CNULL, sizeof(ProbeData));
    ProbeData.DevName = DevName;
    ProbeData.DevData = DevData;
    ProbeData.DevDefine = DevDefine;

    if (ProbeFunc) {
	if (DevDefine) {
	    /*
	     * Add a list of possible device aliases from the DevDefine entry
	     */
	    for (cpp = DevDefine->Aliases; cpp && *cpp; ++cpp) {
		Name = MkDevName(*cpp, DevData->DevUnit, DevDefine->Type, 
				 DevDefine->Flags | DDT_ISALIAS);
		SetAlias(&(ProbeData.AliasNames), Name);
	    }

	    /*
	     * Use the canonical device name whenever possible
	     */
	    if (DevDefine->Name) {
		/* Set the current (passed) name as an alias */
		Name = MkDevName(DevName, DevData->DevUnit,
				 DevDefine->Type, DevDefine->Flags);
		SetAlias(&(ProbeData.AliasNames), Name);
		DevName = DevDefine->Name;
	    }

	    Name = MkDevName(DevName, DevData->DevUnit,
			     DevDefine->Type, DevDefine->Flags);
	} else if (DevType) {
	    Name = MkDevName(DevName, DevData->DevUnit,
			     DevType->Type, 0);
	} else {
	    Name = MkDevName(DevName, DevData->DevUnit, 0, 0);
	}

	(void) memset(&Find, 0, sizeof(Find));
	Find.Tree = *TreePtr;
	Find.NodeName = Name;
	if (DevFind(&Find)) {
	    SImsg(SIM_DBG, "ProbeDevice: <%s> already exists.", Name);
	    return((DevInfo_t *) NULL);
	}
	ProbeData.DevName = Name;
	return((*ProbeFunc)(&ProbeData));
    } else if (DevData->OSDevInfo) {
	/*
	 * No probe function, but we do have OSDevInfo which was
	 * passed in by the OS specific DevData gather function, so use it.
	 */
	return(ProbeOSDevInfo(&ProbeData));
    } else if (FLAGS_ON(DevData->Flags, DD_IS_ROOT))
	/*
	 * No probe function and no OS provided DevInfo, so do a generic
	 * probe.
	 */
	return(DeviceCreate(&ProbeData));

    /*
     * The device is unknown to us.  If it's definetly alive,
     * return a minimal device entry for it.  If it's not alive,
     * ignore it.
     */
    if (DoPrintUnknown && DevName && 
	FLAGS_ON(DevData->Flags, DD_IS_ALIVE))
	return(ProbeUnknown(&ProbeData));

    SImsg(SIM_DBG, "ProbeDevice: <%s> is not defined.", ARG(DevName));

    return((DevInfo_t *) NULL);
}

/*
 * Make a master device from a DevData controller
 */
extern DevInfo_t *MkMasterFromDevData(DevData)
    DevData_t 		       *DevData;
{
    register DevInfo_t 	       *DevInfo = NULL;
    register DevDefine_t       *DevDefine;
    int 			type = 0;
    int 			flags = 0;

    if (DevData->CtlrName) {
	DevInfo = NewDevInfo(NULL);
	if (DevDefine = DevDefGet(DevData->CtlrName, 0, 0)) {
	    type = DevDefine->Type;
	    flags = DevDefine->Flags;
	    DevInfo->Model = DevDefine->Model;
	    DevInfo->ModelDesc = DevDefine->Desc;
	}
	DevInfo->Name = MkDevName(DevData->CtlrName,
				    DevData->CtlrUnit, 
				    type, flags);
    }

    return(DevInfo);
}

/*
 * Make the file name of the raw device
 */
extern char *GetRawFile(Name, Part)
    char 		       *Name;
    char 		       *Part;
{
    static char 		rfile[128];

    if (!Name)
	return((char *) NULL);

    (void) snprintf(rfile, sizeof(rfile), "/dev/r%s%s", 
		    Name, (Part) ? Part : "");

    return(rfile);
}

/*
 * Make the file name of the character device
 */
extern char *GetCharFile(Name, Part)
    char 		       *Name;
    char 		       *Part;
{
    static char 		file[128];

    if (!Name)
	return((char *) NULL);

    (void) snprintf(file, sizeof(file), "/dev/%s%s", Name, (Part) ? Part : "");

    return(file);
}

/*
 * Assign a device a new (unique to Device's type/name) Unit number.
 */
static int AssignDevUnit(DevName)
     char		       *DevName;
{
    register struct DevUnits   *Ptr;
    register struct DevUnits   *Last = NULL;
    struct DevUnits	       *New;

    for (Ptr = DevUnits; Ptr; Ptr = Ptr->Next) {
	Last = Ptr;
	if (EQ(Ptr->Name, DevName))
	    break;
    }
    if (!Ptr) {
	Ptr = New = (struct DevUnits *) xcalloc(1, sizeof(struct DevUnits));
	New->Name = strdup(DevName);
	New->LastUnit = -1;
	if (Last)
	    Last->Next = New;
	else
	    DevUnits = New;
    }

    return(++Ptr->LastUnit);
}

/*
 * Make device name
 */
extern char *MkDevName(Name, Unit, Type, DdtFlags)
    char 		       *Name;
    int 			Unit;
    int 			Type;
    int 			DdtFlags;
{
    static char			Buff[128];

    /* 
     * If DDT_ISALIAS is set, our calling function is doing aliases so
     * we don't want to create a new, unique name.
     */
    if (FLAGS_ON(DdtFlags, DDT_ASSUNIT) && FLAGS_ON(DdtFlags, DDT_ISALIAS))
        return((char *) NULL);

    if (FLAGS_ON(DdtFlags, DDT_ASSUNIT) && Unit < 0) {
	Unit = AssignDevUnit(Name);
    }

    /*
     * Don't attach unit number if this is a pseudo device and DDT_UNITNUM
     * is not set.
     */
    if (Unit < 0 || FLAGS_ON(DdtFlags, DDT_NOUNIT) ||
	((Type == DT_PSEUDO || Type == DT_NONE) && FLAGS_OFF(DdtFlags, 
							     DDT_UNITNUM)))
	snprintf(Buff, sizeof(Buff), "%s", Name);
    else {
	/*
	 * Special handling if the last char of Name is a digit.
	 */
	if (isdigit(Name[strlen(Name) - 1]))
	    snprintf(Buff, sizeof(Buff), "%s/%d", Name, Unit);
	else
	    snprintf(Buff, sizeof(Buff), "%s%d", Name, Unit);
    }

    return(strdup(Buff));
}

/*
 * Make master device name
 */
extern char *MkMasterName(DevData, DdtFlags)
     DevData_t		       *DevData;
     int			DdtFlags;
{
    DevDefine_t		       *DevDef;

    if (!DevData || !DevData->CtlrName)
	return((char *) NULL);

    DevDef = DevDefGet(DevData->CtlrName, 0, 0);

    return(MkDevName(DevData->CtlrName, DevData->CtlrUnit, 
		     (DevDef) ? DevDef->Type : 0,
		     (DevDef) ? DevDef->Flags : 0));
}

/*
 * Create a minimal device type for an unknown device.
 */
extern DevInfo_t *ProbeUnknown(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    DevData_t 		       *DevData;

    if (!ProbeData)
	return((DevInfo_t *) NULL);
    DevData = ProbeData->DevData;

    DevInfo = NewDevInfo((DevInfo_t *) NULL);
    DevInfo->Name = strdup(MkDevName(DevData->DevName, 
				       DevData->DevUnit,
				       -1, 0));
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = DevData->DevUnit;
    DevInfo->NodeID = DevData->NodeID;
    AddDevDesc(DevInfo, "unknown device type", NULL, DA_APPEND);
    DevInfo->Master = MkMasterFromDevData(DevData);

    return(DevInfo);
}

/*
 * Make a nice looking frequency string.
 */
extern char *FreqStr(freq)
    u_long			freq;
{
    static char			buff[64];

    if (freq > MHERTZ)
	(void) snprintf(buff, sizeof(buff), "%d MHz", freq / MHERTZ);
    else
	(void) snprintf(buff, sizeof(buff), "%d Hz", freq);

    return(buff);
}
