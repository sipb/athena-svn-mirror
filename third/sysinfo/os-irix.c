/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-irix.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
#endif

/*
 * SGI IRIX specific functions
 *
 * Basic device information is derived via getinvent(3) and reverse engineering
 * the IRIX hinv(1m) command which uses getinvent(3).  Many parts of the
 * device code found here remains untested.  Some code is #ifdef disabled
 * because I don't know how some #define's in some files (like sys/invent.h)
 * coorespond to "code" names in other .h files.  This is especially true
 * of the graphics code.
 * 
 * Special Graphics information is derived from getinvent(3), and the
 * "gfx_info" structure, various graphics device specific *.h files
 * and reverse engineering the IRIX /usr/gfx/gfxinfo command.
 */

#include "defs.h"
#include <invent.h>
#include <mntent.h>
#include <sys/systeminfo.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <sys/syssgi.h>
/*
 * Disk
 */
#include <sys/dkio.h>
#include <sys/dvh.h>
/*
 * Graphics
 */
#include <sys/gfx.h>
#include <sys/ng1.h>
#include <sys/gr1new.h>
#include <sys/gr2.h>
#include <sys/lg1hw.h>
#include <sys/lg1.h>
#include <sys/venice.h>

/*
 * These are usually only defined if _KERNEL is defined
 */
#ifndef GR2_TYPE_GR2
#define GR2_TYPE_GR2	0
#endif
#ifndef GR2_TYPE_HI1
#define GR2_TYPE_HI1	1
#endif

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelSGI();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelSGI },
    { GetModelFile },
    { GetModelDef },
    { NULL },
};
extern char			       *GetSerialSGI();
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
    { GetSerialSGI },
    { NULL },
};
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
    { NULL },
};

PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchSysinfo },
    { NULL },
};

PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetAppArchSysinfo },
    { NULL },
};
extern char			       *GetCpuTypeSGI();
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeSGI },
    { NULL },
};
extern char			       *GetNumCpuSGI();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNumCpuSGI },
    { NULL },
};
extern char				*GetKernVerSGI();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerSGI },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameUname },
    { NULL },
};
extern char			       *GetOSVerSGI();
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerSGI },
    { GetOSVerSysinfo },
    { NULL },
};
PSI_t GetManShortPSI[] = {		/* Get Short Man Name */
    { GetManShortSysinfo },
    { GetManShortDef },
    { NULL },
};
extern char 			       *GetManLongSysinfoSGI();
PSI_t GetManLongPSI[] = {		/* Get Long Man Name */
    { GetManLongSysinfoSGI },
    { GetManLongDef },
    { NULL },
};
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryPhysmemSym },
    { NULL },
};
extern char			       *GetVirtMemSGI();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemSGI },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeUtmp },
    { NULL },
};

/* 
 * SGI Get Model
 */
extern char *GetModelSGI()
{
    inventory_t		       *Inv;
    register char	       *cp;
    char		       *CpuType = NULL;
    char		       *CpuBoard = NULL;
    char		       *Graphics = NULL;
    char		       *Video = NULL;
    Define_t		       *Def;
    static char			QueryBuff[BUFSIZ];

    setinvent();

    while (Inv = getinvent()) {
	switch (Inv->inv_class) {
	case INV_PROCESSOR:
	    if (Inv->inv_type == INV_CPUBOARD) {
		Def = DefGet("cpuboard", NULL, Inv->inv_state, 0);
		if (Def && Def->ValStr2)
		    CpuBoard = Def->ValStr2;
		else {
		    /*
		     * We couldn't find a definetion so we'll use the
		     * numeric value in case that's in the config file.
		     */
		    if (Debug) Error("Unknown CPU Board Type (%d)",
				     Inv->inv_state);
		    CpuBoard = strdup(itoa(Inv->inv_state));
		}
	    }
	    break;
	case INV_GRAPHICS:
	    Def = DefGet("GraphicsTypes", NULL, Inv->inv_type, 0);
	    if (Def && Def->ValStr2)
		Graphics = Def->ValStr2;
	    break;
	case INV_VIDEO:
	    Def = DefGet("VideoTypes", NULL, Inv->inv_type, 0);
	    if (Def && Def->ValStr2)
		Video = Def->ValStr2;
	    break;
	}
    }

    endinvent();

    /*
     * Ya gotta have a CpuBoard right?
     */
    if (!CpuBoard) {
	if (Debug) Error("GetModelSGI(): No CPU Board found.");
	return((char *) NULL);
    }

    cp = GetCpuTypeSGI();
    if (cp) {
	CpuType = strdup(cp);
	/*
	 * Turn "mips R4000 2.0" into "R4000".
	 */
	if (cp = strchr(CpuType, ' '))
	    CpuType = cp + 1;
	if (cp = strchr(CpuType, ' '))
	    *cp = CNULL;
    }
    if (!CpuType) {
	if (Debug) Error("GetModelSGI(): Cannot determine CPUType.");
	return((char *) NULL);
    }

    /*
     * Format query (CpuType,CpuBoard,Graphics,Video) and see if we
     * can find a match.
     */
    (void) sprintf(QueryBuff, "%s,%s,%s,%s",
		   CpuType, CpuBoard,
		   (Graphics) ? Graphics : "-",
		   (Video) ? Video : "-");
    Def = DefGet("SysModels", QueryBuff, -1, DO_REGEX);
    if (Def && Def->ValStr1)
	return(Def->ValStr1);

    return ((char *) NULL);
}

/*
 * Get serial number using the syssgi() system call.
 */
extern char *GetSerialSGI()
{
    static char			Serial[MAXSYSIDSIZE];
    register char	       *cp;
    int				Okay;

    if (Serial[0])
	return(Serial);

    if (syssgi(SGI_SYSID, Serial) == 0) {
	/*
	 * Silly kludge to see if Serial contains valid information.
	 */
	Okay = TRUE;
	for (cp = Serial; cp && *cp; ++cp)
	    if (!isalnum(*cp)) {
		Okay = FALSE;
		break;
	    }
	if (Okay)
	    return(Serial);
    } else
	if (Debug) Error("syssgi(SGI_SYSID, ...) failed: %s", SYSERR);

    return(GetSerialSysinfo());
}

/*
 * SGI Get Kernel Version
 */
extern char *GetKernVerSGI()
{
    static char 		buf[BUFSIZ];
    char 			version[256];
    long 			style = -1;
  
#if	defined(_SC_KERN_POINTERS)
    style = sysconf(_SC_KERN_POINTERS);
#endif
    sysinfo(SI_VERSION, version, sizeof(version));
    /* IRIX Release 6.0 IP21 Version 08241804 System V - 64 Bit */
    sprintf(buf, "IRIX Release %s %s Version %s System V (%d bit)",
	    GetOSVer(), GetKernArch(),
	    version, (style == -1) ? 32 : style);

    return( (char *) buf);
}

/*
 * SGI Get OS Version number
 */
extern char *GetOSVerSGI()
{
    static char 		build[BUFSIZ];
    char 			patch[50];	/* To hold patch version */
    char 		       *pos = build;
    char 			minver[50];
    static int 			done = 0;
    static char 	       *retval;

    if (done)			/* Already did it */
	return((char *) retval);

    if (sysinfo(_MIPS_SI_OSREL_MAJ, pos, sizeof(build)) < 0) {
	strcpy(build,GetOSVerUname());
	done = 1;
	return ((retval = (char *) build));
    } else
	pos += strlen(build);

    if (sysinfo(_MIPS_SI_OSREL_MIN, minver, sizeof(minver)) >= 0) {
	if (minver[0] != '.')
	    *pos++ = '.';
	strcpy(pos,minver);
	/* Pre-load buffer with an initial '.', then stuff in version after */
	if (sysinfo(_MIPS_SI_OSREL_PATCH, patch+1, sizeof(patch)) >= 0) {
	    patch[0] = '.';	/* Pre-load string with a '.' delimiter */
	    if ( ! ((strlen(patch) == 2) && (patch[1] == '0')) ) {
		strcat(pos,patch);
	    }
	}
    }

    done = 1;			/* We're done! */

    if (pos == build)		/* If we didn't get anything, return NULL */
	return((retval = (char *) NULL));
    else
	return((retval = (char *) build));
}

/*
 * Determine CPU type using sysinfo().
 */
extern char *GetCpuTypeSGI()
{
    static char 		build[BUFSIZ];
    char 		       *pos = build;
    static int 			done = 0;
    static char 	       *retval;
    int				Len = 0;

    if (done)
	return((char *) retval);

    if (sysinfo(SI_ARCHITECTURE, pos, sizeof(build)) >= 0) {
	Len = strlen(build);
	pos += Len;
	*(pos++) = ' ';
    }
    if (sysinfo(_MIPS_SI_PROCESSORS, pos, sizeof(build) - Len + 2) >= 0)
	if ( ( pos = strchr(build,',')) != NULL)
	    *pos = CNULL;

    done = 1;
    if (pos == build)
	return((retval = (char *) NULL));
    else
	return((retval =(char *) build));
}

/*
 * Get Number of CPU's using sysinfo()
 */
extern char *GetNumCpuSGI()
{
    static char 		buf[BUFSIZ];
    static int 			done = 0;
    static char 	       *retval;

    if (done)
	return ((char *) retval);
    done = 1;

    if (sysinfo(_MIPS_SI_NUM_PROCESSORS, buf, sizeof(buf)) < 0)
	return((retval = (char *) NULL));
    else
	return((retval = buf));
}

/*
 * Get the long manufacterers name
 */
extern char *GetManLongSysinfoSGI()
{
    static char 		buf[BUFSIZ];

    if (sysinfo(_MIPS_SI_VENDOR, buf, sizeof(buf)) < 0)
	return((char *) NULL);
    else
	return((char *) buf);
}

/*
 * Get size of virtual memory.
 */
extern char *GetVirtMemSGI()
{
    kvm_t		       *kd;
    long			AvailMem;
    long			UsedMem;
    off_t			Amount = 0;
    nlist_t		       *nlPtr;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, "availsmem", (nlist_t *)NULL, 0)) == NULL)
	    return((char *) NULL);

	if (CheckNlist(nlPtr))
	    return((char *) NULL);

	if (KVMget(kd, nlPtr->n_value, (char *) &AvailMem,
		   sizeof(AvailMem), KDT_DATA) >= 0)
	    Amount = AvailMem;

	if ((nlPtr = KVMnlist(kd, "usermem", (nlist_t *)NULL, 0)) == NULL)
	    return((char *) NULL);

	if (CheckNlist(nlPtr))
	    return((char *) NULL);

	if (KVMget(kd, nlPtr->n_value, (char *) &UsedMem,
		   sizeof(UsedMem), KDT_DATA) >= 0)
	    Amount += UsedMem;

	Amount = Amount * (GETPAGESIZE() / KBYTES);
	KVMclose(kd);
    }

    return(GetVirtMemStr(Amount));
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
}

/*
 * Clean a string of unprintable characters and excess white-space.
 */
static char *CleanStr(String, StrSize)
    char		       *String;
    int				StrSize;
{
    register int		i, n;
    char		       *NewString;

    NewString = (char *) xcalloc(1, StrSize);

    for (i = 0, n = 0; i < StrSize; ++i) {
	if (i == 0)
	    /* Skip initial white space */
	    while (isspace(String[i]))
		++i;
	/* Skip extra white space */
	if (i > 0 && isspace(String[i-1]) && isspace(String[i]))
	    continue;
	if (isprint(String[i]))
	    NewString[n++] = String[i];
    }

    /* Remove trailing white space */
    while (isspace(NewString[n]))
	NewString[n--] = CNULL;

    return(NewString);
}

/*
 * Check to see if MntDevice is one of the "special" ones
 * that IRIX uses.
 */
static int IsSpecialUse(MntDevice, DevName)
    char		       *MntDevice;
    char		       *DevName;
{
    static char			DevFile[256];
    static struct stat		MntStat;
    static struct stat		DevStat;

    if (!EQ(MntDevice, "/dev/root") &&
	!EQ(MntDevice, "/dev/usr") &&
	!EQ(MntDevice, "/dev/swap"))
	return(0);

    (void) sprintf(DevFile, "%s/%s", _PATH_DEV_DSK, DevName);
    if (stat(MntDevice, &MntStat) != 0) {
	if (Debug) Error("stat failed: %s: %s", MntDevice, SYSERR);
	return(0);
    }
    if (stat(DevFile, &DevStat) != 0) {
	if (Debug) Error("stat failed: %s: %s", DevFile, SYSERR);
	return(0);
    }

    if (MntStat.st_rdev == DevStat.st_rdev)
	return(1);
    else
	return(0);
}

/*
 * Look for a partition named DevName in the mount table.
 */
static char *_GetMountInfo(FilePtr, DevName)
    FILE		       *FilePtr;
    char		       *DevName;
{
    struct mntent              *MntEnt;
    register char	       *cp;

    while (MntEnt = getmntent(FilePtr)) {
	if (cp = strrchr(MntEnt->mnt_fsname, '/'))
	    ++cp;
	else
	    cp = MntEnt->mnt_fsname;
	if (EQ(cp, DevName) || IsSpecialUse(MntEnt->mnt_fsname, DevName)) {
	    if (EQ(MntEnt->mnt_type, MNTTYPE_SWAP))
		return(MNTTYPE_SWAP);
	    else
		return(MntEnt->mnt_dir);
	}
    }

    return((char *) NULL);
}

/*
 * Lookup the usage for a given partition.
 */
static char *LookupPartUse(DevName, PartName)
    char		       *DevName;
    char		       *PartName;
{
    static FILE 	       *mntFilePtr = NULL;
    static FILE 	       *fstabFilePtr = NULL;
    static char			Name[256];
    char		       *Use = NULL;
    register char	       *cp;

    if (!DevName || !PartName)
	return((char *) NULL);

    (void) sprintf(Name, "%s%s", DevName, PartName);

    /*
     * First try the current mount table
     */
    if (!mntFilePtr) {
	if ((mntFilePtr = setmntent(MOUNTED, "r")) == NULL) {
	    Error("%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(mntFilePtr);

    if (Use = _GetMountInfo(mntFilePtr, Name))
	return(Use);

    /*
     * Now try the static mount table (/etc/fstab),
     * which may not reflect current reality.
     */
    if (!fstabFilePtr) {
	if ((fstabFilePtr = setmntent(MNTTAB, "r")) == NULL) {
	    Error("%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
	    return(NULL);
	}
    } else
	rewind(fstabFilePtr);

    if (Use = _GetMountInfo(fstabFilePtr, Name))
	return(Use);

    return((char *) NULL);
}

/*
 * Find out what a partition is being used for.
 */
static void GetPartUse(DiskName, Part, PartType, DiskPart)
    char		       *DiskName;
    char		       *Part;
    int			        PartType;
    DiskPart_t		       *DiskPart;
{
    char			Type[128];
    char			Usage[128];
    char		       *Use;

    Type[0] = Usage[0] = CNULL;

    switch (PartType) {
    case PTYPE_VOLHDR:	
	strcpy(Type, "volhdr");		strcpy(Usage, "Volume Header");	
	break;
    case PTYPE_TRKREPL:	
	strcpy(Type, "trkrepl");	strcpy(Usage, "Track Replacement");
	break;
    case PTYPE_SECREPL:	
	strcpy(Type, "secrepl");	strcpy(Usage, "Sector Replacement");
	break;
    case PTYPE_LVOL:	
	strcpy(Type, "lvol");		strcpy(Usage, "Logical Volume");
	break;
    case PTYPE_RLVOL: 
	strcpy(Type, "rlvol");		strcpy(Usage, "Raw Logical Volume");
	break;
    case PTYPE_VOLUME:	
	strcpy(Type, "volume");		strcpy(Usage, "Entire Volume");
	break;
#if	defined(PTYPE_XLV)
    case PTYPE_XLV:
	strcpy(Type, "xlv");		strcpy(Usage, "XLV Volume");
	break;
#endif
#if	defined(PTYPE_XFSLOG)
    case PTYPE_XFSLOG:
	strcpy(Type, "xfslog");		strcpy(Usage, "XFS Log");
	break;
#endif

	/*
	 * These types of partitions should be looked up
	 */
    case PTYPE_RAW:	strcpy(Type, "raw");			break;
    case PTYPE_EFS:	strcpy(Type, "efs");			break;
    case PTYPE_SYSV:	strcpy(Type, "sysv");			break;
    case PTYPE_BSD:	strcpy(Type, "bsd");			break;
#if	defined(PTYPE_XFS)
    case PTYPE_XFS:	strcpy(Type, "xfs");			break;
#endif

    default:		strcpy(Type, "UNKNOWN");		break;
    }

    if (Type[0])
	DiskPart->Type = strdup(Type);

    if (Usage[0])
	DiskPart->Usage = strdup(Usage); 
    else if (Use = LookupPartUse(DiskName, Part))
	DiskPart->Usage = strdup(Use);
}

/*
 * Get Disk Partition information for a given disk
 */
static DiskPart_t *GetDiskPart(DevInfo, VolHdr)
    DevInfo_t		       *DevInfo;
    struct volume_header       *VolHdr;
{
    DiskPart_t		       *DiskPart;
    DiskPart_t		       *Base = NULL;
    static char			PName[4];
    register DiskPart_t	       *Ptr;
    register int		i;

    for (i = 0; i < NPARTAB; ++i) {
	if (VolHdr->vh_pt[i].pt_nblks == 0)
	    continue;
	DiskPart = (DiskPart_t *) xcalloc(1, sizeof(DiskPart_t));
	(void) sprintf(PName, "s%d", i);
	DiskPart->Name = strdup(PName);
	DiskPart->StartSect = VolHdr->vh_pt[i].pt_firstlbn;
	DiskPart->NumSect = VolHdr->vh_pt[i].pt_nblks;
	GetPartUse(DevInfo->Name, PName,
		   VolHdr->vh_pt[i].pt_type, DiskPart);

	if (Base) {
	    for (Ptr = Base; Ptr && Ptr->Next; Ptr = Ptr->Next);
	    Ptr->Next = DiskPart;
	} else
	    Base = DiskPart;
    }

    return(Base);
}

/*
 * Probe a Disk Drive
 */
extern DevInfo_t *ProbeDiskDrive(Inv, TreePtr)
    inventory_t		       *Inv;
    DevInfo_t		       *TreePtr;
{
    static char			DevName[50];
    static char			DevFile[sizeof(DevName) + 
				       sizeof(_PATH_DEV_RDSK) + 4];
    static char			DriveType[SCSI_DEVICE_NAME_SIZE + 1];
    int				fd;
    struct volume_header	VolHdr;
    DiskDrive_t		       *DiskDrive = NULL;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *CtlrDev;
    Define_t		       *Def;
    char		       *Model = NULL;
    int				VolHdrOK = 1;

    if (!DevName)
	return((DevInfo_t *) NULL);

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Type = DT_DISKDRIVE;

    /*
     * Find the disk controller for this disk so that we
     * can figure out the correct disk device name.
     */
    CtlrDev = FindDeviceByType(DT_DISKCTLR, -1, Inv->inv_controller, TreePtr);
    if (!CtlrDev) {
	if (Debug) 
	    Error("No disk controller found for type %d ctlr %d unit %d",
		  Inv->inv_type, Inv->inv_controller, Inv->inv_unit);
	(void) sprintf(DevName, "c%dd%d", Inv->inv_controller, Inv->inv_unit);
	DevInfo->Name = strdup(DevName);
	return(DevInfo);
    }

    /*
     * INV_SCSIFLOPPY is for various types of floppy devices
     */
    if (Inv->inv_type == INV_SCSIFLOPPY) {
	(void) sprintf(DevName, "fds%dd%d", 
		       Inv->inv_controller, Inv->inv_unit);
	(void) sprintf(DevFile, "%s/%svh", _PATH_DEV_RDSK, DevName);
    } else {
	(void) sprintf(DevName, "%sd%d", CtlrDev->Name, Inv->inv_unit);
	(void) sprintf(DevFile, "%s/%svh", _PATH_DEV_RDSK, DevName);
    }

    fd = open(DevFile, O_RDONLY);
    if (fd < 0) {
	if (Debug) Error("open failed: %s: %s", DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    }

    /*
     * Get the Drive Type
     */
    if (ioctl(fd, DIOCDRIVETYPE, DriveType) < 0) {
	if (Debug) Error("ioctl DIOCDRIVETYPE failed: %s: %s", DevFile,SYSERR);
	DriveType[0] = CNULL;
    }
    if (DriveType[0])
	Model = CleanStr(DriveType, (int)sizeof(DriveType));

    /*
     * Get the volume header data.
     */
    if (ioctl(fd, DIOCGETVH, &VolHdr) < 0) {
	if (Debug) Error("ioctl DIOCGETVH failed: %s: %s", DevFile, SYSERR);
	VolHdrOK = 0;
    }

    /*
     * Put it all together
     */
    DevInfo->Master = CtlrDev;
    DevInfo->Name = DevName;
    if (Model)
	DevInfo->Model = Model; 
    else {
	/*
	 * See if there's a generic name in the .cf files
	 */
	Def = DefGet("DiskTypes", NULL, Inv->inv_type, 0);
	if (Def && Def->ValStr1)
	    Model = strdup(Def->ValStr1);
    }

    DiskDrive = (DiskDrive_t *) xcalloc(1, sizeof(DiskDrive_t));
    DevInfo->DevSpec = (caddr_t *) DiskDrive;
    DiskDrive->Unit = Inv->inv_unit;
    if (VolHdrOK) {
	DiskDrive->DataCyl = VolHdr.vh_dp.dp_cyls;
	DiskDrive->Heads = VolHdr.vh_dp.dp_trks0;
	DiskDrive->Sect = VolHdr.vh_dp.dp_secs;
	DiskDrive->SecSize = VolHdr.vh_dp.dp_secbytes;
	DiskDrive->IntrLv = VolHdr.vh_dp.dp_interleave;
	DiskDrive->APC = VolHdr.vh_dp.dp_spares_cyl;
	DiskDrive->DiskPart = GetDiskPart(DevInfo, &VolHdr);

	if (isprint(VolHdr.vh_bootfile[0]))
	    AddDevDesc(DevInfo, VolHdr.vh_bootfile, "Boot File", 0);
    }

    return(DevInfo);
}

/*
 * Get Disk info from Inventory
 */
static DevInfo_t *InvGetDisk(Inv, TreePtr)
    inventory_t		       *Inv;
    DevInfo_t		       *TreePtr;
{
    DevInfo_t		       *DevInfo;
    static char			Buff[128];
    char		       *DiskName = NULL;
    char		        CtlrModel[128];
    char			CtlrName[20];
    int				ClassType = -1;
    Define_t		       *Def;
    static DevDesc_t		DevDesc;

    CtlrModel[0] = CtlrName[0] = CNULL;
    memset(&DevDesc, CNULL, sizeof(DevDesc_t));

    /*
     * See if this is a Disk Controller
     */
    switch (Inv->inv_type) {
    case INV_SCSICONTROL:
	/*
	 * SCSI
	 */
	ClassType = CT_SCSI;
	(void) sprintf(CtlrName, "dks%d", Inv->inv_controller);

	Def = DefGet("scsi-ctlr", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) sprintf(CtlrModel,"Cannot lookup SCSI Cltr (Type=%d)",
			   Inv->inv_state);
	/*
	 * Check for additional bits of info
	 */
	if (Inv->inv_unit) {
	    (void) sprintf(Buff, "%X", Inv->inv_unit);
	    DevDesc.Desc = Buff;
	    DevDesc.Label = "Revision";
	}

	break;
    case INV_DKIPCONTROL:
	/*
	 * IPI
	 */
	ClassType = CT_IPI;
	(void) sprintf(CtlrName, "ipi%d", Inv->inv_controller);
	Def = DefGet("DiskTypes", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) sprintf(CtlrModel,"Cannot lookup INV_DKIPCONTROL (Type=%d)",
			   Inv->inv_state);
	break;
    case INV_XYL714:
    case INV_XYL754:
	/*
	 * SMD
	 */
	ClassType = CT_SMD;
	(void) sprintf(CtlrName, "xyl%d", Inv->inv_controller);
	Def = DefGet("DiskTypes", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) sprintf(CtlrModel, 
			   "Cannot lookup INV_XYL* (Type=%d)", Inv->inv_state);
	break;
    }

    if (CtlrName[0]) {
	/*
	 * It's a controller so finish creating device entry.
	 */
	DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));

	DevInfo->Name = strdup(CtlrName);
	if (CtlrModel[0])
	    DevInfo->Model = strdup(CtlrModel);
	DevInfo->Unit = Inv->inv_controller;
	DevInfo->Type = DT_DISKCTLR;
	DevInfo->ClassType = ClassType;
	if (DevDesc.Desc || DevDesc.Label)
	    AddDevDesc(DevInfo, DevDesc.Desc, DevDesc.Label, 0);
    } else {
	/*
	 * Must be a Disk Drive
	 */
	DevInfo = ProbeDiskDrive(Inv, TreePtr);
	if (DevInfo)
	    DevInfo->ClassType = ClassType;
    }

    return(DevInfo);
}

/*
 * Get I/O Board info from Inventory
 */
static DevInfo_t *InvGetIOBoard(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static char			Desc[128];
    static int			DevNum = 0;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_CARD;
    if (Inv->inv_unit > 0) {
	DevInfo->Unit = Inv->inv_unit;
	AddDevDesc(DevInfo, itoa(Inv->inv_unit), "Ebus Slot", 0);
    }

    Def = DefGet("IOBoardTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, 
			   (DevInfo->Unit) ? DevInfo->Unit : DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "IOBoard%d", DevNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown IOBoard Type (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    Def = DefGet("IOBoardStates", NULL, Inv->inv_state, 0);
    if (Def && Def->ValStr1)
	AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

    return(DevInfo);
}

/*
 * Get Bus info from Inventory
 */
static DevInfo_t *InvGetBus(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static char			Desc[128];

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_BUS;
    DevInfo->Unit = Inv->inv_controller;

    Def = DefGet("BusTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "bus%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown BusTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

/*
 * Get Compression Device info from Inventory
 */
static DevInfo_t *InvGetComp(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static char			Desc[128];

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = Inv->inv_unit;

    Def = DefGet("CompressionTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "compress%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown CompressionTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

/*
 * Get Misc Device info from Inventory
 */
static DevInfo_t *InvGetMisc(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static char			Desc[128];

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = Inv->inv_unit;

    Def = DefGet("MiscTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "misc%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown MiscTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

/*
 * Probe Frame Buffer
 */
extern DevInfo_t *ProbeFrameBuffer(DevInfo, Inv)
    DevInfo_t		       *DevInfo;
    inventory_t		       *Inv;
{
    int				fd;
    struct gfx_getboardinfo_args BoardInfo;
    struct gfx_info		GFXinfo;
    struct gfx_info	       *GFXinfoPtr = NULL;
    struct ng1_info		NG1info;
    struct gr1_info		GR1info;
    struct gr2_info		GR2info;
    struct lg1_info		LG1info;
    struct venice_info		VENICEinfo;
    FrameBuffer_t	       *Frame = NULL;
    static char			Desc[256];

    fd = open(_PATH_DEV_GRAPHICS, O_RDONLY);
    if (fd < 0) {
	if (Debug) Error("Open failed: %s: %s", _PATH_DEV_GRAPHICS, SYSERR);
	return((DevInfo_t *) NULL);
    }

    switch (Inv->inv_type) {
    case INV_NEWPORT:
	BoardInfo.buf = &NG1info;
	BoardInfo.len = sizeof(NG1info);
	GFXinfoPtr = &NG1info.gfx_info;
	break;
    case INV_GR1BOARD:
	BoardInfo.buf = &GR1info;
	BoardInfo.len = sizeof(GR1info);
	GFXinfoPtr = &GR1info.gfx_info;
	break;
    case INV_GR2:
	BoardInfo.buf = &GR2info;
	BoardInfo.len = sizeof(GR2info);
	GFXinfoPtr = &GR2info.gfx_info;
	break;
    case INV_LIGHT:
	BoardInfo.buf = &LG1info;
	BoardInfo.len = sizeof(LG1info);
	GFXinfoPtr = &LG1info.gfx_info;
	break;
#if	defined(INV_VENICE)	/* I don't know what type of GFX this is */
    case INV_VENICE:
	BoardInfo.buf = &VENICEinfo;
	BoardInfo.len = sizeof(VENICEinfo);
	GFXinfoPtr = &VENICEinfo.gfx_info;
	break;
#endif	/* INV_VENICE */
    default:
	BoardInfo.buf = &GFXinfo;
	BoardInfo.len = sizeof(GFXinfo);
	GFXinfoPtr = &GFXinfo;
    }

    /*
     * inv_unit hopefully will always be the board number we want.
     */
    BoardInfo.board = Inv->inv_unit;
    if (ioctl(fd, GFX_GETBOARDINFO, &BoardInfo) < 0) {
	if (Debug) Error("ioctl GFX_GETBOARDINFO failed: %s", SYSERR);
	return((DevInfo_t *) NULL);
    }

    Frame = (FrameBuffer_t *) xcalloc(1, sizeof(FrameBuffer_t));
    DevInfo->DevSpec = (caddr_t *) Frame;

    /*
     * Get Graphics Device specific info
     */
    switch (Inv->inv_type) {
    case INV_NEWPORT:
	Frame->Depth = NG1info.bitplanes;

	AddDevDesc(DevInfo, itoa(NG1info.boardrev), "Board Revision", 0);
	AddDevDesc(DevInfo, itoa(NG1info.ng1_vof_info.fields_sec), 
		   "Frequency (Hz)", 0);
	AddDevDesc(DevInfo, itoa(NG1info.monitortype), 
		   "Monitor Type", 0);
	if (NG1info.videoinstalled)
	    AddDevDesc(DevInfo, "Video is Installed", NULL, 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.rex3rev);
	AddDevDesc(DevInfo, Desc, "REX3 Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.vc2rev);
	AddDevDesc(DevInfo, Desc, "VC2 Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.mcrev);
	AddDevDesc(DevInfo, Desc, "MC Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.xmap9rev);
	AddDevDesc(DevInfo, Desc, "XMAP9 Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.cmaprev);
	AddDevDesc(DevInfo, Desc, "CMAP Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + NG1info.bt445rev);
	AddDevDesc(DevInfo, Desc, "BT445 Revision", 0);
	break;

    case INV_GR1BOARD:
	Frame->Depth = (int) GR1info.Bitplanes;

	switch (GR1info.BoardType) {
	case GR1_TYPE_GR1:	
	    (void) strcpy(Desc, "Personal Iris GR1");		break;
	case GR1_TYPE_VGR_PB:	
	    (void) strcpy(Desc, "9U Sized on Private Bus");	break;
	case GR1_TYPE_PGR:	
	    (void) strcpy(Desc, "6U Sized on Private Bus");	break;
	default:
	    (void) sprintf(Desc, "Unknown (Type %d)", GR1info.BoardType);
	}
	AddDevDesc(DevInfo, Desc, "Board Type", 0);

	(void) sprintf(Desc, "%d", GR1info.REversion);
	AddDevDesc(DevInfo, Desc, "REversion", 0);

	(void) sprintf(Desc, "%d", GR1info.Auxplanes);
	AddDevDesc(DevInfo, Desc, "Auxilary Planes", 0);

	(void) sprintf(Desc, "%d", GR1info.Widplanes);
	AddDevDesc(DevInfo, Desc, "Wid Planes", 0);

	(void) sprintf(Desc, "%d", GR1info.Zplanes);
	AddDevDesc(DevInfo, Desc, "Zplanes", 0);

	(void) sprintf(Desc, "%d", GR1info.Cursorplanes);
	AddDevDesc(DevInfo, Desc, "Cursor Planes", 0);

	if (GR1info.Turbo)
	    AddDevDesc(DevInfo, "Turbo Option", "Has", 0);

	if (GR1info.SmallMonitor)
	    AddDevDesc(DevInfo, "Small Monitor", "Has", 0);

	if (GR1info.VRAM1Meg)
	    AddDevDesc(DevInfo, "1 megabit VRAMs", "Has", 0);

	(void) sprintf(Desc, "%c", GR1info.picrev);
	AddDevDesc(DevInfo, Desc, "PIC Revision", 0);
	break;

    case INV_GR2:
	Frame->Depth = (int) GR2info.Bitplanes;

	switch (GR2info.BoardType) {
	case GR2_TYPE_GR2:	
	    (void) strcpy(Desc, "Express GR2");		break;
	case GR2_TYPE_HI1:	
	    (void) strcpy(Desc, "HI1");			break;
	default:
	    (void) sprintf(Desc, "Unknown (Type %d)", GR2info.BoardType);
	}
	AddDevDesc(DevInfo, Desc, "Board Type", 0);

	(void) sprintf(Desc, "%d", GR2info.Auxplanes);
	AddDevDesc(DevInfo, Desc, "Auxilary Planes", 0);

	(void) sprintf(Desc, "%d", GR2info.Wids);
	AddDevDesc(DevInfo, Desc, "Wids", 0);

	if (GR2info.Zbuffer)
	    AddDevDesc(DevInfo, "ZBuffer Option", "Has", 0);

	(void) sprintf(Desc, "%c", 'A'+GR2info.GfxBoardRev);
	AddDevDesc(DevInfo, Desc, "GFX Board Revision", 0);
 
	(void) sprintf(Desc, "%d", GR2info.MonitorType);
	AddDevDesc(DevInfo, Desc, "Monitor Type", 0);
 
	(void) sprintf(Desc, "%d", GR2info.MonTiming);
	AddDevDesc(DevInfo, Desc, "Monitor Timing", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.PICRev);
	AddDevDesc(DevInfo, Desc, "PIC Revision", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.HQ2Rev);
	AddDevDesc(DevInfo, Desc, "HQ2 Revision", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.GE7Rev);
	AddDevDesc(DevInfo, Desc, "GE7 Revision", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.RE3Rev);
	AddDevDesc(DevInfo, Desc, "RE3 Revision", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.VC1Rev);
	AddDevDesc(DevInfo, Desc, "VC1 Revision", 0);
 
	(void) sprintf(Desc, "%d", GR2info.GEs);
	AddDevDesc(DevInfo, Desc, "GEs", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.VidBckEndRev);
	AddDevDesc(DevInfo, Desc, "Video Back End Revision", 0);
 
	(void) sprintf(Desc, "%c", 'A' + GR2info.VidBrdRev);
	AddDevDesc(DevInfo, Desc, "Video Board Revision", 0);
 
	break;

    case INV_LIGHT:
	(void) sprintf(Desc, "%d", LG1info.boardrev);
	AddDevDesc(DevInfo, Desc, "Board Revision", 0);

	if (LG1info.boardrev >= 1) {
	    (void) sprintf(Desc, "%d", LG1info.monitortype);
	    AddDevDesc(DevInfo, Desc, "Monitor Type", 0);

	    if (LG1info.videoinstalled)
		AddDevDesc(DevInfo, "Video is Installed", NULL, 0);
	}

	(void) sprintf(Desc, "%c", 'A' + LG1info.rexrev);
	AddDevDesc(DevInfo, Desc, "REX Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + LG1info.vc1rev);
	AddDevDesc(DevInfo, Desc, "VC1 Revision", 0);

	(void) sprintf(Desc, "%c", 'A' + LG1info.picrev);
	AddDevDesc(DevInfo, Desc, "PIC Revision", 0);

	break;

#if	defined(INV_VENICE)
    case INV_VENICE:
	Frame->Depth = VENICEinfo.pixel_depth;

	(void) sprintf(Desc, "%d", VENICEinfo.ge_rev);
	AddDevDesc(DevInfo, Desc, "GE Revision", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.ge_count);
	AddDevDesc(DevInfo, Desc, "GE Count", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.rm_rev);
	AddDevDesc(DevInfo, Desc, "RM Revision", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.rm_count);
	AddDevDesc(DevInfo, Desc, "RM Count", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.ge_rev);
	AddDevDesc(DevInfo, Desc, "GT Revision", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.tiles_per_line);
	AddDevDesc(DevInfo, Desc, "Tiles/Line", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.ilOffset);
	AddDevDesc(DevInfo, Desc, "Initial Line Offset", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.pixel_density);
	AddDevDesc(DevInfo, Desc, "Pixel Density", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.hwalk_length);
	AddDevDesc(DevInfo, Desc, "No. of ops available in hblank", 0);

	(void) sprintf(Desc, "%d", VENICEinfo.tex_memory_size);
	AddDevDesc(DevInfo, Desc, "Texture Memory Size", 0);

	break;
#endif	/* INV_VENICE */
    }

    /*
     * Put together generic information
     */
    Frame->Width = GFXinfoPtr->xpmax;
    Frame->Height = GFXinfoPtr->ypmax;
    if (isprint(GFXinfoPtr->name[0]))
	DevInfo->Name = strdup(GFXinfoPtr->name);
    if (isprint(GFXinfoPtr->label[0]))
	AddDevDesc(DevInfo, GFXinfoPtr->label, "Managed By", 0);

    (void) close(fd);

    /* Return something */
    return(DevInfo);
}

/*
 * Get Graphics info from Inventory
 */
static DevInfo_t *InvGetGraphics(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    Define_t		       *DefList;
    char		       *ListName = NULL;
    char			Model[256];
    char			Desc[256];
    char			DevName[256];

    Model[0] = DevName[0] = CNULL;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_FRAMEBUFFER;
    DevInfo->Unit = Inv->inv_unit;

    Def = DefGet("GraphicsTypes", NULL, Inv->inv_type, 0);
    if (Def && Def->ValStr1)
	DevInfo->Model = Def->ValStr1;
    else {
	(void) sprintf(Model, "Unknown Graphics (Type=%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    (void) ProbeFrameBuffer(DevInfo, Inv);

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "%s%d", 
		       (Def && Def->ValStr2) ? Def->ValStr2 : "gfx", 
		       Inv->inv_unit);
	DevInfo->Name = strdup(DevName);
    }

    if (Inv->inv_state)
	switch (Inv->inv_type) {
	case INV_GR1BOARD:
	case INV_GR1BP:
	case INV_GR1ZBUFFER:
	    ListName = "GR1States";
	    break;
	case INV_GR2:
	    ListName = "GR2States";
	    break;
	case INV_NEWPORT:
	    ListName = "NEWPORTStates";
	    break;
	}

    if (ListName)
	for (Def = DefGetList(ListName); Def; Def = Def->Next)
	    if ((Inv->inv_state & Def->KeyNum) && Def->ValStr1)
		AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

    return(DevInfo);
}

/*
 * Get Network info from Inventory
 */
static DevInfo_t *InvGetNetwork(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			ModelBuff[256];
    char		       *Model = NULL;
    char			DevName[128];
    char			Desc[128];
    char		       *DevBase = NULL;

    /*
     * The inventory_t for networks looks like:
     *
     *	inv_type	- Class of device (ether, fddi, etc)
     *	inv_controller	- Model of device
     *	inv_state	- Model? specific
     */
    Def = DefGet("NetworkTypes", NULL, Inv->inv_controller, 0);
    if (Def) {
	if (Def->ValStr1)
	    Model = Def->ValStr1;
	if (Def->ValStr2) 
	    DevBase = Def->ValStr2;
    } 

    if (!Model) {
	(void) sprintf(ModelBuff, "Unknown Network Device (Type=%d)",
		       Inv->inv_type);
	Model = ModelBuff;
    }

    (void) sprintf(DevName, "%s%d", (DevBase) ? DevBase : "netif",
		   Inv->inv_unit);

    if (DevBase)
	/*
	 * Attempt to check system for device
	 */
	DevInfo = ProbeNetif(DevName, (DevData_t *) NULL, 
			     (DevDefine_t *) NULL);
    if (!DevInfo) {
	DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
	DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
	DevInfo->Type = DT_NETIF;
	DevInfo->Name = strdup(DevName);
    }

    if (Model)
	DevInfo->Model = strdup(Model);
	    
    /*
     * Set what we know
     */
    switch (Inv->inv_controller) {
    case INV_FDDI_IPG:
    case INV_ETHER_EC:
	(void) sprintf(Desc, "%d", Inv->inv_state);
	AddDevDesc(DevInfo, Desc, "Version", 0);
	break;
    case INV_ETHER_EE:
	(void) sprintf(Desc, "%d", Inv->inv_state);
	AddDevDesc(DevInfo, Desc, "Ebus Slot", 0);
	break;
    default:
	/*
	 * We don't know what inv_state means, but it may be useful
	 * so we'll just add it.
	 */
	(void) sprintf(Desc, "%d", Inv->inv_state);
	AddDevDesc(DevInfo, Desc, "State", 0);
    }

    /*
     * Certain models indicate the actual controller number in
     * inv_state.  We try to make sure inv_controller only is set
     * when we know it means something so that the calling function
     * doesn't use it by mistake.
     */
    switch (Inv->inv_controller) {
    case INV_ETHER_EE:
	Inv->inv_controller = Inv->inv_state;
	break;
    default:
	Inv->inv_controller = 0;
    }

    return(DevInfo);
}

/*
 * Get Tape info from Inventory
 */
static DevInfo_t *InvGetTape(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];

    Model[0] = DevName[0] = CNULL;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_TAPEDRIVE;

    switch (Inv->inv_type) {
    case INV_SCSIQIC:
    case INV_VSCSITAPE:
	/*
	 * Lookup as a SCSI Tape
	 */
	Def = DefGet("SCSITapeTypes", NULL, Inv->inv_state, 0);
	break;
    default:
	/*
	 * Lookup as a general tape type
	 */
	Def = DefGet("TapeTypes", NULL, Inv->inv_type, 0);
	break;
    }

    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2)
	    (void) sprintf(DevName, "%s%dd%d", Def->ValStr2, 
			   Inv->inv_controller, Inv->inv_unit);
    }
    if (!DevName[0]) 
	(void) sprintf(DevName, "tape%d", Inv->inv_unit);
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown Tape Drive Type %d",Inv->inv_state);
	DevInfo->Model = strdup(Model);
    }

    DevInfo->Name = strdup(DevName);

    return(DevInfo);
}

/*
 * Get Parallel info from Inventory
 */
static DevInfo_t *InvGetParallel(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static int			ParallelNum = 0;

    Model[0] = DevName[0] = CNULL;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_PARALLEL;

    Def = DefGet("ParallelTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, ParallelNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "parallel%d", ParallelNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown Parallel Type (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

/*
 * Get Serial info from Inventory
 */
static DevInfo_t *InvGetSerial(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static int			DevNum = 0;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_SERIAL;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("SerialTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "serial%d", DevNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown Serial Type (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_state > 0)
	AddDevDesc(DevInfo, strdup(itoa(Inv->inv_state)), 
		   "Number of Ports", 0);

    return(DevInfo);
}

/*
 * Get SCSI info from Inventory
 */
static DevInfo_t *InvGetSCSI(Inv, TreePtr)
    inventory_t		       *Inv;
    DevInfo_t		       *TreePtr;
{
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *CtlrDev = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static int			DevNum = 0;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = Inv->inv_unit;

    CtlrDev = FindDeviceByType(DT_DISKCTLR, CT_SCSI, Inv->inv_controller, 
			       TreePtr);
    if (CtlrDev)
	DevInfo->Master = CtlrDev;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("SCSITypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown SCSITypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) sprintf(DevName, "scsi%d", DevNum++);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_state > 0) {
	AddDevDesc(DevInfo, strdup(itoa(Inv->inv_state)), 
		   "State", 0);
	if (Inv->inv_state & INV_REMOVE)
	    AddDevDesc(DevInfo, "Is Removable", NULL, 0);
    }

    return(DevInfo);
}

/*
 * Get Audio info from Inventory
 */
static DevInfo_t *InvGetAudio(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[128];
    static int			DevNum = 0;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_AUDIO;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("AudioTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown AudioTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) sprintf(DevName, "audio%d", DevNum++);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_state > 0) {
	AddDevDesc(DevInfo, strdup(itoa(Inv->inv_state)), 
		   "State", 0);
    }

    return(DevInfo);
}

/*
 * Get Video info from Inventory
 */
static DevInfo_t *InvGetVideo(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char		       *StateList = NULL;
    char			Model[256];
    char			DevName[128];

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = Inv->inv_unit;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("VideoTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, Inv->inv_unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown VideoTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) sprintf(DevName, "video%d", Inv->inv_unit);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_state > 0) {
	AddDevDesc(DevInfo, strdup(itoa(Inv->inv_state)), 
		   "State", 0);
	switch (Inv->inv_type) {
	case INV_VIDEO_EXPRESS:
	    StateList = "ExpressVideoStates";
	    break;
	case INV_VIDEO_VINO:
	    StateList = "VinoVideoStates";
	    break;
	}
	if (StateList)
	    for (Def = DefGetList(StateList); Def; Def = Def->Next)
		if ((Inv->inv_state & Def->KeyNum) && Def->ValStr1)
		    AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);
    }

    return(DevInfo);
}

/*
 * Get Display info from Inventory
 */
static DevInfo_t *InvGetDisplay(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char		       *StateList = NULL;
    char			Model[256];
    char			DevName[128];

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->Unit = Inv->inv_unit;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("DisplayTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) sprintf(DevName, "%s%d", Def->ValStr2, Inv->inv_unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown DisplayTypes (%d)", Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) sprintf(DevName, "display%d", Inv->inv_unit);
    }

    return(DevInfo);
}

/*
 * Get Memory info from Inventory
 */
static DevInfo_t *InvGetMemory(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    char			Model[256];
    char			DevName[20];
    static char			Desc[128];
    char		       *SizeStr;
    static int			MemNum = 0;

    /*
     * INV_MAIN_MB is really just the amount of
     * main memory in MBytes.  It's not needed because
     * we count on INV_MAIN.
     */
    if (Inv->inv_type == INV_MAIN_MB)
	return((DevInfo_t *) NULL);

    Desc[0] = Model[0] = CNULL;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_MEMORY;

    SizeStr = GetSizeStr((u_long)Inv->inv_state, (u_long)BYTES);
    Def = DefGet("MemoryTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1) {
	    (void) sprintf(Model, "%s %s", SizeStr, Def->ValStr1);
	    DevInfo->Model = strdup(Model);
	}
	if (Def->ValStr2)
	    DevInfo->Name = Def->ValStr2;
    }

    if (!DevInfo->Name) {
	(void) sprintf(DevName, "mem%d", MemNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) sprintf(Model, "Unknown Memory Type (%d) %s",
		       Inv->inv_type, SizeStr);
	DevInfo->Model = strdup(Model);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_unit > 0) {
	(void) sprintf(Desc, "%d-way", Inv->inv_unit);
	AddDevDesc(DevInfo, Desc, "Interleave", 0);
    }

    return(DevInfo);
}

/*
 * Get CPU info from Inventory
 */
static DevInfo_t *InvGetCPU(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    char			Model[256];
    char			DevName[128];
    char			Desc[256];
    char			Label[128];
    static int			CPUboardNum = 0;
    static int			CPUNum = 0;
    static int			ProcessorNum = 0;
    Define_t		       *Def;

    Model[0] = DevName[0] = Desc[0] = Label[0] = CNULL;

    switch (Inv->inv_type) {
    case INV_CPUBOARD:
	/*
	 * It doesn't appear there is a real unit number assigned
	 * to a system board, so we'll make one up for now.
	 */
	(void) sprintf(DevName, "cpuboard%d", CPUboardNum++);

	Def = DefGet("cpuboard", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(Model, Def->ValStr1);
	else
	    (void) sprintf(Model, "Type %d CPU Board", Inv->inv_state);

	/*
	 * What vague CPU info there is is associated with the cpuboard
	 * for some silly reason.
	 */
	(void) sprintf(Desc, "%s %d MHz %s CPUs",
		       GetNumCpu(), Inv->inv_controller, 
		       strupper(GetCpuType()));
	break;
    case INV_CPUCHIP:
	/* This doesn't tell us anything */
	break;
    case INV_FPUCHIP:
	(void) sprintf(DevName, "fpu%d", Inv->inv_unit);
	(void) sprintf(Model, "Floating Point Unit");
	(void) sprintf(Label, "Revision");
	(void) sprintf(Desc, "%d", Inv->inv_state);
	break;
    case INV_CCSYNC:
	/* What the hell is this? */
	(void) strcpy(DevName, "ccsync");
	(void) strcpy(Model, "CC Revision 2+ sync join counter");
	break;
    default:
	(void) sprintf(DevName, "processor%d", ProcessorNum++);
	(void) sprintf(Model, "Unknown Processor Type (%d)", Inv->inv_type);
    }

    if (DevName[0]) {
	DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
	DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
	DevInfo->Name = strdup(DevName);
	if (Model[0])
	    DevInfo->Model = strdup(Model);
	if (Desc[0])
	    AddDevDesc(DevInfo, Desc, (Label[0]) ? Label : NULL, 0);
    }

    return(DevInfo);
}

/*
 * Use the SGI getinvent(3) interface.
 */
static int BuildDevicesInvent(TreePtrPtr, Names)
    DevInfo_t 		       **TreePtrPtr;
    char		       **Names;
{
    static DevData_t	        DevData;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *CtlrDev = NULL;
    inventory_t		       *Inv;
    char			Buff[256];
    static int			DevNum = 0;

    setinvent();

    while (Inv = getinvent()) {
	DevInfo = NULL;
	if (Debug) 
	    printf(
	   "INV: Class = %d Type = %d ctrl = %d unit = %d state = %d\n",
		   Inv->inv_class, Inv->inv_type, Inv->inv_controller,
		   Inv->inv_unit, Inv->inv_state);
	switch (Inv->inv_class) {
	case INV_PROCESSOR:
	    DevInfo = InvGetCPU(Inv);
	    break;
	case INV_IOBD:
	    DevInfo = InvGetIOBoard(Inv);
	    break;
	case INV_BUS:
	    DevInfo = InvGetBus(Inv);
	    break;
	case INV_DISK:
	    DevInfo = InvGetDisk(Inv, *TreePtrPtr);
	    break;
	case INV_MEMORY:
	    DevInfo = InvGetMemory(Inv);
	    break;
	case INV_SERIAL:
	    DevInfo = InvGetSerial(Inv);
	    break;
	case INV_PARALLEL:
	    DevInfo = InvGetParallel(Inv);
	    break;
	case INV_TAPE:
	    DevInfo = InvGetTape(Inv);
	    break;
	case INV_GRAPHICS:
	    DevInfo = InvGetGraphics(Inv);
	    break;
	case INV_NETWORK:
	    DevInfo = InvGetNetwork(Inv);
	    break;
	case INV_SCSI:
	    DevInfo = InvGetSCSI(Inv, *TreePtrPtr);
	    break;
	case INV_AUDIO:
	    DevInfo = InvGetAudio(Inv);
	    break;
	case INV_VIDEO:
	    DevInfo = InvGetVideo(Inv);
	    break;
	case INV_DISPLAY:
	    DevInfo = InvGetDisplay(Inv);
	    break;
	case INV_COMPRESSION:
	    DevInfo = InvGetComp(Inv);
	    break;
	case INV_MISC:
	    DevInfo = InvGetMisc(Inv);
	    break;
	default:
	    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
	    (void) sprintf(Buff, "unknown%d", DevNum++);
	    DevInfo->Name = strdup(Buff);
	    DevInfo->Unit = Inv->inv_unit;
	    (void) sprintf(Buff, 
	   "Unknown Device Type (class=%d type=%d ctlr=%d unit=%d state=%d)",
			   Inv->inv_class, Inv->inv_type, Inv->inv_controller,
			   Inv->inv_unit, Inv->inv_state);
	    DevInfo->Model = strdup(Buff);
	}
	/*
	 * See if we can find a generic controller card for this device
	 */
	if (DevInfo && !DevInfo->Master && Inv->inv_controller > 0) {
	    CtlrDev = FindDeviceByType(DT_CARD, -1, Inv->inv_controller, 
				       *TreePtrPtr);
	    if (CtlrDev)
		DevInfo->Master = CtlrDev;
	}
	if (DevInfo)
	    AddDevice(DevInfo, TreePtrPtr, Names);
    }

    endinvent();

    return(0);
}

/*
 * Build device tree using TreePtr.
 * Calls bus and method specific functions to
 * search for devices.
 */
extern int BuildDevices(TreePtr, Names)
    DevInfo_t 		       **TreePtr;
    char		       **Names;
{
    int 			 Found = 1;

    if (BuildDevicesInvent(TreePtr, Names) == 0)
	Found = 0;

    return(Found);
}
