/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_VLDBINT_
#define	_RXGEN_VLDBINT_

#ifdef	KERNEL
/* The following 'ifndefs' are not a good solution to the vendor's omission of surrounding all system includes with 'ifndef's since it requires that this file is included after the system includes...*/
#include "../afs/param.h"
#include "../h/types.h"
#ifndef	SOCK_DGRAM  /* XXXXX */
#include "../h/socket.h"
#endif
#ifndef	DTYPE_SOCKET  /* XXXXX */
#ifdef AFS_DEC_ENV
#include "../h/smp_lock.h"
#endif
#include "../h/file.h"
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
#include "../rpc/types.h"
#ifndef	XDR_GETLONG /* XXXXX */
#include "../rpc/xdr.h"
#endif
#include "../afsint/rxgen_consts.h"
#include "../afs/osi.h"
#include "../rx/rx.h"
#else	/* KERNEL */
#include <sys/types.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <afs/rxgen_consts.h>
#endif	/* KERNEL */

#include	"vl_opcodes.h"	/* directly to other places */
#define VLDBVERSION_4 4
#define VLDBVERSION 3
#define OVLDBVERSION 2
#define VL_MAXNAMELEN 65
#define OMAXNSERVERS 8
#define NMAXNSERVERS 13
#define MAXTYPES 3

struct VldbUpdateEntry {
	u_int32 Mask;
	char name[VL_MAXNAMELEN];
	int32 spares3;
	int32 flags;
	u_int32 ReadOnlyId;
	u_int32 BackupId;
	int32 cloneId;
	int32 nModifiedRepsites;
	u_int32 RepsitesMask[OMAXNSERVERS];
	int32 RepsitesTargetServer[OMAXNSERVERS];
	int32 RepsitesTargetPart[OMAXNSERVERS];
	int32 RepsitesNewServer[OMAXNSERVERS];
	int32 RepsitesNewPart[OMAXNSERVERS];
	int32 RepsitesNewFlags[OMAXNSERVERS];
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
	u_int32 Mask;
	int32 server;
	int32 partition;
	int32 spares3;
	int32 volumeid;
	int32 flag;
};
typedef struct VldbListByAttributes VldbListByAttributes;
bool_t xdr_VldbListByAttributes();

#define VLLIST_SERVER 0x1
#define VLLIST_PARTITION 0x2
#define VLLIST_VOLUMEID 0x8
#define VLLIST_FLAG 0x10

struct vldbentry {
	char name[VL_MAXNAMELEN];
	int32 spares3;
	int32 nServers;
	int32 serverNumber[OMAXNSERVERS];
	int32 serverPartition[OMAXNSERVERS];
	int32 serverFlags[OMAXNSERVERS];
	u_int32 volumeId[MAXTYPES];
	int32 cloneId;
	int32 flags;
};
typedef struct vldbentry vldbentry;
bool_t xdr_vldbentry();


struct nvldbentry {
	char name[VL_MAXNAMELEN];
	int32 nServers;
	int32 serverNumber[NMAXNSERVERS];
	int32 serverPartition[NMAXNSERVERS];
	int32 serverFlags[NMAXNSERVERS];
	u_int32 volumeId[MAXTYPES];
	int32 cloneId;
	int32 flags;
	int32 spares1;
	int32 spares2;
	int32 spares3;
	int32 spares4;
	int32 spares5;
	int32 spares6;
	int32 spares7;
	int32 spares8;
	int32 spares9;
};
typedef struct nvldbentry nvldbentry;
bool_t xdr_nvldbentry();


struct afsUUID {
	u_int32 time_low;
	u_short time_mid;
	u_short time_hi_and_version;
	char clock_seq_hi_and_reserved;
	char clock_seq_low;
	char node[6];
};
typedef struct afsUUID afsUUID;
bool_t xdr_afsUUID();


struct ListAddrByAttributes {
	int32 Mask;
	u_int32 ipaddr;
	int32 index;
	int32 spare1;
	afsUUID uuid;
};
typedef struct ListAddrByAttributes ListAddrByAttributes;
bool_t xdr_ListAddrByAttributes();

#define VLADDR_IPADDR 0x1
#define VLADDR_INDEX 0x2
#define VLADDR_UUID 0x4

struct uvldbentry {
	char name[VL_MAXNAMELEN];
	int32 nServers;
	afsUUID serverNumber[NMAXNSERVERS];
	int32 serverUnique[NMAXNSERVERS];
	int32 serverPartition[NMAXNSERVERS];
	int32 serverFlags[NMAXNSERVERS];
	u_int32 volumeId[MAXTYPES];
	int32 cloneId;
	int32 flags;
	int32 spares1;
	int32 spares2;
	int32 spares3;
	int32 spares4;
	int32 spares5;
	int32 spares6;
	int32 spares7;
	int32 spares8;
	int32 spares9;
};
typedef struct uvldbentry uvldbentry;
bool_t xdr_uvldbentry();


struct vital_vlheader {
	int32 vldbversion;
	int32 headersize;
	int32 freePtr;
	int32 eofPtr;
	int32 allocs;
	int32 frees;
	int32 MaxVolumeId;
	int32 totalEntries[MAXTYPES];
};
typedef struct vital_vlheader vital_vlheader;
bool_t xdr_vital_vlheader();

#define MAX_NUMBER_OPCODES 50

struct vldstats {
	u_int32 start_time;
	int32 requests[MAX_NUMBER_OPCODES];
	int32 aborts[MAX_NUMBER_OPCODES];
	int32 reserved[5];
};
typedef struct vldstats vldstats;
bool_t xdr_vldstats();

#define	VLF_RWEXISTS	    0x1000  /* flags for whole vldb entry */
#define	VLF_ROEXISTS	    0x2000
#define	VLF_BACKEXISTS	    0x4000
#define	VLSF_NEWREPSITE	    0x01    /* flags for indiv. server entry */
#define	VLSF_ROVOL	    0x02
#define	VLSF_RWVOL	    0x04
#define	VLSF_BACKVOL	    0x08

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
	u_int32 *bulkaddrs_val;
} bulkaddrs;
bool_t xdr_bulkaddrs();


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


typedef struct single_uvldbentry *uvldblist;
bool_t xdr_uvldblist();


struct single_uvldbentry {
	uvldbentry VldbEntry;
	uvldblist next_vldb;
};
typedef struct single_uvldbentry single_uvldbentry;
bool_t xdr_single_uvldbentry();


struct uvldb_list {
	uvldblist node;
};
typedef struct uvldb_list uvldb_list;
bool_t xdr_uvldb_list();


typedef struct VLCallBack *vlcallback;
bool_t xdr_vlcallback();


struct VLCallBack {
	u_int32 CallBackVersion;
	u_int32 ExpirationTime;
	u_int32 CallBackType;
	u_int32 Handle;
};
typedef struct VLCallBack VLCallBack;
bool_t xdr_VLCallBack();


/* Opcode-related useful stats for package: VL_ */
#define VL_LOWEST_OPCODE   501
#define VL_HIGHEST_OPCODE	533
#define VL_NUMBER_OPCODES	27

#endif	/* _RXGEN_VLDBINT_ */
