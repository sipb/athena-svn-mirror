/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */
#ifndef __mysysinfo_lib_defs__
#define __mysysinfo_lib_defs__ 

#if defined(HAVE_AUTOCONFIG_H)
#	include "autoconfig.h"
#endif	/* HAVE_AUTOCONFIG_H */

#include "os.h"
#include "config.h"
#include "mcsysinfo.h"
#include "myfileio.h"
#include "dospart.h"

#if defined(HAVE_STRING_H)
#	include <string.h>
#endif	/* HAVE_STRING_H */
#if defined(HAVE_UNAME)
#	include <sys/utsname.h>
#endif	/* HAVE_UNAME */
#if defined(HAVE_SYS_SYSTEMINFO_H)
#	include <sys/systeminfo.h>
#endif	/* HAVE_SYS_SYSTEMINFO_H */

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

#if 	defined(HAVE_KVM_H)
#	include <kvm.h>
#else
#	include "mykvm.h"
#endif	/* HAVE_KVM_H */

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

#if !defined(MAXHOSTNAMLEN)
#	define MAXHOSTNAMLEN 	256
#endif

/*
 * Default disk sector size
 */
#define SECSIZE_DEFAULT		512

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
 * KVM Data Type
 */
#define KDT_DATA		0		/* Data buffer */
#define KDT_STRING		1		/* NULL terminated string */

/*
 * Definetion Options
 */
#define DO_REGEX		0x0001		/* Perform regex match */

/*
 * CleanString Flags
 */
#define CSF_ENDNONPR		0x001		/* End when we hit nonprint. */
#define CSF_ALPHANUM		0x002		/* End when we hit AlphaNum */
#define CSF_BSWAP		0x004		/* Byte Swap the string */

/*
 * Types
 */
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
    Ident_t	       *Ident;			/* Unique ID to find */
    char	       *Serial;			/* Serial # to find */
    int			DevType;		/* DevType */
    int			ClassType;		/* ClassType */
    int			Unit;			/* Unit # */
    /* We operate on these */
    DevInfo_t	       *Tree;			/* Device Tree */
    int			Expr;			/* DFE_* Expression Flags */
    int			Flags;			/* DFF_* Option Flags */
    char	        Reason[128];		/* List of what matched */
} DevFind_t;

/*
 * DevFind Expression commands (DevFind_t.Expr)
 */
#define DFE_OR		0			/* OR all checks */
#define DFE_AND		1			/* AND all checks */

/*
 * DevFind Flags (DevFind_t.Expr)
 */
#define DFF_NOTYPE	0x001			/* Don't match on Type */

/*
 * SoftInfo Find query
 */
typedef struct {
    /* Conditions to search for */
    SoftInfo_t	       *Addr;			/* Address of entry */
    char	       *Name;			/* Name of pkg */
    char	       *Version;		/* Version of pkg */
    /* Operators */
    SoftInfo_t	       *Tree;			/* Search Tree */
    int			Expr;			/* DFE_* Expression Flags */
    char		Reason[128];		/* List of what matched */
} SoftInfoFind_t;

/*
 * Now include the declarations
 */
#include "declare.h"
#include "probe.h"

#endif	/* __mysysinfo_lib_defs__ */
