/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_BUTC_
#define	_RXGEN_BUTC_

#ifdef	KERNEL
/* The following 'ifndefs' are not a good solution to the vendor's omission of surrounding all system includes with 'ifndef's since it requires that this file is included after the system includes...*/
#include "../afs/param.h"
#ifdef	UKERNEL
#include "../afs/sysincludes.h"
#include "../rx/xdr.h"
#include "../rx/rx.h"
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
#else	/* KERNEL */
#include <afs/param.h>
#include <afs/stds.h>
#include <sys/types.h>
#include <rx/xdr.h>
#include <rx/rx.h>
#include <afs/rxgen_consts.h>
#endif	/* KERNEL */

#ifdef AFS_NT40_ENV
#ifndef AFS_RXGEN_EXPORT
#define AFS_RXGEN_EXPORT __declspec(dllimport)
#endif /* AFS_RXGEN_EXPORT */
#else /* AFS_NT40_ENV */
#define AFS_RXGEN_EXPORT
#endif /* AFS_NT40_ENV */

#define TC_STATINDEX 4
#define TC_MAXDUMPPATH 256
#define TC_MAXNAMELEN 64
#define TC_MAXARRAY 2000000000
#define TC_MAXFORMATLEN 100
#define TC_MAXHOSTLEN 32
#define TC_MAXTAPELEN 32

struct tc_dumpDesc {
	afs_int32 vid;
	afs_int32 vtype;
	char name[TC_MAXNAMELEN];
	afs_int32 partition;
	afs_int32 date;
	afs_int32 cloneDate;
	afs_uint32 hostAddr;
};
typedef struct tc_dumpDesc tc_dumpDesc;
bool_t xdr_tc_dumpDesc();

#define RDFLAG_LASTDUMP 0x1
#define RDFLAG_SKIP 0x2
#define RDFLAG_FIRSTDUMP 0x4

struct tc_restoreDesc {
	afs_int32 flags;
	char tapeName[TC_MAXTAPELEN];
	afs_uint32 dbDumpId;
	afs_uint32 initialDumpId;
	afs_int32 position;
	afs_int32 origVid;
	afs_int32 vid;
	afs_int32 partition;
	afs_int32 dumpLevel;
	afs_uint32 hostAddr;
	char oldName[TC_MAXNAMELEN];
	char newName[TC_MAXNAMELEN];
};
typedef struct tc_restoreDesc tc_restoreDesc;
bool_t xdr_tc_restoreDesc();


struct tc_dumpStat {
	afs_int32 dumpID;
	afs_uint32 KbytesDumped;
	afs_uint32 bytesDumped;
	afs_int32 volumeBeingDumped;
	afs_int32 numVolErrs;
	afs_int32 flags;
};
typedef struct tc_dumpStat tc_dumpStat;
bool_t xdr_tc_dumpStat();


struct tc_tapeLabel {
	afs_int32 size;
	char afsname[TC_MAXTAPELEN];
	char pname[TC_MAXTAPELEN];
	afs_uint32 tapeId;
};
typedef struct tc_tapeLabel tc_tapeLabel;
bool_t xdr_tc_tapeLabel();


struct tc_TMInfo {
	afs_int32 capacity;
	afs_int32 flags;
};
typedef struct tc_TMInfo tc_TMInfo;
bool_t xdr_tc_TMInfo();


struct tc_tapeSet {
	afs_int32 id;
	char tapeServer[TC_MAXHOSTLEN];
	char format[TC_MAXFORMATLEN];
	int maxTapes;
	afs_int32 a;
	afs_int32 b;
	afs_int32 expDate;
	afs_int32 expType;
};
typedef struct tc_tapeSet tc_tapeSet;
bool_t xdr_tc_tapeSet();


struct tc_tcInfo {
	afs_int32 tcVersion;
};
typedef struct tc_tcInfo tc_tcInfo;
bool_t xdr_tc_tcInfo();

#define TC_STAT_DONE 1
#define TC_STAT_OPRWAIT 2
#define TC_STAT_DUMP 4
#define TC_STAT_ABORTED 8
#define TC_STAT_ERROR 16
#define TSK_STAT_FIRST		0x1	/* get id of first task */
#define TSK_STAT_END		0x2	/* no more tasks */
#define TSK_STAT_NOTFOUND	0x4	/* couldn't find task id requested */
#define TSK_STAT_XBSA	       0x10     /* An XBSA server */
#define TSK_STAT_ADSM	       0x20	/* An ADSM XBSA server */

typedef struct tc_dumpArray {
	u_int tc_dumpArray_len;
	tc_dumpDesc *tc_dumpArray_val;
} tc_dumpArray;
bool_t xdr_tc_dumpArray();


typedef struct tc_restoreArray {
	u_int tc_restoreArray_len;
	tc_restoreDesc *tc_restoreArray_val;
} tc_restoreArray;
bool_t xdr_tc_restoreArray();


struct tc_dumpInterface {
	char dumpPath[TC_MAXDUMPPATH];
	char volumeSetName[TC_MAXNAMELEN];
	char dumpName[TC_MAXNAMELEN];
	struct tc_tapeSet tapeSet;
	afs_int32 parentDumpId;
	afs_int32 dumpLevel;
	int doAppend;
};
typedef struct tc_dumpInterface tc_dumpInterface;
bool_t xdr_tc_dumpInterface();


struct tciStatusS {
	char taskName[TC_MAXNAMELEN];
	afs_uint32 taskId;
	afs_uint32 flags;
	afs_uint32 dbDumpId;
	afs_uint32 nKBytes;
	char volumeName[TC_MAXNAMELEN];
	afs_int32 volsFailed;
	afs_int32 lastPolled;
};
typedef struct tciStatusS tciStatusS;
bool_t xdr_tciStatusS();


/* Opcode-related useful stats for package: TC_ */
#define TC_LOWEST_OPCODE   0
#define TC_HIGHEST_OPCODE	12
#define TC_NUMBER_OPCODES	13

#endif	/* _RXGEN_BUTC_ */
