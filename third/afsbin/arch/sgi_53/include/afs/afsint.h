/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_AFSINT_
#define	_RXGEN_AFSINT_

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
	char readersReading;
	char numWaiting;
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
#define VICECONNBAD	1234
#define VICETOKENDEAD	1235
#define AFS_LOCKWAIT	(5*60)

struct CBS {
	int32 SeqLen;
	char *SeqBody;
};
typedef struct CBS CBS;
bool_t xdr_CBS();


struct BBS {
	int32 MaxSeqLen;
	int32 SeqLen;
	char *SeqBody;
};
typedef struct BBS BBS;
bool_t xdr_BBS();


struct AFSAccessList {
	int32 MaxSeqLen;
	int32 SeqLen;
	char *SeqBody;
};
typedef struct AFSAccessList AFSAccessList;
bool_t xdr_AFSAccessList();


typedef AFSFid ViceFid;
bool_t xdr_ViceFid();


typedef int32 ViceDataType;
bool_t xdr_ViceDataType();

#define Invalid 0
#define File 1 
#define Directory 2 
#define SymbolicLink 3 
#ifdef	KERNEL
#define	xdr_array(a,b,c,d,e,f)	xdr_arrayN(a,b,c,d,e,f)
#endif

struct BD {
	int32 host;
	int32 portal;
	int32 session;
};
typedef struct BD BD;
bool_t xdr_BD();


struct AFSVolSync {
	u_int32 spare1;
	u_int32 spare2;
	u_int32 spare3;
	u_int32 spare4;
	u_int32 spare5;
	u_int32 spare6;
};
typedef struct AFSVolSync AFSVolSync;
bool_t xdr_AFSVolSync();


struct AFSOldFetchStatus {
	u_int32 InterfaceVersion;
	u_int32 FileType;
	u_int32 LinkCount;
	u_int32 Length;
	u_int32 DataVersion;
	u_int32 Author;
	u_int32 Owner;
	u_int32 CallerAccess;
	u_int32 AnonymousAccess;
	u_int32 UnixModeBits;
	u_int32 ParentVnode;
	u_int32 ParentUnique;
	u_int32 SegSize;
	u_int32 ClientModTime;
	u_int32 ServerModTime;
	u_int32 Group;
};
typedef struct AFSOldFetchStatus AFSOldFetchStatus;
bool_t xdr_AFSOldFetchStatus();


struct AFSFetchStatus {
	u_int32 InterfaceVersion;
	u_int32 FileType;
	u_int32 LinkCount;
	u_int32 Length;
	u_int32 DataVersion;
	u_int32 Author;
	u_int32 Owner;
	u_int32 CallerAccess;
	u_int32 AnonymousAccess;
	u_int32 UnixModeBits;
	u_int32 ParentVnode;
	u_int32 ParentUnique;
	u_int32 SegSize;
	u_int32 ClientModTime;
	u_int32 ServerModTime;
	u_int32 Group;
	u_int32 SyncCounter;
	u_int32 dataVersionHigh;
	u_int32 spare2;
	u_int32 spare3;
	u_int32 spare4;
};
typedef struct AFSFetchStatus AFSFetchStatus;
bool_t xdr_AFSFetchStatus();


struct AFSStoreStatus {
	u_int32 Mask;
	u_int32 ClientModTime;
	u_int32 Owner;
	u_int32 Group;
	u_int32 UnixModeBits;
	u_int32 SegSize;
};
typedef struct AFSStoreStatus AFSStoreStatus;
bool_t xdr_AFSStoreStatus();

#define	AFS_SETMODTIME	1
#define	AFS_SETOWNER	2
#define	AFS_SETGROUP		4
#define	AFS_SETMODE		8
#define	AFS_SETSEGSIZE	16
#define	AFS_FSYNC       1024

typedef int32 ViceVolumeType;
bool_t xdr_ViceVolumeType();

#define ReadOnly 0
#define ReadWrite 1

struct ViceDisk {
	int32 BlocksAvailable;
	int32 TotalBlocks;
	DiskName Name;
};
typedef struct ViceDisk ViceDisk;
bool_t xdr_ViceDisk();


struct ViceStatistics {
	u_int32 CurrentMsgNumber;
	u_int32 OldestMsgNumber;
	u_int32 CurrentTime;
	u_int32 BootTime;
	u_int32 StartTime;
	int32 CurrentConnections;
	u_int32 TotalViceCalls;
	u_int32 TotalFetchs;
	u_int32 FetchDatas;
	u_int32 FetchedBytes;
	int32 FetchDataRate;
	u_int32 TotalStores;
	u_int32 StoreDatas;
	u_int32 StoredBytes;
	int32 StoreDataRate;
	u_int32 TotalRPCBytesSent;
	u_int32 TotalRPCBytesReceived;
	u_int32 TotalRPCPacketsSent;
	u_int32 TotalRPCPacketsReceived;
	u_int32 TotalRPCPacketsLost;
	u_int32 TotalRPCBogusPackets;
	int32 SystemCPU;
	int32 UserCPU;
	int32 NiceCPU;
	int32 IdleCPU;
	int32 TotalIO;
	int32 ActiveVM;
	int32 TotalVM;
	int32 EtherNetTotalErrors;
	int32 EtherNetTotalWrites;
	int32 EtherNetTotalInterupts;
	int32 EtherNetGoodReads;
	int32 EtherNetTotalBytesWritten;
	int32 EtherNetTotalBytesRead;
	int32 ProcessSize;
	int32 WorkStations;
	int32 ActiveWorkStations;
	int32 Spare1;
	int32 Spare2;
	int32 Spare3;
	int32 Spare4;
	int32 Spare5;
	int32 Spare6;
	int32 Spare7;
	int32 Spare8;
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
	int32 Vid;
	int32 ParentId;
	char Online;
	char InService;
	char Blessed;
	char NeedsSalvage;
	int32 Type;
	int32 MinQuota;
	int32 MaxQuota;
	int32 BlocksInUse;
	int32 PartBlocksAvail;
	int32 PartMaxBlocks;
};
typedef struct VolumeStatus VolumeStatus;
bool_t xdr_VolumeStatus();


struct AFSFetchVolumeStatus {
	int32 Vid;
	int32 ParentId;
	char Online;
	char InService;
	char Blessed;
	char NeedsSalvage;
	int32 Type;
	int32 MinQuota;
	int32 MaxQuota;
	int32 BlocksInUse;
	int32 PartBlocksAvail;
	int32 PartMaxBlocks;
};
typedef struct AFSFetchVolumeStatus AFSFetchVolumeStatus;
bool_t xdr_AFSFetchVolumeStatus();


struct AFSStoreVolumeStatus {
	int32 Mask;
	int32 MinQuota;
	int32 MaxQuota;
};
typedef struct AFSStoreVolumeStatus AFSStoreVolumeStatus;
bool_t xdr_AFSStoreVolumeStatus();

#define AFS_SETMINQUOTA 1
#define AFS_SETMAXQUOTA 2

struct AFSVolumeInfo {
	u_int32 Vid;
	int32 Type;
	u_int32 Type0;
	u_int32 Type1;
	u_int32 Type2;
	u_int32 Type3;
	u_int32 Type4;
	u_int32 ServerCount;
	u_int32 Server0;
	u_int32 Server1;
	u_int32 Server2;
	u_int32 Server3;
	u_int32 Server4;
	u_int32 Server5;
	u_int32 Server6;
	u_int32 Server7;
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
	u_int32 Vid;
	int32 Type;
	u_int32 Type0;
	u_int32 Type1;
	u_int32 Type2;
	u_int32 Type3;
	u_int32 Type4;
	u_int32 ServerCount;
	u_int32 Server0;
	u_int32 Server1;
	u_int32 Server2;
	u_int32 Server3;
	u_int32 Server4;
	u_int32 Server5;
	u_int32 Server6;
	u_int32 Server7;
};
typedef struct VolumeInfo VolumeInfo;
bool_t xdr_VolumeInfo();


typedef int32 ViceLockType;
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

typedef int32 ViceOfflineType;
bool_t xdr_ViceOfflineType();

#define NoSalvage 0
#define Salvage 1
#ifdef KERNEL
#include "../afs/longc_procs.h"
#endif
#define FLUSHMAX 10

typedef struct ViceIds {
	u_int ViceIds_len;
	int32 *ViceIds_val;
} ViceIds;
bool_t xdr_ViceIds();


typedef struct IPAddrs {
	u_int IPAddrs_len;
	int32 *IPAddrs_val;
} IPAddrs;
bool_t xdr_IPAddrs();


/* Opcode-related useful stats for package: RXAFS_ */
#define RXAFS_LOWEST_OPCODE   130
#define RXAFS_HIGHEST_OPCODE	163
#define RXAFS_NUMBER_OPCODES	34

#endif	/* _RXGEN_AFSINT_ */
