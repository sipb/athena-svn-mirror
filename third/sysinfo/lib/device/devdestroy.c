/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Device Destroy (free memory) functions
 */

#include <stdio.h>
#include "defs.h"

/*
 * Destroy a char ** list
 */
static int DestroyCharList(List)
     char		      **List;
{
    register char	     **cpp;
    if (!List)
	return -1;

    for (cpp = List; cpp && *cpp; ++cpp)
	if (*cpp)
	    (void) free(*cpp);

    (void) free(List);

    return 0;
}

/*
 * Destroy a list of Desc_t
 */
extern int DestroyDesc(DescList)
     Desc_t		       *DescList;
{
    register Desc_t	       *dp;
    register Desc_t	       *Last;

    if (!DescList)
	return -1;

    for (dp = DescList; dp; ) {
	if (dp->Label)		(void) free(dp->Label);
	if (dp->Desc)		(void) free(dp->Desc);
	Last = dp;
	dp = dp->Next;
	(void) free(Last);
    }

    return 0;
}

/*
 * Destroy DiskDrive
 */
static int DestroyDiskDrive(Disk)
     DiskDrive_t	       *Disk;
{
    register DiskPart_t	       *dp;
    register DiskPart_t	       *Last;

    if (!Disk)
	return -1;

    /* Disk->DataType is always static */

    if (Disk->Label)	(void) free(Disk->Label);

    for (dp = Disk->DiskPart; dp; ) {
	if (dp->PartInfo)	PartInfoDestroy(dp->PartInfo);
	Last = dp;
	dp = dp->Next;
	(void) free(Last);
    }

    (void) free(Disk);

    return 0;
}

/*
 * Destroy DiskDriveData
 */
static int DestroyDiskDriveData(Data)
     DiskDriveData_t	       *Data;
{
    if (!Data)
	return -1;

    if (Data->HWdata)	DestroyDiskDrive(Data->HWdata);
    if (Data->OSdata)	DestroyDiskDrive(Data->OSdata);

    return 0;
}

/*
 * Destory a Monitor_t
 */
static int DestroyMonitor(Mon)
     Monitor_t		       *Mon;
{
    if (!Mon)
	return -1;

    if (Mon->Resolutions)
	(void) DestroyCharList(Mon->Resolutions);

    (void) free(Mon);

    return 0;
}

/*
 * Destroy a given device and all it's siblings and children.
 * --RECURSE--
 */
static int DestroyDevice(DevInfo)
     DevInfo_t		       *DevInfo;
{
    DevInfo_t		       *Ptr;

    if (!DevInfo)
	return -1;

    if (DevInfo->Name)		(void) free(DevInfo->Name);
    if (DevInfo->Driver)	(void) free(DevInfo->Driver);
    if (DevInfo->AltName)	(void) free(DevInfo->AltName);
    if (DevInfo->Aliases)	(void) DestroyCharList(DevInfo->Aliases);
    if (DevInfo->Files)		(void) DestroyCharList(DevInfo->Files);
    if (DevInfo->Vendor)	(void) free(DevInfo->Vendor);
    if (DevInfo->Model)		(void) free(DevInfo->Model);
    if (DevInfo->ModelDesc)	(void) free(DevInfo->ModelDesc);
    if (DevInfo->Serial)	(void) free(DevInfo->Serial);
    if (DevInfo->Revision)	(void) free(DevInfo->Revision);
    if (DevInfo->DescList)	(void) DestroyDesc(DevInfo->DescList);
    if (DevInfo->MasterName)	(void) free(DevInfo->MasterName);

    /* Destroy Device Specific data */
    switch (DevInfo->Type) {
    case DT_DISKDRIVE:
	(void) DestroyDiskDriveData((DiskDriveData_t *)DevInfo->DevSpec);
	break;
    case DT_MONITOR:
	(void) DestroyMonitor((Monitor_t *)DevInfo->DevSpec);
	break;
    case DT_FRAMEBUFFER:
	(void) free(DevInfo->DevSpec);
	break;
    }
    
    for (Ptr = DevInfo->Slaves; Ptr; Ptr = Ptr->Next)
	DestroyDevice(Ptr);

    for (Ptr = DevInfo->Next; Ptr; Ptr = Ptr->Next)
	DestroyDevice(Ptr);

    (void) free(DevInfo);

    return 0;
}

/*
 * Destroy (free memory) a Device Tree as created by GetDeviceTree().
 */
extern int DestroyDeviceTree(Root)
    DevInfo_t		       *Root;
{
    return DestroyDevice(Root);
}
