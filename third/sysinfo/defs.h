/*
 * Copyright (c) 1992-1994 Michael A. Cooper.
 * This software may be freely distributed provided it is not sold for 
 * profit and the author is credited appropriately.
 */

/*
 * $Id: defs.h,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $
 */

#ifndef __sysinfo_defs__
#define __sysinfo_defs__ 

#include <stdio.h>
#include "os.h"
#include "options.h"		/* Need to include options.h before errno.h */
#include <errno.h>
#include "version.h"
#include "config.h"

#if defined(HAVE_UNAME)
#	include <sys/utsname.h>
#endif	/* HAVE_UNAME */
#if defined(HAVE_SYSINFO)
#	include <sys/systeminfo.h>
#endif	/* HAVE_SYSINFO */

/*
 * nlist information
 */
#if defined(HAVE_NLIST)
#	include <fcntl.h>
#	include <nlist.h>
#if	defined(NLIST_TYPE)
	typedef NLIST_TYPE	nlist_t;
#else
	typedef struct nlist	nlist_t;
#endif
#if	!defined(NLIST_FUNC)
#	define NLIST_FUNC	nlist
#endif
#endif	/* HAVE_NLIST */

/*
 * Offset type
 */
#if !defined(OFF_T_TYPE)
#       define OFF_T_TYPE	off_t
#endif

#if defined(NEED_KVM)
#	include "kvm.h"
#else
#if defined(HAVE_KVM)
#	include <kvm.h>
#endif	/* HAVE_KVM */
#endif	/* NEED_KVM */

/*
 * File containing our CPU model name.  Overrides all other methods
 * for determing model name.
 */
#if !defined(MODELFILE)
#	define MODELFILE	"/etc/sysmodel"
#endif

/*
 * System call failure value
 */
#if !defined(SYSFAIL)
#	define SYSFAIL		-1
#endif

#define CNULL			'\0'
#define C_NULL			CNULL

/*
 * How to get system error string
 */
#if	defined(SYSERR)
#	undef SYSERR
#endif	/* SYSERR */
#if	defined(IS_POSIX_SOURCE)
extern		int errno;
#define		SYSERR			strerror(errno)
#else	/* !IS_POSIX_SOURCE */
extern		int 			errno;
extern		int 			sys_nerr;
extern		char			*sys_errlist[];
#define		SYSERR \
((errno>0 && errno<sys_nerr) ? sys_errlist[errno] : "(unknown system error)")
#endif	/* IS_POSIX_SOURCE */

#if !defined(MAXHOSTNAMLEN)
#	define MAXHOSTNAMLEN 	256
#endif

#define MHERTZ			1000000			/* MegaHertz */
#define BYTES			((u_long)1)		/* Bytes */
#define KBYTES			((u_long)1024)		/* Kilobytes */
#define MBYTES			((u_long)1048576)	/* Megabytes */
#define GBYTES			((u_long)1073741824)	/* Gigabytes */

/*
 * Misc macros
 */
#define EQ(a,b)			(strcasecmp(a,b)==0)
#define EQN(a,b,n)		(strncasecmp(a,b,n)==0)
#define ARG(s)			((s) ? s : "<null>")
#define PS(s)			( ( s && *s ) ? s : "" )

/*
 * CheckNlist() breaks in a variety of ways on
 * various OS's.
 */
#if !defined(BROKEN_NLIST_CHECK)
#	define CheckNlist(p) 	_CheckNlist(p)
#else
#	define CheckNlist(p) 	0
#endif

/*
 * Get nlist n_name
 */
#if defined(__MACH__)
#	define GetNlName(N)	((N).n_un.n_name)
#	define GetNlNamePtr(N)	((N)->n_un.n_name)
#else
#	define GetNlName(N)	((N).n_name)
#	define GetNlNamePtr(N)	((N)->n_name)
#endif	/* __MACH__ */

/*
 * Conversion macros
 */
#define bytes_to_gbytes(N)	( (float) N / (float) GBYTES )
#define bytes_to_mbytes(N)	( (float) N / (float) MBYTES )
#define bytes_to_kbytes(N)	( (float) (N / KBYTES) )
#define mbytes_to_bytes(N)	( (float) ( (float) N * MBYTES ) )
#define kbytes_to_mbytes(N)	( (float) ( (float) N / KBYTES ) )
#define kbytes_to_gbytes(N)	( (float) ( (float) N / MBYTES ) )
#define mbytes_to_gbytes(N) 	( (float) ( (float) N / KBYTES ) )
#define nsect_to_bytes(N,S)  	( S ? ( (( (float) N) / (float) \
					 (1024 / S)) * KBYTES) : 0 )
#define nsect_to_mbytes(N,S)	( S ? ( (( (float) N) / (float) \
					 (1024 / S)) / KBYTES) : 0 )

/*
 * Are flags f set in b?
 */
#define FLAGS_ON(b,f)		((b != 0) && (b & f))

/*
 * KVM Data Type
 */
#define KDT_DATA		0		/* Data buffer */
#define KDT_STRING		1		/* NULL terminated string */

/*
 * Format Types
 */
#define FT_PRETTY		1		/* Normal pretty output */
#define FT_REPORT		2		/* Report output */
#define FormatType_t		int

/*
 * Verbosity levels
 */
#define L_GENERAL		0x0001		/* General verbosity */
#define L_BRIEF			0x0002		/* Brief */
#define L_TERSE			0x0004		/* Briefest */
#define L_DESC			0x0010		/* Description info */
#define L_CONFIG		0x0020		/* Configuratin info */
#define L_DEBUG			0x0040		/* Debug info */
#define L_ALL 	(L_GENERAL|L_DESC|L_CONFIG)
/*
 * Verbosity macros
 */
#define VL_BRIEF		(Level & L_BRIEF)
#define VL_TERSE		(Level & L_TERSE)
#define VL_GENERAL		(Level & L_GENERAL)
#define VL_DESC			(Level & L_DESC)
#define VL_CONFIG		(Level & L_CONFIG)
#define VL_ALL			(Level == L_ALL)
#define VL_DEBUG		(Level & L_DEBUG)

/*
 * Definetion Options
 */
#define DO_REGEX		0x0001		/* Perform regex match */

/*
 * Definetion List names
 */
#define DL_SYSMODEL		"SysModel"	/* System Models */
#define DL_SUBSYSMODEL		"SubSysModel"	/* Sub System Models */
#define DL_SYSCONF		"SysConf"	/* System Configuration */
#define DL_KARCH		"KArch"		/* Kernel Architectures */
#define DL_CPU			"CPU"		/* CPU types */
#define DL_OBP			"OBP"		/* Open Boot Prom info */
#define DL_VPD			"VPD"		/* Vital Product Data */
#define DL_NETTYPE		"NetType"	/* Network Types */
#define DL_CATEGORY		"Category"	/* Categorys */
#define DL_PART			"Part"		/* Part info */
#define DL_TAPEINFO		"TapeInfo"	/* Tape info */
#define DL_KERNEL		"Kernel"	/* Kernel Variable */

/*
 * Report names
 */
#define R_NAME			"name"		/* Name description */
#define R_DESC			"desc"		/* Description info */
#define R_PART			"part"		/* Partition info */
#define R_TOTALDISK		"totaldisk"	/* Total disk capacity */

/*
 * Class Names
 */
#define CN_GENERAL		"General"
#define CN_KERNEL		"Kernel"
#define CN_SYSCONF		"SysConf"
#define CN_DEVICE		"Device"

/*
 * Types
 */
typedef void *			Opaque_t;	/* Hide type */
typedef int			OBPnodeid_t;	/* OBP node ID type */

/*
 * Name list
 */
struct _namelist {
    char		       *nl_name;
    struct _namelist	       *nl_next;
};
typedef struct _namelist namelist_t;

/*
 * Platform Specific Interface
 */
typedef struct {
    char		      *(*Get)();
} PSI_t;

/*
 * List Info type
 */
struct _ListInfo {
    int				IntKey;		/* Int key */
    char		       *Name;		/* Keyword name */
    char		       *Desc;		/* Description */
    void		      (*List)();	/* List function */
};
typedef struct _ListInfo	ListInfo_t;

/*
 * Class Information
 */
typedef struct {
    char		       *Name;		/* Class name */
    char		       *Label;		/* It's label/description */
    void		      (*Show)();	/* Show function */
    void		      (*List)();	/* List arguments function */
    int				Enabled;	/* Is enabled or not */
} ClassInfo_t;

/*
 * Show Information
 */
typedef struct {
    char		       *Name;		/* Name */
    char		       *Label;		/* Label/description */
    char		     *(*Get)();		/* Get value function */
    int				Enabled;	/* Show this item? */
} ShowInfo_t;

/*
 * Device description
 */
struct _DevDesc {
    char		       *Label;		/* Label of description */
    char		       *Desc;		/* Description */
    int				Flags;		/* Is primary description */
    struct _DevDesc	       *Next;		/* Pointer to next entry */
};
typedef struct _DevDesc DevDesc_t;
/*
 * Description Action
 */
#define DA_APPEND		0x01		/* Append entry */
#define DA_INSERT		0x02		/* Insert entry */
#define DA_PRIME		0x10		/* Indicate primary */

/*
 * Main Device information
 *
 * Used after device info has been obtained 
 */
struct _DevInfo {
    char			*Name;		/* Name (e.g. cgtwo0) */
    char			*AltName;	/* Alt name */
    char		       **Files;		/* Device files */
    int				 Type;		/* Device type (eg DT_TAPE) */
    int				 ClassType;	/* Class type (eg SCSI,IPI) */
    char			*Model;		/* Model */
    char			*ModelDesc;	/* Model Specific Description*/
    DevDesc_t			*DescList;	/* Device Description */
    int				 Unit;		/* Unit number */
    int				 Addr;		/* Address */
    int				 Prio;		/* Priority */
    int				 Vec;		/* Vector */
    int				 NodeID;	/* ID of this node */
    char			*MasterName;	/* Name of master */
    caddr_t			*DevSpec;	/* Device specific info */
    struct _DevInfo		*Master;	/* Device controller */
    struct _DevInfo		*Slaves;	/* Devices on this device */
    struct _DevInfo		*Next;		/* Pointer to next device */
};
typedef struct _DevInfo DevInfo_t;

/*
 * Device types (DevInfo_t.Type)
 */
#define DT_NONE			1		/* Sort of ignore */
#define DTN_NONE		"none"
#define DT_GENERIC		2		/* Generic Device */
#define DTN_GENERIC		"generic"
#define DT_DEVICE		3		/* Real Device */
#define DTN_DEVICE		"device"
#define DT_DISKDRIVE		4		/* Disk Drive */
#define DTN_DISKDRIVE		"DiskDrive"
#define DT_DISKCTLR		5		/* Disk Controller */
#define DTN_DISKCTLR		"diskctlr"
#define DT_TAPEDRIVE		6		/* Tape Drive */
#define DTN_TAPEDRIVE		"tapedrive"
#define DT_TAPECTLR		7		/* Tape Controller */
#define DTN_TAPECTLR		"tapectlr"
#define DT_FRAMEBUFFER		8		/* Frame Buffer */
#define DTN_FRAMEBUFFER		"framebuffer"
#define DT_NETIF		9		/* Network Interface */
#define DTN_NETIF		"netif"
#define DT_BUS			10		/* System Bus */
#define DTN_BUS			"bus"
#define DT_PSEUDO		11		/* Pseudo Device */
#define DTN_PSEUDO		"pseudo"
#define DT_CPU			12		/* CPU */
#define DTN_CPU			"cpu"
#define DT_MEMORY		13		/* Memory */
#define DTN_MEMORY		"memory"
#define DT_KEYBOARD		14		/* Keyboard */
#define DTN_KEYBOARD		"keyboard"
#define DT_CDROM		15		/* CD-ROM */
#define DTN_CDROM		"cdrom"
#define DT_SERIAL		16		/* Serial */
#define DTN_SERIAL		"serial"
#define DT_PARALLEL		17		/* Parallel */
#define DTN_PARALLEL		"parallel"
#define DT_CARD			18		/* Generic Card */
#define DTN_CARD		"card"
#define DT_AUDIO		18		/* Audio */
#define DTN_AUDIO		"audio"

/*
 * Class Types (DevInfo_t.ClassType)
 */
#define CT_SCSI			1		/* SCSI */
#define CT_IPI			2		/* SMD */
#define CT_SMD			3		/* IPI */

/*
 * Disk type
 */
#define DKT_GENERIC		1		/* Generic disk */
#define DKT_CDROM		2		/* CD-ROM */

/*
 * Disk Partition information.
 */
struct _DiskPart {
    char		*Name;		/* Partition name */
    char	        *Type;		/* Type of partition */
    char		*Usage;		/* Usage information */
    int			 StartSect;	/* Starting sector */
    int			 NumSect;	/* Number of sectors */
    struct _DiskPart	*Next;		/* Pointer to next DiskPart_t */
};
typedef struct _DiskPart DiskPart_t;

/*
 * Disk Drive specific data
 */
struct _DiskDrive {
    char		*Label;		/* Disk label */
    int			 Unit;		/* Unit number */
    int			 Slave;		/* Slave number */
    int			 DataCyl;	/* # data cylinders */
    int			 PhyCyl;	/* # physical cylinders */
    int			 AltCyl;	/* # alternate cylinders */
    int			 Heads;		/* Number of heads */
    int			 Sect;		/* Number of sectors */
    int			 PhySect;	/* Number of physical sector */
    int			 PROMRev;	/* PROM Revision */
    int			 APC;		/* Alternates / Cyl (SCSI) */
    int			 RPM;		/* Revolutions Per Minute */
    int			 IntrLv;	/* Interleave factor */
    int			 SecSize;	/* Size of Sector (bytes) */
    float		 Size;		/* Size of disk in bytes */
    int			 Flags;		/* Info flags */
    DevInfo_t		*Ctlr;		/* Controller disk is on */
    struct _DiskPart	*DiskPart;	/* Partition information */
    struct _DiskDrive	*Next;		/* Pointer to next disk */
};
typedef struct _DiskDrive DiskDrive_t;

/*
 * Disk Flags (DiskDrive_t.Flags)
 */
#define DF_HEXUNIT	0x01			/* Unit is prt 3 hex digits */

/*
 * Generic Name/Value table
 */
typedef struct {
    int			value;			/* Value field */
    char       	       *name;			/* Corresponding name */
    char       	       *valuestr;		/* Value string field */
} NameTab_t;

/*
 * Definetion information
 */
struct _Define {
    char	       *KeyStr;			/* Key string */
    long	        KeyNum;			/* Key numeric */
    char	       *ValStr1;		/* Value string 1 */
    char	       *ValStr2;		/* Value string 2 */
    char	       *ValStr3;		/* Value string 3 */
    char	       *ValStr4;		/* Value string 4 */
    char	       *ValStr5;		/* Value string 5 */
    struct _Define     *Next;			/* Next member */
};
typedef struct _Define Define_t;

/*
 * Definetion List
 */
struct _DefineList {
    char	       *Name;			/* Name of list */
    Define_t	       *Defines;		/* Definetion list */
    struct _DefineList *Next;			/* Next member */
};
typedef struct _DefineList DefineList_t;
    
/*
 * Generic Key/Value table
 */
typedef struct {
    char       	       *Key;		/* Corresponding name */
    long		Lvalue;		/* Long value field */
    char       	       *Svalue;		/* Value string field */
} KeyTab_t;

/*
 * FrameBuffer specific data
 */
typedef struct {
    int			 Height;		/* Height (in pixels) */
    int			 Width;			/* Width (in pixels) */
    int			 Depth;			/* Depth (bits/pixel) */
    u_long		 Size;			/* Total size (in bytes) */
    u_long		 VMSize;		/* Video memory (in bytes) */
    int			 CMSize;		/* Color Map Size (#entries) */
} FrameBuffer_t;

/*
 * Network Interface specific data
 */
struct _netif {
    char		*TypeName;		/* Name of address type */
    char		*HostAddr;		/* Host address */
    char		*HostName;		/* Host name */
    char		*MACaddr;		/* Current MAC address */
    char		*MACname;		/* Current MAC name */
    char		*FacMACaddr;		/* Factory MAC address */
    char		*FacMACname;		/* Factory MAC name */
    char		*NetAddr;		/* Network address */
    char		*NetName;		/* Network name */
    struct _netif	*Next;			/* Pointer to next element */
};
typedef struct _netif NetIF_t;

/*
 * Address family table
 */
typedef struct {
    int			Type;			/* Type value */
    char	       *Name;			/* Name value */
    NetIF_t	     *(*GetNetIF)();		/* Function to get netif */
} AddrFamily_t;

/*
 * Basic device data
 *
 * Used when searching for initial list of devices
 */
typedef struct {
    char		*DevName;		/* Name of device */
    int			 DevUnit;		/* Device specific unit # */
    int			 Slave;			/* Slave number */
    dev_t		 DevNum;		/* Device number */
    char		*CtlrName;		/* Name of Controller */
    int			 CtlrUnit;		/* Controller # */
    int			 Flags;			/* Device flags */
    int			 NodeID;		/* ID of this node */
} DevData_t;

/*
 * Flags for DevData_t.Flags
 */
#define DD_MAYBE_ALIVE	0x1			/* Device may be alive */
#define DD_IS_ALIVE	0x2			/* Device is alive */
#define DD_IS_ROOT	0x4			/* Is root dev node */

/*
 * Device Type information
 */
typedef struct {
    int				Type;		/* Device type */
    char		       *Name;		/* Type Name */
    char		       *Desc;		/* Description */
    void		      (*Print)();	/* Print function */
    DevInfo_t		     *(*Probe)();	/* Probe function */
    int				Enabled;	/* Enable this entry */
} DevType_t;

/*
 * Device definetion structure
 */
struct _DevDefine {
    char	       *Name;			/* Name of device */
    char	      **Aliases;		/* List of name aliases */
    int			Ident;			/* Device identifier */
    int			Type;			/* Device type */
    DevInfo_t	     *(*Probe)();		/* Probe device function */
    char	       *Model;			/* Model */
    char	       *Desc;			/* Description */
    char	       *File;			/* Device file name */
    int			Flags;			/* Flags */
    long		DevFlags;		/* Device specific flags */
    struct _DevDefine  *Next;			/* Next member */
};
typedef struct _DevDefine DevDefine_t;

/*
 * Flags for DevDefine_t.Flags
 */
#define DDT_LENCMP	0x01			/* Compare by length */
#define DDT_NOUNIT	0x02			/* No unit number */
#define DDT_ZOMBIE	0x03			/* Doesn't have to be ALIVE */
#define DDT_DEFINFO	0x04			/* Use default definetions */

/*
 * Declarations
 */
extern int 			DoPrintUnknown;
extern int 			Debug;
extern int 			Level;
extern int 			Terse;
extern int 			UseProm;
extern char 		       *UnSupported;
extern DefineList_t	       *Definetions;
extern DevDefine_t	       *DevDefinetions;

#if	!defined(HAVE_STR_DECLARE)
char 			       *strchr();
char		 	       *strrchr();
char 			       *strdup();
char 			       *strcat();
char 			       *strtok();
#endif	/* HAVE_STR_DECLARE */

char			       *itoa();
char	 		       *xmalloc();
char	 		       *xrealloc();
char	 		       *xcalloc();
int			        StrToArgv();
void				DestroyArgv();
extern char		       *strlower();
extern char		       *strupper();
extern void			strtolower();

extern DevDefine_t	       *DevDefGet();
extern void			DevDefAdd();

extern Define_t		       *DefGet();
extern Define_t		       *DefGetList();
extern void			DefAdd();
extern int			DefValid();

extern void			Report();

extern DevDesc_t	       *PrimeDescPtr();
extern DevInfo_t	       *FindDeviceByName();
extern DevInfo_t	       *FindDeviceByNodeID();
extern DevInfo_t	       *FindDeviceByType();
extern DevInfo_t	       *MkMasterFromDevData();
extern DevInfo_t	       *ProbeDevice();
extern DevInfo_t	       *ProbeUnknown();
extern DevInfo_t 	       *NewDevInfo();
extern DevInfo_t 	       *OBPprobeCPU();
extern DevInfo_t 	       *ProbeCDROMDrive();
extern DevInfo_t 	       *ProbeDiskDrive();
extern DevInfo_t 	       *ProbeFrameBuffer();
extern DevInfo_t 	       *ProbeGeneric();
extern DevInfo_t 	       *ProbeKbd();
extern DevInfo_t 	       *ProbeNetif();
extern DevInfo_t 	       *ProbeTapeDrive();
extern DiskDrive_t	       *NewDiskDrive();
extern DiskPart_t  	       *NewDiskPart();
extern FrameBuffer_t 	       *NewFrameBuffer();
extern NetIF_t	 	       *NewNetif();
extern char		       *FreqStr();
extern char		       *GetCpuName();
extern char		       *GetCpuType();
extern char		       *GetCpuTypeCmds();
extern char		       *GetCpuTypeDef();
extern char		       *GetCpuTypeHostInfo();
extern char		       *GetCpuTypeSysinfo();
extern char		       *GetCpuTypeTest();
extern char		       *GetHostName();
extern char		       *GetKernArch();
extern char		       *GetKernArchCmds();
extern char		       *GetKernArchDef();
extern char		       *GetKernArchSysinfo();
extern char		       *GetKernArchUname();
extern char		       *GetKernArchTest();
extern char		       *GetNameTabName();
extern char		       *GetOSNameSysinfo();
extern char		       *GetOSNameUname();
extern char		       *GetOSNameDef();
extern char		       *GetOSVerDef();
extern char		       *GetOSVerKernVer();
extern char		       *GetOSVerSysinfo();
extern char		       *GetOSVerUname();
extern char		       *GetSizeStr();
extern char		       *GetTapeModel();
extern char		       *MkDevName();
extern char		       *RunCmds();
extern char		       *RunTestFiles();
extern char	 	       *GetMemoryPhysmemSym();
extern char	 	       *MkDevName();
extern char 		       *GetAppArch();
extern char 		       *GetAppArchCmds();
extern char 		       *GetAppArchDef();
extern char 		       *GetAppArchSysinfo();
extern char 		       *GetAppArchTest();
extern char 		       *GetBootTime();
extern char 		       *GetBootTimeUtmp();
extern char 		       *GetBootTimeSym();
extern char 		       *GetCPU();
extern char 		       *GetCharFile();
extern char 		       *GetDiskCapacity();
extern char 		       *GetHostAddrs();
extern char 		       *GetHostAliases();
extern char 		       *GetHostID();
extern char 		       *GetKernVer();
extern char 		       *GetKernVerSym();
extern char 		       *GetMan();
extern char 		       *GetManLong();
extern char 		       *GetManLongDef();
extern char 		       *GetManLongSysinfo();
extern char 		       *GetManShort();
extern char 		       *GetManShortDef();
extern char 		       *GetManShortSysinfo();
extern char 		       *GetMemory();
extern char 		       *GetMemoryStr();
extern char 		       *GetModel();
extern char 		       *GetModelCmd();
extern char 		       *GetModelDef();
extern char 		       *GetModelFile();
extern char 		       *GetNumCpu();
extern char 		       *GetNumCpuHostInfo();
extern char 		       *GetNumCpuSysconf();
extern char 		       *GetOSName();
extern char 		       *GetOSVer();
extern char 		       *GetRawFile();
extern char 		       *GetRomVer();
extern char 		       *GetSerial();
extern char 		       *GetSerialSysinfo();
extern char 		       *GetVirtMem();
extern char 		       *GetVirtMemAnoninfo();
extern char 		       *GetVirtMemNswap();
extern char 		       *GetVirtMemStr();
extern char 		       *PSIquery();
extern char 		       *PrimeDesc();
extern int		        AddDevice();
extern void		        DeviceList();
extern void		        DeviceShow();
extern void		        KernelList();
extern void		        KernelShow();
extern void		        SetMacInfo();
extern void 		        DetectDevices();
extern void 		        GeneralList();
extern void 		        GeneralShow();
extern void 		        SysConfList();
extern void 		        SysConfShow();

extern void		        ClassShowLabel();
extern void		        ClassShowValue();
extern void			ClassList();
extern void			ClassSetInfo();
extern int			ClassCall();
extern void			ClassCallList();

extern void			TypeList();
extern void			TypeSetInfo();
extern DevType_t	       *TypeGetByType();
extern DevType_t	       *TypeGetByName();

extern namelist_t	       *NameListFind();
extern void 		        NameListAdd();
extern void 		        NameListFree();

extern void			DevTypesInit();
extern DevType_t		DevTypes[];
extern FormatType_t		FormatType;

extern char		       *VarSub();

#if	ARG_TYPE == ARG_STDARG
extern void			Error(char *fmt, ...);
#else	/* !ARG_STDARG */
extern void			Error();
#endif	/* ARG_STDARG */

#if	defined(HAVE_KVM)
extern kvm_t 		       *KVMopen();
extern void			KVMclose();
extern int			KVMget();
extern nlist_t		       *KVMnlist();
#endif	/* HAVE_KVM */

#endif /* __sysinfo_defs__ */
