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
 * FreeBSD specific functions
 */

#include "defs.h"
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/device.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vm/swap_pager.h>
#include <machine/console.h>		/* ProbeConsole() */
#include <machine/mouse.h>		/* ProbeMouse() */
#include <scsi.h>
#include <sys/scsiio.h>
#include "myscsi.h"
#include <pci/pcivar.h>
#include <pci/pci_ioctl.h>
#include <i386/isa/isa_device.h>	/* ISA*() functions */
#include <sys/buf.h>			/* For fdc.h */
#include <i386/isa/fdc.h>		/* ProbeFloppyCtlr() */
#include <machine/ioctl_fd.h>		/* ProbeFloppy() */

/*
 * For consistancy
 */
static kvm_t			       *kd;

/*
 * ISA table type
 */
typedef struct {
    char		       *SymName;	/* Symbol name */
    int				DevType;	/* Type of Devices */
} ISAtable_t;

/*
 * List of ISA device tables to look for in kernel
 */
ISAtable_t ISAtables[] = {
    { "_isa_devtab_tty",	DT_SERIAL },
    { "_isa_devtab_bio",	0 },
    { "_isa_devtab_net",	DT_NETIF },
    { "_isa_devtab_null",	0 },
    { 0 },
};

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
 * Type used internally for building lists of devices we find
 */
struct _DevList {
    char		       *Name;		/* Dev Name ('sd0') */
    char		       *File;		/* Dev File ('/dev/rsd0') */
    int				Bus;		/* SCSI Bus # */
    int				Unit;		/* SCSI Unit # (target) */
    int				Lun;		/* Logical Unit # */
    DevInfo_t		       *DevInfo;	/* Filled in DevInfo */
    struct _DevList	       *Next;
};
typedef struct _DevList		DevList_t;

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
extern char			       *GetVirtMemNswapSym();
extern char			       *GetVirtMemSysCtl();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
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

    Amount = DivRndUp(ptoa(Maxmem), (Large_t)MBYTES);

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

    if (!ProbeData)
	return((DevInfo_t *) NULL);

    DevInfo = DeviceCreate(ProbeData);

    (void) snprintf(DevFile, sizeof(DevFile), "%s/%s", 
		    _PATH_DEV, DevInfo->Name);
    File = DevFile;
    fd = open(File, O_RDONLY|O_NONBLOCK|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: Cannot open for reading: %s", File, SYSERR);
	/*
	 * Try the default mouse device
	 */
	File = DefMouse;
	fd = open(File, O_RDONLY|O_NONBLOCK|O_NDELAY);
	if (fd < 0) {
	    SImsg(SIM_GERR, "%s: Cannot open for reading: %s", File, SYSERR);
	    return(DevInfo);
	}
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

/*
 * Probe a Floppy Controller (fdc)
 */
extern DevInfo_t *ProbeFloppyCtlr(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo;
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
    static struct pci_device	PciDev;
    static char			DriverName[PCI_DEV_NAME_MAX];
    static char			DevName[PCI_DEV_NAME_MAX];
    char		       *Name = NULL;

    DevName[0] = CNULL;

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
    Info.VendorID = PciConf->pc_devid & 0xffff;
    Info.DeviceID = PciConf->pc_devid >> 16;
    Info.SubDeviceID = PciConf->pc_subid;
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

/*
 * Issue a SCSI command and return the results.
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

    ReqPtr = &Req;
    if (SCSIREQ_ERROR(ReqPtr)) {
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed.", 
	      ScsiCmd->DevFile, CdbPtr->cmd);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
    return(0);
}

/*
 * Query a SCSI device with base (driver) named Name and Unit.
 */
static DevList_t *ScsiProbe(Name, Unit, DevType)
     char		       *Name;
     int			Unit;
     int			DevType;
{
    DevInfo_t		       *DevInfo = NULL;
    DevList_t		       *DevList;
    struct scsi_addr		Addr;
    static char			DevName[64];
    static char			File[128];
    int				fd;

    (void) snprintf(DevName, sizeof(DevName), "%s%d", Name, Unit);
    (void) snprintf(File, sizeof(File), "%s/r%s.ctl", _PATH_DEV, DevName);

    /*
     * Must be opened O_RDWR for SCIOCCOMMAND to work
     */
    fd = open(File, O_RDWR|O_NDELAY);
    if (fd < 0) {
	SImsg(SIM_DBG, "%s: open %s failed: %s", DevName, File, SYSERR);
	return((DevList_t *) NULL);
    }

    /*
     * Get the SCSI addressing info
     */
    if (ioctl(fd, SCIOCIDENTIFY, &Addr) < 0) {
	SImsg(SIM_DBG, "%s: ioctl SCIOCIDENTIFY failed: %s", File, SYSERR);
	(void) close(fd);
	return((DevList_t *) NULL);
    }
    if (Addr.scbus < 0 || Addr.target < 0) {
	SImsg(SIM_DBG, "%s: Invalid scsi_addr info: scbus=%d target=%d lun=%d",
	      File, Addr.scbus, Addr.target, Addr.lun);
	(void) close(fd);
	return((DevList_t *) NULL);
    }
    SImsg(SIM_DBG, "%s:\tSCSI IDENT: bus=%d target=%d lun=%d", 
	  File, Addr.scbus, Addr.target, Addr.lun);

    (void) close(fd);

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
    DevList->Bus = Addr.scbus;
    DevList->Unit = Addr.target;
    DevList->Lun = Addr.lun;
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
     * Look for SCSI Disks (sd)
     */
    for (u = 0; u < MAX_SCSI_UNITS; ++u) {
	if (New = ScsiProbe("sd", u, DT_DISKDRIVE)) {
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
    for (u = 0; u < MAX_SCSI_UNITS; ++u) {
	if (New = ScsiProbe("st", u, DT_TAPEDRIVE)) {
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

    fd = open(_PATH_DEVPCI, O_RDONLY, 0);
    if (fd < 0) {
	SImsg(SIM_GERR, "%s: open failed: %s", _PATH_DEVPCI, SYSERR);
	return(-1);
    }

    io.pci_len = sizeof(Conf);
    io.pci_buf = Conf;

    if (ioctl(fd, PCIOCGETCONF, &io) < 0) {
	SImsg(SIM_GERR, "%s: ioctl PCIOCGETCONF failed: %s", 
	      _PATH_DEVPCI, SYSERR);
	(void) close(fd);
	return(-1);
    }

    (void) close(fd);

    ConfEnd = &Conf[ io.pci_len / sizeof(Conf[0]) ];
    for (ConfPtr = Conf; ConfPtr < ConfEnd; ConfPtr++) {
	SImsg(SIM_DBG,
        "PCI: %s class=0x%06x card=0x%08lx chip=0x%08lx rev=0x%02x hdr=0x%02x",
	      PCIname(ConfPtr, 0),
	      ConfPtr->pc_class >> 8, ConfPtr->pc_subid,
	      ConfPtr->pc_devid, ConfPtr->pc_class & 0xff, ConfPtr->pc_hdr);

	PCIprobeDevice(ConfPtr, TreePtr, SearchNames);
    }

    return(0);
}

/*
 * Get the driver name for IsaDev.
 */
static char *ISAgetDriverName(IsaDev)
     struct isa_device	       *IsaDev;
{
    static struct isa_driver	Driver;
    static char			Name[64];

    if (!IsaDev)
	return((char *) NULL);

    if (!IsaDev->id_driver) {
	SImsg(SIM_DBG, "No id_driver found for IsaDev 0x%x", IsaDev);
	return((char *) NULL);
    }

    /*
     * Read in the id_driver
     */
    if (KVMget(kd, (KVMaddr_t) IsaDev->id_driver, (void *) &Driver,
		   sizeof(Driver), KDT_DATA) != 0) {
	SImsg(SIM_GERR, "Cannot read isa_driver entry from 0x%x.", 
	      IsaDev->id_driver);
	return((char *) NULL);
    }

    /*
     * Now read in the actual driver name.
     */
    Name[0] = CNULL;
    if (KVMget(kd, (KVMaddr_t) Driver.name, (void *) Name,
		   sizeof(Name), KDT_STRING) != 0) {
	SImsg(SIM_GERR, "Cannot read id_driver.name entry from 0x%x.", 
	      Driver.name);
	return((char *) NULL);
    }

    if (!Name[0])
	return((char *) NULL);
    else
	return(Name);
}

/*
 * Probe an ISA device.
 *
 * Return -1 on error.
 * Return 0 on success.
 */
static int ISAprobeDevice(IsaDev, DrName, DevType, IsaMaster, 
			  TreePtr, SearchNames)
     struct isa_device	       *IsaDev;
     char		       *DrName;
     int			DevType;
     DevInfo_t 		       *IsaMaster;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    DevInfo_t		       *DevInfo;
    DevDefine_t		       *DevDef = NULL;
    static DevData_t		DevData;

    if (!IsaDev || !DrName)
	return(-1);

    SImsg(SIM_DBG, "ISA: Probing <%s(%d)> DevType=%d", 
	  DrName, IsaDev->id_unit, DevType);

    (void) memset(&DevData, 0, sizeof(DevData));
    DevData.NodeID = IsaDev->id_id;
    DevData.DevName = DrName;
    DevData.DevUnit = IsaDev->id_unit;
    DevData.DevType = DevType;
    DevData.Flags = DD_IS_ALIVE;
    DevData.CtlrDevInfo = IsaMaster;
    DevDef = DevDefGet(DrName, 0, 0);

    if (DevInfo = ProbeDevice(&DevData, TreePtr, SearchNames, DevDef)) {
	/* 
	 * Add ISA specific info to DevInfo
	 */
	if (IsaDev->id_irq > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_irq), "IRQ", DA_APPEND);
	if (IsaDev->id_iobase > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_iobase), "IObase", DA_APPEND);
	if (IsaDev->id_drq > 0)
	    AddDevDesc(DevInfo, itoax(IsaDev->id_drq), "DRQ", DA_APPEND);
	if (IsaDev->id_msize > 0)
	    AddDevDesc(DevInfo, itoa(IsaDev->id_msize), "Size of I/O Memory",
		       DA_APPEND);
	if (IsaDev->id_maddr > 0)
	    AddDevDesc(DevInfo,  itoax(IsaDev->id_maddr), 
		       "Physical I/O Memory Address", DA_APPEND);
	/* 
	 * Now add the device to the Device Tree
	 */
	AddDevice(DevInfo, TreePtr, SearchNames);
	return(0);
    }
	
    return(-1);
}

/*
 * Iterate over a isa_devtab_* table and call the probe functin for
 * each device we find.
 */
static int ISAprobeDevTab(DevTable, DevType, TreePtr, SearchNames)
     struct isa_device	       *DevTable;
     int			DevType;
     DevInfo_t 		      **TreePtr;
     char		      **SearchNames;
{
    static DevInfo_t	       *IsaMaster;
    static struct isa_device	Device;
    struct isa_device	       *Ptr;
    char		       *DrName;
    int				Found = 0;

    if (!DevTable)
	return(-1);

    if (!IsaMaster) {
	/* Create a nice top master node */
	IsaMaster = NewDevInfo(NULL);
	IsaMaster->Driver = "isa";
	IsaMaster->Name = "isa0";	/* Fake */
	IsaMaster->Unit = 0;		/* Fake */
	IsaMaster->Type = DT_BUS;
	IsaMaster->ClassType = CT_ISA;
	AddDevice(IsaMaster, TreePtr, SearchNames);
    }

    for (Ptr = DevTable; Ptr; ++Ptr) {
	if (KVMget(kd, (KVMaddr_t) Ptr, (void *) &Device,
		   sizeof(Device), KDT_DATA) != 0) {
	    SImsg(SIM_GERR, "Cannot read isa_device entry from 0x%x.", Ptr);
	    /* Return now since this may be serious */
	    return(-1);
	}

	if (!Device.id_driver)
	    break;

	if (!(DrName = ISAgetDriverName(&Device)))
	    continue;

	SImsg(SIM_DBG, 
"ISA: <%s(%d)> id=0x%x (%d) iobase=0x%x irq=0x%x alive=%d enabled=%d flags=0x%x",
	      DrName, Device.id_unit, Device.id_id, Device.id_id,
	      Device.id_iobase, Device.id_irq,
	      Device.id_alive, Device.id_enabled,
	      Device.id_flags);

	/*
	 * Only probe device if it's enabled
	 */
	if (Device.id_enabled)
	    if (ISAprobeDevice(&Device, DrName, DevType, 
			       IsaMaster, TreePtr, SearchNames) == 0)
		++Found;
    }

    return(Found);
}

/*
 * Get the pointer to the first entry of a isa_devtab_* table from the kernel
 */
static struct isa_device *ISAgetDevTabPtr(Symbol)
     char		       *Symbol;
{
    struct nlist	       *nlPtr;
    struct isa_device	       *DevTabPtr;

    if (!Symbol)
	return((struct isa_device *) NULL);

    if ((nlPtr = KVMnlist(kd, Symbol, (struct nlist *)NULL, 0)) == NULL)
	return((struct isa_device *) NULL);

    if (CheckNlist(nlPtr))
	return((struct isa_device *) NULL);

    DevTabPtr = (struct isa_device *) nlPtr->n_value;

    return(DevTabPtr);
}

/*
 * Probe for ISA devices
 */
static int ISAprobe(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    ISAtable_t		       *Table;
    struct isa_device	       *DevTabPtr;
    int				Found = 0;

    SImsg(SIM_DBG, "ISA: Probing...");

    if (!kd)
	if (!(kd = KVMopen()))
	    return(-1);

    for (Table = ISAtables; Table && Table->SymName; ++Table) {
	if (DevTabPtr = ISAgetDevTabPtr(Table->SymName))
	    Found += ISAprobeDevTab(DevTabPtr, Table->DevType, 
				    TreePtr, SearchNames);
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
	case DT_CDROM:		DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	case DT_CONSOLE:	DevTypes[i].Probe = ProbeConsole;	break;
	case DT_POINTER:	DevTypes[i].Probe = ProbeMouse;		break;
	case DT_FLOPPYCTLR:	DevTypes[i].Probe = ProbeFloppyCtlr;	break;
	}
}
