/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_VOLINT_
#define	_RXGEN_VOLINT_

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

#define AFSVolSTATINDEX 16
#define SIZE 1024

struct volser_status {
	afs_int32 volID;
	afs_int32 nextUnique;
	int type;
	afs_int32 parentID;
	afs_int32 cloneID;
	afs_int32 backupID;
	afs_int32 restoredFromID;
	afs_int32 maxQuota;
	afs_int32 minQuota;
	afs_int32 owner;
	afs_int32 creationDate;
	afs_int32 accessDate;
	afs_int32 updateDate;
	afs_int32 expirationDate;
	afs_int32 backupDate;
	afs_int32 copyDate;
};
typedef struct volser_status volser_status;
bool_t xdr_volser_status();


struct destServer {
	afs_int32 destHost;
	afs_int32 destPort;
	afs_int32 destSSID;
};
typedef struct destServer destServer;
bool_t xdr_destServer();


struct volintInfo {
	char name[32];
	afs_int32 volid;
	afs_int32 type;
	afs_int32 backupID;
	afs_int32 parentID;
	afs_int32 cloneID;
	afs_int32 status;
	afs_int32 copyDate;
	u_char inUse;
	u_char needsSalvaged;
	u_char destroyMe;
	afs_int32 creationDate;
	afs_int32 accessDate;
	afs_int32 updateDate;
	afs_int32 backupDate;
	int dayUse;
	int filecount;
	int maxquota;
	int size;
	afs_int32 flags;
	afs_int32 spare0;
	afs_int32 spare1;
	afs_int32 spare2;
	afs_int32 spare3;
};
typedef struct volintInfo volintInfo;
bool_t xdr_volintInfo();

#define VOLINT_STATS_NUM_RWINFO_FIELDS 4
#define VOLINT_STATS_SAME_NET 0
#define VOLINT_STATS_SAME_NET_AUTH 1
#define VOLINT_STATS_DIFF_NET 2
#define VOLINT_STATS_DIFF_NET_AUTH 3
#define VOLINT_STATS_NUM_TIME_RANGES 6
#define VOLINT_STATS_TIME_CAP_0 60
#define VOLINT_STATS_TIME_CAP_1 600
#define VOLINT_STATS_TIME_CAP_2 3600
#define VOLINT_STATS_TIME_CAP_3 86400
#define VOLINT_STATS_TIME_CAP_4 604800
#define VOLINT_STATS_NUM_TIME_FIELDS 6
#define VOLINT_STATS_TIME_IDX_0 0
#define VOLINT_STATS_TIME_IDX_1 1
#define VOLINT_STATS_TIME_IDX_2 2
#define VOLINT_STATS_TIME_IDX_3 3
#define VOLINT_STATS_TIME_IDX_4 4
#define VOLINT_STATS_TIME_IDX_5 5

struct volintXInfo {
	char name[32];
	afs_int32 volid;
	afs_int32 type;
	afs_int32 backupID;
	afs_int32 parentID;
	afs_int32 cloneID;
	afs_int32 status;
	afs_int32 copyDate;
	u_char inUse;
	afs_int32 creationDate;
	afs_int32 accessDate;
	afs_int32 updateDate;
	afs_int32 backupDate;
	int dayUse;
	int filecount;
	int maxquota;
	int size;
	afs_int32 stat_reads[VOLINT_STATS_NUM_RWINFO_FIELDS];
	afs_int32 stat_writes[VOLINT_STATS_NUM_RWINFO_FIELDS];
	afs_int32 stat_fileSameAuthor[VOLINT_STATS_NUM_TIME_FIELDS];
	afs_int32 stat_fileDiffAuthor[VOLINT_STATS_NUM_TIME_FIELDS];
	afs_int32 stat_dirSameAuthor[VOLINT_STATS_NUM_TIME_FIELDS];
	afs_int32 stat_dirDiffAuthor[VOLINT_STATS_NUM_TIME_FIELDS];
};
typedef struct volintXInfo volintXInfo;
bool_t xdr_volintXInfo();


struct transDebugInfo {
	afs_int32 tid;
	afs_int32 time;
	afs_int32 creationTime;
	afs_int32 returnCode;
	afs_int32 volid;
	afs_int32 partition;
	short iflags;
	char vflags;
	char tflags;
	char lastProcName[30];
	int callValid;
	afs_int32 readNext;
	afs_int32 transmitNext;
	int lastSendTime;
	int lastReceiveTime;
};
typedef struct transDebugInfo transDebugInfo;
bool_t xdr_transDebugInfo();


struct pIDs {
	afs_int32 partIds[26];
};
typedef struct pIDs pIDs;
bool_t xdr_pIDs();


struct diskPartition {
	char name[32];
	char devName[32];
	int lock_fd;
	int totalUsable;
	int free;
	int minFree;
};
typedef struct diskPartition diskPartition;
bool_t xdr_diskPartition();


struct restoreCookie {
	char name[32];
	afs_int32 type;
	afs_int32 clone;
	afs_int32 parent;
};
typedef struct restoreCookie restoreCookie;
bool_t xdr_restoreCookie();


struct replica {
	afs_int32 trans;
	struct destServer server;
};
typedef struct replica replica;
bool_t xdr_replica();


typedef struct manyDests {
	u_int manyDests_len;
	replica *manyDests_val;
} manyDests;
bool_t xdr_manyDests();


typedef struct manyResults {
	u_int manyResults_len;
	afs_int32 *manyResults_val;
} manyResults;
bool_t xdr_manyResults();


typedef struct transDebugEntries {
	u_int transDebugEntries_len;
	transDebugInfo *transDebugEntries_val;
} transDebugEntries;
bool_t xdr_transDebugEntries();


typedef struct volEntries {
	u_int volEntries_len;
	volintInfo *volEntries_val;
} volEntries;
bool_t xdr_volEntries();


typedef struct partEntries {
	u_int partEntries_len;
	afs_int32 *partEntries_val;
} partEntries;
bool_t xdr_partEntries();


typedef struct volXEntries {
	u_int volXEntries_len;
	volintXInfo *volXEntries_val;
} volXEntries;
bool_t xdr_volXEntries();


/* Opcode-related useful stats for package: AFSVol */
#define AFSVolLOWEST_OPCODE   100
#define AFSVolHIGHEST_OPCODE	128
#define AFSVolNUMBER_OPCODES	29

#define AFSVolNO_OF_STAT_FUNCS	29

AFS_RXGEN_EXPORT
extern const char *AFSVolfunction_names[];

#endif	/* _RXGEN_VOLINT_ */
