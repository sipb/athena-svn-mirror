/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_BUTC_
#define	_RXGEN_BUTC_

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

#define TC_MAXDUMPPATH 256
#define TC_MAXNAMELEN 64
#define TC_MAXARRAY 2000000000
#define TC_MAXFORMATLEN 100
#define TC_MAXHOSTLEN 32
#define TC_MAXTAPELEN 32

struct tc_dumpDesc {
	int32 vid;
	int32 vtype;
	char name[TC_MAXNAMELEN];
	int32 partition;
	int32 date;
	int32 cloneDate;
	u_int32 hostAddr;
};
typedef struct tc_dumpDesc tc_dumpDesc;
bool_t xdr_tc_dumpDesc();


struct tc_restoreDesc {
	int32 frag;
	char tapeName[TC_MAXTAPELEN];
	u_int32 dbDumpId;
	u_int32 initialDumpId;
	int32 position;
	int32 origVid;
	int32 vid;
	int32 partition;
	int32 dumpLevel;
	u_int32 hostAddr;
	char oldName[TC_MAXNAMELEN];
	char newName[TC_MAXNAMELEN];
};
typedef struct tc_restoreDesc tc_restoreDesc;
bool_t xdr_tc_restoreDesc();


struct tc_dumpStat {
	int32 dumpID;
	u_int32 KbytesDumped;
	u_int32 bytesDumped;
	int32 volumeBeingDumped;
	int32 numVolErrs;
	int32 flags;
};
typedef struct tc_dumpStat tc_dumpStat;
bool_t xdr_tc_dumpStat();


struct tc_tapeLabel {
	int32 size;
	char afsname[TC_MAXTAPELEN];
	char pname[TC_MAXTAPELEN];
	u_int32 tapeId;
};
typedef struct tc_tapeLabel tc_tapeLabel;
bool_t xdr_tc_tapeLabel();


struct tc_TMInfo {
	int32 capacity;
	int32 flags;
};
typedef struct tc_TMInfo tc_TMInfo;
bool_t xdr_tc_TMInfo();


struct tc_tapeSet {
	int32 id;
	char tapeServer[TC_MAXHOSTLEN];
	char format[TC_MAXFORMATLEN];
	int maxTapes;
	int32 a;
	int32 b;
	int32 expDate;
	int32 expType;
};
typedef struct tc_tapeSet tc_tapeSet;
bool_t xdr_tc_tapeSet();


struct tc_tcInfo {
	int32 tcVersion;
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
	int32 parentDumpId;
	int32 dumpLevel;
	int doAppend;
};
typedef struct tc_dumpInterface tc_dumpInterface;
bool_t xdr_tc_dumpInterface();


struct tciStatusS {
	char taskName[TC_MAXNAMELEN];
	u_int32 taskId;
	u_int32 flags;
	u_int32 dbDumpId;
	u_int32 nKBytes;
	char volumeName[TC_MAXNAMELEN];
	int32 volsFailed;
	int32 lastPolled;
};
typedef struct tciStatusS tciStatusS;
bool_t xdr_tciStatusS();


/* Opcode-related useful stats for package: TC_ */
#define TC_LOWEST_OPCODE   0
#define TC_HIGHEST_OPCODE	11
#define TC_NUMBER_OPCODES	12

#endif	/* _RXGEN_BUTC_ */
