/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_UBIK_INT_
#define	_RXGEN_UBIK_INT_

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


struct BDesc {
	int32 host;
	short portal;
	int32 session;
};
typedef struct BDesc BDesc;
bool_t xdr_BDesc();


struct net_version {
	int32 epoch;
	int32 counter;
};
typedef struct net_version net_version;
bool_t xdr_net_version();


struct net_tid {
	int32 epoch;
	int32 counter;
};
typedef struct net_tid net_tid;
bool_t xdr_net_tid();


struct ubik_debug {
	int32 now;
	int32 lastYesTime;
	int32 lastYesHost;
	int32 lastYesState;
	int32 lastYesClaim;
	int32 lowestHost;
	int32 lowestTime;
	int32 syncHost;
	int32 syncTime;
	struct net_version syncVersion;
	struct net_tid syncTid;
	int32 amSyncSite;
	int32 syncSiteUntil;
	int32 nServers;
	int32 lockedPages;
	int32 writeLockedPages;
	struct net_version localVersion;
	int32 activeWrite;
	int32 tidCounter;
	int32 anyReadLocks;
	int32 anyWriteLocks;
	int32 recoveryState;
	int32 currentTrans;
	int32 writeTrans;
	int32 epochTime;
};
typedef struct ubik_debug ubik_debug;
bool_t xdr_ubik_debug();


struct ubik_sdebug {
	int32 addr;
	int32 lastVoteTime;
	int32 lastBeaconSent;
	int32 lastVote;
	struct net_version remoteVersion;
	int32 currentDB;
	int32 beaconSinceDown;
	int32 up;
};
typedef struct ubik_sdebug ubik_sdebug;
bool_t xdr_ubik_sdebug();

#define BULK_ERROR 1

typedef struct bulkdata {
	u_int bulkdata_len;
	char *bulkdata_val;
} bulkdata;
bool_t xdr_bulkdata();


#include <rx/rx_multi.h>
#define multi_VOTE_Beacon(state, voteStart, Version, tid) \
	multi_Body(StartVOTE_Beacon(multi_call, state, voteStart, Version, tid), EndVOTE_Beacon(multi_call))


/* Opcode-related useful stats for package: VOTE_ */
#define VOTE_LOWEST_OPCODE   10000
#define VOTE_HIGHEST_OPCODE	10003
#define VOTE_NUMBER_OPCODES	4


/* Opcode-related useful stats for package: DISK_ */
#define DISK_LOWEST_OPCODE   20000
#define DISK_HIGHEST_OPCODE	20010
#define DISK_NUMBER_OPCODES	11

#endif	/* _RXGEN_UBIK_INT_ */
