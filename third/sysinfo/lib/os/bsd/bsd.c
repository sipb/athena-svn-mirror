/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * Functions common to various BSD OS's
 */

#include "defs.h"
#include "dospart.h"
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
	if (errno != ENOENT && 	/* No such device */
	    errno != ENXIO)	/* Device not configured */
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
 * Get the amount of used space for this partition
 */
#include <sys/mount.h>
static int GetPartAmtUsed(Part)
     PartInfo_t		       *Part;
{
    static struct statfs	Statfs;

    if (!Part || Part->Usage != PIU_FILESYS)
	return -1;

    if (statfs(Part->MntName, &Statfs) != 0) {
	SImsg(SIM_DBG, "%s: statfs failed: %s", Part->MntName, SYSERR);
	return -1;
    }

    Part->AmtUsed = (Large_t) ((Statfs.f_blocks - Statfs.f_bfree) * 
			       Statfs.f_bsize);

    return 0;
}

/*
 * Get partition usage for a given device by looking it up in fstab
 */
static int GetPartUsage(PartInfo)
     PartInfo_t		       *PartInfo;
{
    struct fstab	       *Fstab;
    static char		      **Argv;

    (void) setfsent();

    if (Fstab = getfsspec(PartInfo->DevPath)) {
	if (Fstab->fs_file && Fstab->fs_file[0] == '/')
	    PartInfo->MntName = strdup(Fstab->fs_file);
	if (Fstab->fs_vfstype) {
	    PartInfo->Type = strdup(Fstab->fs_vfstype);
	    if (EQ(Fstab->fs_vfstype, "swap"))
		PartInfo->Usage = PIU_SWAP;
	    else
		PartInfo->Usage = PIU_FILESYS;
	}
	if (Fstab->fs_mntops)	
	    if (StrToArgv(Fstab->fs_mntops, ",", &Argv, NULL, 0) > 0)
		PartInfo->MntOpts = Argv;
	return(0);
    }

    return(-1);
}

/*
 * Get the Disk Partition information from DiskLabel
 */
static int GetDiskPart(DiskLabel, DevInfo, DiskDrive, BasePath)
     struct disklabel	       *DiskLabel;
     DevInfo_t		       *DevInfo;
     DiskDrive_t	       *DiskDrive;
     char		       *BasePath;
{
    static char			DevFile[MAXPATHLEN];
    register int		i;
    register int		n;
    DiskPart_t 		       *Base = NULL;
    DiskPart_t		       *Last = NULL;
    DiskPart_t		       *New;
    PartInfo_t		       *PartInfo;
    register DiskPart_t	       *pdp;
    static char			Name[2];

    for (i = 0; i < DiskLabel->d_npartitions; ++i) {
	/* Skip empty partitions */
	if (DiskLabel->d_partitions[i].p_size <= 0)
	    continue;

	/*
	 * Try to find the proper device file
	 */
	snprintf(Name, sizeof(Name),  "%c", 'a' + i);
	(void) snprintf(DevFile, sizeof(DevFile), "%s%s", 
			BasePath, Name);
	if (!FileExists(DevFile)) {
	    (void) snprintf(DevFile, sizeof(DevFile), "%s%d", 
			    BasePath, i);
	    if (!FileExists(DevFile)) {
		(void) snprintf(DevFile, sizeof(DevFile), "%s/%s%s", 
				_PATH_DEV, DevInfo->Name, Name);
		if (!FileExists(DevFile)) {
		    continue;
		}
	    }
	}

	PartInfo = PartInfoCreate(NULL);
	PartInfo->Name = strdup(Name);
	PartInfo->DevPath = strdup(DevFile);

	(void) snprintf(DevFile, sizeof(DevFile), "%s/r%s",
			DirName(PartInfo->DevPath), 
			BaseName(PartInfo->DevPath));

	PartInfo->DevPathRaw = strdup(DevFile);

	PartInfo->Num = i;
	PartInfo->StartSect = 
	    (Large_t) DiskLabel->d_partitions[i].p_offset;
	PartInfo->NumSect = 
	    (Large_t) DiskLabel->d_partitions[i].p_size;
	if (DiskLabel->d_secsize)
	    PartInfo->SecSize = DiskLabel->d_secsize;
	else if (DiskDrive->SecSize)
	    PartInfo->SecSize = DiskDrive->SecSize;
	else
	    PartInfo->SecSize = SECSIZE_DEFAULT;

	switch (DiskLabel->d_partitions[i].p_fstype) {
#if	defined(FS_BSDFFS)
	case FS_BSDFFS:
#endif	/* FS_BSDFFS */
#if	defined(FS_BSDLFS)
	case FS_BSDLFS:
#endif	/* FS_BSDLFS */
	    PartInfo->TypeNum = PIU_FILESYS;
	    break;
#if	defined(FS_SWAP)
	case FS_SWAP:
	    PartInfo->TypeNum = PIU_SWAP;
	    break;
#endif	/* FS_SWAP */
	}

	if (GetPartUsage(PartInfo) != 0) {
	    /* We didn't find an fstab entry, so set the type */
	    n = DiskLabel->d_partitions[i].p_fstype;
	    if (n >= 0 && n <= FSMAXTYPES)
		PartInfo->Type = strdup(fstypenames[n]);
	}
	(void) GetPartAmtUsed(PartInfo);

	New = NewDiskPart(NULL);
	New->PartInfo = PartInfo;
	if (Last) {
	    Last->Next = New;
	    Last = New;
	} else
	    Base = Last = New;
    }

    if (DiskDrive->DiskPart)
	DiskDrive->DiskPart->Next = Base;
    else
	DiskDrive->DiskPart = Base;

    return 0;
}

/*
 * Set DiskDrive information based on "disklabel" information.
 */
static int SetDiskLabel(DevInfo, DiskDriveData, DevFile, DiskLabel)
     DevInfo_t		       *DevInfo;
     DiskDriveData_t	       *DiskDriveData;
     char		       *DevFile;
     struct disklabel	       *DiskLabel;
{
    DiskDrive_t		       *DiskDrive = NULL;
    Define_t		       *Define;
    static char		        BasePath[256];
    static DosPartQuery_t	Query;

    if (!DevInfo || !DiskLabel)
	return -1;

    (void) memset(&Query, 0, sizeof(Query));
    (void) snprintf(BasePath, sizeof(BasePath), "%s/%s",
		    DirName(DevFile), DevInfo->Name);
    Query.BasePath = BasePath;
    Query.DevInfo = DevInfo;
    Query.CtlDev = DevFile;
    
    if (DiskLabel->d_magic != DISKMAGIC) {
	SImsg(SIM_DBG, "%s: Disk label magic wrong/missing.", DevInfo->Name);
	/*
	 * Try to get the DOS partition info anyway which is usually seperate.
	 */
	(void) DosPartGet(&Query);
	return -1;
    }

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

    DiskDrive->SecSize = (int) DiskLabel->d_secsize;
    DiskDrive->Sect = (u_long) DiskLabel->d_nsectors;
    DiskDrive->Tracks = (u_long) DiskLabel->d_ntracks;
    DiskDrive->DataCyl = (u_long) DiskLabel->d_ncylinders;
    DiskDrive->AltCyl = (u_long) DiskLabel->d_acylinders;
    DiskDrive->RPM = (u_long) DiskLabel->d_rpm;
    DiskDrive->IntrLv = (u_long) DiskLabel->d_interleave;

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
	DiskDrive->Label = CleanString(DiskLabel->d_typename,
				       sizeof(DiskLabel->d_typename), 0);

    /*
     * Use the Label as the Model if we haven't managed to set Model yet
     */
    if (!DevInfo->Model)
	DevInfo->Model = DiskDrive->Label;

    if (Define = DefGet("DiskDriveTypes", NULL, DiskLabel->d_type, 0))
	DevInfo->ModelDesc = Define->ValStr1;

    /*
     * See if the OS gave us the DOS slice name.
     */
    if (eqn(DiskLabel->d_typename, DevInfo->Name, strlen(DevInfo->Name)) &&
	strlen(DiskLabel->d_typename) > strlen(DevInfo->Name))
	(void) snprintf(BasePath, sizeof(BasePath), "%s/%s",
			DirName(DevFile), DiskLabel->d_typename);
    /*
     * Get the DOS partitioning first which is required on OS's like
     * FreeBSD which are layered: 
     *		RawDisk -> DOS Part -> BSD DiskLabel
     */
    Query.BasePath = BasePath;
    (void) DosPartGet(&Query);

    (void) GetDiskPart(DiskLabel, DevInfo, DiskDrive, BasePath);

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

    DevFile[0] = CNULL;

    if (ProbeData->DevDefine && ProbeData->DevDefine->File &&
	ProbeData->DevData) {
	(void) snprintf(DevFile, sizeof(DevFile),
			ProbeData->DevDefine->File,
			ProbeData->DevData->DevUnit);
	(void) strcpy(CtlFile, DevFile);
    } else {
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
    }

    /*
     * See if we can open the device (to see if it exists) before
     * we start allocing memory.
     */
    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Open for read failed: %s", DevFile, SYSERR);
	if (errno == ENXIO)	/* Device not configured */
	    return((DevInfo_t *) NULL);
    }

    DevInfo = CreateBaseDiskDrive(ProbeData, DevName);

    if (fd > 0) {
	DevAddFile(DevInfo, strdup(DevFile));

	if (DevInfo->DevSpec)
	    DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
	else {
	    DiskDriveData = NewDiskDriveData(NULL);
	    DevInfo->DevSpec = (void *) DiskDriveData;
	}

	/*
	 * Only use DIOCGDINFO on disks and CDs (which are sometimes
	 * removable hard disks)
	 */
	if (DevInfo->Type == DT_DISKDRIVE || DevInfo->Type == DT_CD) {
	    /*
	     * DIOCGDINFO is our primary source of information 
	     */
	    (void) memset(&DiskLabel, 0, sizeof(DiskLabel));
	    if (ioctl(fd, DIOCGDINFO, &DiskLabel) == 0)
		SetDiskLabel(DevInfo, DiskDriveData, DevFile, &DiskLabel);
	    else
		SImsg(SIM_GERR, "%s: ioctl DIOCGDINFO failed: %s", 
		      DevFile, SYSERR);
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
	DevAddFile(DevInfo, strdup(CtlFile));

	(void) ScsiQuery(DevInfo, CtlFile, fd, TRUE);

	(void) close(fd);
    }

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Build Partition information
 */
extern int BuildPartInfo(PartInfoTree, SearchExp)
     PartInfo_t		      **PartInfoTree;
     char		      **SearchExp;
{
    return BuildPartInfoDevices(PartInfoTree, SearchExp);
}
