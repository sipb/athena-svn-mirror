/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

#ifndef lint
static char *RCSid = "$Id: os-hpux.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
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
    { GetKernArchHPUX1 },
    { GetKernArchHPUX2 },
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
extern char			       *GetVirtMemHPUX();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemHPUX },
    { NULL },
};
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeUtmp },
    { NULL },
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
 * HPUX Get Model name
 */
static char 			CpuSpeedSYM[] = "itick_per_tick";
#ifdef notdef	/* This works to */
static char 			CpuSpeedSYM[] = "iticks_per_10_msec";
#endif
extern char *GetModelHPUX()
{
    struct nlist	       *nlptr;
    static struct utsname	un;
    off_t			Addr;
    kvm_t		       *kd;
    int				Speed;
    char		       *Machine = NULL;
    static char			Model[100];

    if (Model[0])
	return(Model);

    if (uname(&un) != 0)
	return((char *) NULL);

    if (un.machine && un.machine[0])
	Machine = un.machine;

    /*
     * Get the CPU speed
     */
    if (!(kd = KVMopen()))
	return(Machine);
    if ((nlptr = KVMnlist(kd, CpuSpeedSYM, (struct nlist *)NULL, 0)) == NULL)
	return(Machine);
    if (CheckNlist(nlptr))
	return(Machine);
    if (KVMget(kd, (u_long) nlptr->n_value, (char *) &Speed, 
	       sizeof(Speed), KDT_DATA)) {
	if (Debug) Error("Cannot read `%s' from kernel.", CpuSpeedSYM);
	Speed = -1;
    }
    KVMclose(kd);

    if (Speed > 0)
	(void) sprintf(Model, "%s/%d", Machine, (Speed * 100) / MHERTZ);
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
	if (Debug) Error("sysconf(_SC_CPU_VERSION) failed: %s.", SYSERR);
	return((char *) NULL);
    }

    IOtype = sysconf(_SC_IO_TYPE);
    if (IOtype < 0) {
	if (Debug) Error("sysconf(_SC_IO_TYPE) failed: %s.", SYSERR);
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
	default:
	    if (Debug) Error("Unknown IOtype: %d.", IOtype);
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
    if (Status != 0) {
	if (Debug) Error("pstat(PSTAT_DYNAMIC) returned status %d.", Status);
	return((char *) NULL);
    }

    Num = pst.psd_proc_cnt;
    if (Num < 1) {
	if (Debug) Error("pstat(PSTAT_DYNAMIC) gave bad processor count (%d).",
			 Num);
	return((char *) NULL);
    }
    (void) sprintf(Buff, "%d", Num);

    return(Buff);
}

/*
 * Get size of virtual memory.
 */
static char			swdevtSYM[] = "swdevt";
static char			nswapdevSYM[] = "nswapdev";
extern char *GetVirtMemHPUX()
{
    struct nlist	       *nlptr;
    extern char			swdevtSYM[];
    struct swdevt		Swap;
    long			NumSwap;
    size_t			Amount = 0;
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
	if (Debug) Error("Cannot read `%s' from kernel.", nswapdevSYM);
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
	    if (Debug) Error("Cannot read `%s' at 0x%x", swdevtSYM, Addr);
	    continue;
	}

	Addr += sizeof(struct swdevt);

	if (Swap.sw_dev < 0 || !Swap.sw_enable)
	    continue;

#if OSMVER == 10
	Amount += (size_t) (Swap.sw_nblksavail * DEV_BSIZE);
#else
	Amount += (size_t) (Swap.sw_nblks * DEV_BSIZE);
#endif
    }

    KVMclose(kd);

    return(GetSizeStr((u_long)Amount, BYTES));
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
    long			Num;

    pstatbuff.pst_static = &pst;
    Status = pstat(PSTAT_STATIC, pstatbuff, sizeof(pst), 0, 0);
    if (Status != 0) {
	if (Debug) Error("pstat(PSTAT_STATIC) returned status %d.", Status);
	return((char *) NULL);
    }

    Num = pst.physical_memory * pst.page_size;
    if (Num < 1) {
	if (Debug) Error("pstat(PSTAT_STATIC) gave bad physical_memory (%d).",
			 Num);
	return((char *) NULL);
    }

    return(GetSizeStr((u_long)Num, BYTES));
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
