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
 * SunOS 5.x specific routines
 */

#include "defs.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/cdio.h>
#include <sys/fdio.h>
#if	defined(HAVE_VOLMGT)
#include <volmgt.h>
#endif	/* HAVE_VOLMGT */

/*
 * CPU (model) symbol
 */
char 				CpuSYM[] = "_cputype";

static char			Buff[BUFSIZ];
DevInfo_t		       *DevInfo;
extern char		        RomVecSYM[];
struct stat			StatBuf;

/* 
 * Get device file list.
 * Finds a list of match device files.
 * File list is set to ArgvPtr.
 * Returns number of files found (in ArgvPtr).
 *
 * If DevDefine->File is set, use that to find file name.
 * Otherwise, scan DevDir looking for a device file that matches 
 * DevData->DevNum.
 */
static int GetDeviceFile(ProbeData, DevDir, ArgvPtr)
     ProbeData_t	       *ProbeData;
     char		       *DevDir;
     char		     ***ArgvPtr;
{
    static DIR		       *DirPtr = NULL;
    struct dirent	       *DirEnt;
    static char			PathBuff[MAXPATHLEN];
    char		       *Path = PathBuff;
    static char			LastDir[MAXPATHLEN];
    register int		i;
    int				Len;
    int				Argc;
    char		      **Argv = NULL;
    static char		      **FileArgv = NULL;
    static int			FileArgc = 0;
    char		       *File;
    DevData_t 		       *DevData;
    DevDefine_t		       *DevDefine;

    if (!ProbeData)
	return(0);
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    /* Don't bother trying if the devnum is 0 */
    if (DevData->DevNum <= 0)
	return(0);

    if (FileArgv && FileArgc > 0) {
	DestroyArgv(&FileArgv, FileArgc);
	FileArgv = NULL;
	FileArgc = 0;
    }

    if (DevDefine->File) {
	/*
	 * DevDefine->File is a space seperated list of file names.
	 * If a '%' is found, then the unit number is inserted at
	 * that point.
	 */
	Argc = StrToArgv(DevDefine->File, " ", &Argv, NULL, 0);
	if (Argc <= 0)
	    return(0);
	FileArgv = (char **) xcalloc(Argc, sizeof(char **));
	for (i = 0; i < Argc; ++i) {
	    File = Argv[i];
	    /* Allow NULL members of Argv */
	    if (!File || !File[0])
		continue;
	    /*
	     * If File doesn't start with '/', then insert DevDir.
	     * If File contains '%', then insert the device number.
	     */
	    PathBuff[0] = CNULL;
	    if (File[0] != '/')
		(void) snprintf(PathBuff, sizeof(PathBuff),  "%s/", DevDir);
	    Path = PathBuff + (Len = strlen(PathBuff));

	    if (strchr(File, '%') && DevData->DevUnit >= 0)
		(void) snprintf(Path, sizeof(PathBuff)-Len,
				File, DevData->DevUnit);
	    else
		(void) strcat(Path, File);

	    if (Path[0]) {
		if (stat(Path, &StatBuf) == 0)
		    FileArgv[FileArgc++] = strdup(Path);
		else {
		    SImsg(SIM_GERR, "Stat of %s failed: %s.", Path, SYSERR);
		    Argv[i] = NULL;
		}
	    } else
		SImsg(SIM_DBG, "Convert pathname failed: %s", File);
	}
	if (Argc)
	    DestroyArgv(&Argv, Argc);
    } else {
	if (!DirPtr || !EQ(DevDir, LastDir)) {
	    if (!(DirPtr = opendir(DevDir))) {
		SImsg(SIM_GERR, "Cannot open directory \"%s\": %s", 
		      DevDir, SYSERR);
		return(-1);
	    }
	    (void) strcpy(LastDir, DevDir);
	} else
	    rewinddir(DirPtr);

	Path = PathBuff;
	while (DirEnt = readdir(DirPtr)) {
	    if (EQ(DirEnt->d_name, ".") || EQ(DirEnt->d_name, ".."))
		continue;
	
	    (void) snprintf(Path, sizeof(PathBuff), "%s/%s", 
			    DevDir, DirEnt->d_name);

	    if (stat(Path, &StatBuf) < 0) {
		SImsg(SIM_GERR, "Stat of %s failed: %s.", Path, SYSERR);
		continue;
	    }

#if	defined(DEBUG_VERBOSE)
	    SImsg(SIM_DBG, "GetDeviceFile dev '%s' (%d) got '%s' (%d)",
		  DevData->DevName, DevData->DevNum,
		  Path, StatBuf.st_rdev);
#endif	/* DEBUG_VERBOSE */

	    if (StatBuf.st_rdev == DevData->DevNum) {
		FileArgv = (char **) xcalloc(2, sizeof(char *));
		FileArgv[FileArgc++] = strdup(Path);
		break;
	    }
	}
    }

    if (FileArgc)
	*ArgvPtr = FileArgv;

    return(FileArgc);
}

/*
 * Create a tape device
 */
static DevInfo_t *CreateTapeDrive(ProbeData, DevName, TapeName)
     ProbeData_t	       *ProbeData;
     char		       *DevName;
     char		       *TapeName;
{
    DevInfo_t		       *DevInfo;

    ProbeData->DevName = DevName;
    DevInfo = DeviceCreate(ProbeData);
    if (!EQ(DevName, TapeName))
	DevInfo->AltName = strdup(TapeName);
    DevInfo->Type 		= DT_TAPEDRIVE;

    return(DevInfo);
}

/*
 * Probe a Tape Drive.
 */
extern DevInfo_t *ProbeTapeDrive(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t 		       *DevInfo;
    char 		       *DevFile;
    char 		       *DevName;
    char 		       *Model = NULL;
    char		      **FileList;
    int				FileNum;
    static char 		DevFilen[MAXPATHLEN];
    struct mtget 		mtget;
    register char	       *cp;
    int 			fd;
    DevDesc_t		       *ScsiDevDesc;
    char 		       *TapeName;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;

    TapeName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    FileNum = GetDeviceFile(ProbeData, _PATH_DEV_RMT, &FileList);
    if (FileNum < 1) {
	SImsg(SIM_GERR, "Cannot find tape device for <%s>", TapeName);
	return((DevInfo_t *)NULL);
    }
    /* Only bother with the first file name */
    DevFile = FileList[0];

    /* 
     * Use DevFile to get name of device.
     * Takes something like "/dev/rmt/0l" and
     * turns it into "rmt/0".
     */
    cp = DevFile;
    /* Skip over "/dev/" */
    if ((int)strlen(DevFile) > 5 && EQN(DevFile, "/dev/", 4))
	cp += 5;
    DevName = cp;
    if (cp = strrchr(DevFile, '/')) {
	++cp;
	while (isdigit(*++cp));
	if (*cp && !isdigit(*cp))
	    *cp = CNULL;
    }

    /* Use the no rewind file */
    (void) snprintf(DevFilen, sizeof(DevFilen), "%sn", DevFile);

    /*
     * The O_NDELAY flag will allow the device to be opened even if no
     * media is loaded.
     */
    if ((fd = open(DevFilen, O_RDONLY|O_NDELAY)) < 0) {
	SImsg(SIM_GERR, "%s Cannot open for read: %s.", DevFilen, SYSERR);

	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if ((DevDefine->Model || DevDefine->Desc) &&
	    FLAGS_ON(DevData->Flags, DD_IS_ALIVE)) {
	    DevInfo = CreateTapeDrive(ProbeData, DevName, TapeName);
	    return(ProbeData->RetDevInfo = DevInfo);
	} else
	    return((DevInfo_t *) NULL);
    }

    if (ioctl(fd, MTIOCGET, &mtget) != 0) {
	SImsg(SIM_GERR, "%s: Cannot extract tape status: %s.", 
	      DevName, SYSERR);
	return((DevInfo_t *) NULL);
    }

    cp = GetTapeModel(mtget.mt_type);
    if (cp)
	Model = strdup(cp);
    else
	Model = "unknown";

    /*
     * Set device info
     */
    DevInfo = CreateTapeDrive(ProbeData, DevName, TapeName);
    if (Model)
	DevInfo->Model 		= Model;
    else
	DevInfo->Model 		= DevDefine->Model;
    DevInfo->Master 		= MkMasterFromDevData(DevData);

    (void) ScsiQuery(DevInfo, DevFilen, fd, TRUE);

    (void) close(fd);

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Get SVR4 style partition information
 */
DKvtoc *GETvtoc(fd, DevFile)
    int				fd;
    char		       *DevFile;
{
    static DKvtoc 		vtoc;

    if (ioctl(fd, DKIOCGVTOC, &vtoc) < 0) {
	SImsg(SIM_GERR, "%s: DKIOCGVTOC failed: %s.", DevFile, SYSERR);
	return((DKvtoc *)NULL);
    }

    return(&vtoc);
}

/*
 * Get controller information
 */
DKcinfo *GETdk_cinfo(fd, DevFile)
    int				fd;
    char		       *DevFile;
{
    static DKcinfo 		dk_cinfo;

    if (ioctl(fd, DKIOCINFO, &dk_cinfo) < 0) {
	SImsg(SIM_GERR, "%s: DKIOCINFO failed: %s.", DevFile, SYSERR);
	return((DKcinfo *)NULL);
    }

    return(&dk_cinfo);
}

/*
 * Get disk geometry
 */
DKgeom *GETdk_geom(fd, DevFile)
    int				fd;
    char		       *DevFile;
{
    static DKgeom 		dk_geom;

    if (ioctl(fd, DKIOCGGEOM, &dk_geom) < 0) {
	SImsg(SIM_GERR, "%s: DKIOCGGEOM failed: %s.", DevFile, SYSERR);
	return((DKgeom *)NULL);
    }

    return(&dk_geom);
}

#if	defined(HAVE_HDIO)
/*
 * Get disk type structure.
 */
static DKtype *GETdk_type(fd, file)
    int 			fd;
    char 		       *file;
{
    static DKtype 		dk_type;

    if (ioctl(fd, HDKIOCGTYPE, &dk_type) < 0) {
	if (errno != ENOTTY)
	    SImsg(SIM_GERR, "%s: DKIOCGTYPE failed: %s.", file, SYSERR);
	return(NULL);
    }

    return(&dk_type);
}
#endif	/* HAVE_HDIO */

/*
 * Find mount point for a given device
 */
static char *GetMountInfo(DevName, PartName)
    char		       *DevName;
    char		       *PartName;
{
    static FILE 	       *mntFilePtr = NULL;
    static FILE 	       *vfstabFilePtr = NULL;
    struct mnttab 	        MntTab;
    struct vfstab		vfstab;
    static char			Name[MAXPATHLEN];
    static char			Path[MAXPATHLEN];
    register char	       *cp;

    if (!DevName || !PartName)
	return((char *) NULL);

    (void) snprintf(Name, sizeof(Name),  "%s%s", DevName, PartName);

    /*
     * First try the current mount table
     */
    if (!mntFilePtr) {
	if ((mntFilePtr = fopen(MNTTAB, "r")) == NULL) {
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", 
		  MNTTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(mntFilePtr);

    while (getmntent(mntFilePtr, &MntTab) == 0) {
	if (!MntTab.mnt_special)
	    continue;
	if (cp = strrchr(MntTab.mnt_special, '/'))
	    ++cp;
	else
	    cp = MntTab.mnt_special;
	if (EQ(cp, Name))
	    return(strdup(MntTab.mnt_mountp));
    }

    /*
     * Now try the static mount table, which may not reflect current reality.
     */
    if (!vfstabFilePtr) {
	if ((vfstabFilePtr = fopen(VFSTAB, "r")) == NULL) {
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", 
		  VFSTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(vfstabFilePtr);

    (void) snprintf(Path, sizeof(Path), "%s/%s", _PATH_DEV_DSK, Name);

    if (getvfsspec(vfstabFilePtr, &vfstab, Path) == 0) {
	if (EQ(vfstab.vfs_fstype, "swap"))
	    return("swap");
	else if (vfstab.vfs_mountp && !EQ(vfstab.vfs_mountp, "-"))
	    return(strdup(vfstab.vfs_mountp));
    }

    return((char *) NULL);
}

/*
 * Translate disk partition information from basic
 * extracted disk info.
 */
static DiskPart_t *GetDiskPart(DevName, dk_vtoc)
    char 		       *DevName;
    DKvtoc 		       *dk_vtoc;
{
    register ushort 		i;
    static char 		pname[4];
    DiskPart_t 		       *Base = NULL;
    DiskPart_t 		       *Last = NULL;
    DiskPart_t		       *DiskPart = NULL;

    if (!DevName || !dk_vtoc)
	return((DiskPart_t *) NULL);

    pname[2] = CNULL;
    /*
     * dk_vtoc->v_nparts is always 0 after the first time through
     * here, so we use V_NUMPAR.
     */
    for (i = 0; i < V_NUMPAR; ++i) {
	/* Skip partitions that have no size */
	if (!dk_vtoc->v_part[i].p_size)
	    continue;
	DiskPart = (DiskPart_t *) xcalloc(1, sizeof(DiskPart_t));
	(void) snprintf(pname, sizeof(pname),  "s%d", i);
	DiskPart->Name = strdup(pname);
	DiskPart->Usage = GetMountInfo(DevName, pname);
	DiskPart->StartSect = (Large_t) dk_vtoc->v_part[i].p_start;
	DiskPart->NumSect = (Large_t) dk_vtoc->v_part[i].p_size;

	if (Base) {
	    Last->Next = DiskPart;
	    Last = DiskPart;
	} else
	    Base = Last = DiskPart;
    }

    return(Base);
}

/*
 * Extract disk label
 */
static char *GetDiskLabel(dk_vtoc)
    DKvtoc		       *dk_vtoc;
{
    register char	       *cp;
    register char	       *bp;
    register char	       *end;
    char		       *Label = NULL;

    if (!dk_vtoc || !dk_vtoc->v_asciilabel)
	return((char *)NULL);

    Label = dk_vtoc->v_asciilabel;

    /*
     * The label normally has geometry information in it we don't want
     * to see, so we trim out anything starting with " cyl".
     */
    for (cp = Label; cp && *cp; ++cp)
	if (*cp == ' ' && strncasecmp(cp, " cyl", 4) == 0) {
	    *cp = CNULL;
	    break;
	}

    /*
     * Zap any trailing white space we find
     */
    bp = Label;
    bp += strlen(Label);
    end = bp;
    for (cp = end - 1; cp > Label && *cp == ' '; --cp);
    if (cp != end - 1 && cp && *++cp == ' ')
	*cp = CNULL;

    /*
     * Check what we have so far.
     * "DEFAULT" is sometimes used by Solaris x86.
     */
    if (!Label || !Label[0] || EQ(Label, "DEFAULT"))
	return((char *) NULL);

    return(strdup(Label));
}

/*
 * Get the name of the controller for a disk.
 */
static char *GetDkCtlrName(dk_cinfo)
    DKcinfo 		       *dk_cinfo;
{
    if (!dk_cinfo)
	return((char *) NULL);

    (void) snprintf(Buff, sizeof(Buff), "%s%d", 
		    dk_cinfo->dki_cname, dk_cinfo->dki_cnum);

    return(strdup(Buff));
}

/*
 * Get a disk controller device from disk info.
 */
static DevInfo_t *GetDkCtlrDevice(DevData, dk_cinfo)
    DevData_t 		       *DevData;
    DKcinfo	 	       *dk_cinfo;
{
    DevInfo_t 		       *MkMasterFromDevData();
    DevInfo_t 		       *DiskCtlr;

    if (!dk_cinfo)
	return((DevInfo_t *) NULL);

    /*
     * Get name of controller from devdata if available
     */
    if (DevData && DevData->CtlrName)
	DiskCtlr = MkMasterFromDevData(DevData);
    else {
	if ((DiskCtlr = NewDevInfo(NULL)) == NULL)
	    return((DevInfo_t *) NULL);
    }

    DiskCtlr->Type = DT_DISKCTLR;

    /*
     * Get name of controller from devdata if available
     */
    if (DevData && DevData->CtlrName)
	DiskCtlr = MkMasterFromDevData(DevData);

    if (!DiskCtlr->Name) {
	DiskCtlr->Name = GetDkCtlrName(dk_cinfo);
	DiskCtlr->Unit = dk_cinfo->dki_cnum;
    }
    DiskCtlr->Addr = dk_cinfo->dki_addr;
    DiskCtlr->Prio = dk_cinfo->dki_prio;
    DiskCtlr->Vec = dk_cinfo->dki_vec;
    SetDiskCtlrModel(DiskCtlr, dk_cinfo->dki_ctype);

    return(DiskCtlr);
}

/*
 * Convert all we've learned about a disk to a DevInfo_t.
 */
static DevInfo_t *CreateDiskDrive(ProbeData, DevName,
				  dk_vtoc, dk_cinfo, dk_geom, dk_type)
     ProbeData_t	       *ProbeData;
     char 		       *DevName;
     DKvtoc		       *dk_vtoc;
     DKcinfo	 	       *dk_cinfo;
     DKgeom 		       *dk_geom;
     DKtype 		       *dk_type;
{
    DevInfo_t 		       *DevInfo;
    DevInfo_t 		       *DiskCtlr = NULL;
    DiskDriveData_t 	       *DiskDriveData = NULL;
    DiskDrive_t 	       *DiskDrive = NULL;
    DevData_t 		       *DevData;
    static DevInfo_t		ScsiDevInfo;
    int				GotScsi = FALSE;
    int				DevFD;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevFD = ProbeData->FileDesc;
    DevData = ProbeData->DevData;

    ProbeData->DevName = DevName;
    if (!(DevInfo = DeviceCreate(ProbeData))) {
	SImsg(SIM_GERR, "Cannot create new device entry.");
	return((DevInfo_t *) NULL);
    }

    /*
     * We call ScsiQuery() here because it needs a DevInfo to operate on.
     */
    if (ScsiQuery(DevInfo, ProbeData->DevFile, DevFD, TRUE) == 0)
	GotScsi = TRUE;

    if (dk_vtoc == NULL) {
	SImsg(SIM_GERR, "%s: No table of contents found on disk.", DevName);
	if (!GotScsi)
	    return((DevInfo_t *) NULL);
    }

    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	if ((DiskDriveData = NewDiskDriveData(NULL)) == NULL) {
	    SImsg(SIM_GERR, "Cannot create new DiskDriveData entry.");
	    return((DevInfo_t *) NULL);
	}
	DevInfo->DevSpec = (void *) DiskDriveData;
    }

    if ((DiskDrive = NewDiskDrive(NULL)) == NULL) {
	SImsg(SIM_GERR, "Cannot create new DiskDrive entry.");
	return((DevInfo_t *) NULL);
    }
    /*
     * DiskDrive is the OS DiskDrive data
     */
    DiskDriveData->OSdata = DiskDrive;

    (void) snprintf(Buff, sizeof(Buff), "%s%d", 
		    DevData->DevName, DevData->DevUnit);
    DevInfo->AltName 		= strdup(Buff);
    DevInfo->Type 		= DT_DISKDRIVE;

    if (dk_vtoc && dk_vtoc->v_sanity == VTOC_SANE) {
	/*
	 * Only read partition info we we're going to print it later.
	 */
	if (VL_ALL)
	    DiskDrive->DiskPart	= GetDiskPart(DevName, dk_vtoc);
	if (!DiskDrive->Label)
	    DiskDrive->Label 	= GetDiskLabel(dk_vtoc);
	DiskDrive->SecSize 	= dk_vtoc->v_sectorsz;
	if (dk_vtoc->v_volume[0]) {
	    (void) snprintf(Buff, sizeof(Buff), "\"%.*s\"", 
			    sizeof(dk_vtoc->v_volume), dk_vtoc->v_volume);
	    AddDevDesc(DevInfo, Buff, "Volume Name", DA_APPEND);
	}
    }
    if (!DevInfo->Model)
	DevInfo->Model 		= DiskDrive->Label;

    if (dk_cinfo) {
	DiskDrive->Unit 	= dk_cinfo->dki_unit;
	DiskDrive->Slave 	= dk_cinfo->dki_slave;;
    }
    if (dk_geom) {
	DiskDrive->DataCyl 	= dk_geom->dkg_ncyl;
	DiskDrive->PhyCyl 	= dk_geom->dkg_pcyl;
	DiskDrive->AltCyl 	= dk_geom->dkg_acyl;
	DiskDrive->Tracks 	= dk_geom->dkg_nhead;
	DiskDrive->Sect 	= dk_geom->dkg_nsect;
	DiskDrive->APC 		= dk_geom->dkg_apc;
	DiskDrive->RPM 		= dk_geom->dkg_rpm;
	DiskDrive->IntrLv 	= dk_geom->dkg_intrlv;
    }
#if	defined(HAVE_HDIO)
    if (dk_type) {
	DiskDrive->PhySect 	= dk_type->hdkt_hsect;
	DiskDrive->PROMRev 	= dk_type->hdkt_promrev;
    }
#endif	/* HAVE_HDIO */

    if (DiskCtlr = GetDkCtlrDevice(DevData, dk_cinfo))
      DevInfo->Master 		= DiskCtlr;

#if	defined(i386)
    (void) DosPartGet(DevInfo);
#endif

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

    if (DiskName)
	ProbeData->DevName = DiskName;

    DevInfo = DeviceCreate(ProbeData);
    if (!DevInfo)
	return((DevInfo_t *) NULL);

    /*
     * See if there's a good alternative name we can set.
     */
    if (DevData)
	AltName = MkDevName(DevData->DevName, DevData->DevUnit,
			    DevDefine->Type, DevDefine->Flags);
    else if (!EQ(DiskName, DevName))
	AltName = DiskName;
    if (AltName && !EQ(DevInfo->Name, AltName))
	DevInfo->AltName = strdup(AltName);

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Create CDROM device
 */
static DevInfo_t *CreateCDROM(ProbeData, DevName, CDspeed)
     ProbeData_t	       *ProbeData;
     char 		       *DevName;
     int			CDspeed;
{
    DevInfo_t 		       *DevInfo;
    Define_t		       *Def;

    DevInfo = CreateBaseDiskDrive(ProbeData, DevName);

    Def = DefGet(DL_CDSPEED, NULL, CDspeed, 0);
    if (Def)
	DevInfo->ModelDesc = Def->ValStr1;
    else
	SImsg(SIM_UNKN, "Unknown CDROM Speed: 0x%x", CDspeed);

    return(DevInfo);
}

#if	defined(HAVE_VOLMGT)
/*
 * Use the Solaris Volume Management API to acquire the device.
 * NOTE: Most of this code doesn't do us much good right now because
 * not all ioctl()'s are passed through volfs(7) to the underlying driver.
 * In particuliar CDROMGDRVSPEED doesn't work as of at least SunOS 5.6.
 *
 * Returns the pathname to the acquire device on success.
 * Returns NULL on failure.
 */
static char *VolMgtAcquire(ProbeData)
     ProbeData_t	       *ProbeData;
{
    const char		       *VolRoot = NULL;
    char		       *VolPath = NULL;
    char		       *ErrPtr = NULL;
    pid_t			ProcID = 0;
    char		       *cp;

    if (!(VolRoot = volmgt_root())) {
	SImsg(SIM_GERR, "Get volume manager root failed: %s", SYSERR);
	return((char *) NULL);
    }

    VolPath = (char *) xmalloc(strlen(VolRoot) +
			       strlen(ProbeData->DevFile) + 1);

    (void) snprintf(VolPath, sizeof(VolPath),  "%s%s", 
		    VolRoot, ProbeData->DevFile);
    /* 
     * VolPath should be something like "/vol/dev/rdsk/c0t2d0s2" for a CD.
     * If so, remove the 'sX' from VolPath.
     */
    if (EQN(VolPath + strlen(VolRoot), "/dev/rdsk/c", 11))
	if (cp = strrchr(VolPath, 's'))
	    *cp = CNULL;

    if (!volmgt_acquire(VolPath, ProgramName, 0, &ErrPtr, &ProcID)) {
	SImsg(SIM_GERR, "%s: volmgt_acquire failed: %s", VolPath, SYSERR);
	return((char *) NULL);
    }

    return(VolPath);
}

/*
 * Release device VolPath by calling the volmgt_* API.
 */
static char *VolMgtRelease(VolPath)
     char		       *VolPath;
{
    if (!volmgt_release(VolPath))
	SImsg(SIM_GERR, "%s: volmgt_release failed: %s", VolPath, SYSERR);

    (void) free(VolPath);
}

/*
 * Use VolMgt API to get attributes for a device.
 * This only works if media is loaded.
 */
static void VolMgtGetAttr(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t 		       *DevInfo = NULL;
    char		       *VolPath;
    char		       *cp;

    if (!ProbeData)
	return;

    DevInfo = ProbeData->UseDevInfo;
    VolPath = ProbeData->DevFile;

    if (cp = media_getattr(VolPath, "s-access")) {
	if (EQ(cp, "seq"))
	    cp = "Sequential Access";
	else if (EQ(cp, "rand"))
	    cp = "Random Access";
	AddDevDesc(DevInfo, cp, "Access Type", DA_APPEND);
    }

    if (cp = media_getattr(VolPath, "s-density"))
	AddDevDesc(DevInfo, cp, "Media Density", DA_APPEND);

    if (cp = media_getattr(VolPath, "s-parts"))
	AddDevDesc(DevInfo, cp, "Partitions", DA_APPEND);

    if (cp = media_getattr(VolPath, "s-location"))
	AddDevDesc(DevInfo, cp, "Location", DA_APPEND);

    if (cp = media_getattr(VolPath, "s-mejectable"))
	AddDevDesc(DevInfo, cp, "Has Manual Eject", DA_APPEND);
}
#endif	/* HAVE_VOLMGT */

/*
 * Probe a specific disk drive as file DevFile.
 * The UsingVolMgt flag indicates whether we're using VolMgt on this call.
 */
static DevInfo_t *ProbeDiskDriveFile(ProbeData, UsingVolMgt)
     ProbeData_t	       *ProbeData;
     int			UsingVolMgt;
{
    DevInfo_t 		       *DevInfo = NULL;
    DKcinfo 		       *dk_cinfo;
    DKgeom 		       *dk_geom;
    DKtype 		       *dk_type;
    DKvtoc 		       *dk_vtoc;
    char		       *DevName;
    char		       *cp;
    int				CDspeed = 0;
    int				Len;
    int				fd;
    int				Status;
    char 		       *DiskName;
    char		       *VolDevFile = NULL;
    char 		       *DevFile;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;

    if (!ProbeData || !(DevFile = ProbeData->DevFile)) {
	SImsg(SIM_GERR, "ProbeDiskDriveFile: Missing parameters.");
	return((DevInfo_t *)NULL);
    }
    DiskName = ProbeData->DevName;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;

    /* Use basename of DevInfo file for virtual disk device name */
    DevName = strdup(DevFile);
    if (cp = strrchr(DevName, '/'))
	DevName = ++cp;
    if (EQN(DevFile, "/dev/rdsk/c", 11)) {
	/* Zap slice part of name */
	if (cp = strrchr(DevName, 's'))
	    *cp = CNULL;
    }

    /*
     * Set ProbeData for other function calls
     */
    ProbeData->DevFile = DevFile;
    ProbeData->DevName = DevName;

    /*
     * Try opening the disk device.  Usually this will be the "s0" device.
     * If that fails, try opening "s2".  Sometimes there's no "s0"
     * partition, but there is an "s2".
     * 
     * The O_NDELAY flag will allow the device to be opened even if no
     * media is loaded.
     */
    fd = open(DevFile, O_RDONLY|O_NDELAY);
    ProbeData->FileDesc = fd;
    if (fd < 0) {
	Len = strlen(DevFile);
	if (DevFile[Len-1] == '0') {
	    DevFile[Len-1] = '2';
	    fd = open(DevFile, O_RDONLY|O_NDELAY);
	}
    }
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", DevFile, SYSERR);
#if	defined(HAVE_VOLMGT)
	if (errno == EBUSY && !UsingVolMgt) {
	    /*
	     * The device is busy, so let's try acquiring the device from
	     * the Volume Manager.  If that succeeds, call this function
	     * again, but use the Vol Manager device pathname.
	     */
	    if (VolDevFile = VolMgtAcquire(ProbeData)) {
		ProbeData->DevFile = VolDevFile;
		DevInfo = ProbeDiskDriveFile(ProbeData, TRUE);
		ProbeData->UseDevInfo = DevInfo;
		VolMgtGetAttr(ProbeData);
		VolMgtRelease(VolDevFile);
		return(DevInfo);
	    }
	}
#endif	/* HAVE_VOLMGT */
	if (errno == EBUSY || 
	    ((DevDefine->Model || DevDefine->Desc || DevData->DevNum > 0) &&
	     FLAGS_ON(DevData->Flags, DD_IS_ALIVE))) {
	    /*
	     * If we know for sure this drive is present and we
	     * know something about it, then create a minimal device.
	     */
	    return(CreateBaseDiskDrive(ProbeData, DiskName));
	}
	return((DevInfo_t *) NULL);
    }

    Status = -1;
#if	defined(CDROMGDRVSPEED)
    /*
     * If this is a DT_DISKDRIVE, it could really be a CDROM, so this is how
     * we tell the difference.
     */
    if (DevDefine->Type == DT_DISKDRIVE || DevDefine->Type == DT_CDROM) {
	Status = ioctl(fd, CDROMGDRVSPEED, &CDspeed);
	if (Status != 0)
	    SImsg(SIM_GERR, "%s: ioctl CDROMGDRVSPEED failed: %s", 
		  DevFile, SYSERR);
    }
#endif
    if (Status == 0)
	/*
	 * It's definetely a CDROM.
	 */
	DevInfo = CreateCDROM(ProbeData, DevName, CDspeed);
    else if (DevDefine->Type == DT_DISKDRIVE) {
	/*
	 * It's a normal Disk Drive.
	 */
	if ((dk_vtoc = GETvtoc(fd, DevFile)) == NULL)
	    SImsg(SIM_GERR, "%s: get vtoc failed.", DevFile);
	if ((dk_cinfo = GETdk_cinfo(fd, DevFile)) == NULL)
	    SImsg(SIM_GERR, "%s: get dk_cinfo failed.", DevFile);
	if ((dk_geom = GETdk_geom(fd, DevFile)) == NULL)
	    SImsg(SIM_GERR, "%s: get dk_geom failed.", DevFile);
#if	defined(HAVE_HDIO)
	if ((dk_type = GETdk_type(fd, DevFile)) == NULL)
	    SImsg(SIM_DBG, "%s: no dk_type info available.", DevFile);
#endif	/* HAVE_HDIO */

	DevInfo = CreateDiskDrive(ProbeData, DevName,
				  dk_vtoc, dk_cinfo, dk_geom, dk_type);
	if (!DevInfo)
	    SImsg(SIM_GERR, "%s: Cannot convert DiskDrive information.", 
		  DevName);
    }
    if (!DevInfo)
	/*
	 * If we still haven't gotten any information on what type of 
	 * DiskDrive this really is, create a basic entry anyway.
	 */
	DevInfo = CreateBaseDiskDrive(ProbeData, DiskName);

    close(fd);
    ProbeData->FileDesc = -1;

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Probe a disk drive by general name (DiskName)
 */
extern DevInfo_t *ProbeDiskDrive(ProbeData)
     ProbeData_t	       *ProbeData;
{
    char		      **FileList;
    int				FileCount;
    register int		i;
    char 		       *DiskName;

    if (!ProbeData || !(DiskName = ProbeData->DevName))
	return((DevInfo_t *)NULL);

    FileCount = GetDeviceFile(ProbeData, _PATH_DEV_RDSK, &FileList);
    if (FileCount < 1) {
	SImsg(SIM_GERR, "Cannot find disk device file for <%s>.", DiskName);
	return((DevInfo_t *)NULL);
    }

    for (i = 0; i < FileCount; ++i) {
	ProbeData->DevFile = FileList[i];
	if (DevInfo = ProbeDiskDriveFile(ProbeData, FALSE))
	    return(ProbeData->RetDevInfo = DevInfo);
    }

    return((DevInfo_t *)NULL);
}

/*
 * Probe a Floppy device referenced by ProbeData->DevFile.
 * UsingVolMgt means we're using VolMgt on this call.
 */
static DevInfo_t *ProbeFloppyFile(ProbeData, UsingVolMgt)
     ProbeData_t	       *ProbeData;
     int			UsingVolMgt;
{
    DevInfo_t 		       *DevInfo = NULL;
    static struct fd_char	FDchar;
    static struct fd_drive	FDdrive;
    char		       *DevName;
    int				fd;
    char		       *VolDevFile = NULL;
    char 		       *DevFile;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
    DiskDriveData_t	       *DiskDriveData = NULL;
    DiskDrive_t		       *Disk = NULL;

    if (!ProbeData || !ProbeData->DevFile)
	return((DevInfo_t *)NULL);

    DevFile = ProbeData->DevFile;
    DevData = ProbeData->DevData;
    DevDefine = ProbeData->DevDefine;
    DevData->DevType = DT_FLOPPY;

    /*
     * Try opening the disk device.  Usually this will be the "s0" device.
     * If that fails, try opening "s2".  Sometimes there's no "s0"
     * partition, but there is an "s2".
     * 
     * The O_NDELAY flag will allow the device to be opened even if no
     * media is loaded.
     */
    fd = open(DevFile, O_RDONLY|O_NDELAY|O_NONBLOCK);
    ProbeData->FileDesc = fd;
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", DevFile, SYSERR);
#if	defined(HAVE_VOLMGT)
	if (errno == EBUSY && !UsingVolMgt) {
	    /*
	     * The device is busy, so let's try acquiring the device from
	     * the Volume Manager.  If that succeeds, call this function
	     * again, but use the Vol Manager device pathname.
	     */
	    if (VolDevFile = VolMgtAcquire(ProbeData)) {
		ProbeData->DevFile = VolDevFile;
		DevInfo = ProbeDiskDriveFile(ProbeData, TRUE);
		ProbeData->UseDevInfo = DevInfo;
		VolMgtGetAttr(ProbeData);
		VolMgtRelease(VolDevFile);
		return(DevInfo);
	    }
	}
#endif	/* HAVE_VOLMGT */
	if (errno == EBUSY || 
	    ((DevDefine->Model || DevDefine->Desc || DevData->DevNum > 0) &&
	     FLAGS_ON(DevData->Flags, DD_IS_ALIVE))) {
	    /*
	     * If we know for sure this drive is present and we
	     * know something about it, then create a minimal device.
	     */
	    return(DeviceCreate(ProbeData));
	}
	return((DevInfo_t *) NULL);
    }

    if (!DevInfo)
	DevInfo = DeviceCreate(ProbeData);

    (void) memset(&FDchar, 0, sizeof(FDchar));
    if (ioctl(fd, FDIOGCHAR, &FDchar) < 0) {
	SImsg(SIM_DBG, "%s: ioctl FDIOGCHAR failed: %s", DevFile, SYSERR);
    } else {
	/* Setup Disk info */
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
	/* Set what we got */
	Disk->Tracks = FDchar.fdc_nhead;
	Disk->SecSize = FDchar.fdc_sec_size;
	Disk->Sect = FDchar.fdc_secptrack;
	Disk->DataCyl = FDchar.fdc_ncyl;
	Disk->StepsPerTrack = FDchar.fdc_steps;
	if (FDchar.fdc_transfer_rate > 0)
	    AddDevDesc(DevInfo, itoa(FDchar.fdc_transfer_rate), 
		       "Transfer Rate (Kb/s)", DA_APPEND);
    }

    (void) memset(&FDdrive, 0, sizeof(FDdrive));
    if (ioctl(fd, FDGETDRIVECHAR, &FDdrive) < 0) {
	SImsg(SIM_DBG, "%s: ioctl FDGETDRIVECHAR failed: %s", DevFile, SYSERR);
    } else {
	if (FDdrive.fdd_ejectable > 0)
	    AddDevDesc(DevInfo, "Manual Eject", "Has", DA_APPEND);
	if (FDdrive.fdd_maxsearch > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_maxsearch),
		       "Size of per unit search table", DA_APPEND);
	if (FDdrive.fdd_writeprecomp > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_writeprecomp),
		       "Precompensation start cylinder", DA_APPEND);
	if (FDdrive.fdd_writereduce > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_writereduce),
		       "Reduce write current start cylinder", DA_APPEND);
	if (FDdrive.fdd_stepwidth > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_stepwidth),
		       "Width of step pulse (1us)", DA_APPEND);
	if (FDdrive.fdd_steprate > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_steprate),
		       "Step Rate (100us)", DA_APPEND);
	if (FDdrive.fdd_headsettle > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_headsettle),
		       "Head settle delay (100us)", DA_APPEND);
	if (FDdrive.fdd_headload > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_headload),
		       "Head load delay (100us)", DA_APPEND);
	if (FDdrive.fdd_headunload > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_headunload),
		       "Head unload delay (100us)", DA_APPEND);
	if (FDdrive.fdd_motoron > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_motoron),
		       "Motor On delay (100us)", DA_APPEND);
	if (FDdrive.fdd_motoroff > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_motoroff),
		       "Motor Off delay (100us)", DA_APPEND);
	if (FDdrive.fdd_precomplevel > 0)
	    AddDevDesc(DevInfo, itoa(FDdrive.fdd_precomplevel),
		       "Pre Comp Level (ns)", DA_APPEND);
    }

    close(fd);
    ProbeData->FileDesc = -1;

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Probe a floppy disk drive
 */
extern DevInfo_t *ProbeFloppy(ProbeData)
     ProbeData_t	       *ProbeData;
{
    char		      **FileList;
    int				FileCount;
    register int		i;
    char 		       *DevName;

    if (!ProbeData || !(DevName = ProbeData->DevName))
	return((DevInfo_t *)NULL);

    FileCount = GetDeviceFile(ProbeData, _PATH_DEV_RDSK, &FileList);
    if (FileCount < 1) {
	SImsg(SIM_GERR, "%s: Cannot find floppy device file.", DevName);
	return((DevInfo_t *)NULL);
    }

    for (i = 0; i < FileCount; ++i) {
	ProbeData->DevFile = FileList[i];
	if (DevInfo = ProbeFloppyFile(ProbeData, FALSE))
	    return(ProbeData->RetDevInfo = DevInfo);
    }

    return((DevInfo_t *)NULL);
}

/*
 * Get hostid
 */
long gethostid()
{
    char 			Buff[64];

    sysinfo(SI_HW_SERIAL, Buff, sizeof(Buff));

    return(atol(Buff));
}

/*
 * Get system page size
 */
int getpagesize()
{
    long			siz;

    if ((siz = sysconf(_SC_PAGESIZE)) == -1) {
	SImsg(SIM_GERR, "Cannot get pagesize from sysconf(): %s", SYSERR);
	return(0);
    }

    return((int) siz);
}

#if	defined(HAVE_SUNROMVEC)
#include <sys/comvec.h>
#endif	/* HAVE_SUNROMVEC */

/*
 * Get ROM Version number
 */
extern char *GetRomVerSun()
{
    static char			RomRev[32];
#if	defined(HAVE_SUNROMVEC)
    struct nlist	       *nlptr;
    static union sunromvec	Rom;
    kvm_t		       *kd;
    union sunromvec	       *romp;

    if (!(kd = KVMopen()))
	return((char *) NULL);

    if ((nlptr = KVMnlist(kd, RomVecSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);

    if (CheckNlist(nlptr))
	return((char *) NULL);

    /*
     * Read the kernel pointer to the sunromvec structure.
     */
    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &romp, 
	       sizeof(romp), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read sunromvec pointer from kernel.");
	return((char *) NULL);
    }

    /*
     * Read sunromvec from the kernel
     */
    if (KVMget(kd, (u_long) romp, (char *) &Rom, 
	       sizeof(union sunromvec), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read sunromvec from kernel.");
	return((char *) NULL);
    }

    if (Rom.sunmon.v_mon_id) {
	/*
	 * Read the version string from the address indicated by v_mon_id.
	 * Read 1 byte at a time until '\0' is encountered.
	 */
	if (KVMget(kd, (u_long) Rom.sunmon.v_mon_id, RomRev, 
		   sizeof(RomRev), KDT_STRING)) {
	    SImsg(SIM_GERR, "Cannot read sunmon rom revision from kernel.");
	    return((char *) NULL);
	}
    }

    if (Rom.obp.op_mon_id && !RomRev[0]) {
	/*
	 * XXX Hardcoded values
	 */
	(void) snprintf(RomRev, sizeof(RomRev),  "%d.%d", 
		       Rom.obp.op_mon_id >> 16, Rom.obp.op_mon_id & 0xFFFF);
    }

    KVMclose(kd);

#endif	/* HAVE_SUNROMVEC */

    return((RomRev[0]) ? RomRev : (char *) NULL);
}

/*
 * Get amount of physical memory using sysmem() call.
 */
extern char *GetMemorySunOSsysmem()
{
    char		       *Val = NULL;
#if	defined(USE_SYSMEM)
    /*
     * sysmem() is an undocumented function that returns the
     * amount of system (physical) memory in bytes.  It's in
     * /usr/lib/libadm.a.  Discovered by reference in Sun
     * PatchID 101050-01.
     *
     * NOTE: sysmem() is broken in SunOS 5.5 and 5.5.1 so we avoid it.
     */
    long			Amount;

    Amount = sysmem();
    if (Amount < 0) {
	SImsg(SIM_GERR, "sysmem() failed: %s", SYSERR);
	return((char *) NULL);
    }

    Val = GetMemoryStr(DivRndUp(Amount, (Large_t) MBYTES));
#endif	/* USE_SYSMEM */
    return(Val);
}

/*
 * Get amount of physical memory using sysconf()
 */
extern char *GetMemorySunOSsysconf()
{
    char		       *Val = NULL;
#if	defined(_SC_PAGESIZE) && defined(_SC_PHYS_PAGES)
    long long			Amount;
    long			PageSize;
    long			NumPages;

    PageSize = sysconf(_SC_PAGESIZE);
    NumPages = sysconf(_SC_PHYS_PAGES);
    if (PageSize < 0 || NumPages < 0) {
      SImsg(SIM_GERR, "sysconf() failed: %s", SYSERR);
      return((char *) NULL);
    }
    Amount = (long long) PageSize * NumPages;

    Val = GetMemoryStr((int) ((Amount+MBYTES-1) / MBYTES), MBYTES);
#endif	/* _SC_* */
    return(Val);
}

/*
 * Get the system to detect all disk drives on the system. 
 * We only open the "s2" file for each device to avoid a
 * performance hit of opening every device file.
 */
static void DetectDisks()
{
    static DIR		       *DirPtr;
    struct dirent	       *DirEnt;
    static char			DevName[MAXPATHLEN];
    static char			PathName[MAXPATHLEN];
    static namelist_t	       *UsedList = NULL;
    register char	       *cp;
    int			        FileD;

    if (!(DirPtr = opendir(_PATH_DEV_RDSK))) {
	SImsg(SIM_GERR, "Cannot open directory %s: %s.", 
	      _PATH_DEV_RDSK, SYSERR);
	return;
    }

    while (DirEnt = readdir(DirPtr)) {
	if (EQ(DirEnt->d_name, ".") || EQ(DirEnt->d_name, ".."))
	    continue;

	(void) strcpy(DevName, DirEnt->d_name);
	if (cp = strchr(DevName, 's'))
	    *cp = CNULL;
	if (NameListFind(UsedList, DevName))
	    continue;

	/* 
	 * Always use slice 2 which should always 
	 * have a partition table.
	 */
	(void) snprintf(PathName, sizeof(PathName),  "%s/%ss2", _PATH_DEV_RDSK, DevName);
	if ((FileD = open(PathName, O_RDONLY)) > 0)
	    (void) close(FileD);
	NameListAdd(&UsedList, DevName);
    }

    NameListFree(UsedList);
    (void) closedir(DirPtr);
}

/*
 * Get the system to detect all tape drives on the system. 
 */
static void DetectTapes()
{
    static DIR		       *DirPtr;
    struct dirent	       *DirEnt;
    static char			DevName[MAXPATHLEN];
    static char			PathName[MAXPATHLEN];
    static namelist_t	       *UsedList = NULL;
    register char	       *cp;
    int			        FileD;

    if (!(DirPtr = opendir(_PATH_DEV_RMT))) {
	SImsg(SIM_GERR, "Cannot open directory %s: %s.", 
	      _PATH_DEV_RMT, SYSERR);
	return;
    }

    while (DirEnt = readdir(DirPtr)) {
	if (EQ(DirEnt->d_name, ".") || EQ(DirEnt->d_name, ".."))
	    continue;

	(void) strcpy(DevName, DirEnt->d_name);
	for (cp = DevName; cp && isdigit(*cp); ++cp);
	if (cp)
	    *cp = CNULL;
	if (NameListFind(UsedList, DevName))
	    continue;

	/* 
	 * Always use the "n" device to avoid rewinding the tape.
	 */
	(void) snprintf(PathName, sizeof(PathName),  "%s/%sn", _PATH_DEV_RMT, DevName);
	if ((FileD = open(PathName, O_RDONLY)) > 0)
	    (void) close(FileD);
	NameListAdd(&UsedList, DevName);
    }

    NameListFree(UsedList);
    (void) closedir(DirPtr);
}

/*
 * Get the system to detect the floppy disk drive.
 */
static int DetectFloppy()
{
    static char			DevName[MAXPATHLEN];
    int				FileD;

    (void) snprintf(DevName, sizeof(DevName),  "%s/rdiskette", _PATH_DEV);
    if ((FileD = open(DevName, O_RDONLY)) > 0)
	(void) close(FileD);
}

/*
 * The system does not "see" all devices until something attempts
 * to first use them.  These routines will cause the system to look
 * for and "see" certain devices that are not normally seen after
 * a system boot.
 */
extern void DetectDevices()
{
    DetectDisks();
    DetectTapes();
    DetectFloppy();
}

/*
 * There's no easy way to get this out of /kernel/unix so we
 * do the same thing the kernel does.
 */
extern char *GetKernVerSunOS5()
{
    static char			Buff[128];
    static struct utsname 	un;

    if (uname(&un) == -1)
	return((char *) NULL);

    (void) snprintf(Buff, sizeof(Buff),  
	   "SunOS Release %s Version %s%s [UNIX(R) System V Release 4.0]",
		   un.release, un.version,
#ifdef _LP64
		   " 64-bit"
#else
		   ""
#endif
		   );

    return(Buff);
}

/*
 * Get the OS Dist information by reading the first line of /etc/release
 */
extern char *GetOSDistSunOS5()
{
#if	defined(OS_RELEASE_FILE)
    static char			Buff[256];
    FILE		       *fp;
    register char	       *cp;

    fp = fopen(OS_RELEASE_FILE, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: Cannot open OS release file: %s", 
	      OS_RELEASE_FILE, SYSERR);
	return((char *) NULL);
    }

    if (!fgets(Buff, sizeof(Buff), fp)) {
	SImsg(SIM_GERR, "%s: Read OS release file failed: %s",
	      OS_RELEASE_FILE, SYSERR);
	return((char *) NULL);
    }
    (void) fclose(fp);

    /* Zap trailing newline */
    if (cp = strchr(Buff, '\n')) 
	*cp = CNULL;

    /* Skip leading white space */
    for (cp = Buff; isspace(*cp); ++cp);

    return(cp);
#endif	/* OS_RELEASE_FILE */
}
