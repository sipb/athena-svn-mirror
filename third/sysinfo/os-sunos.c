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
 * General SunOS specific routines
 */

#include "defs.h"
#include "sunos-kdt.h"
#include <utmp.h>

#if	OSMVER == 5
#include "os-sunos5.h"
#include <sys/scsi/impl/uscsi.h>		/* USCSICMD */
#if		OSVER >= 56
#include <sys/scsi/scsi_types.h>		/* For stdef.h */
#else		/* OSVER < 56 */
#include <sys/t_lock.h>				/* For stdef.h */
#endif		/* 5.6 */
#include <sys/scsi/targets/stdef.h>		/* For st_drivetype */
#else	/* OSMVER != 5 */
#include "os-sunos4.h"
#include <scsi/targets/stdef.h>			/* For st_drivetype */
#include <scsi/impl/uscsi.h>			/* USCSICMD */
#endif	/* SUNOS == 5 */
#include "myscsi.h"
#include "sunos-obp.h"

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
    { GetCpuTypeIsalist },
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
#if	OSMVER == 5
extern char		       *GetKernVerSunOS5();
#endif	/* OSMVER == 5 */
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
#if	OSMVER == 5
extern char		       *GetOSDistSunOS5();
#endif	/* OSMVER == 5 */
PSI_t GetOSDistPSI[] = {		/* Get OS Dist */
#if	OSMVER == 5
    { GetOSDistSunOS5 },
#endif	/* OSMVER == 5 */
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
extern char		       *GetMemorySunOSsysmem();
extern char		       *GetMemorySunOSsysconf();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
#if	OSMVER == 5
    { GetMemorySunOSsysmem },
    { GetMemorySunOSsysconf },
#endif	/* OSMVER == 5 */
    { GetMemoryPhysmemSym },
    { NULL },
};
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemSwapctl },
    { GetVirtMemAnoninfo },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
#if	defined(HAVE_GETUTID)
    { GetBootTimeGetutid },
#endif	/* HAVE_GETUTID */
#if	defined(BOOT_TIME)
    { GetBootTimeUtmp },
#endif	/* BOOT_TIME */
    { GetBootTimeSym },
    { NULL },
};

cputype_t			CpuType = 0;
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
    DevInfo_t		       *DevInfo;
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
	SImsg(SIM_GERR, "Cannot read cpu type from kernel.");
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

    SImsg(SIM_DBG, "CPU = 0x%x  Name = <%s>", CpuType, ARG(Name));

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
	SImsg(SIM_UNKN, "Kernel Arch 0x%x not defined; Cpu = 0x%x Mask = 0x%x",
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
	SImsg(SIM_GERR, "Cannot read \"%s\" from kernel.", IdpromSYM);
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
    static char			Buff[128];
#if	defined(HAVE_IDPROM)
    struct idprom	       *idprom;

    if (!(idprom = GetIDPROM()))
	return((char *)NULL);

    /*
     * id_format is not set correctly under SunOS 4.x
     */
    if (idprom->id_format != IDFORM_1)
	SImsg(SIM_DBG, "Warning: IDPROM format (%d) is incorrect.",
	      idprom->id_format);

    (void) snprintf(Buff, sizeof(Buff),  "%d", idprom->id_serial);

#endif	/* HAVE_IDPROM */
    return((Buff[0]) ? Buff : (char *)NULL);
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
    register int		l;
    kvm_t		       *kd;
    struct nlist	       *nlptr;
    u_long			Addr;
    static struct st_drivetype  STDriveType;
    static char			Buff[128];
    static char			Name[128];
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
	    SImsg(SIM_GERR, "Read num drive types fromm kernel failed.");

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
		    SImsg(SIM_GERR, 
			  "Cannot read ST Drive Types entry from 0x%x.", Addr);
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
			SImsg(SIM_GERR, "Failed to read STDriveType name.");
			break;
		    }
		    KVMclose(kd);
		    (void) strcpy(Buff, Name);
		    if (STDriveType.vid[0])
			(void) snprintf(Buff + (l=strlen(Name)), 
					sizeof(Buff)-l, 
					" (%s)",
					STDriveType.vid);
		    return(Buff);
		}
	    }
	} else
	    SImsg(SIM_GERR, "Cannot read ST drive types from kernel.");
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
 * Lookup a Vendors abbreviation (e.g. 'SUNW') and return the full name.
 */
extern char *GetVendorName(Abbr)
     char		       *Abbr;
{
    Define_t		       *Def;

    if (Def = DefGet("Vendor", Abbr, 0, 0))
	return(Def->ValStr1);

    SImsg(SIM_DBG, "Warning: <%s> is not a defined Vendor in config/*.cf.", 
	  Abbr);

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
#if	OSMVER == 5
	case DT_FLOPPY:		DevTypes[i].Probe = ProbeFloppy;	break;
#endif
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
	SImsg(SIM_GERR, "Cannot read ncpu from kernel.");
	return((char *) NULL);
    }

    KVMclose(kd);

    if (NumCpu > 0) {
	(void) snprintf(Buff, sizeof(Buff),  "%d", NumCpu);
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
 * Get system model using sysinfo(SI_ISALIST) call.
 * See isalist(5).
 */
extern char *GetModelISAList()
{
    char		       *BuffPtr = NULL;
#if	defined(SI_ISALIST)
    static char			Buff[128];
    char		       *cp;
    int				DoCapAll = FALSE;
    int 			DoCapOnce = FALSE;

    if (sysinfo(SI_ISALIST, Buff, sizeof(Buff)) <= 0)
	return;

    /*
     * Now cleanup what we got.  Take something like
     * "pentium_pro+mmx pentium i486 i386" and make it into
     * "Pentium Pro MMX".
     */
    for (cp = Buff; *cp; ++cp) {
	if (cp == Buff || DoCapOnce || DoCapAll)
	    *cp = toupper(*cp);
	if (DoCapOnce) DoCapOnce = FALSE;
	if (*cp == ' ' || *cp == '\t') {
	    /* End of first word so we're done! */
	    *cp = CNULL;
	    break;
	} else if (*cp == '_') {
	    *cp = ' ';
	    DoCapOnce = TRUE;
	    DoCapAll = FALSE;
	} else if (*cp == '+') {
	    *cp = ' ';
	    DoCapAll = TRUE;
	}
    }

    BuffPtr = Buff;
#endif /* SI_ISALIST */
    return(BuffPtr);
}

/*
 * Get system CPU type from OBP or KDT.
 */
extern char *GetSysModel()
{
    char		       *Name;
    char		       *cp;

    /*
     * First try getting it from the kernel, if that
     * fails, then query the OBP directly.
     */
    Name = KDTgetSysModel();
#if	defined(HAVE_OPENPROM)
    if (!Name)
	Name = OBPgetSysModel();
#endif	/* HAVE_OPENPROM */

    /*
     * Use sysinfo(SI_ISALIST) if we're running on a x86
     */
    if (Name && EQ(Name, "i86pc"))
	if (cp = GetModelISAList())
	    return(cp);

    if (Name)
	return(KDTcleanName(Name, TRUE));
    else
	return((char *) NULL);
}

/*
 * Expand a key string into something more easily read.
 */
extern char *ExpandKey(String)
    char		       *String;
{
    static char			Buff[BUFSIZ];
    register int		Len;
    register char	       *cp;

    if (!String)
	return((char *)NULL);

    Len = strlen(String);
    (void) strncpy(Buff, String, (Len < sizeof(Buff)) ? Len : sizeof(Buff));
    Buff[Len] = CNULL;

    /* Capitolize first letter */
    Buff[0] = toupper(Buff[0]);

    for (cp = &Buff[0]; cp && *cp; ++cp) {
	if (*cp == '-' || *cp == '_') {
	    *cp++ = ' ';
	    if (*cp)
		*cp = toupper(*cp);
	}
    }

    /* Remove ending '?' */
    Len = strlen(Buff);
    if (Buff[Len-1] == '?')
	Buff[Len-1] = CNULL;

    return(Buff);
}

/*
 * Get a string version of a long 
 */
static char *GetLongValStr(String, Size)
    char		       *String;
    int				Size;
{
    static char			Buff[128];
    static char			Buff2[4];
    long			lval;
    register int		i;
    unsigned char		c;

    if (Size > sizeof(Buff)) {
	SImsg(SIM_GERR, "GetLongValStr(..., %d): Buffer too small (%d).",
	      Size, sizeof(Buff));
	return((char *) NULL);
    }

    /*
     * We convert String -> hex-string-number -> long -> decimal-number
     * This torterous route is the only way to insure we handle large 
     * values on 32 and 64-bit systems.  Sigh!
     */

    /* Convert to hex-string-number */
    Buff[0] = CNULL;
    for (i = 0; i < Size; ++i) {
	c = (unsigned char) String[i];
	(void) snprintf(Buff2, sizeof(Buff2), "%2.2x", c);
	(void) strcat(Buff, Buff2);
    }

    /* Convert to long */
    lval = strtol(Buff, (char **)NULL, 16);

    /* Convert to decimal-number */
    (void) snprintf(Buff, sizeof(Buff), "%u", lval);

    return(Buff);
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
extern char *DecodeVal(Key, Value, Size)
    char		       *Key;
    char		       *Value;
    int				Size;
{
    static char			Buff[512];
    static char			Buff2[2];
    register int		i;
    int				Len;
    int				OpMask = 0xff;

    if (Size == 0 || !Value)
	return((char *) NULL);

    if (IsString(Value, Size)) {
	(void) strncpy(Buff, Value, 
		       (Size < sizeof(Buff)) ? Size : sizeof(Buff)-1);
	Buff[Size] = CNULL;
	return(Buff);
    } else {
	if (Size > 4) {
	    /* Get the hex string representatin */
	    (void) strcpy(Buff, "0x");
	    Len = strlen(Buff);
	    for (i = 0; i < Size && i < sizeof(Buff)-Len-4; ++i) {
		if (i > 0 && ((i % 4) == 0))
		    (void) strcat(Buff, ".0x");
		(void) snprintf(Buff2, sizeof(Buff2), "%02x", 
				Value[i] & OpMask);
		(void) strcat(Buff, Buff2);
		Len = strlen(Buff);
	    }
	} else
	    /* Get the decimal string representation */
	    (void) snprintf(Buff, sizeof(Buff), "%s", 
			    GetLongValStr(Value, Size));

	return(Buff);
    }
}

/*
 * Issue a SCSI command and return the results.
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
    static char			Buff[SCSI_BUF_LEN];
    static struct uscsi_cmd	Cmd;
    ScsiCdbG0_t		       *CdbPtr = NULL;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Cmd, 0, sizeof(Cmd));

    Cmd.uscsi_cdb = (caddr_t) ScsiCmd->Cdb;
    Cmd.uscsi_cdblen = ScsiCmd->CdbLen;
    Cmd.uscsi_bufaddr = (caddr_t) Buff;
    Cmd.uscsi_buflen = sizeof(Buff);
    Cmd.uscsi_flags = USCSI_ISOLATE | USCSI_SILENT | USCSI_READ;
#if	OSMVER >= 5
    Cmd.uscsi_timeout = MySCSI_CMD_TIMEOUT;	/* uscsi_timeout==seconds */
#endif	/* OSMVER >= 5 */

    /*
     * Send cmd to device
     */
    errno = 0;
    (void) ioctl(ScsiCmd->DevFD,  USCSICMD, &Cmd);
    if (Cmd.uscsi_status != 0 || errno > 0)  {
	/* 
	 * We just need Cdb.cmd so it's ok to assume ScsiCdbG0_t here
	 */
	CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;
	SImsg(SIM_GERR, "%s: SCSI command 0x%x failed: %s", 
	      ScsiCmd->DevFile, CdbPtr->cmd, SYSERR);
	return(-1);
    }

    ScsiCmd->Data = (void *) Buff;
    return(0);
}

#ifdef NOTYET
/*
 * dadkio_status values
 */
static char *DadErrs[] = {
    "Command succeeded",		/* DADKIO_STAT_NO_ERROR */
    "Device not ready",			/* DADKIO_STAT_NOT_READY */
    "Media error",			/* DADKIO_STAT_MEDIUM_ERROR */
    "Hardware error",			/* DADKIO_STAT_HARDWARE_ERROR */
    "Illegal Request",			/* DADKIO_STAT_ILLEGAL_REQUEST */
    "Illegal Block Address",		/* DADKIO_STAT_ILLEGAL_ADDRESS */
    "Device is write protected",	/* DADKIO_STAT_WRITE_PROTECTED */
    "No response from device",		/* DADKIO_STAT_TIMED_OUT */
    "Parity error in data",		/* DADKIO_STAT_PARITY */
    "Error on bus",			/* DADKIO_STAT_BUS_ERROR */
    "Data recovered via ECC",		/* DADKIO_STAT_SOFT_ERROR */
    "No resources for command",		/* DADKIO_STAT_NO_RESOURCES */
    "Device is not formatted",		/* DADKIO_STAT_NOT_FORMATTED */
    "Device is reserved",		/* DADKIO_STAT_RESERVED */
    "Feature not supported"		/* DADKIO_STAT_NOT_SUPPORTED */
};
#define NumDadErrs	(sizeof(DadErrs)/sizeof(char *))

/*
 * Issue a ATA command and return the results.
 */
extern int AtaCmd(AtaCmd)
     AtaCmd_t		       *AtaCmd;
{
    static char			Buff[ATA_BUF_LEN];
    static struct dadkio_rwcmd	Cmd;

    if (!AtaCmd || !AtaCmd->Cdb || !AtaCmd->CdbLen || 
	AtaCmd->DevFD < 0 || !AtaCmd->DevFile) {
	SImsg(SIM_DBG, "AtaCmd: Bad parameters.");
	return(-1);
    }

    (void) memset(Buff, 0, sizeof(Buff));
    (void) memset(&Cmd, 0, sizeof(Cmd));

    Cmd.cmd = DADKIO_RWCMD_READ;
    Cmd.flags = DADKIO_FLAG_SILENT;		/* No console messages */
    Cmd.blkaddr = 0; /* XXX What should this be? */
    Cmd.bufaddr = (caddr_t) Buff;
    Cmd.buflen = sizeof(Buff);

    /*
     * Send cmd to device
     */
    if (ioctl(AtaCmd->DevFD, DIOCTL_RWCMD, &Cmd) < 0) {
	/* 
	 * We just need Cdb.cmd so it's ok to assume AtaCdbG0_t here
	 */
	SImsg(SIM_GERR, "%s: ATA ioctl DIOCTL_RWCMD command failed: %s", 
	      AtaCmd->DevFile, SYSERR);
	return(-1);
    }

    if (Cmd.status.status) {
	if (Cmd.status.status > 0 && Cmd.status.status < NumDadErrs)
	    SImsg(SIM_DBG, "%s: ATA query failed: %s",
		  AtaCmd->DevFile, DadErrs[Cmd.status.status]);
	else
	    SImsg(SIM_DBG, "%s: ATA query failed with status %d.",
		  AtaCmd->DevFile, Cmd.status.status);
	return(-1);
    }

    AtaCmd->Data = (void *) Buff;
    return(0);
}
#endif	/* NOTYET */
