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
 * Linux specific functions
 */

#include "defs.h"
#include <sys/stat.h>
#include <linux/kernel.h>
#include <linux/sys.h>
#include <unistd.h>

/*
 * Platform Specific Interfaces
 */
extern char			       *GetModelProc();
PSI_t GetModelPSI[] = {			/* Get system Model */
    { GetModelProc },
    { GetModelFile },
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
extern char			       *GetCpuTypeProc();
PSI_t GetCpuTypePSI[] = {		/* Get CPU Type */
    { GetCpuTypeProc },
    { NULL },
};
PSI_t GetNumCpuPSI[] = {		/* Get Number of CPU's */
    { NULL },
};
extern char			       *GetKernVerProc();
extern char			       *GetKernVerLinux();
PSI_t GetKernVerPSI[] = {		/* Get Kernel Version */
    { GetKernVerProc },
    { GetKernVerLinux },
    { NULL },
};
PSI_t GetOSNamePSI[] = {		/* Get OS Name */
    { GetOSNameUname },
    { NULL },
};
extern char			       *GetOSDistLinux();
PSI_t GetOSDistPSI[] = {		/* Get OS Dist */
    { GetOSDistLinux },
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
extern char			       *GetMemorySysinfo();
extern char			       *GetMemoryKcore();
PSI_t GetMemoryPSI[] = {		/* Get amount of memory */
    { GetMemoryKcore },
    { GetMemorySysinfo },
    { NULL },
};
extern char			       *GetVirtMemLinux();
PSI_t GetVirtMemPSI[] = {		/* Get amount of virtual memory */
    { GetVirtMemLinux },
    { NULL },
};
extern char			       *GetBootTimeProc();
PSI_t GetBootTimePSI[] = {		/* Get System Boot Time */
    { GetBootTimeProc },
    { NULL },
};

/*
 * Get Boot Time by using the /proc/uptime file.
 */
extern char *GetBootTimeProc()
{
    FILE		       *fp;
    static char			Buff[64];
    char		       *cp;
    char		       *DateStr;
    time_t			Uptime;
    time_t			BootTime;

    fp = fopen(PROC_FILE_UPTIME, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: open readonly failed: %s", 
	      PROC_FILE_UPTIME, SYSERR);
	return((char *) NULL);
    }

    if (!fgets(Buff, sizeof(Buff), fp)) {
	SImsg(SIM_GERR, "%s: Read uptime failed: %s",
	      PROC_FILE_UPTIME, SYSERR);
	(void) fclose(fp);
	return((char *) NULL);
    }

    if (cp = strchr(Buff, ' '))
	*cp = CNULL;

    Uptime = (time_t) strtol(Buff, NULL, 0);
    if (Uptime <= 0) {
	SImsg(SIM_GERR, "Convert `%s' to long failed", Buff);
	(void) fclose(fp);
	return((char *) NULL);
    }

    BootTime = time(NULL);
    BootTime -= Uptime;

    DateStr = (char *) ctime(&BootTime);
    if (cp = strchr(DateStr, '\n'))
	*cp = CNULL;

    (void) fclose(fp);

    return(DateStr);
}

/*
 * Get System Model using the /proc/cpuinfo file.
 */
extern char *GetModelProc()
{
    FILE		       *fp;
    static char			Buff[256];
    char		       *Cpu = NULL;
    char		       *Vendor = NULL;
    char		      **Argv;
    int				Argc;

    if (Buff[0])
	return(Buff);

    fp = fopen(PROC_FILE_CPUINFO, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: open readonly failed: %s", 
	      PROC_FILE_CPUINFO, SYSERR);
	return((char *) NULL);
    }

    while (fgets(Buff, sizeof(Buff), fp)) {
	Argc = StrToArgv(Buff, ":", &Argv, NULL, 0);
	if (Argc < 2)
	    continue;
	if (EQ(Argv[0], "cpu"))
	    Cpu = Argv[1];
	else if (EQ(Argv[0], "vendor_id"))
	    Vendor = Argv[1];
	if (Cpu && Vendor)
	    break;
    }

    Buff[0] = CNULL;
    if (Vendor)
	(void) strcpy(Buff, Vendor);
    if (Cpu) {
	if (Buff[0]) {
	    strcat(Buff, " ");
	    strcat(Buff, Cpu);
	} else
	    strcpy(Buff, Cpu);
    }

    (void) fclose(fp);

    return(Buff);
}

/*
 * Get System Memory using size of /proc/kcore
 */
extern char *GetMemoryKcore()
{
    static char		       *MemStr = NULL;
    Large_t			MemBytes = 0;
    Large_t			Amount = 0;
    char			Kcore[] = "/proc/kcore";
    struct stat			StatBuf;

    if (MemStr)
	return(MemStr);

    if (stat(Kcore, &StatBuf) != 0) {
	SImsg(SIM_GERR, "%s: stat failed: %s", Kcore, SYSERR);
	return((char *) NULL);
    }

    MemBytes = (Large_t) (StatBuf.st_size - 4096);
    Amount = DivRndUp(MemBytes, (Large_t)MBYTES);
    MemStr = GetMemoryStr(Amount);
    
    return(MemStr);
}

/*
 * Get System Memory using sysinfo() system call
 */
extern char *GetMemorySysinfo()
{
    struct sysinfo		si;
    static char		       *MemStr = NULL;
    Large_t			MemBytes = 0;
    Large_t			Amount = 0;

    if (MemStr)
	return(MemStr);

    if (sysinfo(&si) != 0) {
	SImsg(SIM_GERR, "sysinfo() system call failed: %s", SYSERR);
	return((char *) NULL);
    }

    /*
     * sysinfo.totalram represents total USABLE physical memory.  Memory
     * reserved by the kernel is not included.  So this is as close as we
     * can get for now.  Sigh!
     */
    MemBytes = (Large_t) si.totalram;
    Amount = DivRndUp(MemBytes, (Large_t)MBYTES);
    MemStr = GetMemoryStr(Amount);
    
    return(MemStr);
}

/*
 * Get Virtual Memory using sysinfo() system call
 */
extern char *GetVirtMemLinux()
{
    struct sysinfo		si;
    static char		       *MemStr = NULL;
    Large_t			MemBytes = 0;
    Large_t			Amount = 0;

    if (MemStr)
	return(MemStr);

    if (sysinfo(&si) != 0) {
	SImsg(SIM_GERR, "sysinfo() system call failed: %s", SYSERR);
	return((char *) NULL);
    }

    MemBytes = (Large_t) (si.totalram + si.totalswap);
    Amount = DivRndUp(MemBytes, (Large_t)MBYTES);
    MemStr = GetMemoryStr(Amount);
    
    return(MemStr);
}

/*
 * Get CPU Type from /proc/cpuinfo
 */
extern char *GetCpuTypeProc()
{
    FILE		       *fp;
    static char			Buff[256];
    static char		       *Cpu = NULL;
    char		      **Argv;
    int				Argc;

    if (Cpu)
	return(Cpu);

    fp = fopen(PROC_FILE_CPUINFO, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: open readonly failed: %s", 
	      PROC_FILE_CPUINFO, SYSERR);
	return((char *) NULL);
    }

    while (fgets(Buff, sizeof(Buff), fp)) {
	Argc = StrToArgv(Buff, ":", &Argv, NULL, 0);
	if (Argc < 2)
	    continue;
	if (EQ(Argv[0], "cpu")) {
	    Cpu = Argv[1];
	    break;
	}
    }

    (void) fclose(fp);

    return(Cpu);

}

/*
 * Get Kernel Version string using /proc/version
 */
extern char *GetKernVerProc()
{
    FILE		       *fp;
    static char			Buff[512];
    char		      **Argv;
    int				Argc;

    if (Buff[0])
	return(Buff);

    fp = fopen(PROC_FILE_VERSION, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: open readonly failed: %s", 
	      PROC_FILE_VERSION, SYSERR);
	return((char *) NULL);
    }

    if (!fgets(Buff, sizeof(Buff), fp)) {
	SImsg(SIM_GERR, "%s: read failed: %s", PROC_FILE_VERSION, SYSERR);
	return((char *) NULL);
    }

    (void) fclose(fp);

    return(Buff);

}

/*
 * Get Kernel Version string using uname()
 */
extern char *GetKernVerLinux()
{
    static struct utsname	uts;
    static char		       *VerStr = NULL;

    if (uname(&uts) != 0) {
	SImsg(SIM_GERR, "uname() system call failed: %s", SYSERR);
	return((char *) NULL);
    }

    VerStr = uts.version;

    return(VerStr);
}

/*
 * What distribution (vendor) of Linux are we?
 * This code works on S.u.S.E. Linux.  Don't know about others.
 */
extern char *GetOSDistLinux()
{
    static char			Buff[256];
    register char	       *cp;
    register char	       *End;
    char			IssueFile[] = "/etc/issue";
    char			Welcome[] = "Welcome to ";
    FILE		       *fp;
    int				Found = FALSE;

    if (!(fp = fopen(IssueFile, "r"))) {
	SImsg(SIM_GERR, "%s: Cannot open to get OS Dist: %s",
	      IssueFile, SYSERR);
	return((char *) NULL);
    }

    while (fgets(Buff, sizeof(Buff), fp)) {
	for (cp = Buff; cp && *cp && *cp != '\n' && !isalpha(*cp); ++cp);
	if (*cp == '\n' || !strlen(cp))
	    continue;
	/* Found first nonblank line */
	Found = TRUE;
	break;
    }

    (void) fclose(fp);

    if (!Found)
	return((char *) NULL);

    if (EQN(cp, Welcome, sizeof(Welcome)-1))
	cp += sizeof(Welcome)-1;

    if (End = strchr(cp, '-')) {
	--End;
	while (*End && isspace(*End))
	    --End;
	*++End = CNULL;
    }

    return(cp);
}
