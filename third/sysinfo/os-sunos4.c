/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-sunos4.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * SunOS 4.x specific routines
 */

#include "defs.h"
#include <sys/stat.h>
#include <sys/mtio.h>

/*
 * Characters for disk partitions
 */
char 			PartChars[] = "abcdefgh";

/*
 * CPU (model) symbol
 */
char 			CpuSYM[] = "_cpu";

/*
 * Name of generic magnetic tape device.
 */
#define MTNAME		"mt"


/*
 * Generally used variables
 */
static kvm_t 		       *kd = NULL;
static struct stat 		StatBuf;
static DevInfo_t 	       *DevInfo;
static char 			Buf[BUFSIZ];
extern char		        RomVecSYM[];
static int			OpenPROMTraverse();

#if	defined(HAVE_MAINBUS)
/*
 * Build a device tree by searching the MainBus
 */

#define DV_SIZE	(sizeof(struct mb_device))
#define DR_SIZE (sizeof(struct mb_driver))
extern char		 	MainBusSYM[];

/*
 * Build device tree by looking at mainbus (mb) devices
 */
extern int BuildMainBus(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    struct nlist	       *nlptr;
    static struct mb_device 	Device;
    static struct mb_driver 	Driver;
    static char 		CtlrName[BUFSIZ], DevName[BUFSIZ];
    static DevData_t 		DevData;
    u_long 			Addr, DeviceAddr;
    DevInfo_t 		       *DevInfo;

    /*
     * Read table address from kernel
     */
    if (!(kd = KVMopen()))
	return(-1);

    if ((nlptr = KVMnlist(kd, MainBusSYM, (struct nlist *)NULL, 0)) == NULL)
	return(-1);

    if (CheckNlist(nlptr))
	return(-1);

    /*
     * Read each device table entry.  A NULL device.mb_driver
     * indicates that we're at the end of the table.
     */
    for (DeviceAddr = nlptr->n_value; DeviceAddr; 
	 DeviceAddr += DV_SIZE) {

	/*
	 * Read this device
	 */
	if (KVMget(kd, DeviceAddr, (char *) &Device, DV_SIZE, KDT_DATA)) {
	    if (Debug) 
		Error("Cannot read mainbus device from address 0x%x.", 
		      DeviceAddr);
	    KVMclose(kd);
	    return(-1);
	}

	/*
	 * See if we're done.
	 */
	if (!Device.md_driver)
	    break;

	/*
	 * Read the driver structure
	 */
	Addr = (u_long) Device.md_driver;
	if (KVMget(kd, Addr, (char *) &Driver, DR_SIZE, KDT_DATA)) {
	    if (Debug) 
		Error("Cannot read driver for mainbus address 0x%x.", Addr);
	    continue;
	}

	/*
	 * Get the device name
	 */
	if (Addr = (u_long) Driver.mdr_dname) {
	    if (KVMget(kd, Addr, (char *) DevName, 
		       sizeof(DevName), KDT_STRING)) {
		if (Debug)
		    Error("Cannot read device name from address 0x%x.", Addr);
		continue;
	    }
	} else
	    DevName[0] = CNULL;

	/*
	 * Get the controller name
	 * XXX - not if "Device.md_ctlr" is -1; work around botch
	 * in current Auspex releases, where some boards (File Processor,
	 * Primary Memory, etc.) have both a device and a controller name,
	 * despite the fact that there's not both a controller and a
	 * set of 1 or more devices.
	 */
	if ((Addr = (u_long) Driver.mdr_cname) && Device.md_ctlr != -1) {
	    if (KVMget(kd, Addr, (char *) CtlrName, 
		       sizeof(CtlrName), KDT_STRING)) {
		if (Debug)
		    Error("Cannot read controller name from address 0x%x.", 
			  Addr);
		continue;
	    }
	} else
	    CtlrName[0] = CNULL;

	/* Make sure devdata is clean */
	bzero(&DevData, sizeof(DevData_t));

	/* Set what we know */
	if (DevName[0]) {
	    DevData.DevName = strdup(DevName);
	    DevData.DevUnit = Device.md_unit;
	}
	if (CtlrName[0]) {
	    DevData.CtlrName = strdup(CtlrName);
	    DevData.CtlrUnit = Device.md_ctlr;
	}
	/* 
	 * Mainbus devices such, as SCSI targets, may not exist
	 * but the controller reports them as present
	 */
	if (Device.md_alive)
	    DevData.Flags |= DD_MAYBE_ALIVE;

	if (Debug)
	    printf("MainBus: Found \"%s\" (Unit %d) on \"%s\" (Unit %d) %s\n", 
		   DevData.DevName, DevData.DevUnit,
		   DevData.CtlrName, DevData.CtlrUnit,
		   (DevData.Flags & DD_MAYBE_ALIVE) ? "[MAYBE-ALIVE]" : "");

	/* Probe and add device */
	if (DevInfo = ProbeDevice(&DevData, TreePtr, SearchNames))
	    AddDevice(DevInfo, TreePtr, (char **)NULL);
    }

    KVMclose(kd);
    return(0);
}
#endif	/* HAVE_MAINBUS */

/*
 * Get disk info structure.
 */
static DKinfo *GETdk_info(d, file)
    int 			d;
    char 		       *file;
{
    static DKinfo 		dk_info;

    if (ioctl(d, DKIOCINFO, &dk_info) < 0) {
	if (Debug) Error("%s: DKIOCINFO: %s.", file, SYSERR);
	return(NULL);
    }

    return(&dk_info);
}

/*
 * Get disk configuration structure.
 */
static DKconf *GETdk_conf(d, file, disktype)
    int 			d;
    char 		       *file;
    int				disktype;
{
    static DKconf 		dk_conf;

    if (disktype == DKT_CDROM) {
	if (Debug) 
	    Error("%s: Get CDROM disk configuration info is not supported.",
		  file);
	return((DKconf *) NULL);
    }

    if (ioctl(d, DKIOCGCONF, &dk_conf) < 0) {
	if (Debug) Error("%s: DKIOCGCONF: %s.", file, SYSERR);
	return((DKconf *) NULL);
    }

    return(&dk_conf);
}

/*
 * Get disk geometry structure.
 */
static DKgeom *GETdk_geom(d, file, disktype)
    int 			d;
    char 		       *file;
    int				disktype;
{
    static DKgeom 		dk_geom;

    if (disktype == DKT_CDROM) {
	if (Debug) 
	    Error("%s: Get CDROM disk geometry info is not supported.", file);
	return((DKgeom *) NULL);
    }

    if (ioctl(d, DKIOCGGEOM, &dk_geom) < 0) {
	if (Debug) Error("%s: DKIOCGGEOM: %s.", file, SYSERR);
	return((DKgeom *) NULL);
    }

    return(&dk_geom);
}

/*
 * Get disk type structure.
 */
static DKtype *GETdk_type(d, file)
    int 			d;
    char 		       *file;
{
    static DKtype 		dk_type;

    if (ioctl(d, DKIOCGTYPE, &dk_type) < 0) {
	if (errno != ENOTTY)
	    if (Debug) Error("%s: DKIOCGTYPE: %s.", file, SYSERR);
	return(NULL);
    }

    return(&dk_type);
}

/*
 * Check the checksum of a disklabel.
 */
static int DkLblCheckSum(DkLabel)
    DKlabel 		       *DkLabel;
{
    register short 	       *Ptr, Sum = 0;
    register short 		Count;

    Count = (sizeof (DKlabel)) / (sizeof (short));
    Ptr = (short *)DkLabel;

    /*
     * Take the xor of all the half-words in the label.
     */
    while (Count--)
	Sum ^= *Ptr++;

    /*
     * The total should be zero for a correct checksum
     */
    return(Sum);
}

/*
 * Get label information from label on disk.
 * The label is stored in the first sector of the disk.
 * We use the driver specific "read" flag with the DKIOCSCMD
 * ioctl to read the first sector.  There should be a special
 * ioctl to just read the label.
 */
static DKlabel *GETdk_label(d, file, dk_info, disktype)
    int 			d;
    char 		       *file;
    DKinfo	 	       *dk_info;
    int				disktype;
{
    struct dk_cmd 		dk_cmd;
    static DKlabel	 	dk_label;
    DevDefine_t		       *DevDefine;

    if (!file || !dk_info)
	return((DKlabel *) NULL);

    /*
     * CDROM's don't support DKIOCSCMD and doing a DKIOCSCMD on
     * a CDROM drive can sometimes crash a system.
     */
    if (disktype == DKT_CDROM) {
	if (Debug) Error("%s: Reading CDROM labels is not supported.", file);
	return((DKlabel *) NULL);
    }

    DevDefine = DevDefGet(NULL, DT_DISKCTLR, dk_info->dki_ctype);
    if (!DevDefine) {
	Error("Controller type %d is unknown.", dk_info->dki_ctype);
	return((DKlabel *) NULL);
    }

    if (DevDefine->DevFlags <= 0) {
	if (Debug)
	    Error("Read block on controller type \"%s\" is unsupported.",
		  DevDefine->Model);
	return((DKlabel *) NULL);
    }

    bzero((char *) &dk_cmd, sizeof(dk_cmd));
    dk_cmd.dkc_cmd = DevDefine->DevFlags;
    dk_cmd.dkc_flags = DK_SILENT | DK_ISOLATE;
    dk_cmd.dkc_blkno = (daddr_t)0;
    dk_cmd.dkc_secnt = 1;
    dk_cmd.dkc_bufaddr = (char *) &dk_label;
    dk_cmd.dkc_buflen = SECSIZE;

    if (ioctl(d, DKIOCSCMD, &dk_cmd) < 0) {
	if (Debug) Error("%s: DKIOCSCMD: %s.", file, SYSERR);
	return((DKlabel *) NULL);
    }

    if (dk_label.dkl_magic != DKL_MAGIC) {
	Error("%s: Disk not labeled.", file);
	return((DKlabel *) NULL);
    }

    if (DkLblCheckSum(&dk_label)) {
	Error("%s: Bad label checksum.", file);
	return((DKlabel *) NULL);
    }

    return(&dk_label);
}

/*
 * Get the name of a disk (i.e. sd0).
 */
static char *GetDiskName(name, dk_conf, dk_info)
    char 		       *name;
    DKconf 		       *dk_conf;
    DKinfo 		       *dk_info;
{
    if (!dk_conf || !dk_info) {
	if (name)
	    return(name);
	return((char *) NULL);
    }

#if	defined(DKI_HEXUNIT)
    if (FLAGS_ON(dk_info->dki_flags, DKI_HEXUNIT))
	(void) sprintf(Buf, "%s%3.3x", dk_conf->dkc_dname, dk_conf->dkc_unit);
    else
#endif 	/* DKI_HEXUNIT */
	(void) sprintf(Buf, "%s%d", dk_conf->dkc_dname, dk_conf->dkc_unit);

    return(strdup(Buf));
}

/*
 * Get the name of the controller for a disk.
 */
static char *GetDiskCtlrName(dk_conf)
    DKconf 		       *dk_conf;
{
    if (!dk_conf)
	return((char *) NULL);

    (void) sprintf(Buf, "%s%d", dk_conf->dkc_cname, dk_conf->dkc_cnum);

    return(strdup(Buf));
}

/*
 * Get a disk controller device from disk info.
 */
static DevInfo_t *GetDiskCtlrDevice(DevData, dk_info, dk_conf)
    DevData_t 		       *DevData;
    DKinfo	 	       *dk_info;
    DKconf 		       *dk_conf;
{
    DevInfo_t 		       *MkMasterFromDevData();
    DevInfo_t 		       *DiskCtlr;

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

    if (dk_conf) {
	if (!DiskCtlr->Name) {
	    DiskCtlr->Name = GetDiskCtlrName(dk_conf);
	    DiskCtlr->Unit = dk_conf->dkc_cnum;
	}
	DiskCtlr->Addr = dk_conf->dkc_addr;
	DiskCtlr->Prio = dk_conf->dkc_prio;
	DiskCtlr->Vec = dk_conf->dkc_vec;
    }

    if (dk_info)
	SetDiskCtlrModel(DiskCtlr, dk_info->dki_ctype);

    return(DiskCtlr);
}

/*
 * Get disk label info from the extracted dk_label info.
 */
static char *GetDiskLabel(dk_label)
    DKlabel 		       *dk_label;
{
    register char 	       *cp;
    char		       *label;

    if (!dk_label)
	return((char *) NULL);

    label = strdup(dk_label->dkl_asciilabel);

    /*
     * The label normally has geometry information in it we don't want
     * to see, so we trim out anything starting with " cyl".
     */
    for (cp = label; cp && *cp; ++cp)
	if (*cp == ' ' && strncasecmp(cp, " cyl", 4) == 0)
	    *cp = CNULL;

    return(label);
}

/*
 * Get filesystem mount info for a partition.
 */
static char *GetMountInfo(name, part)
    char 		       *name;
    char 		       *part;
{
    static FILE 	       *mountedFP = NULL;
    static FILE 	       *mnttabFP = NULL;
    struct mntent 	       *mntent;
    char 		       *file;

    if (!name)
	return((char *) NULL);

    file = GetCharFile(name, part);

    /*
     * First try currently mounted filesystems (/etc/mtab)
     */
    if (!mountedFP) {
	if ((mountedFP = setmntent(MOUNTED, "r")) == NULL) {
	    Error("%s: Cannot open for reading: %s.", MOUNTED, SYSERR);
	    return(NULL);
	}
    } else
	rewind(mountedFP);

    while (mntent = getmntent(mountedFP))
	if (mntent->mnt_fsname && EQ(mntent->mnt_fsname, file))
	    return(mntent->mnt_dir);

    /*
     * Now try static information (/etc/fstab)
     */
    if (!mnttabFP) {
	if ((mnttabFP = setmntent(MNTTAB, "r")) == NULL) {
	    Error("%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(mnttabFP);

    while (mntent = getmntent(mnttabFP))
	if (mntent->mnt_fsname && EQ(mntent->mnt_fsname, file))
	    return(mntent->mnt_dir);

    return((char *) NULL);
}

/*
 * Extract the disk partition info from a disk.
 */
static DiskPart_t *ExtractDiskPart(name, part, dk_conf, dk_geom)
    char 		       *name;
    char 		       *part;
    DKconf	 	       *dk_conf;
    DKgeom	 	       *dk_geom;
{
    static DiskPart_t 		DiskPart;
    struct dk_map 		dk_map;
    char 		       *file;
    char 		       *p;
    int 			d;

    if (!name || !dk_conf || !dk_geom)
	return((DiskPart_t *) NULL);

    file = GetRawFile(name, part);

    if (stat(file, &StatBuf) != 0) {
	if (Debug) Error("%s: No such partition.", file);
	return((DiskPart_t *) NULL);
    }

    if ((d = open(file, O_RDONLY)) < 0) {
	if (Debug)
	    Error("%s: Cannot open for read: %s.", file, SYSERR);
	return((DiskPart_t *) NULL);
    }

    if (ioctl(d, DKIOCGPART, &dk_map) != 0) {
	Error("%s: Cannot extract partition info: %s.", 
		file, SYSERR);
	return((DiskPart_t *) NULL);
    }
 
    (void) close(d);

    /*
     * Skip empty partitions
     */
    if (!dk_map.dkl_nblk) {
	if (Debug) Error("%s: partition has no size.", file);
	return((DiskPart_t *) NULL);
    }

    bzero((char *) &DiskPart, sizeof(DiskPart_t));

    DiskPart.Name = strdup(part);

    if (p = GetMountInfo(name, part))
	DiskPart.Usage = strdup(p);
    /* 
     * If this is the "b" partition on the root device, 
     *  then assume it's swap 
     */
    else if (dk_conf->dkc_unit == 0 && strcmp(part, "b") == 0)
	DiskPart.Usage = "swap";

    DiskPart.StartSect = dk_map.dkl_cylno *
	(dk_geom->dkg_nhead * dk_geom->dkg_nsect);
    DiskPart.NumSect = dk_map.dkl_nblk;

    return(&DiskPart);
}

/*
 * Translate disk partition information from basic
 * extracted disk info.
 */
static DiskPart_t *GetDiskPart(name, dk_conf, dk_geom)
    char 		       *name;
    DKconf 		       *dk_conf;
    DKgeom 		       *dk_geom;
{
    extern char 		PartChars[];
    register DiskPart_t        *Ptr;
    DiskPart_t		       *DiskPart;
    DiskPart_t 		       *Base = NULL;
    register int 		i;
    static char 		pname[2];

    if (!name || !dk_conf || !dk_geom)
	return((DiskPart_t *) NULL);

    pname[1] = CNULL;
    for (i = 0; PartChars[i]; ++i) {
	pname[0] = PartChars[i];
	if (DiskPart = ExtractDiskPart(name, pname, dk_conf, dk_geom)) {
	    if (Base) {
		for (Ptr = Base; Ptr && Ptr->Next; Ptr = Ptr->Next);
		Ptr->Next = NewDiskPart(DiskPart);
	    } else {
		Base = NewDiskPart(DiskPart);
	    }
	}
    }

    return(Base);
}

/*
 * Convert all we've learned about a disk to a DevInfo_t.
 */
static DevInfo_t *dkToDevInfo(name, disktype, DevData,
			      dk_info, dk_label, dk_conf, dk_geom, dk_type)
    char 		       *name;
    int				disktype;
    DevData_t 		       *DevData;
    DKinfo	 	       *dk_info;
    DKlabel	 	       *dk_label;
    DKconf 		       *dk_conf;
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
	Error("Cannot create new DiskCtlr device entry.");
	return((DevInfo_t *) NULL);
    }

    if ((DiskDrive = NewDiskDrive(NULL)) == NULL) {
	Error("Cannot create new DiskDrive entry.");
	return((DevInfo_t *) NULL);
    }

    DevInfo->Name 		= GetDiskName(name, dk_conf, dk_info);
    DevInfo->Type 		= DT_DISKDRIVE;
    /*
     * Only read partition info we we're going to print it later.
     */
    if (VL_ALL)
	DiskDrive->DiskPart 	= GetDiskPart(name, dk_conf, dk_geom);
    DiskDrive->Label 	= GetDiskLabel(dk_label);
    DevInfo->Model 		= DiskDrive->Label;

    if (disktype == DKT_CDROM && DiskDrive->Label == NULL)
	DevInfo->Model 		= "CD-ROM";
	
    if (dk_conf) {
	DiskDrive->Unit 	= dk_conf->dkc_unit;
	DiskDrive->Slave 	= dk_conf->dkc_slave;;
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
    if (dk_type) {
	DiskDrive->PhySect 	= dk_type->dkt_hsect;
	DiskDrive->PROMRev 	= dk_type->dkt_promrev;
    }
    if (dk_info) {
#if	defined(DKI_HEXUNIT)
	if (FLAGS_ON(dk_info->dki_flags, DKI_HEXUNIT))
	    DiskDrive->Flags |= DF_HEXUNIT;
#endif 	/* DKI_HEXUNIT */
    }
    DiskDrive->SecSize 	= SECSIZE;

    DiskCtlr 			= GetDiskCtlrDevice(DevData, dk_info, dk_conf);

    DevInfo->DevSpec 		= (caddr_t *) DiskDrive;
    DevInfo->Master 		= DiskCtlr;

    return(DevInfo);
}

/*
 * Query and learn about a disk.
 */
extern DevInfo_t *ProbeDiskDriveGeneric(disktype, name, DevData, DevDefine)
    /*ARGSUSED*/
    int				disktype;
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    DevInfo_t 		       *DevInfo;
    DKinfo 		       *dk_info = NULL;
    DKconf 		       *dk_conf = NULL;
    DKtype 		       *dk_type = NULL;
    DKlabel 		       *dk_label = NULL;
    DKgeom 		       *dk_geom = NULL;
    char 		       *rfile;
    int 			d;

    if (!name)
	return((DevInfo_t *) NULL);

#if	defined(HAVE_IPI)
    /*
     * XXX - Kludge for IPI "id" disks.
     */
    if (EQ(DevData->DevName, "id")) {
	static char		Buf[BUFSIZ];

	(void) sprintf(Buf, "%s%3.3x", 
		       DevData->DevName, DevData->DevUnit);
	name = Buf;
    }
#endif	/* HAVE_IPI */

    if (disktype == DKT_CDROM)
	rfile = GetRawFile(name, NULL);
    else {
	if (stat(rfile = GetRawFile(name, NULL), &StatBuf) != 0)
	    /*
	     * Get the name of the whole disk raw device.
	     */
	    rfile = GetRawFile(name, "c");
    }

    if ((d = open(rfile, O_RDONLY)) < 0) {
	if (Debug) Error("%s: Cannot open for reading: %s.", rfile, SYSERR);
	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if (errno == EBUSY || errno == EIO ||
	    ((DevDefine->Model || DevDefine->Desc) &&
	     FLAGS_ON(DevData->Flags, DD_IS_ALIVE))) {
	    DevInfo = NewDevInfo((DevInfo_t *) NULL);
	    DevInfo->Name = strdup(name);
	    DevInfo->Unit = DevData->DevUnit;
	    DevInfo->Master = MkMasterFromDevData(DevData);
	    DevInfo->Type = DT_DISKDRIVE;
	    DevInfo->Model = DevDefine->Model;
	    DevInfo->ModelDesc = DevDefine->Desc;
	    return(DevInfo);
	} else
	    return((DevInfo_t *) NULL);
    }

    if ((dk_conf = GETdk_conf(d, rfile, disktype)) == NULL)
	if (Debug) Error("%s: get dk_conf failed.", rfile);

    if ((dk_info = GETdk_info(d, rfile)) == NULL)
	if (Debug) Error("%s: get dk_info failed.", rfile);

    if ((dk_geom = GETdk_geom(d, rfile, disktype)) == NULL)
	if (Debug) Error("%s: get dk_geom failed.", rfile);

    if ((dk_label = GETdk_label(d, rfile, dk_info, disktype)) == NULL)
	if (Debug) Error("%s: get dk_label failed.", rfile);

    /*
     * Not all controllers support dk_type
     */
    dk_type = GETdk_type(d, rfile);

    close(d);

    if (!(DevInfo = dkToDevInfo(name, disktype, DevData,
				dk_info, dk_label, 
				dk_conf, dk_geom, dk_type))) {
	Error("%s: Cannot convert DiskDrive information.", name);
	return((DevInfo_t *) NULL);
    }

    return(DevInfo);
}

/*
 * Probe normal disk drive by calling Generic probe routine.
 */
extern DevInfo_t *ProbeDiskDrive(name, DevData, DevDefine)
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    return(ProbeDiskDriveGeneric(DKT_GENERIC, name, DevData, DevDefine));
}

/*
 * Probe CDROM disk drive by calling Generic probe routine.
 */
extern DevInfo_t *ProbeCDROMDrive(name, DevData, DevDefine)
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    return(ProbeDiskDriveGeneric(DKT_CDROM, name, DevData, DevDefine));
}

/*
 * Probe a tape device
 * XXX - this loses if somebody's using the tape, as tapes are exclusive-open
 * devices, and our open will therefore fail.
 * This also loses if there's no tape in the drive, as the open will fail.
 * The above probably applies to most other flavors of UNIX.
 */
extern DevInfo_t *ProbeTapeDrive(name, DevData, DevDefine)
     /*ARGSUSED*/
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
    DevInfo_t 		       *DevInfo;
    char 		       *file;
    char 		       *Model = NULL;
    char			rfile[BUFSIZ];
    static char 		Buf[BUFSIZ];
    struct mtget 		mtget;
    register char	       *cp;
    int 			d;

    /*
     * Don't use GetRawFile; that'll just stick an "r" in front of the
     * device name, meaning it'll return the rewind-on-close device.
     * Somebody may have left the tape positioned somewhere other than
     * at the BOT to, for example, write a dump there later in the
     * evening; it'd be rather rude to reposition it out from under them.
     *
     * The above probably applies to most other flavors of UNIX.
     */
    if (!name)
	file = NULL;
    else {
	(void) sprintf(rfile, "/dev/nr%s", name);
	file = rfile;
    }

    if ((d = open(file, O_RDONLY)) < 0) {
	if (Debug)
	    Error("%s Cannot open for read: %s.", file, SYSERR);

	/*
	 * --RECURSE--
	 * If we haven't tried the "mt" name yet, try it now
	 */
	if (strncmp(name, MTNAME, strlen(MTNAME)) != 0) {
	    (void) sprintf(Buf, "%s%d", MTNAME, DevData->DevUnit);
	    DevInfo = ProbeTapeDrive(Buf, DevData, DevDefine);
	    if (DevInfo)
		return(DevInfo);
	}

	/*
	 * If we know for sure this drive is present and we
	 * know something about it, then create a minimal device.
	 */
	if ((DevDefine->Model || DevDefine->Desc) &&
	    FLAGS_ON(DevData->Flags, DD_IS_ALIVE)) {
	    DevInfo = NewDevInfo((DevInfo_t *) NULL);
	    /* 
	     * Recreate name from devdata since we might have had to
	     * call ourself with name "rmt?"
	     */
	    (void) sprintf(Buf, "%s%d", DevData->DevName, 
			   DevData->DevUnit);
	    DevInfo->Name = strdup(Buf);
	    DevInfo->Unit = DevData->DevUnit;
	    DevInfo->Master = MkMasterFromDevData(DevData);
	    DevInfo->Type = DT_TAPEDRIVE;
	    DevInfo->Model = DevDefine->Model;
	    DevInfo->ModelDesc = DevDefine->Desc;
	    return(DevInfo);
	} else
	    return((DevInfo_t *) NULL);
    }

    if (ioctl(d, MTIOCGET, &mtget) != 0) {
	Error("%s: Cannot extract tape status: %s.", file, SYSERR);
	return((DevInfo_t *) NULL);
    }

    (void) close(d);

    cp = GetTapeModel(mtget.mt_type);
    if (cp)
	Model = strdup(cp);
    else
	Model = "unknown";

    /*
     * Create and set device info
     */
    DevInfo = NewDevInfo(NULL);
    DevInfo->Name = strdup(name);
    DevInfo->Type = DT_TAPEDRIVE;
    if (Model)
	DevInfo->Model = Model;
    else
	DevInfo->Model = DevDefine->Model;
    DevInfo->ModelDesc = DevDefine->Desc;
    DevInfo->Unit = DevData->DevUnit;
    DevInfo->Master = MkMasterFromDevData(DevData);

    return(DevInfo);
}

/*
 * Query and learn about a device attached to an Auspex Storage Processor.
 * They'll show up as "ad" in the Mainbus info structure, but that
 * merely reflects the way the slots are set up in the config file.
 * We need to find out what type of device it is at this particular
 * instant (it's subject to change - perhaps even while we're running,
 * but there's not a heck of a lot we can do about that).
 *
 * We do that by trying it as a CD-ROM first, then as a disk, then as
 * a tape; that loses if it's a tape and somebody's using it, but
 * tapes on most if not all UNIX systems can't be probed by us if
 * somebody's using it.
 * The reason why we try it first as a CD-ROM is that if the CD has a
 * partition table, the Auspex driver lets you open the partitions as
 * if it were a disk.
 */
extern DevInfo_t *ProbeSPDrive(name, DevData, DevDefine)
     /*ARGSUSED*/
    char 		       *name;
    DevData_t 		       *DevData;
    DevDefine_t	     	       *DevDefine;
{
    DevInfo_t 		       *thedevice;
    char			devname[BUFSIZ];

    /*
     * Try it first as a CD-ROM.
     */
    (void) sprintf(devname, "acd%d", DevData->DevUnit);
    DevData->DevName = "acd";
    DevDefine->Model = "CD-ROM";
    if (thedevice = ProbeCDROMDrive(devname, DevData, DevDefine))
	return(thedevice);

    /*
     * Not a CD-ROM.  Try a disk.
     */
    (void) sprintf(devname, "ad%d", DevData->DevUnit);
    DevData->DevName = "ad";
    DevDefine->Model = NULL;
    if (thedevice = ProbeDiskDrive(devname, DevData, DevDefine))
	return(thedevice);

    /*
     * Not a disk.  Try a tape.
     */
    (void) sprintf(devname, "ast%d", DevData->DevUnit);
    DevData->DevName = "ast";
    if (thedevice = ProbeTapeDrive(devname, DevData, DevDefine))
	return(thedevice);

    /*
     * None of the above.  Who knows?
     */
    return((DevInfo_t *) NULL);
}

#if	defined(HAVE_SUNROMVEC)
/*
 * Be backwards compatible with pre-4.1.2 code
 */
#include <mon/sunromvec.h>
#if	defined(OPENPROMS) && !(defined(ROMVEC_VERSION) && \
				(ROMVEC_VERSION == 0 || ROMVEC_VERSION == 1))
#define v_mon_id op_mon_id
#endif
#endif	/* HAVE_SUNROMVEC */

/*
 * Get ROM Version number
 *
 * If "romp" is "defined" (in <mon/sunromvec.h>), then take that
 * as the address of the kernel pointer to "rom" (struct sunromvec).
 * Otherwise, nlist "romp" from the kernel.
 */
extern char *GetRomVerSun()
{
    static char			RomRev[16];
    struct nlist	       *nlptr;
#if	defined(HAVE_SUNROMVEC)
    static struct sunromvec	Rom;
    kvm_t		       *kd;
#if	!defined(romp)
    struct sunromvec	       *romp;

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

#else	/* romp */

    if (!(kd = KVMopen()))
	return((char *) NULL);

#endif	/* romp */

    /*
     * Read the sunromvec structure from the kernel
     */
    /*SUPPRESS 25*/
    if (KVMget(kd, (u_long) romp, (char *) &Rom, 
	       sizeof(struct sunromvec), KDT_DATA)) {
	if (Debug) Error("Cannot read sunromvec from kernel.");
	return((char *) NULL);
    }

#if	!defined(romp)

    /*
     * XXX Hardcoded values
     */
    (void) sprintf(RomRev, "%d.%d", Rom.v_mon_id >> 16, Rom.v_mon_id & 0xFFFF);

#else	/* romp */

    /*
     * Read the version string from the address indicated by Rom.v_mon_id.
     */
    if (KVMget(kd, (u_long) Rom.v_mon_id, RomRev, 
	       sizeof(RomRev), KDT_STRING)) {
	if (Debug) Error("Cannot read rom revision from kernel.");
	return((char *) NULL);
    }
#endif	/* romp */

    KVMclose(kd);

#endif	/* HAVE_SUNROMVEC */

    return((RomRev[0]) ? RomRev : (char *) NULL);
}
