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
 * SunOS OBP definetions
 */
#ifndef __sunos_obp_h__
#define __sunos_obp_h__ 

#if	defined(HAVE_OPENPROM)

/*
 * OBP device file
 */
#define _PATH_OPENPROM		"/dev/openprom"

/*
 * OBP keywords
 */
#define OBP_ALIASES		"aliases"
#define OBP_BANNERNAME		"banner-name"
#define OBP_BOARD		"board#"
#define OBP_BOARDTYPE		"board-type"
#define OBP_CLOCKFREQ		"clock-frequency"
#define OBP_CPUID		"cpu-id"
#define OBP_DCACHESIZE		"dcache-size"
#define OBP_DEVICEID		"device-id"
#define OBP_DEVTYPE		"device_type"
#define OBP_DEVTYPE2		"device-type"
#define OBP_ECACHESIZE		"ecache-size"
#define OBP_EDID		"edid_data"
#define OBP_FHC			"fhc"
#define OBP_KEYBOARD		"keyboard"
#define OBP_MEMUNIT		"mem-unit"
#define OBP_MODEL		"model"
#define OBP_NAME		"name"
#define OBP_SIZE		"size"
#define OBP_SYSBOARD		"sysboard"
#define OBP_VENDORID		"vendor-id"
#ifndef OBP_CPU
#define OBP_CPU			"cpu"
#endif

/*
 * OBPio_t is used to do I/O with openprom.
 */
typedef union {
    char			opio_buff[OPROMMAXPARAM];
    struct openpromio		opio_oprom;
} OBPio_t;

/*
 * OBPprop_t is used to store properties for OBP nodes.
 */
typedef struct _OBPprop {
    char		       *Key;		/* Key as ASCII */
    char		       *Value;		/* Value as ASCII string */
    void		       *Raw;		/* Uninterpretted data */
    size_t			RawLen;		/* Length of Raw */
    struct _OBPprop	       *Next;	
} OBPprop_t;

/*
 * OBP Node entry used to represent tree of all OBP nodes.
 */
typedef struct _obpnode {
    char		       *Name;
    OBPnodeid_t		        NodeID;
    OBPnodeid_t		        ParentID;
    char		       *ParentName;
    OBPprop_t		       *PropTable;
    struct _obpnode	       *Children;
    struct _obpnode	       *Next;
} OBPnode_t;

/*
 * Parameters for SubSys
 */
typedef struct _SubSysVar {
    OBPnode_t		       *OBPtree;
    int				IntVal;
    char		       *StrVal;
} SubSysVar_t;

/*
 * Declarations
 */
extern OBPnode_t	       *OBPfindNodeByID();
extern OBPprop_t	       *OBPfindProp();
extern char	 	       *OBPfindPropVal();
extern DevInfo_t 	       *OBPprobeCPU();

#endif	/* HAVE_OPENPROM */

#endif 	/* __sunos_obp_h__ */
