/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: devices.c,v 1.1.1.1 1996-10-07 20:16:48 ghudson Exp $";
#endif

/*
 * Device routines
 */

#include <stdio.h>
#include "defs.h"

extern void PrintDiskdrive();
extern void PrintFrameBuffer();
extern void PrintNetIf();
extern void PrintDevice();
extern void PrintGeneric();

static DevInfo_t 	       *RootDev = NULL;
static float 			TotalDisk = 0;
extern int			OffSetAmt;

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
	if (EQ(Ptr->Desc, Desc))
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
 * Get a nice size string
 */
extern char *GetSizeStr(Amt, Unit)
    u_long			Amt;
    u_long			Unit;
{
    static char			Buff[100];

    Buff[0] = CNULL;

    if (Unit) {
	if (Unit == GBYTES)
	    (void) sprintf(Buff, "%d GB", Amt);
	else if (Unit == MBYTES) {
	    if (Amt > KBYTES)
		(void) sprintf(Buff, "%.1f GB", (float) mbytes_to_gbytes(Amt));
	    else
		(void) sprintf(Buff, "%d MB", Amt);
	} else if (Unit == KBYTES) {
	    if (Amt > MBYTES)
		(void) sprintf(Buff, "%.1f GB", (float) kbytes_to_gbytes(Amt));
	    else if (Amt > KBYTES)
		(void) sprintf(Buff, "%d MB", (u_long) kbytes_to_mbytes(Amt));
	    else
		(void) sprintf(Buff, "%d KB", Amt);
	}
    }

    if (Buff[0] == CNULL) {
	if (Amt < KBYTES)
	    (void) sprintf(Buff, "%d Bytes", Amt);
	else if (Amt < MBYTES)
	    (void) sprintf(Buff, "%d KB", (u_long) bytes_to_kbytes(Amt));
	else if (Amt < GBYTES)
	    (void) sprintf(Buff, "%d MB", (u_long) bytes_to_mbytes(Amt));
	else
	    (void) sprintf(Buff, "%.1f GB", (float) bytes_to_gbytes(Amt));
    }

    return(Buff);
}

/*
 * List valid arguments for Devices class. 
 */
extern void DeviceList()
{
    printf(
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
	if (Debug)
	    printf("No devices were found.\n");
	return;
    }

    ClassShowLabel(MyInfo);

    TotalDisk = (float) 0;

    PrintDevice(RootDev, 0);

    if (VL_DESC && TotalDisk > (float) 0)
	switch (FormatType) {
	case FT_PRETTY:
	    printf("\nTotal Disk Capacity is %s\n", 
		   GetSizeStr((u_long) TotalDisk, MBYTES));
	    break;
	case FT_REPORT:
	    RptData[0] = GetSizeStr((u_long) TotalDisk, MBYTES);
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
	printf("%*s", cnt, "");
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
	printf("%*s%-22s:", OffSetAmt, "", Name);
    else
	printf("%*s%22s:", OffSetAmt, "", Name);
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
 * Print general device information in FT_PRETTY format
 */
static void PrintDeviceInfoPretty(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{     
    register DevDesc_t	       *DevDesc;
    DevType_t 		       *mdt;
    char		       *cp;

    if (!DevInfo->Name)
	return;

    if (VL_CONFIG) printf("\n");
    PrOffSet(OffSet);
    printf("%s", DevInfo->Name);

    if (DevInfo->AltName)
	printf(" (%s)", DevInfo->AltName);
    
    if (DevInfo->Model || DevInfo->ModelDesc ||
	(DevType && DevType->Desc)) {
	printf(" is a");
	if (DevInfo->Model)
	    printf(" \"%s\"", DevInfo->Model);

	if (DevInfo->ModelDesc)
	    printf(" %s", DevInfo->ModelDesc);

	if (DevType && DevType->Desc)
	    printf(" %s", DevType->Desc);
	else if ((cp = PrimeDesc(DevInfo)) && !EQ(cp, DevInfo->Model))
	    printf(" %s", cp);
    } else if (cp = PrimeDesc(DevInfo)) {
	printf(" is a");
	printf(" %s", cp);
    }

    if (DevInfo->Name)
	printf("\n");

    if (VL_DESC || VL_CONFIG) {
	if (DevInfo->DescList) {
	    for (DevDesc = DevInfo->DescList; DevDesc; 
		 DevDesc = DevDesc->Next) {
		PrDevLabel(GetDescLabel(DevDesc), OffSet);
		printf(" %s\n", DevDesc->Desc);
	    }
	} else if ((mdt = TypeGetByType(DevInfo->Type)) && mdt && mdt->Desc) {
	    PrDevLabel(GetDescLabel(NULL), OffSet);
	    printf(" %s\n", mdt->Desc);
	}

	if (DevInfo->Master && DevInfo->Master->Name) {
	    PrDevLabel("Connected to", OffSet);
	    if (DevInfo->Master->Name)
		printf(" %s", DevInfo->Master->Name);
	    else if (DevInfo->Master->Model) {
		printf(" %s", DevInfo->Master->Model);
		if (mdt = TypeGetByType(DevInfo->Master->Type))
		    printf(" %s", mdt->Desc);
	    }
	    printf("\n");
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
    register char	      **cpp;
    static char		       *RptData[12];
    static char			FileBuff[BUFSIZ];
    static char			UnitBuff[50];
    static char			AddrBuff[50];
    static char			PrioBuff[50];
    static char			VecBuff[50];
    static char			NodeIDBuff[50];

    if (!DevInfo->Name)
	return;

    RptData[0] = PS(DevInfo->AltName);

    if (DevInfo->Master)
	RptData[1] = PS(DevInfo->Master->Name);
    else if (DevInfo->MasterName)
	RptData[1] = PS(DevInfo->MasterName);

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
	RptData[3] = PS(DevType->Name);
	RptData[4] = PS(DevType->Desc);
    }
    RptData[5] = PS(DevInfo->Model);
    RptData[6] = PS(DevInfo->ModelDesc);

    (void) sprintf(UnitBuff, "%d", DevInfo->Unit);	RptData[7] = UnitBuff;
    (void) sprintf(AddrBuff, "0x%x", DevInfo->Addr);	RptData[8] = AddrBuff;
    (void) sprintf(PrioBuff, "%d", DevInfo->Prio);	RptData[9] = PrioBuff;
    (void) sprintf(VecBuff, "%d", DevInfo->Vec);	RptData[10] = VecBuff;
    (void) sprintf(NodeIDBuff, "%d", DevInfo->NodeID);RptData[11] = NodeIDBuff;

    Report(CN_DEVICE, R_NAME, PS(DevInfo->Name), RptData, 12);

    /*
     * Report description info
     */
    for (DevDesc = DevInfo->DescList; DevDesc; DevDesc = DevDesc->Next) {
	RptData[0] = PS(DevDesc->Label);
	RptData[1] = PS(DevDesc->Desc);
	Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
    }
}

/*
 * Print general device information
 */
static void PrintDeviceInfo(DevInfo, DevType, OffSet)
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
 * Print info about a generic device
 */
extern void PrintGeneric(DevInfo, DevType, OffSet)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
{
    register DevInfo_t 	       *pd;

    PrintDeviceInfo(DevInfo, DevType, OffSet);

    if (FormatType == FT_REPORT)
	return;

    if (VL_CONFIG) {
	if (DevInfo->Unit >= 0) {
	    PrDevLabel("Unit", OffSet);
	    printf(" %d\n", DevInfo->Unit);
	}
	if (DevInfo->Addr >= 0) {
	    PrDevLabel("Address", OffSet);
	    printf(" 0x%x\n", DevInfo->Addr);
	}
	if (DevInfo->Prio >= 0) {
	    PrDevLabel("Priority", OffSet);
	    printf(" %d\n", DevInfo->Prio);
	}
	if (DevInfo->Vec >= 0) {
	    PrDevLabel("Vector", OffSet);
	    printf(" %d\n", DevInfo->Vec);
	}
    }

    if (VL_CONFIG) {
	if (DevInfo->Slaves && (DevInfo->Name || DevInfo->Model || 
				  (DevType && DevType->Desc))) {
	    PrDevLabel("Attached Device(s)", OffSet);
	    for (pd = DevInfo->Slaves; pd; pd = pd->Next)
		printf(" %s", pd->Name);
	    printf("\n");
	}
    }
}

/*
 * Print info about disk partitioning in FT_PRETTY format.
 */
static void PrintDiskPartPretty(Disk, OffSet)
    DiskDrive_t		       *Disk;
    int 			OffSet;
{
    register DiskPart_t 	       *pp;

    printf("\n");
    PrOffSet(OffSet);
    printf("%40s\n", "Partition Information");

    PrOffSet(OffSet);
    printf("%8s %10s %10s %9s\n",
	   "", "START", "NUMBER OF", "SIZE");

    PrOffSet(OffSet);
    printf("%8s %10s %10s %9s %8.8s %s\n",
	   "PART", "SECTOR", "SECTORS", "(MB)", "TYPE", "USAGE");

    for (pp = Disk->DiskPart; pp; pp = pp->Next) {
	PrOffSet(OffSet);
	printf("%8s %10d %10d %9.2f %8.8s %s\n",
	       pp->Name,
	       pp->StartSect,
	       pp->NumSect,
	       bytes_to_mbytes(nsect_to_bytes(pp->NumSect, Disk->SecSize)),
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

    for (pp = Disk->DiskPart; pp; pp = pp->Next) {
	(void) sprintf(StartBuff, "%d", pp->StartSect);
	(void) sprintf(NumBuff, "%d", pp->NumSect);
	(void) sprintf(SizeBuff, "%d", 
		       bytes_to_mbytes(nsect_to_bytes(pp->NumSect, 
						      Disk->SecSize)));
	RptData[0] = pp->Name;
	RptData[1] = StartBuff;
	RptData[2] = NumBuff;
	RptData[3] = SizeBuff;
	RptData[4] = PS(pp->Type);
	RptData[5] = PS(pp->Usage);
	Report(CN_DEVICE, R_DESC, R_PART, RptData, 5);
    }
}

/*
 * Get the capacity of a disk drive
 */
extern char *GetDiskSize(DiskDrive)
    DiskDrive_t		       *DiskDrive;
{
    u_long			Amt = 0;

    if (DiskDrive->DataCyl && DiskDrive->Sect && DiskDrive->Heads) {
	DiskDrive->Size = (float) nsect_to_mbytes(DiskDrive->DataCyl * 
						     DiskDrive->Sect * 
						     DiskDrive->Heads, 
						     DiskDrive->SecSize);

	if (DiskDrive->Size > 0)
	    Amt = (u_long) DiskDrive->Size;
    }

    if (Amt)
	return(GetSizeStr(Amt, MBYTES));
    else
	return((char *) NULL);
}

/*
 * Print Disk Drive information in FT_PRETTY format.
 */
static void PrintDiskDrivePretty(DevInfo, DevType, OffSet, Disk, DiskSize)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    int 			OffSet;
    DiskDrive_t		       *Disk;
    char		       *DiskSize;
{
    static char			Buff[BUFSIZ];
    char 			Buff2[BUFSIZ];

    if (DiskSize) {
	AddDevDesc(DevInfo, DiskSize, "Capacity", DA_APPEND);
    }

    if (Disk->Unit >= 0) {
	if (FLAGS_ON(Disk->Flags, DF_HEXUNIT))
	    (void) sprintf(Buff, "%.3x", Disk->Unit);
	else
	    (void) sprintf(Buff, "%d", Disk->Unit);
	AddDevDesc(DevInfo, Buff, "Unit Number", DA_APPEND);
    }
    if (Disk->Slave >= 0)
	AddDevDesc(DevInfo, itoa(Disk->Slave), "Slave Number", DA_APPEND);

    if (Disk->RPM > 0)
	AddDevDesc(DevInfo, itoa(Disk->RPM), "RPM", DA_APPEND);

    if (Disk->APC > 0)
	AddDevDesc(DevInfo, itoa(Disk->APC), "APC", DA_APPEND);

    if (Disk->IntrLv > 0)
	AddDevDesc(DevInfo, itoa(Disk->IntrLv), "Interleave", DA_APPEND);

    if (Disk->PhySect > 0)
	AddDevDesc(DevInfo, itoa(Disk->PhySect), "Hard Sectors", DA_APPEND);

    if (Disk->PROMRev > 0)
	AddDevDesc(DevInfo, itoa(Disk->PROMRev), "PROM Revision", DA_APPEND);

    if (Disk->Heads > 0)
	AddDevDesc(DevInfo, itoa(Disk->Heads), "Heads", DA_APPEND);

    if (Disk->Sect > 0)
	AddDevDesc(DevInfo, itoa(Disk->Sect), "Sectors/Track", DA_APPEND);

    if (Disk->PhyCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->PhyCyl), "Physical Cylinders", 
		   DA_APPEND);

    if (Disk->DataCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->DataCyl), "Data Cylinders", DA_APPEND);

    if (Disk->AltCyl > 0)
	AddDevDesc(DevInfo, itoa(Disk->AltCyl), "Alt Cylinders", DA_APPEND);
}

/*
 * Print Disk Drive information in FT_REPORT format.
 *
 * Output format is:
 *
 *	device|desc|$Name|$Label|$Desc|$Flags
 */
static void PrintDiskDriveReport(DevInfo, DevType, Disk, DiskSize)
    DevInfo_t 		       *DevInfo;
    DevType_t 		       *DevType;
    DiskDrive_t		       *Disk;
    char		       *DiskSize;
{
    static char		       *RptData[3];
    static char			Buff[50];

    if (strlen(DiskSize) < sizeof(RptData[0])) {
	(void) strcpy(Buff, PS(DiskSize));
	RptData[0] = "capacity";
	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
    }

    if (FLAGS_ON(Disk->Flags, DF_HEXUNIT))
	(void) sprintf(Buff, "%.3x", Disk->Unit);
    else
	(void) sprintf(Buff, "%d", Disk->Unit);
    RptData[0] = "unit";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->Slave);
    RptData[0] = "slave";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->APC);
    RptData[0] = "apc";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->IntrLv);
    RptData[0] = "interleave";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->Heads);
    RptData[0] = "heads";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->Sect);
    RptData[0] = "sectors";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->PhyCyl);
    RptData[0] = "phycyl";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->DataCyl);
    RptData[0] = "datacyl";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    (void) sprintf(Buff, "%d", Disk->AltCyl);
    RptData[0] = "altcyl";	RptData[1] = Buff;
    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

    if (Disk->PhySect) {
	(void) sprintf(Buff, "%d", Disk->PhySect);
	RptData[0] = "hardsectors";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
    }
    if (Disk->PROMRev) {
	(void) sprintf(Buff, "%d", Disk->PROMRev);
	RptData[0] = "promrev";	RptData[1] = Buff;
	Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
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
    DiskDrive_t		       *Disk = NULL;
    char		       *DiskSize = NULL;

    if (DevInfo && DevInfo->DevSpec)
	Disk = (DiskDrive_t *) DevInfo->DevSpec;

    if (Disk && !DevInfo->ModelDesc && (DiskSize = GetDiskSize(Disk)))
	DevInfo->ModelDesc = DiskSize;

    if (Disk) {
	TotalDisk += Disk->Size;

	if (VL_CONFIG) {
	    switch (FormatType) {
	    case FT_PRETTY: 
		PrintDiskDrivePretty(DevInfo, DevType, OffSet, Disk, DiskSize);
		break;
	    case FT_REPORT:	
		PrintDiskDriveReport(DevInfo, DevType, Disk, DiskSize);
		break;
	    }
	}
    }

    PrintDeviceInfo(DevInfo, DevType, OffSet);

    if (VL_ALL && Disk && Disk->DiskPart)
	switch (FormatType) {
	case FT_PRETTY: 
	    PrintDiskPartPretty(Disk, OffSet);
	    break;
	case FT_REPORT:	
	    PrintDiskPartReport(Disk, DevInfo);
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
    static char			Buff[BUFSIZ];
    static char		       *RptData[3];

    if (DevInfo && DevInfo->DevSpec)
	fb = (FrameBuffer_t *) DevInfo->DevSpec;

    if (fb && VL_CONFIG) {
	if (FormatType == FT_PRETTY) {
	    if (fb->CMSize)
		AddDevDesc(DevInfo, itoa(fb->CMSize), "Color Map Size",
			   DA_INSERT);
	    if (fb->Size)
		AddDevDesc(DevInfo, itoa((u_long)bytes_to_kbytes(fb->Size)),
			   "Size (KB)", DA_INSERT);
	    if (fb->VMSize)
		AddDevDesc(DevInfo, itoa((u_long)bytes_to_kbytes(fb->VMSize)),
			   "Video Memory (KB)", DA_INSERT);
	    if (fb->Height)
		AddDevDesc(DevInfo, itoa(fb->Height), "Height", DA_APPEND);
	    if (fb->Width)
		AddDevDesc(DevInfo, itoa(fb->Width), "Width", DA_APPEND);
	    if (fb->Depth)
		AddDevDesc(DevInfo, itoa(fb->Depth), "Depth (bits)", 
			   DA_APPEND);
	} else if (FormatType == FT_REPORT) {
	    if (fb->CMSize) {
		(void) sprintf(Buff, "%d", fb->CMSize);
		RptData[0] = "colormapsize";	RptData[1] = Buff;
		Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
	    }
		   
	    if (fb->VMSize) {
		(void) sprintf(Buff, "%d", fb->VMSize);
		RptData[0] = "videomemsize";	RptData[1] = Buff;
		Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
	    }

	    (void) sprintf(Buff, "%d", fb->Size);
	    RptData[0] = "videomemsize";	RptData[1] = Buff;
	    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

	    (void) sprintf(Buff, "%d", fb->Height);
	    RptData[0] = "height";	RptData[1] = Buff;
	    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

	    (void) sprintf(Buff, "%d", fb->Width);
	    RptData[0] = "width";	RptData[1] = Buff;
	    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);

	    (void) sprintf(Buff, "%d", fb->Depth);
	    RptData[0] = "depth";	RptData[1] = Buff;
	    Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 2);
	}
    }

    PrintDeviceInfo(DevInfo, DevType, OffSet);
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
	    printf(" %s\n", ni->TypeName);
	}

	if (ni->HostAddr) {
	    PrDevLabel("Host Address", OffSet);
	    printf(" %-18s [%s]\n", ni->HostAddr,
		   (ni->HostName) ? ni->HostName : "<unknown>");
	}

	if (ni->NetAddr) {
	    PrDevLabel("Network Address", OffSet);
	    printf(" %-18s [%s]\n", ni->NetAddr, 
		   (ni->NetName) ? ni->NetName : "<unknown>");
	}

	if (ni->MACaddr) {
	    PrDevLabel("Current MAC Addr", OffSet);
	    printf(" %-18s [%s]\n", ni->MACaddr,
		   (ni->MACname && ni->MACname[0]) 
		   ? ni->MACname : "<unknown>");
	}

	if (ni->FacMACaddr) {
	    PrDevLabel("Factory MAC Addr", OffSet);
	    printf(" %-18s [%s]\n", ni->FacMACaddr,
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
	RptData[0] = PS(ni->TypeName);
	RptData[1] = PS(ni->HostAddr);
	RptData[2] = PS(ni->HostName);
	RptData[3] = PS(ni->NetAddr);
	RptData[4] = PS(ni->NetName);
	RptData[5] = PS(ni->MACaddr);
	RptData[6] = PS(ni->MACname);
	RptData[7] = PS(ni->FacMACaddr);
	RptData[8] = PS(ni->FacMACname);
	Report(CN_DEVICE, R_DESC, PS(DevInfo->Name), RptData, 9);
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
    PrintDeviceInfo(DevInfo, DevType, OffSet);

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
 * --RECURSE--
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

    New->Next = NULL;

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
 * --RECURSE--
 * Find device named "name" in tree "treeptr".
 * This function recursively calls itself looking for 
 * the device "name".
 */
extern DevInfo_t *FindDeviceByName(Name, TreePtr)
    char 		       *Name;
    DevInfo_t 		       *TreePtr;
{
    DevInfo_t 		       *Ptr;

    if (!Name || !TreePtr)
	return((DevInfo_t *) NULL);

    if (TreePtr->Name && Name && EQ(TreePtr->Name, Name))
	return(TreePtr);

    if (TreePtr->Slaves)
	if (Ptr = FindDeviceByName(Name, TreePtr->Slaves))
	    return(Ptr);

    if (TreePtr->Next)
	if (Ptr = FindDeviceByName(Name, TreePtr->Next))
	    return(Ptr);

    return((DevInfo_t *) NULL);
}

/*
 * --RECURSE--
 * Find device with node id of "NodeID" in tree "treeptr".
 * This function recursively calls itself.
 */
extern DevInfo_t *FindDeviceByNodeID(NodeID, TreePtr)
    OBPnodeid_t			NodeID;
    DevInfo_t 		       *TreePtr;
{
    DevInfo_t 		       *Ptr;

    if (NodeID == 0 || !TreePtr)
	return((DevInfo_t *) NULL);

    if (TreePtr->NodeID && TreePtr->NodeID == NodeID)
	return(TreePtr);

    if (TreePtr->Slaves)
	if (Ptr = FindDeviceByNodeID(NodeID, TreePtr->Slaves))
	    return(Ptr);

    if (TreePtr->Next)
	if (Ptr = FindDeviceByNodeID(NodeID, TreePtr->Next))
	    return(Ptr);

    return((DevInfo_t *) NULL);
}

/*
 * --RECURSE--
 * Find a device of type DevType and Unit.
 */
extern DevInfo_t *FindDeviceByType(Type, ClassType, Unit, TreePtr)
    int				Type;
    int				ClassType;
    int				Unit;
    DevInfo_t		       *TreePtr;
{
    DevInfo_t		       *Ptr;

    if (!TreePtr)
	return((DevInfo_t *)NULL);

    if ((Type < 0 || TreePtr->Type == Type) && 
	(ClassType < 0 || TreePtr->ClassType == ClassType) &&
	TreePtr->Unit == Unit)
	return(TreePtr);

    if (TreePtr->Slaves)
	if (Ptr = FindDeviceByType(Type, ClassType, Unit, TreePtr->Slaves))
	    return(Ptr);

    if (TreePtr->Next)
	if (Ptr = FindDeviceByType(Type, ClassType, Unit, TreePtr->Next))
	    return(Ptr);

    return((DevInfo_t *)NULL);
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
    CHECK(Dev1->Model, 		Dev2->Model);
    CHECK(Dev1->ModelDesc, 	Dev2->ModelDesc);
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
 * --RECURSE--
 * Add a device to a device list.
 */
extern int AddDevice(DevInfo, TreePtr, SearchNames)
    DevInfo_t 		       *DevInfo;
    DevInfo_t 		      **TreePtr;
    char		      **SearchNames;
{
    register DevInfo_t 	       *master = NULL, *mp = NULL;
    DevInfo_t		       *fdev;
    int				Okay;

    if (!DevInfo || !TreePtr) {
	Error("Invalid parameter passed to AddDevice()");
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
	    if (Debug) Error("AddDevice: Device `%s' not in search list.",
			     DevInfo->Name);
	    return(0);	/* Lie */
	}
    }

    /*
     * Make sure device hasn't already been added
     */
    if ((fdev = FindDeviceByName(DevInfo->Name, *TreePtr)) ||
	(fdev = FindDeviceByNodeID(DevInfo->NodeID, *TreePtr))) {
	if (Debug) 
	    printf("AddDevice: Device '%s' already exists; master = '%s'\n",
		   DevInfo->Name, (master) ? master->Name : "?");
	/*
	 * Assume that the new device has more/better info than the
	 * existing device.
	 */
	if (DevInfo->Model)
	    fdev->Model = DevInfo->Model;
	if (DevInfo->ModelDesc)
	    fdev->ModelDesc = DevInfo->ModelDesc;
	if (DevInfo->DescList)
	    fdev->DescList = DevInfo->DescList;
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
	if (*TreePtr) {
	    master = FindDeviceByNodeID(DevInfo->Master->NodeID,*TreePtr);
	    if (!master)
		master = FindDeviceByName(DevInfo->Master->Name,*TreePtr);
	    if (master && EQ(master->Name, DevInfo->Master->Name))
		/* Check and fix any differences in info between master's */
		CheckDevice(master, DevInfo->Master);
	} else
	    master = NULL;
	if (!master) {
	    if (AddDevice(DevInfo->Master, TreePtr, SearchNames) != 0) {
		if (Debug) Error("Cannot add master '%s' to device tree.", 
				 DevInfo->Name);
		return(-1);
	    }
	    master = DevInfo->Master;
	}
    } else {
	if (!*TreePtr)
	    *TreePtr = NewDevInfo((DevInfo_t *)NULL);
	master = *TreePtr;
    }

    if (master->Name)
	master->Name = strdup(master->Name);

    if (master->Slaves) {
	/* Add to existing list of slaves */
	for (mp = master->Slaves; mp && mp->Next; mp = mp->Next);
	mp->Next = DevInfo;
    } else
	/* Create first slave */
	master->Slaves = DevInfo;

    return(0);
}

/*
 * Create a device entry for a generic device
 */
extern DevInfo_t *ProbeGeneric(Name, DevData, DevDefine)
     /*ARGSUSED*/
    char 		       *Name;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
    DevInfo_t		       *DevInfo;

    /*
     * DT_GENERIC devices MUST be marked alive to proceed
     */
    if (DevDefine && DevDefine->Type == DT_GENERIC && 
	!(FLAGS_ON(DevData->Flags, DD_IS_ALIVE) ||
	  FLAGS_ON(DevDefine->Flags, DDT_ZOMBIE) ||
	  FLAGS_ON(DevData->Flags, DD_MAYBE_ALIVE)))
	return((DevInfo_t *) NULL);

    DevInfo = NewDevInfo((DevInfo_t *) NULL);
    if (Name)
	DevInfo->Name = strdup(Name);
    else if (DevDefine)
	DevInfo->Name = strdup(MkDevName(DevData->DevName, 
					   DevData->DevUnit,
					   DevDefine->Type,
					   DevDefine->Flags));
    else 
	DevInfo->Name = DevData->DevName;

    if (DevDefine) {
	DevInfo->Type = DevDefine->Type;
	DevInfo->Model = DevDefine->Model;
	if (DevDefine->Desc)
	    DevInfo->ModelDesc = DevDefine->Desc;
    }
    DevInfo->Unit = DevData->DevUnit;
    DevInfo->NodeID = DevData->NodeID;
    DevInfo->Master = MkMasterFromDevData(DevData);

    return(DevInfo);
}

/*
 * Search for and call an appropriate probe function for this 
 * device
 */
extern DevInfo_t *ProbeDevice(DevData, TreePtr, Search)
    DevData_t 		       *DevData;
    DevInfo_t 		      **TreePtr;
    char		      **Search;
{
    register DevDefine_t       *DevDefine;
    register char 	       *Name = NULL;
    DevInfo_t 		       *ProbeUnknown();
    char		       *DevName = NULL;

    DevName = DevData->DevName;

    if (DevDefine = DevDefGet(DevData->DevName, 0, 0)) {
	/*
	 * Automatically assign ProbeGeneric as probe function if
	 * none is already set.
	 */
	if ((*DevDefine->Probe) == NULL)
	    DevDefine->Probe = ProbeGeneric;

	/*
	 * Use the canonical device name whenever possible
	 */
	if (DevDefine->Name)
	    DevName = DevDefine->Name;

	Name = MkDevName(DevName, DevData->DevUnit,
			 DevDefine->Type, DevDefine->Flags);
#ifdef notdef
	if (Search && !SearchCheck(Name, Search)) {
	    if (Debug) printf("ProbeDevice: `%s' not in search list.\n", Name);
	    return((DevInfo_t *) NULL);
	}
#endif
	if (FindDeviceByName(Name, *TreePtr)) {
	    if (Debug) printf("ProbeDevice: `%s' already exists.\n", Name);
	    return((DevInfo_t *) NULL);
	}
	return((*DevDefine->Probe)(Name, DevData, DevDefine));
    } else if (FLAGS_ON(DevData->Flags, DD_IS_ROOT))
	return(ProbeGeneric(DevName, DevData, (DevDefine_t *) NULL));

    /*
     * The device is unknown to us.  If it's definetly alive,
     * return a minimal device entry for it.  If it's not alive,
     * ignore it.
     */
    if (DoPrintUnknown && DevName && 
	FLAGS_ON(DevData->Flags, DD_IS_ALIVE))
	return(ProbeUnknown(DevName, DevData));

    if (Debug)
	printf("ProbeDevice: `%s' is not defined.\n", ARG(DevName));

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
    static char 		rfile[BUFSIZ];

    if (!Name)
	return((char *) NULL);

    (void) sprintf(rfile, "/dev/r%s%s", Name, (Part) ? Part : "");

    return(rfile);
}

/*
 * Make the file name of the character device
 */
extern char *GetCharFile(Name, Part)
    char 		       *Name;
    char 		       *Part;
{
    static char 		file[BUFSIZ];

    if (!Name)
	return((char *) NULL);

    (void) sprintf(file, "/dev/%s%s", Name, (Part) ? Part : "");

    return(file);
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
    static char			Buf[BUFSIZ];

    /*
     * Don't attach unit number if this is a pseudo device.
     */
    if (Unit < 0 || Type == DT_PSEUDO || Type == DT_NONE || 
	(DdtFlags & DDT_NOUNIT))
	sprintf(Buf, "%s", Name);
    else
	sprintf(Buf, "%s%d", Name, Unit);

    return(strdup(Buf));
}

/*
 * Create a minimal device type for an unknown device.
 */
extern DevInfo_t *ProbeUnknown(Name, DevData)
    /*ARGSUSED*/
    char 		       *Name;
    DevData_t 		       *DevData;
{
    DevInfo_t		       *DevInfo;

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
    static char			buff[BUFSIZ];

    if (freq > MHERTZ)
	(void) sprintf(buff, "%d MHz", freq / MHERTZ);
    else
	(void) sprintf(buff, "%d Hz", freq);

    return(buff);
}
