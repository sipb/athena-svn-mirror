/*
 * Copyright (c) 1992-1996 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-sunos.c,v 1.1.1.1 1996-10-07 20:16:53 ghudson Exp $";
#endif

/*
 * General SunOS specific routines
 */

#include "defs.h"
#include "kdt-sunos.h"
#include <utmp.h>
#include <sys/stat.h>

/*
 * Name of frame buffer "indirect" device.
 */
#define FBDEVICE		"fb"

#if	OSMVER == 5
#include "os-sunos5.h"
#include <sys/t_lock.h>				/* For stdef.h */
#include <sys/scsi/targets/stdef.h>		/* For st_drivetype */
#else
#include "os-sunos4.h"
#include <scsi/targets/stdef.h>			/* For st_drivetype */
#endif	/* SUNOS == 5 */

/*
 * RomVec
 */
char 				RomVecSYM[] = "_romp";

/*
 * idprom
 */
char 				IdpromSYM[] = "_idprom";

#if	defined(HAVE_MAINBUS)
/*
 * MainBus symbol
 */
char 				MainBusSYM[] = "_mbdinit";
#endif	/* HAVE_MAINBUS */

/*
 * Top level Kernel Device Tree symbol
 */
char 				KDTsymbol[] = "_top_devinfo";

/*
 * SCSI Tape drive table
 */
char				STdrivetypeSYM[] = "_st_drivetypes";
char				STndrivetypeSYM[] = "_st_ndrivetypes";

/*
 * Number of CPU's
 */
char				NCpuSYM[] = "_ncpu";

/*
 * Platform Specific Interfaces
 */
extern char		       *GetKernArchSun();
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchSysinfo },
    { GetKernArchUname },
    { GetKernArchSun },
    { GetKernArchCmds },
    { GetAppArch },
    { NULL },
};
PSI_t GetAppArchPSI[] = {		/* Get Application Architecture */
    { GetAppArchDef },
    { GetAppArchSysinfo },
    { GetAppArchCmds },
    { GetAppArchTest },
    { NULL },
};
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeSysinfo },
    { GetCpuTypeTest },
    { GetCpuTypeCmds },
    { NULL },
};
extern char		       *GetNumCpuNCpuSym();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNumCpuSysconf },
    { GetNumCpuNCpuSym },
    { NULL },
};
extern char		       *GetKernVerSunOS5();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
#if	OSMVER == 5
    { GetKernVerSunOS5 },
#endif	/* OSMVER == 5 */
    { GetKernVerSym },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameSysinfo },
    { GetOSNameUname },
    { NULL },
};
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerSysinfo },
    { GetOSVerUname },
    { NULL },
};
char 			       *GetModelSun();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelFile },
    { GetModelSun },
    { NULL },
};
char 			       *GetSerialIDPROM();
PSI_t GetSerialPSI[] = {		/* Get Serial Number */
#if	OSMVER == 5
    { GetSerialSysinfo },		/* Doesn't work under sunos4 */
    { GetSerialIDPROM },		/* Doesn't work under sunos4 */
#endif
    { NULL },
};
char 			       *GetRomVerSun();
#if	defined(HAVE_OPENPROM)
char 			       *OBPgetRomVersion();
#endif	/* HAVE_OPENPROM */
PSI_t GetRomVerPSI[] = {		/* Get ROM Version */
#if	defined(HAVE_OPENPROM)
    { OBPgetRomVersion },
#endif	/* HAVE_OPENPROM */
    { GetRomVerSun },
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
extern char		       *GetMemorySunOS5();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
#if	OSMVER == 5
    { GetMemorySunOS5 },
#endif	/* OSMVER == 5 */
    { GetMemoryPhysmemSym },
    { NULL },
};
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemAnoninfo },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
#if	defined(BOOT_TIME)
    { GetBootTimeUtmp },
#endif	/* BOOT_TIME */
    { GetBootTimeSym },
    { NULL },
};

struct stat			StatBuf;
cputype_t			CpuType = 0;
static DevInfo_t	       *DevInfo;
static char 			Buf[BUFSIZ];
kvm_t		       	       *kd;
extern char		        CpuSYM[];
extern char		        IdpromSYM[];

/*
 * Build miscellaneous device tree
 */
extern int BuildMisc(TreePtr, SearchNames)
    DevInfo_t		      **TreePtr;
    char		      **SearchNames;
{
    DevInfo_t		       *ProbeKbd();
    int				Found = 1;

    if (DevInfo = ProbeKbd(TreePtr)) {
	AddDevice(DevInfo, TreePtr, SearchNames);
	Found = 0;
    }

    return(Found);
}

/*
 * Build device tree using TreePtr.
 * Calls bus and method specific functions to
 * search for devices.
 */
extern int BuildDevices(TreePtr, SearchNames)
    DevInfo_t 		       **TreePtr;
    char		       **SearchNames;
{
    int 			 Found = 1;

#if	OSMVER >= 5
    DetectDevices();
#endif

    if (KDTbuild(TreePtr, SearchNames) == 0)
	Found = 0;

#if	defined(HAVE_OPENPROM)
    if (OBPbuild(TreePtr, SearchNames) == 0)
	Found = 0;
#endif	/* HAVE_OPENPROM */

#if	defined(HAVE_MAINBUS)
    if (BuildMainBus(TreePtr, SearchNames) == 0)
	Found = 0;
#endif	/* HAVE_MAINBUS */

    if (BuildMisc(TreePtr, SearchNames) == 0)
	Found = 0;

    return(Found);
}

/*
 * Get the cpu type from the kernel
 */
cputype_t SunGetCpuType()
{
    struct nlist	       *nlptr;
    kvm_t		       *kd;
    cputype_t			cpu;

    if (!(kd = KVMopen()))
	return(-1);

    if ((nlptr = KVMnlist(kd, CpuSYM, (struct nlist *)NULL, 0)) == NULL)
	return(-1);

    if (CheckNlist(nlptr))
	return(-1);

    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &cpu, 
	       sizeof(cpu), KDT_DATA)) {
	if (Debug) Error("Cannot read cpu type from kernel.");
	return(-1);
    }

    KVMclose(kd);

    return(cpu);
}

/*
 * Determine our cpu model name.
 *
 * We first try to find the "cpu" or "cputype" symbol in the kernel.
 * If that's not present or is of an unknown value, we use the OBP
 * root node name.
 */
char *GetModelSun()
{
    Define_t		       *Model;
    register char	       *cp = NULL;
    register char	       *Name = NULL;

    if (CpuType <= 0 && !UseProm)
	CpuType = SunGetCpuType();

    if (CpuType > 0 && !UseProm) {
	Model = DefGet(DL_SYSMODEL, NULL, (long) CpuType, 0);
	if (Model)
	    Name = Model->ValStr1;
    }

    /*
     * SunOS no longer identifies individual machines by the "cputype"
     * symbol in the kernel.  Instead, the information is stored in the
     * kernel's device tree.
     *
     * This started with the SPARCclassic, LX, and SPARCcenter 2000.
     */
    if (!Name || UseProm)
	if (cp = GetSysModel())
	    Name = strdup(cp);

    if (Debug) printf("CPU = 0x%x  Name = <%s>\n", CpuType, ARG(Name));

    return(Name);
}

#if 	defined(CPU_ARCH) /* Sun */
#define	ARCH_MASK CPU_ARCH
#endif	/* CPU_ARCH */
#if 	defined(CPU_TYPE) /* Solbourne */
#define ARCH_MASK CPU_TYPE
#endif	/* CPU_TYPE */
/*
 * Determine our kernel architecture name from our hostid.
 */
extern char *GetKernArchSun()
{
#if	defined(ARCH_MASK)
    register int 		i;
    Define_t		       *Def;

    if (!CpuType)
	if ((CpuType = SunGetCpuType()) < 0)
	    return((char *) NULL);

    Def = DefGet(DL_KARCH, NULL, (long)(ARCH_MASK & CpuType), 0);
    if (Def && Def->ValStr1)
	return(Def->ValStr1);

    if (Debug)
	Error("Kernel Arch 0x%x not defined; Cpu = 0x%x Mask = 0x%x", 
	      CpuType & ARCH_MASK, CpuType, ARCH_MASK);
#endif	/* ARCH_MASK */

    return((char *) NULL);
}

#if	defined(HAVE_IDPROM)
/*
 * Get IDPROM information.
 */
static struct idprom *GetIDPROM()
{
    static struct idprom	idprom;
    struct nlist	       *nlptr;
    kvm_t		       *kd;

    if (!(kd = KVMopen()))
	return((struct idprom *)NULL);

    if ((nlptr = KVMnlist(kd, IdpromSYM, (struct nlist *)NULL, 0)) == NULL)
	return((struct idprom *)NULL);

    if (CheckNlist(nlptr))
	return((struct idprom *)NULL);

    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &idprom, 
	       sizeof(idprom), KDT_DATA)) {
	if (Debug) Error("Cannot read \"%s\" from kernel.", IdpromSYM);
	return((struct idprom *)NULL);
    }

    KVMclose(kd);

    return(&idprom);
}
#endif	/* HAVE_IDPROM */

/*
 * Get system serial number from ID PROM
 */
extern char *GetSerialIDPROM()
{
    static char			buff[BUFSIZ];
#if	defined(HAVE_IDPROM)
    struct idprom	       *idprom;

    if (!(idprom = GetIDPROM()))
	return((char *)NULL);

    /*
     * id_format is not set correctly under SunOS 4.x
     */
    if (idprom->id_format != IDFORM_1)
	if (Debug) Error("Warning: IDPROM format (%d) is incorrect.",
			 idprom->id_format);

    (void) sprintf(buff, "%d", idprom->id_serial);

#endif	/* HAVE_IDPROM */
    return((buff[0]) ? buff : (char *)NULL);
}

/*
 * Probe a FrameBuffer.
 */
extern DevInfo_t *ProbeFrameBuffer(FBname, DevData, DevDefine)
    char 		       *FBname;
    DevData_t 		       *DevData;
    DevDefine_t	 	       *DevDefine;
{
#if	defined(FBIOMONINFO)
    struct mon_info		mon_info;
#endif	/* FBIOMONINFO */
#if	defined(FBIOGXINFO)
    struct cg6_info 		cg6_info;
#endif	/* FBIOGXINFO */
    struct fbgattr		fbgattr;
    DevInfo_t 		       *DevInfo;
    FrameBuffer_t 	       *fb;
    DevDefine_t		       *FBdef;
    char 		       *File;
    static char			FileBuff[MAXPATHLEN];
    static char			Buff[BUFSIZ], Buff2[BUFSIZ];
    int 			FileDesc;
    register int		i;

    if (!FBname)
	return((DevInfo_t *) NULL);

    if (Debug)
	printf("ProbeFrameBuffer '%s'\n", FBname);

#if	OSMVER == 5
    if (DevDefine->File)
	(void) sprintf(FileBuff, "%s/%s%d", 
		       _PATH_DEV_FBS, DevDefine->File, 
		       DevData->DevUnit);
    else if (EQ(FBname, FBDEVICE))
	(void) sprintf(FileBuff, "%s/%s", _PATH_DEV, FBname);
    else
	(void) sprintf(FileBuff, "%s/%s", _PATH_DEV_FBS, FBname);
    File = FileBuff;
#else
    File = GetCharFile(FBname, NULL);
#endif	/* OSMVER == 5 */

    /*
     * Check the device file.  If the stat fails because
     * the device doesn't exist, trying the default framebuffer
     * device /dev/fb.
     */
    if (stat(File, &StatBuf) != 0) {
	if (errno == ENOENT && !EQ(FBname, FBDEVICE)) {
	    if (Debug) 
		Error("Framebuffer device `%s' does not exist.  Trying `%s'.",
		      FBname, FBDEVICE);
	    return(ProbeFrameBuffer(FBDEVICE, DevData, DevDefine));
	}
    }

    if ((FileDesc = open(File, O_RDONLY)) < 0) {
	if (Debug) Error("%s: Cannot open for reading: %s.", File, SYSERR);
	return((DevInfo_t *) NULL);
    }

    /*
     * Get real fb attributes
     */
    if (ioctl(FileDesc, FBIOGATTR, &fbgattr) != 0) {
	if (Debug) Error("%s: FBIOGATTR failed: %s.", File, SYSERR);
	if (ioctl(FileDesc, FBIOGTYPE, &fbgattr.fbtype) != 0) {
	    if (Debug) Error("%s: FBIOGTYPE failed: %s.", File, SYSERR);
	    return((DevInfo_t *) NULL);
	}
    }

    /*
     * We're committed to try
     */
    if (!(fb = NewFrameBuffer(NULL))) {
	Error("Cannot create new frame buffer.");
	return((DevInfo_t *) NULL);
    }

    if (!(DevInfo = NewDevInfo(NULL))) {
	Error("Cannot create new frame buffer device entry.");
	return((DevInfo_t *) NULL);
    }

#if	defined(FBIOGXINFO)
    /*
     * GX (cgsix) info
     */
    if (ioctl(FileDesc, FBIOGXINFO, &cg6_info) == 0) {
	AddDevDesc(DevInfo, itoa(cg6_info.slot), "SBus Slot", DA_APPEND);
	AddDevDesc(DevInfo, itoa(cg6_info.boardrev), "Board Revision", 
		   DA_APPEND);
	AddDevDesc(DevInfo, 
		   (cg6_info.hdb_capable) ? "Double Buffered" : 
		   "Single Buffered", "Buffering", DA_APPEND);
	if (cg6_info.vmsize)
	    fb->VMSize 	= mbytes_to_bytes(cg6_info.vmsize);
    } else
	if (Debug) Error("%s: FBIOGXINFO failed: %s.", File, SYSERR);
#endif 	/* FBIOGXINFO */

#if	defined(FBIOMONINFO)
    /*
     * Monitor info
     */
    if (ioctl(FileDesc, FBIOMONINFO, &mon_info) == 0) {
	AddDevDesc(DevInfo, FreqStr(mon_info.pixfreq), 
		   "Monitor Pixel Frequency", DA_APPEND);
	AddDevDesc(DevInfo, FreqStr(mon_info.hfreq), 
		   "Monitor Horizontal Frequency", DA_APPEND);
	AddDevDesc(DevInfo, FreqStr(mon_info.vfreq), 
		   "Monitor Vertical Frequency", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.hsync), 
		   "Monitor Horizontal Sync (pixels)", DA_APPEND);
	AddDevDesc(DevInfo, itoa(mon_info.vsync), 
		   "Monitor Vertical Sync (scanlines)", DA_APPEND);
    } else
	if (Debug) Error("%s: FBIOMONINFO failed: %s.", File, SYSERR);
#endif	/* FBIOMONINFO */

    /*
     * We're done doing ioctl()'s
     */
    close(FileDesc);

    /*
     * Find out what type of fb this is.
     */
    if (FLAGS_ON(DevDefine->Flags, DDT_DEFINFO)) {
	DevInfo->Model = DevDefine->Model;
    } else if (FBdef = DevDefGet(NULL, DT_FRAMEBUFFER, 
				 fbgattr.fbtype.fb_type)) {
	if (FBdef->Model) {
	    (void) strcpy(Buff, FBdef->Model);
	    if (FBdef->Name)
		(void) sprintf(Buff + strlen(FBdef->Model), " [%s]",
			       FBdef->Name);
	    DevInfo->Model	= strdup(Buff);
	} else
	    DevInfo->Model 	= FBdef->Name;
    } else {
	Error("Device `%s' is an unknown type (%d) of frame buffer.",
	      FBname, fbgattr.fbtype.fb_type);
	DevInfo->Model 		= "UNKNOWN";
    }

#if	defined(FB_ATTR_NEMUTYPES)
    /*
     * See if this fb emulates other fb's.
     */
    Buff[0] = CNULL;
    for (i = 0; i < FB_ATTR_NEMUTYPES && fbgattr.emu_types[i] >= 0; ++i)
	if (fbgattr.emu_types[i] != fbgattr.fbtype.fb_type &&
	    (FBdef = DevDefGet(NULL, DT_FRAMEBUFFER, 
			       (long)fbgattr.emu_types[i]))) {
	    if (Buff[0])
		(void) strcat(Buff, ", ");
	    (void) strcat(Buff,
			  (FBdef->Name) ? FBdef->Name : 
			  ((FBdef->Model) ? FBdef->Model : ""));
	}
    if (Buff[0])
	AddDevDesc(DevInfo, Buff, "Emulates", DA_APPEND);
#endif	/* FB_ATTR_NEMUTYPES */

    /*
     * Put things together
     */
    DevInfo->Name 			= FBname;
    DevInfo->Type 			= DT_FRAMEBUFFER;
    DevInfo->NodeID			= DevData->NodeID;
    DevInfo->DevSpec	 		= (caddr_t *) fb;

    fb->Height 				= fbgattr.fbtype.fb_height;
    fb->Width 				= fbgattr.fbtype.fb_width;
    fb->Depth 				= fbgattr.fbtype.fb_depth;
    fb->Size 				= fbgattr.fbtype.fb_size;
    fb->CMSize 				= fbgattr.fbtype.fb_cmsize;

    DevInfo->Master 			= MkMasterFromDevData(DevData);

    return(DevInfo);
}

/*
 * Get the Model name for a tape drive of type Type.
 */
extern char *GetTapeModel(Type)
    char			Type;
{
    extern char		        STdrivetypeSYM[];
    extern char		        STndrivetypeSYM[];
    register int		i;
    kvm_t		       *kd;
    struct nlist	       *nlptr;
    u_long			Addr;
    static struct st_drivetype  STDriveType;
    static char			Buff[BUFSIZ];
    static char			Name[BUFSIZ];
    int				NumTypes = 0;
    DevDefine_t		       *DevDef;

    kd = KVMopen();
    if (!kd)
	return((char *) NULL);

    /*
     * Read in the number of table entries in st_drivetypes.
     */
    nlptr = KVMnlist(kd, STndrivetypeSYM, (struct nlist *)NULL, 0);
    if (nlptr && CheckNlist(nlptr) == 0)
	if (KVMget(kd, (u_long) nlptr->n_value, (int *)&NumTypes,
		   sizeof(NumTypes), KDT_DATA))
	    if (Debug) Error("Read num drive types fromm kernel failed.");

    if (NumTypes > 0) {
	/*
	 * Read the SCSI Tape Drive Types table.
	 */
	nlptr = KVMnlist(kd, STdrivetypeSYM, (struct nlist *)NULL, 0);
	if (nlptr || CheckNlist(nlptr) == 0) {
	    for (Addr = nlptr->n_value, i = 0; i < NumTypes;
		 Addr += sizeof(STDriveType), ++i) {

		if (KVMget(kd, Addr, 
			   (struct st_drivetype *) &STDriveType, 
			   sizeof(STDriveType), KDT_DATA)) {
		    if (Debug) 
			Error("Cannot read ST Drive Types entry from 0x%x.",
			      Addr);
		    break;
		}

		if (STDriveType.type == Type) {
		    /*
		     * Found the right entry.
		     */
		    Addr = (u_long) STDriveType.name;
		    if (Addr == 0)
			break;
		    if (KVMget(kd, Addr, (char *) Name, 
			       sizeof(Name), KDT_STRING)) {
			if (Debug) Error("Failed to read STDriveType name.");
			break;
		    }
		    KVMclose(kd);
		    (void) strcpy(Buff, Name);
		    if (STDriveType.vid[0])
			(void) sprintf(Buff + strlen(Name), " (%s)",
				       STDriveType.vid);
		    return(Buff);
		}
	    }
	} else if (Debug)
	    Error("Cannot read ST drive types from kernel.");
    }

    KVMclose(kd);

    /*
     * We didn't find the entry in a kernel table, so
     * we now check our static builtin table.
     */
    DevDef = DevDefGet(NULL, DT_TAPEDRIVE, (long)Type);
    if (DevDef && DevDef->Model)
	return(DevDef->Model);

    /*
     * No Luck!
     */
    return((char *) NULL);
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
	case DT_FRAMEBUFFER:	DevTypes[i].Probe = ProbeFrameBuffer;	break;
	case DT_KEYBOARD:	DevTypes[i].Probe = ProbeKbd;		break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
#if	OSMVER == 4
	case DT_CDROM:		DevTypes[i].Probe = ProbeCDROMDrive;	break;
#endif	/* OSMVER==4 */
#if	defined(HAVE_OPENPROM)
	case DT_CPU:		DevTypes[i].Probe = OBPprobeCPU;	break;
#endif	/* HAVE_OPENPROM */
	}
}

/*
 * Find the "_ncpu" symbol to determine number
 * of CPU's in system.
 */
extern char *GetNumCpuNCpuSym()
{
    struct nlist	       *nlptr;
    static char			Buff[50];
    extern char			NCpuSYM[];
    kvm_t		       *kd;
    int				NumCpu;

    if (!(kd = KVMopen()))
	return((char *) NULL);

    if ((nlptr = KVMnlist(kd, NCpuSYM, (struct nlist *)NULL, 0)) == NULL)
	return((char *) NULL);

    if (CheckNlist(nlptr))
	return((char *) NULL);

    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &NumCpu, 
	       sizeof(NumCpu), KDT_DATA)) {
	if (Debug) Error("Cannot read ncpu from kernel.");
	return((char *) NULL);
    }

    KVMclose(kd);

    if (NumCpu > 0) {
	(void) sprintf(Buff, "%d", NumCpu);
	return(Buff);
    } else
	return((char *)NULL);
}

/*
 * Set the disk controller model name from a disk.
 */
extern int SetDiskCtlrModel(DevInfo, CtlrType)
    DevInfo_t		       *DevInfo;
    int				CtlrType;
{
    DevDefine_t		       *DevDefine;

    DevDefine = DevDefGet(NULL, DT_DISKCTLR, CtlrType);
    if (!DevDefine)
	return(0);

    if (DevDefine->Model)
	DevInfo->Model = DevDefine->Model;
    if (DevDefine->Desc)
	DevInfo->ModelDesc = DevDefine->Desc;

    return(0);
}

/*
 * Get system CPU type from OBP.
 */
extern char *GetSysModel()
{
    char		       *Name;

    /*
     * First try getting it from the kernel, if that
     * fails, then query the OBP directly.
     */
    Name = KDTgetSysModel();
#if	defined(HAVE_OPENPROM)
    if (!Name)
	Name = OBPgetSysModel();
#endif	/* HAVE_OPENPROM */

    if (Name)
	return(KDTcleanName(Name, TRUE));
    else
	return((char *) NULL);
}

/*
 * Expand a key string into something more easily read.
 */
extern char *ExpandKey(string)
    char		       *string;
{
    register char	       *cp;
    register int		len;
    static char			buff[BUFSIZ];

    if (!string)
	return((char *)NULL);

    (void) strcpy(buff, string);

    /* Capitolize first letter */
    buff[0] = toupper(buff[0]);

    for (cp = &buff[0]; cp && *cp; ++cp) {
	if (*cp == '-' || *cp == '_') {
	    *cp++ = ' ';
	    if (*cp)
		*cp = toupper(*cp);
	}
    }

    /* Remove ending '?' */
    len = strlen(buff);
    if (buff[len-1] == '?')
	buff[len-1] = CNULL;

    return(buff);
}

/*
 * Get a string version of a long 
 */
static char *GetLongValStr(String)
    char		       *String;
{
    static char			buff[BUFSIZ];
    long			lval;
    long		       *lptr;
    long			nval;

    lval = strtol(String, (char **)NULL, 0);
    lptr = (long *) &String[0];
    nval = *lptr;
    (void) sprintf(buff, "%u", (u_long) nval);

    return(buff);
}

/*
 * Determine if opvalue is a string or not.
 */
extern int IsString(opvalue, opsize)
    char		       *opvalue;
    int				opsize;
{
    register int		i;

    for (i = 0; i < opsize - 1; ++i) {
	if (opvalue[i] == CNULL)
	    return(0);
	if (iscntrl(opvalue[i]) || !isascii(opvalue[i]))
	    return(0);
    }

    return(1);
}

/*
 * Decode a value into a printable string.
 */
extern char *DecodeVal(Value, Size)
    char		       *Value;
    int				Size;
{
    static char			Buff[2*BUFSIZ];
    static char			Buff2[2];
    register int		i;
    int				OpMask = 0xff;

    if (Size == 0 || !Value)
	return((char *) NULL);

    if (IsString(Value, Size)) {
	(void) strcpy(Buff, Value);
	return(Buff);
    } else {
	if (Size > 4) {
	    if (Size > OpMask)
		return((char *) NULL);
	    (void) strcpy(Buff, "0x");
	    for (i = 0; i < Size; ++i) {
		if (i > 0 && ((i % 4) == 0))
		    (void) strcat(Buff, ".0x");
		(void) sprintf(Buff2, "%02x", Value[i] & OpMask);
		(void) strcat(Buff, Buff2);
	    }
	} else
	    (void) strcpy(Buff, GetLongValStr(Value));
	return(Buff);
    }
}
