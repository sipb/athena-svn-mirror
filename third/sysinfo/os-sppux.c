/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-sppux.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
#endif

/*
 * Convex SPP-UX specific functions
 */

#include "defs.h"
#include <utmp.h>
#include <sys/cnx_sysinfo.h>
#include <sys/libio.h>
#include <sys/conf.h>

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelCnxUname();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelCnxUname },
    { GetModelFile },
    { GetModelDef },
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
PSI_t GetKernArchPSI[] = {		/* Get Kernel Architecture */
    { GetKernArchDef },
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
extern char			       *GetNumCpuCnxSysinfo();
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { GetNumCpuCnxSysinfo },
    { NULL },
};
extern char			       *GetKernVerCnxSysinfo();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerCnxSysinfo },
    { NULL },
};
extern char			       *GetOSNameCnxUname();
extern char			       *GetOSNameCnxSysinfo();
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameCnxUname },
    { GetOSNameCnxSysinfo },
    { NULL },
};
extern char			       *GetOSVerCnxSysinfo();
PSI_t GetOSVerPSI[] = {			/* Get OS Version */
    { GetOSVerCnxSysinfo },
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
extern char			       *GetMemoryCnxSysinfo();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryCnxSysinfo },
    { GetMemoryPhysmemSym },
    { NULL },
};
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
#if	defined(BOOT_TIME)
    { GetBootTimeUtmp },
#endif	/* BOOT_TIME */
    { NULL },
};

/*
 * Get system Model type using cnx_uname()
 */
extern char *GetModelCnxUname()
{
    static struct utsname	un;
    static char			Buff[BUFSIZ];

    if (cnx_uname(&un) != 0)
	return((char *) NULL);

#if	defined(MODEL_NAME)
    (void) sprintf(Buff, "%s ", MODEL_NAME);
#else
    Buff[0] = CNULL;
#endif
    (void) strcat(Buff, un.machine);

    return(Buff);
}

/*
 * Get Serial Number
 */
extern char *GetSerialUname()
{
    static struct utsname	un;

    if (cnx_uname(&un) != 0)
	return((char *) NULL);

    return( (un.idnumber[0]) ? un.idnumber : (char *)NULL );
}

/*
 * Determine system application architecture using sysconf().
 */
extern char *GetAppArchHPUX()
{
    long			CpuVersion;

    CpuVersion = sysconf(_SC_CPU_VERSION);
    if (CpuVersion < 0) {
	if (Debug) Error("sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
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
	if (Debug) Error("sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    Def = DefGet(DL_CPU, NULL, CpuVersion, 0);
    if (Def && Def->ValStr1)
	return(Def->ValStr1);

    return((char *) NULL);
}

/*
 * Get Number of CPU's using cnx_sysinfo()
 */
extern char *GetNumCpuCnxSysinfo()
{
    cnx_is_complex_basic_info_data_t 
				Basic;
    cnx_is_target_data_t	Target;
    int				Status;
    static char			Buff[10];
    unsigned long		Num;

    cnx_sysinfo_target_complex(&Target);
    Status = cnx_sysinfo(CNX_IS_COMPLEX_BASIC_INFO, &Target, &Basic,
			 1, CNX_IS_COMPLEX_BASIC_INFO_COUNT, NULL);
    if (Status != 1) {
	if (Debug) 
	    Error("cnx_sysinfo(CNX_IS_COMPLEX_BASIC_INFO) failed: %s", SYSERR);
	return((char *) NULL);
    }

    Num = Basic.cpu_cnt;
    if (Num < 1) {
	if (Debug)
	    Error("cnx_sysinfo(CNX_IS_COMPLEX_BASIC_INFO): Bad cpu count (%d)",
		  Num);
	return((char *) NULL);
    }
    (void) sprintf(Buff, "%d", Num);

    return(Buff);
}

/*
 * Get kernel version string using cnx_sysinfo()
 */
extern char *GetKernVerCnxSysinfo()
{
    cnx_is_complex_vers_info_data_t 
				Version;
    cnx_is_target_data_t	Target;
    int				Status;
    static char		       *VerString = NULL;
    char		       *cp;

    if (VerString)
	return(VerString);

    cnx_sysinfo_target_complex(&Target);
    Status = cnx_sysinfo(CNX_IS_COMPLEX_VERS_INFO, &Target, &Version,
			 1, CNX_IS_COMPLEX_VERS_INFO_COUNT, NULL);
    if (Status != 1) {
	if (Debug) 
	    Error("cnx_sysinfo(CNX_IS_COMPLEX_VERS_INFO) failed: %s", SYSERR);
	return((char *) NULL);
    }

    cp = Version.server_version;
    if (cp && *cp)
	VerString = strdup(cp);
    else {
	if (Debug)
	    Error("cnx_sysinfo(CNX_IS_COMPLEX_VERS_INFO): No version string.");
	return((char *) NULL);
    }

    return(VerString);
}

/*
 * Get OS name using cnx_uname()
 */
extern char *GetOSNameCnxUname()
{
    static struct utsname	un;

    if (cnx_uname(&un) != 0)
	return((char *) NULL);

    return((un.sysname[0]) ? un.sysname : (char *) NULL);
}

/*
 * Get our OS name using cnx_sysinfo()
 */
extern char *GetOSNameCnxSysinfo()
{
    static char			OSName[100];
    char		       *KernStr;
    register char	       *cp;

    if (OSName[0])
	return(OSName);

    KernStr = GetKernVerCnxSysinfo();
    if (!KernStr)
	return((char *) NULL);
    /*
     * Find the end of the first argument which should
     * be the OS name.
     */
    cp = strchr(KernStr, '_');
    if (!cp) {
	cp = strchr(KernStr, ' ');
	if (!cp)
	    return((char *) NULL);
    }

    if (cp - KernStr > sizeof(OSName)) {
	if (Debug) Error("GetOSNameCnx(): OSName buffer too small.");
	return((char *) NULL);
    }

    (void) strncpy(OSName, KernStr, cp - KernStr);
    OSName[cp - KernStr] = CNULL;

    return(OSName);
}

/*
 * Get our OS version using cnx_sysinfo()
 */
extern char *GetOSVerCnxSysinfo()
{
    static char			OSVer[100];
    char		       *KernStr;
    register char	       *cp;

    if (OSVer[0])
	return(OSVer);

    KernStr = GetKernVerCnxSysinfo();
    if (!KernStr)
	return((char *) NULL);

    /*
     * Find the start of the version number
     */
    while (KernStr && *KernStr && !isdigit(*KernStr))
	++KernStr;

    /*
     * Find the end of the first argument which should
     * be the OS name.
     */
    cp = strchr(KernStr, ' ');
    if (!cp)
	return((char *) NULL);

    if (cp - KernStr > sizeof(OSVer)) {
	if (Debug) Error("GetOSVerCnxSysinfo(): OSVer buffer too small.");
	return((char *) NULL);
    }

    (void) strncpy(OSVer, KernStr, cp - KernStr);
    OSVer[cp - KernStr] = CNULL;

    return(OSVer);
}

/*
 * Get amount of physical memory using cnx_sysinfo()
 */
extern char *GetMemoryCnxSysinfo()
{
    cnx_is_complex_mem_info_data_t 
				Mem;
    cnx_is_target_data_t	Target;
    u_long			Num;
    int				Status;
    static char			Buff[100];

    cnx_sysinfo_target_complex(&Target);
    Status = cnx_sysinfo(CNX_IS_COMPLEX_MEM_INFO, &Target, &Mem,
			 1, CNX_IS_COMPLEX_MEM_INFO_COUNT, NULL);
    if (Status != 1) {
	if (Debug) 
	    Error("cnx_sysinfo(CNX_IS_COMPLEX_MEM_INFO) failed: %s", SYSERR);
	return((char *) NULL);
    }

    Num = Mem.total_memory;

    return(GetSizeStr(Num, BYTES));
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
#ifdef notdef
    register int		i;

    for (i = 0; DevTypes[i].Name; ++i)
	switch (DevTypes[i].Type) {
	case DT_DISKDRIVE:	DevTypes[i].Probe = ProbeDiskDrive;	break;
	case DT_FRAMEBUFFER:	DevTypes[i].Probe = ProbeFrameBuffer;	break;
	case DT_KEYBOARD:	DevTypes[i].Probe = ProbeKbd;		break;
	case DT_NETIF:		DevTypes[i].Probe = ProbeNetif;		break;
	case DT_TAPEDRIVE:	DevTypes[i].Probe = ProbeTapeDrive;	break;
#if	defined(HAVE_OPENPROM)
	case DT_CPU:		DevTypes[i].Probe = OBPprobeCPU;	break;
#endif	/* HAVE_OPENPROM */
	}
#endif
}
