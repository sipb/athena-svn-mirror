/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
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
#if	defined(HAVE_SYS_DISKIO_H)
#include <sys/diskio.h>
#endif
#if	defined(HAVE_SYS_HILIOCTL_H)
#include <sys/hilioctl.h>
#endif
#if	defined(HAVE_SYS_AUDIO_H)
#include <sys/audio.h>
#endif
#if	defined(HAVE_SYS_FRAMEBUF_H)
#include <sys/framebuf.h>
#endif

#if	defined(HAVE_SCSI_CTL)
#include "myscsi.h"
#endif	/* HAVE_SCSI_CTL */
#if	defined(HAVE_SYS_SCSI_H)
#include <sys/scsi.h>
#endif

#include <sys/pstat.h>
#include <sys/conf.h>

/*
 * CPU Information type
 */
struct _CpuInfo {
    char		       *Model;		/* Model of CPU */
    char		       *Arch;		/* Architecture Name */
    char		       *ArchVer;	/* Architecture Version */
};
typedef struct _CpuInfo		CpuInfo_t;

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelHPUX1();
extern char			       *GetModelHPUX2();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelFile },
    { GetModelHPUX1 },
    { GetModelHPUX2 },
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
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
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
extern char			       *GetCpuTypeHPUX1();
extern char			       *GetCpuTypeHPUX2();
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeHPUX2 },
    { GetCpuTypeHPUX1 },
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
 * Paths to the "model" command
 */
static char		       *ModelPaths[] = {
    "/usr/bin/model", NULL
};

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
 * Get the speed (in MHz) of the CPU.
 */
static char 			CpuSpeedSYM[] = "itick_per_tick";
#ifdef notdef	/* This works to */
static char 			CpuSpeedSYM[] = "iticks_per_10_msec";
#endif
extern int GetCPUSpeedItick()
{
    struct nlist	       *nlptr;
    off_t			Addr;
    kvm_t		       *kd;
    static int			Speed;
    int				SpeedVar;
    long			ClockHz;

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

    ClockHz = sysconf(_SC_CLK_TCK);    /* Get the Clock cycle time in Hz */

    if (SpeedVar > 0)
	Speed = SpeedVar / ClockHz / 100;

    SImsg(SIM_DBG, "GetCPUSpeedItick: Speed=%d %s=%d ClockHz=%d",
	  Speed, CpuSpeedSYM, SpeedVar, ClockHz);

    return(Speed);
}

/*
 * Get the speed (in MHz) of the CPU using pstat()
 */
extern int GetCPUSpeedPstat()
{
    union pstun			pstatbuff;
    struct pst_processor	pst;
    static int			Speed;
    int				SpeedVar;
    long			ClockHz;
    int				Status;

    if (Speed)
	return(Speed);

    pstatbuff.pst_processor = &pst;
    Status = pstat(PSTAT_PROCESSOR, pstatbuff, sizeof(pst), (size_t)1, 0);
    if (Status == -1) {
	SImsg(SIM_GERR, "pstat(PSTAT_PROCESSOR) returned status %d.", Status);
	return 0;
    }

    SpeedVar = pst.psp_iticksperclktick;
    ClockHz = sysconf(_SC_CLK_TCK); /* Get the Clock cycle time in Hz */

    if (SpeedVar > 0)
	Speed = SpeedVar / ClockHz / 100;

    SImsg(SIM_DBG, "GetCPUSpeedPstat: Speed=%d Itick=%d ClockHz=%d",
	  Speed, SpeedVar, ClockHz);

    return(Speed);
}

/*
 * Get the CPU Speed (in MHz)
 * Assumes all CPU's are the same.
 */
extern int GetCPUSpeed()
{
    static int			Speed;

    if (Speed)
	return(Speed);

    if (!Speed)
	Speed = GetCPUSpeedPstat();

    if (!Speed)
	Speed = GetCPUSpeedItick();

    return Speed;
}

/*
 * HPUX Get Model name by running the "model" command
 */
extern char *GetModelHPUX1()
{
    return RunCmds(ModelPaths, FALSE);
}

/*
 * HPUX Get Model name using uname()
 */
extern char *GetModelHPUX2()
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
 * Determine system kernel architecture using sysconf().
 */
extern char *GetKernArchHPUX1()
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
extern char *GetCpuTypeHPUX1()
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
 * Determine CPU info using sched.models
 */
extern CpuInfo_t *GetCpuInfo()
{
    static CpuInfo_t		CpuInfo;
    static struct utsname	un;
    char		       *Machine = NULL;
    static char			Line[128];
    char		       *Model;
    FILE		       *fp;
    char		       *cp;
    int				Argc;
    char		      **Argv;
    MCSIquery_t			Query;

    if (CpuInfo.Model)
	return &CpuInfo;

    if (uname(&un) != 0)
	return (CpuInfo_t *) NULL;

    if (!un.machine || !un.machine[0])
	return (CpuInfo_t *) NULL;
	
    if (Model = strrchr(un.machine, '/'))
	++Model;
    else
	Model = un.machine;

    if (!(fp = fopen(_PATH_SCHED_MODELS, "r"))) {
	SImsg(SIM_DBG, "%s: Open for read failed: %s",
	      _PATH_SCHED_MODELS, SYSERR);
	return (CpuInfo_t *) NULL;
    }

    /*
     * The format of the file is:
     *  Model	ArchVersion	CpuType
     */
    while (fgets(Line, sizeof(Line), fp)) {
	if (EQN(Line, "/*", 2))
	    continue;
	if (cp = strchr(Line, '\n'))
	    *cp = CNULL;
	Argc = StrToArgv(Line, " \t", &Argv, NULL, 0);
	if (Argc < 3)
	    continue;
	if (EQ(Argv[0], Model)) {
	    CpuInfo.Model = strdup(Argv[2]);
	    CpuInfo.ArchVer = strdup(Argv[1]);
	    memset(&Query, CNULL, sizeof(Query));
	    Query.Op = MCSIOP_CREATE;
	    Query.Cmd = MCSI_APPARCH;
	    if (mcSysInfo(&Query) == 0)
		CpuInfo.Arch = strdup((char *) Query.Out);
	    break;
	}
    }
    (void) fclose(fp);

    SImsg(SIM_DBG, "GetCpuInfo: Model=<%s> Arch=<%s> ArchVer=<%s>",
	  CpuInfo.Model, CpuInfo.Arch, CpuInfo.ArchVer);

    return &CpuInfo;
}

/*
 * Determine CPU type using GetCpuInfo()
 */
extern char *GetCpuTypeHPUX2()
{
    static char			Type[128];
    CpuInfo_t		       *CpuInfo;

    if (Type[0])
	return Type;

    if (!(CpuInfo = GetCpuInfo()))
	return (char *) NULL;

    (void) snprintf(Type, sizeof(Type), "%s", CpuInfo->Model);
    SImsg(SIM_DBG, "GetCpuTypeHPUX2: Type=<%s>", Type);

    return (Type[0]) ? Type : (char *) NULL;
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
    Status = pstat(PSTAT_DYNAMIC, pstatbuff, sizeof(pst), (size_t)1, 0);
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
    Status = pstat(PSTAT_STATIC, pstatbuff, sizeof(pst), (size_t)1, 0);
    /*    Status = pstat_static(&pst, sizeof(pst), (size_t)1, 0);*/
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
    Cmd.max_msecs = 1 * 1000;	/* max_msecs==milliseconds */
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
	      ScsiCmd->DevFile, CdbPtr->Cmd, SYSERR);
	return(-1);
    }

    if (Cmd.cdb_status != S_GOOD) {
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed with cdb_status=0x%x", 
	      ScsiCmd->DevFile, CdbPtr->Cmd, Cmd.cdb_status);
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
static DevInfo_t *ProbeDiskFile(ProbeData, DevFile)
     ProbeData_t 	       *ProbeData;
     char 		       *DevFile;
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
	case CDROM_DEV_TYPE:	DevType = DT_CD;		break;
	default:
	    if (!DevInfo->ModelDesc) {
		SImsg(SIM_DBG, "ProbeDiskFile(%s): Call DefGet type=%d",
		    DevFile, Describe.dev_type);
		Def = DefGet("DiskDevType", NULL, Describe.dev_type, 0);
		if (Def)
		    DevInfo->ModelDesc = Def->ValStr1;
		SImsg(SIM_DBG, "ProbeDiskFile(%s): Call DefGet done", DevFile);
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

    SImsg(SIM_DBG, "ProbeDiskFile(%s): returning", DevFile);
    return(DevInfo);
}

/*
 * Probe a DiskDrive device.
 */
extern DevInfo_t *ProbeDiskDrive(ProbeData)
     ProbeData_t 	       *ProbeData;
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
extern DevInfo_t *ProbeTapeDrive(ProbeData)
     ProbeData_t 	       *ProbeData;
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
static void SetModel(DevInfo, NewModel)
     DevInfo_t 		       *DevInfo;
     char 		       *NewModel;
{
    register char	       *cp;

    if (!DevInfo || !NewModel)
	return;

    SImsg(SIM_DBG, "SetModel(%s) New=<%s> Old=<%s>",
	  DevInfo->Name, NewModel, PRTS(DevInfo->Model));

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
static DevInfo_t *ProbeFrameBufferFile(ProbeData, DevFile)
     ProbeData_t 	       *ProbeData;
     char 		       *DevFile;
{
    int				fd;
    FrameBuffer_t	       *FrameBuffer = NULL;
    DevInfo_t		       *DevInfo;
    Define_t		       *Define;
    crt_frame_buffer_t		CrtFB;

    if (!ProbeData || !ProbeData->UseDevInfo || !DevFile)
	return((DevInfo_t *) NULL);
    DevInfo = ProbeData->UseDevInfo;

    SImsg(SIM_DBG, "ProbeFrameBufferFile(%s, %s)", 
	  ProbeData->DevName, DevFile);

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
	if (FLAGS_ON(CrtFB.crt_attributes, Define->KeyNum)) {
	    SImsg(SIM_DBG, "ProbeFrameBufferFile(%s, %s): Attr=<%s>", 
		  ProbeData->DevName, DevFile, Define->ValStr1);
	    AddDevDesc(DevInfo, Define->ValStr1, NULL, DA_APPEND);
	}

    DevInfo->DevSpec = FrameBuffer;
#endif	/* GCDESCRIBE */

    (void) close(fd);

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Probe a FrameBuffer device.
 */
extern DevInfo_t *ProbeFrameBuffer(ProbeData)
     ProbeData_t 	       *ProbeData;
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
static DevInfo_t *ProbeAudioFile(ProbeData, DevFile)
     ProbeData_t 	       *ProbeData;
     char 		       *DevFile;
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
static DevInfo_t *ProbeAudio(ProbeData)
     ProbeData_t 	       *ProbeData;
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
 *
 * Interesting Files:
 * /etc/.supported_bits - Match SysModel to 32/64
 * /usr/sam/lib/mo/sched.models - Match Model to PA-RISC info
 */
static DevInfo_t *ProbeCPU(ProbeData)
     ProbeData_t 	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    static char			Buff[256];
    size_t			BuffLen = 0;
    static char			SpdStr[128];
    char		       *cp;
    int				Speed;
    CpuInfo_t		       *CpuInfo;

    if (!ProbeData || !ProbeData->UseDevInfo)
	return((DevInfo_t *) NULL);

    SImsg(SIM_DBG, "ProbeCPU(%s)", ProbeData->DevName);

    DevInfo = ProbeData->UseDevInfo;

    /*
     * This is all stupid stuff that there has to be a better way for!
     */
    Speed = GetCPUSpeed();
    SpdStr[0] = CNULL;
    if (Speed > 0)
	(void) snprintf(SpdStr, sizeof(SpdStr),  "%d MHz", Speed);

    if (SpdStr[0]) {
	SImsg(SIM_DBG, "ProbeCPU(%s): Speed=<%s>",
	      ProbeData->DevName, SpdStr);
	snprintf(Buff, sizeof(Buff), "%s", SpdStr);
	BuffLen = strlen(SpdStr);
    }

    if (CpuInfo = GetCpuInfo()) {
	(void) snprintf(&Buff[BuffLen], sizeof(Buff)-BuffLen, "%s%s %s %s",
			(BuffLen) ? " " : "",
			CpuInfo->Model, CpuInfo->Arch, CpuInfo->ArchVer);
	BuffLen = strlen(Buff);
    }

    DevInfo->Model = strdup(Buff);

    SImsg(SIM_DBG, "ProbeCPU(%s): Model=<%s>",
	  ProbeData->DevName, Buff);

    return(DevInfo);
}

/*
 * Probe and get info about a Keyboard device.
 */
static DevInfo_t *ProbeKeyboard(ProbeData, KbdIdent)
     ProbeData_t 	       *ProbeData;
     int 			KbdIdent;
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
static DevInfo_t *ProbeHILFile(ProbeData)
     ProbeData_t 	       *ProbeData;
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
extern void ProbeHIL(ProbeData)
     ProbeData_t 	       *ProbeData;
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
static char *ConvertMAC(AddrStr, AddrLen)
     char 		       *AddrStr;
     int 			AddrLen;
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
extern void HPUXSetMacInfo(DevInfo, NetIf)
     DevInfo_t 		       *DevInfo;
     NetIF_t 		       *NetIf;
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
 * Build the device tree
 */
extern int BuildDevices(TreePtr, SearchNames)
     DevInfo_t 		      **TreePtr;
     char 		      **SearchNames;
{
    return(BuildIOScan(TreePtr, SearchNames));
}
#endif	/* HAVE_DEVICE_SUPPORT */

/*
 * Build Partition information
 */
#include <mntent.h>
#include <sys/statvfs.h>
extern int BuildPartInfo(PartInfoTree, SearchExp)
     PartInfo_t		      **PartInfoTree;
     char		      **SearchExp;
{
    FILE		       *FilePtr;
    struct mntent	       *Mnt;
    struct statvfs		Statvfs;
    PartInfo_t		       *PartInfo;
    char		      **Argv;
    int				Argc = 0;
    int				i;
    int				Ignore = FALSE;

    FilePtr = setmntent(MNT_MNTTAB, "r");
    if (!FilePtr) {
	SImsg(SIM_GERR, "%s: Open (setmntent) for reading failed: %s", 
	      MNT_MNTTAB, SYSERR);
	return -1;
    }

    SImsg(SIM_DBG, "Size Large_t=%d f_size=%d ull=%d", 
	  sizeof(Large_t), sizeof(unsigned long), sizeof(unsigned long long));

    while (Mnt = getmntent(FilePtr)) {
	SImsg(SIM_DBG, "Getmntent: fsname=<%s> dir=<%s> type=<%s> opts=<%s>",
	      Mnt->mnt_fsname, Mnt->mnt_dir, Mnt->mnt_type, Mnt->mnt_opts);

	if (EQ(Mnt->mnt_type, MNTTYPE_IGNORE) ||
	    EQ(Mnt->mnt_type, MNTTYPE_NFS) ||
	    EQ(Mnt->mnt_type, MNTTYPE_NFS3) ||
	    EQ(Mnt->mnt_type, MNTTYPE_DFS) ||
	    EQ(Mnt->mnt_type, MNTTYPE_LOFS))
	    continue;

	if (Mnt->mnt_opts && !EQ(Mnt->mnt_opts, "-") &&
	    !EQ(Mnt->mnt_opts, "defaults"))
	    Argc = StrToArgv(Mnt->mnt_opts, ",", &Argv, NULL, 0);
	for (i = 0; i < Argc && Ignore == FALSE; ++i) {
	    if (EQ(Argv[i], MNTTYPE_IGNORE))
		Ignore = TRUE;
	}
	if (Ignore)
	    continue;

	PartInfo = PartInfoCreate(NULL);
	PartInfo->DevPath = strdup(Mnt->mnt_fsname);
	PartInfo->MntName = strdup(Mnt->mnt_dir);
	PartInfo->Type = strdup(Mnt->mnt_type);

	if (EQ(Mnt->mnt_type, MNTTYPE_SWAP))
	    PartInfo->Usage = PIU_SWAP;
	else if (EQ(Mnt->mnt_type, MNTTYPE_HFS) ||
		 EQ(Mnt->mnt_type, MNTTYPE_VXFS) ||
		 EQ(Mnt->mnt_type, MNTTYPE_CDFS))
	    PartInfo->Usage = PIU_FILESYS;

	if (Argc > 0)
	    PartInfo->MntOpts = Argv;

	if (statvfs(PartInfo->MntName, &Statvfs) == 0) {
	    SImsg(SIM_DBG, 
		  "Statvfs(%s) f_size=%ld f_frsize=%ld f_size=%ld f_bfree=%ld",
		  PartInfo->MntName, Statvfs.f_size, Statvfs.f_frsize,
		  Statvfs.f_size, Statvfs.f_bfree);

	    PartInfo->Size = (Large_t) ((Large_t)Statvfs.f_size * 
					(Large_t)Statvfs.f_frsize);
	    PartInfo->AmtUsed = (Large_t) 
		(((Large_t)Statvfs.f_size - (Large_t)Statvfs.f_bfree) * 
		 (Large_t)Statvfs.f_frsize);
	} else {
	    SImsg(SIM_GERR, "%s: Statvfs failed: %s", 
		  PartInfo->MntName, SYSERR);
	}

	(void) PartInfoAdd(PartInfoTree, PartInfo);
    }

    endmntent(FilePtr);

    return 0;
}
