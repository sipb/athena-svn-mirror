/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * This file contains definetions to all Probe/Dev related functions used
 * internally to build the device tree.
 */
#ifndef __probe_h__
#define __probe_h__ 

/*
 * Size of standard configuration data for a PCI device
 */
#define PCI_CONFIG_SIZE		256

/*
 * These macros read values out of a standard PCI configuration buffer
 * (cfg).  The values are indicated by pos and usually are things like
 * PCI_CLASS_DEVICE, PCI_REVISION_ID, etc which are provided by OS .h files.
 * On Linux this is usually in /usr/include/linux/pci.h.
 */
#define PCI_CONFIG_BYTE(cfg,pos)	(cfg[pos])
#define PCI_CONFIG_WORD(cfg,pos)	(cfg[pos] | cfg[pos+1] << 8)
#define PCI_CONFIG_LONG(cfg,pos)	(cfg[pos] | cfg[pos+1] << 8 |\
						cfg[pos+2] << 16 |\
						cfg[pos+3] << 24)

/*
 * General means of passing parameters around for queries
 */
typedef struct {
    DevInfo_t		       *DevInfo;	/* Device Info */
    char		       *DevFile;	/* Name of file to use */
    int				DevFD;		/* FileDesc if open */
    int				OverRide;	/* Flags */
    void		       *Data;		/* Data being passed */
    size_t			DataLen;	/* Length of Data */
} Query_t;

/*
 * PCI stuff
 */
typedef u_long			PCIid_t;	/* PCI ID type */

/*
 * Info used for general PCI functions
 */
typedef struct {
    PCIid_t			VendorID;	/* Vendor's ID */
    PCIid_t			DeviceID;	/* Device's ID */
    PCIid_t			SubDeviceID;	/* Sub Dev ID */
    PCIid_t			Class;		/* Class Info */
    PCIid_t			Revision;	/* Revision */
    PCIid_t			Header;		/* Header */
    int				Bus;		/* PCI Bus # */
    int				Unit;		/* PCI Unit # */
    int				SubUnit;	/* Sub Unit # */
    DevInfo_t		       *DevInfo;	/* What to operate on */
} PCIinfo_t;

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

#endif /* __probe_h__ */
