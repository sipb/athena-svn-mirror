/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
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

#endif /* __probe_h__ */
