/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_AFSINT_
#define	_RXGEN_AFSINT_

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
#define VICECONNBAD	1234
#define VICETOKENDEAD	1235
#define AFS_LOCKWAIT	(5*60)

struct CBS {
	afs_int32 SeqLen;
	char *SeqBody;
};
typedef struct CBS CBS;
bool_t xdr_CBS();


struct BBS {
	afs_int32 MaxSeqLen;
	afs_int32 SeqLen;
	char *SeqBody;
};
typedef struct BBS BBS;
bool_t xdr_BBS();


struct AFSAccessList {
	afs_int32 MaxSeqLen;
	afs_int32 SeqLen;
	char *SeqBody;
};
typedef struct AFSAccessList AFSAccessList;
bool_t xdr_AFSAccessList();


typedef AFSFid ViceFid;
bool_t xdr_ViceFid();


typedef afs_int32 ViceDataType;
bool_t xdr_ViceDataType();

#define Invalid 0
#define File 1 
#define Directory 2 
#define SymbolicLink 3 
#ifdef	KERNEL
#define	xdr_array(a,b,c,d,e,f)	xdr_arrayN(a,b,c,d,e,f)
#endif

struct BD {
	afs_int32 host;
	afs_int32 portal;
	afs_int32 session;
};
typedef struct BD BD;
bool_t xdr_BD();


struct AFSVolSync {
	afs_uint32 spare1;
	afs_uint32 spare2;
	afs_uint32 spare3;
	afs_uint32 spare4;
	afs_uint32 spare5;
	afs_uint32 spare6;
};
typedef struct AFSVolSync AFSVolSync;
bool_t xdr_AFSVolSync();


struct AFSOldFetchStatus {
	afs_uint32 InterfaceVersion;
	afs_uint32 FileType;
	afs_uint32 LinkCount;
	afs_uint32 Length;
	afs_uint32 DataVersion;
	afs_uint32 Author;
	afs_uint32 Owner;
	afs_uint32 CallerAccess;
	afs_uint32 AnonymousAccess;
	afs_uint32 UnixModeBits;
	afs_uint32 ParentVnode;
	afs_uint32 ParentUnique;
	afs_uint32 SegSize;
	afs_uint32 ClientModTime;
	afs_uint32 ServerModTime;
	afs_uint32 Group;
};
typedef struct AFSOldFetchStatus AFSOldFetchStatus;
bool_t xdr_AFSOldFetchStatus();


struct AFSFetchStatus {
	afs_uint32 InterfaceVersion;
	afs_uint32 FileType;
	afs_uint32 LinkCount;
	afs_uint32 Length;
	afs_uint32 DataVersion;
	afs_uint32 Author;
	afs_uint32 Owner;
	afs_uint32 CallerAccess;
	afs_uint32 AnonymousAccess;
	afs_uint32 UnixModeBits;
	afs_uint32 ParentVnode;
	afs_uint32 ParentUnique;
	afs_uint32 SegSize;
	afs_uint32 ClientModTime;
	afs_uint32 ServerModTime;
	afs_uint32 Group;
	afs_uint32 SyncCounter;
	afs_uint32 dataVersionHigh;
	afs_uint32 spare2;
	afs_uint32 spare3;
	afs_uint32 spare4;
};
typedef struct AFSFetchStatus AFSFetchStatus;
bool_t xdr_AFSFetchStatus();


struct AFSStoreStatus {
	afs_uint32 Mask;
	afs_uint32 ClientModTime;
	afs_uint32 Owner;
	afs_uint32 Group;
	afs_uint32 UnixModeBits;
	afs_uint32 SegSize;
};
typedef struct AFSStoreStatus AFSStoreStatus;
bool_t xdr_AFSStoreStatus();

#define	AFS_SETMODTIME	1
#define	AFS_SETOWNER	2
#define	AFS_SETGROUP		4
#define	AFS_SETMODE		8
#define	AFS_SETSEGSIZE	16
#define	AFS_FSYNC       1024

typedef afs_int32 ViceVolumeType;
bool_t xdr_ViceVolumeType();

#define ReadOnly 0
#define ReadWrite 1

struct ViceDisk {
	afs_int32 BlocksAvailable;
	afs_int32 TotalBlocks;
	DiskName Name;
};
typedef struct ViceDisk ViceDisk;
bool_t xdr_ViceDisk();


struct ViceStatistics {
	afs_uint32 CurrentMsgNumber;
	afs_uint32 OldestMsgNumber;
	afs_uint32 CurrentTime;
	afs_uint32 BootTime;
	afs_uint32 StartTime;
	afs_int32 CurrentConnections;
	afs_uint32 TotalViceCalls;
	afs_uint32 TotalFetchs;
	afs_uint32 FetchDatas;
	afs_uint32 FetchedBytes;
	afs_int32 FetchDataRate;
	afs_uint32 TotalStores;
	afs_uint32 StoreDatas;
	afs_uint32 StoredBytes;
	afs_int32 StoreDataRate;
	afs_uint32 TotalRPCBytesSent;
	afs_uint32 TotalRPCBytesReceived;
	afs_uint32 TotalRPCPacketsSent;
	afs_uint32 TotalRPCPacketsReceived;
	afs_uint32 TotalRPCPacketsLost;
	afs_uint32 TotalRPCBogusPackets;
	afs_int32 SystemCPU;
	afs_int32 UserCPU;
	afs_int32 NiceCPU;
	afs_int32 IdleCPU;
	afs_int32 TotalIO;
	afs_int32 ActiveVM;
	afs_int32 TotalVM;
	afs_int32 EtherNetTotalErrors;
	afs_int32 EtherNetTotalWrites;
	afs_int32 EtherNetTotalInterupts;
	afs_int32 EtherNetGoodReads;
	afs_int32 EtherNetTotalBytesWritten;
	afs_int32 EtherNetTotalBytesRead;
	afs_int32 ProcessSize;
	afs_int32 WorkStations;
	afs_int32 ActiveWorkStations;
	afs_int32 Spare1;
	afs_int32 Spare2;
	afs_int32 Spare3;
	afs_int32 Spare4;
	afs_int32 Spare5;
	afs_int32 Spare6;
	afs_int32 Spare7;
	afs_int32 Spare8;
	ViceDisk Disk1;
	ViceDisk Disk2;
	ViceDisk Disk3;
	ViceDisk Disk4;
	ViceDisk Disk5;
	ViceDisk Disk6;
	ViceDisk Disk7;
	ViceDisk Disk8;
	ViceDisk Disk9;
	ViceDisk Disk10;
};
typedef struct ViceStatistics ViceStatistics;
bool_t xdr_ViceStatistics();


struct VolumeStatus {
	afs_int32 Vid;
	afs_int32 ParentId;
	char Online;
	char InService;
	char Blessed;
	char NeedsSalvage;
	afs_int32 Type;
	afs_int32 MinQuota;
	afs_int32 MaxQuota;
	afs_int32 BlocksInUse;
	afs_int32 PartBlocksAvail;
	afs_int32 PartMaxBlocks;
};
typedef struct VolumeStatus VolumeStatus;
bool_t xdr_VolumeStatus();


struct AFSFetchVolumeStatus {
	afs_int32 Vid;
	afs_int32 ParentId;
	char Online;
	char InService;
	char Blessed;
	char NeedsSalvage;
	afs_int32 Type;
	afs_int32 MinQuota;
	afs_int32 MaxQuota;
	afs_int32 BlocksInUse;
	afs_int32 PartBlocksAvail;
	afs_int32 PartMaxBlocks;
};
typedef struct AFSFetchVolumeStatus AFSFetchVolumeStatus;
bool_t xdr_AFSFetchVolumeStatus();


struct AFSStoreVolumeStatus {
	afs_int32 Mask;
	afs_int32 MinQuota;
	afs_int32 MaxQuota;
};
typedef struct AFSStoreVolumeStatus AFSStoreVolumeStatus;
bool_t xdr_AFSStoreVolumeStatus();

#define AFS_SETMINQUOTA 1
#define AFS_SETMAXQUOTA 2

struct AFSVolumeInfo {
	afs_uint32 Vid;
	afs_int32 Type;
	afs_uint32 Type0;
	afs_uint32 Type1;
	afs_uint32 Type2;
	afs_uint32 Type3;
	afs_uint32 Type4;
	afs_uint32 ServerCount;
	afs_uint32 Server0;
	afs_uint32 Server1;
	afs_uint32 Server2;
	afs_uint32 Server3;
	afs_uint32 Server4;
	afs_uint32 Server5;
	afs_uint32 Server6;
	afs_uint32 Server7;
	u_short Port0;
	u_short Port1;
	u_short Port2;
	u_short Port3;
	u_short Port4;
	u_short Port5;
	u_short Port6;
	u_short Port7;
};
typedef struct AFSVolumeInfo AFSVolumeInfo;
bool_t xdr_AFSVolumeInfo();


struct VolumeInfo {
	afs_uint32 Vid;
	afs_int32 Type;
	afs_uint32 Type0;
	afs_uint32 Type1;
	afs_uint32 Type2;
	afs_uint32 Type3;
	afs_uint32 Type4;
	afs_uint32 ServerCount;
	afs_uint32 Server0;
	afs_uint32 Server1;
	afs_uint32 Server2;
	afs_uint32 Server3;
	afs_uint32 Server4;
	afs_uint32 Server5;
	afs_uint32 Server6;
	afs_uint32 Server7;
};
typedef struct VolumeInfo VolumeInfo;
bool_t xdr_VolumeInfo();


typedef afs_int32 ViceLockType;
bool_t xdr_ViceLockType();


typedef struct AFSBulkStats {
	u_int AFSBulkStats_len;
	AFSFetchStatus *AFSBulkStats_val;
} AFSBulkStats;
bool_t xdr_AFSBulkStats();

#define LockRead		0
#define LockWrite		1
#define LockExtend	2
#define LockRelease	3

typedef afs_int32 ViceOfflineType;
bool_t xdr_ViceOfflineType();

#define NoSalvage 0
#define Salvage 1
#ifdef KERNEL
#include "../afs/longc_procs.h"
#endif
#define FLUSHMAX 10

typedef struct ViceIds {
	u_int ViceIds_len;
	afs_int32 *ViceIds_val;
} ViceIds;
bool_t xdr_ViceIds();


typedef struct IPAddrs {
	u_int IPAddrs_len;
	afs_int32 *IPAddrs_val;
} IPAddrs;
bool_t xdr_IPAddrs();

#define RXAFS_STATINDEX 7

/* Opcode-related useful stats for package: RXAFS_ */
#define RXAFS_LOWEST_OPCODE   130
#define RXAFS_HIGHEST_OPCODE	163
#define RXAFS_NUMBER_OPCODES	34

#define RXAFS_NO_OF_STAT_FUNCS	34

AFS_RXGEN_EXPORT
extern const char *RXAFS_function_names[];

#endif	/* _RXGEN_AFSINT_ */
