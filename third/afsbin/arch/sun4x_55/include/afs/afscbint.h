/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_AFSCBINT_
#define	_RXGEN_AFSCBINT_

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

#ifndef FSINT_COMMON_XG

struct AFSFid {
	u_int32 Volume;
	u_int32 Vnode;
	u_int32 Unique;
};
typedef struct AFSFid AFSFid;
bool_t xdr_AFSFid();


struct AFSCallBack {
	u_int32 CallBackVersion;
	u_int32 ExpirationTime;
	u_int32 CallBackType;
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
	int32 addr;
	int32 cell;
	AFSFid netFid;
	int32 Length;
	int32 DataVersion;
	struct AFSDBLockDesc lock;
	int32 callback;
	int32 cbExpires;
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
	int32 *AFSCB_CollData_val;
} AFSCB_CollData;
bool_t xdr_AFSCB_CollData();


typedef struct AFS_CollData {
	u_int AFS_CollData_len;
	int32 *AFS_CollData_val;
} AFS_CollData;
bool_t xdr_AFS_CollData();

#define AFSCB_XSTATSCOLL_CALL_INFO 0
#define AFSCB_XSTATSCOLL_PERF_INFO 1
#define AFSCB_XSTATSCOLL_FULL_PERF_INFO 2
#define AFS_XSTATSCOLL_CALL_INFO 0
#define AFS_XSTATSCOLL_PERF_INFO 1
#define AFS_XSTATSCOLL_FULL_PERF_INFO 2

typedef u_int32 VolumeId;
bool_t xdr_VolumeId();


typedef u_int32 VolId;
bool_t xdr_VolId();


typedef u_int32 VnodeId;
bool_t xdr_VnodeId();


typedef u_int32 Unique;
bool_t xdr_Unique();


typedef u_int32 UserId;
bool_t xdr_UserId();


typedef u_int32 FileVersion;
bool_t xdr_FileVersion();


typedef u_int32 Date;
bool_t xdr_Date();


typedef int32 ErrorCode;
bool_t xdr_ErrorCode();


typedef int32 Rights;
bool_t xdr_Rights();

#define AFS_DISKNAMESIZE 32

typedef char DiskName[AFS_DISKNAMESIZE];
bool_t xdr_DiskName();

#define CALLBACK_VERSION 1
#endif /* FSINT_COMMON_XG */
#ifdef KERNEL
#include "../afs/longc_procs.h"
#endif

#include <rx/rx_multi.h>
#define multi_RXAFSCB_CallBack(Fids_Array, CallBacks_Array) \
	multi_Body(StartRXAFSCB_CallBack(multi_call, Fids_Array, CallBacks_Array), EndRXAFSCB_CallBack(multi_call))


/* Opcode-related useful stats for package: RXAFSCB_ */
#define RXAFSCB_LOWEST_OPCODE   204
#define RXAFSCB_HIGHEST_OPCODE	210
#define RXAFSCB_NUMBER_OPCODES	7

#endif	/* _RXGEN_AFSCBINT_ */
