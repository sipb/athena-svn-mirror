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
 * Functions common to various BSD OS's
 */

#include "defs.h"
#include <fstab.h>
#include <sys/mtio.h>
#define DKTYPENAMES			/* Get stuff from disklabel.h */
#include <sys/disklabel.h>
#undef DKTYPENAMES

/*
 * Probe a Tape Drive.
 */
extern DevInfo_t *ProbeTapeDrive(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t 		       *DevInfo = NULL;
    char 		       *DevName = NULL;
    char 		       *Model = NULL;
    char		      **FileList;
    int				FileNum;
    static char			DevFile[128];
    static char			CtlFile[128];
    struct mtget 		MtGet;
#if	defined(MTIOGNAME)
    static struct mt_name	MtName;
#endif	/* MTIOGNAME */
    int				Finished = FALSE;
    register char	       *cp;
    int 			fd;
    DevDesc_t		       *ScsiDevDesc;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
    Define_t		       *Define;

    DevName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    if (DevData && DevData->OSDevInfo)
	DevInfo = DevData->OSDevInfo;

    /*
     * DevFile is used for MTIO* ioctl's (mtio)
     * CtlFile is used for SCIO* ioctl's (SCSI)
     */
    (void) snprintf(DevFile, sizeof(DevFile), "%s/nr%s", 
		    _PATH_DEV, DevName);
    (void) snprintf(CtlFile, sizeof(CtlFile), "%s/r%s.ctl", 
		    _PATH_DEV, DevName);
    /*
     * Not all OS's have CtlFile so if it doesn't exist, use DevFile
     */
    if (!FileExists(CtlFile))
	(void) strcpy(CtlFile, DevFile);

    /*
     * Need to open RDWR for MTIOCGET
     */
    fd = open(DevFile, O_RDWR|O_NONBLOCK|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s Cannot open for read/write: %s.", DevFile, SYSERR);
	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if (errno != ENOENT)
	    if (!DevInfo)
		DevInfo = DeviceCreate(ProbeData);
    } else {
#if	defined(MTIOGNAME)
	if (ioctl(fd, MTIOGNAME, &MtName) != 0) {
	    SImsg(SIM_GERR, "%s: ioctl MTIOGNAME failed: %s.", 
		  DevFile, SYSERR);
	} else {
	    SImsg(SIM_DBG, 
		  "\t%s: MTIOGNAME: Vendor=<%s> Product=<%s> Rev=<%s>",
		  DevName, MtName.mn_vendor, MtName.mn_product, 
		  MtName.mn_revision);

	    /*
	     * Set device info
	     */
	    if (!DevInfo)
		DevInfo = DeviceCreate(ProbeData);
	    DevAddFile(DevInfo, strdup(DevFile));
#define OK(s)	( s && s[0] && isalpha(s[0]) )
	    if (OK(MtName.mn_product))
		DevInfo->Model 		= strdup(MtName.mn_product);
	    else if (DevDefine && DevDefine->Model)
		DevInfo->Model 		= DevDefine->Model;
	    if (OK(MtName.mn_vendor))
		DevInfo->Vendor		= strdup(MtName.mn_vendor);
	    if (OK(MtName.mn_revision))
		DevInfo->Revision	= strdup(MtName.mn_revision);
#undef	OK
	    DevInfo->Master 		= MkMasterFromDevData(DevData);
	    Finished = TRUE;
	}
#endif	/* MTIOGNAME */
	if (!Finished) {
	    if (ioctl(fd, MTIOCGET, &MtGet) != 0) {
		SImsg(SIM_GERR, "%s: Cannot extract tape status: %s.", 
		      DevFile, SYSERR);
	    } else {
		SImsg(SIM_DBG, "\t%s: MTIO: type=0x%x dsreg=0x%x", 
		      DevName, MtGet.mt_type, MtGet.mt_dsreg);

		if (Define = DefGet("TapeDriveTypes", NULL, MtGet.mt_type, 0))
		    Model = Define->ValStr1;

		/*
		 * Set device info
		 */
		if (!DevInfo)
		    DevInfo = DeviceCreate(ProbeData);
		DevAddFile(DevInfo, strdup(DevFile));
		if (Model)
		    DevInfo->Model 		= Model;
		else if (DevDefine && DevDefine->Model)
		    DevInfo->Model 		= DevDefine->Model;
		DevInfo->Master 		= MkMasterFromDevData(DevData);
	    }
	}

	(void) close(fd);
    }

    /*
     * Use the special control file for SCSI queries
     */
    fd = open(CtlFile, O_RDWR|O_NDELAY);
    if (fd < 0)
	SImsg(SIM_GERR, "%s: Open for read/write failed: %s", CtlFile, SYSERR);
    else {
	if (!DevInfo)
	    DevInfo = DeviceCreate(ProbeData);
	DevAddFile(DevInfo, strdup(CtlFile));

	(void) ScsiQuery(DevInfo, CtlFile, fd, TRUE);

	(void) close(fd);
    }

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Create a base DiskDrive device.
 */
static DevInfo_t *CreateBaseDiskDrive(ProbeData, DiskName)
     ProbeData_t	       *ProbeData;
     char		       *DiskName;
{
    DevInfo_t		       *DevInfo;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
    char		       *DevName;
    char		       *AltName = NULL;

    DevName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    if (DevData && DevData->OSDevInfo)
	DevInfo = DevData->OSDevInfo;
    else {
	if (DiskName)
	    ProbeData->DevName = strdup(DiskName);
	else
	    ProbeData->DevName = strdup(DevName);
	DevInfo = DeviceCreate(ProbeData);
    }

    if (DevData && DevData->DevName)
	AltName = MkDevName(DevData->DevName, DevData->DevUnit,
			    (DevDefine) ? DevDefine->Type : 0, 
			    (DevDefine) ? DevDefine->Flags : 0);
    else
	AltName = strdup(DiskName);

    if (EQ(DevInfo->Name, AltName))
	(void) free(AltName);
    else
	DevInfo->AltName = AltName;

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Set mount point for a given device by looking it up in fstab
 */
static int SetMountInfo(DevInfo, DiskPart, PartName)
     DevInfo_t		       *DevInfo;
     DiskPart_t		       *DiskPart;
     char		       *PartName;
{
    static char			DevFile[sizeof(_PATH_DEV) + 24];
    struct fstab	       *Fstab;

    (void) setfsent();
    (void) snprintf(DevFile, sizeof(DevFile), "%s/%s%s", 
		    _PATH_DEV, DevInfo->Name, PartName);
    if (Fstab = getfsspec(DevFile)) {
	DiskPart->Usage = strdup(Fstab->fs_file);
	DiskPart->Type = strdup(Fstab->fs_vfstype);
	return(0);
    }

    return(-1);
}

/*
 * Get the Disk Partition information from DiskLabel
 */
static DiskPart_t *GetDiskPart(DevInfo, DiskLabel)
     DevInfo_t		       *DevInfo;
     struct disklabel	       *DiskLabel;
{
    register int		i;
    DiskPart_t 		       *Base = NULL;
    DiskPart_t		       *DiskPart;
    register DiskPart_t	       *pdp;
    static char			Name[2];

    for (i = 0; i < DiskLabel->d_npartitions; ++i) {
	/* Skip empty partitions */
	if (DiskLabel->d_partitions[i].p_size <= 0)
	    continue;
	DiskPart = (DiskPart_t *) xcalloc(1, sizeof(DiskPart_t));

	snprintf(Name, sizeof(Name),  "%c", 'a' + i);
	DiskPart->Name = strdup(Name);

	if (SetMountInfo(DevInfo, DiskPart, Name) != 0)
	    /* We didn't find an fstab entry, so set the type */
	    DiskPart->Type = fstypenames[DiskLabel->d_partitions[i].p_fstype];

	DiskPart->StartSect = 
	    (Large_t) DiskLabel->d_partitions[i].p_offset;
	DiskPart->NumSect = 
	    (Large_t) DiskLabel->d_partitions[i].p_size;

	if (Base) {
	    for (pdp = Base; pdp && pdp->Next; pdp = pdp->Next);
	    pdp->Next = DiskPart;
	} else
	    Base = DiskPart;
    }

    return(Base);
}

/*
 * Set DiskDrive information based on "disklabel" information.
 */
static int SetDiskLabel(DevInfo, DiskDriveData, DiskLabel)
     DevInfo_t		       *DevInfo;
     DiskDriveData_t	       *DiskDriveData;
     struct disklabel	       *DiskLabel;
{
    DiskDrive_t		       *DiskDrive = NULL;
    Define_t		       *Define;

    if (!DevInfo || !DiskLabel)
	return(-1);

    /*
     * Even if the label is valid, some pieces of information are
     * valid.
     */
    if (DiskLabel->d_magic != DISKMAGIC)
	SImsg(SIM_DBG, "%s: Bad disk label - using anyway.", DevInfo->Name);

    SImsg(SIM_DBG, "\t%s: DiskLabel: d_typename=<%s> type=%d subtype=%d", 
	  DevInfo->Name, 
	  isalpha(DiskLabel->d_typename[0]) ? DiskLabel->d_typename : "",
	  DiskLabel->d_type, DiskLabel->d_subtype);

    if (DiskDriveData->OSdata)
	DiskDrive = DiskDriveData->OSdata;
    else {
	DiskDrive = NewDiskDrive(NULL);
	DiskDriveData->OSdata = DiskDrive;
    }

    DiskDrive->SecSize = DiskLabel->d_secsize;
    DiskDrive->Sect = DiskLabel->d_nsectors;
    DiskDrive->Tracks = DiskLabel->d_ntracks;
    DiskDrive->DataCyl = DiskLabel->d_ncylinders;
    DiskDrive->AltCyl = DiskLabel->d_acylinders;
    DiskDrive->RPM = DiskLabel->d_rpm;
    DiskDrive->IntrLv = DiskLabel->d_interleave;

    /*
     * Check what flags are set
     */
    for (Define = DefGetList("DiskDriveFlags"); Define; Define = Define->Next)
	if (FLAGS_ON(DiskLabel->d_flags, Define->KeyNum))
	    AddDevDesc(DevInfo, Define->ValStr1, "Has", DA_APPEND);

    /*
     * Use d_typename if the first character is Alpha
     */
    if (isalpha(DiskLabel->d_typename[0]))
	DiskDrive->Label = strdup(DiskLabel->d_typename);

    /*
     * Use the Label as the Model if we haven't managed to set Model yet
     */
    if (!DevInfo->Model)
	DevInfo->Model = DiskDrive->Label;

    if (Define = DefGet("DiskDriveTypes", NULL, DiskLabel->d_type, 0))
	DevInfo->ModelDesc = Define->ValStr1;

    DiskDrive->DiskPart = GetDiskPart(DevInfo, DiskLabel);

    return(0);
}

/*
 * Probe a DiskDrive device
 */
extern DevInfo_t *ProbeDiskDrive(ProbeData)
     ProbeData_t	       *ProbeData;
{
    char		       *DevName;
    static char			DevFile[sizeof(_PATH_DEV) + 64];
    static char			CtlFile[sizeof(_PATH_DEV) + 64];
    int				fd;
    static struct disklabel	DiskLabel;
    DevInfo_t		       *DevInfo = NULL;
    DiskDriveData_t	       *DiskDriveData = NULL;

    if (!ProbeData || !(DevName = ProbeData->DevName))
	return((DevInfo_t *)NULL);

    DevInfo = CreateBaseDiskDrive(ProbeData, DevName);

    /*
     * DevFile is used for DIOC* ioctl's (disklabel)
     * CtlFile is used for SCIO* ioctl's (SCSI)
     */
    (void) snprintf(DevFile, sizeof(DevFile), "%s/r%sc", 
		    _PATH_DEV, DevName);
    (void) snprintf(CtlFile, sizeof(CtlFile), "%s/r%s.ctl", 
		    _PATH_DEV, DevName);
    /*
     * Not all OS's have CtlFile so if it doesn't exist, use DevFile
     */
    if (!FileExists(CtlFile))
	(void) strcpy(CtlFile, DevFile);

    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0)
	SImsg(SIM_GERR, "%s: Open for read failed: %s", DevFile, SYSERR);
    else {
	DevAddFile(DevInfo, strdup(DevFile));

	if (DevInfo->DevSpec)
	    DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
	else {
	    DiskDriveData = NewDiskDriveData(NULL);
	    DevInfo->DevSpec = (void *) DiskDriveData;
	}

	/*
	 * Only use DIOCGDINFO on disks (not CDROMs, etc.)
	 */
	if (DevInfo->Type == DT_DISKDRIVE) {
	    /*
	     * DIOCGDINFO is our primary source of information 
	     */
	    (void) memset(&DiskLabel, 0, sizeof(DiskLabel));
	    if (ioctl(fd, DIOCGDINFO, &DiskLabel) == 0)
		SetDiskLabel(DevInfo, DiskDriveData, &DiskLabel);
	    else
		SImsg(SIM_GERR, "%s: ioctl DIOCGDINFO failed: %s", 
		      DevFile, SYSERR);
	}

	/*
	 * Get DOS Partition info
	 */
	(void) DosPartGet(DevInfo, DevFile, fd);

	(void) close(fd);
    }

    /*
     * Use the special control file for SCSI queries
     */
    fd = open(CtlFile, O_RDWR|O_NDELAY);
    if (fd < 0)
	SImsg(SIM_GERR, "%s: Open for read/write failed: %s", CtlFile, SYSERR);
    else {
	DevAddFile(DevInfo, strdup(CtlFile));

	(void) ScsiQuery(DevInfo, CtlFile, fd, TRUE);

	(void) close(fd);
    }

    return(ProbeData->RetDevInfo = DevInfo);
}
