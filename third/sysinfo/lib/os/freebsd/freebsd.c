/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

#ifndef lint
static char *RCSid = "$Revision: 1.1.1.1 $";
#endif

/*
 * FreeBSD specific functions
 */

#include "defs.h"
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <machine/console.h>		/* ProbeConsole() */
#include <machine/mouse.h>		/* ProbeMouse() */
#if	OSMVER <= 2
#include <scsi.h>
#endif	/* OSMVER <= 2 */
#if	OSMVER >= 4
#include <cam/scsi/scsi_all.h>
#include <sys/pciio.h>
#include <vm/vm.h>
#else	/* OSMVER < 4 */
#include <sys/device.h>
#include <sys/scsiio.h>
#include <pci/pcivar.h>
#include <pci/pci_ioctl.h>
#endif	/* OSMVER */
#include "myscsi.h"
#include <machine/ioctl_fd.h>		/* ProbeFloppy() */
#include <vm/swap_pager.h>

#if	OSMVER >= 3
#define MyPCIgetVendorID(p)	(p->pc_vendor)
#define MyPCIgetDeviceID(p)	(p->pc_device)
#define MyPCIgetSubDeviceID(p)	(p->pc_subdevice)
#else	/* OSMVER < 3 */
#define MyPCIgetDeviceID(p)	(p->pc_devid >> 16)
#define MyPCIgetVendorID(p)	(p->pc_devid & 0xffff)
#define MyPCIgetSubDeviceID(p)	(p->pc_subid)
#endif	/* OSMVER  */

/*
 * For consistancy
 */
static kvm_t			       *kd;

/*
 * Structure defining information about a CPU
 */
typedef struct {
    char		       *Type;		/* Model: i486DX */
    char		       *Model;		/* Model: Intel Pentium */
    char		       *Class;		/* Class: i586 */
    char		       *Vendor;		/* Vendor: GenuineIntel */
    int				Speed;		/* Speed in MHz */
} CpuInfo_t;

/*
 * Our local Device Definetions
 */
#ifndef MAX_SCSI_UNITS
#define MAX_SCSI_UNITS		7		/* Max # of devs / bus */
#endif
#ifndef MAX_ATA_UNITS
#define MAX_ATA_UNITS		4		/* Max # of devs / bus */
#endif
#ifndef MAX_FD_DEVS
#define MAX_FD_DEVS		2		/* Max # of floppy drives */
#endif

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelUseCpuInfo();
extern char			       *GetModelSysCtl();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelUseCpuInfo },
    { GetModelSysCtl },
    { NULL },
};
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
    { NULL },
};
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
    { NULL },
};
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchUname },
    { NULL },
};
PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetKernArchUname },		/* Not a typo */
    { NULL },
};
extern char			       *GetCpuTypeFreeBSD();
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeFreeBSD },
    { GetModelSysCtl },
    { NULL },
};
extern char			       *GetNcpuSysCtl();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNcpuSysCtl },
    { NULL },
};
extern char			       *GetKernVerSysCtl();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerSysCtl },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameUname },
    { NULL },
};
extern char			       *GetOSDistSysCtl();
PSI_t GetOSDistPSI[] = {		/* Get OS Dist */
    { GetOSDistSysCtl },
    { NULL },
};
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
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
extern char			       *GetMemoryMaxmem();
extern char			       *GetMemorySysCtl();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryMaxmem },
    { GetMemorySysCtl },
    { NULL },
};
#if	defined(HAVE_KVM_GETSWAPINFO)
extern char			       *GetVirtMemKvmGetSwapInfo();
#endif
extern char			       *GetVirtMemNswapSym();
extern char			       *GetVirtMemSysCtl();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
#if	defined(HAVE_KVM_GETSWAPINFO)
    { GetVirtMemKvmGetSwapInfo },
#endif
    { GetVirtMemSysCtl },
    { GetVirtMemNswapSym },
    { NULL },
};
extern char			       *GetBootTimeSysCtl();
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeSysCtl },
    { NULL },
};

/*
 * Get system memory by looking for "Maxmem" kernel symbol.
 * This contains the accurate amount whereas "physmem" and the
 * sysctl(HW_PHYSMEM) both return < than the actual amount.
 */
extern char *GetMemoryMaxmem()
{
    struct nlist	       *nlPtr;
    int				Maxmem;
    static char			MaxmemSYM[] = "_Maxmem";
    Large_t			Amount;

    if (!kd)
	if (!(kd = KVMopen()))
	    return((char *) NULL);

    if ((nlPtr = KVMnlist(kd, MaxmemSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);

    if (CheckNlist(nlPtr))
	return((char *) NULL);	

    if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &Maxmem, 
	       sizeof(Maxmem), KDT_DATA) != 0) {
	SImsg(SIM_GERR, "Cannot read Maxmem from kernel: %s", SYSERR);
	return((char *) NULL);
    }

    Amount = DivRndUp((Large_t)ptoa(Maxmem), (Large_t)MBYTES);

    SImsg(SIM_DBG, "GetMemoryMaxmem: Maxmem=%d", ptoa(Maxmem));

    return(GetMemoryStr(Amount));
}

/*
 * Fetch information about the system CPU.
 */
static CpuInfo_t *FetchCpuInfo()
{
    static CpuInfo_t		CpuInfo;
    kvm_t		       *kd;
    nlist_t		       *nlPtr;
    static int			CpuType;
    static char			CpuTypeSYM[] = "_cpu";
    static int			CpuClass;
    static char			CpuClassSYM[] = "_cpu_class";
    static char			CpuVendor[128];
    static char			CpuVendorSYM[] = "_cpu_vendor";
    static char			CpuModel[128];
    static char			CpuModelSYM[] = "_cpu_model";
    static u_long		CpuId;
    static char			CpuIdSYM[] = "_cpu_id";
    static u_long		CpuHigh;
    static char			CpuHighSYM[] = "_cpu_high";
    static u_long		CpuFeature;
    static char			CpuFeatureSYM[] = "_cpu_feature";
    static char			Query[128];
    long			CpuFreq;
    size_t			CpuFreqLen = sizeof(CpuFreq);
    Define_t		       *Define;

    if (CpuInfo.Model)
	return(&CpuInfo);

    if (!(kd = KVMopen()))
	return((CpuInfo_t *) NULL);

    /*
     * Get CPU Type
     */
    if ((nlPtr = KVMnlist(kd, CpuTypeSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuType, 
		   sizeof(CpuType), KDT_DATA) == 0) {
	    Define = DefGet("CPUtype", (char *) NULL, CpuType, 0);
	    if (Define)
		CpuInfo.Type = Define->ValStr1;
	    else
		SImsg(SIM_UNKN, "CPU type %d not found in config/*.cf files.", 
		      CpuType);
	} else
	    SImsg(SIM_GERR, "Cannot read CPU type from kernel.");
    }

    /*
     * Get CPU Class
     */
    if ((nlPtr = KVMnlist(kd, CpuClassSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuClass, 
		   sizeof(CpuClass), KDT_DATA) == 0) {
	    Define = DefGet("CPUclass", (char *) NULL, CpuClass, 0);
	    if (Define)
		CpuInfo.Class = Define->ValStr1;
	    else
		SImsg(SIM_UNKN, "CPU Class %d not found in config/*.cf files.",
		      CpuClass);
	} else
	    SImsg(SIM_GERR, "Cannot read CPU Class from kernel.");
    }

    /*
     * Get CPU Vendor
     */
    if ((nlPtr = KVMnlist(kd, CpuVendorSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuVendor, 
		   sizeof(CpuVendor), KDT_STRING) == 0) {
	    CpuInfo.Vendor = CpuVendor;
	} else
	    SImsg(SIM_GERR, "Cannot read CPU Vendor from kernel.");
    }

    /*
     * Get CPU Model
     */
    if ((nlPtr = KVMnlist(kd, CpuModelSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuModel, 
		   sizeof(CpuModel), KDT_STRING) == 0) {
	    CpuInfo.Model = CpuModel;
	} else
	    SImsg(SIM_GERR, "Cannot read CPU Model from kernel.");
    }

    /*
     * Get cpu_high 
     */
    if ((nlPtr = KVMnlist(kd, CpuHighSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuHigh, 
		   sizeof(CpuHigh), KDT_DATA) == 0) {
	    /* No problem */
	} else
	    SImsg(SIM_GERR, "Cannot read cpu_high  from kernel.");
    }

    /*
     * Get cpu_id 
     */
    if ((nlPtr = KVMnlist(kd, CpuIdSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuId, 
		   sizeof(CpuId), KDT_DATA) == 0) {
	    /* No problem */
	} else
	    SImsg(SIM_GERR, "Cannot read cpu_id  from kernel.");
    }

    /*
     * Get cpu_feature 
     */
    if ((nlPtr = KVMnlist(kd, CpuFeatureSYM, (struct nlist *)NULL, 0)) &&
	(CheckNlist(nlPtr) == 0)) {
	if (KVMget(kd, (KVMaddr_t) nlPtr->n_value, (void *) &CpuFeature, 
		   sizeof(CpuFeature), KDT_DATA) == 0) {
	    /* Do nothing for now */
	} else
	    SImsg(SIM_GERR, "Cannot read cpu_feature from kernel.");
    }

    KVMclose(kd);

    /*
     * Look up the Clock Frequency for this class of CPU.
     * In FreeBSD 2.x look for "machdep.CLASS_freq" (i.e. machdep.i586_freq).
     * In FreeBSD 3.0 and later it's "machdep.tsc_freq".
     */
    CpuFreq = 0;
    (void) snprintf(Query, sizeof(Query), "machdep.%s_freq", CpuInfo.Class);
    if (sysctlbyname(Query, &CpuFreq, &CpuFreqLen, NULL, 0) < 0) {
	CpuFreq = 0;
	SImsg(SIM_GERR, "sysctl lookup <%s> failed: %s", Query, SYSERR);
	/*
	 * The Class specific entry wasn't found, so try the new "tsc"
	 * entry as found on FreeBSD 3.0 and later.
	 */
	(void) snprintf(Query, sizeof(Query), "machdep.tsc_freq");
	if (sysctlbyname(Query, &CpuFreq, &CpuFreqLen, NULL, 0) < 0) {
	    CpuFreq = 0;
	    SImsg(SIM_GERR, "sysctl lookup <%s> failed: %s", Query, SYSERR);
	}
    }
    if (CpuFreq)
	CpuInfo.Speed = CpuFreq / MHERTZ;

    SImsg(SIM_DBG, "CpuInfo: Type=%d(%s) Class=%d(%s) Vendor=<%s> Model=<%s>",
	  CpuType, CpuInfo.Type, CpuClass, CpuInfo.Class, CpuVendor, CpuModel);
    SImsg(SIM_DBG, "CpuInfo: Speed=%d Id=0x%lx High=0x%lx Features=0x%lx",
	  CpuInfo.Speed, CpuId, CpuHigh, CpuFeature);

    return(&CpuInfo);
}

/*
 * Use Cpu Information to build system model name.
 * See <machine/cpu.h> and <machine/cputypes.h>
 */
extern char *GetModelUseCpuInfo()
{
    static char			Model[128];
    CpuInfo_t		       *CpuInfo;

    if (!(CpuInfo = FetchCpuInfo()))
	return((char *) NULL);

    (void) snprintf(Model, sizeof(Model), "%s %s",
		    CpuInfo->Vendor, CpuInfo->Model);
    if (CpuInfo->Speed)
	(void) snprintf(&Model[strlen(Model)], sizeof(Model), " %d-MHz",
			CpuInfo->Speed);

    return(Model);
}

/*
 * FreeBSD method for getting CPU Type.
 * See <machine/cpu.h> and <machine/cputypes.h>
 */
extern char *GetCpuTypeFreeBSD()
{
    CpuInfo_t		       *CpuInfo;
    static char			Type[256];

    if (Type[0])
	return(Type);

    if (!(CpuInfo = FetchCpuInfo()))
	return((char *) NULL);

    (void) snprintf(Type, sizeof(Type), "%s %s %s",
		    CpuInfo->Class, CpuInfo->Vendor, CpuInfo->Model);

    return(Type);
}

/*
 * Use the "nswap" symbol to determine amount of
 * virtual memory.
 */
#if	!defined(NSWAP_SYM)
#	define NSWAP_SYM	"_nswap"
#endif	/* NSWAP_SYM */

extern char *GetVirtMemNswapSym()
{
    kvm_t		       *kd;
    int				Nswap;
    Large_t			Amount = 0;
    nlist_t		       *nlPtr;

    if (kd = KVMopen()) {
	if ((nlPtr = KVMnlist(kd, NSWAP_SYM, (nlist_t *)NULL, 0)) == NULL)
	    return(0);

	if (CheckNlist(nlPtr))
	    return(0);

	if (KVMget(kd, nlPtr->n_value, (char *) &Nswap,
		   sizeof(Nswap), KDT_DATA) == 0) {
	    Amount = (Large_t)Nswap / (Large_t)NSWAP_SIZE;

	    SImsg(SIM_DBG, "GetVirtMemNswap: Amount=%.0f Nswap=%d",
		  (float) Amount, Nswap);
	}

	KVMclose(kd);
    }

    if (Amount)
	return(GetVirtMemStr(Amount));
    else
	return((char *) NULL);
}

/*
 * Use kvm_getswapinfo() to get swap info
 */
#if	defined(HAVE_KVM_GETSWAPINFO)
#include <kvm.h>
extern char *GetVirtMemKvmGetSwapInfo()
{
    struct kvm_swap		Swap[16];
    int				Num;
    register int		i;
    kvm_t		       *kd;
    Large_t			PageCount = 0;
    Large_t			PageSize = 0;
    int				Amount = 0;

    kd = KVMopen();
    if (!kd) {
	return (char *) NULL;
    }

    Num = kvm_getswapinfo(kd, Swap, sizeof(Swap)/sizeof(Swap[0]), 0);
    if (Num < 0) {
	KVMclose(kd);
	return (char *) NULL;
    }

    for (i = 0; i < Num; ++i)
	PageCount += (Large_t) Swap[i].ksw_total;

    KVMclose(kd);

    PageSize = (Large_t) getpagesize();
    Amount = (int) ((PageCount * PageSize) / (Large_t)KBYTES);

    SImsg(SIM_DBG, "GetVirtMemKvmGetSwapInfo: Amount=%d PageCount=%d Size=%d",
	  Amount, PageCount, PageSize);

    return GetVirtMemStr((Large_t)Amount);
}
#endif	/* HAVE_KVM_GETSWAPINFO */

/*
 * Probe a System Console (syscons, sc)
 */
extern DevInfo_t *ProbeConsole(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    Define_t		       *Define;
    static char		        DefConsole[] = "/dev/console";
    char		       *DevFile = DefConsole;
    int				fd;
    int				IntVal = 0;
    static vid_info_t		VirtInfo;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevInfo = DeviceCreate(ProbeData);

    fd = open(DevFile, O_RDWR|O_NONBLOCK|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Cannot open RDWR: %s", DevFile, SYSERR);
	return(DevInfo);
    }
    DevAddFile(DevInfo, DevFile);

    /*
     * Figure out what type of console this is
     */
    IntVal = 0;
    if (ioctl(fd, CONS_CURRENT, &IntVal) == 0) {
	if (Define = DefGet("ConsoleTypes", NULL, IntVal, 0))
	    DevInfo->Model = Define->ValStr1;
	else
	    SImsg(SIM_DBG, "%s: Could not find %d in `ConsoleTypes'.",
		  DevFile, IntVal);
    } else {
	SImsg(SIM_GERR, "%s: ioctl CONS_CURRENT failed: %s", DevFile, SYSERR);
    }

    /*
     * Get Virtual Console info
     */
    (void) memset(&VirtInfo, 0, sizeof(VirtInfo));
    if (ioctl(fd, CONS_GETINFO, &VirtInfo) == 0) {
	if (VirtInfo.size)
	    AddDevDesc(DevInfo, atoi(VirtInfo.size), "Virtual Console Size",
		       DA_APPEND);
	if (VirtInfo.m_num)
	    AddDevDesc(DevInfo, atoi(VirtInfo.m_num), "Num Virtual Consoles",
		       DA_APPEND);
    } else {
	SImsg(SIM_GERR, "%s: ioctl CONS_GETINFO failed: %s", DevFile, SYSERR);
    }
	
    /*
     * Console Version
     */
    IntVal = 0;
    if (ioctl(fd, CONS_GETVERS, &IntVal) == 0) {
	AddDevDesc(DevInfo, itoax(IntVal), "Version", DA_APPEND);
    } else {
	SImsg(SIM_GERR, "%s: ioctl CONS_GETVERS failed: %s", DevFile, SYSERR);
    }

    (void) close(fd);

    return(DevInfo);
}

/*
 * Probe a System Mouse (syscons, sc)
 */
extern DevInfo_t *ProbeMouse(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    Define_t		       *Define;
    static char		        DefMouse[] = "/dev/sysmouse";
    char			DevFile[128];
    char		       *File = NULL;
    int				fd;
    int				Len;
    static char			IfType[64];
    static char			Type[64];
    static char			ModelSpec[256];
    static mousehw_t		Mouse;
    static mousemode_t		Mode;
    register char	       *cp;

    /*
     * If ProbeData is NULL, use the default mouse device
     */
    if (!ProbeData) {
	DevInfo = (DevInfo_t *) NewDevInfo(NULL);
	File = DefMouse;
	if (cp = strrchr(File, '/'))
	    DevInfo->Name = cp + 1;
	DevInfo->Type = DT_POINTER;
    } else {
	DevInfo = DeviceCreate(ProbeData);
	(void) snprintf(DevFile, sizeof(DevFile), "%s/%s", 
			_PATH_DEV, DevInfo->Name);
	File = DevFile;
    }

    fd = open(File, O_RDONLY|O_NONBLOCK|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Cannot open for reading: %s", File, SYSERR);
	return((DevInfo_t *) NULL);
    }
    DevAddFile(DevInfo, File);

    /*
     * Get general mouse hardware info
     */
    (void) memset(&Mouse, 0, sizeof(Mouse));
    if (ioctl(fd, MOUSE_GETHWINFO, &Mouse) == 0) {
#ifdef	NOTYET	/* buttons doesn't appear to contain valid data */
	if (Mouse.buttons > 0)
	    AddDevDesc(DevInfo, itoa(Mouse.buttons), "Number of Buttons",
		       DA_APPEND);
#endif	/* NOTYET */
	if (Define = DefGet("MouseModels", NULL, Mouse.model, 0))
	    DevInfo->Model = strdup(Define->ValStr1);
	else
	    SImsg(SIM_DBG, "%s: Could not find %d in `MouseModels'.",
		  File, Mouse.model);

	IfType[0] = CNULL;
	Type[0] = CNULL;
	if (Define = DefGet("MouseIfTypes", NULL, Mouse.iftype, 0))
	    (void) snprintf(IfType, sizeof(IfType), "%s", Define->ValStr1);
	else
	    SImsg(SIM_DBG, "%s: Could not find %d in `MouseIfTypes'.",
		  File, Mouse.iftype);
	if (Define = DefGet("MouseTypes", NULL, Mouse.type, 0))
	    (void) snprintf(Type, sizeof(Type), "%s", Define->ValStr1);
	else
	    SImsg(SIM_DBG, "%s: Could not find %d in `MouseTypes'.",
		  File, Mouse.type);

	if (IfType[0])
	    (void) strncpy(ModelSpec, IfType, sizeof(IfType));
	if (Type[0]) {
	    Len = strlen(ModelSpec);
	    (void) snprintf(&ModelSpec[Len], sizeof(ModelSpec)-Len, "%s%s",
			    (Len) ? " " : "", Type);
	}
	DevInfo->ModelDesc = strdup(ModelSpec);
    } else {
	SImsg(SIM_GERR, "%s: ioctl MOUSE_GETHWINFO failed: %s", 
	      File, SYSERR);
    }

    /*
     * Get mouse mode info
     */
    (void) memset(&Mode, 0, sizeof(Mode));
    if (ioctl(fd, MOUSE_GETMODE, &Mode) == 0) {
	if (Define = DefGet("MouseProtocols", NULL, Mode.protocol, 0))
	    AddDevDesc(DevInfo, Define->ValStr1, "Protocol", DA_APPEND);
	else
	    SImsg(SIM_DBG, "%s: Could not find %d in `MouseProtocols'.",
		  File, Mode.protocol);

	if (Mode.rate > 0)
	    AddDevDesc(DevInfo, itoa(Mode.rate), "Report Rate", DA_APPEND);
	if (Mode.resolution > 0)
	    AddDevDesc(DevInfo, itoa(Mode.resolution), "Resolution", 
		       DA_APPEND);
	if (Mode.accelfactor > 0)
	    AddDevDesc(DevInfo, itoa(Mode.accelfactor), 
		       "Acceleration Factor", DA_APPEND);
	if (Mode.level > 0)
	    AddDevDesc(DevInfo, itoax(Mode.level), "Driver Version", 
		       DA_APPEND);
	if (Mode.packetsize > 0)
	    AddDevDesc(DevInfo, itoa(Mode.packetsize), 
		       "Packet Size", DA_APPEND);
    } else {
	SImsg(SIM_GERR, "%s: ioctl MOUSE_GETMODE failed: %s", 
	      File, SYSERR);
    }

    (void) close(fd);

    return(DevInfo);
}

/*
 * Probe a Floppy Disk (fd)
 */
extern DevInfo_t *ProbeFloppy(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
    char		       *File;
    static struct fd_type       Type;
    static char			DevFile[128];
    DiskDrive_t		       *Disk;
    DiskDriveData_t	       *DiskDriveData;
    int				fd;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevInfo = DeviceCreate(ProbeData);
    File = ProbeData->DevFile;
    if (!File) {
	if (DevInfo->Name) {
	    (void) snprintf(DevFile, sizeof(DevFile), "%s/r%s", 
			    _PATH_DEV, DevInfo->Name);
	    File = DevFile;
	}
    }

    /*
     * Make sure we have a file descriptor.
     * Usually it's passed to us in ProbeData.
     */
    if (ProbeData->FileDesc)
	fd = ProbeData->FileDesc;
    else {
	if (!File) {
	    SImsg(SIM_DBG, "%s: Cannot determine filename - skipping query.",
		  DevInfo->Name);
	    return(DevInfo);
	}
	fd = open(File, O_RDONLY|O_NDELAY|O_NONBLOCK);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: open for read failed: %s", File, SYSERR);
	    return(DevInfo);
	}
    }

    if (ioctl(fd, FD_GTYPE, &Type) == 0) {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
	Disk = NewDiskDrive(NULL);
	DiskDriveData->HWdata = Disk;

	Disk->Sect = Type.sectrac;
	Disk->SecSize = Type.secsize;
	Disk->DataCyl = Type.heads;
	Disk->Tracks = Type.tracks;
	Disk->SectGap = Type.gap;
	if (Type.size) {
	    Disk->Size = (float) ((float)Type.size / 
				  (float)((Type.secsize) ? Type.secsize : 1));
	    if (Disk->Size)
		/* Use 1000 = 1 kb */
		Disk->Size /= 1000;
	}
    } else {
	SImsg(SIM_GERR, "%s: ioctl FD_GTYPE failed: %s", File, SYSERR);
    }

    /*
     * Only close fd if we opened it
     */
    if (!ProbeData->FileDesc)
	(void) close(fd);

    return(DevInfo);
}

/* 
 * The fdc_data type disappeared in FreeBSD 4.0
 */
#if	OSMVER <= 3	
/*
 * Get the fdc_data for fdc controller unit == CtlrUnit
 */
static struct fdc_data *GetFdcData(CtlrUnit)
     int			CtlrUnit;
{
    struct nlist	       *nlPtr;
    char			FdcDataSYM[] = "_fdc_data";
    static struct fdc_data	FdcData;
    KVMaddr_t			Addr;

    if (CtlrUnit < 0)
	return((struct fdc_data *) NULL);

    if (!kd)
	if (!(kd = KVMopen()))
	    return((struct fdc_data *) NULL);

    if ((nlPtr = KVMnlist(kd, FdcDataSYM, (struct nlist *)NULL, 0)) == NULL)
	return((struct fdc_data *) NULL);

    if (CheckNlist(nlPtr))
	return((struct fdc_data *) NULL);

    /*
     * Since fdc_data is a table, we use the offset for this CtlrUnit
     */
    Addr = nlPtr->n_value + (sizeof(struct fdc_data) * CtlrUnit);
    if (KVMget(kd, Addr, (void *) &FdcData,
	       sizeof(FdcData), KDT_DATA) != 0) {
	SImsg(SIM_GERR, "Cannot read fdc_data[%d] from kernel: %s", 
	      CtlrUnit, SYSERR);
	return((struct fdc_data *) NULL);
    }

    return(&FdcData);
}
#endif	/* OSMVER <= 3 */

/*
 * Probe a Floppy Controller (fdc)
 */
extern DevInfo_t *ProbeFloppyCtlr(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo = NULL;
#if	OSMVER <= 3
    DevInfo_t		       *ChildDevInfo;
    DevInfo_t		       *Last = NULL;
    Define_t		       *Define;
    static ProbeData_t		ChildProbe;
    static DevData_t		ChildDevData;
    static char			DevName[16];
    static char			DevFile[128];
    char		       *File = NULL;
    int				fd;
    register int		i;
    struct fdc_data	       *FdcData;

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevInfo = DeviceCreate(ProbeData);

    /*
     * Get data about this controller
     */
    if (FdcData = GetFdcData(DevInfo->Unit)) {
	if (FdcData->fdct >= 0) {
	    if (Define = DefGet("FloppyCtlrTypes", NULL, FdcData->fdct, 0))
		DevInfo->Model = Define->ValStr1;
	    else
		SImsg(SIM_DBG, 
		      "%s: FDC Type %d is not defined in `FloppyCtlrTypes'",
		      DevInfo->Name, FdcData->fdct);
	}
	if (FdcData->dmachan > 0)
	    AddDevDesc(DevInfo, itoa(FdcData->dmachan), 
		       "DMA Channels", DA_APPEND);
	/* FdcData->baseport == IsaDev->iobase */
    }

    /*
     * Now look for our children
     */
    for (i = 0; i < MAX_FD_DEVS; ++i) {
	(void) snprintf(DevName, sizeof(DevName), "fd%d", i);
	(void) snprintf(DevFile, sizeof(DevFile), "%s/r%s", 
			_PATH_DEV, DevName);
	File = DevFile;
	fd = open(File, O_RDONLY|O_NONBLOCK|O_NDELAY);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s", File, SYSERR);
	    continue;
	}
	(void) memset(&ChildProbe, 0, sizeof(ChildProbe));
	(void) memset(&ChildDevData, 0, sizeof(ChildDevData));
	ChildDevData.DevName = "fd";
	ChildDevData.DevUnit = i;
	ChildDevData.DevType = DT_FLOPPY;
	ChildDevData.CtlrName = DevInfo->Driver;
	ChildDevData.CtlrUnit = DevInfo->Unit;
	ChildProbe.DevData = &ChildDevData;
	ChildProbe.DevName = DevName;
	ChildProbe.CtlrDevInfo = DevInfo;
	ChildProbe.DevFile = File;
	ChildProbe.FileDesc = fd;

	if (ChildDevInfo = ProbeFloppy(&ChildProbe)) {
	    /* Add the new Child to our list of Slaves */
	    if (!DevInfo->Slaves)
		DevInfo->Slaves = Last = ChildDevInfo;
	    else {
		Last->Next = ChildDevInfo;
		Last = ChildDevInfo;
	    }
	}

	(void) close(fd);
    }
#endif	/* OSMVER <= 3 */
    return(DevInfo);
}

/*
 * Create a printable name for a PCI device with no driver info.
 */
static char *PCIname(PciConf, Flags)
     struct pci_conf	       *PciConf;
     int			Flags;
{
    static char			Name[PCI_DEV_NAME_MAX];

    if (FLAGS_ON(Flags, 0x1)) {
	(void) snprintf(Name, sizeof(Name), "pci%d:%d:",
			PciConf->pc_sel.pc_bus, PciConf->pc_sel.pc_dev);
    } else {
	(void) snprintf(Name, sizeof(Name), "pci%d:%d:%d",
			PciConf->pc_sel.pc_bus, PciConf->pc_sel.pc_dev, 
			PciConf->pc_sel.pc_func);
    }

    return(Name);
}

/*
 * Get the device name for a PCI device
 */
static char *PCIgetDevName(PciConf, TreePtr, SearchNames)
     struct pci_conf	       *PciConf;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{   
    kvm_t		       *kd;
#if	OSMVER < 3
    static struct pci_device	PciDev;
#endif	/* OSMVER < 3 */
    static char			DriverName[PCI_DEV_NAME_MAX];
    static char			DevName[PCI_DEV_NAME_MAX];
    char		       *Name = NULL;

    DevName[0] = CNULL;

#if	OSMVER >= 3
    return(PciConf->pd_name);
#else	/* OSMVER < 3 */
    if (PciConf->pc_dvp) {
	if (!(kd = KVMopen()))
	    return((char *) NULL);

	if (KVMget(kd, (KVMaddr_t) PciConf->pc_dvp, (void *) &PciDev, 
		   sizeof(PciDev), KDT_DATA)) {
	    SImsg(SIM_GERR, "%s: Cannot read device driver from kernel.",
		  PCIname(PciConf, 0));
	    return((char *) NULL);
	}
	if (PciDev.pd_name) {
	    if (KVMget(kd, (KVMaddr_t) PciDev.pd_name, (void *) DriverName, 
		       sizeof(DriverName), KDT_STRING)) {
		SImsg(SIM_GERR, "%s: Cannot read device name from kernel.",
		      PCIname(PciConf, 0));
		return((char *) NULL);
	    }
	    SImsg(SIM_DBG, "PCI: %s    Name=<%s>", PCIname(PciConf, 0), 
		  DriverName);
	    return(DriverName);
	}
	KVMclose(kd);
    }

    if (DevName[0]) {
	return(DevName);
    } else {
	Name = PCIname(PciConf, 0x1);
	SImsg(SIM_DBG, "%s: No Driver Name was found.  Using <%s>",
	      Name, Name);
	return(Name);
    }
#endif	/* OSMVER > 3 */
}

/*
 * Set what PCI Device info we can by looking things up in pci.cf
 */
static void PCIsetInfo(DevData, PciConf)
     DevData_t		       *DevData;
     struct pci_conf	       *PciConf;
{
    static PCIinfo_t		Info;

    if (!DevData || !PciConf)
	return;

    PCInewInfo(&Info);
    Info.VendorID = MyPCIgetVendorID(PciConf);
    Info.DeviceID = MyPCIgetDeviceID(PciConf);
    Info.SubDeviceID = MyPCIgetSubDeviceID(PciConf);
    Info.Class = PciConf->pc_class >> 8;
    Info.Revision = PciConf->pc_class & 0xff;
    Info.Header = PciConf->pc_hdr;
    Info.Bus = PciConf->pc_sel.pc_bus;
    Info.Unit = PciConf->pc_sel.pc_dev;
    Info.SubUnit = PciConf->pc_sel.pc_func;
    Info.DevInfo = NewDevInfo(NULL);

    PCIsetDeviceInfo(&Info);

    DevData->OSDevInfo = Info.DevInfo;
}

/*
 * Create a PCI bus (top level) device if needed.
 */
static void PCIcreatePCI(DevData, TreePtr, SearchNames)
     DevData_t		       *DevData;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    char		       *DevName;
    DevInfo_t		       *DevInfo;
    DevDefine_t		       *DevDef;
    static DevFind_t		Find;

    DevName = MkDevName(DevData->CtlrName, DevData->CtlrUnit, 0, 0);
    (void) memset(&Find, 0, sizeof(Find));
    Find.Tree = *TreePtr;
    Find.NodeName = DevName;
    if (DevFind(&Find))
	/* Already exists */
	return;

    DevInfo = NewDevInfo(NULL);
    DevInfo->Name = DevName;
    DevInfo->Unit = DevData->CtlrUnit;

    DevDef = DevDefGet(DevData->CtlrName, 0, 0);
    if (DevDef) {
	DevInfo->Type = DevDef->Type;
	DevInfo->ClassType = CT_PCI;
	DevInfo->Model = DevDef->Model;
	DevInfo->ModelDesc = DevDef->Desc;;
    } else {
	DevInfo->Type = DT_BUS;
	DevInfo->ClassType = CT_PCI;
	DevInfo->Model = "PCI";
	DevInfo->ModelDesc = "System Bus";
    }

    AddDevice(DevInfo, TreePtr, SearchNames);
}

#if	HAVE_LIBCAM
/*
 * Issue a SCSI command and return the results.
 *
 * FreeBSD 3.x ScsiCmd using camlib.
 */
#include <camlib.h>
#include <cam/scsi/scsi_message.h>
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
    static char			Buff[SCSI_BUF_LEN];
    struct cam_device	       *CamDevice;
    union ccb		       *CamCcb;
    extern char		        cam_errbuf[];

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    if (!(CamDevice = cam_open_device(ScsiCmd->DevFile, O_RDWR))) {
	SImsg(SIM_GERR, "%s: cam_open_spec_device failed: %s",
	      ScsiCmd->DevFile, cam_errbuf);
	return(-1);
    }

    if (!(CamCcb = cam_getccb(CamDevice))) {
	SImsg(SIM_GERR, "%s: cam_getccb failed: %s",
	      ScsiCmd->DevFile, cam_errbuf);
	(void) cam_close_device(CamDevice);
	return(-1);
    }

    /*
     * Initialize
     */
    (void) memset(&(&CamCcb->ccb_h)[1], 0, sizeof(struct ccb_scsiio));
    (void) memset(Buff, 0, sizeof(Buff));

    cam_fill_csio(&CamCcb->csio,
		  1,	    			/* retries */ 
		  NULL,    			/* cbfcnp */ 
		  CAM_DIR_IN,   		/* flags */ 
		  MSG_SIMPLE_Q_TAG,    		/* tag_action */ 
		  (u_int8_t *) Buff,    	/* buffer */ 
		  sizeof (Buff),    		/* buffer len */ 
		  SSD_FULL_SIZE,    		/* sense_len */ 
		  ScsiCmd->CdbLen,		/* cdb_len */
		  MySCSI_CMD_TIMEOUT * 1000);   /* timeout */ 
    /*
     * Setup CAM CCB using our CDB
     */
    (void) memcpy(&CamCcb->csio.cdb_io.cdb_bytes, ScsiCmd->Cdb, 
		  sizeof(CamCcb->csio.cdb_io.cdb_bytes));
    /* 
     * Disable freezing the device queue 
     */
    CamCcb->ccb_h.flags |= CAM_DEV_QFRZDIS;

    if (cam_send_ccb(CamDevice, CamCcb) < 0) {
	SImsg(SIM_GERR, "%s: cam_send_ccb failed: %s",
	      ScsiCmd->DevFile, cam_errbuf);
	(void) cam_close_device(CamDevice);
	(void) cam_freeccb(CamCcb);
	return(-1);
    }

    (void) cam_close_device(CamDevice);
    (void) cam_freeccb(CamCcb);

    ScsiCmd->Data = (void *) Buff;

    return(0);
}
#else	/* !HAVE_LIBCAM */
/*
 * Issue a SCSI command and return the results.
 *
 * FreeBSD 2.x ScsiCmd is a straight SCIOCCOMMAND
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
    static char			Buff[SCSI_BUF_LEN];
    ScsiCdbG0_t		       *CdbPtr = NULL;
    static scsireq_t		Req;
    scsireq_t		       *ReqPtr;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Req, 0, sizeof(Req));

    (void) memcpy(Req.cmd, ScsiCmd->Cdb, ScsiCmd->CdbLen);
    Req.cmdlen = ScsiCmd->CdbLen;
    Req.databuf = (caddr_t) Buff;
    Req.datalen = sizeof(Buff);
    Req.timeout = MySCSI_CMD_TIMEOUT * 1000;	/* timeout == milliseconds */
    Req.flags = SCCMD_READ;

    /* 
     * We just need Cdb.cmd so it's ok to assume ScsiCdbG0_t here
     */
    CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;
    errno = 0;
    if (ioctl(ScsiCmd->DevFD, SCIOCCOMMAND, (void *) &Req) == -1) {
	SImsg(SIM_GERR, "%s: ioctl SCIOCCOMMAND SCSI cmd 0x%x failed: %s",
	      ScsiCmd->DevFile, CdbPtr->cmd, SYSERR);
	return(-1);
    }

#if	!defined(SCSIREQ_ERROR)
#define SCSIREQ_ERROR(r)	(r->retsts != SCCMD_OK)
#endif

    ReqPtr = &Req;
    if (SCSIREQ_ERROR(ReqPtr)) {
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed.", 
	      ScsiCmd->DevFile, CdbPtr->cmd);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
    return(0);
}
#endif	/* HAVE_LIBCAM */

/*
 * Query a SCSI device with base (driver) named Name and Unit.
 * 
 * We used to use ioctl SCIOCIDENTIFY to get SCSI Addr info, but that
 * doesn't work with the da/sa drivers in FreeBSD 3.0
 */
static DevList_t *ScsiProbe(Name, Unit, DevType)
     char		       *Name;
     int			Unit;
     int			DevType;
{
    DevInfo_t		       *DevInfo = NULL;
    DevList_t		       *DevList;
    static char			DevName[64];
    static char			File[128];
    int				fd;

    (void) snprintf(DevName, sizeof(DevName), "%s%d", Name, Unit);
#if	OSMVER >= 3
    (void) snprintf(File, sizeof(File), "%s/r%s", _PATH_DEV, DevName);
#else	/* OSMVER < 3 */
    (void) snprintf(File, sizeof(File), "%s/r%s.ctl", _PATH_DEV, DevName);
#endif

    fd = open(File, O_RDONLY|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_DBG, "%s: open %s failed: %s", DevName, File, SYSERR);
	if (errno != EBUSY)
	    return((DevList_t *) NULL);
    } else {
	(void) close(fd);
    }

    SImsg(SIM_DBG, "%s:\tSCSI Probe: Device Exists", File);

    /*
     * Create the DevInfo
     */
    DevInfo = NewDevInfo(NULL);
    DevInfo->Name = strdup(DevName);
    DevInfo->Driver = strdup(Name);
    DevInfo->ClassType = CT_SCSI;
    DevInfo->Unit = Unit;
    DevInfo->Type = DevType;
    DevAddFile(DevInfo, strdup(File));

    /*
     * Crate DevList and return it.
     */
    DevList = (DevList_t *) xcalloc(1, sizeof(DevList_t));
    DevList->Name = DevInfo->Name;
    DevList->File = DevInfo->Files[0];
    DevList->Unit = Unit;
    DevList->DevInfo = DevInfo;

    return(DevList);
}

/*
 * Get and return a list of all SCSI devices we can find.
 * We only build the list once to save time.
 */
static DevList_t *ScsiGetAll()
{
    register int		u;
    static DevList_t	       *ScsiDevices;
    register DevList_t	       *New = NULL;
    register DevList_t	       *Last = NULL;

    if (ScsiDevices)
	return(ScsiDevices);

    /*
     * Look for SCSI Disks
     */
#if	OSMVER >= 3
#define MySCSI_DISK_NAME "da"
#else
#define MySCSI_DISK_NAME "sd"
#endif	/* OSMVER */
    for (u = 0; u < MAX_SCSI_UNITS; ++u) {
	if (New = ScsiProbe(MySCSI_DISK_NAME, u, DT_DISKDRIVE)) {
	    if (!ScsiDevices)
		ScsiDevices = Last = New;
	    else {
		Last->Next = New;
		Last = New;
	    }
	}
    }

    /*
     * Look for SCSI Tapes
     */
#if	OSMVER >= 3
#define MySCSI_TAPE_NAME "sa"
#else
#define MySCSI_TAPE_NAME "st"
#endif	/* OSMVER */
    for (u = 0; u < MAX_SCSI_UNITS; ++u) {
	if (New = ScsiProbe(MySCSI_TAPE_NAME, u, DT_TAPEDRIVE)) {
	    if (!ScsiDevices)
		ScsiDevices = Last = New;
	    else {
		Last->Next = New;
		Last = New;
	    }
	}
    }

    return(ScsiDevices);
}

/*
 * Query an ATA/IDE/EIDE device with base (driver) named Name and Unit.
 */
static DevList_t *AtaProbe(Name, Unit, DevType)
     char		       *Name;
     int			Unit;
     int			DevType;
{
    DevInfo_t		       *DevInfo = NULL;
    DevList_t		       *DevList;
    static char			DevName[64];
    static char			File[128];
    int				fd;

    (void) snprintf(DevName, sizeof(DevName), "%s%d", Name, Unit);
    (void) snprintf(File, sizeof(File), "%s/r%s", _PATH_DEV, DevName);

    /*
     * All we can do right now is see if open() succeeds.
     * We really need a means to issue ATA READ PARAMETERS (AtaParam_t)
     */
    fd = open(File, O_RDWR|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_DBG, "%s: open %s failed: %s", DevName, File, SYSERR);
	return((DevList_t *) NULL);
    }
    (void) close(fd);

    SImsg(SIM_DBG, "%s:\tATA PROBE: Found <%s>", DevName, File);

    /*
     * Create the DevInfo
     */
    DevInfo = NewDevInfo(NULL);
    DevInfo->Name = strdup(DevName);
    DevInfo->Driver = strdup(Name);
    DevInfo->ClassType = CT_ATA;
    DevInfo->Unit = Unit;
    DevInfo->Type = DevType;
    DevAddFile(DevInfo, strdup(File));

    /*
     * Crate DevList and return it.
     */
    DevList = (DevList_t *) xcalloc(1, sizeof(DevList_t));
    DevList->Name = DevInfo->Name;
    DevList->File = DevInfo->Files[0];
    DevList->Bus = 0;		/* XXX Assume 0 */
    DevList->Unit = Unit;
    DevList->DevInfo = DevInfo;

    return(DevList);
}

/*
 * Get and return a list of all ATA devices we can find.
 * We only build the list once to save time.
 */
static DevList_t *AtaGetAll()
{
    register int		u;
    static DevList_t	       *AtaDevices;
    register DevList_t	       *New = NULL;
    register DevList_t	       *Last = NULL;

    if (AtaDevices)
	return(AtaDevices);

    /*
     * Look for ATA Disks (sd)
     */
    for (u = 0; u < MAX_ATA_UNITS; ++u) {
	if (New = AtaProbe("wd", u, DT_DISKDRIVE)) {
	    if (!AtaDevices)
		AtaDevices = Last = New;
	    else {
		Last->Next = New;
		Last = New;
	    }
	}
    }

    return(AtaDevices);
}

/*
 * Controller probe function
 */
static int ProbeCtlr(CtlrDevInfo, TreePtr, SearchNames)
     DevInfo_t		       *CtlrDevInfo;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    DevInfo_t		       *ChildDevInfo = NULL;
    DevData_t			Child;
    int				ClassType;
    register DevList_t	       *Ptr;

    if (!CtlrDevInfo)
	return(-1);

    ClassType = CtlrDevInfo->ClassType;
    switch (ClassType) {
    case CT_SCSI:	Ptr = ScsiGetAll();	break;
    case CT_ATA:	Ptr = AtaGetAll();	break;
    default:
	SImsg(SIM_DBG, "%s: ClassType %d has no ProbeCtlr GetAll() func.",
	      CtlrDevInfo->Name, ClassType);
	return(-1);
    }

    for ( ; Ptr; Ptr = Ptr->Next)
	if (Ptr->Bus == CtlrDevInfo->Unit) {
	    /* 
	     * This device is on this Ctlr
	     */
	    (void) memset(&Child, 0, sizeof(Child));
	    Child.Slave = -1;
	    Child.DevNum = -1;
	    Child.Flags |= DD_IS_ALIVE;
	    Child.DevName = Ptr->DevInfo->Driver;
	    Child.DevUnit = Ptr->DevInfo->Unit;
	    Child.CtlrName = CtlrDevInfo->Driver;
	    Child.CtlrUnit = CtlrDevInfo->Unit;
	    Child.OSDevInfo = Ptr->DevInfo;
	    Child.OSDevInfo->Master = CtlrDevInfo;
	    if (ChildDevInfo = ProbeDevice(&Child, TreePtr, SearchNames, NULL))
		AddDevice(ChildDevInfo, TreePtr, SearchNames);
	}

    return(0);
}

/*
 * Call Device Specific probe function
 */
static int ProbeDeviceSpec(DevInfo, TreePtr, SearchNames)
     DevInfo_t		       *DevInfo;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    char		       *Name;

    if (!DevInfo)
	return(-1);

    Name = DevInfo->Name;

    switch (DevInfo->Type) {
    case DT_DISKCTLR:
    case DT_CONTROLLER:
	switch (DevInfo->ClassType) {
	case CT_SCSI:
	case CT_ATA:
	    ProbeCtlr(DevInfo, TreePtr, SearchNames);	break;
	default:
	    SImsg(SIM_DBG, 
		  "%s: No probe func for DevType=%d ClassType=%d",
		  Name, DevInfo->Type, DevInfo->ClassType);
	}
	break;
    default:
	SImsg(SIM_DBG, 
	      "%s: No probe func for DevType=%d",
	      Name, DevInfo->Type);
    }

    return(0);
}

/*
 * Probe a PCI device
 */
static int PCIprobeDevice(PciConf, TreePtr, SearchNames)
     struct pci_conf	       *PciConf;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{   
    DevInfo_t		       *DevInfo = NULL;
    static DevData_t		DevData;
    char		       *DevName;
    static char			Name[64];

    if (!PciConf)
	return(-1);

    DevName = PCIgetDevName(PciConf);
    (void) memset((void *) &DevData, 0, sizeof(DevData));
    DevData.NodeID = (int) PciConf;	/* Fake up a NodeID */
    DevData.Slave = -1;
    DevData.DevNum = -1;
    DevData.Flags |= DD_IS_ALIVE;
    DevData.DevName = strdup(DevName);
    DevData.DevUnit = PciConf->pc_sel.pc_func;
    DevData.CtlrName = "pci";
    DevData.CtlrUnit = PciConf->pc_sel.pc_bus;
    PCIcreatePCI(&DevData, TreePtr, SearchNames);

    PCIsetInfo(&DevData, PciConf);
    /*
     * Certain types of devices need pc_dev and pc_func to fully 
     * identify themselves with a uniq device name.
     */
    if (EQ(DevName, "chip")) {
	(void) snprintf(Name, sizeof(Name), "%s%d:%d:%d",
			DevName, PciConf->pc_sel.pc_bus,
			PciConf->pc_sel.pc_dev, PciConf->pc_sel.pc_func);
	DevData.OSDevInfo->Name = strdup(Name);
    }

    /* Probe and add device */
    if (TreePtr && (DevInfo = (DevInfo_t *) 
		    ProbeDevice(&DevData, TreePtr, SearchNames, NULL))) {
	AddDevice(DevInfo, TreePtr, SearchNames);
    }

    /*
     * Do followup work such as looking for children on this device
     */
    ProbeDeviceSpec(DevInfo, TreePtr, SearchNames);

    return(0);
}

/*
 * Probe for PCI devices
 *
 * We currently use the PCIOCGETCONF ioctl on /dev/pci.
 * We could probably also read "pci_dev_list" from /kernel.
 */
static int PCIprobe(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    int				fd;
    struct pci_conf_io		io;
    struct pci_conf		Conf[MAX_PCI_DEVICES];
    struct pci_conf	       *ConfPtr;
    struct pci_conf	       *ConfEnd;

    SImsg(SIM_DBG, "PCI: Probing...");

    fd = open(_PATH_DEVPCI, O_RDWR, 0);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open failed: %s", _PATH_DEVPCI, SYSERR);
	return(-1);
    }

    memset(&io, CNULL, sizeof(io));
#if	OSMVER >= 3
    io.match_buf_len = sizeof(Conf);
    io.matches = Conf;
#else 	/* OSMVER < 3 */
    io.pci_len = sizeof(Conf);
    io.pci_buf = Conf;
#endif	/* OSMVER */

    if (ioctl(fd, PCIOCGETCONF, &io) < 0) {
	SImsg(SIM_GERR, "%s: ioctl PCIOCGETCONF failed: %s", 
	      _PATH_DEVPCI, SYSERR);
	(void) close(fd);
	return(-1);
    }

    (void) close(fd);

#if	OSMVER >= 3
    ConfEnd = &Conf[ io.num_matches ];
#else 	/* OSMVER < 3 */
    ConfEnd = &Conf[ io.pci_len / sizeof(Conf[0]) ];
#endif	/* OSMVER */

    for (ConfPtr = Conf; ConfPtr < ConfEnd; ConfPtr++) {
#if	OSMVER >= 3
	SImsg(SIM_DBG,
        "PCI: %s vendor=0x%04lx device=0x04lx class=0x%06x card=0x%08lx",
	      ConfPtr->pd_name,
	      ConfPtr->pc_vendor, ConfPtr->pc_device,
	      ConfPtr->pc_class >> 8, ConfPtr->pc_subdevice);
	SImsg(SIM_DBG,
        "PCI: %s rev=0x%02x hdr=0x%02x",
	      ConfPtr->pd_name,
	      ConfPtr->pc_revid, ConfPtr->pc_hdr);
#else	/* OSMVER < 3 */
	SImsg(SIM_DBG,
        "PCI: %s class=0x%06x card=0x%08lx chip=0x%08lx rev=0x%02x hdr=0x%02x",
	      PCIname(ConfPtr, 0),
	      ConfPtr->pc_class >> 8, ConfPtr->pc_subid,
	      ConfPtr->pc_devid, ConfPtr->pc_class & 0xff, ConfPtr->pc_hdr);
#endif	/* OSMVER >= 3 */
	PCIprobeDevice(ConfPtr, TreePtr, SearchNames);
    }

    return(0);
}

/*
 * Probe for Misc. devices
 */
static int Miscprobe(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    DevInfo_t		       *DevInfo;
    int				Found = 0;

    SImsg(SIM_DBG, "MISC: Probing...");

    if (!kd)
	if (!(kd = KVMopen()))
	    return(-1);

    if (DevInfo = ProbeMouse(NULL)) {
	++Found;
	AddDevice(DevInfo, TreePtr, SearchNames);
    }

    (void) KVMclose(kd);
    kd = NULL;

    if (Found)
	return(0);
    else {
	SImsg(SIM_DBG, "ISA: No devices where found.");
	return(-1);
    }
}

/*
 * Build device tree and place in TreePtr.
 */
extern int BuildDevices(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    int				Status = 0;

    if (PCIprobe(TreePtr, SearchNames) != 0)
	--Status;

    if (ISAprobe(TreePtr, SearchNames) != 0)
	--Status;

    if (Miscprobe(TreePtr, SearchNames) != 0)
	--Status;

    return(0);
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
    register int		i;

    for (i = 0; DevTypes[i].Name; ++i)
	switch (DevTypes[i].Type) {
	case DT_DISKDRIVE:	DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_CD:		DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_DVD:		DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	case DT_CONSOLE:	DevTypes[i].Probe = ProbeConsole;	break;
	case DT_POINTER:	DevTypes[i].Probe = ProbeMouse;		break;
	case DT_FLOPPYCTLR:	DevTypes[i].Probe = ProbeFloppyCtlr;	break;
	}
}
