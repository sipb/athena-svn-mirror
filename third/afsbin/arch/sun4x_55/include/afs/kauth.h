/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_KAUTH_
#define	_RXGEN_KAUTH_

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


struct ka_CBS {
	int32 SeqLen;
	char *SeqBody;
};
typedef struct ka_CBS ka_CBS;
bool_t xdr_ka_CBS();


struct ka_BBS {
	int32 MaxSeqLen;
	int32 SeqLen;
	char *SeqBody;
};
typedef struct ka_BBS ka_BBS;
bool_t xdr_ka_BBS();

#define MAXKAKVNO 127
#define KAFNORMAL 0x001
#define KAFFREE 0x002
#define KAFOLDKEYS 0x010
#define KAFSPECIAL 0x100
#define KAFASSOCROOT 0x200
#define KAFASSOC 0x400
#define KAFADMIN 0x004
#define KAFNOTGS 0x008
#define KAFNOSEAL 0x020
#define KAFNOCPW 0x040
#define KAFNEWASSOC 0x080
#define KAF_SETTABLE_FLAGS (KAFADMIN | KAFNOTGS | KAFNOSEAL | KAFNOCPW | KAFNEWASSOC)

struct EncryptionKey {
	char key[8];
};
typedef struct EncryptionKey EncryptionKey;
bool_t xdr_EncryptionKey();

#define KAMAJORVERSION 5
#define KAMINORVERSION 2
#ifndef NEVERDATE
#define NEVERDATE 037777777777		/* a date that will never come */
#endif
#ifndef Date
#define Date u_int32
#endif
#if !defined(AFS_AIX22_ENV) && !defined(AFS_HPUX_ENV)
#define AUTH_LOG
#endif

typedef char *kaname;
bool_t xdr_kaname();


struct kaident {
	char name[64];
	char instance[64];
};
typedef struct kaident kaident;
bool_t xdr_kaident();


struct kaentryinfo {
	int32 minor_version;
	int32 flags;
	u_int32 user_expiration;
	u_int32 modification_time;
	struct kaident modification_user;
	u_int32 change_password_time;
	int32 max_ticket_lifetime;
	int32 key_version;
	EncryptionKey key;
	u_int32 keyCheckSum;
	u_int32 misc_auth_bytes;
	int32 reserved3;
	int32 reserved4;
};
typedef struct kaentryinfo kaentryinfo;
bool_t xdr_kaentryinfo();


struct kasstats {
	int32 minor_version;
	int32 allocs;
	int32 frees;
	int32 cpws;
	int32 reserved1;
	int32 reserved2;
	int32 reserved3;
	int32 reserved4;
};
typedef struct kasstats kasstats;
bool_t xdr_kasstats();


struct katimeval {
	int32 tv_sec;
	int32 tv_usec;
};
typedef struct katimeval katimeval;
bool_t xdr_katimeval();


struct karpcstats {
	int requests;
	int aborts;
};
typedef struct karpcstats karpcstats;
bool_t xdr_karpcstats();


struct kadstats {
	int32 minor_version;
	int32 host;
	u_int32 start_time;
	int32 hashTableUtilization;
	struct karpcstats Authenticate;
	struct karpcstats ChangePassword;
	struct karpcstats GetTicket;
	struct karpcstats CreateUser;
	struct karpcstats SetPassword;
	struct karpcstats SetFields;
	struct karpcstats DeleteUser;
	struct karpcstats GetEntry;
	struct karpcstats ListEntry;
	struct karpcstats GetStats;
	struct karpcstats GetPassword;
	struct karpcstats GetRandomKey;
	struct karpcstats Debug;
	struct karpcstats UAuthenticate;
	struct karpcstats UGetTicket;
	struct karpcstats Unlock;
	struct karpcstats LockStatus;
	int32 string_checks;
	int32 reserved1;
	int32 reserved2;
	int32 reserved3;
	int32 reserved4;
};
typedef struct kadstats kadstats;
bool_t xdr_kadstats();

#define KADEBUGKCINFOSIZE 25

struct ka_kcInfo {
	u_int32 used;
	int32 kvno;
	char primary;
	char keycksum;
	char principal[64];
};
typedef struct ka_kcInfo ka_kcInfo;
bool_t xdr_ka_kcInfo();


struct ka_debugInfo {
	int32 minorVersion;
	int32 host;
	u_int32 startTime;
	int noAuth;
	u_int32 lastTrans;
	char lastOperation[16];
	char lastAuth[256];
	char lastUAuth[256];
	char lastTGS[256];
	char lastUTGS[256];
	char lastAdmin[256];
	char lastTGSServer[256];
	char lastUTGSServer[256];
	u_int32 nextAutoCPW;
	int updatesRemaining;
	u_int32 dbHeaderRead;
	int32 dbVersion;
	int32 dbFreePtr;
	int32 dbEofPtr;
	int32 dbKvnoPtr;
	int32 dbSpecialKeysVersion;
	int32 cheader_lock;
	int32 keycache_lock;
	int32 kcVersion;
	int kcSize;
	int kcUsed;
	struct ka_kcInfo kcInfo[KADEBUGKCINFOSIZE];
	int32 reserved1;
	int32 reserved2;
	int32 reserved3;
	int32 reserved4;
};
typedef struct ka_debugInfo ka_debugInfo;
bool_t xdr_ka_debugInfo();


/* Opcode-related useful stats for package: KAA_ */
#define KAA_LOWEST_OPCODE   1
#define KAA_HIGHEST_OPCODE	22
#define KAA_NUMBER_OPCODES	4


/* Opcode-related useful stats for package: KAT_ */
#define KAT_LOWEST_OPCODE   3
#define KAT_HIGHEST_OPCODE	23
#define KAT_NUMBER_OPCODES	2


/* Opcode-related useful stats for package: KAM_ */
#define KAM_LOWEST_OPCODE   4
#define KAM_HIGHEST_OPCODE	15
#define KAM_NUMBER_OPCODES	12

#endif	/* _RXGEN_KAUTH_ */
