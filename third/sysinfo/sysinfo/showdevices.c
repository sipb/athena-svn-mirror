/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Show Devices related functions.
 */

#include "defs.h"

extern void PrintDiskdrive();
extern void PrintFrameBuffer();
extern void PrintMonitor();
extern void PrintNetIf();
extern void PrintDevice();
extern void PrintGeneric();

static float 			TotalDisk = 0;

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
    static DevInfo_t 	       *RootDev;
    static char		       *RptData[2];
    static MCSIquery_t		Query;

    (void) memset(&Query, CNULL, sizeof(Query));
    Query.Op = MCSIOP_CREATE;
    Query.Cmd = MCSI_DEVTREE;
    if (mcSysInfo(&Query) != 0) {
	return;
    }
    RootDev = (DevInfo_t *) Query.Out;

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
	    ShowLabel("Vendor", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Vendor);
	}
	if (DevInfo->Model) {
	    ShowLabel("Model", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Model);
	}
	if (DevInfo->Serial) {
	    ShowLabel("Serial", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Serial);
	}
	if (DevInfo->Ident) {
	    ShowLabel("Identifier", OffSet);
	    SImsg(SIM_INFO, " %s\n", IdentString(DevInfo->Ident));
	}
	if (DevInfo->Revision) {
	    ShowLabel("Revision", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevInfo->Revision);
	}
	if (DevInfo->Unit >= 0) {
	    ShowLabel("Unit", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Unit);
	}
	if (DevInfo->Addr >= 0) {
	    ShowLabel("Address", OffSet);
	    SImsg(SIM_INFO, " 0x%x\n", DevInfo->Addr);
	}
	if (DevInfo->Prio >= 0) {
	    ShowLabel("Priority", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Prio);
	}
	if (DevInfo->Vec >= 0) {
	    ShowLabel("Vector", OffSet);
	    SImsg(SIM_INFO, " %d\n", DevInfo->Vec);
	}
	if (DevType && DevType->Desc) {
	    ShowLabel("Device Type", OffSet);
	    SImsg(SIM_INFO, " %s\n", DevType->Desc);
	}
	if (ClassType = ClassTypeGetByType(DevInfo->Type, DevInfo->ClassType)){
	    ShowLabel("Class", OffSet);
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
    ShowOffSet(OffSet);
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
	    ShowLabel("Alias Names", OffSet);
	    for (cpp = DevInfo->Aliases; cpp && *cpp; ++cpp)
		SImsg(SIM_INFO, " %s", *cpp);
	    SImsg(SIM_INFO, "\n");
	}
	if (DevInfo->DescList)
	    for (DevDesc = DevInfo->DescList; DevDesc; 
		 DevDesc = DevDesc->Next) {
		ShowLabel(GetDescLabel(DevDesc), OffSet);
		SImsg(SIM_INFO, " %s\n", DevDesc->Desc);
	    }
	if (DevInfo->Files && DevInfo->Files[0]) {
	    ShowLabel("Device Files", OffSet);
	    for (cpp = DevInfo->Files; cpp && *cpp; ++cpp)
		SImsg(SIM_INFO, " %s", *cpp);
	    SImsg(SIM_INFO, "\n");
	}
	if (DevInfo->Master && DevInfo->Master->Name) {
	    ShowLabel("Connected to", OffSet);
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
	    ShowLabel("Attached Device(s)", OffSet);
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
    static char		       *RptData[18];
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
    if (DevInfo->Ident)
	RptData[17] = IdentString(DevInfo->Ident);

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
    register DiskPart_t        *dp;
    PartInfo_t		       *PartInfo;
    DiskPart_t		       *DiskPart;
    static char			Buff[128];
    static char			TitleDef[] = "Partition Information";
    int				SecSize = 512;

    if (!Disk || !Disk->DiskPart || !Disk->DiskPart->PartInfo)
	return;

    DiskPart = Disk->DiskPart;
    PartInfo = DiskPart->PartInfo;

    if (PartInfo->SecSize)
	SecSize = PartInfo->SecSize;
    else if (Disk->SecSize)
	SecSize = Disk->SecSize;

    SImsg(SIM_INFO, "\n");
    ShowOffSet(OffSet);
    if (PartInfo->Title)
	(void) snprintf(Buff, sizeof(Buff), "%s %s", 
			PartInfo->Title, TitleDef);
    else
	(void) snprintf(Buff, sizeof(Buff), "%s", TitleDef);
    SImsg(SIM_INFO, "%40s\n", Buff);

    ShowOffSet(OffSet);
    SImsg(SIM_INFO, "%8s %10s %10s %9s\n",
	   "", "START", "NUMBER OF", "SIZE");

    ShowOffSet(OffSet);
    SImsg(SIM_INFO, "%8s %10s %10s %9s %8.8s %s\n",
	   "PART", "SECTOR", "SECTORS", "(MB)", "TYPE", "USAGE");

    for (dp = DiskPart; dp; dp = dp->Next) {
	ShowOffSet(OffSet);
	SImsg(SIM_INFO, "%8s %10.0f %10.0f %9.2f %8.8s %s\n",
	       (dp->PartInfo->Name) ? dp->PartInfo->Name : "",
	       (float) dp->PartInfo->StartSect,
	       (float) dp->PartInfo->NumSect,
	       bytes_to_mbytes(nsect_to_bytes(dp->PartInfo->NumSect, SecSize)),
	       (dp->PartInfo->Type) ? dp->PartInfo->Type : "",
	       (dp->PartInfo->MntName) ? dp->PartInfo->MntName : ""
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
    register DiskPart_t        *dp;
    PartInfo_t		       *pi;
    static char		       *RptData[7];
    static char			StartBuff[50];
    static char			NumBuff[50];
    static char			SizeBuff[50];
    int				SecSize = 512;

    if (!Disk || !DevInfo)
	return;

    if (Disk->SecSize)
	SecSize = Disk->SecSize;

    for (dp = Disk->DiskPart; dp; dp = dp->Next) {
	pi = dp->PartInfo;
	if (pi->SecSize) 
	    SecSize = pi->SecSize;
	(void) snprintf(StartBuff, sizeof(StartBuff), "%.0f",
			(float) pi->StartSect);
	(void) snprintf(NumBuff, sizeof(NumBuff), "%.0f", (float) pi->NumSect);
	(void) snprintf(SizeBuff, sizeof(SizeBuff), "%9.2f", 
			bytes_to_mbytes(nsect_to_bytes(pi->NumSect, SecSize)));
	RptData[0] = R_PART;
	RptData[1] = PRTS(pi->Name);
	RptData[2] = StartBuff;
	RptData[3] = NumBuff;
	RptData[4] = SizeBuff;
	RptData[5] = PRTS(pi->Type);
	RptData[6] = PRTS(pi->MntName);
	Report(CN_DEVICE, R_DESC, PRTS(DevInfo->Name), RptData, 7);
    }
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
		ShowLabel("Max Horizontal Image Size (cm)", OffSet);
		SImsg(SIM_INFO, " %d\n", Mon->MaxHorSize);
	    }
	    if (Mon->MaxVerSize) {
		ShowLabel("Max Vertical Image Size (cm)", OffSet);
		SImsg(SIM_INFO, " %d\n", Mon->MaxVerSize);
	    }
	    if (Mon->Resolutions) {
		ShowLabel("Supported Resolutions", OffSet);
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
	    ShowLabel("Address Type", OffSet);
	    SImsg(SIM_INFO, " %s\n", ni->TypeName);
	}

	if (ni->HostAddr) {
	    ShowLabel("Host Address", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->HostAddr,
		   (ni->HostName) ? ni->HostName : "<unknown>");
	}

	if (ni->NetAddr) {
	    ShowLabel("Network Address", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->NetAddr, 
		   (ni->NetName) ? ni->NetName : "<unknown>");
	}

	if (ni->MACaddr) {
	    ShowLabel("Current MAC Addr", OffSet);
	    SImsg(SIM_INFO, " %-18s [%s]\n", ni->MACaddr,
		   (ni->MACname && ni->MACname[0]) 
		   ? ni->MACname : "<unknown>");
	}

	if (ni->FacMACaddr) {
	    ShowLabel("Factory MAC Addr", OffSet);
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
