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
 * HP-UX specific functions
 */

#include "defs.h"
#if OSMVER == 9
#if	defined(hp9000s800)
#	include <sys/libIO.h>
#else
#	include <sys/libio.h>
#endif 
#endif
#if OSMVER >= 10
#include <sys/diskio.h>
#include <sys/hilioctl.h>
#include <sys/audio.h>
#include <sys/framebuf.h>
#endif	/* OSMVER >= 10 */
#if	defined(HAVE_SCSI_CTL)
#include "myscsi.h"
#include <sys/scsi.h>
#endif	/* HAVE_SCSI_CTL */
#include <sys/pstat.h>
#include <sys/conf.h>

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelHPUX();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelFile },
    { GetModelHPUX },
    { NULL },
};
extern char			       *GetSerialUname();
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
    { GetSerialUname },
    { NULL },
};
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
    { NULL },
};
extern char			       *GetKernArchHPUX1();
extern char			       *GetKernArchHPUX2();
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchHPUX2 },
    { GetKernArchHPUX1 },
    { GetKernArchTest },
    { NULL },
};
extern char			       *GetAppArchHPUX();
PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetAppArchHPUX },
    { GetAppArchTest },
    { NULL },
};
extern char			       *GetCpuTypeHPUX();
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeHPUX },
    { NULL },
};
extern char			       *GetNumCpuPSTAT();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNumCpuPSTAT },
    { NULL },
};
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerSym },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameUname },
    { NULL },
};
extern char			       *GetOSDistHPUX();
PSI_t GetOSDistPSI[] = {		/* Get OS Distribution */
    { GetOSDistHPUX },
    { NULL },
};
extern char			       *GetOSVerHPUX();
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerHPUX },
    { GetOSVerUname },
    { NULL },
};
PSI_t GetManShortPSI[] = {		/* Get Short Man Name */
    { GetManShortSysinfo },
    { GetManShortDef },
    { NULL },
};
PSI_t GetManLongPSI[] = {		/* Get Long Man Name */
    { GetManLongSysinfo },
    { GetManLongDef },
    { NULL },
};
extern char			       *GetMemoryPSTAT();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryPSTAT },
    { GetMemoryPhysmemSym },
    { NULL },
};
extern char			       *GetVirtMemHPUX1();
extern char			       *GetVirtMemHPUX2();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemHPUX2 },
    { GetVirtMemHPUX1 },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeUtmp },
    { NULL },
};

/*
 * Data from ioscan(1m)
 */
typedef struct _IOSdata {
    char		       *DevType;	/* PROCESSOR,MEMORY,... */
    char		       *DevClass;	/* disk,graphics,lan,... */
    char		       *DevData;	/* N N N N N ... */
    char		       *DevCat;		/* pa,core,scsi */
    char		       *BusType;	/* pa,wsio,core,eisa,... */
    char		       *Driver;		/* Name of Device driver */
    char		       *NexusName;	/* eisa,bus_adapter,... */
    char		       *HWpath;		/* HW Path N/N/N... */
    char		       *SWpath;		/* SW Path root.bus... */
    char		       *SWstate;	/* CLAIMED,UNCLAIMED,... */
    char		       *Desc;		/* Description of device */
    int				Unit;		/* Device Unit */
    int				BusUnit;	/* My bus's Unit number */
    char		      **DevFiles;	/* List of device files */
    struct _IOSdata	       *Next;
} IOSdata_t;

/*
 * HPUX Get OS Version number
 */
extern char *GetOSVerHPUX()
{
    register char	       *Ptr = NULL;

    Ptr = GetOSVerUname();
    if (!Ptr)
	return((char *) NULL);

    /*
     * Usually un.release (from uname()) looks something like "A.09.03".
     * We want to clean that up to be "9.03".
     */
    for ( ; Ptr && (isalpha(*Ptr) || *Ptr == '.' || *Ptr == '0'); 
	 ++Ptr);

    return(Ptr);
}

/*
 * Get Serial Number using uname()
 */
extern char *GetSerialUname()
{
    static struct utsname	un;

    if (uname(&un) != 0)
	return((char *) NULL);

    return( (un.idnumber[0]) ? un.idnumber : (char *)NULL );
}

/*
 * Get OS Distribution using uname()
 */
extern char *GetOSDistHPUX()
{
    static struct utsname	un;

    if (uname(&un) != 0)
	return((char *) NULL);

    return( (un.release[0]) ? un.release : (char *)NULL );
}

/*
 * Get the speed (in MHz) of the CPU.  Assumes only 1 CPU.
 */
static char 			CpuSpeedSYM[] = "itick_per_tick";
#ifdef notdef	/* This works to */
static char 			CpuSpeedSYM[] = "iticks_per_10_msec";
#endif
extern int GetCPUSpeed()
{
    struct nlist	       *nlptr;
    off_t			Addr;
    kvm_t		       *kd;
    static int			Speed;
    int				SpeedVar;

    if (Speed)
	return(Speed);

    /*
     * Get the CPU speed
     */
    if (!(kd = KVMopen()))
	return(0);
    if ((nlptr = KVMnlist(kd, CpuSpeedSYM, (struct nlist *)NULL, 0)) == NULL) {
	KVMclose(kd);
	return(0);
    }
    if (CheckNlist(nlptr)) {
	KVMclose(kd);
	return(0);
    }
    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &SpeedVar, 
	       sizeof(SpeedVar), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read `%s' from kernel.", CpuSpeedSYM);
	SpeedVar = -1;
    }
    KVMclose(kd);

    if (SpeedVar > 0)
	Speed = SpeedVar / 10000;

    return(Speed);
}

/*
 * HPUX Get Model name
 */
extern char *GetModelHPUX()
{
    static struct utsname	un;
    char		       *Machine = NULL;
    static char			Model[100];
    int				Speed;

    if (Model[0])
	return(Model);

    if (uname(&un) != 0)
	return((char *) NULL);

    if (un.machine && un.machine[0])
	Machine = un.machine;

    Speed = GetCPUSpeed();

    if (Speed > 0)
	(void) snprintf(Model, sizeof(Model),  "%s/%d", Machine, Speed);
    else
	(void) strcpy(Model, Machine);

    return(Model);
}

/*
 * Determine system kernel architecture using uname()
 */
extern char *GetKernArchHPUX1()
{
    Define_t		       *DefList;
    register char	       *Model;
    register int		i;

    Model = GetModelHPUX();
    if (!Model)
	return((char *) NULL);

    for (DefList = DefGetList(DL_CPU); DefList; DefList = DefList->Next)
	if (DefList->KeyStr && 
	    strncmp(Model, DefList->KeyStr, strlen(DefList->KeyStr)) == 0)
	    return(DefList->ValStr1);

    return((char *) NULL);
}

/*
 * Determine system kernel architecture using sysconf().
 */
extern char *GetKernArchHPUX2()
{
    long			CpuVersion;
    long			IOtype;

    CpuVersion = sysconf(_SC_CPU_VERSION);
    if (CpuVersion < 0) {
	SImsg(SIM_GERR, "sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    IOtype = sysconf(_SC_IO_TYPE);
    if (IOtype < 0) {
	SImsg(SIM_GERR, "sysconf(_SC_IO_TYPE) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    /*
     * Info gathered from sysconf(2) man page.
     * XXX - This will only work for PA-RISC machines.
     */
    if (CPU_IS_PA_RISC(CpuVersion))
	switch (IOtype) {
	case IO_TYPE_WSIO:		return(HP_KA_HP700);
	case IO_TYPE_SIO:		return(HP_KA_HP800);
#if	defined(IO_TYPE_CDIO) && defined(HP_KA_HPCOMB)
	case IO_TYPE_CDIO:		return(HP_KA_HPCOMB);
#endif
	default:
	    SImsg(SIM_UNKN, "Unknown IOtype: %d.", IOtype);
	}

    return((char *) NULL);
}

/*
 * Determine system application architecture using sysconf().
 */
extern char *GetAppArchHPUX()
{
    long			CpuVersion;

    CpuVersion = sysconf(_SC_CPU_VERSION);
    if (CpuVersion < 0) {
	SImsg(SIM_GERR, "sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    if (CPU_IS_PA_RISC(CpuVersion))
	return(HP_AA_PARISC);
    else if (CPU_IS_HP_MC68K(CpuVersion))
	return(HP_AA_MC68K);

    return((char *) NULL);
}

/*
 * Determine CPU type using sysconf().
 */
extern char *GetCpuTypeHPUX()
{
    long			CpuVersion;
    register int		i;
    Define_t		       *Def;

    CpuVersion = sysconf(_SC_CPU_VERSION);
    if (CpuVersion < 0) {
	SImsg(SIM_GERR, "sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    Def = DefGet(DL_CPU, NULL, CpuVersion, 0);
    if (Def && Def->ValStr1)
	return(Def->ValStr1);

    return((char *) NULL);
}

/*
 * Get Number of CPU's using the (currently) undocumented pstat() call.
 */
extern char *GetNumCpuPSTAT()
{
    union pstun			pstatbuff;
    struct pst_dynamic		pst;
    int				Status;
    static char			Buff[10];
    long			Num;

    pstatbuff.pst_dynamic = &pst;
    Status = pstat(PSTAT_DYNAMIC, pstatbuff, sizeof(pst), 0, 0);
    if (Status == -1) {
	SImsg(SIM_GERR, "pstat(PSTAT_DYNAMIC) returned status %d.", Status);
	return((char *) NULL);
    }

    Num = pst.psd_proc_cnt;
    if (Num < 1) {
	SImsg(SIM_GERR, "pstat(PSTAT_DYNAMIC) gave bad processor count (%d).",
	      Num);
	return((char *) NULL);
    }
    (void) snprintf(Buff, sizeof(Buff),  "%d", Num);

    return(Buff);
}

/*
 * Get size of virtual memory.
 */
extern char *GetVirtMemHPUX1()
{
#if	OSMVER <= 10
    static char			swdevtSYM[] = "swdevt";
    static char			nswapdevSYM[] = "nswapdev";
    struct nlist	       *nlptr;
    struct swdevt		Swap;
    long			NumSwap;
    Large_t			Amount = 0;
    off_t			Addr;
    kvm_t		       *kd;

    if (!(kd = KVMopen()))
	return((char *) NULL);

    /*
     * Get the number of swap devices
     */
    if ((nlptr = KVMnlist(kd, nswapdevSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);
    if (CheckNlist(nlptr))
	return((char *) NULL);
    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &NumSwap, 
	       sizeof(NumSwap), KDT_DATA)) {
	SImsg(SIM_GERR, "Cannot read `%s' from kernel.", nswapdevSYM);
	return((char *) NULL);
    }

    /*
     * Get the address of the start of the swdevt table.
     */
    if ((nlptr = KVMnlist(kd, swdevtSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);
    if (CheckNlist(nlptr))
	return((char *) NULL);
    Addr = nlptr->n_value;

    /*
     * Iterate over each swap device entry
     */
    while (NumSwap-- > 0) {
	if (KVMget(kd, Addr, (char *) &Swap, sizeof(Swap), KDT_DATA)) {
	    SImsg(SIM_GERR, "Cannot read `%s' at 0x%x", swdevtSYM, Addr);
	    continue;
	}

	Addr += sizeof(struct swdevt);

	if (Swap.sw_dev < 0 || !Swap.sw_enable)
	    continue;

#if OSMVER >= 10
	Amount += (Large_t) (Swap.sw_nblksavail * DEV_BSIZE);
#else
	Amount += (Large_t) (Swap.sw_nblks * DEV_BSIZE);
#endif
    }

    KVMclose(kd);

    return(GetSizeStr(Amount, BYTES));
#endif	/* OSMVER <= 10 */
}

/*
 * Get size of virtual memory using pstat()
 */
extern char *GetVirtMemHPUX2()
{
#if	OSMVER >= 10
#define MAXSWAP 10
    static Large_t		Amount;
    static struct pst_swapinfo	SwapInfo[MAXSWAP];
    register int		i;

    if (!Amount) {
	if (pstat_getswap(SwapInfo, sizeof(struct pst_swapinfo), 
			  MAXSWAP, 0) < 0) {
	    SImsg(SIM_GERR, "pstat_getswap() failed: %s", SYSERR);
	    return((char *) NULL);
	}

	for (i = 0; i < MAXSWAP; ++i) {
	    SImsg(SIM_DBG, 
		  "swapinfo: idx=%d mnt=<%s> flags=0x%x blks=%d size=%d",
		  i, SwapInfo[i].pss_mntpt, SwapInfo[i].pss_flags,
		  SwapInfo[i].pss_nblksenabled, SwapInfo[i].pss_swapchunk);
	    /*
	     * nblksenabled appears to be in Kbytes.  <sys/pstat.h> claims
	     * pss_swapchunk is "block size", but that tends to be 2K which
	     * doesn't jive with output from swapinfo(8).  Hence, we assume
	     * here that nblksenabled is just in Kbytes.
	     */
	    if (FLAGS_ON(SwapInfo[i].pss_flags, SW_ENABLED))
		Amount += SwapInfo[i].pss_nblksenabled;
	}

#undef MAXSWAP

	SImsg(SIM_DBG, "GetVirtMemHPUX2: Amount=%d", Amount);
    }

    return(GetSizeStr(Amount, KBYTES));
#endif	/* OSMVER >= 10 */
}

/*
 * Get amount of physical memory 
 * using the (currently) undocumented pstat() call.
 */
extern char *GetMemoryPSTAT()
{
    union pstun			pstatbuff;
    struct pst_static		pst;
    int				Status;
    Large_t			Num;

    pstatbuff.pst_static = &pst;
    Status = pstat(PSTAT_STATIC, pstatbuff, sizeof(pst), 0, 0);
    if (Status == -1) {
	SImsg(SIM_GERR, "pstat(PSTAT_STATIC) returned status %d.", Status);
	return((char *) NULL);
    }

    Num = (Large_t)pst.physical_memory * (Large_t)pst.page_size;
    if (Num < 1) {
	SImsg(SIM_GERR, "pstat(PSTAT_STATIC) gave bad physical_memory (%d).",
	      Num);
	return((char *) NULL);
    }

    return(GetSizeStr(Num, BYTES));
}

#if	defined(HAVE_DEVICE_SUPPORT)
/*
 * Issue a SCSI command and return the results.
 * Uses the scsi_ctl(7) interface.
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
#if	defined(HAVE_SCSI_CTL)
    static char			Buff[SCSI_BUF_LEN];
    static struct sctl_io	Cmd;
    ScsiCdbG0_t		       *CdbPtr = NULL;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Cmd, 0, sizeof(Cmd));

    (void) memcpy(Cmd.cdb, (caddr_t) ScsiCmd->Cdb, ScsiCmd->CdbLen);
    Cmd.cdb_length = ScsiCmd->CdbLen;
    Cmd.data = (caddr_t) Buff;
    Cmd.data_length = sizeof(Buff);
    Cmd.max_msecs = MySCSI_CMD_TIMEOUT * 1000;	/* max_msecs==milliseconds */
    Cmd.flags = SCTL_READ;

    /* 
     * We just need Cdb.cmd so it's ok to assume ScsiCdbG0_t here
     */
    CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;

    /*
     * Send cmd to device
     */
    if (ioctl(ScsiCmd->DevFD, SIOC_IO, &Cmd) == -1) {
	SImsg(SIM_GERR, "%s: ioctl SIOC_IO for SCSI 0x%x failed: %s", 
	      ScsiCmd->DevFile, CdbPtr->cmd, SYSERR);
	return(-1);
    }

    if (Cmd.cdb_status != S_GOOD) {
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed with cdb_status=0x%x", 
	      ScsiCmd->DevFile, CdbPtr->cmd, Cmd.cdb_status);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
#endif	/* HAVE_SCSI_CTL */
    return(0);
}

/*
 * Probe a disk drive.  
 * Set as much DiskDrive info in DevInfo as we can find.
 */
static DevInfo_t *ProbeDiskFile(ProbeData_t *ProbeData, char *DevFile)
{
    int				fd;
    int				DescStatus = -1;
    int				CapStatus = -1;
    unsigned int		BlkSize = DEV_BSIZE;
    DevInfo_t		       *DevInfo = NULL;
    DiskDriveData_t	       *DiskDriveData = NULL;
    DiskDrive_t		       *HWdisk;
    disk_describe_type		Describe;
    capacity_type		Capacity;
    int				DevType = 0;
    Define_t		       *Def;
    char		       *cp;

    if (!ProbeData || !ProbeData->UseDevInfo || !DevFile)
	return((DevInfo_t *) NULL);

    if (!EQN(DevFile, "/dev/r", 6))
	return((DevInfo_t *) NULL);

    DevInfo = ProbeData->UseDevInfo;

    /*
     * If Vendor isn't set, but Model is and there's a SPACE in Model,
     * then assume the format is <Vendor Model...> and make it so.
     */
    if (!DevInfo->Vendor && DevInfo->Model && 
	(cp = strchr(DevInfo->Model, ' '))) {

	DevInfo->Vendor = DevInfo->Model;
	*cp = CNULL;
	DevInfo->Model = cp + 1;
    }

    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open O_RDONLY failed: %s", DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    }

    /*
     * Setup Disk Drive info
     */
    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }
    if (DiskDriveData->HWdata)
	HWdisk = DiskDriveData->HWdata;
    else
	HWdisk = DiskDriveData->HWdata = NewDiskDrive(NULL);

#if	defined(DIOC_DESCRIBE)
    DescStatus = ioctl(fd, DIOC_DESCRIBE, &Describe);
    if (DescStatus == 0) {
	SImsg(SIM_DBG, 
	      "%s: DIOC_DESCRIBE lgblksz=%.0f model=<%s> intf=%d devtype=%d",
	      DevFile, (float)Describe.lgblksz, Describe.model_num,
	      Describe.intf_type, Describe.dev_type);
	if (Describe.lgblksz)
	    BlkSize = Describe.lgblksz;

	DevType = 0;
	switch (Describe.dev_type) {
	case DISK_DEV_TYPE:	DevType = DT_DISKDRIVE;		break;
	case CDROM_DEV_TYPE:	DevType = DT_CDROM;		break;
	default:
	    if (!DevInfo->ModelDesc) {
		Def = DefGet("DiskDevType", NULL, Describe.dev_type, 0);
		if (Def)
		    DevInfo->ModelDesc = Def->ValStr1;
	    }
	}
	if (!DevInfo->Type && DevType)
	    DevInfo->Type = DevType;

    } else
	SImsg(SIM_GERR, "%s: ioctl DIOC_DESCRIBE failed: %s", DevFile, SYSERR);
#endif	/* DIOC_DESCRIBE */

#if	defined(DIOC_CAPACITY)
    if (!HWdisk->Size) {
	CapStatus = ioctl(fd, DIOC_CAPACITY, &Capacity);
	if (CapStatus == 0) {
	    HWdisk->Size = (float) (Capacity.lba / DEV_BSIZE);
	    SImsg(SIM_DBG, "%s: DIOC_CAPACITY lba=%.0f",
		  DevFile, (float)Capacity.lba);
	} else
	    SImsg(SIM_GERR, "%s: ioctl DIOC_CAPACITY failed: %s", 
		  DevFile, SYSERR);
    }
#endif	/* DIOC_CAPACITY */

    if (!HWdisk->SecSize)
	HWdisk->SecSize = BlkSize;

    return(DevInfo);
}
/*
 * Probe a DiskDrive device.
 */
extern DevInfo_t *ProbeDiskDrive(ProbeData_t *ProbeData)
{
    DevInfo_t		       *dPtr;
    char		      **DevFile;
    char		       *File;
    static char			Path[MAXPATHLEN];

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeDiskDrive(%s)", ProbeData->DevName);

    /*
     * Perform SCSI queries of the device
     */
    if (ProbeData->UseDevInfo->Files && ProbeData->UseDevInfo->Files[0]) {
	if (File = strrchr(ProbeData->UseDevInfo->Files[0], '/')) {
	    ++File;
	    /*
	     * First try the /dev/rscsi device.  If that fails,
	     * try the /dev/rdsk device.
	     * For ScsiQuery() to succeed, we must be able to open
	     * the devices Read/Write.
	     */
	    (void) snprintf(Path, sizeof(Path), "%s/%s",
			    _PATH_DEV_RSCSI, File);
	    if (ScsiQuery(ProbeData->UseDevInfo, Path, -1, TRUE) != 0) {
		(void) snprintf(Path, sizeof(Path), "%s/%s",
				_PATH_DEV_RDSK, File);
		(void) ScsiQuery(ProbeData->UseDevInfo, Path, -1, TRUE);
	    }
	}
    }

    /*
     * Call the actual probe routine with a device file name until 
     * we succeed.
     */
    for (DevFile = ProbeData->UseDevInfo->Files; DevFile && DevFile[0]; 
	 ++DevFile)
	if (dPtr = ProbeDiskFile(ProbeData, *DevFile))
	    return(dPtr);

    return((DevInfo_t *) NULL);
}

/*
 * Probe a TapeDrive device.
 * Only works if media is loaded in the drive so open() succeeds.
 */
extern DevInfo_t *ProbeTapeDrive(ProbeData_t *ProbeData)
{
    char		       *File;
    static char			Path[MAXPATHLEN];

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeTapeDrive(%s)", ProbeData->DevName);

    /*
     * Perform SCSI queries of the device
     */
    if (ProbeData->UseDevInfo->Files && ProbeData->UseDevInfo->Files[0]) {
	if (File = strrchr(ProbeData->UseDevInfo->Files[0], '/')) {
	    ++File;
	    /*
	     * First try the /dev/rscsi device.  If that fails,
	     * try the /dev/rdsk device.  
	     * For ScsiQuery() to succeed, we must be able to open
	     * the devices Read/Write.
	     */
	    (void) snprintf(Path, sizeof(Path), "%s/%s",
			    _PATH_DEV_RSCSI, File);
	    if (ScsiQuery(ProbeData->UseDevInfo, Path, -1, TRUE) != 0) {
		(void) snprintf(Path, sizeof(Path), "%s/%s",
				_PATH_DEV_RMT, File);
		(void) ScsiQuery(ProbeData->UseDevInfo, Path, -1, TRUE);
	    }
	}
    }

    return(ProbeData->UseDevInfo);
}

/*
 * Set model info in DevInfo to be NewModel.
 * If Model is already present, then prepend.
 */
static void SetModel(DevInfo_t *DevInfo, char *NewModel)
{
    register char	       *cp;

    if (!DevInfo || !NewModel)
	return;

    if (!DevInfo->Model)
	DevInfo->Model = NewModel;
    else {
	cp = (char *) xmalloc(strlen(DevInfo->Model) +
			      strlen(NewModel) + 2);
	(void) snprintf(cp, sizeof(cp),  "%s %s", NewModel, DevInfo->Model);
	free(DevInfo->Model);
	DevInfo->Model = cp;
    }
}

/*
 * Probe a FrameBuffer file.
 * Set as much info in DevInfo as we can find.
 */
static DevInfo_t *ProbeFrameBufferFile(ProbeData_t *ProbeData, char *DevFile)
{
    int				fd;
    FrameBuffer_t	       *FrameBuffer = NULL;
    DevInfo_t		       *DevInfo;
    Define_t		       *Define;
    crt_frame_buffer_t		CrtFB;

    if (!ProbeData || !ProbeData->UseDevInfo || !DevFile)
	return((DevInfo_t *) NULL);
    DevInfo = ProbeData->UseDevInfo;

    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open O_RDONLY failed: %s", DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    }

#if	defined(GCDESCRIBE)
    if (ioctl(fd, GCDESCRIBE, &CrtFB) != 0) {
	SImsg(SIM_GERR, "%s: ioctl GCDESCRIBE failed: %s", DevFile, SYSERR);
	(void) close(fd);
	return((DevInfo_t *) NULL);
    }

    FrameBuffer = NewFrameBuffer(NULL);

    if (Define = DefGet("FrameBufID", NULL, CrtFB.crt_id))
	SetModel(DevInfo, Define->ValStr1);
    else
	SImsg(SIM_UNKN, "Unknown FrameBufID %d", CrtFB.crt_id);

    if (CrtFB.crt_name[0])
	SetModel(DevInfo, strdup(CrtFB.crt_name));
    FrameBuffer->Size = CrtFB.crt_map_size;
    FrameBuffer->Height = CrtFB.crt_total_y;
    FrameBuffer->Width = CrtFB.crt_total_x;
    FrameBuffer->CMSize = CrtFB.crt_planes;
    AddDevDesc(DevInfo, itoa(CrtFB.crt_bits_per_pixel), 
	       "Bits per Pixel", DA_APPEND);
    AddDevDesc(DevInfo, GetSizeStr(CrtFB.crt_x_pitch, BYTES), 
	       "Length of row", DA_APPEND);
    AddDevDesc(DevInfo, GetSizeStr(CrtFB.crt_plane_size, BYTES), 
	       "Total Plane Size", DA_APPEND);

    /*
     * Check list of possible attributes and add those that this fb has
     */
    for (Define = DefGetList("FBAttr"); Define; Define = Define->Next)
	if (FLAGS_ON(CrtFB.crt_attributes, Define->KeyNum))
	    AddDevDesc(DevInfo, Define->ValStr1, NULL, DA_APPEND);

    DevInfo->DevSpec = FrameBuffer;
#endif	/* GCDESCRIBE */

    (void) close(fd);

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Probe a FrameBuffer device.
 */
extern DevInfo_t *ProbeFrameBuffer(ProbeData_t *ProbeData)
{
    DevInfo_t		       *dPtr;
    char		      **DevFile;

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeFrameBuffer(%s)", ProbeData->DevName);

    /*
     * Call the actual probe routine with a device file name until 
     * we succeed.
     */
    if (!ProbeData->UseDevInfo->Files) {
	if (dPtr = ProbeFrameBufferFile(ProbeData, FB_DEFAULT_FILE))
	    return(dPtr);
    } else {
	for (DevFile = ProbeData->UseDevInfo->Files; DevFile && DevFile[0]; 
	     ++DevFile)
	    if (dPtr = ProbeFrameBufferFile(ProbeData, *DevFile))
		return(dPtr);
    }

    return((DevInfo_t *) NULL);
}

/*
 * Probe an Audio file
 * Set as much Audio info in DevInfo as we can find.
 */
static DevInfo_t *ProbeAudioFile(ProbeData_t *ProbeData, char *DevFile)
{
    int				fd;
    struct audio_describe	ADesc;
    Define_t		       *Define;
    DevInfo_t		       *DevInfo;

    if (!ProbeData || !ProbeData->UseDevInfo || !DevFile)
	return((DevInfo_t *) NULL);

    DevInfo = ProbeData->UseDevInfo;

    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open O_RDONLY failed: %s", DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    }

    if (ioctl(fd, AUDIO_DESCRIBE, &ADesc) != 0) {
	SImsg(SIM_GERR, "%s: ioctl AUDIO_DESCRIBE failed: %s", 
	      DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    }

    /*
     * Lookup the model
     */
    if (Define = DefGet("AudioTypes", NULL, ADesc.audio_id)) {
	SetModel(DevInfo, Define->ValStr1);
    } else {
	SImsg(SIM_UNKN, "Unknown AudioType %d", ADesc.audio_id);
    }

    /*
     * Add what else we care about
     */
    AddDevDesc(DevInfo, itoa(ADesc.nrates), "Number of Rates", DA_APPEND);
    AddDevDesc(DevInfo, itoa(ADesc.nchannels), "Number Channels", DA_APPEND);
    AddDevDesc(DevInfo, itoa(ADesc.max_bits_per_sample), 
	       "Sample Rate", DA_APPEND);

    return(DevInfo);
}
	
/*
 * Probe Audio device.
 */
static DevInfo_t *ProbeAudio(ProbeData_t *ProbeData)
{
    DevInfo_t		       *dPtr;
    char		      **DevFile;

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeAudio(%s)", ProbeData->DevName);

    /*
     * Call the actual probe routine with a device file name until 
     * we succeed.
     */
    for (DevFile = ProbeData->UseDevInfo->Files; DevFile && DevFile[0]; 
	 ++DevFile)
	if (dPtr = ProbeAudioFile(ProbeData, *DevFile))
	    return(dPtr);

    return((DevInfo_t *) NULL);
}

/*
 * Probe CPU device.
 */
static DevInfo_t *ProbeCPU(ProbeData_t *ProbeData)
{
    DevInfo_t		       *DevInfo;
    static char			Buff[128];
    char		       *cp;
    int				Speed;

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    DevInfo = ProbeData->UseDevInfo;

    /*
     * This is all stupid stuff that there has to be a better way for!
     */
    Speed = GetCPUSpeed();
    Buff[0] = CNULL;
    if (Speed > 0)
	(void) snprintf(Buff, sizeof(Buff),  "%d MHz", Speed);

    if (Buff[0])
	SetModel(DevInfo, strdup(Buff));

    return(DevInfo);
}

/*
 * Probe and get info about a Keyboard device.
 */
static DevInfo_t *ProbeKeyboard(ProbeData_t *ProbeData, int KbdIdent)
{
    DevInfo_t		       *DevInfo = NULL;
    int				Status;
    int				KbdType;
    char			LangID[2];
    char			LangDef[64];
    char		       *Model;
    Define_t		       *Define;

    DevInfo = NewDevInfo(NULL);
    DevInfo->Type = DT_KEYBOARD;
    DevInfo->Model = "Unknown Type";

    /*
     * Look up the type of keyboard
     */
    KbdType = KBD_IDCODE_MASK & KbdIdent;
    if (Define = DefGet("KbdType", NULL, KbdType, 0)) {
	DevInfo->Model = Define->ValStr1;
	AddDevDesc(DevInfo, Define->ValStr1, "Type", DA_INSERT);
    }

    /*
     * Now get the keyboard's Language based on the type.
     */
    Status = ioctl(ProbeData->FileDesc, KBD_READ_LANGUAGE, &LangID);
    if (Status != 0) {
	SImsg(SIM_GERR, "%s: ioctl KBD_READ_LANGUAGE failed: %s", 
	      ProbeData->DevFile, SYSERR);
    } else {
	(void) snprintf(LangDef, sizeof(LangDef),  "KbdLang%d", KbdType);
	if ((Define = DefGet(LangDef, NULL, LangID[0], 0)) &&
	    Define->ValStr1) {
	    Model = (char *) xcalloc(1, strlen(DevInfo->Model) + 
				     strlen(Define->ValStr1 + 2));
	    (void) snprintf(Model, sizeof(Model), "%s %s", 
			    DevInfo->Model, Define->ValStr1);
	    DevInfo->Model = Model;
	    AddDevDesc(DevInfo, Define->ValStr1, "Language", DA_APPEND);
	}
    }

    return(DevInfo);
}

/*
 * Probe an HIL device file.
 */
static DevInfo_t *ProbeHILFile(ProbeData_t *ProbeData)
{
    DevInfo_t		       *dPtr = NULL;
    int				fd;
    int				Status;
    char			Ident[128];
    char		       *DevName;
    char		       *DevFile;
    char		       *cp;

    if (!(DevFile = ProbeData->DevFile) || !(DevName = ProbeData->DevName))
	return((DevInfo_t *) NULL);
    
    /*
     * Only bother with keyboard devices since open() on all other HIL
     * devices always results in EBUSY.
     */
    fd = open(DevFile, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open O_RDONLY failed: %s", DevFile, SYSERR);
	return((DevInfo_t *) NULL);
    } else
	SImsg(SIM_DBG, "%s: open O_RDONLY succeeded", DevFile);
    ProbeData->FileDesc = fd;

    if (EQ(DevName, "hilkbd")) {
#if	defined(KBD_READ_CONFIG)
	if (ioctl(fd, KBD_READ_CONFIG, &Ident) != 0) {
	    SImsg(SIM_GERR, "%s: ioctl KBD_READ_CONFIG failed: %s", 
		  DevFile, SYSERR);
	} else
	    dPtr = ProbeKeyboard(ProbeData, Ident[0]);
#endif	/* KBD_READ_CONFIG */
    }

    close(fd);
    ProbeData->FileDesc = -1;

    return(dPtr);
}

/*
 * Probe a HIL device.
 */
static void ProbeHIL(ProbeData_t *ProbeData)
{
    DevInfo_t		       *dPtr;
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *LastPtr = NULL;
    char		      **DevFile;
    char		       *cp;

    if (!ProbeData->UseDevInfo)
	return;

    DevInfo = ProbeData->UseDevInfo;

    SImsg(SIM_DBG, "ProbeHIL(%s)", DevInfo->Name);

    /*
     * Call the actual probe routine with a device file name until 
     * we succeed.
     */
    for (DevFile = DevInfo->Files; DevFile && DevFile[0]; ++DevFile) {
	if (cp = strrchr(*DevFile, '/'))
	    ProbeData->DevName = ++cp;
	else
	    ProbeData->DevName = *DevFile;
	ProbeData->DevFile = *DevFile;
	if (dPtr = ProbeHILFile(ProbeData)) {
	    /* Set the device's Name if not already set */
	    if (!dPtr->Name)
		dPtr->Name = ProbeData->DevName;
	    /* Set the device's File name if not already set */
	    DevAddFile(dPtr, *DevFile);
	    /* Add it to the Slaves list */
	    if (LastPtr) {
		LastPtr->Next = dPtr;
		LastPtr = dPtr;
	    } else
		LastPtr = DevInfo->Slaves = dPtr;
	}
    }
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
    register int		i;

    for (i = 0; DevTypes[i].Name; ++i)
	switch (DevTypes[i].Type) {
	case DT_AUDIO:		DevTypes[i].Probe = ProbeAudio;		break;
	case DT_CPU:		DevTypes[i].Probe = ProbeCPU;		break;
	case DT_DISKDRIVE:	DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
	case DT_FRAMEBUFFER:	DevTypes[i].Probe = ProbeFrameBuffer;	break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	}
}

#if	defined(SETMACINFO_FUNC)
#include <netio.h>

/*
 * Convert MAC address found in AddrStr into human readable form.
 */
static char *ConvertMAC(char *AddrStr, int AddrLen)
{
    static char		        AddrBuf[64];

    (void) snprintf(AddrBuf, sizeof(AddrBuf),  "%x:%x:%x:%x:%x:%x",
		   (unsigned char) AddrStr[0], (unsigned char) AddrStr[1], 
		   (unsigned char) AddrStr[2], (unsigned char) AddrStr[3], 
		   (unsigned char) AddrStr[4], (unsigned char) AddrStr[5]);

    return( strdup(AddrBuf) );
}

/*
 * HPUX Set MAC Info function.
 * Use the interfaces described in lan(7).
 */
extern void HPUXSetMacInfo(DevInfo_t *DevInfo, NetIF_t *NetIf)
{
    register char	      **DevFiles;
    struct fis			fis;
    int				fd;

    if (!DevInfo || !NetIf)
	return;

    /*
     * Try each device file until we set at least MACaddr.
     */
    for (DevFiles = DevInfo->Files; 
	 DevFiles && *DevFiles && !NetIf->MACaddr; ++DevFiles) {

	fd = open(*DevFiles, O_RDONLY|O_NDELAY);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: open O_RDONLY failed: %s", *DevFiles,SYSERR);
	    continue;
	}

	/*
	 * See lan(7) for details
	 */
#if	defined(LOCAL_ADDRESS)
	fis.reqtype = LOCAL_ADDRESS;
	if (ioctl(fd, NETSTAT, &fis) == 0) {
	    NetIf->MACaddr = ConvertMAC((char *)fis.value.s, fis.vtype);
	} else {
	    SImsg(SIM_GERR, "%s: ioctl NETSTAT.LOCAL_ADDRESS failed: %s", 
		  *DevFiles, SYSERR);
	    (void) close(fd);
	    continue;
	}
#endif	/* LOCAL_ADDRESS */

#if	defined(PERMANENT_ADDRESS)
	fis.reqtype = PERMANENT_ADDRESS;
	if (ioctl(fd, NETSTAT, &fis) == 0) {
	    NetIf->FacMACaddr = ConvertMAC((char *)fis.value.s, fis.vtype);
	} else {
	    SImsg(SIM_GERR, "%s: ioctl NETSTAT.PERMANENT_ADDRESS failed: %s", 
		  *DevFiles, SYSERR);
	    (void) close(fd);
	    continue;
	}
#endif	/* PERMANENT_ADDRESS */

	(void) close(fd);
    }
}
#endif	/* SETMACINFO_FUNC */

/*
 * Check to see if IOSmaster is the master for IOSchild
 */
static int IOScheckMaster(IOSdata_t *IOSchild, IOSdata_t *IOSmaster)
{
    char		       *Last;
    int				SWLen = 0;
    int				HWLen = 0;

    if (Last = strrchr(IOSchild->SWpath, '.'))
	SWLen = Last - IOSchild->SWpath;
    else
	SWLen = strlen(IOSchild->SWpath);

#ifdef notdef
    /*
     * Check if the possible Master's (IOSchild) BusUnit == the
     * child's (IOSmaster) Unit AND the SWpath aligns
     */
    if ((IOSmaster->Unit == IOSchild->BusUnit) &&
	(IOSmaster->BusUnit == IOSchild->BusUnit) &&
	EQN(IOSmaster->SWpath, IOSchild->SWpath, SWLen))
	return(TRUE);
#endif

    /*
     * Does the software path (e.g. "root.bus_adaptor") and the
     * hardware path (e.g "4/0/1") match?
     */
    if (IOSmaster->HWpath)
	HWLen = strlen(IOSmaster->HWpath);

    if (EQN(IOSmaster->HWpath, IOSchild->HWpath, HWLen) &&
	EQN(IOSmaster->SWpath, IOSchild->SWpath, SWLen))
	return(TRUE);

    return(FALSE);
}

/*
 * Find the master device for client IOSdata.
 */
static DevInfo_t *IOSfindMaster(DevInfo_t *DevInfo, IOSdata_t *IOSdata)
{
    IOSdata_t		       *iPtr;
    DevInfo_t		       *dPtr;
    DevInfo_t		       *FoundDevInfo;

    if (!DevInfo || !IOSdata)
	return((DevInfo_t *) NULL);

    if (iPtr = (IOSdata_t *) DevInfo->OSdata)
	if (IOScheckMaster(IOSdata, iPtr))
	    return(DevInfo);

    for (dPtr = DevInfo->Slaves; dPtr; dPtr = dPtr->Next)
	if (FoundDevInfo = IOSfindMaster(dPtr, IOSdata))
	    return(FoundDevInfo);

    for (dPtr = DevInfo->Next; dPtr; dPtr = dPtr->Next)
	if (FoundDevInfo = IOSfindMaster(dPtr, IOSdata))
	    return(FoundDevInfo);

    return((DevInfo_t *) NULL);
}

/*
 * Take all the data we found from ioscan(1m) found in IOSdataList
 * and build a Device_t tree into TreePtr.
 */
static int IOSdataToDevice(DevInfo_t **TreePtr, char **SearchNames, 
			   IOSdata_t *IOSdataList)
{
    static ProbeData_t		ProbeData;
    IOSdata_t		       *IOSdata;
    DevInfo_t		       *DevInfo = NULL;
    DevDefine_t		       *DevDefine;
    char		       *cp;

    for (IOSdata = IOSdataList; IOSdata; IOSdata = IOSdata->Next) {
	SImsg(SIM_DBG, 
	"Driver=<%s> NexusN=<%s> DevType=<%s> DevClass=<%s> BusType=<%s>",
	      IOSdata->Driver, IOSdata->NexusName,
	      IOSdata->DevType, IOSdata->DevClass, IOSdata->BusType);
	SImsg(SIM_DBG, 
	      "\tHWpath=<%s> SWpath=<%s> Unit=%d BusUnit=%d",
	      IOSdata->HWpath, IOSdata->SWpath, IOSdata->Unit,
	      IOSdata->BusUnit);
	SImsg(SIM_DBG, "\tDesc=<%s>", IOSdata->Desc);

	if (!IOSdata->DevClass)
	    continue;

	DevDefine = DevDefGet(IOSdata->DevClass, 0, 0);
	if (!DevDefine) {
	    SImsg(SIM_DBG, "No such device type as `%s' - IGNORED.",
		  IOSdata->DevClass);
	    continue;
	}

	DevInfo = NewDevInfo(NULL);
	DevInfo->Name = MkDevName(IOSdata->Driver, IOSdata->Unit,
				  DevDefine->Type, DevDefine->Flags);
	/* Set alias/alt names if different from primary */
	if (cp = MkDevName(IOSdata->DevClass, IOSdata->Unit,
			   DevDefine->Type, DevDefine->Flags))
	    if (!EQ(DevInfo->Name, cp)) {
		DevInfo->Aliases = (char **) xcalloc(2, sizeof(char *));
		DevInfo->Aliases[0] = cp;
		DevInfo->AltName = cp;
	    }

	DevInfo->Type = DevDefine->Type;
	DevInfo->Unit = IOSdata->Unit;
	DevInfo->Model = IOSdata->Desc;
	DevInfo->ModelDesc = DevDefine->Desc;
	DevInfo->Files = IOSdata->DevFiles;
	AddDevDesc(DevInfo, IOSdata->SWstate, "Software State", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->SWpath, "Software Path", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->HWpath, "Hardware Path", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevClass, "OS Device Class", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevType, "OS Device Type", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevCat, "OS Device Category", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->BusType, "OS Bus Type", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->NexusName, "Nexus Name", DA_APPEND);
	AddDevDesc(DevInfo, IOSdata->DevData, "Device Data", DA_APPEND);
	if (IOSdata->BusUnit >= 0)
	    AddDevDesc(DevInfo, itoa(IOSdata->BusUnit), "Bus Unit", DA_APPEND);

	DevInfo->OSdata = (void *) IOSdata;
	if (DevInfo->Master = IOSfindMaster(*TreePtr, IOSdata))
	    DevInfo->MasterName = DevInfo->Master->Name;

	/*
	 * Prep data needed for probe functions.
	 */
	(void) memset(&ProbeData, CNULL, sizeof(ProbeData));
	ProbeData.DevName = DevInfo->Name;
	ProbeData.AliasNames = DevInfo->Aliases;
	ProbeData.DevDefine = DevDefine;
	ProbeData.UseDevInfo = DevInfo;

	/*
	 * Special check for HP specific HIL
	 */
	if (EQ("hil", DevDefine->Name))
	    ProbeHIL(&ProbeData);
	/*
	 * Call device specific probe routine, if any
	 */
	else if ((*DevDefine->Probe) != NULL)
	    (*DevDefine->Probe)(&ProbeData);

	AddDevice(DevInfo, TreePtr, SearchNames);
    }

    return(0);
}

/*
 * Parse an input line from ioscan(1m) containg a list of files and add
 * to IOSdata->DevFiles list.
 */
static int IOSdataParseFiles(char *LineStr, int LineNo, IOSdata_t *IOSdata)
{
    char		      **Argv = NULL;
    int				Argc;
    register int		Count = 0;
    register int		OldCount = 0;
    register int		i;
    char		      **Ptr = NULL;

    if (!IOSdata)
	return(-1);

    /*
     * File names are seperated by white space
     */
    while (isspace(*LineStr))
	++LineStr;
    Argc = StrToArgv(LineStr, " ", &Argv, NULL, 0);
    for (i = 0; i < Argc; ++i) {
	if (!Argv[i] || Argv[i][0] != '/')
	    continue;
	++Count;
    }

    /*
     * XXX realloc() appears to be broken so we have to do things the 
     * hard way.
     */
#ifndef MAXDEVFILES
#define MAXDEVFILES 48
#endif

    if (!IOSdata->DevFiles) {
	Ptr = IOSdata->DevFiles = (char **) xcalloc(MAXDEVFILES + 1, 
						    sizeof(char *));
    } else {
	for (Ptr = IOSdata->DevFiles; Ptr && *Ptr; ++Ptr, ++OldCount);
    }

    if (OldCount + Count > MAXDEVFILES) {
	SImsg(SIM_WARN, "WARNING: MAXDEVFILES (%d) exceeded.", MAXDEVFILES);
	return(-1);
    }

    for (i = 0; i < Argc; ++i) {
	if (!Argv[i] || Argv[i][0] != '/')
	    continue;
	*Ptr = Argv[i];
	*(++Ptr) = NULL;
    }

    return(0);
}

/*
 * Clean a string of unprintable characters and excess white-space.
 */
static char *CleanStr(char *String, int StrSize)
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
 * Parse an input line from ioscan(1m) and create an IOSdata_t
 */
static IOSdata_t *IOSdataParse(char *LineStr, int LineNo)
{
    char		      **Argv = NULL;
    int				Argc;
    IOSdata_t		       *IOSdata = NULL;

    Argc = StrToArgv(LineStr, ":", &Argv, NULL, 0);
    if (Argc != 19) {
	SImsg(SIM_WARN, 
	 "Input from %s: Line %d: Wrong number of fields (expect %d got %d).",
	 _PATH_IOSCAN, LineNo, 19+1, Argc+1);
	return((IOSdata_t *) NULL);
    }

    IOSdata = (IOSdata_t *) xcalloc(1, sizeof(IOSdata_t));
    IOSdata->Unit = IOSdata->BusUnit = -1;

    if (Argv[12]) IOSdata->Unit 	= atoi(Argv[12]);
    if (Argv[18]) IOSdata->BusUnit 	= atoi(Argv[18]);
    IOSdata->DevType		 	= Argv[16];
    IOSdata->DevClass 			= Argv[8];
    IOSdata->DevCat 			= Argv[0];
    IOSdata->DevData 			= Argv[11];
    IOSdata->BusType 			= Argv[1];
    IOSdata->Driver 			= Argv[9];
    IOSdata->NexusName 			= Argv[14];
    IOSdata->HWpath 			= Argv[10];
    IOSdata->SWpath 			= Argv[13];
    IOSdata->SWstate 			= Argv[15];
    if (Argv[17] && Argv[17][0])
	IOSdata->Desc 			= CleanStr(Argv[17], strlen(Argv[17]));

    return(IOSdata);
}

/*
 * Build the device tree using ioscan(1m)
 */
extern int BuildIOScan(DevInfo_t **TreePtr, char **SearchNames)
{
    IOSdata_t		       *IOSdataList = NULL;
    IOSdata_t		       *LastIOS = NULL;
    IOSdata_t		       *IOSdataPtr = NULL;
    FILE		       *IOScmd;
    int				LineNo = 0;
    char		        CmdBuff[sizeof(_PATH_IOSCAN) + 
				       sizeof(IOSCAN_ARGS) + 4];
    static char			LineBuff[8192];
    char		       *cp;

    if (!FileExists(_PATH_IOSCAN)) {
	SImsg(SIM_CERR, 
	      "The ioscan(1m) command is required for device support, but is not installed on this system as <%s>.", 
	      _PATH_IOSCAN);
	return(-1);
    }

    (void) snprintf(CmdBuff, sizeof(CmdBuff), "%s %s", 
		    _PATH_IOSCAN, IOSCAN_ARGS);
    SImsg(SIM_DBG, "Running <%s>", CmdBuff);
    if (!(IOScmd = popen(CmdBuff, "r"))) {
	SImsg(SIM_GERR, "popen of `%s' failed: %s", CmdBuff, SYSERR);
	return(-1);
    }

    while (fgets(LineBuff, sizeof(LineBuff), IOScmd)) {
	++LineNo;
	if (cp = strchr(LineBuff, '\n'))
	    *cp = CNULL;
	SImsg(SIM_DBG, "IOSCANINPUT: %s", LineBuff);
	if (strchr(LineBuff, ':')) {
	    /* Normal data line */
	    IOSdataPtr = IOSdataParse(LineBuff, LineNo);
	    if (LastIOS) {
		LastIOS->Next = IOSdataPtr;
		LastIOS = IOSdataPtr;
	    } else
		LastIOS = IOSdataList = IOSdataPtr;
	} else
	    /* Must be a list of files */
	    IOSdataParseFiles(LineBuff, LineNo, IOSdataPtr);
    }

    pclose(IOScmd);

#if	!defined(SYSINFO_DEV)
    if (Debug) {
	/*
	 * Provide more useful debugging info.
	 */
	(void) snprintf(CmdBuff, sizeof(CmdBuff),  "%s -k", _PATH_IOSCAN);
	SImsg(SIM_DBG, "Running <%s>", CmdBuff);
	if (!(IOScmd = popen(CmdBuff, "r"))) {
	    SImsg(SIM_GERR, "popen of `%s' failed: %s", CmdBuff, SYSERR);
	    return(-1);
	}
	while (fgets(LineBuff, sizeof(LineBuff), IOScmd))
	    SImsg(SIM_DBG|SIM_NONL, "IOSCANTREE: %s", LineBuff);
	pclose(IOScmd);
    }
#endif	/* SYSINFO_DEV */
	
    return(IOSdataToDevice(TreePtr, SearchNames, IOSdataList));
}

/*
 * Build the device tree
 */
extern int BuildDevices(DevInfo_t **TreePtr, char **SearchNames)
{
    return(BuildIOScan(TreePtr, SearchNames));
}
#endif	/* HAVE_DEVICE_SUPPORT */
