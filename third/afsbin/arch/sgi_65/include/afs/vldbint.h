/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_VLDBINT_
#define	_RXGEN_VLDBINT_

#ifdef	KERNEL
/* The following 'ifndefs' are not a good solution to the vendor's omission of surrounding all system includes with 'ifndef's since it requires that this file is included after the system includes...*/
#include "../afs/param.h"
#ifdef	UKERNEL
#include "../afs/sysincludes.h"
#include "../rx/xdr.h"
#include "../rx/rx.h"
#include "../rx/rx_globals.h"
#else	/* UKERNEL */
#include "../h/types.h"
#ifndef	SOCK_DGRAM  /* XXXXX */
#include "../h/socket.h"
#endif
#ifndef	DTYPE_SOCKET  /* XXXXX */
#ifdef AFS_DEC_ENV
#include "../h/smp_lock.h"
#endif
#ifndef AFS_LINUX22_ENV
#include "../h/file.h"
#endif
#endif
#ifndef	S_IFMT  /* XXXXX */
#include "../h/stat.h"
#endif
#ifndef	IPPROTO_UDP /* XXXXX */
#include "../netinet/in.h"
#endif
#ifndef	DST_USA  /* XXXXX */
#include "../h/time.h"
#endif
#ifndef AFS_LINUX22_ENV
#include "../rpc/types.h"
#endif /* AFS_LINUX22_ENV */
#ifndef	XDR_GETLONG /* XXXXX */
#ifdef AFS_LINUX22_ENV
#ifndef quad_t
#define quad_t __quad_t
#define u_quad_t __u_quad_t
#endif
#endif
#ifdef AFS_LINUX22_ENV
#include "../rx/xdr.h"
#else /* AFS_LINUX22_ENV */
#include "../rpc/xdr.h"
#endif /* AFS_LINUX22_ENV */
#endif /* XDR_GETLONG */
#endif   /* UKERNEL */
#include "../afsint/rxgen_consts.h"
#include "../afs/afs_osi.h"
#include "../rx/rx.h"
#include "../rx/rx_globals.h"
#else	/* KERNEL */
#include <afs/param.h>
#include <afs/stds.h>
#include <sys/types.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <rx/rx_globals.h>
#include <afs/rxgen_consts.h>
#endif	/* KERNEL */

#ifdef AFS_NT40_ENV
#ifndef AFS_RXGEN_EXPORT
#define AFS_RXGEN_EXPORT __declspec(dllimport)
#endif /* AFS_RXGEN_EXPORT */
#else /* AFS_NT40_ENV */
#define AFS_RXGEN_EXPORT
#endif /* AFS_NT40_ENV */

#define VL_STATINDEX 15
#include	"vl_opcodes.h"	/* directly to other places */
#ifdef KERNEL
#define	xdr_array(a,b,c,d,e,f)	xdr_arrayN(a,b,c,d,e,f)
#include "../afs/longc_procs.h"
#endif
#define VLDBVERSION_4 4
#define VLDBVERSION 3
#define OVLDBVERSION 2
#define VL_MAXNAMELEN 65
#define OMAXNSERVERS 8
#define NMAXNSERVERS 13
#define MAXTYPES 3

struct VldbUpdateEntry {
	afs_uint32 Mask;
	char name[VL_MAXNAMELEN];
	afs_int32 spares3;
	afs_int32 flags;
	afs_uint32 ReadOnlyId;
	afs_uint32 BackupId;
	afs_int32 cloneId;
	afs_int32 nModifiedRepsites;
	afs_uint32 RepsitesMask[OMAXNSERVERS];
	afs_int32 RepsitesTargetServer[OMAXNSERVERS];
	afs_int32 RepsitesTargetPart[OMAXNSERVERS];
	afs_int32 RepsitesNewServer[OMAXNSERVERS];
	afs_int32 RepsitesNewPart[OMAXNSERVERS];
	afs_int32 RepsitesNewFlags[OMAXNSERVERS];
};
typedef struct VldbUpdateEntry VldbUpdateEntry;
bool_t xdr_VldbUpdateEntry();

#define VLUPDATE_VOLUMENAME 0x0001
#define VLUPDATE_FLAGS 0x0004
#define VLUPDATE_READONLYID 0x0008
#define VLUPDATE_BACKUPID 0x0010
#define VLUPDATE_REPSITES 0x0020
#define VLUPDATE_CLONEID 0x0080
#define VLUPDATE_VOLNAMEHASH 0x0100
#define VLUPDATE_RWID 0x0200
#define VLUPDATE_REPS_DELETE 0x0100
#define VLUPDATE_REPS_ADD 0x0200
#define VLUPDATE_REPS_MODSERV 0x0400
#define VLUPDATE_REPS_MODPART 0x0800
#define VLUPDATE_REPS_MODFLAG 0x1000
#define VLSERVER_FLAG_UUID 0x0010
#define DEFAULTBULK 10000

typedef struct bulk {
	u_int bulk_len;
	char *bulk_val;
} bulk;
bool_t xdr_bulk();


struct VldbListByAttributes {
	afs_uint32 Mask;
	afs_int32 server;
	afs_int32 partition;
	afs_int32 spares3;
	afs_int32 volumeid;
	afs_int32 flag;
};
typedef struct VldbListByAttributes VldbListByAttributes;
bool_t xdr_VldbListByAttributes();

#define VLLIST_SERVER 0x1
#define VLLIST_PARTITION 0x2
#define VLLIST_VOLUMEID 0x8
#define VLLIST_FLAG 0x10

struct vldbentry {
	char name[VL_MAXNAMELEN];
	afs_int32 spares3;
	afs_int32 nServers;
	afs_int32 serverNumber[OMAXNSERVERS];
	afs_int32 serverPartition[OMAXNSERVERS];
	afs_int32 serverFlags[OMAXNSERVERS];
	afs_uint32 volumeId[MAXTYPES];
	afs_int32 cloneId;
	afs_int32 flags;
};
typedef struct vldbentry vldbentry;
bool_t xdr_vldbentry();


struct nvldbentry {
	char name[VL_MAXNAMELEN];
	afs_int32 nServers;
	afs_int32 serverNumber[NMAXNSERVERS];
	afs_int32 serverPartition[NMAXNSERVERS];
	afs_int32 serverFlags[NMAXNSERVERS];
	afs_uint32 volumeId[MAXTYPES];
	afs_int32 cloneId;
	afs_int32 flags;
	afs_int32 matchindex;
	afs_int32 spares2;
	afs_int32 spares3;
	afs_int32 spares4;
	afs_int32 spares5;
	afs_int32 spares6;
	afs_int32 spares7;
	afs_int32 spares8;
	afs_int32 spares9;
};
typedef struct nvldbentry nvldbentry;
bool_t xdr_nvldbentry();


struct ListAddrByAttributes {
	afs_int32 Mask;
	afs_uint32 ipaddr;
	afs_int32 index;
	afs_int32 spare1;
	afsUUID uuid;
};
typedef struct ListAddrByAttributes ListAddrByAttributes;
bool_t xdr_ListAddrByAttributes();

#define VLADDR_IPADDR 0x1
#define VLADDR_INDEX 0x2
#define VLADDR_UUID 0x4

struct uvldbentry {
	char name[VL_MAXNAMELEN];
	afs_int32 nServers;
	afsUUID serverNumber[NMAXNSERVERS];
	afs_int32 serverUnique[NMAXNSERVERS];
	afs_int32 serverPartition[NMAXNSERVERS];
	afs_int32 serverFlags[NMAXNSERVERS];
	afs_uint32 volumeId[MAXTYPES];
	afs_int32 cloneId;
	afs_int32 flags;
	afs_int32 spares1;
	afs_int32 spares2;
	afs_int32 spares3;
	afs_int32 spares4;
	afs_int32 spares5;
	afs_int32 spares6;
	afs_int32 spares7;
	afs_int32 spares8;
	afs_int32 spares9;
};
typedef struct uvldbentry uvldbentry;
bool_t xdr_uvldbentry();


struct vital_vlheader {
	afs_int32 vldbversion;
	afs_int32 headersize;
	afs_int32 freePtr;
	afs_int32 eofPtr;
	afs_int32 allocs;
	afs_int32 frees;
	afs_int32 MaxVolumeId;
	afs_int32 totalEntries[MAXTYPES];
};
typedef struct vital_vlheader vital_vlheader;
bool_t xdr_vital_vlheader();

#define MAX_NUMBER_OPCODES 50

struct vldstats {
	afs_uint32 start_time;
	afs_int32 requests[MAX_NUMBER_OPCODES];
	afs_int32 aborts[MAX_NUMBER_OPCODES];
	afs_int32 reserved[5];
};
typedef struct vldstats vldstats;
bool_t xdr_vldstats();

#define	VLF_RWEXISTS	    0x1000  /* flags for whole vldb entry */
#define	VLF_ROEXISTS	    0x2000
#define	VLF_BACKEXISTS	    0x4000
#define	VLF_DFSFILESET	    0x8000  /* Volume is really DFS fileset */
#define	VLSF_NEWREPSITE	    0x01    /* flags for indiv. server entry */
#define	VLSF_ROVOL	    0x02
#define	VLSF_RWVOL	    0x04
#define	VLSF_BACKVOL	    0x08
#define	VLSF_DONTUSE	    0x20    /* no conflict with VLSERVER_FLAG_UUID */

typedef struct bulkentries {
	u_int bulkentries_len;
	vldbentry *bulkentries_val;
} bulkentries;
bool_t xdr_bulkentries();


typedef struct nbulkentries {
	u_int nbulkentries_len;
	nvldbentry *nbulkentries_val;
} nbulkentries;
bool_t xdr_nbulkentries();


typedef struct ubulkentries {
	u_int ubulkentries_len;
	uvldbentry *ubulkentries_val;
} ubulkentries;
bool_t xdr_ubulkentries();


typedef struct bulkaddrs {
	u_int bulkaddrs_len;
	afs_uint32 *bulkaddrs_val;
} bulkaddrs;
bool_t xdr_bulkaddrs();


struct VLCallBack {
	afs_uint32 CallBackVersion;
	afs_uint32 ExpirationTime;
	afs_uint32 CallBackType;
	afs_uint32 Handle;
};
typedef struct VLCallBack VLCallBack;
bool_t xdr_VLCallBack();

#if !defined(KERNEL)

typedef struct single_vldbentry *vldblist;
bool_t xdr_vldblist();


struct single_vldbentry {
	vldbentry VldbEntry;
	vldblist next_vldb;
};
typedef struct single_vldbentry single_vldbentry;
bool_t xdr_single_vldbentry();


struct vldb_list {
	vldblist node;
};
typedef struct vldb_list vldb_list;
bool_t xdr_vldb_list();


typedef struct single_nvldbentry *nvldblist;
bool_t xdr_nvldblist();


struct single_nvldbentry {
	nvldbentry VldbEntry;
	nvldblist next_vldb;
};
typedef struct single_nvldbentry single_nvldbentry;
bool_t xdr_single_nvldbentry();


struct nvldb_list {
	nvldblist node;
};
typedef struct nvldb_list nvldb_list;
bool_t xdr_nvldb_list();

#endif /* !defined(KERNEL) */

/* Opcode-related useful stats for package: VL_ */
#define VL_LOWEST_OPCODE   501
#define VL_HIGHEST_OPCODE	534
#define VL_NUMBER_OPCODES	28

#define VL_NO_OF_STAT_FUNCS	28

AFS_RXGEN_EXPORT
extern const char *VL_function_names[];

#endif	/* _RXGEN_VLDBINT_ */
