/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
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
#include <math.h>

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
    { GetNumCpuSysconf },
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

struct stat 			Stat;

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

    DateStr = TimeToStr(BootTime, NULL);

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
    char		       *Model = NULL;
    char                        Speed[64];
    float			sp = 0;
    char		      **Argv;
    int				Argc;
    int				Cleanup;

    if (Buff[0])
	return(Buff);

    fp = fopen(PROC_FILE_CPUINFO, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: open readonly failed: %s", 
	      PROC_FILE_CPUINFO, SYSERR);
	return((char *) NULL);
    }

    Speed[0] = CNULL;
    while (fgets(Buff, sizeof(Buff), fp)) {
	Cleanup = TRUE;
	Argc = StrToArgv(Buff, ":", &Argv, NULL, 0);
	if (Argc < 2)
	    continue;
	if (EQ(Argv[0], "cpu"))
	    Cpu = Argv[1];
	else if (EQ(Argv[0], "vendor_id"))
	    Vendor = Argv[1];
	else if (EQ(Argv[0], "model name"))
	    Model = Argv[1];
	else if (EQ(Argv[0], "cpu MHz")) {
	    /* 
	     * This value is not always perfectly accurate as Linux estimates
	     * the actual Mhz by doing a loop test at boot.
	     */
	    if (sscanf(Argv[1], "%f", &sp))	
	(void) sprintf(Speed, "%.0f Mhz", rint((double)sp));
	} else {
	    DestroyArgv(&Argv, Argc);
	    Cleanup = FALSE;
	}

	if (Cleanup)
	    (void) free(Argv[0]);
    }

    Buff[0] = CNULL;
    if (Vendor) {
	(void) strcpy(Buff, Vendor);
	(void) free(Vendor);
    }
    if (Speed[0]) {
	if (Buff[0]) (void) strcat(Buff, " ");
	(void) strcat(Buff, Speed);
    }
    if (Model) {
	if (Buff[0]) strcat(Buff, " ");
	strcat(Buff, Model);
	(void) free(Model);
    }
    if (Cpu) {
	if (Buff[0]) strcat(Buff, " ");
	strcat(Buff, Cpu);
	(void) free(Cpu);
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
    char		       *cp;
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
	    /* Linux 2.0 */
	    Cpu = Argv[1];
	    break;
	} else if (EQ(Argv[0], "model name")) {
	    /* Linux 2.2 */
	    Cpu = Argv[1];
	    if (cp = strchr(Cpu, ' '))
		*cp = CNULL;
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
	/* Some distributions have VGA control chars in them */
	if (!isalpha(Buff[0]))
	    continue;
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
    else if (EQN(cp, "Linux ", 6))
	cp += 6;

    if (End = strchr(cp, '-')) {
	--End;
	while (*End && isspace(*End))
	    --End;
	*++End = CNULL;
    }

    return(cp);
}

/*
 * Build Software Information tree
 */
extern int BuildSoftInfo(SoftInfoTree, SearchExp)
     SoftInfo_t		      **SoftInfoTree;
     char		      **SearchExp;
{
    return BuildSoftInfoRPM(SoftInfoTree, SearchExp);
}

#if	defined(HAVE_DEVICE_SUPPORT)

#include <linux/pci.h>	/* For PCI_* */
#include "myscsi.h"
#if	defined(HAVE_SCSI_SCSI_H)
#	include <scsi/scsi.h>
#else	/* ! HAVE_SCSI_SCSI_H */
# 	if 	defined(HAVE__USR_SRC_LINUX_INCLUDE_SCSI_SCSI_H)
#		include "/usr/src/linux/include/scsi/scsi.h"
#	endif	/* HAVE__USR_SRC_LINUX_INCLUDE_SCSI_SCSI_H */
#endif	/* HAVE_SCSI_SCSI_H */
#if	defined(HAVE_SCSI_SG_H)
#	include <scsi/sg.h>
#	define HAVE_SCSI_SUPPORT
#else	/* !HAVE_SCSI_SCSI_H */
# 	if 	defined(HAVE__USR_SRC_LINUX_INCLUDE_SCSI_SG_H)
#		include "/usr/src/linux/include/scsi/sg.h"
#		define HAVE_SCSI_SUPPORT
#	endif	/* HAVE__USR_SRC_LINUX_INCLUDE_SCSI_SG_H */
#endif	/* HAVE_SCSI_SCSI_H */
#include "myata.h"
#include <linux/hdreg.h>
#include <sys/vfs.h>

/*
 * Get FS usage for Part
 */
static int GetFileSysUsage(Part)
     PartInfo_t		       *Part;
{
    struct statfs		Statfs;

    if (!Part || !Part->MntName)
	return -1;

    if (statfs(Part->MntName, &Statfs) == 0) {
	Part->AmtUsed = (Large_t) ((Statfs.f_blocks - Statfs.f_bfree) * 
				   Statfs.f_bsize);
    }

    return 0;
}

/*
 * Mount list info
 */
struct _MntList {
    char		       *Device;		/* Device name */
    char		       *Dir;		/* Dir mounted on */
    char		       *Type;		/* fs type */
    char		       *Opts;		/* Options */
    struct _MntList	       *Next;
};
typedef struct _MntList		MntList_t;

/*
 * Get a list of mounted filesystems from File and add it to the
 * list at location MntListPtr.
 */
static int GetMountList(File, MntListPtr)
     char		       *File;
     MntList_t		      **MntListPtr;
{
    FILE		       *fp;
    MntList_t		       *Last = NULL;
    MntList_t		       *NewList;
    MntList_t		       *MntList = NULL;
    struct mntent	       *Ent;

    if (!File || !MntListPtr)
	return -1;

    fp = setmntent(File, "r");
    if (!fp) {
	SImsg(SIM_GERR, "%s: Cannot open: %s", File, SYSERR);
	return -1;
    }

    /*
     * Find the end of MntListPtr and set Last
     */
    for (MntList = *MntListPtr; MntList; ) {
	if (!MntList->Next)
	    Last = MntList;
	MntList = MntList->Next;
    }
	
    while (Ent = getmntent(fp)) {
	NewList = (MntList_t *) xcalloc(1, sizeof(MntList_t));
	NewList->Device = strdup(Ent->mnt_fsname);
	NewList->Dir = strdup(Ent->mnt_dir);
	NewList->Type = strdup(Ent->mnt_type);
	NewList->Opts = strdup(Ent->mnt_opts);

	if (!Last) 
	    MntList = Last = NewList;
	else {
	    Last->Next = NewList;
	    Last = NewList;
	}
    }

    (void) endmntent(fp);

    if (!*MntListPtr)
	*MntListPtr = MntList;

    return 0;
}

/*
 * Get Mount info for partition Part
 */
static int GetPartUsage(DevName, DiskPart)
     char		       *DevName;
     DiskPart_t		       *DiskPart;
{
    static MntList_t   	       *MntList;
    MntList_t       	       *Mnt;
    static char			Name[128];
    PartInfo_t		       *Part = NULL;
    char		      **Argv;

    if (!DevName || !DiskPart || !DiskPart->PartInfo)
	return -1;

    Part = DiskPart->PartInfo;

    if (!MntList) {
	/*
	 * Build list of mount entries ONCE
	 */
	/*
	 * We first look in _PATH_MOUNTED (/etc/mtab) to see what's
	 * currently mounted.  Then we look in _PATH_MNTTAB (/etc/fstab)
	 * to see what may be unmounted.
	 */
	(void) GetMountList(_PATH_MOUNTED, &MntList);
	(void) GetMountList(_PATH_MNTTAB, &MntList);
    }

    (void) snprintf(Name, sizeof(Name), "%s%s%d", 
		    _PATH_DEV, DevName, Part->Num);

    Part->DevPath = strdup(Name);

    for (Mnt = MntList; Mnt; Mnt = Mnt->Next)
	if (eq(Mnt->Device, Name)) {
	    Part->Usage = PIU_FILESYS;
	    Part->MntName = Mnt->Dir;
	    Part->Type = Mnt->Type;
	    if (Mnt->Opts && StrToArgv(Mnt->Opts, ",", &Argv, NULL, 0) > 0)
		Part->MntOpts = Argv;
	    (void) GetFileSysUsage(Part);
	    break;
	}

    return 0;
}

/*
 * Get Partition information
 */
static int GetPart(DevName, DiskDrive)
     char		       *DevName;
     DiskDrive_t	       *DiskDrive;
{
    DiskPart_t		       *Part;
    MntList_t		       *Mnt;

    if (!DevName || !DiskDrive)
	return -1;

    for (Part = DiskDrive->DiskPart; Part; Part = Part->Next)
	GetPartUsage(DevName, Part);

    return 0;
}

/*
 * Read one line of a file
 */
static char *GetOneLine(File)
     char		       *File;
{
    FILE		       *Fp;
    static char			Buff[1024];
    char		       *cp;

    if (Fp = fopen(File, "r")) {
	if (fgets(Buff, sizeof(Buff), Fp)) {
	    if (cp = strchr(Buff, '\n'))
		*cp = CNULL;
	    return Buff;
	}
	(void) fclose(Fp);
    }

    return (char *) NULL;
}

/*
 * Probe a DiskDrive device
 */
extern DevInfo_t *ProbeDiskDrive(ProbeData)
     ProbeData_t	       *ProbeData;
{
    DevInfo_t		       *DevInfo = NULL;
    DiskDrive_t		       *Disk = NULL;
    DiskDriveData_t	       *DiskDriveData = NULL;
    FILE		       *Fp;
    static DosPartQuery_t	Query;
    static char		      **Argv;
    int				Argc;
    char		       *DevFile;
    static char			Buff[512];
    static char			Dir[128];
    static char			File[128];
    char		       *cp;

    if (!ProbeData || !ProbeData->DevName || 
	!ProbeData->UseDevInfo ||
	!ProbeData->DevFile || ProbeData->FileDesc <= 0)
	return((DevInfo_t *)NULL);

    DevInfo = ProbeData->UseDevInfo;
    DevFile = ProbeData->DevFile;

    if (DevInfo->DevSpec)
	DiskDriveData = (DiskDriveData_t *) DevInfo->DevSpec;
    else {
	DiskDriveData = NewDiskDriveData(NULL);
	DevInfo->DevSpec = (void *) DiskDriveData;
    }

    /*
     * Linux "partitions" are really just DOS partitions.
     * First we do our standard get DOS partitions, then we
     * add usage info to it.
     */
    (void) memset(&Query, 0, sizeof(Query));
    Query.DevInfo = DevInfo;
    Query.CtlDev = DevFile;
    (void) DosPartGet(&Query);
    (void) GetPart(DevInfo->Name, 
		   (DiskDriveData->HWdata) ? DiskDriveData->HWdata :
		   DiskDriveData->OSdata);

    /*
     * This stuff obviously only works on ATA (IDE) drives
     */
    (void) snprintf(Dir, sizeof(Dir), "/proc/ide/%s", ProbeData->DevName);
    (void) snprintf(File, sizeof(File), "%s/capacity", Dir);
    if (cp = GetOneLine(File)) {
	Large_t			Size;

	if (!DiskDriveData->HWdata)
	    DiskDriveData->HWdata = NewDiskDrive(NULL);
	if (!DiskDriveData->OSdata)
	    DiskDriveData->OSdata = NewDiskDrive(NULL);

	if (!DiskDriveData->OSdata->Size) {
	    Size = atoL(cp);
	    DiskDriveData->OSdata->Size = (float) (((float)Size / (float)2) / 
						   (float)1024);
	}

	/*
	 * Read the geometry
	 */
	(void) snprintf(File, sizeof(File), "%s/geometry", Dir);
	if (Fp = fopen(File, "r")) {
	    while (fgets(Buff, sizeof(Buff), Fp)) {
		if (cp = strchr(Buff, '\n'))
		    *cp = CNULL;
		if ((cp = strchr(Buff, ' ')) || (cp = strchr(Buff, '\t'))) {
		    *cp = CNULL;
		    ++cp;
		}
		if (EQ(Buff, "physical"))
		    Disk = DiskDriveData->HWdata;
		else if (EQ(Buff, "logical"))
		    Disk = DiskDriveData->OSdata;
		else {
		    SImsg(SIM_DBG, "The word <%s> from %s is unknown.",
			  Buff, File);
		    continue;
		}

		if ((Argc = StrToArgv(cp, "/", &Argv, NULL, 0)) &&
		    Argc == 3) {
		    Disk->DataCyl = (u_long) atol(Argv[0]);
		    Disk->Tracks = (u_long) atol(Argv[1]);
		    Disk->Sect = (u_long) atol(Argv[2]);
		}

	    }
	    (void) fclose(Fp);
	}
    }

    return(ProbeData->RetDevInfo = DevInfo);
}

/*
 * Issue a SCSI command and return the results.  This function uses
 * the SCSI Generic (sg) interface.  See 
 * 	http://www.linuxdoc.org/HOWTO/SCSI-Programming-HOWTO.html
 */
extern int ScsiCmd(ScsiCmd)
     ScsiCmd_t		       *ScsiCmd;
{
#if	defined(HAVE_SCSI_SUPPORT)
#define SG_OFFSET		sizeof(struct sg_header)
    static char			Buff[SG_OFFSET + SCSI_BUF_LEN];
    int				Status;
    int				Timeout;
    ScsiCdbG0_t		       *CdbPtr = NULL;
    struct sg_header		ScsiHeader;

    if (!ScsiCmd || !ScsiCmd->Cdb || !ScsiCmd->CdbLen || 
	ScsiCmd->DevFD < 0 || !ScsiCmd->DevFile) {
	SImsg(SIM_DBG, "ScsiCmd: Bad parameters.");
	return -1;
    }

    /*
     * Set the timeout
     */
    Timeout = MySCSI_CMD_TIMEOUT * 1000;	/* timeout == milliseconds */
    if (ioctl(ScsiCmd->DevFD, SG_SET_TIMEOUT, &Timeout) < 0) {
	SImsg(SIM_DBG, "%s: ScsiCmd: Set SG Timeout to %d msecs failed: %s",
	      ScsiCmd->DevFile, Timeout, SYSERR);
    }

    (void) memset(Buff, 0, sizeof(Buff));

    /*
     * SCSI Header for sg
     */
    (void) memset(&ScsiHeader, 0, sizeof(ScsiHeader));
    ScsiHeader.reply_len = sizeof(Buff);
    ScsiHeader.twelve_byte = ScsiCmd->CdbLen == 12;
    (void) memcpy(Buff, &ScsiHeader, sizeof(ScsiHeader));

    /*
     * Add Command Data Block
     */
    (void) memcpy(Buff + SG_OFFSET, ScsiCmd->Cdb, ScsiCmd->CdbLen);

    /* 
     * We just need Cdb.cmd so it's ok to assume ScsiCdbG0_t here
     */
    CdbPtr = (ScsiCdbG0_t *) ScsiCmd->Cdb;

    /*
     * Issue SCSI Command
     */
    Status = write(ScsiCmd->DevFD, Buff, SG_OFFSET + ScsiCmd->CdbLen);
    if (Status < 0 || Status != SG_OFFSET + ScsiCmd->CdbLen || 
	ScsiHeader.result) {
	SImsg(SIM_GERR, 
	      "%s: SCSI command issue 0x%x failed: %s",
	      ScsiCmd->DevFile, CdbPtr->Cmd, SYSERR);
	return -1;
    }

    /*
     * Read results of command
     */
    Status = read(ScsiCmd->DevFD, Buff, sizeof(Buff));
    if (Status < 0) {
	SImsg(SIM_GERR, 
	      "%s: SCSI command read command 0x%x failed: %s",
	      ScsiCmd->DevFile, CdbPtr->Cmd, SYSERR);
	return -1;
    }
	
    ScsiCmd->Data = (void *) Buff + SG_OFFSET;

#endif	/* HAVE_SCSI_SUPPORT */

    return 0;
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
    DevInfo_t		     *(*ProbeFunc)() = NULL;
    DevList_t		       *DevList;
    static ProbeData_t		ProbeData;
    static char			DevName[64];
    static char			File[128];
    static int			CDUnits;	/* # CDs found */
    static int			DiskUnits;	/* # Disks found */
    static int			TapeUnits;	/* # Tapes found */
    int				fd;
    struct Scsi_Id {
	long	l1; /* target | lun << 8 | channel << 16 | low_ino << 24 */
	long	l2; /* Unique id */
    } 				Scsi_Id;

    (void) snprintf(DevName, sizeof(DevName), "%s%c", Name, Unit + 'a');
    (void) snprintf(File, sizeof(File), "%s%s", _PATH_DEV, DevName);

    /*
     * Only use O_RDWR as other flags affect sg driver
     */
    fd = open(File, O_RDWR);
    if (fd < 0) {
	SImsg(SIM_DBG, "%s: open %s failed: %s", DevName, File, SYSERR);
	if (errno != EBUSY)
	    return((DevList_t *) NULL);
    }

    SImsg(SIM_DBG, "%s:\tSCSI Probe: Device Exists", File);

    /*
     * Create the DevInfo
     */
    DevInfo = NewDevInfo(NULL);
    DevInfo->Name = strdup(DevName);
    DevInfo->Driver = strdup(Name);
    DevInfo->ClassType = CT_SCSI;
    DevInfo->Type = DevType;
    DevAddFile(DevInfo, strdup(File));

    /*
     * Crate DevList and return it.
     */
    DevList = (DevList_t *) xcalloc(1, sizeof(DevList_t));
    DevList->Name = DevInfo->Name;
    DevList->File = DevInfo->Files[0];
    DevList->DevInfo = DevInfo;

    /*
     * Identify SCSI specific ID of this device now
     */
    if (ioctl(fd, SCSI_IOCTL_GET_BUS_NUMBER, &DevList->Bus) < 0) {
	SImsg(SIM_DBG, "%s: ioctl to GET_BUS_NUMER failed: %s",
	      File, SYSERR);
    }
    if (ioctl(fd, SCSI_IOCTL_GET_IDLUN, &Scsi_Id) < 0) {
	SImsg(SIM_DBG, "%s: ioctl to GET_IDLUN failed: %s",
	      File, SYSERR);
    }
    DevInfo->Unit = DevList->Unit = Scsi_Id.l1 & 0xFF;
    DevList->Lun = (Scsi_Id.l1 >> 8L) & 0xFF;

    /*
     * Do full SCSI query of this device
     */
    (void) ScsiQuery(DevInfo, File, fd, TRUE);

    /*
     * Now that Type is known, set the canonical device name properly.
     */
    DevName[0] = CNULL;
    switch (DevInfo->Type) {
    case DT_DISKDRIVE:
	(void) snprintf(DevName, sizeof(DevName), "sd%c", (DiskUnits++) + 'a');
	ProbeFunc = ProbeDiskDrive;
	break;
    case DT_TAPEDRIVE:
	(void) snprintf(DevName, sizeof(DevName), "st%d", TapeUnits++);
	break;
    case DT_CD:
	(void) snprintf(DevName, sizeof(DevName), "sr%d", CDUnits++);
	break;
    }
    if (DevName[0]) {
	DevInfo->AltName = DevInfo->Name;
	DevInfo->Name = strdup(DevName);
	(void) snprintf(File, sizeof(File), "%s%s", _PATH_DEV, DevName);
	DevAddFile(DevInfo, strdup(File));
    }
    if (ProbeFunc) {
        (void) memset(&ProbeData, CNULL, sizeof(ProbeData));
	ProbeData.DevName = DevInfo->Name;
	ProbeData.DevFile = File;
	ProbeData.FileDesc = fd;
	ProbeData.UseDevInfo = DevInfo;
        (void) (*ProbeFunc)(&ProbeData);
    }

    (void) close(fd);

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
    static DevList_t	       *Last;
    register DevList_t	       *New = NULL;

    if (ScsiDevices)
	return(ScsiDevices);

    /*
     * Look for SCSI devices via generic SG driver
     */
    for (u = 0; u < MAX_SCSI_UNITS; ++u) {
	if (New = ScsiProbe("sg", u, 0)) {
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
 * Issue an ATA command and return the results.
 * This function uses the HDIO_DRIVE_CMD ioctl to issue raw ATA
 * commands to the device.
 */
extern int AtaCmd(AtaCmd)
     AtaCmd_t		       *AtaCmd;
{
#if	defined(HDIO_DRIVE_CMD)
    static char			Buff[HDCMD_OFFSET + ATA_BUF_LEN];
    static HDcmd_t		HDcmd;

    if (!AtaCmd || !AtaCmd->Cdb || AtaCmd->DevFD < 0 || !AtaCmd->DevFile) {
	SImsg(SIM_DBG, "AtaCmd: Bad parameters.");
	return -1;
    }

    (void) memset(&HDcmd, 0, sizeof(HDcmd));
    HDcmd.Cmd = AtaCmd->Cdb->Cmd;
    /*
     * Add the size of this command + the expected data length, divide by
     * 512 (1 sector) and round up the result.
     */
    HDcmd.SectLen = rint( (HDCMD_OFFSET + AtaCmd->DataLen) / 512);

    (void) memcpy(Buff, &HDcmd, HDCMD_OFFSET);

    if (ioctl(AtaCmd->DevFD, HDIO_DRIVE_CMD, Buff) < 0) {
	SImsg(SIM_DBG, "%s: AtaCmd command %s failed: %s",
	      AtaCmd->DevFile, AtaCmd->CmdName, SYSERR);
	return -1;
    }

    AtaCmd->Data = (void *) (Buff + HDCMD_OFFSET);

    return 0;
#else	/* !HDIO_DRIVE_CMD */
    return -1;
#endif	/* HDIO_DRIVE_CMD */
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
    DevList_t		       *DevList = NULL;
    DevInfo_t		     *(*ProbeFunc)() = NULL;
    static ProbeData_t		ProbeData;
    static char			DevName[64];
    static char			Dir[128];
    static char			File[128];
    static Query_t		Query;
    static AtaIdent_t		AtaIdent;
    static struct hd_driveid	DriveID;
    int				fd = -1;
    char		       *cp;

    (void) snprintf(DevName, sizeof(DevName), "%s%c", Name, 'a' + Unit);
    (void) snprintf(File, sizeof(File), "%s%s", _PATH_DEV, DevName);

    /*
     * All we can do right now is see if open() succeeds.
     * We really need a means to issue ATA READ PARAMETERS (AtaParam_t)
     */
    fd = open(File, O_RDONLY|O_NONBLOCK);
    if (fd >= 0) {
#if	defined(HAVE_ATA)
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
	 * Perform ATA Queries
	 */
	(void) AtaQuery(DevInfo, File, fd, TRUE);

#if	defined(USE_HDIO_GET_IDENTITY)
	/* 
	 * We no longer use this, but use the direct HDIO_DRIVE_CMD command
	 * in place of HDIO_GET_IDENTIFY.
	 */
	(void) memset(&DriveID, CNULL, sizeof(DriveID));
 
	if (ioctl(fd, HDIO_GET_IDENTITY, &DriveID) < 0) {
	    SImsg(SIM_DBG, "%s: ioctl HDIO_GET_IDENTITY failed: %s",
		  File, SYSERR);
	} else {
	    /* AtaIdent and DriveID should contain same data */
	    (void) memset(&AtaIdent, CNULL, sizeof(AtaIdent));
	    (void) memcpy(&AtaIdent, &DriveID, sizeof(AtaIdent));
	    (void) memset(&Query, CNULL, sizeof(Query));
	    Query.DevInfo = DevInfo;
	    Query.DevFile = File;
	    Query.DevFD = fd;
	    Query.Data = (void *) &AtaIdent;
	    Query.DataLen = sizeof(AtaIdent);
	    (void) AtaIdentDecode(&Query);
	}
#endif	/* HDIO_GET_IDENTITY */
#endif	/* HAVE_ATA */
    } else {
	SImsg(SIM_DBG, "%s: open %s failed: %s", DevName, File, SYSERR);

	/*
	 * Try to look at the OS provided device files which often
	 * work when we cannot query the device ourself.
	 */
	(void) snprintf(Dir, sizeof(Dir), "/proc/ide/%s", DevName);
	if (stat(Dir, &Stat) != 0)
	    return (DevList_t *) NULL;

	(void) snprintf(File, sizeof(File), "%s/model", Dir);
	cp = GetOneLine(File);
	if (!cp)
	    /* Don't bother checking the rest of the files */
	    return (DevList_t *) NULL;

	SImsg(SIM_DBG, "%s:\tATA /proc/ide: Found <%s>", DevName, cp);

	DevInfo = NewDevInfo(NULL);
	DevInfo->Name = strdup(DevName);
	DevInfo->Driver = strdup(Name);
	DevInfo->ClassType = CT_ATA;
	DevInfo->Unit = Unit;
	DevInfo->Type = DevType;
	DevInfo->Model = strdup(cp);

	(void) snprintf(File, sizeof(File), "%s/media", Dir);
	if (cp = GetOneLine(File)) {
	    DevType_t		*Type;

	    if (Type = TypeGetByName(cp))
		DevInfo->Type = Type->Type;
	}
    }

    /*
     * Choose the Device type specific probe function
     */
    switch (DevInfo->Type) {
    	case DT_FLOPPY:
    	case DT_DISKDRIVE:	ProbeFunc = ProbeDiskDrive; break;
    }

    if (ProbeFunc) {
        (void) memset(&ProbeData, CNULL, sizeof(ProbeData));
	ProbeData.DevName = DevInfo->Name;
	ProbeData.DevFile = File;
	ProbeData.FileDesc = fd;
	ProbeData.UseDevInfo = DevInfo;
        (void) (*ProbeFunc)(&ProbeData);
    }

    (void) close(fd);

    if (!DevInfo)
	return (DevList_t *) NULL;

    /*
     * Create DevList
     */
    DevList = (DevList_t *) xcalloc(1, sizeof(DevList_t));
    DevList->Name = DevInfo->Name;
    if (DevInfo->Files)
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
    static DevList_t	       *Last;
    register DevList_t	       *New = NULL;

    if (AtaDevices)
	return(AtaDevices);

    /*
     * Look for ATA Disks
     */
    for (u = 0; u < MAX_ATA_UNITS; ++u) {
	if (New = AtaProbe("hd", u, DT_DISKDRIVE)) {
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
	return -1;

    ClassType = CtlrDevInfo->ClassType;
    switch (ClassType) {
    case CT_SCSI:	Ptr = ScsiGetAll();	break;
    case CT_ATA:	Ptr = AtaGetAll();	break;
    default:
	SImsg(SIM_DBG, "%s: ClassType %d has no ProbeCtlr GetAll() func.",
	      CtlrDevInfo->Name, ClassType);
	return -1;
    }

    for ( ; Ptr; Ptr = Ptr->Next)
        /* 
	 * ATA devices never have good Bus info.
	 * The SCSI Ctlr's Bus # is disguised as Vec 
	 */
	if (ClassType == CT_ATA || Ptr->Bus == CtlrDevInfo->Vec) {
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

    return 0;
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
    static char			DevName[64];
    static int			NetIfs;
    static ProbeData_t		ProbeData;

    if (!DevInfo)
	return -1;

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
    case DT_NETIF:
        /* 
	 * XXX We assume we get the NetIf's (ethernets) in order they are
	 * dynamically numbered by the kernel (ethX).
	 */
	(void) snprintf(DevName, sizeof(DevName), "eth%d", NetIfs++);
	DevInfo->AltName = DevInfo->Name;
	DevInfo->Name = strdup(DevName);
	(void) memset(&ProbeData, 0, sizeof(ProbeData));
	ProbeData.DevName = DevInfo->Name;
	ProbeData.UseDevInfo = DevInfo;
	(void) ProbeNetif(&ProbeData);
	break;
    case DT_BRIDGE:
	/* No special probing for these device types */
	break;
    default:
	if (DevInfo->Type >= 0)
	    SImsg(SIM_DBG, 
		  "%s: No probe func for DevType=%d",
		  Name, DevInfo->Type);
    }

    return 0;
}

/*
 * Set the PCI configuration info for this device (PCIinfo)
 */
static int PCISetConfig(PCIinfo)
     PCIinfo_t		       *PCIinfo;
{
    static char			File[128];
    static char			Config[PCI_CONFIG_SIZE];
    int				fd;
    int				Status;

    /*
     * Open the PCI configuration device file for this specific device
     */
    (void) snprintf(File, sizeof(File), "%s/%02x/%02x.%d",
		    PROC_PATH_BUS_PCI,
		    PCIinfo->Bus, PCIinfo->Unit, PCIinfo->SubDeviceID);

    fd = open(File, O_RDONLY);
    if (fd < 0) {
        SImsg(SIM_DBG, "%s: Cannot open PCI device file to get CLASS info: %s",
	      File, SYSERR);
	return -1;
    }

    Status = read(fd, Config, sizeof(Config));
    (void) close(fd);

    if (Status != sizeof(Config)) {
        SImsg(SIM_DBG, "%s: Read PCI device failed (result %d): %s",
	      File, Status, SYSERR);
	return -1;
    }

    PCIinfo->Class = PCI_CONFIG_WORD(Config, PCI_CLASS_DEVICE);
    PCIinfo->Revision = PCI_CONFIG_BYTE(Config, PCI_REVISION_ID);
    PCIinfo->Header = PCI_CONFIG_BYTE(Config, PCI_HEADER_TYPE);

    return 0;
}

/*
 * Build the device tree by scanning the PCI bus
 */
extern int BuildPCI(DevInfo_t **TreePtr, char **SearchNames)
{
    DevInfo_t                  *DevInfo;
    FILE		       *fp;
    static char                 Buff[512];
    char                        NumStr[16];
    char                      **Argv;
    int                         Argc;
    static PCIinfo_t            Info;
    static DevData_t		DevData;
    char                        DevName[64];
    PCIid_t			DefNum;
    PCIid_t			Vendor;
    PCIid_t			IRQ;

    if (!(fp = fopen(PROC_FILE_PCI_DEV, "r"))) {
	SImsg(SIM_GERR, "%s: Open failed: %s", PROC_FILE_PCI_DEV, SYSERR);
	return -1;
    }

    while (fgets(Buff, sizeof(Buff)-1, fp)) {
	/*
	 * Format of input is:
	 *    DEFNUM VVVVMMMM IRQ ...
	 *
	 * DEFNUM = Bus, Unit, Func
	 * DD = Device Number
	 * VVVV = Vendor PCI ID
	 * MMMM = Vendor's Model ID
	 * IRQ = IRQ
	 * ... = The rest we ignore
	 */
        Argc = sscanf(Buff, "%x %x %x", 
		      &DefNum, &Vendor, &IRQ);
	if (Argc != 3) {
	    SImsg(SIM_GERR, "%s: Wrong number of arguments found (got %d)",
		  PROC_FILE_PCI_DEV, Argc);
	    continue;
	}
	(void) PCInewInfo(&Info);

	Info.Bus = (PCIid_t) (DefNum >> 8U);
	Info.Unit = (PCIid_t) PCI_SLOT(DefNum & 0xFF);
	Info.SubDeviceID = (PCIid_t) PCI_FUNC(DefNum & 0xFF);

	Info.VendorID = Vendor >> 16U;
	Info.DeviceID = Vendor & 0xFFFF;

	(void) PCISetConfig(&Info);

	SImsg(SIM_DBG, 
	      "PCI: Bus=0x%02x Unit=0x%02x SubID=0x%x Vendor=0x%04x Device=0x%04x Class=0x%04x",
	      Info.Bus, Info.Unit, Info.SubDeviceID,
	      Info.VendorID, Info.DeviceID, Info.Class);

	PCIsetDeviceInfo(&Info);

	AddDesc(&(Info.DevInfo->DescList), "IRQ", itoa(IRQ), DA_INSERT);

	/* Build DevData for proper probing */
	(void) memset(&DevData, 0, sizeof(DevData));
	DevData.DevName = Info.DevInfo->Name;
	DevData.DevUnit = Info.Unit;
	DevData.Slave = -1;
	DevData.DevNum = -1;
	DevData.Flags |= DD_IS_ALIVE;
	DevData.CtlrName = "pci";
	DevData.CtlrUnit = Info.Bus;
	DevData.OSDevInfo = Info.DevInfo;

	/* Probe and add device */
	if (TreePtr && (DevInfo = (DevInfo_t *) 
			ProbeDevice(&DevData, TreePtr, SearchNames, NULL))) {
	    AddDevice(DevInfo, TreePtr, SearchNames);
	    if (DevInfo->Master) {
		DevInfo->Master->Type = DT_BUS;
		DevInfo->Master->ClassType = CT_PCI;
	    }
	}

	/*
	 * Do followup work such as looking for children on this device
	 */
	DevInfo->Vec = Info.SubDeviceID;	/* Hide the bus # */
	ProbeDeviceSpec(DevInfo, TreePtr, SearchNames);
	DevInfo->Vec = -1;			/* Zap it */
    }

    (void) fclose(fp);
    return 0;
}

/*
 * Build the device tree
 */
extern int BuildDevices(DevInfo_t **TreePtr, char **SearchNames)
{
    /* Just scan the PCI bus for now */
    return BuildPCI(TreePtr, SearchNames);
}

/*
 * Initialize the OS specific parts of the Device Types table
 */
void DevTypesInit()
{
    /* Do nothing */
}

#if	defined(HAVE__LLSEEK)
/*
 * Use the Linux specific _llseek() for now.
 */
#include <sys/types.h>
#include <unistd.h>
#include <linux/unistd.h>
extern Offset_t Linuxllseek(fd, Offset, Flags)
     int			fd;
     Offset_t			Offset;
     int			Flags;
{
    int				Ret;
    Offset_t			Result;

    Ret = _llseek(fd, ((unsigned long long) Offset) >> 32,
		  ((unsigned long long) Offset) & 0xffffffff,
		  &Result, Flags);

    return (Ret == -1) ? (Offset_t) Ret : Result;
}
#endif	/* HAVE__LLSEEK */
#endif	/* HAVE_DEVICE_SUPPORT */

/*
 * Build Partition information
 */
extern int BuildPartInfo(PartInfoTree, SearchExp)
     PartInfo_t		      **PartInfoTree;
     char		      **SearchExp;
{
    return BuildPartInfoDevices(PartInfoTree, SearchExp);
}
