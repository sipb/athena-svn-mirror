/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * DOS Partition functions
 */

#include "defs.h"
#include "dospart.h"

/*
 * Read the MBR from File
 * Return < 0 on error.
 * Return 0 on success.
 * Return 1 if no partition.
 */
static int DosPartGetMBR(Mbr, MbrLen, File)
     MBR_t		       *Mbr;
     size_t			MbrLen;
     char		       *File;
{
    int				Amt;
    int				SecSize;
    int				fd;

    fd = open(File, O_RDONLY|O_NDELAY|O_NONBLOCK);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open for read failed: %s", File, SYSERR);
	return(-1);
    }

    (void) lseek(fd, DOS_BB_SECTOR, 0);

    for (SecSize = MIN_SEC_SIZE; SecSize <= MAX_SEC_SIZE; SecSize *= 2) {
	Amt = read(fd, (void *) Mbr->BootInst, SecSize);
	if (Amt == SecSize) {
	    /* Success */
	    (void) close(fd);
	    if (Mbr->Signature != DOS_BOOT_MAGIC) {
		SImsg(SIM_DBG, "%s: No DOS Partition found.", File);
		return(1);
	    }
	    return(0);
	}
    }

    SImsg(SIM_GERR, "%s: read MBR failed.", File);
    (void) close(fd);
    return(-1);
}

/*
 * Return description of what Dos Partition Type is
 */
extern char *DosPartType(Type)
     u_char			Type;
{
    Define_t		       *Def;

    if (Def = DefGet(DL_DOSPARTTYPES, NULL, (long)Type, 0))
	return(Def->ValStr1);

    SImsg(SIM_DBG, "DOS Partition Type 0x%2x (%d) is unknown.", Type, Type);

    return((char *) NULL);
}

/*
 * Read all the DOS Parts for DevInfo
 */
extern int DosPartGet(DevInfo)
     DevInfo_t		       *DevInfo;
{
    int				Status;
    static MBR_t		Mbr;
    DiskDriveData_t	       *DiskDriveData;
    DiskDrive_t		       *Disk;
    DiskPart_t		       *NewPart;
    DiskPart_t		       *Last = NULL;
    register int		i;
    static char			Buff[64];
    static char		        DevFile[128];
    char		       *File = DevFile;

    if (!DevInfo || !DevInfo->Name)
	return(-1);

    /*
     * FreeBSD uses /dev/rXXN
     * BSD/OS uses /dev/rXXNc
     * Solaris uses /dev/rdsk/cXtXdXs2
     */
    /* FreeBSD */
    (void) snprintf(DevFile, sizeof(DevFile), "%s/r%s",
		    _PATH_DEV, DevInfo->Name);
    if (!FileExists(DevFile)) {
	/* Most BSD's */
	(void) snprintf(DevFile, sizeof(DevFile), "%s/r%sc",
			_PATH_DEV, DevInfo->Name);
	if (!FileExists(DevFile))
	    /* Solaris */
	    (void) snprintf(DevFile, sizeof(DevFile), "%s/rdsk/%sp0",
			    _PATH_DEV, DevInfo->Name);

    }

    Status = DosPartGetMBR(&Mbr, sizeof(Mbr), File);
    if (Status < 0)
	return(-1);
    else if (Status > 0)
	return(0);

    if (File == DevFile)
	DevAddFile(DevInfo, strdup(File));

    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }

    if (DiskDriveData->HWdata)
	Disk = DiskDriveData->HWdata;
    else {
	Disk = NewDiskDrive(NULL);
	DiskDriveData->HWdata = Disk;
    }

    for (i = 0; i < DOS_NPARTS; ++i) {
	NewPart = NewDiskPart(NULL);
	if (i == 0)
	    NewPart->Title = "DOS";
	(void) snprintf(Buff, sizeof(Buff), "s%d", i);
	NewPart->Name = strdup(Buff);
	/*
	 * We use Type and Usage backwards here because Usage gets fully
	 * displayed on output and Type gets truncated.
	 */
	if (Mbr.Parts[i].dp_flags == DOS_ACTIVE)
	    NewPart->Type = "ACTIVE";
	NewPart->Usage = DosPartType(Mbr.Parts[i].dp_type);
	if (!NewPart->Usage) {
	    (void) snprintf(Buff, sizeof(Buff), "Unknown Type 0x%02x (%d)",
			    Mbr.Parts[i].dp_type, Mbr.Parts[i].dp_type);
	    NewPart->Usage = strdup(Buff);
	}
	NewPart->NumType = Mbr.Parts[i].dp_type;
	NewPart->StartSect = Mbr.Parts[i].dp_start;
	NewPart->NumSect = Mbr.Parts[i].dp_size;

	if (!Disk->DiskPart)
	    Disk->DiskPart = Last = NewPart;
	else {
	    Last->Next = NewPart;
	    Last = NewPart;
	}
    }

    return(0);
}
