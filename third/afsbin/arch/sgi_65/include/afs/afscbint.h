/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_AFSCBINT_
#define	_RXGEN_AFSCBINT_

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

#ifndef FSINT_COMMON_XG

struct AFSFid {
	afs_uint32 Volume;
	afs_uint32 Vnode;
	afs_uint32 Unique;
};
typedef struct AFSFid AFSFid;
bool_t xdr_AFSFid();


struct AFSCallBack {
	afs_uint32 CallBackVersion;
	afs_uint32 ExpirationTime;
	afs_uint32 CallBackType;
};
typedef struct AFSCallBack AFSCallBack;
bool_t xdr_AFSCallBack();


struct AFSDBLockDesc {
	char waitStates;
	char exclLocked;
	short readersReading;
	short numWaiting;
	short spare;
	int pid_last_reader;
	int pid_writer;
	int src_indicator;
};
typedef struct AFSDBLockDesc AFSDBLockDesc;
bool_t xdr_AFSDBLockDesc();


struct AFSDBCacheEntry {
	afs_int32 addr;
	afs_int32 cell;
	AFSFid netFid;
	afs_int32 Length;
	afs_int32 DataVersion;
	struct AFSDBLockDesc lock;
	afs_int32 callback;
	afs_int32 cbExpires;
	short refCount;
	short opens;
	short writers;
	char mvstat;
	char states;
};
typedef struct AFSDBCacheEntry AFSDBCacheEntry;
bool_t xdr_AFSDBCacheEntry();


struct AFSDBLock {
	char name[16];
	struct AFSDBLockDesc lock;
};
typedef struct AFSDBLock AFSDBLock;
bool_t xdr_AFSDBLock();

#define CB_EXCLUSIVE 1
#define CB_SHARED 2
#define CB_DROPPED 3
#define AFSNAMEMAX 256
#define AFSPATHMAX 1024
#define AFSOPAQUEMAX 1024

typedef struct AFSOpaque {
	u_int AFSOpaque_len;
	char *AFSOpaque_val;
} AFSOpaque;
bool_t xdr_AFSOpaque();

#define AFSCBMAX 50

typedef struct AFSCBFids {
	u_int AFSCBFids_len;
	AFSFid *AFSCBFids_val;
} AFSCBFids;
bool_t xdr_AFSCBFids();


typedef struct AFSCBs {
	u_int AFSCBs_len;
	AFSCallBack *AFSCBs_val;
} AFSCBs;
bool_t xdr_AFSCBs();

#define AFSCB_XSTAT_VERSION 2
#define AFS_XSTAT_VERSION 2
#define AFSCB_MAX_XSTAT_LONGS 2048
#define AFS_MAX_XSTAT_LONGS 1024

typedef struct AFSCB_CollData {
	u_int AFSCB_CollData_len;
	afs_int32 *AFSCB_CollData_val;
} AFSCB_CollData;
bool_t xdr_AFSCB_CollData();


typedef struct AFS_CollData {
	u_int AFS_CollData_len;
	afs_int32 *AFS_CollData_val;
} AFS_CollData;
bool_t xdr_AFS_CollData();

#define AFSCB_XSTATSCOLL_CALL_INFO 0
#define AFSCB_XSTATSCOLL_PERF_INFO 1
#define AFSCB_XSTATSCOLL_FULL_PERF_INFO 2
#define AFS_XSTATSCOLL_CALL_INFO 0
#define AFS_XSTATSCOLL_PERF_INFO 1
#define AFS_XSTATSCOLL_FULL_PERF_INFO 2

typedef afs_uint32 VolumeId;
bool_t xdr_VolumeId();


typedef afs_uint32 VolId;
bool_t xdr_VolId();


typedef afs_uint32 VnodeId;
bool_t xdr_VnodeId();


typedef afs_uint32 Unique;
bool_t xdr_Unique();


typedef afs_uint32 UserId;
bool_t xdr_UserId();


typedef afs_uint32 FileVersion;
bool_t xdr_FileVersion();


typedef afs_int32 ErrorCode;
bool_t xdr_ErrorCode();


typedef afs_int32 Rights;
bool_t xdr_Rights();

#define AFS_DISKNAMESIZE 32

typedef char DiskName[AFS_DISKNAMESIZE];
bool_t xdr_DiskName();

#define CALLBACK_VERSION 1
#define AFS_MAX_INTERFACE_ADDR 32

struct interfaceAddr {
	int numberOfInterfaces;
	afsUUID uuid;
	afs_int32 addr_in[AFS_MAX_INTERFACE_ADDR];
	afs_int32 subnetmask[AFS_MAX_INTERFACE_ADDR];
	afs_int32 mtu[AFS_MAX_INTERFACE_ADDR];
};
typedef struct interfaceAddr interfaceAddr;
bool_t xdr_interfaceAddr();

#define AFSMAXCELLHOSTS 8

typedef afs_int32 serverList[AFSMAXCELLHOSTS];
bool_t xdr_serverList();


typedef struct cacheConfig {
	u_int cacheConfig_len;
	afs_uint32 *cacheConfig_val;
} cacheConfig;
bool_t xdr_cacheConfig();

#endif /* FSINT_COMMON_XG */
#ifdef KERNEL
#include "../afs/longc_procs.h"
#endif
#define RXAFSCB_STATINDEX 6

#include <rx/rx_multi.h>
#define multi_RXAFSCB_CallBack(Fids_Array, CallBacks_Array) \
	multi_Body(StartRXAFSCB_CallBack(multi_call, Fids_Array, CallBacks_Array), EndRXAFSCB_CallBack(multi_call))


#define multi_RXAFSCB_Probe() \
	multi_Body(StartRXAFSCB_Probe(multi_call), EndRXAFSCB_Probe(multi_call))


#define multi_RXAFSCB_ProbeUuid(clientUuid) \
	multi_Body(StartRXAFSCB_ProbeUuid(multi_call, clientUuid), EndRXAFSCB_ProbeUuid(multi_call))


/* Opcode-related useful stats for package: RXAFSCB_ */
#define RXAFSCB_LOWEST_OPCODE   204
#define RXAFSCB_HIGHEST_OPCODE	218
#define RXAFSCB_NUMBER_OPCODES	15

#define RXAFSCB_NO_OF_STAT_FUNCS	15

AFS_RXGEN_EXPORT
extern const char *RXAFSCB_function_names[];

#endif	/* _RXGEN_AFSCBINT_ */
