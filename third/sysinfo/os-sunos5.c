/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-sunos5.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
#endif

/*
 * SunOS 5.x specific routines
 */

#include "defs.h"
#include <dirent.h>
#include <sys/stat.h>

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
static int GetDeviceFile(DevData, DevDefine, DevDir, ArgvPtr)
    DevData_t 		       *DevData;
    DevDefine_t		       *DevDefine;
    char		       *DevDir;
    char		     ***ArgvPtr;
{
    static DIR		       *DirPtr = NULL;
    struct dirent	       *DirEnt;
    static char			Path[MAXPATHLEN];
    static char			LastDir[MAXPATHLEN];
    register int		i;
    int				Argc;
    char		      **Argv = NULL;
    static char		      **FileArgv = NULL;
    static int			FileArgc = 0;
    char		       *File;

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
	Argc = StrToArgv(DevDefine->File, " ", &Argv);
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
	    if (File[0] != '/')
		(void) sprintf(Path, "%s/", DevDir);
	    else
		Path[0] = CNULL;
	    if (strchr(File, '%') && DevData->DevUnit >= 0)
		(void) sprintf(Path + strlen(Path), File, DevData->DevUnit);
	    else
		(void) strcat(Path, File);

	    if (stat(Path, &StatBuf) == 0)
		FileArgv[FileArgc++] = strdup(Path);
	    else {
		if (Debug) Error("Stat of %s failed: %s.", Path, SYSERR);
		Argv[i] = NULL;
	    }
	}
	if (Argc)
	    DestroyArgv(&Argv, Argc);
    } else {
	if (!DirPtr || !EQ(DevDir, LastDir)) {
	    if (!(DirPtr = opendir(DevDir))) {
		if (Debug) Error("Cannot open directory \"%s\": %s", 
				 DevDir, SYSERR);
		return(-1);
	    }
	    (void) strcpy(LastDir, DevDir);
	} else
	    rewinddir(DirPtr);

	while (DirEnt = readdir(DirPtr)) {
	    if (EQ(DirEnt->d_name, ".") || EQ(DirEnt->d_name, ".."))
		continue;
	
	    (void) sprintf(Path, "%s/%s", DevDir, DirEnt->d_name);

	    if (stat(Path, &StatBuf) < 0) {
		if (Debug) Error("Stat of %s failed: %s.", Path, SYSERR);
		continue;
	    }

#if	defined(DEBUG_VERBOSE)
	    if (Debug) printf("GetDeviceFile dev '%s' (%d) got '%s' (%d)\n",
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
 * Probe a Tape Drive.
 */
extern DevInfo_t *ProbeTapeDrive(TapeName, DevData, DevDefine)
    /*ARGSUSED*/
    char 		       *TapeName;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
    DevInfo_t 		       *DevInfo;
    char 		       *DevFile;
    char 		       *DevName;
    char 		       *Model = NULL;
    char		      **FileList;
    int				FileNum;
    static char 		DevFilen[BUFSIZ];
    struct mtget 		mtget;
    register char	       *cp;
    int 			fd;

    FileNum = GetDeviceFile(DevData, DevDefine, _PATH_DEV_RMT, &FileList);
    if (FileNum < 1) {
	if (Debug) Error("Cannot find tape device for `%s'", TapeName);
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
    (void) sprintf(DevFilen, "%sn", DevFile);

    if ((fd = open(DevFilen, O_RDONLY)) < 0) {
	if (Debug)
	    Error("%s Cannot open for read: %s.", DevFilen, SYSERR);

	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if ((DevDefine->Model || DevDefine->Desc) &&
	    FLAGS_ON(DevData->Flags, DD_IS_ALIVE)) {
	    DevInfo 			= NewDevInfo((DevInfo_t *) NULL);
	    DevInfo->Name 		= strdup(DevName);
	    DevInfo->AltName 		= strdup(TapeName);
	    DevInfo->Unit 		= DevData->DevUnit;
	    DevInfo->Master 		= MkMasterFromDevData(DevData);
	    DevInfo->Type 		= DT_TAPEDRIVE;
	    DevInfo->NodeID		= DevData->NodeID;
	    DevInfo->Model 		= DevDefine->Model;
	    DevInfo->ModelDesc		= DevDefine->Desc;
	    return(DevInfo);
	} else
	    return((DevInfo_t *) NULL);
    }

    if (ioctl(fd, MTIOCGET, &mtget) != 0) {
	Error("%s: Cannot extract tape status: %s.", DevName, SYSERR);
	return((DevInfo_t *) NULL);
    }

    (void) close(fd);

    cp = GetTapeModel(mtget.mt_type);
    if (cp)
	Model = strdup(cp);
    else
	Model = "unknown";

    /*
     * Create and set device info
     */
    DevInfo 			= NewDevInfo(NULL);
    DevInfo->Name 		= strdup(DevName);
    DevInfo->AltName 		= strdup(TapeName);
    DevInfo->Type 		= DT_TAPEDRIVE;
    DevInfo->NodeID		= DevData->NodeID;
    if (Model)
	DevInfo->Model 		= Model;
    else
	DevInfo->Model 		= DevDefine->Model;
    DevInfo->Unit 		= DevData->DevUnit;
    DevInfo->ModelDesc 		= DevDefine->Desc;
    DevInfo->Master 		= MkMasterFromDevData(DevData);

    return(DevInfo);
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
	if (Debug) Error("%s: DKIOCGVTOC: %s.", DevFile, SYSERR);
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
	if (Debug) Error("%s: DKIOCINFO: %s.", DevFile, SYSERR);
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
	if (Debug) Error("%s: DKIOCGGEOM: %s.", DevFile, SYSERR);
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
	    if (Debug) Error("%s: DKIOCGTYPE: %s.", file, SYSERR);
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
    static char			Name[BUFSIZ];
    static char			Path[BUFSIZ];
    register char	       *cp;

    if (!DevName || !PartName)
	return((char *) NULL);

    (void) sprintf(Name, "%s%s", DevName, PartName);

    /*
     * First try the current mount table
     */
    if (!mntFilePtr) {
	if ((mntFilePtr = fopen(MNTTAB, "r")) == NULL) {
	    Error("%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
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
	    Error("%s: Cannot open for reading: %s.", VFSTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(vfstabFilePtr);

    (void) sprintf(Path, "%s/%s", _PATH_DEV_DSK, Name);

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
    register DiskPart_t	       *pdp;
    register ushort 		i;
    static char 		pname[3];
    DiskPart_t 		       *Base = NULL;
    DiskPart_t		       *DiskPart;

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
	(void) sprintf(pname, "s%d", i);
	DiskPart = (DiskPart_t *) xcalloc(1, sizeof(DiskPart_t));
	DiskPart->Name = strdup(pname);
	DiskPart->Usage = GetMountInfo(DevName, pname);
	DiskPart->StartSect = dk_vtoc->v_part[i].p_start;
	DiskPart->NumSect = dk_vtoc->v_part[i].p_size;

	if (Base) {
	    for (pdp = Base; pdp && pdp->Next; pdp = pdp->Next);
	    pdp->Next = DiskPart;
	} else
	    Base = DiskPart;
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
    char		       *Label;

    if (!dk_vtoc)
	return((char *)NULL);

    Label = strdup(dk_vtoc->v_asciilabel);

    /*
     * The label normally has geometry information in it we don't want
     * to see, so we trim out anything starting with " cyl".
     */
    for (cp = Label; cp && *cp; ++cp)
	if (*cp == ' ' && strncasecmp(cp, " cyl", 4) == 0)
	    *cp = CNULL;

    return(Label);
}

/*
 * Get the name of the controller for a disk.
 */
static char *GetDkCtlrName(dk_cinfo)
    DKcinfo 		       *dk_cinfo;
{
    if (!dk_cinfo)
	return((char *) NULL);

    (void) sprintf(Buff, "%s%d", dk_cinfo->dki_cname, dk_cinfo->dki_cnum);

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
static DevInfo_t *DKtoDiskDrive(DevName, DevData,
				dk_vtoc, dk_cinfo, dk_geom, dk_type)
    char 		       *DevName;
    DevData_t 		       *DevData;
    DKvtoc		       *dk_vtoc;
    DKcinfo	 	       *dk_cinfo;
    DKgeom 		       *dk_geom;
    DKtype 		       *dk_type;
{
    DevInfo_t 		       *DevInfo, *DiskCtlr;
    DiskDrive_t 		       *DiskDrive;

    if ((DevInfo = NewDevInfo(NULL)) == NULL) {
	Error("Cannot create new device entry.");
	return((DevInfo_t *) NULL);
    }

    if ((DiskCtlr = NewDevInfo(NULL)) == NULL) {
	Error("Cannot create new dkctlr device entry.");
	return((DevInfo_t *) NULL);
    }

    if ((DiskDrive = NewDiskDrive(NULL)) == NULL) {
	Error("Cannot create new DiskDrive entry.");
	return((DevInfo_t *) NULL);
    }

    (void) sprintf(Buff, "%s%d", DevData->DevName, DevData->DevUnit);
    DevInfo->AltName 		= strdup(Buff);
    DevInfo->Name 		= DevName;
    DevInfo->Type 		= DT_DISKDRIVE;
    DevInfo->NodeID 		= DevData->NodeID;

    /*
     * Only read partition info we we're going to print it later.
     */
    if (VL_ALL)
	DiskDrive->DiskPart 	= GetDiskPart(DevName, dk_vtoc);
    DiskDrive->Label 	= GetDiskLabel(dk_vtoc);
    DevInfo->Model 		= DiskDrive->Label;

    if (dk_cinfo) {
	DiskDrive->Unit 	= dk_cinfo->dki_unit;
	DiskDrive->Slave 	= dk_cinfo->dki_slave;;
    }
    if (dk_geom) {
	DiskDrive->DataCyl 	= dk_geom->dkg_ncyl;
	DiskDrive->PhyCyl 	= dk_geom->dkg_pcyl;
	DiskDrive->AltCyl 	= dk_geom->dkg_acyl;
	DiskDrive->Heads 	= dk_geom->dkg_nhead;
	DiskDrive->Sect 	= dk_geom->dkg_nsect;
	DiskDrive->APC 	= dk_geom->dkg_apc;
	DiskDrive->RPM 	= dk_geom->dkg_rpm;
	DiskDrive->IntrLv 	= dk_geom->dkg_intrlv;
    }
#if	defined(HAVE_HDIO)
    if (dk_type) {
	DiskDrive->PhySect 	= dk_type->hdkt_hsect;
	DiskDrive->PROMRev 	= dk_type->hdkt_promrev;
    }
#endif	/* HAVE_HDIO */
    DiskDrive->SecSize 	= dk_vtoc->v_sectorsz;

    if (dk_vtoc->v_volume[0]) {
	(void) sprintf(Buff, "\"%.*s\"", 
		       sizeof(dk_vtoc->v_volume), dk_vtoc->v_volume);
	AddDevDesc(DevInfo, Buff, "Volume Name", DA_APPEND);
    }

    DiskCtlr 			= GetDkCtlrDevice(DevData, dk_cinfo);

    DevInfo->DevSpec 		= (caddr_t *) DiskDrive;
    DevInfo->Master 		= DiskCtlr;

    return(DevInfo);
}

/*
 * Probe a specific disk drive as file DevFile.
 */
static DevInfo_t *_ProbeDiskDrive(DiskName, DevFile, DevData, DevDefine)
    /*ARGSUSED*/
    char 		       *DiskName;
    char 		       *DevFile;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    DevInfo_t 		       *DevInfo;
    DKcinfo 		       *dk_cinfo;
    DKgeom 		       *dk_geom;
    DKtype 		       *dk_type;
    DKvtoc 		       *dk_vtoc;
    char		       *DevName;
    char		       *cp;
    int				Len;
    int				fd;

    if (!DevFile)
	return((DevInfo_t *)NULL);

    /* Use basename of DevInfo file for virtual disk device name */
    DevName = strdup(DevFile);
    if (cp = strrchr(DevName, '/'))
	DevName = ++cp;
    /* Zap slice part of name */
    if (cp = strrchr(DevName, 's'))
	*cp = CNULL;

    /*
     * Try opening the disk device.  Usually this will be the "s0" device.
     * If that fails, and then try opening "s2".  Sometimes there's no "s0"
     * partition, but there is an "s2".
     */
    fd = open(DevFile, O_RDONLY);
    if (fd < 0) {
	Len = strlen(DevFile);
	if (DevFile[Len-1] == '0') {
	    DevFile[Len-1] = '2';
	    fd = open(DevFile, O_RDONLY);
	}
    }
    if (fd < 0) {
	if (Debug) Error("%s: Cannot open for reading: %s.", DevFile, SYSERR);
	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if (errno == EBUSY || 
	    ((DevDefine->Model || DevDefine->Desc || DevData->DevNum > 0) &&
	     FLAGS_ON(DevData->Flags, DD_IS_ALIVE))) {
	    DevInfo = NewDevInfo((DevInfo_t *) NULL);
	    /* The DiskName is probably best if we used File */
	    if (DevDefine->File) {
		DevInfo->Name = strdup(DiskName);
		DevInfo->AltName = strdup(DevName);
	    } else {
		DevInfo->Name = strdup(DevName);
		DevInfo->AltName = strdup(DiskName);
	    }
	    DevInfo->Unit = DevData->DevUnit;
	    DevInfo->Master = MkMasterFromDevData(DevData);
	    DevInfo->Type = DT_DISKDRIVE;
	    DevInfo->NodeID = DevData->NodeID;
	    DevInfo->Model = DevDefine->Model;
	    DevInfo->ModelDesc = DevDefine->Desc;
	    return(DevInfo);
	} else
	    return((DevInfo_t *) NULL);
    }

    if ((dk_vtoc = GETvtoc(fd, DevFile)) == NULL)
	if (Debug) Error("%s: get vtoc failed.", DevFile);
    if ((dk_cinfo = GETdk_cinfo(fd, DevFile)) == NULL)
	if (Debug) Error("%s: get dk_cinfo failed.", DevFile);
    if ((dk_geom = GETdk_geom(fd, DevFile)) == NULL)
	if (Debug) Error("%s: get dk_geom failed.", DevFile);
#if	defined(HAVE_HDIO)
    if ((dk_type = GETdk_type(fd, DevFile)) == NULL)
	if (Debug) Error("%s: no dk_type info available.", DevFile);
#endif	/* HAVE_HDIO */

    close(fd);

    if (!(DevInfo = DKtoDiskDrive(DevName, DevData,
				  dk_vtoc, dk_cinfo, dk_geom, dk_type))) {
	Error("%s: Cannot convert DiskDrive information.", DevName);
	return((DevInfo_t *) NULL);
    }

    return(DevInfo);
}

/*
 * Probe a disk drive by general name (DiskName)
 */
extern DevInfo_t *ProbeDiskDrive(DiskName, DevData, DevDefine)
    /*ARGSUSED*/
    char 		       *DiskName;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    char		      **FileList;
    int				FileCount;
    register int		i;

    if (!DiskName)
	return((DevInfo_t *)NULL);

    FileCount = GetDeviceFile(DevData, DevDefine, _PATH_DEV_RDSK, &FileList);
    if (FileCount < 1) {
	if (Debug) Error("Cannot find disk device file for `%s'.", DiskName);
	return((DevInfo_t *)NULL);
    }

    for (i = 0; i < FileCount; ++i)
	if (DevInfo = _ProbeDiskDrive(DiskName, FileList[i], 
				     DevData, DevDefine))
	    return(DevInfo);

    return((DevInfo_t *)NULL);
}

/*
 * Get hostid
 */
long gethostid()
{
    char 			Buff[BUFSIZ];

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
	Error("Cannot get pagesize from sysconf(): %s", SYSERR);
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
	if (Debug) Error("Cannot read sunromvec pointer from kernel.");
	return((char *) NULL);
    }

    /*
     * Read sunromvec from the kernel
     */
    if (KVMget(kd, (u_long) romp, (char *) &Rom, 
	       sizeof(union sunromvec), KDT_DATA)) {
	if (Debug) Error("Cannot read sunromvec from kernel.");
	return((char *) NULL);
    }

    if (Rom.sunmon.v_mon_id) {
	/*
	 * Read the version string from the address indicated by v_mon_id.
	 * Read 1 byte at a time until '\0' is encountered.
	 */
	if (KVMget(kd, (u_long) Rom.sunmon.v_mon_id, RomRev, 
		   sizeof(RomRev), KDT_STRING)) {
	    if (Debug) Error("Cannot read sunmon rom revision from kernel.");
	    return((char *) NULL);
	}
    }

    if (Rom.obp.op_mon_id && !RomRev[0]) {
	/*
	 * XXX Hardcoded values
	 */
	(void) sprintf(RomRev, "%d.%d", 
		       Rom.obp.op_mon_id >> 16, Rom.obp.op_mon_id & 0xFFFF);
    }

    KVMclose(kd);

#endif	/* HAVE_SUNROMVEC */

    return((RomRev[0]) ? RomRev : (char *) NULL);
}

/*
 * Get amount of physical memory
 */
extern char *GetMemorySunOS5()
{
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
	if (Debug) Error("sysmem failed: %s", SYSERR);
	return((char *) NULL);
    }

    return(GetMemoryStr(DivRndUp((u_long) Amount, MBYTES)));
#else	/* !USE_SYSMEM */
    long long			Amount;
    long			PageSize;
    long			NumPages;

    PageSize = sysconf(_SC_PAGESIZE);
    NumPages = sysconf(_SC_PHYS_PAGES);
    if (PageSize < 0 || NumPages < 0) {
      if (Debug) Error("sysconf() failed: %s", SYSERR);
      return((char *) NULL);
    }
    Amount = (long long) PageSize * NumPages;

    return(GetMemoryStr((int) ((Amount+MBYTES-1) / MBYTES), MBYTES));
#endif	/* USE_SYSMEM */
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
	if (Debug) Error("Cannot open directory %s: %s.", 
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
	(void) sprintf(PathName, "%s/%ss2", _PATH_DEV_RDSK, DevName);
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
	if (Debug) Error("Cannot open directory %s: %s.", 
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
	(void) sprintf(PathName, "%s/%sn", _PATH_DEV_RMT, DevName);
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

    (void) sprintf(DevName, "%s/rdiskette", _PATH_DEV);
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
    static char			Buff[BUFSIZ];
    static struct utsname 	un;

    if (uname(&un) == -1)
	return((char *) NULL);

    (void) sprintf(Buff, 
	   "SunOS Release %s Version %s [UNIX(R) System V Release 4.0]",
		   un.release, un.version);

    return(Buff);
}
