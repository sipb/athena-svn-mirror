/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Common SysInfo Disk functions
 */

#include "defs.h"

/*
 * Do setup for *Decode() funcs which need a DiskDrive_t to work on
 */
extern DiskDrive_t *DiskSetup(DevInfo, What)
     DevInfo_t		       *DevInfo;
     char		       *What;
{
    DiskDriveData_t	       *DiskDriveData;

    if (!DevInfo || !What)
	return((DiskDrive_t *) NULL);

    /*
     * We may want to add other DevType's as we discover them, but
     * right now we only know how to deal with DiskDrive's.
     */
    if (DevInfo->Type != DT_DISKDRIVE) {
	SImsg(SIM_DBG, "%s: %s unsupported for DevType %d",
	      DevInfo->Name, What, DevInfo->Type);
	return((DiskDrive_t *) NULL);
    }

    /*
     * Find or create the DiskDriveData and DiskDrive we need
     */
    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }
    if (!DiskDriveData->HWdata)
	DiskDriveData->HWdata = NewDiskDrive(NULL);

    return(DiskDriveData->HWdata);
}
