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
 * Device routines
 */

#include <stdio.h>
#include "defs.h"

extern void PrintDiskdrive();
extern void PrintFrameBuffer();
extern void PrintMonitor();
extern void PrintNetIf();
extern void PrintDevice();
extern void PrintGeneric();
static void DevTreeVerify();

static DevInfo_t 	       *RootDev = NULL;
static DevInfo_t	       *OrphanList = NULL;
static float 			TotalDisk = 0;
extern int			OffSetAmt;
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
int AddDevDesc(DevInfo, Desc, Label, Action)
    DevInfo_t		       *DevInfo;
    char		       *Desc;
    char		       *Label;
    int				Action;
{
    register DevDesc_t	       *Ptr;
    DevDesc_t		       *New;

    if (!DevInfo || !Desc)
	return(-1);

    /*
     * Check for duplicates
     */
    for (Ptr = DevInfo->DescList; Ptr; Ptr = Ptr->Next)
	if (EQ(Ptr->Desc, Desc) && EQ(Ptr->Label, Label))
	    return(-1);

    /*
     * Create new dev description
     */
    New = (DevDesc_t *) xcalloc(sizeof(DevDesc_t), 1);
    New->Desc = strdup(Desc);
    if (Label)
	New->Label = strdup(Label);
    if (Action & DA_PRIME)
	New->Flags |= DA_PRIME;

    /*
     * Add to list
     */
    if (DevInfo->DescList) {
	if (Action & DA_INSERT) {
	    New->Next = DevInfo->DescList;
	    DevInfo->DescList = New;
	} else {
	    for (Ptr = DevInfo->DescList; Ptr && Ptr->Next; 
		 Ptr = Ptr->Next);
	    Ptr->Next = New;
	}
    } else
	DevInfo->DescList = New;

    return(0);
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
    for (i = 0, cpp = Mon->Resolutions; cpp && cpp[i]; ++i)
	/* Do case sensitive match check */
	if (strcmp(cpp[i], Resolution) == 0)
	    return;

    /*
     * Re alloc buffer and add File
     */
    Mon->Resolutions = (char **) xrealloc(Mon->Resolutions, 
					  (i+2)*sizeof(char *));
    Mon->Resolutions[i] = Resolution;
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
 * Get a nice size string
 */
extern char *GetSizeStr(Amt, Unit)
    Large_t			Amt;
    Large_t			Unit;
{
    static char			Buff[100];

    Buff[0] = CNULL;

    if (Unit) {
	if (Unit == GBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f GB", (float) Amt);
	else if (Unit == MBYTES) {
	    if (Amt > KBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.1f GB",
				(float) mbytes_to_gbytes(Amt));
	    else
		(void) snprintf(Buff, sizeof(Buff), "%.0f MB",
				(float) Amt);
	} else if (Unit == KBYTES) {
	    if (Amt > MBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.1f GB",
				(float) kbytes_to_gbytes(Amt));
	    else if (Amt > KBYTES)
		(void) snprintf(Buff, sizeof(Buff), "%.0f MB",
				(float) kbytes_to_mbytes(Amt));
	    else
		(void) snprintf(Buff, sizeof(Buff), "%.0f KB",
				(float) Amt);
	}
    }

    if (Buff[0] == CNULL) {
	if (Amt < KBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f Bytes", (float) Amt);
	else if (Amt < MBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f KB",
			    (float) bytes_to_kbytes(Amt));
	else if (Amt < GBYTES)
	    (void) snprintf(Buff, sizeof(Buff), "%.0f MB",
			    (float) bytes_to_mbytes(Amt));
	else
	    (void) snprintf(Buff, sizeof(Buff), "%.1f GB",
			    (float) bytes_to_gbytes(Amt));
    }

    return(Buff);
}

/*
 * List valid arguments for Devices class. 
 */
extern void DeviceList()
{
    SImsg(SIM_INFO, 
"\n\nTo show specific devices use `-class device -show Dev1,Dev2...'.\n\n");
}

/*
 * Print all device info
 */
extern void DeviceShow(MyInfo, Names)
    ClassInfo_t		       *MyInfo;
    char		      **Names;
{
    static char		       *RptData[2];

#if	defined(HAVE_DEVICE_SUPPORT)
    if (BuildDevices(&RootDev, Names) != 0)
	return;
#endif	/* HAVE_DEVICE_SUPPORT */

    if (!RootDev) {
	SImsg(SIM_DBG, "No devices were found.");
	return;
    }

    DevTreeVerify(&RootDev);

    ClassShowBanner(MyInfo);

    TotalDisk = (float) 0;

    PrintDevice(RootDev, 0);

    if (VL_DESC && TotalDisk > (float) 0)
	switch (FormatType) {
	case FT_PRETTY:
	    SImsg(SIM_INFO, "\nTotal Disk Capacity is %s\n", 
		   GetSizeStr((Large_t) TotalDisk, 
			      (Large_t) MBYTES));
	    break;
	case FT_REPORT:
	    RptData[0] = GetSizeStr((Large_t) TotalDisk, 
				    (Large_t) MBYTES);
	    Report(CN_DEVICE, R_TOTALDISK, NULL, RptData, 1);
	    break;
	}
}

/*
 * --RECURSE--
 * Print info about a device.  Recursively calls itself for slaves and
 * next elements
 */
extern void PrintDevice(DevInfo, OffSet)
    DevInfo_t 		       *DevInfo;
    int 			OffSet;
{
    DevType_t 		       *DevType = NULL;

    DevType = TypeGetByType(DevInfo->Type);
    if (!DevType)
	DevType = TypeGetByType(DT_GENERIC);

    /*
     * If device->Name is not set, this is the root of the device tree
     */
    if (DevInfo->Name && DevType->Enabled) {
	if (DevType->Print)
	    (*DevType->Print)(DevInfo, DevType, OffSet);
	else
	    PrintGeneric(DevInfo, DevType, OffSet);
    }

    /*
     * Descend
     */
    if (DevInfo->Slaves)
	PrintDevice(DevInfo->Slaves, (DevInfo->Name) ? 
		    OffSet+OffSetAmt : 0);

    /*
     * Traverse
     */
    if (DevInfo->Next)
	PrintDevice(DevInfo->Next, (DevInfo->Name) ? OffSet : 0);
}

/*
 * Print Off Set space
 */
static void PrOffSet(cnt)
    int 			cnt;
{
    if (FormatType == FT_PRETTY)
	SImsg(SIM_INFO, "%*s", cnt, "");
}

/*
 * Print a device label
 */
static void PrDevLabel(Name, OffSet)
    char 		       *Name;
    int 			OffSet;
{
    PrOffSet(OffSet);
    if (VL_CONFIG)
	SImsg(SIM_INFO, "%*s%-22s:", OffSetAmt, "", Name);
    else
	SImsg(SIM_INFO, "%*s%22s:", OffSetAmt, "", Name);
}

char *GetDescLabel(DevDesc)
    DevDesc_t		       *DevDesc;
{
    if (DevDesc && DevDesc->Label)
	return(DevDesc->Label);
    else
	return("Description");
}

/*
 * Get the prime description of a device.
 */
extern char *PrimeDesc(DevInfo)
    DevInfo_t		       *DevInfo;
{
    register DevDesc_t	       *DevDesc;

    if (!DevInfo)
	return((char *) NULL);

    for (DevDesc = DevInfo->DescList; DevDesc; DevDesc = DevDesc->Next)
	if (FLAGS_ON(DevDesc->Flags, DA_PRIME))
	    return(DevDesc->Desc);

    return((char *) NULL);
}

/*
 * Get a DevDesc_t pointer to the prime description of a device.
 */
extern DevDesc_t *PrimeDescPtr(DevInfo)
    DevInfo_t		       *DevInfo;
{
    register DevDesc_t	       *DevDesc;

    if (!DevInfo)
	return((DevDesc_t *) NULL);

    for (DevDesc = DevInfo->DescList; DevDesc; DevDesc = DevDesc->Next)
	if (FLAGS_ON(DevDesc->Flags, DA_PRIME))
	    return(DevDesc);

    return((DevDesc_t *) NULL);
}

/*
 * Print Generic device info
 */
extern void PrintGenericInfo(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    ClassType_t		       *ClassType = NULL;

    if (FormatType == FT_REPORT)
	return;

    if (VL_CONFIG) {
	if (DevInfo->Vendor) {
	    PrDevLabel("Vendor", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Vendor);
	}
	if (DevInfo->Model) {
	    PrDevLabel("Model", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Model);
	}
	if (DevInfo->Serial) {
	    PrDevLabel("Serial", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Serial);
	}
	if (DevInfo->Revision) {
	    PrDevLabel("Revision", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Revision);
	}
	if (DevInfo->Unit >= 0) {
	    PrDevLabel("Unit", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Unit);
	}
	if (DevInfo->Addr >= 0) {
	    PrDevLabel("Address", OffSet);
	    SImsg(SIM_INFO, " 0x%x\n", DevInfo->Addr);
	}
	if (DevInfo->Prio >= 0) {
	    PrDevLabel("Priority", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Prio);
	}
	if (DevInfo->Vec >= 0) {
	    PrDevLabel("Vector", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Vec);
	}
	if (DevType && DevType->Desc) {
	    PrDevLabel("Device Type", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevType->Desc);
	}
	if (ClassType = ClassTypeGetByType(DevInfo->Type, DevInfo->ClassType)){
	    PrDevLabel("Class", OffSet);
	    SImsg(SIM_INFO, " %s\n", 
		  (ClassType->Desc) ? ClassType->Desc : ClassType->Name);
	}
    }
}

/*
 * Print general device information in FT_PRETTY format
 */
static void PrintDeviceInfoPretty(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{     
    register DevDesc_t	       *DevDesc;
    DevType_t 		       *mdt;
    ClassType_t		       *ClassType = NULL;
    register DevInfo_t 	       *pd;
    register char	       *cp;
    register char	      **cpp;

    if (!DevInfo->Name)
	return;

    if (VL_CONFIG) SImsg(SIM_INFO, "\n");
    PrOffSet(OffSet);
    SImsg(SIM_INFO, "%s", DevInfo->Name);

    if (DevInfo->AltName)
	SImsg(SIM_INFO, " (%s)", DevInfo->AltName);
    
    if (DevInfo->Vendor || DevInfo->Model || DevInfo->ModelDesc ||
	(DevType && DevType->Desc)) {
	SImsg(SIM_INFO, " is a");
	if (DevInfo->Vendor || DevInfo->Model)
	    SImsg(SIM_INFO, " \"%s%s%s\"", 
		  (DevInfo->Vendor) ? DevInfo->Vendor : "",
		  (DevInfo->Vendor && DevInfo->Model) ? " " : "",
		  (DevInfo->Model) ? DevInfo->Model : "");

	if (ClassType = ClassTypeGetByType(DevInfo->Type, DevInfo->ClassType))
	    SImsg(SIM_INFO, " %s", 
		  (ClassType->Desc) ? ClassType->Desc : ClassType->Name);

	/* Make sure ModelDesc != ClassType */
	if (DevInfo->ModelDesc &&
	    (!ClassType || !EQ(ClassType->Name, DevInfo->ModelDesc)))
	    SImsg(SIM_INFO, " %s", DevInfo->ModelDesc);

	if (DevType && DevType->Desc)
	    SImsg(SIM_INFO, " %s", DevType->Desc);
	else if ((cp = PrimeDesc(DevInfo)) && !EQ(cp, DevInfo->Model))
	    SImsg(SIM_INFO, " %s", cp);
    } else if (cp = PrimeDesc(DevInfo)) {
	SImsg(SIM_INFO, " is a");
	SImsg(SIM_INFO, " %s", cp);
    }

    if (DevInfo->Name)
	SImsg(SIM_INFO, "\n");

    PrintGenericInfo(DevInfo, DevType, OffSet);

    if (VL_DESC || VL_CONFIG) {
	if (DevInfo->Aliases && DevInfo->Aliases[0]) {
	    PrDevLabel("Alias Names", OffSet);
	    for (cpp = DevInfo->Aliases; cpp && *cpp; ++cpp)
		SImsg(SIM_INFO, " %s", *cpp);
	    SImsg(SIM_INFO, "\n");
	}
	if (DevInfo->DescList)
	    for (DevDesc = DevInfo->DescList; DevDesc; 
		 DevDesc = DevDesc->Next) {
		PrDevLabel(GetDescLabel(DevDesc), OffSet);
		SImsg(SIM_INFO, " %s\n", DevDesc->Desc);
	    }
	if (DevInfo->Files && DevInfo->Files[0]) {
	    PrDevLabel("Device Files", OffSet);
	    for (cpp = DevInfo->Files; cpp && *cpp; ++cpp)
		SImsg(SIM_INFO, " %s", *cpp);
	    SImsg(SIM_INFO, "\n");
	}
	if (DevInfo->Master && DevInfo->Master->Name) {
	    PrDevLabel("Connected to", OffSet);
	    if (DevInfo->Master->Name)
		SImsg(SIM_INFO, " %s", DevInfo->Master->Name);
	    else if (DevInfo->Master->Model) {
		SImsg(SIM_INFO, " %s", DevInfo->Master->Model);
		if (mdt = TypeGetByType(DevInfo->Master->Type))
		    SImsg(SIM_INFO, " %s", mdt->Desc);
	    }
	    SImsg(SIM_INFO, "\n");
	}
    }

    if (VL_CONFIG) {
	if (DevInfo->Slaves && (DevInfo->Name || DevInfo->Model || 
				  (DevType && DevType->Desc))) {
	    PrDevLabel("Attached Device(s)", OffSet);
	    for (pd = DevInfo->Slaves; pd; pd = pd->Next)
		SImsg(SIM_INFO, " %s", pd->Name);
	    SImsg(SIM_INFO, "\n");
	}
    }
}

/*
 * Print general device information in FT_REPORT format
 */
static void PrintDeviceInfoReport(DevInfo, DevType)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
{     
    register DevDesc_t	       *DevDesc;
    ClassType_t		       *ClassType = NULL;
    register char	      **cpp;
    static char		       *RptData[17];
    static char			FileBuff[BUFSIZ];
    static char			UnitBuff[50];
    static char			AddrBuff[50];
    static char			PrioBuff[50];
    static char			VecBuff[50];
    static char			NodeIDBuff[50];

    if (!DevInfo->Name)
	return;

    memset(RptData, CNULL, sizeof(RptData));

    RptData[0] = PRTS(DevInfo->AltName);

    if (DevInfo->Master)
	RptData[1] = PRTS(DevInfo->Master->Name);
    else if (DevInfo->MasterName)
	RptData[1] = PRTS(DevInfo->MasterName);

    FileBuff[0] = CNULL;
    for (cpp = DevInfo->Files; cpp && *cpp; ++cpp) {
	if (FileBuff[0])
	    strcat(FileBuff, ",");
	if (strlen(*cpp) + strlen(FileBuff) + 3 >= sizeof(FileBuff))
	    break;
	strcat(FileBuff, *cpp);
    }
    if (FileBuff[0])
	RptData[2] = FileBuff;

    if (DevType) {
	RptData[3] = PRTS(DevType->Name);
	RptData[4] = PRTS(DevType->Desc);
    }
    RptData[5] = PRTS(DevInfo->Model);
    RptData[6] = PRTS(DevInfo->ModelDesc);

    if (DevInfo->Unit >= 0) {
	(void) snprintf(UnitBuff, sizeof(UnitBuff), "%d", DevInfo->Unit);
	RptData[7] = UnitBuff;
    }
    if (DevInfo->Addr != -1 && DevInfo->Addr != 0) {
	(void) snprintf(AddrBuff, sizeof(AddrBuff), "0x%x", DevInfo->Addr);
	RptData[8] = AddrBuff;
    }
    if (DevInfo->Prio != 0 && DevInfo->Prio != -1) {
	(void) snprintf(PrioBuff, sizeof(PrioBuff), "%d", DevInfo->Prio);
	RptData[9] = PrioBuff;
    }
    if (DevInfo->Vec != 0 && DevInfo->Vec != -1) {
	(void) snprintf(VecBuff, sizeof(VecBuff), "%d", DevInfo->Vec);	
	RptData[10] = VecBuff;
    }
    if (DevInfo->NodeID != 0 && DevInfo->NodeID != -1 && 
	DevInfo->NodeID != -2) {
	(void) snprintf(NodeIDBuff, sizeof(NodeIDBuff), "%d", DevInfo->NodeID);
	RptData[11] = NodeIDBuff;
    }
    if (ClassType = ClassTypeGetByType(DevInfo->Type, DevInfo->ClassType)) {
	RptData[12] = ClassType->Name;
	RptData[13] = (ClassType->Desc) ? ClassType->Desc : "";
    } 
    if (DevInfo->Vendor)
	RptData[14] = DevInfo->Vendor;
    if (DevInfo->Serial)
	RptData[15] = DevInfo->Serial;
    if (DevInfo->Revision)
	RptData[16] = DevInfo->Revision;

    Report(CN_DEVICE, R_NAME, PRTS(DevInfo->Name), RptData, 
	   sizeof(RptData) / sizeof(char *));


    /*
     * Report description info
     */
    for (DevDesc = DevInfo->DescList; DevDesc; DevDesc = DevDesc->Next) {
	RptData[0] = PRTS(DevDesc->Label);
	RptData[1] = PRTS(DevDesc->Desc);
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
    }
}

/*
 * Print general device information
 */
extern void PrintGeneric(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    switch (FormatType) {
    case FT_PRETTY:	PrintDeviceInfoPretty(DevInfo, DevType, OffSet); break;
    case FT_REPORT:	PrintDeviceInfoReport(DevInfo, DevType); break;
    }
}

/*
 * Print info about disk partitioning in FT_PRETTY format.
 */
static void PrintDiskPartPretty(Disk, OffSet)
    DiskDrive_t		       *Disk;
    int 			OffSet;
{
    register DiskPart_t        *pp;
    static char			Buff[128];
    static char			TitleDef[] = "Partition Information";
    int				SecSize = 512;

    if (!Disk)
	return;

    if (Disk->SecSize)
	SecSize = Disk->SecSize;

    SImsg(SIM_INFO, "\n");
    PrOffSet(OffSet);
    if (Disk->DiskPart && Disk->DiskPart->Title)
	(void) snprintf(Buff, sizeof(Buff), "%s %s", 
			Disk->DiskPart->Title, TitleDef);
    else
	(void) snprintf(Buff, sizeof(Buff), "%s", TitleDef);
    SImsg(SIM_INFO, "%40s\n", Buff);

    PrOffSet(OffSet);
    SImsg(SIM_INFO, "%8s %10s %10s %9s\n",
	   "", "START", "NUMBER OF", "SIZE");

    PrOffSet(OffSet);
    SImsg(SIM_INFO, "%8s %10s %10s %9s %8.8s %s\n",
	   "PART", "SECTOR", "SECTORS", "(MB)", "TYPE", "USAGE");

    for (pp = Disk->DiskPart; pp; pp = pp->Next) {
	PrOffSet(OffSet);
	SImsg(SIM_INFO, "%8s %10.0f %10.0f %9.2f %8.8s %s\n",
	       pp->Name,
	       (float) pp->StartSect,
	       (float) pp->NumSect,
	       bytes_to_mbytes(nsect_to_bytes(pp->NumSect, SecSize)),
	       (pp->Type) ? pp->Type : "",
	       (pp->Usage) ? pp->Usage : ""
	       );
    }
}

/*
 * Print info about disk partitioning in FT_REPORT format.
 *
 * Output is in format:
 *
 *	device|desc|$Name|part|$PartName|$StartSect|$NumSect|$Size|$Use
 */
static void PrintDiskPartReport(Disk, DevInfo)
    DiskDrive_t		       *Disk;
    DevInfo_t		       *DevInfo;
{
    register DiskPart_t        *pp;
    static char		       *RptData[7];
    static char			StartBuff[50];
    static char			NumBuff[50];
    static char			SizeBuff[50];

    if (!Disk || !DevInfo)
	return;

    for (pp = Disk->DiskPart; pp; pp = pp->Next) {
	(void) snprintf(StartBuff, sizeof(StartBuff), "%.0f",
			(float) pp->StartSect);
	(void) snprintf(NumBuff, sizeof(NumBuff), "%.0f", (float) pp->NumSect);
	(void) snprintf(SizeBuff, sizeof(SizeBuff), "%9.2f", 
		       bytes_to_mbytes(nsect_to_bytes(pp->NumSect, 
						      Disk->SecSize)));
	RptData[0] = R_PART;
	RptData[1] = PRTS(pp->Name);
	RptData[2] = StartBuff;
	RptData[3] = NumBuff;
	RptData[4] = SizeBuff;
	RptData[5] = PRTS(pp->Type);
	RptData[6] = PRTS(pp->Usage);
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 7);
    }
}

/*
 * Get the capacity of a disk drive
 */
extern char *GetDiskSize(DevInfo, DiskDrive)
    DevInfo_t		       *DevInfo;
    DiskDrive_t		       *DiskDrive;
{
    float			Amt = 0;
    static char			Buff[32];
    u_long			Cyls = 0;

    if (!DevInfo || !DiskDrive)
	return((char *) NULL);

    SImsg(SIM_DBG, 
	 "%s: GetDiskSize size=%.2f dcyl=%d pcyl=%d sect=%d hd=%d secsize=%d",
	  DevInfo->Name,
	  (float)DiskDrive->Size,
	  DiskDrive->DataCyl, DiskDrive->PhyCyl, 
	  DiskDrive->Sect, DiskDrive->Tracks,
	  DiskDrive->SecSize);

    if (DiskDrive->DataCyl)
	Cyls = DiskDrive->DataCyl;
    else if (DiskDrive->PhyCyl)
	Cyls = DiskDrive->PhyCyl;

    if (DiskDrive->Size == 0 &&
	Cyls && DiskDrive->Sect && DiskDrive->Tracks) {

	DiskDrive->Size = (float) nsect_to_mbytes(Cyls * 
						  DiskDrive->Sect * 
						  DiskDrive->Tracks, 
						  DiskDrive->SecSize);
    }

    if (DiskDrive->Size > 0)
	Amt = DiskDrive->Size;

    /* If it's small enough, we care about the precision so we do it here */
    if (Amt > 0 && Amt < 5) {
	(void) snprintf(Buff, sizeof(Buff), "%.1f MB", Amt);
	return(Buff);
    } else if (Amt)
	return(GetSizeStr((Large_t) Amt, MBYTES));
    else
	return((char *) NULL);
}

/*
 * Assemble the "Label" piece of disk info for output.
 */
static char *PrDkLbl(Disk, Label)
     DiskDrive_t	       *Disk;
     char		       *Label;
{
    static char			Buff[256];
    int				Len;

    if (!Disk || !Label)
	return((char *) NULL);

    Buff[0] = CNULL;

    if (Disk->DataType)
	(void) snprintf(Buff, sizeof(Buff), "%s ", Disk->DataType);

    Len = strlen(Buff);
    (void) snprintf(&Buff[Len], sizeof(Buff)-Len, "%s", Label);

    return(Buff);
}

/*
 * Print Disk Drive information in FT_PRETTY format.
 */
static void PrintDiskDrivePretty(DevInfo, Disk, OffSet)
    DevInfo_t 		       *DevInfo;
    DiskDrive_t		       *Disk;
    int 			OffSet;
{
    static char			Buff[16];
    char		       *DiskSize;

    if (!DevInfo || !Disk)
	return;

    if (DiskSize = GetDiskSize(DevInfo, Disk))
	AddDevDesc(DevInfo, DiskSize,
		   PrDkLbl(Disk, "Capacity"), DA_APPEND);
    if (Disk->Label)
	AddDevDesc(DevInfo, Disk->Label, 
		   PrDkLbl(Disk, "Disk Label"), DA_APPEND);
    if (Disk->Unit >= 0) {
	if (FLAGS_ON(Disk->Flags, DF_HEXUNIT))
	    (void) snprintf(Buff, sizeof(Buff), "%.3x", Disk->Unit);
	else
	    (void) snprintf(Buff, sizeof(Buff), "%d", Disk->Unit);
	AddDevDesc(DevInfo, Buff, 
		   PrDkLbl(Disk, "Unit Number"), DA_APPEND);
    }
    if (Disk->Slave >= 0)
	AddDevDesc(DevInfo, itoa(Disk->Slave), 
		   PrDkLbl(Disk, "Slave Number"), DA_APPEND);

    /* Cylinders */
    if (Disk->DataCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->DataCyl), 
		   PrDkLbl(Disk, "Data Cylinders"), DA_APPEND);
    if (Disk->PhyCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->PhyCyl), 
		   PrDkLbl(Disk, "Physical Cylinders"), DA_APPEND);
    if (Disk->AltCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->AltCyl), 
		   PrDkLbl(Disk, "Alt Cylinders"), DA_APPEND);
    if (Disk->CylSkew > 0)
	AddDevDesc(DevInfo, itoa(Disk->CylSkew), 
		   PrDkLbl(Disk, "Cylinder Skew"), DA_APPEND);
    if (Disk->APC > 0)
	AddDevDesc(DevInfo, itoa(Disk->APC), 
		   PrDkLbl(Disk, "Alternates / Cylinder"), DA_APPEND);

    /* Tracks */
    if (Disk->Tracks > 0)
	AddDevDesc(DevInfo, itoa(Disk->Tracks), 
		   PrDkLbl(Disk, "Tracks"), DA_APPEND);
    if (Disk->AltTracksPerZone > 0)
	AddDevDesc(DevInfo, itoa(Disk->AltTracksPerZone), 
		   PrDkLbl(Disk, "Alt Tracks/Zone"), DA_APPEND);
    if (Disk->AltTracksPerVol > 0)
	AddDevDesc(DevInfo, itoa(Disk->AltTracksPerVol), 
		   PrDkLbl(Disk, "Alt Tracks/Volume"), DA_APPEND);
    if (Disk->TrackSkew > 0)
	AddDevDesc(DevInfo, itoa(Disk->TrackSkew), 
		   PrDkLbl(Disk, "Track Skew"), DA_APPEND);

    /* Floppy related */
    if (Disk->StepsPerTrack > 0)
	AddDevDesc(DevInfo, itoa(Disk->StepsPerTrack), 
		   PrDkLbl(Disk, "Steps/Tracks"), DA_APPEND);

    /* Sectors */
    if (Disk->Sect > 0)
	AddDevDesc(DevInfo, itoa(Disk->Sect), 
		   PrDkLbl(Disk, "Sectors/Track"), DA_APPEND);
    if (Disk->SecSize > 0)
	AddDevDesc(DevInfo, itoa(Disk->SecSize), 
		   PrDkLbl(Disk, "Sector Size"), DA_APPEND);
    if (Disk->PhySect > 0)
	AddDevDesc(DevInfo, itoa(Disk->PhySect), 
		   PrDkLbl(Disk, "Physical Sectors/Track"), DA_APPEND);
    if (Disk->AltSectPerZone > 0)
	AddDevDesc(DevInfo, itoa(Disk->AltSectPerZone), 
		   PrDkLbl(Disk, "Alt Sectors/Zone"), DA_APPEND);
    if (Disk->SectGap > 0)
	AddDevDesc(DevInfo, itoa(Disk->SectGap), 
		   PrDkLbl(Disk, "Sector Gap Length"), DA_APPEND);

    /* General Misc */
    if (Disk->RPM > 0)
	AddDevDesc(DevInfo, itoa(Disk->RPM), 
		   PrDkLbl(Disk, "RPM"), DA_APPEND);
    if (Disk->IntrLv > 0)
	AddDevDesc(DevInfo, itoa(Disk->IntrLv), 
		   PrDkLbl(Disk, "Interleave"), DA_APPEND);
    if (Disk->PROMRev > 0)
	AddDevDesc(DevInfo, itoa(Disk->PROMRev), 
		   PrDkLbl(Disk, "PROM Revision"), DA_APPEND);

}

/*
 * Print Disk Drive information in FT_REPORT format.
 *
 * Output format is:
 *
 *	device|desc|$Name|$Label|$Desc|$Flags|$DataType
 */
static void PrintDiskDriveReport(DevInfo, Disk)
    DevInfo_t 		       *DevInfo;
    DiskDrive_t		       *Disk;
{
    static char		       *RptData[4];
    static char			Buff[50];
    char		       *DiskSize = NULL;

    if (!DevInfo || !Disk)
	return;

    /* Set the DataType for all calls below */
    RptData[2] = Disk->DataType;

    if (DiskSize = GetDiskSize(DevInfo, Disk)) {
	(void) snprintf(Buff, sizeof(Buff), "%s", DiskSize);
	RptData[0] = "capacity";
	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }

    if (Disk->Label) {
	RptData[0] = "disklabel";
	RptData[1] = Disk->Label;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->Unit >= 0) {
	if (FLAGS_ON(Disk->Flags, DF_HEXUNIT))
	    (void) snprintf(Buff, sizeof(Buff), "%.3x", Disk->Unit);
	else
	    (void) snprintf(Buff, sizeof(Buff), "%d", Disk->Unit);
	RptData[0] = "unit";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->Slave >= 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->Slave);
	RptData[0] = "slave";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }

    /* Cylinders */
    if (Disk->DataCyl > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->DataCyl);
	RptData[0] = "datacyl";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->PhyCyl > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->PhyCyl);
	RptData[0] = "phycyl";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->AltCyl > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->AltCyl);
	RptData[0] = "altcyl";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->CylSkew > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->CylSkew);
	RptData[0] = "cylskew";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->APC > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->APC);
	RptData[0] = "apc";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }

    /* Tracks */
    if (Disk->Tracks > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->Tracks);
	RptData[0] = "tracks";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->AltTracksPerZone > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->AltTracksPerZone);
	RptData[0] = "alttracksperzone";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->AltTracksPerVol > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->AltTracksPerVol);
	RptData[0] = "alttrackspervol";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->TrackSkew > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->TrackSkew);
	RptData[0] = "trackskew";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }

    /* Sectors */
    if (Disk->Sect > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->Sect);
	RptData[0] = "sectors";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->SecSize > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->SecSize);
	RptData[0] = "sectsize";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->PhySect > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->PhySect);
	RptData[0] = "hardsectors";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->AltSectPerZone > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->AltSectPerZone);
	RptData[0] = "altsectperzone";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }

    /* General Misc */
    if (Disk->RPM > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->RPM);
	RptData[0] = "rpm";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->IntrLv > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->IntrLv);
	RptData[0] = "interleave";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
    if (Disk->PROMRev > 0) {
	(void) snprintf(Buff, sizeof(Buff), "%d", Disk->PROMRev);
	RptData[0] = "promrev";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 3);
    }
}

/*
 * Print info about a disk device.
 */
extern void PrintDiskdrive(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    DiskDriveData_t	       *Data = NULL;
    DiskDrive_t		       *Disk = NULL;
    char		       *DiskSize = NULL;

    if (DevInfo && DevInfo->DevSpec)
	Data = (DiskDriveData_t *) DevInfo->DevSpec;

    /*
     * Make sure DataType is set
     */
    if (Data && Data->HWdata && !Data->HWdata->DataType)
	Data->HWdata->DataType = DK_DTYPE_HW;
    if (Data && Data->OSdata && !Data->OSdata->DataType)
	Data->OSdata->DataType = DK_DTYPE_OS;
	
    /*
     * Disk is what we should use for general info about this disk.
     * We prefer the HW data over the OS data.
     */
    if (Data && Data->HWdata)
	Disk = Data->HWdata;
    else if (Data && Data->OSdata)
	Disk = Data->OSdata;

    if (Disk) {
	if (DiskSize = GetDiskSize(DevInfo, Disk))
	    DevInfo->ModelDesc = strdup(DiskSize);
	TotalDisk += Disk->Size;
    }

    if (Data) {
	if (VL_CONFIG) {
	    switch (FormatType) {
	    case FT_PRETTY:
		if (Data->HWdata)
		    PrintDiskDrivePretty(DevInfo, Data->HWdata, OffSet);
		if (Data->OSdata)
		    PrintDiskDrivePretty(DevInfo, Data->OSdata, OffSet);
		break;
	    case FT_REPORT:
		if (Data->HWdata)
		    PrintDiskDriveReport(DevInfo, Data->HWdata);
		if (Data->OSdata)
		    PrintDiskDriveReport(DevInfo, Data->OSdata);
		break;
	    }
	}
    }

    PrintGeneric(DevInfo, DevType, OffSet);

    if (VL_ALL && Data && Data->HWdata && Data->HWdata->DiskPart)
	switch (FormatType) {
	case FT_PRETTY: 
	    PrintDiskPartPretty(Data->HWdata, OffSet);
	    break;
	case FT_REPORT:	
	    PrintDiskPartReport(Data->HWdata, DevInfo);
	    break;
	}
    if (VL_ALL && Data && Data->OSdata && Data->OSdata->DiskPart)
	switch (FormatType) {
	case FT_PRETTY: 
	    PrintDiskPartPretty(Data->OSdata, OffSet);
	    break;
	case FT_REPORT:	
	    PrintDiskPartReport(Data->OSdata, DevInfo);
	    break;
	}
}

/*
 * Print info about a frame buffer.
 */
extern void PrintFrameBuffer(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    FrameBuffer_t 	       *fb = NULL;
    static char		       *RptData[3];

    if (DevInfo && DevInfo->DevSpec)
	fb = (FrameBuffer_t *) DevInfo->DevSpec;

    if (fb && VL_CONFIG) {
	if (FormatType == FT_PRETTY) {
	    if (fb->CMSize)
		AddDevDesc(DevInfo, itoa(fb->CMSize), "Color Map Size",
			   DA_INSERT);
	    if (fb->Size)
		AddDevDesc(DevInfo, 
			   GetSizeStr((Large_t)fb->Size, BYTES),
			   "Total Size", DA_INSERT);
	    if (fb->VMSize)
		AddDevDesc(DevInfo, 
			   GetSizeStr((Large_t)fb->VMSize, BYTES),
			   "Video Memory", DA_INSERT);
	    if (fb->Height)
		AddDevDesc(DevInfo, itoa(fb->Height), "Height", DA_APPEND);
	    if (fb->Width)
		AddDevDesc(DevInfo, itoa(fb->Width), "Width", DA_APPEND);
	    if (fb->Depth)
		AddDevDesc(DevInfo, itoa(fb->Depth), "Depth (bits)", 
			   DA_APPEND);
	} else if (FormatType == FT_REPORT) {
	    if (fb->CMSize > 0) {
		RptData[0] = "colormapsize";	RptData[1] = itoa(fb->CMSize);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }
		   
	    if (fb->VMSize > 0) {
		RptData[0] = "videomemsize";	RptData[1] = itoa(fb->VMSize);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }

	    if (fb->Size > 0) {
		RptData[0] = "videomemsize";	RptData[1] = itoa(fb->Size);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }

	    if (fb->Height > 0) {
		RptData[0] = "height";	RptData[1] = itoa(fb->Height);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }

	    if (fb->Width > 0) {
		RptData[0] = "width";	RptData[1] = itoa(fb->Width);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }

	    if (fb->Depth > 0) {
		RptData[0] = "depth";	RptData[1] = itoa(fb->Depth);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }
	}
    }

    PrintGeneric(DevInfo, DevType, OffSet);
}

/*
 * Print info about a Monitor
 */
extern void PrintMonitor(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    Monitor_t	 	       *Mon = NULL;
    static char			Buff[BUFSIZ];
    static char		       *RptData[3];
    register char	      **cpp;

    PrintGeneric(DevInfo, DevType, OffSet);

    if (DevInfo && DevInfo->DevSpec)
	Mon = (Monitor_t *) DevInfo->DevSpec;

    if (Mon && VL_CONFIG) {
	if (FormatType == FT_PRETTY) {
	    if (Mon->MaxHorSize) {
		PrDevLabel("Max Horizontal Image Size (cm)", OffSet);
		SImsg(SIM_INFO, " %d\n", Mon->MaxHorSize);
	    }
	    if (Mon->MaxVerSize) {
		PrDevLabel("Max Vertical Image Size (cm)", OffSet);
		SImsg(SIM_INFO, " %d\n", Mon->MaxVerSize);
	    }
	    if (Mon->Resolutions) {
		PrDevLabel("Supported Resolutions", OffSet);
		for (cpp = Mon->Resolutions; cpp && *cpp; ++cpp)
		    SImsg(SIM_INFO, " %s", *cpp);
		SImsg(SIM_INFO, "\n");
	    }
	} else if (FormatType == FT_REPORT) {
	    if (Mon->Resolutions) {
		Buff[0] = CNULL;
		for (cpp = Mon->Resolutions; cpp && *cpp; ++cpp) {
		    if (Buff[0])
			strcat(Buff, ",");
		    if (strlen(*cpp) + strlen(Buff) + 3 >= sizeof(Buff))
			break;
		    strcat(Buff, *cpp);
		}
		RptData[0] = "resolutions";
		RptData[1] = Buff;
		Report(CN_DEVICE, R_DESC, Buff, RptData, 2);
	    }
	    if (Mon->MaxHorSize) {
		RptData[0] = "maxhorsize";
		RptData[1] = itoa(Mon->MaxHorSize);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }
	    if (Mon->MaxVerSize) {
		RptData[0] = "maxversize";
		RptData[1] = itoa(Mon->MaxVerSize);
		Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 2);
	    }
	}
    }
}

/*
 * Print info about a network interface in FT_PRETTY format
 */
static void PrintNetIfPretty(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    register NetIF_t	       *ni;

    for (ni = (NetIF_t *) DevInfo->DevSpec; ni; ni = ni->Next) {
	if (ni->TypeName) {
	    PrDevLabel("Address Type", OffSet);
	    SImsg(SIM_INFO, " %s\n", ni->TypeName);
	}

	if (ni->HostAddr) {
	    PrDevLabel("Host Address", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->HostAddr,
		   (ni->HostName) ? ni->HostName : "<unknown>");
	}

	if (ni->NetAddr) {
	    PrDevLabel("Network Address", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->NetAddr, 
		   (ni->NetName) ? ni->NetName : "<unknown>");
	}

	if (ni->MACaddr) {
	    PrDevLabel("Current MAC Addr", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->MACaddr,
		   (ni->MACname && ni->MACname[0]) 
		   ? ni->MACname : "<unknown>");
	}

	if (ni->FacMACaddr) {
	    PrDevLabel("Factory MAC Addr", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->FacMACaddr,
		   (ni->FacMACname && ni->FacMACname[0]) 
		   ? ni->FacMACname : "<unknown>");
	}
    }
}

/*
 * Print info about a network interface in FT_REPORT format
 *
 * Output format is:
 *
 *	device|desc|$Name|$AddrType|$HostAddr|$HostName|\
 *		$NetAddr|$NetName|$MACaddr|$MACname|
 *		$FacMACaddr|$FacMACname
 */
static void PrintNetIfReport(DevInfo, DevType)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
{
    register NetIF_t	       *ni;
    static char		       *RptData[9];

    for (ni = (NetIF_t *) DevInfo->DevSpec; ni; ni = ni->Next) {
	RptData[0] = PRTS(ni->TypeName);
	RptData[1] = PRTS(ni->HostAddr);
	RptData[2] = PRTS(ni->HostName);
	RptData[3] = PRTS(ni->NetAddr);
	RptData[4] = PRTS(ni->NetName);
	RptData[5] = PRTS(ni->MACaddr);
	RptData[6] = PRTS(ni->MACname);
	RptData[7] = PRTS(ni->FacMACaddr);
	RptData[8] = PRTS(ni->FacMACname);
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 9);
    }
}

/*
 * Print info about a network interface
 */
extern void PrintNetIf(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    PrintGeneric(DevInfo, DevType, OffSet);

    if (!DevInfo->DevSpec)
	return;

    if (VL_CONFIG) {
	switch(FormatType) {
	case FT_PRETTY:	PrintNetIfPretty(DevInfo, DevType, OffSet);	break;
	case FT_REPORT:	PrintNetIfReport(DevInfo, DevType);		break;
	}
    }
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
    if (Old->DescList) New->DescList = Old->DescList;

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

    if (!Old)
	return(New);

    /* Bulk copy what we can */
    memcpy((char *) New, (char *) Old, sizeof(DiskPart_t));

    New->Next = NULL;

    /* Copy contents of pointers */
    if (Old->Name)	New->Name = strdup(Old->Name);
    if (Old->Usage)	New->Usage = strdup(Old->Usage);
    if (Old->Type)	New->Type = strdup(Old->Type);

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
     * Is Driver in the list of driver names which we should ignore Serial?
     */
    if (Find->Tree->Driver)
	for (cpp = IgnoreSerialList; cpp && *cpp; ++cpp)
	    if (EQ(Find->Tree->Driver, *cpp))
		return(FALSE);

    if (Find->Serial && EQ(Find->Serial, Find->Tree->Serial)) {
	DevFindAddMatch(Find, "Serial");
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
	return(FALSE);

    if (Find->Unit >= 0 && Find->Unit == Find->Tree->Unit &&
	(Find->DevType < 0 || Find->DevType == Find->Tree->Type) &&
	(Find->ClassType < 0 || Find->ClassType == Find->Tree->ClassType)) {
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
	    DevFindMatchSerial(Find) ||
	    DevFindMatchType(Find))
	    return(Tree);
    } else if (Find->Expr == DFE_AND) {
	if (DevFindMatchName(Find) &&
	    DevFindMatchID(Find) &&
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

    if (OrphanList) {
	for (Ptr = OrphanList; Ptr && Ptr->Next; Ptr = Ptr->Next);
	Ptr->Next = DevInfo;
    } else {
	OrphanList = DevInfo;
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
    DevInfo_t		       *Found;
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
	if (Found = DevFind(&Find)) {
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
	    snprintf(Buff, sizeof(Buff), "%s(%d)", Name, Unit);
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
