/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.3 $
 */

#ifndef __sysinfo_defs__
#define __sysinfo_defs__ 

#include "mconfig.h"
#include "version.h"
#include "mcl.h"

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
 * KVM Address type for KVMget().
 * Should always match 2nd argument type for kvm_read(3)
 */
#if 	defined(KVMADDR_T)
typedef KVMADDR_T		KVMaddr_t;
#else
typedef u_long			KVMaddr_t;
#endif

/*
 * Internal type for large values
 */
#if !defined(SYSINFO_LARGE_T)
#	define SYSINFO_LARGE_T	u_long
#endif
typedef SYSINFO_LARGE_T		Large_t;

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

/*
 * SysInfo Message types
 */
#define SIM_INFO	0x01		/* Normal INFOmation */
#define SIM_WARN	0x02		/* Warnings */
#define SIM_GERR	0x04		/* General errors */
#define SIM_CERR	0x10		/* Critical errors */
#define SIM_DBG		0x20		/* Debugging */
#define SIM_UNKN	0x40		/* Things that are unknown 2 SysInfo */
#define SIM_DEFAULT	(SIM_INFO|SIM_WARN|SIM_CERR)
					/* Default SIM output */
#define SIM_ALL		(SIM_INFO|SIM_WARN|SIM_GERR|SIM_CERR|SIM_UNKN)
					/* Everything, but DBG */
/*
 * SysInfo Message flags
 * Must not conflict with SIM_* defines above!
 */
#define SIM_NONL	0x1000		/* Don't add newline to output */
#define SIM_NOLBL	0x2000		/* Don't print "sysinfo: *ERROR:" */

#if !defined(MAXHOSTNAMLEN)
#	define MAXHOSTNAMLEN 	256
#endif

#define MHERTZ			1000000				/* MegaHertz */
#define BYTES			((Large_t)1)			/* Bytes */
#define KBYTES			((Large_t)1024)			/* Kilobytes */
#define MBYTES			((Large_t)1048576)		/* Megabytes */
#define GBYTES			((Large_t)1073741824)		/* Gigabytes */

/*
 * Misc macros
 */
#define EQ(a,b)			(a && b && strcasecmp(a,b)==0)
#define EQN(a,b,n)		(a && b && strncasecmp(a,b,n)==0)
#define ARG(s)			((s) ? s : "<null>")
#define PRTS(s)			( ( s && *s ) ? s : "" )

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
#define bytes_to_kbytes(N)	( (float) (N / (float) KBYTES) )
#define mbytes_to_bytes(N)	( (float) ( (float) N * (float) MBYTES ) )
#define kbytes_to_mbytes(N)	( (float) ( (float) N / (float) KBYTES ) )
#define kbytes_to_gbytes(N)	( (float) ( (float) N / (float) MBYTES ) )
#define mbytes_to_gbytes(N) 	( (float) ( (float) N / (float) KBYTES ) )
#define nsect_to_bytes(N,S)  	( S ? ( (( (float) N) / (float) \
					 (1024 / S)) * (float) KBYTES) : 0 )
#define nsect_to_mbytes(N,S)	( S ? ( (( (float) N) / (float) \
					 (1024 / S)) / (float) KBYTES) : 0 )

/*
 * Are flags f set in b?
 */
#define FLAGS_ON(b,f)		((b != 0) && (b & f))
#define FLAGS_OFF(b,f)		((b == 0) || ((b != 0) && !(b & f)))

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
 * Message Levels
 */
#define L_GENERAL		0x0001		/* General verbosity */
#define L_BRIEF			0x0002		/* Brief */
#define L_TERSE			0x0004		/* Briefest */
#define L_DESC			0x0010		/* Description info */
#define L_CONFIG		0x0020		/* Configuratin info */
#define L_ALL 	(L_GENERAL|L_DESC|L_CONFIG)
/*
 * Verbosity macros
 */
#define VL_BRIEF		(MsgLevel & L_BRIEF)
#define VL_TERSE		(MsgLevel & L_TERSE)
#define VL_GENERAL		(MsgLevel & L_GENERAL)
#define VL_DESC			(MsgLevel & L_DESC)
#define VL_CONFIG		(MsgLevel & L_CONFIG)
#define VL_ALL			(MsgLevel == L_ALL)

/*
 * Debug macro
 */
#define Debug			FLAGS_ON(MsgClassFlags, SIM_DBG)

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
#define DL_SCSI_DTYPE		"SCSIdtype"	/* SCSI Device Types */
#define DL_SCSI_FLAGS		"SCSIflags"	/* SCSI Flags */
#define DL_CDSPEED		"CDspeed"	/* CD Speeds */
#define DL_DOSPARTTYPES		"DosPartTypes"	/* DOS Partition Types */

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
    void		      (*Show)();	/* Show function */
    void		      (*List)();	/* List arguments function */
    char		       *Label;		/* It's label/description */
    char		       *Banner;		/* Banner */
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
    char			*Driver;	/* Driver Name (e.g. cgtwo) */
    char		       **Aliases;	/* Alias Names */
    char			*AltName;	/* Alt name */
    char		       **Files;		/* Device files */
    int				 Type;		/* Device type (eg DT_TAPE) */
    int				 ClassType;	/* Class type (eg SCSI,IPI) */
    char			*Vendor;	/* Hardware Vendor */
    char			*Model;		/* Model */
    char			*ModelDesc;	/* eg SCSI, 4.0GB, etc. */
    char			*Serial;	/* Serial Number */
    char			*Revision;	/* Revision Info */
    DevDesc_t			*DescList;	/* Device Description */
    int				 Unit;		/* Unit number */
    int				 Addr;		/* Address */
    int				 Prio;		/* Priority */
    int				 Vec;		/* Vector */
    int				 NodeID;	/* ID of this node */
    char			*MasterName;	/* Name of master */
    void			*DevSpec;	/* Device specific info */
    void			*OSdata;	/* Data from OS */
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
#define DT_PSEUDO		2		/* Pseudo Device */
#define DTN_PSEUDO		"pseudo"
#define DT_DRIVER		3		/* Device Driver */
#define DTN_DRIVER		"driver"
#define DT_GENERIC		4		/* Generic Device */
#define DTN_GENERIC		"generic"
#define DT_DEVICE		5		/* Real Device */
#define DTN_DEVICE		"device"
#define DT_DISKDRIVE		6		/* Disk Drive */
#define DTN_DISKDRIVE		"diskdrive"
#define DT_DISKCTLR		7		/* Disk Controller */
#define DTN_DISKCTLR		"diskctlr"
#define DTN_MSD			"msd"
#define DT_TAPEDRIVE		8		/* Tape Drive */
#define DTN_TAPEDRIVE		"tapedrive"
#define DT_TAPECTLR		9		/* Tape Controller */
#define DTN_TAPECTLR		"tapectlr"
#define DT_FRAMEBUFFER		10		/* Frame Buffer */
#define DTN_FRAMEBUFFER		"framebuffer"
#define DTN_FB			"fb"
#define DTN_VID			"vid"
#define DT_GFXACCEL		11		/* Graphics Accelerator */
#define DTN_GFXACCEL		"gfxaccel"	/* Not the same as DT_FRAME* */
#define DT_NETIF		12		/* Network Interface */
#define DTN_NETIF		"netif"
#define DTN_NET			"net"
#define DT_BUS			13		/* System Bus */
#define DTN_BUS			"bus"
#define DT_BRIDGE		14		/* System Bus Bridge */
#define DTN_BRIDGE		"bridge"
#define DT_CONTROLLER		15		/* Generic Controller */
#define DTN_CONTROLLER		"ctlr"
#define DT_CPU			16		/* CPU */
#define DTN_CPU			"cpu"
#define DT_MEMORY		17		/* Memory */
#define DTN_MEMORY		"memory"
#define DTN_MEM			"mem"
#define DT_KEYBOARD		18		/* Keyboard */
#define DTN_KEYBOARD		"keyboard"
#define DTN_KEY			"key"
#define DT_CDROM		19		/* CD-ROM */
#define DTN_CDROM		"cdrom"
#define DT_SERIAL		20		/* Serial */
#define DTN_SERIAL		"serial"
#define DTN_COM			"com"
#define DT_PARALLEL		21		/* Parallel */
#define DTN_PARALLEL		"parallel"
#define DTN_PRT			"prt"
#define DT_CARD			22		/* Generic Card */
#define DTN_CARD		"card"
#define DT_AUDIO		23		/* Audio */
#define DTN_AUDIO		"audio"
#define DT_MFC			24		/* Multi Function Card */
#define DTN_MFC			"mfc"
#define DT_POINTER		25		/* Pointer (mouse, etc) */
#define DTN_POINTER		"pointer"
#define DTN_PTR			"ptr"
#define DTN_MOUSE		"mouse"
#define DT_MONITOR		26		/* Video Monitor */
#define DTN_MONITOR		"monitor"
#define DT_FLOPPY		27		/* Floppy Disk Drive */
#define DTN_FLOPPY		"floppy"
#define DT_FLOPPYCTLR		28		/* Floppy Disk Controller */
#define DTN_FLOPPYCTLR		"floppyctlr"
#define DT_PRINTER		29		/* Printer */
#define DTN_PRINTER		"printer"
#define DT_WORM			30		/* WORM */
#define DTN_WORM		"WORM"
#define DT_SCANNER		31		/* Scanner */
#define DTN_SCANNER		"scanner"
#define DT_CONSOLE		32		/* Console */
#define DTN_CONSOLE		"console"

/*
 * Class Type - Classify devices into sub types according to their DevType.
 * For DT_DISK there is CT_SCSI, CT_IDE, etc.
 * For DT_NETIF there MIGHT be CT_ETHER, CT_FDDI, etc.
 */
#define MAX_CLASS_DEVTYPES	10		/* Max number of DevTypes list
						   in ClassType->DevTypes */
typedef struct {
    int				DevTypes[MAX_CLASS_DEVTYPES];/* DevType List */
    int				Type;		/* ClassType */
    char		       *Name;		/* Type Name */
    char		       *Desc;		/* Description */
    int			      (*Probe)();	/* Probe function */
} ClassType_t;

/*
 * Class Types (DevInfo_t.ClassType)
 * Numeric value must be unique to each class
 */
	/* ClassTypes for DT_DISKDRIVE and DT_DISKCTLR */
#define CT_SCSI			1
#define CT_SCSI_S		"SCSI"
#define CT_IPI			2
#define CT_IPI_S		"IPI"
#define CT_SMD			3
#define CT_SMD_S		"SMD"
#define CT_ATA			4
#define CT_ATA_S		"ATA"
#define CT_IDE_S		"IDE"
	/* ClassTypes for DT_BUS */
#define CT_PCI			20
#define CT_PCI_S		"PCI"
#define CT_ISA			21
#define CT_ISA_S		"ISA"
#define CT_PNPISA		22
#define CT_PNPISA_S		"PNPISA"
#define CT_EISA			23
#define CT_EISA_S		"EISA"
#define CT_SBUS			24
#define CT_SBUS_S		"SBus"
#define CT_MCA			25
#define CT_MCA_S		"MCA"
	/* ClassTypes for DT_NETIF */
#define CT_ETHER10M		40
#define CT_ETHER10M_S		"ether"
#define CT_ETHER100M		41
#define CT_ETHER100M_S		"ether100m"
#define CT_ETHER1G		42
#define CT_ETHER1G_S		"ether1g"
#define CT_FDDI			43
#define CT_FDDI_S		"FDDI"
#define CT_ATM			44
#define CT_ATM_S		"ATM"
#define CT_TOKEN		45
#define CT_TOKEN_S		"token"
#define CT_HIPPI		46
#define CT_HIPPI_S		"HIPPI"
#define CT_ISDN			47
#define CT_ISDN_S		"ISDN"
	/* ClassTypes for DT_MONITOR */
#define CT_RGBCOLOR		60
#define CT_RGBCOLOR_S		"RGB"
#define CT_NONRGBCOLOR		61
#define CT_NONRGBCOLOR_S	"NONRGB"
#define CT_MONO			62
#define CT_MONO_S		"Mono"

/*
 * Disk type
 */
#define DKT_GENERIC		1		/* Generic disk */
#define DKT_CDROM		2		/* CD-ROM */

/*
 * Disk Partition information.
 */
struct _DiskPart {
    char	        *Title;		/* Title of what this Part is */
    char		*Name;		/* Partition name */
    char	        *Type;		/* Type of partition */
    int			 NumType;	/* Numeric val for type */
    char		*Usage;		/* Usage information */
    Large_t		 StartSect;	/* Starting sector */
    Large_t		 NumSect;	/* Number of sectors */
    struct _DiskPart	*Next;		/* Pointer to next DiskPart_t */
};
typedef struct _DiskPart DiskPart_t;

/*
 * Values for DiskDrive_t->DataType
 */
#define DK_DTYPE_HW	"HW"		/* Hardware */
#define DK_DTYPE_OS	"OS"		/* OS */

/*
 * Disk Drive specific data
 */
struct _DiskDrive {
    char		*DataType;	/* HW, OS */
    char		*Label;		/* Disk label */
    int			 Unit;		/* Unit number */
    int			 Slave;		/* Slave number */
    /* Cylinders */
    u_long		 DataCyl;	/* # (usable) data cylinders */
    u_long		 PhyCyl;	/* # physical cylinders */
    u_long		 AltCyl;	/* # alternate cylinders */
    u_long		 CylSkew;	/* Cylinder Skew */
    u_long		 APC;		/* Alternates / Cyl (SCSI) */
    /* Tracks (Heads) */
    u_long		 Tracks;	/* # Tracks */
    u_long		 AltTracksPerZone;/* # Alt Tracks / zone */
    u_long		 AltTracksPerVol;/* # alternate Tracks / volume */
    u_long		 TrackSkew;	/* Track Skew */
    /* Sectors */
    u_long		 Sect;		/* # usable sectors / Track */
    u_long		 SecSize;	/* Size of Sector (bytes) */
    u_long		 PhySect;	/* # physical sectors / Track */
    u_long		 AltSectPerZone;/* # Alternate Sectors / Zone */
    u_long		 SectGap;	/* Gap length between sectors */
    /* Floppy Related */
    u_long		 StepsPerTrack;	/* Steps per track */
    /* General Misc */
    u_long		 RPM;		/* Revolutions Per Minute */
    u_long		 IntrLv;	/* Interleave factor */
    u_long		 PROMRev;	/* PROM Revision */
    /* SysInfo Misc */
    float		 Size;		/* Size of disk (MB) */
    int			 Flags;		/* Info flags */
    DevInfo_t		*Ctlr;		/* Controller disk is on */
    struct _DiskPart	*DiskPart;	/* Partition information */
};
typedef struct _DiskDrive DiskDrive_t;

/*
 * DiskDriveData_t contains multiple DiskDrive_t's because the actual
 * hardware data may differ from what the OS uses.  For instance in
 * x86 UNIX land, the OS may be installed inside of a DOS partition.
 * In this case, the OS may think the disk is "600MB" when in fact it's
 * actually installed in a 600MB partition on a 4GB disk.
 */
typedef struct {
    DiskDrive_t		*HWdata;	/* The actual hardware Drive data */
    DiskDrive_t		*OSdata;	/* The OS's view of the Drive */
} DiskDriveData_t;
    
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
 * Conditions for matching criteria
 */
struct _Condition {
    char		       *Key;		/* Key word */
    char		       *StrVal;		/* String Value */
    int				IntVal;		/* Int Value */
    int				Matches;	/* Matches? */
    int				Flags;		/* Flags */
    struct _Condition	       *Next;		/* Next */
};
typedef struct _Condition	Condition_t;

#define CONFL_SETONLY		0x01		/* Entry only used for set vars */

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
    Condition_t	       *Conditions;		/* Matching conditions */
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
 * Monitor specific data (DT_MONITOR)
 */
typedef struct {
    int			MaxHorSize;		/* Max Horizontal (cm) */
    int			MaxVerSize;		/* Max Vertical (cm) */
    char	      **Resolutions;		/* Supported Resolutions */
} Monitor_t;

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
    char		*DevName;		/* Name of device (no unit) */
    char	       **DevAliases;		/* Dev aliases */
    int			 DevUnit;		/* Device specific unit # */
    int			 DevType;		/* If we know the DevType_t */
    int			 Slave;			/* Slave number */
    dev_t		 DevNum;		/* Device number */
    char		*CtlrName;		/* Name of Controller */
    int			 CtlrUnit;		/* Controller # */
    DevInfo_t		*CtlrDevInfo;		/* Ctlr's DevInfo ptr */
    int			 Flags;			/* DevData Device flags */
    int			 OSflags;		/* Flags passed from OS */
    int			 NodeID;		/* ID of this node */
    DevInfo_t		*OSDevInfo;		/* OS provided DevInfo */
} DevData_t;

/*
 * Flags for DevData_t.Flags
 */
#define DD_MAYBE_ALIVE	0x01			/* Device may be alive */
#define DD_IS_ALIVE	0x02			/* Device is alive */
#define DD_IS_ROOT	0x04			/* Is root dev node */

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
    int			ClassType;		/* ClassType */
    DevInfo_t	     *(*Probe)();		/* Probe device function */
    char	       *Vendor;			/* Hardware Vendor */
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
#define DDT_LENCMP	0x0001			/* Compare by length */
#define DDT_NOUNIT	0x0002			/* No unit number */
#define DDT_UNITNUM	0x0004			/* Force unit number */
#define DDT_ASSUNIT	0x0010			/* Assign unit number */
#define DDT_ZOMBIE	0x0020			/* Doesn't have to be ALIVE */
#define DDT_DEFINFO	0x0040			/* Use default definetions */
#define DDT_ISALIAS	0x0100			/* We're mk'ing an alias */

/*
 * Data needed for Probe routines
 */
struct _ProbeData {
    /* For argument passing */
    char	       *DevName;		/* Device's Name (vx0) */
    char	      **AliasNames;		/* Aliases for DevName */
    char	       *DevFile;		/* Name of device file */
    int			FileDesc;		/* File Descriptor */
    DevData_t	       *DevData;		/* Device Data */
    DevDefine_t	       *DevDefine;		/* Device Definetions */
    void	       *Opaque;			/* Opaque data */
    DevInfo_t	       *CtlrDevInfo;		/* Ctlr's DevInfo if we know */
    DevInfo_t	       *UseDevInfo;		/* DevInfo we can use */
    /* Returned/set data */
    DevInfo_t	       *RetDevInfo;		/* Returned Data */
};
typedef struct _ProbeData 	ProbeData_t;

/*
 * DevFind query
 */
typedef struct {
    /* These are the conditions to search for */
    char	       *NodeName;		/* Name of node to find */
    OBPnodeid_t		NodeID;			/* Node ID to find */
    char	       *Serial;			/* Serial # to find */
    int			DevType;		/* DevType */
    int			ClassType;		/* ClassType */
    int			Unit;			/* Unit # */
    /* We operate on these */
    DevInfo_t	       *Tree;			/* Device Tree */
    int			Expr;			/* DFE_* Expression Flags */
    char	        Reason[128];		/* List of what matched */
} DevFind_t;

/*
 * DevFind Expression commands (DevFind_t.Expr)
 */
#define DFE_OR		0			/* OR all checks */
#define DFE_AND		1			/* AND all checks */

/*
 * Now include the declarations
 */
#include "declare.h"
#include "probe.h"

#endif /* __sysinfo_defs__ */
