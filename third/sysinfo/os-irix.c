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
#include "myscsi.h"
#include <invent.h>
#include <mntent.h>
#include <sys/systeminfo.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <sys/syssgi.h>
#include <sys/dsreq.h>

/*
 * Disk
 */
#include <sys/dkio.h>
#include <sys/dvh.h>

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
PSI_t GetOSDistPSI[] = {		/* Get OS Dist */
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
    { GetVirtMemSwapctl },
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
    static char			Query[256];

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
		    SImsg(SIM_UNKN, "Unknown CPU Board Type (%d)",
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
	SImsg(SIM_DBG, "GetModelSGI(): No CPU Board found.");
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
	SImsg(SIM_GERR, "GetModelSGI(): Cannot determine CPUType.");
	return((char *) NULL);
    }

    /*
     * Format query (CpuType,CpuBoard,Graphics,Video) and see if we
     * can find a match.
     */
    (void) snprintf(Query, sizeof(Query),  "%s,%s,%s,%s",
		    CpuType, CpuBoard,
		    (Graphics) ? Graphics : "-",
		    (Video) ? Video : "-");
    SImsg(SIM_DBG, "GetModelSGI: Look for <%s>", Query);
    Def = DefGet("SysModels", Query, -1, DO_REGEX);
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
	SImsg(SIM_GERR, "syssgi(SGI_SYSID, ...) failed: %s", SYSERR);

    return(GetSerialSysinfo());
}

/*
 * SGI Get Kernel Version
 */
extern char *GetKernVerSGI()
{
    static char 		Buff[128];
    char 			Version[256];
    long 			Style = -1;
  
#if	defined(_SC_KERN_POINTERS)
    Style = sysconf(_SC_KERN_POINTERS);
#endif
    sysinfo(SI_VERSION, Version, sizeof(Version));
    /* IRIX Release 6.0 IP21 Version 08241804 System V - 64 Bit */
    snprintf(Buff, sizeof(Buff),
	     "IRIX Release %s %s Version %s System V (%d bit)",
	     GetOSVer(), GetKernArch(),
	     Version, (Style == -1) ? 32 : Style);

    return( (char *) Buff);
}

/*
 * SGI Get OS Version number
 */
extern char *GetOSVerSGI()
{
    static char 		Build[256];
    char 			Patch[50];	/* To hold patch version */
    char 		       *pos = Build;
    char 			MinVer[50];
    int				Len;
    static int 			done = 0;
    static char 	       *RetVal;

    if (done)			/* Already did it */
	return((char *) RetVal);

    if (sysinfo(_MIPS_SI_OSREL_MAJ, Build, sizeof(Build)) < 0) {
	(void) snprintf(Build, sizeof(Build), "%s", GetOSVerUname());
	done = 1;
	return ((RetVal = (char *) Build));
    } else
	pos += strlen(Build);

    if (sysinfo(_MIPS_SI_OSREL_MIN, MinVer, sizeof(MinVer)) >= 0) {
	if (MinVer[0] != '.')
	    *pos++ = '.';
	Len = pos - &Build[0];
	(void) snprintf(pos, sizeof(Build)-Len, "%s", MinVer);

	/* Pre-load buffer with an initial '.', then stuff in version after */
	if (sysinfo(_MIPS_SI_OSREL_PATCH, Patch+1, sizeof(Patch)) >= 0) {
	    Patch[0] = '.';	/* Pre-load string with a '.' delimiter */
	    if ( ! ((strlen(Patch) == 2) && (Patch[1] == '0')) ) {
		Len = pos - Build;
		(void) snprintf(pos, sizeof(Build)-Len, "%s", Patch);
	    }
	}
    }

    done = 1;			/* We're done! */

    if (pos == Build)		/* If we didn't get anything, return NULL */
	return((RetVal = (char *) NULL));
    else
	return((RetVal = (char *) Build));
}

/*
 * Determine CPU type using sysinfo().
 */
extern char *GetCpuTypeSGI()
{
    static char 		Build[128];
    char 		       *pos = Build;
    static int 			done = 0;
    static char 	       *RetVal;
    int				Len = 0;

    if (done)
	return((char *) RetVal);

    if (sysinfo(SI_ARCHITECTURE, Build, sizeof(Build)) >= 0) {
	Len = strlen(Build);
	pos += Len;
	*(pos++) = ' ';
    }
    if (sysinfo(_MIPS_SI_PROCESSORS, pos, sizeof(Build) - Len + 2) >= 0)
	if ( ( pos = strchr(Build,',')) != NULL)
	    *pos = CNULL;

    done = 1;
    if (pos == Build)
	return((RetVal = (char *) NULL));
    else
	return((RetVal =(char *) Build));
}

/*
 * Get Number of CPU's using sysinfo()
 */
extern char *GetNumCpuSGI()
{
    static char 		Buff[32];
    static int 			done = 0;
    static char 	       *RetVal;

    if (done)
	return ((char *) RetVal);
    done = 1;

    if (sysinfo(_MIPS_SI_NUM_PROCESSORS, Buff, sizeof(Buff)) < 0)
	return((RetVal = (char *) NULL));
    else
	return((RetVal = Buff));
}

/*
 * Get the long manufacterers name
 */
extern char *GetManLongSysinfoSGI()
{
    static char 		Buff[128];

    if (sysinfo(_MIPS_SI_VENDOR, Buff, sizeof(Buff)) < 0)
	return((char *) NULL);
    else
	return((char *) Buff);
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
 * Issue a SCSI command and return the results.
 * Uses the ds(7M) interface.
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
#if	defined(DS_ENTER)
    static char			Buff[SCSI_BUF_LEN];
    static dsreq_t		Cmd;
    ScsiCdbG0_t		       *CdbPtr = NULL;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Cmd, 0, sizeof(Cmd));

    Cmd.ds_cmdbuf = (caddr_t) ScsiCmd->Cdb;
    Cmd.ds_cmdlen = ScsiCmd->CdbLen;
    Cmd.ds_databuf = (caddr_t) Buff;
    Cmd.ds_datalen = sizeof(Buff);
    Cmd.ds_time = MySCSI_CMD_TIMEOUT * 1000;	/* ds_time==milliseconds */
    Cmd.ds_flags = DSRQ_READ;

    /* 
     * We just need Cdb.cmd so it's ok to assume ScsiCdbG0_t here
     */
    CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;

    /*
     * Send cmd to device
     */
    if (ioctl(ScsiCmd->DevFD,  DS_ENTER, &Cmd) == -1) {
	SImsg(SIM_GERR, "%s: ioctl DS_ENTER for SCSI 0x%x failed: %s", 
	      ScsiCmd->DevFile, CdbPtr->cmd, SYSERR);
	return(-1);
    }

    if (Cmd.ds_status != 0) {
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed with ds_status=%d", 
	      ScsiCmd->DevFile, CdbPtr->cmd, Cmd.ds_status);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
#endif	/* DS_ENTER */
    return(0);
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

    NewString = (char *) xcalloc(1, StrSize + 1);

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

    (void) snprintf(DevFile, sizeof(DevFile), "%s/%s", _PATH_DEV_DSK, DevName);
    if (stat(MntDevice, &MntStat) != 0) {
	SImsg(SIM_GERR, "stat failed: %s: %s", MntDevice, SYSERR);
	return(0);
    }
    if (stat(DevFile, &DevStat) != 0) {
	SImsg(SIM_GERR, "stat failed: %s: %s", DevFile, SYSERR);
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

    (void) snprintf(Name, sizeof(Name), "%s%s", DevName, PartName);

    /*
     * First try the current mount table
     */
    if (!mntFilePtr) {
	if ((mntFilePtr = setmntent(MOUNTED, "r")) == NULL) {
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
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
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s.", MNTTAB, SYSERR);
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
#if	defined(PTYPE_VOLHDR)
    case PTYPE_VOLHDR:	
	strcpy(Type, "volhdr");		strcpy(Usage, "Volume Header");	
	break;
#endif
#if	defined(PTYPE_TRKREPL)
    case PTYPE_TRKREPL:	
	strcpy(Type, "trkrepl");	strcpy(Usage, "Track Replacement");
	break;
#endif
#if	defined(PTYPE_SECREPL)
    case PTYPE_SECREPL:	
	strcpy(Type, "secrepl");	strcpy(Usage, "Sector Replacement");
	break;
#endif
#if	defined(PTYPE_LVOL)
    case PTYPE_LVOL:	
	strcpy(Type, "lvol");		strcpy(Usage, "Logical Volume");
	break;
#endif
#if	defined(PTYPE_RLVOL)
    case PTYPE_RLVOL: 
	strcpy(Type, "rlvol");		strcpy(Usage, "Raw Logical Volume");
	break;
#endif
#if	defined(PTYPE_VOLUME)
    case PTYPE_VOLUME:	
	strcpy(Type, "volume");		strcpy(Usage, "Entire Volume");
	break;
#endif
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
#if	defined(PTYPE_RAW)
    case PTYPE_RAW:	strcpy(Type, "raw");			break;
#endif
#if	defined(PTYPE_EFS)
    case PTYPE_EFS:	strcpy(Type, "efs");			break;
#endif
#if	defined(PTYPE_SYSV)
    case PTYPE_SYSV:	strcpy(Type, "sysv");			break;
#endif
#if	defined(PTYPE_BSD)
    case PTYPE_BSD:	strcpy(Type, "bsd");			break;
#endif
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
	(void) snprintf(PName, sizeof(PName),  "s%d", i);
	DiskPart->Name = strdup(PName);
	DiskPart->StartSect = (Large_t) VolHdr->vh_pt[i].pt_firstlbn;
	DiskPart->NumSect = (Large_t) VolHdr->vh_pt[i].pt_nblks;
	/* Store pt_type for possible later use in finding drive capacity */
	DiskPart->NumType = VolHdr->vh_pt[i].pt_type;
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
 * Get total disk size (capacity) in bytes.
 * Look for the PTYPE_VOLUME (entire disk) partiton and use its 
 * size as return value.
 */
float IRIXGetDiskSize(DiskDrive)
    DiskDrive_t		       *DiskDrive;
{
    DiskPart_t		       *dp;

#if	defined(PTYPE_VOLUME)
    for (dp = DiskDrive->DiskPart; dp; dp = dp->Next) {
	if (dp->NumType == PTYPE_VOLUME) {
	    SImsg(SIM_DBG, "DISK PTYPE_VOLUME numsects=%.0f size=%.0f",
		  (float) dp->NumSect, (float) DiskDrive->SecSize);
	    return((float) ((float)dp->NumSect * (float)DiskDrive->SecSize));
	}
    }
#else
    SImsg(SIM_DBG, "PTYPE_VOLUME is not defined on this OS");
#endif	/* PTYPE_VOLUME */

    return((float) 0);
}

/*
 * Find the Disk Device.
 * Stores device name in DevNamePtr and File name in DevFilePtr.
 * Returns 0 on success.
 */
static int FindDiskDev(CtlrDev, Inv, DevName, DevNameSize, 
		       DevFile, DevFileSize)
     DevInfo_t		       *CtlrDev;
     inventory_t	       *Inv;
     char		       *DevName;
     size_t			DevNameSize;
     char		       *DevFile;
     size_t			DevFileSize;
{
    register int		i;

    /*
     * INV_SCSIFLOPPY is for various types of floppy devices
     */
    if (Inv->inv_type == INV_SCSIFLOPPY) {
	(void) snprintf(DevName, DevNameSize, "fds%dd%d", 
			Inv->inv_controller, Inv->inv_unit);
	(void) snprintf(DevFile, DevFileSize, "%s/%svh", 
			_PATH_DEV_RDSK, DevName);
	return(0);
    }

    /*
     * Normal disk drive name?
     */
    (void) snprintf(DevName, DevNameSize, "%sd%d", 
		    CtlrDev->Name, Inv->inv_unit);
    (void) snprintf(DevFile, DevFileSize, "%s/%svh", 
		    _PATH_DEV_RDSK, DevName);
    if (FileExists(DevFile))
	return(0);

    /*
     * Check for Logical Unit Number (LUN) devices.
     * i.e. dksXdXlX
     */
    for (i = 0; i < SI_MAX_LUN; ++i) {
	(void) snprintf(DevName, DevNameSize, "%sd%dl%d", 
			CtlrDev->Name, Inv->inv_unit, i);
	(void) snprintf(DevFile, DevFileSize, "%s/%svh", 
			_PATH_DEV_RDSK, DevName);
	if (FileExists(DevFile))
	    return(0);
    }

    return(1);
}

/*
 * Query disk using SCSI methods
 */
static int ProbeDiskDriveScsi(DevInfo, DevFile)
     DevInfo_t		       *DevInfo;
     char		       *DevFile;
{
    int				fd;

    fd = open(DevFile, O_RDONLY|O_NDELAY|O_NONBLOCK);
    if (fd < 0) {
	SImsg(SIM_GERR, "open failed: %s: %s", DevFile, SYSERR);
	return(-1);
    }

    (void) ScsiQuery(DevInfo, DevFile, fd, TRUE);

    (void) close(fd);

    return(0);
}

/*
 * Query disk using normal OS methods
 */
static int ProbeDiskDriveQuery(DevInfo, DevFile)
     DevInfo_t		       *DevInfo;
     char		       *DevFile;
{
    DiskDriveData_t	       *DiskDriveData = NULL;
    DiskDrive_t		       *OSdisk = NULL;
    DiskDrive_t		       *HWdisk = NULL;
    char		       *Vendor = NULL;
    char		       *Model = NULL;
    u_int			DiskCap = 0;
    static struct volume_header	VolHdr;
    static char			DriveType[SCSI_DEVICE_NAME_SIZE + 1];
    int				fd;

    fd = open(DevFile, O_RDONLY);
    if (fd < 0) {
	SImsg(SIM_GERR, "open failed: %s: %s", DevFile, SYSERR);
	return(-1);
    }

    if (!DevInfo->Model || !DevInfo->Vendor) {
	/*
	 * Get the Drive Type
	 */
	if (ioctl(fd, DIOCDRIVETYPE, DriveType) < 0)
	    SImsg(SIM_GERR, "ioctl DIOCDRIVETYPE failed: %s: %s", 
		  DevFile,SYSERR);
	else {
	    if (Vendor = strchr(DriveType, ' ')) {
		*Vendor = CNULL;
		++Vendor;
	    } else
		Vendor = DriveType;
	    if (!DevInfo->Vendor)
		DevInfo->Vendor = strdup(Vendor);
	    if (!DevInfo->Model)
		DevInfo->Model = strdup(DriveType);
	}
    }

    DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    OSdisk = DiskDriveData->OSdata;
    HWdisk = DiskDriveData->HWdata;

    /*
     * Get the volume header data.
     */
    if (ioctl(fd, DIOCGETVH, &VolHdr) < 0) {
	SImsg(SIM_GERR, "ioctl DIOCGETVH failed: %s: %s", DevFile, SYSERR);
    } else {
	OSdisk->DiskPart = GetDiskPart(DevInfo, &VolHdr);
	OSdisk->SecSize = VolHdr.vh_dp.dp_secbytes;
#if	OSVER <= 63
	/*
	 * This stuff was removed starting with IRIX 6.4
	 */
	OSdisk->DataCyl = VolHdr.vh_dp.dp_cyls;
	OSdisk->Tracks = VolHdr.vh_dp.dp_trks0;
	OSdisk->Sect = VolHdr.vh_dp.dp_secs;
	OSdisk->IntrLv = VolHdr.vh_dp.dp_interleave;
	OSdisk->APC = VolHdr.vh_dp.dp_spares_cyl;
#endif	/* OSVER <= 63 */
	if (isprint(VolHdr.vh_bootfile[0]))
	    AddDevDesc(DevInfo, VolHdr.vh_bootfile, "Boot File", 0);
    }

#if	defined(DIOCREADCAPACITY)
    /*
     * Get the Disk Capacity
     */
    if (ioctl(fd, DIOCREADCAPACITY, &DiskCap) < 0)
	SImsg(SIM_GERR, "ioctl DIOCREADCAPACITY failed: %s: %s", 
	      DevFile, SYSERR);
    else {
	SImsg(SIM_DBG, "DISK: %s DIOCREADCAPACITY = %d", DevFile, DiskCap);
	/*
	 * If we still don't have the disk capacity, try getting it
	 * from the partition table.
	 */
	DiskCap = (u_int) IRIXGetDiskSize(OSdisk);
	if (!HWdisk->Size && DiskCap && OSdisk->SecSize) {
	    HWdisk->Size = (float) nsect_to_mbytes(DiskCap, OSdisk->SecSize);
	    SImsg(SIM_DBG, "DISK: %s: size=%.0f MB", 
		  DevInfo->Name, OSdisk->Size);
	}
    }
#endif	/* DIOCREADCAPACITY */

    (void) close(fd);

    return(0);
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
    static DevFind_t		Find;
    DiskDriveData_t	       *DiskDriveData = NULL;
    DiskDrive_t		       *OSdisk = NULL;
    DiskDrive_t		       *HWdisk = NULL;
    DevInfo_t		       *DevInfo = NULL;
    DevInfo_t		       *CtlrDev;
    DevType_t		       *DevType = NULL;
    ClassType_t		       *ClassType = NULL;
    Define_t		       *Def;

    if (!Inv)
	return((DevInfo_t *) NULL);

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Type = DT_DISKDRIVE;

    /*
     * Find the disk controller for this disk so that we
     * can figure out the correct disk device name.
     */
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = TreePtr;
    Find.DevType = DT_CONTROLLER;
    Find.ClassType = -1;
    Find.Unit = Inv->inv_controller;
    CtlrDev = DevFind(&Find);
    if (!CtlrDev) {
	SImsg(SIM_DBG, 
	      "No disk controller found for type %d ctlr %d unit %d",
	      Inv->inv_type, Inv->inv_controller, Inv->inv_unit);
	(void) snprintf(DevName, sizeof(DevName), "c%dd%d", 
			Inv->inv_controller, Inv->inv_unit);
	DevInfo->Name = strdup(DevName);
	return(DevInfo);
    }

    /*
     * Setup DiskDrive data
     */
    if (DevInfo->DevSpec) 
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }
    if (DiskDriveData->OSdata)
	OSdisk = DiskDriveData->OSdata;
    else
	DiskDriveData->OSdata = OSdisk = NewDiskDrive(NULL);
    if (DiskDriveData->HWdata)
	HWdisk = DiskDriveData->HWdata;
    else
	DiskDriveData->HWdata = HWdisk = NewDiskDrive(NULL);

    /*
     * Put it all together
     */
    DevInfo->Master = CtlrDev;
    DevInfo->Name = DevName;
    OSdisk->Unit = Inv->inv_unit;

    /*
     * Perform SCSI query of drive
     */
    (void) snprintf(DevFile, sizeof(DevFile), "%s/sc%dd%dl0",
		    _PATH_DEV_SCSI, Inv->inv_controller, Inv->inv_unit);
    ProbeDiskDriveScsi(DevInfo, DevFile);

    /*
     * If we know this to be a Floppy, set it here, AFTER the SCSI
     * query has been done to override that.
     */
    if (Inv->inv_type == INV_SCSIFLOPPY)
	DevInfo->Type = DT_FLOPPY;

    /*
     * Find name of disk device (DevName) and device file (DevFile)
     */
    if (FindDiskDev(CtlrDev, Inv, DevName, sizeof(DevName), 
		    DevFile, sizeof(DevFile)) != 0) {
	SImsg(SIM_GERR, 
	      "Cannot find disk device file for type %d ctlr %d unit %d",
	      Inv->inv_type, Inv->inv_controller, Inv->inv_unit);
	return((DevInfo_t *) NULL);
    }
    DevAddFile(DevInfo, strdup(DevFile));
    /*
     * Do OS Query's
     */
    ProbeDiskDriveQuery(DevInfo, DevFile);

    /*
     * See if there's a generic name in the .cf files
     */
    Def = DefGet("DiskTypes", NULL, Inv->inv_type, 0);
    if (Def->ValStr2)
	if (DevType = TypeGetByName(Def->ValStr2))
	    DevInfo->Type = DevType->Type;
    if (Def->ValStr3)
	if (ClassType = ClassTypeGetByName(DevInfo->Type, Def->ValStr3))
	    DevInfo->ClassType = ClassType->Type;

    if (!DevInfo->Model && Def && Def->ValStr1)
	DevInfo->Model = Def->ValStr1;

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
#if	defined(INV_SCSICONTROL)
    case INV_SCSICONTROL:
	/*
	 * SCSI
	 */
	ClassType = CT_SCSI;
	(void) snprintf(CtlrName, sizeof(CtlrName), "dks%d",
			Inv->inv_controller);

	Def = DefGet("scsi-ctlr", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) snprintf(CtlrModel, sizeof(CtlrModel), 
			    "Cannot lookup SCSI Cltr (Type=%d)",
			    Inv->inv_state);
	/*
	 * Check for additional bits of info
	 */
	if (Inv->inv_unit) {
	    (void) snprintf(Buff, sizeof(Buff),  "%X", Inv->inv_unit);
	    DevDesc.Desc = Buff;
	    DevDesc.Label = "Revision";
	}

	break;
#endif	/* INV_SCSICONTROL */
#if	defined(INV_DKIPCONTROL)
    case INV_DKIPCONTROL:
	/*
	 * IPI
	 */
	ClassType = CT_IPI;
	(void) snprintf(CtlrName, sizeof(CtlrName), "ipi%d", 
			Inv->inv_controller);
	Def = DefGet("DiskTypes", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) snprintf(CtlrModel, sizeof(CtlrModel), 
			    "Cannot lookup INV_DKIPCONTROL (Type=%d)",
			    Inv->inv_state);
	break;
#endif	/* INV_DKIPCONTROL */
#if	defined(INV_XYL714)
    case INV_XYL714:
#endif	/* INV_XYL714 */
#if	defined(INV_XYL754)
    case INV_XYL754:
	/*
	 * SMD
	 */
	ClassType = CT_SMD;
	(void) snprintf(CtlrName, sizeof(CtlrName), "xyl%d", 
			Inv->inv_controller);
	Def = DefGet("DiskTypes", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(CtlrModel, Def->ValStr1);
	else
	    (void) snprintf(CtlrModel, sizeof(CtlrModel),  
			   "Cannot lookup INV_XYL* (Type=%d)", Inv->inv_state);
	break;
#endif	/* INV_XYL754 */
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
	DevInfo->Type = DT_CONTROLLER;
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

#if	defined(INV_O200IO)
    if (Inv->inv_type == INV_O200IO)
	/* See if we can find the specific type of O2000 board */
	Def = DefGet("O2000ioTypes", NULL, Inv->inv_state, 0);
    else
#endif	/* INV_O200IO */
	/* Just get the generic I/O Board type */
	Def = DefGet("IOBoardTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) snprintf(DevName, sizeof(DevName),  "%s%d", Def->ValStr2, 
			   (DevInfo->Unit) ? DevInfo->Unit : DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "IOBoard%d", DevNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown IOBoard Type (%d)", 
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    Def = NULL;
    if (Inv->inv_type == INV_EVIO)
	Def = DefGet("EVIOBoardStates", NULL, Inv->inv_state, 0);
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
    ClassType_t		       *Class;
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
	if (Def->ValStr3)
	    if (Class = ClassTypeGetByName(DT_BUS, Def->ValStr3))
		DevInfo->ClassType = Class->Type;
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "bus%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model && !DevInfo->ClassType) {
	(void) snprintf(Model, sizeof(Model), "Unknown BusTypes (%d)",
			Inv->inv_type);
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d",
			    Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "compress%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown CompressionTypes (%d)",
			Inv->inv_type);
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "misc%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown MiscTypes (%d)",
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

/*
 * Get PROM Device info from Inventory
 */
static DevInfo_t *InvGetPROM(Inv)
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

    Def = DefGet("PROMTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, DevInfo->Unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "prom%d", DevInfo->Unit);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown PROMTypes (%d)", 
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    return(DevInfo);
}

#if	defined(HAVE_GRAPHICS_HDRS)
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
	SImsg(SIM_GERR, "Open failed: %s: %s", _PATH_DEV_GRAPHICS, SYSERR);
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
	SImsg(SIM_GERR, "ioctl GFX_GETBOARDINFO failed: %s", SYSERR);
	return((DevInfo_t *) NULL);
    }

    Frame = (FrameBuffer_t *) xcalloc(1, sizeof(FrameBuffer_t));
    DevInfo->DevSpec = (void *) Frame;

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

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.rex3rev);
	AddDevDesc(DevInfo, Desc, "REX3 Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.vc2rev);
	AddDevDesc(DevInfo, Desc, "VC2 Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.mcrev);
	AddDevDesc(DevInfo, Desc, "MC Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.xmap9rev);
	AddDevDesc(DevInfo, Desc, "XMAP9 Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.cmaprev);
	AddDevDesc(DevInfo, Desc, "CMAP Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + NG1info.bt445rev);
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
	    (void) snprintf(Desc, sizeof(Desc), "Unknown (Type %d)", 
			    GR1info.BoardType);
	}
	AddDevDesc(DevInfo, Desc, "Board Type", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR1info.REversion);
	AddDevDesc(DevInfo, Desc, "REversion", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR1info.Auxplanes);
	AddDevDesc(DevInfo, Desc, "Auxilary Planes", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR1info.Widplanes);
	AddDevDesc(DevInfo, Desc, "Wid Planes", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR1info.Zplanes);
	AddDevDesc(DevInfo, Desc, "Zplanes", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR1info.Cursorplanes);
	AddDevDesc(DevInfo, Desc, "Cursor Planes", 0);

	if (GR1info.Turbo)
	    AddDevDesc(DevInfo, "Turbo Option", "Has", 0);

	if (GR1info.SmallMonitor)
	    AddDevDesc(DevInfo, "Small Monitor", "Has", 0);

	if (GR1info.VRAM1Meg)
	    AddDevDesc(DevInfo, "1 megabit VRAMs", "Has", 0);

	(void) snprintf(Desc, sizeof(Desc),  "%c", GR1info.picrev);
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
	    (void) snprintf(Desc, sizeof(Desc), "Unknown (Type %d)", 
			    GR2info.BoardType);
	}
	AddDevDesc(DevInfo, Desc, "Board Type", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.Auxplanes);
	AddDevDesc(DevInfo, Desc, "Auxilary Planes", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.Wids);
	AddDevDesc(DevInfo, Desc, "Wids", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.GEs);
	AddDevDesc(DevInfo, Desc, "GEs", 0);
 
	if (GR2info.Zbuffer)
	    AddDevDesc(DevInfo, "Z-Buffer", "Has", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.VidBckEndRev);
	AddDevDesc(DevInfo, Desc, "Video Back End Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.VidBrdRev);
	AddDevDesc(DevInfo, Desc, "Video Board Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.MonitorType);
	AddDevDesc(DevInfo, Desc, "Monitor Type", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.MonTiming);
	AddDevDesc(DevInfo, Desc, "Monitor Timing", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%d", GR2info.GfxBoardRev);
	AddDevDesc(DevInfo, Desc, "GFX Board Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.PICRev - 1);
	AddDevDesc(DevInfo, Desc, "PIC Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.HQ2Rev - 1);
	AddDevDesc(DevInfo, Desc, "HQ2 Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.GE7Rev - 1);
	AddDevDesc(DevInfo, Desc, "GE7 Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", GR2info.RE3Rev);
	AddDevDesc(DevInfo, Desc, "RE3 Revision", 0);
 
	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + GR2info.VC1Rev - 1);
	AddDevDesc(DevInfo, Desc, "VC1 Revision", 0);
 
	break;

    case INV_LIGHT:
	(void) snprintf(Desc, sizeof(Desc), "%d", LG1info.boardrev);
	AddDevDesc(DevInfo, Desc, "Board Revision", 0);

	if (LG1info.boardrev >= 1) {
	    (void) snprintf(Desc, sizeof(Desc), "%d", LG1info.monitortype);
	    AddDevDesc(DevInfo, Desc, "Monitor Type", 0);

	    if (LG1info.videoinstalled)
		AddDevDesc(DevInfo, "Video is Installed", NULL, 0);
	}

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + LG1info.rexrev);
	AddDevDesc(DevInfo, Desc, "REX Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + LG1info.vc1rev);
	AddDevDesc(DevInfo, Desc, "VC1 Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%c", 'A' + LG1info.picrev);
	AddDevDesc(DevInfo, Desc, "PIC Revision", 0);

	break;

#if	defined(INV_VENICE)
    case INV_VENICE:
	Frame->Depth = VENICEinfo.pixel_depth;

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.ge_rev);
	AddDevDesc(DevInfo, Desc, "GE Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.ge_count);
	AddDevDesc(DevInfo, Desc, "GE Count", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.rm_rev);
	AddDevDesc(DevInfo, Desc, "RM Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.rm_count);
	AddDevDesc(DevInfo, Desc, "RM Count", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.ge_rev);
	AddDevDesc(DevInfo, Desc, "GT Revision", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.tiles_per_line);
	AddDevDesc(DevInfo, Desc, "Tiles/Line", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.ilOffset);
	AddDevDesc(DevInfo, Desc, "Initial Line Offset", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.pixel_density);
	AddDevDesc(DevInfo, Desc, "Pixel Density", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.hwalk_length);
	AddDevDesc(DevInfo, Desc, "No. of ops available in hblank", 0);

	(void) snprintf(Desc, sizeof(Desc), "%d", VENICEinfo.tex_memory_size);
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
#endif	/* HAVE_GRAPHICS_HDRS */

/*
 * Get Graphics info from Inventory
 */
static DevInfo_t *InvGetGraphics(Inv)
    inventory_t		       *Inv;
{
#if	defined(HAVE_GRAPHICS_HDRS)
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    Define_t		       *DefList;
    char		       *ListName = NULL;
    char		       *GrType = NULL;
    char			Model[256];
    char			Desc[256];
    char			DevName[256];
    static char			GrTmp[128];

    Model[0] = DevName[0] = CNULL;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_FRAMEBUFFER;
    DevInfo->Unit = Inv->inv_unit;

    Def = DefGet("GraphicsTypes", NULL, Inv->inv_type, 0);
    if (Def && Def->ValStr1)
	DevInfo->Model = Def->ValStr1;
    else {
	(void) snprintf(Model, sizeof(Model), "Unknown Graphics (Type=%d)",
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }

    (void) ProbeFrameBuffer(DevInfo, Inv);

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "%s%d", 
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
	    GrType = "GR2";
	    break;
	case INV_NEWPORT:
	    ListName = "NEWPORTStates";
	    break;
	case INV_MGRAS:
#if	defined(INV_MGRAS_ARCHS)
	    Def = DefGet("MGRASarchs", NULL, 
			 Inv->inv_state & INV_MGRAS_ARCHS, 0);
	    if (Def)
		AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);
	    else
		if (Debug) printf("Unknown MGRASarchs: 0x%x\n", 
				  Inv->inv_state & INV_MGRAS_ARCHS);
#endif	/* INV_MGRAS_ARCHS */

	    Def = DefGet("MGRASges", NULL, 
			 Inv->inv_state & INV_MGRAS_GES, 0);
	    if (Def) AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

	    Def = DefGet("MGRASres", NULL, 
			 Inv->inv_state & INV_MGRAS_RES, 0);
	    if (Def) AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

	    Def = DefGet("MGRAStrs", NULL, 
			 Inv->inv_state & INV_MGRAS_TRS, 0);
	    if (Def) AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

	    break;
	}

    if (ListName)
	for (Def = DefGetList(ListName); Def; Def = Def->Next)
	    if ((Inv->inv_state & Def->KeyNum) && Def->ValStr1)
		AddDevDesc(DevInfo, Def->ValStr1, NULL, 0);

    /*
     * See if there's a more specific submodel defined.
     */
    if (GrType) {
	SImsg(SIM_DBG, 
	      "GRDEBUG: GraphicsType=`%s' class=%d type=%d state=0x%x",
	      GrType, Inv->inv_class, Inv->inv_type, Inv->inv_state);
	(void) snprintf(GrTmp, sizeof(GrTmp), "%sconfigs", GrType);
	Def = DefGetList(GrTmp, NULL, Inv->inv_state, 0);
	if (Def)
	    DevInfo->Model = Def->ValStr1;
    }

    return(DevInfo);
#else	/* !defined(HAVE_GRAPHICS_HDRS) */
    SImsg(SIM_GERR, 
"Graphics/FrameBuffer found, but SysInfo compiled without HAVE_GRAPHICS_HDRS."
	  );
    return((DevInfo_t *) NULL);
#endif	/* HAVE_GRAPHICS_HDRS */
}

/*
 * Get Network info from Inventory
 */
static DevInfo_t *InvGetNetwork(Inv)
    inventory_t		       *Inv;
{
    DevInfo_t		       *DevInfo = NULL;
    Define_t		       *Def;
    ClassType_t		       *Class = NULL;
    char			ModelBuff[256];
    char		       *Model = NULL;
    char			DevName[128];
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
	if (Def->ValStr3)
	    Class = ClassTypeGetByName(DT_NETIF, Def->ValStr3);
    } 

    if (!Model) {
	(void) snprintf(ModelBuff, sizeof(ModelBuff),
			"Unknown Network Device (Type=%d)", Inv->inv_type);
	Model = ModelBuff;
    }

    (void) snprintf(DevName, sizeof(DevName), "%s%d",
		    (DevBase) ? DevBase : "netif", Inv->inv_unit);

    if (DevBase) {
	/*
	 * Attempt to check system for device
	 */
	ProbeData_t		ProbeData;

	memset(&ProbeData, CNULL, sizeof(ProbeData));
	ProbeData.DevName = DevName;
	DevInfo = ProbeNetif(&ProbeData);
    }
    if (!DevInfo) {
	DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
	DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
	DevInfo->Type = DT_NETIF;
	DevInfo->Name = strdup(DevName);
    }

    if (Model)
	DevInfo->Model = strdup(Model);
    if (Class)
	DevInfo->ClassType = Class->Type;
	    
    /*
     * Set what we know
     */
    switch (Inv->inv_controller) {
    case INV_FDDI_IPG:
    case INV_ETHER_EC:
	AddDevDesc(DevInfo, itoa(Inv->inv_state), "Version", 0);
	break;
    case INV_ETHER_EE:
	AddDevDesc(DevInfo, itoa(Inv->inv_state), "Ebus Slot", 0);
	break;
    default:
	/*
	 * We don't know what inv_state means, but it may be useful
	 * so we'll just add it.
	 */
	AddDevDesc(DevInfo, itoa(Inv->inv_state), "State", 0);
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
    static char			DevFile[MAXPATHLEN];
    char			Model[256];
    char			DevName[128];
    int				ClassType = 0;
    ClassType_t		       *Class = NULL;

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
	ClassType = CT_SCSI;
	/*
	 * Perform SCSI queries
	 */
	(void) snprintf(DevFile, sizeof(DevFile), "%s/sc%dd%dl0",
			_PATH_DEV_SCSI, Inv->inv_controller, Inv->inv_unit);
	(void) ScsiQuery(DevInfo, DevFile, -1, TRUE);
	break;
    default:
	/*
	 * Lookup as a general tape type
	 */
	Def = DefGet("TapeTypes", NULL, Inv->inv_type, 0);
	if (Def && Def->ValStr3)
	    if (Class = ClassTypeGetByName(DT_TAPEDRIVE, Def->ValStr3))
		ClassType = Class->Type;
	break;
    }

    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2)
	    (void) snprintf(DevName, sizeof(DevName), "%s%dd%d", Def->ValStr2, 
			    Inv->inv_controller, Inv->inv_unit);
    }
    if (!DevName[0]) 
	(void) snprintf(DevName, sizeof(DevName), "tape%d", Inv->inv_unit);
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown Tape Drive Type %d",
			Inv->inv_state);
	DevInfo->Model = strdup(Model);
    }

    DevInfo->Name = strdup(DevName);
    DevInfo->ClassType = ClassType;

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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d",
			    Def->ValStr2, ParallelNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "parallel%d", ParallelNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown Parallel Type (%d)",
			Inv->inv_type);
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

#if	defined(INV_INVISIBLE)
    if (Inv->inv_type == INV_INVISIBLE)
	return((DevInfo_t *) NULL);
#endif	/* INV_INVISIBLE */

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_SERIAL;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("SerialTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1)
	    DevInfo->Model = Def->ValStr1;
	if (Def->ValStr2) {
	    (void) snprintf(DevName, sizeof(DevName), "%s%d",
			    Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "serial%d", DevNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown Serial Type (%d)",
			Inv->inv_type);
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
    DevType_t		       *DevType = NULL;
    Define_t		       *Def;
    static DevFind_t		Find;
    char			Model[256];
    char			DevName[128];
    static int			DevNum = 0;

    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
    DevInfo->Unit = DevInfo->Addr = DevInfo->Prio = DevInfo->Vec = -1;
    DevInfo->Type = DT_GENERIC;
    DevInfo->ClassType = CT_SCSI;
    DevInfo->Unit = Inv->inv_unit;

    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = TreePtr;
    Find.DevType = DT_CONTROLLER;
    Find.ClassType = CT_SCSI;
    Find.Unit = Inv->inv_controller;
    if (CtlrDev = DevFind(&Find))
	DevInfo->Master = CtlrDev;

    Model[0] = DevName[0] = CNULL;

    Def = DefGet("SCSITypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr2) {
	    if (DevType = TypeGetByName(Def->ValStr2))
		DevInfo->Type = DevType->Type;
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
	if (Def->ValStr1)
	    /*
	     * If we were unable to lookup ValStr2 (the DevType) then
	     * go ahead and use ValStr1 as the Model.
	     */
	    if (!DevInfo->Type)
		DevInfo->Model = Def->ValStr1;
    }

    if (!DevInfo->Model && !DevInfo->Type) {
	(void) snprintf(Model, sizeof(Model), "Unknown SCSITypes (%d)", 
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) snprintf(DevName, sizeof(DevName), "scsi%d", DevNum++);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_state > 0) {
	AddDevDesc(DevInfo, strdup(itoa(Inv->inv_state)), "State", DA_APPEND);
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, DevNum++);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown AudioTypes (%d)",
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) snprintf(DevName, sizeof(DevName), "audio%d", DevNum++);
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d",
			    Def->ValStr2, Inv->inv_unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown VideoTypes (%d)",
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) snprintf(DevName, sizeof(DevName), "video%d", Inv->inv_unit);
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
	    (void) snprintf(DevName, sizeof(DevName), "%s%d", 
			    Def->ValStr2, Inv->inv_unit);
	    DevInfo->Name = strdup(DevName);
	}
    }

    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown DisplayTypes (%d)",
			Inv->inv_type);
	DevInfo->Model = strdup(Model);
    }
    if (!DevInfo->Name) {
	DevInfo->Name = strdup(DevName);
	(void) snprintf(DevName, sizeof(DevName), "display%d", Inv->inv_unit);
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

    SizeStr = GetSizeStr((Large_t)Inv->inv_state, BYTES);
    Def = DefGet("MemoryTypes", NULL, Inv->inv_type, 0);
    if (Def) {
	if (Def->ValStr1) {
	    (void) snprintf(Model, sizeof(Model), "%s %s", 
			    SizeStr, Def->ValStr1);
	    DevInfo->Model = strdup(Model);
	}
	if (Def->ValStr2)
	    DevInfo->Name = Def->ValStr2;
    }

    if (!DevInfo->Name) {
	(void) snprintf(DevName, sizeof(DevName), "mem%d", MemNum++);
	DevInfo->Name = strdup(DevName);
    }
    if (!DevInfo->Model) {
	(void) snprintf(Model, sizeof(Model), "Unknown Memory Type (%d) %s",
		       Inv->inv_type, SizeStr);
	DevInfo->Model = strdup(Model);
    }

    /*
     * Get what else we can find.
     */
    if (Inv->inv_unit > 0) {
	(void) snprintf(Desc, sizeof(Desc), "%d-way", Inv->inv_unit);
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
	(void) snprintf(DevName, sizeof(DevName), "cpuboard%d", CPUboardNum++);

	Def = DefGet("cpuboard", NULL, Inv->inv_state, 0);
	if (Def && Def->ValStr1)
	    (void) strcpy(Model, Def->ValStr1);
	else
	    (void) snprintf(Model, sizeof(Model), "Type %d CPU Board", 
			    Inv->inv_state);

	/*
	 * What vague CPU info there is is associated with the cpuboard
	 * for some silly reason.
	 */
	(void) snprintf(Desc, sizeof(Desc), "%s %d MHz %s CPUs",
		       GetNumCpu(), Inv->inv_controller, 
		       strupper(GetCpuType()));
	break;
    case INV_CPUCHIP:
	/* This doesn't tell us anything */
	break;
    case INV_FPUCHIP:
	(void) snprintf(DevName, sizeof(DevName), "fpu%d", Inv->inv_unit);
	(void) snprintf(Model, sizeof(Model), "Floating Point Unit");
	(void) snprintf(Label, sizeof(Label), "Revision");
	(void) snprintf(Desc, sizeof(Desc), "%d", Inv->inv_state);
	break;
    case INV_CCSYNC:
	/* What the hell is this? */
	(void) strcpy(DevName, "ccsync");
	(void) strcpy(Model, "CC Revision 2+ sync join counter");
	break;
    default:
	(void) snprintf(DevName, sizeof(DevName), "processor%d", 
			ProcessorNum++);
	(void) snprintf(Model, sizeof(Model), "Unknown Processor Type (%d)", 
			Inv->inv_type);
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
    static DevFind_t		Find;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *CtlrDev = NULL;
    inventory_t		       *Inv;
    char			Buff[256];
    static int			DevNum = 0;

    setinvent();

    while (Inv = getinvent()) {
	DevInfo = NULL;
	SImsg(SIM_DBG, 
	      "INV: Class = %d Type = %d ctrl = %d unit = %d state = %d",
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
#if	OSVER >= 64
	case INV_PROM:
	    DevInfo = InvGetPROM(Inv);
	    break;
#endif	/* OSVER >= 64 */
	default:
	    DevInfo = (DevInfo_t *) xcalloc(1, sizeof(DevInfo_t));
	    (void) snprintf(Buff, sizeof(Buff), "unknown%d", DevNum++);
	    DevInfo->Name = strdup(Buff);
	    DevInfo->Unit = Inv->inv_unit;
	    (void) snprintf(Buff, sizeof(Buff), 
	   "Unknown Device Type (class=%d type=%d ctlr=%d unit=%d state=%d)",
			    Inv->inv_class, Inv->inv_type, Inv->inv_controller,
			    Inv->inv_unit, Inv->inv_state);
	    DevInfo->Model = strdup(Buff);
	}
	/*
	 * See if we can find a generic controller card for this device
	 */
	if (DevInfo && !DevInfo->Master && Inv->inv_controller > 0) {
	    (void) memset(&Find, 0, sizeof(Find));
	    Find.Tree = *TreePtrPtr;
	    Find.DevType = DT_CARD;
	    Find.ClassType = -1;
	    Find.Unit = Inv->inv_controller;
	    if (CtlrDev = DevFind(&Find))
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
