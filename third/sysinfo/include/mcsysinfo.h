/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * Primary header file for mcSysInfo() API.
 */

#ifndef __mcsysinfo_h__
#define __mcsysinfo_h__ 

#if	defined(HAVE_AUTOCONFIG_H)
#include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */
#include <stdio.h>
#include "mctypes.h"
#include "mcdefs.h"

/*
 * Values for MCSIquery_t->Cmd
 */
#define MCSI_HOSTNAME		1		/* Host Name */
#define MCSI_HOSTALIASES	2		/* Host Aliases */
#define MCSI_HOSTADDRS		3		/* Host Addresses */
#define MCSI_HOSTID		4		/* Host ID */
#define MCSI_SERIAL		5		/* Serial # */
#define MCSI_MANSHORT		6		/* Manufacturer (Short) */
#define MCSI_MANLONG		7		/* Manufacturer (Long) */
#define MCSI_MAN		8		/* Manufacturer (Short+Long) */
#define MCSI_MODEL		9		/* System Model */
#define MCSI_PHYSMEM		10		/* Physical Memory */
#define MCSI_VIRTMEM		11		/* Virtual Memory */
#define MCSI_ROMVER		12		/* ROM Version */
#define MCSI_NUMCPU		13		/* # of CPUs */
#define MCSI_CPUTYPE		14		/* CPU Type */
#define MCSI_APPARCH		15		/* Application Arch */
#define MCSI_KERNARCH		16		/* Kernel Arch */
#define MCSI_OSNAME		17		/* OS Name */
#define MCSI_OSVER		18		/* OS Version */
#define MCSI_OSDIST		19		/* OS Distribution */
#define MCSI_KERNVER		20		/* Kernel Version */
#define MCSI_BOOTTIME		21		/* Boot Time */
#define MCSI_CURRENTTIME	22		/* Current Time */
#define MCSI_DEVTREE		40		/* Device Tree */
#define MCSI_KERNELVAR		50		/* Kernel Variables */
#define MCSI_SYSCONF		51		/* sysconf() info */
#define MCSI_SOFTINFO		60		/* Software Tree */
#define MCSI_PARTITION		70		/* Partition Info Tree */

/*
 * Values for MCSIquery_t->Op (Operator)
 */
#define MCSIOP_CREATE		0		/* Create (get) req. data */
#define MCSIOP_DESTROY		1		/* Destroy previous data */
#define MCSIOP_PROGRAM		2		/* Set program name */

/*
 * Values for MCSIquery_t->Flags (Option Flags)
 */
#define MCSIF_DATA		0x000		/* Return basic data type */
#define MCSIF_STRING		0x001		/* Return single string */

/*
 * Query type for mcSysInfo() calls
 */
typedef struct {
    /* Caller's Input */
    int			Op;		/* MCSIOP_* operator */
    int			Cmd;		/* MCSI_* command to perform */
    int			Flags;		/* Option Flags */
    char	      **SearchExp;	/* Search Expression */
    Opaque_t	        In;		/* Ptr to Input */
    size_t		InSize;		/* Size of In */
    /* Results */
    Opaque_t	        Out;		/* Ptr to Output */
    size_t		OutSize;	/* Size of Out */
} MCSIquery_t;

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

#define MHERTZ			1000000				/* MegaHertz */
#define BYTES			((Large_t)1)			/* Bytes */
#define KBYTES			((Large_t)1024)			/* Kilobytes */
#define MBYTES			((Large_t)1048576)		/* Megabytes */
#define GBYTES			((Large_t)1073741824)		/* Gigabytes */

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
 * Misc macros
 */
#define EQ(a,b)			(a && b && strcasecmp(a,b)==0)
#define EQN(a,b,n)		(a && b && strncasecmp(a,b,n)==0)
#define eq(a,b)			(a && b && strcmp(a,b)==0)
#define eqn(a,b,n)		(a && b && strncmp(a,b,n)==0)
#define ARG(s)			((s) ? s : "<null>")
#define PRTS(s)			( ( s && *s ) ? s : "" )

/*
 * Are flags f set in b?
 */
#define FLAGS_ON(b,f)		((b != 0) && (b & f))
#define FLAGS_OFF(b,f)		((b == 0) || ((b != 0) && !(b & f)))

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
#define DL_SCSI_COMP_ALG	"SCSIcompAlg"	/* Compression Algorithms */
#define DL_SCSI_FLAGS		"SCSIflags"	/* SCSI Flags */
#define DL_SCSI_ANSI		"SCSIansiVer"	/* SCSI ANSI Version */
#define DL_CDSPEED		"CDspeed"	/* CD Speeds */
#define DL_DOSPARTTYPES		"DosPartTypes"	/* DOS Partition Types */
#define DL_PCI_CLASS		"PCIclass"	/* PCI Class types */

/*
 * Report names
 */
#define R_NAME			"name"		/* Name description */
#define R_DESC			"desc"		/* Description info */
#define R_PART			"part"		/* Partition info */
#define R_FILE			"file"		/* File info */
#define R_TOTALDISK		"totaldisk"	/* Total disk capacity */

/*
 * Class Names
 */
#define CN_GENERAL		"General"
#define CN_KERNEL		"Kernel"
#define CN_SYSCONF		"SysConf"
#define CN_DEVICE		"Device"
#define CN_SOFTINFO		"Software"
#define CN_PARTITION		"Partition"

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
    int				GetType;	/* What to get */
    int				Enabled;	/* Show this item? */
} ShowInfo_t;

/*
 * Description
 */
struct _Desc {
    char		       *Label;		/* Label of description */
    char		       *Desc;		/* Description */
    int				Flags;		/* Is primary description */
    struct _Desc	       *Next;		/* Pointer to next entry */
};
typedef struct _Desc 		Desc_t;
typedef struct _Desc 		DevDesc_t;	/* For backwards compat */

/*
 * Description Action
 */
#define DA_APPEND		0x01		/* Append entry */
#define DA_INSERT		0x02		/* Insert entry */
#define DA_PRIME		0x10		/* Indicate primary */

/*
 * Unique Identifier
 */
#define IDT_UNKNOWN		0		/* Unknown format */
#define IDT_ASCII		1		/* ASCII data in Identifier */
#define IDT_BINARY		2		/* Binary (unprintable) */
typedef struct {
    int				Type;		/* Type of Identifier IDT_* */
    size_t			Length;		/* Length of Identifier */
    u_char		       *Identifier;	/* Unique ID */
} Ident_t;

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
    Ident_t			*Ident;		/* Unique Identifier */
    char			*Revision;	/* Revision Info */
    DevDesc_t			*DescList;	/* Device Description */
    int				 Unit;		/* Unit number */
    int				 NodeID;	/* ID of this node */
    void			*DevSpec;	/* Device specific info */
    char			*MasterName;	/* Name of master */
    struct _DevInfo		*Master;	/* Device controller */
    struct _DevInfo		*Slaves;	/* Devices on this device */
    struct _DevInfo		*Next;		/* Pointer to next device */
    /* Internal use only */
    void			*OSdata;	/* Data from OS */
    /* Obsolete */
    int				 Addr;		/* Address */
    int				 Prio;		/* Priority */
    int				 Vec;		/* Vector */
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
#define DT_FB			DT_FRAMEBUFFER
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
#define DT_CTLR			DT_CONTROLLER
#define DTN_CONTROLLER		"ctlr"
#define DTN_ADAPTER		"adapter"
#define DT_CPU			16		/* CPU */
#define DTN_CPU			"cpu"
#define DT_MEMORY		17		/* Memory */
#define DTN_MEMORY		"memory"
#define DTN_MEM			"mem"
#define DT_KEYBOARD		18		/* Keyboard */
#define DTN_KEYBOARD		"keyboard"
#define DTN_KEY			"key"
#define DT_CD			19		/* CD-* */
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
#define DT_DVD			33		/* DVD-* */
#define DTN_DVD			"dvd"

/*
 * Backwards compatibility
 */
#define DT_CDROM		DT_CD		/* CD-ROM */
#define DTN_CD			"cd"

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
#define CT_NUBUS		26
#define CT_NUBUS_S		"NuBus"
#define CT_PCMCIA		27
#define CT_PCMCIA_S		"PCMCIA"
#define CT_CARDBUS		28
#define CT_CARDBUS_S		"CardBus"
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
	/* ClassTypes for DT_FRAMEBUFFER */
#define CT_VGA			70
#define CT_VGA_S		"VGA"
#define CT_XGA			71
#define CT_XGA_S		"XGA"
	/* ClassTypes for DT_SERIAL */
#define CT_RS232		80
#define CT_RS232_S		"RS-232"
#define CT_FIREWIRE		81
#define CT_FIREWIRE_S		"FireWire"
#define CT_USB			82
#define CT_USB_S		"USB"
#define CT_SSA			83
#define CT_SSA_S		"SSA"
#define CT_ACCESS		84
#define CT_ACCESS_S		"ACCESS"
#define CT_FIBER		85
#define CT_FIBER_S		"Fiber"
	/* ClassTypes for DT_CPU */
#define CT_CPU			100
#define CT_CPU_S		"CPU Chip"
#define CT_CPUBOARD		101
#define CT_CPUBOARD_S		"CPU Board"
#define CT_FPU			102
#define CT_FPU_S		"FPU"

/*
 * Partition Information (CN_PARTITION)
 */
struct _PartInfo {
    char	       *Title;		/* Describes this entry */
    char	       *DevPath;	/* Block Dev Path 
					   e.g. /dev/dsk/c0t0s1 */
    char	       *DevPathRaw;	/* Raw (Char) Dev Path 
					   e.g. /dev/rdsk/c0t0s1 */
    char	       *DevName;	/* Dev Name e.g. c0t0 */
    char	       *BaseName;	/* c0t0s1 */
    char	       *Name;		/* Name e.g. s1 */
    int			Num;		/* Part # for this disk e.g. 1 */
    char	       *Type;		/* Type e.g. ufs */
    char	       *TypeDesc;	/* Description of Type */
    u_int		TypeNum;	/* Numeric value of Type if any */
    char	       *UsageStatus;	/* String value for Usage (below) */
    char	       *MntName;	/* Mount Name e.g. /usr */
    char	      **MntOpts;	/* Mount Options e.g. rw,quota */
    Large_t		Size;		/* Size of partition in bytes */
    Large_t		AmtUsed;	/* Amt of space currently used */
    int			SecSize;	/* Size of 1 sector */
    Large_t		StartSect;	/* Starting Sector # */
    Large_t		EndSect;	/* Ending Sector # */
    Large_t		NumSect;	/* Number of Sectors (size) */
    /* Internal use only */
    int			Usage;		/* How used, one of PIU_* */
    struct _PartInfo   *Next;
};
typedef struct _PartInfo PartInfo_t;

/*
 * Partition Information Usage (PartInfo_t->Usage)
 */
#define PIU_UNUSED	0		/* Unused as best we can tell */
#define PIU_FILESYS	1		/* Regular filesystem */
#define PIU_SWAP	2		/* Used for swapping */
#define PIU_UNKNOWN	100		/* Used in unknown manner */

/*
 * Disk type
 */
#define DKT_GENERIC		1		/* Generic disk */
#define DKT_CDROM		2		/* CD-ROM */

/*
 * Disk Partition information.
 */
struct _DiskPart {
    PartInfo_t		*PartInfo;	/* Information about this part */
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
    int			 SecSize;	/* Size of Sector (bytes) */
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
    int			ValInt1;		/* Value int 1 */
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
 * Software File Data
 */
struct _SoftFileData {
    int			Type;			/* File, dir, symlink, etc */
    char	       *Path;			/* Path to file */
    char	       *LinkTo;			/* Link To what */
    Large_t		FileSize;		/* Size of file */
    char	       *MD5;			/* MD5 checksum */
    char	       *CheckSum;		/* Platform specific checksum */
    char	      **PkgNames;		/* List of used by Pkgs */
    struct _SoftFileData *Next;			/* Next entry */
};
typedef struct _SoftFileData SoftFileData_t;

/*
 * List of SoftFileData_t entries
 */
struct _SoftFileList {
    SoftFileData_t     *FileData;		/* Ptr to FileData entry */
    struct _SoftFileList *Next;			/* Next entry in our list */
};
typedef struct _SoftFileList SoftFileList_t;

/*
 * SoftFile Types for SoftFileData.Type
 */
#define SFT_FILE	1			/* Plain file */
#define SFT_FILE_S	"File"
#define SFT_HLINK	2			/* Hard Link */
#define SFT_HLINK_S	"Hard-Link"
#define SFT_SLINK	3			/* Soft Link */
#define SFT_SLINK_S	"Symbolic-Link"
#define SFT_DIR		4			/* Directory */
#define SFT_DIR_S	"Directory"
#define SFT_CDEV	5			/* Device File */
#define SFT_CDEV_S	"Character-Device"
#define SFT_BDEV	6			/* Device File */
#define SFT_BDEV_S	"Block-Device"

/*
 * Software Information type.
 */
struct _SoftInfo {
    char	       *EntryType;		/* Entry Type MC_SET_*_S */
    int			EntryTypeNum;		/* Numeric EntryType */
    char	       *Name;			/* Name */
    char	       *Version;		/* Version */
    char	       *Revision;		/* Revision for this Version */
    char	       *Desc;			/* Description of software */
    char	       *DescVerbose;		/* Verbose Description */
    char	       *URL;	 		/* Product URL */
    char	       *License; 		/* Product License */
    char	       *Copyright; 		/* Product Copyright */
    char	       *Category;		/* Category pkg belongs to */
    char	       *SubCategory;		/* Sub Category */
    char	       *OSname;			/* OS name runs on */
    char	       *OSversion;		/* OS version runs on */
    char	       *Arch;			/* Architecture runs on */
    char	       *ISArch;			/* Instruct. Set Arch. */
    char	       *InstDate;		/* Installation Date */
    char	       *BuildDate;		/* Build Date */
    char	       *ProdStamp;		/* Production Stamp */
    char	       *BaseDir;		/* Base Dir for files */
    Large_t		DiskUsage;		/* Space used by FileList */
    SoftFileList_t     *FileList;		/* List of Files */
    Desc_t	       *DescList;		/* Misc Descriptions */
    /* Vendor's info */
    char	       *VendorName;		/* Name of vendor */
    char	       *VendorEmail;		/* Email of vendor */
    char	       *VendorPhone;		/* Phone of vendor */
    char	       *VendorStock;		/* Vendor's Stock # for Pkg */
    /* Tree Links */
    struct _SoftInfo   *Master;			/* (Up) Our parent */
    struct _SoftInfo   *Slaves;			/* (Down) Children */
    struct _SoftInfo   *Next;			/* (Side) Siblings */
};
typedef struct _SoftInfo SoftInfo_t;

/*
 * Values for SoftInfo_t.EntryType
 * Keep in sync with EntryTypes[] in softinfo.c
 */
#define MC_SET_PKG	1			/* Entry is a package */
#define MC_SET_PKG_S	"pkg"
#define MC_SET_PROD	2			/* Entry is a product */
#define MC_SET_PROD_S	"product"

/*
 * Misc functions
 */
#if	defined(ARG_TYPE) && ARG_TYPE == ARG_STDARG
extern void			SImsg(int MsgType, char *fmt, ...);
#else	/* !ARG_STDARG */
extern void			SImsg();
#endif	/* ARG_STDARG */

extern char		       *GetSizeStr();
extern char		       *IdentString();
extern char		       *SoftInfoFileType();
extern DevType_t 	       *TypeGetByType();
extern ClassType_t	       *ClassTypeGetByType();
char			       *itoa();
char			       *Ltoa();

#endif /* __mcsysinfo_h__ */
