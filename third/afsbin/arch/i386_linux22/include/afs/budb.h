/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_BUDB_
#define	_RXGEN_BUDB_

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

#ifndef NEVERDATE
#define NEVERDATE 037777777777		/* a date that will never come */
#endif
#ifndef Date
#define Date afs_uint32
#endif
#define BUDB_MAJORVERSION 2
#ifndef dumpId
#define dumpId afs_uint32 
#endif
#define	TB_DUMPSCHEDULE	0
#define	TB_VOLUMESET	1
#define	TB_TAPEHOSTS	2
#define	TB_NUM		3	/* no. of block types */
#define	TB_MAX		6	/* unused items are spares */

struct budb_principal {
	char name[256];
	char instance[128];
	char cell[256];
};
typedef struct budb_principal budb_principal;
bool_t xdr_budb_principal();


struct budb_tapeSet {
	afs_int32 id;
	char tapeServer[32];
	char format[32];
	afs_int32 maxTapes;
	afs_int32 a;
	afs_int32 b;
};
typedef struct budb_tapeSet budb_tapeSet;
bool_t xdr_budb_tapeSet();

#define budb_MakeTapeName(name,set,seq) sprintf (name, (set)->format, (set)->a*(seq) + (set)->b)

struct budb_dumpEntry {
	afs_uint32 id;
	afs_uint32 parent;
	afs_int32 level;
	afs_int32 flags;
	char volumeSetName[32];
	char dumpPath[256];
	char name[32];
	afs_uint32 created;
	afs_uint32 incTime;
	afs_int32 nVolumes;
	struct budb_tapeSet tapes;
	struct budb_principal dumper;
	afs_uint32 initialDumpID;
	afs_uint32 appendedDumpID;
};
typedef struct budb_dumpEntry budb_dumpEntry;
bool_t xdr_budb_dumpEntry();

#define BUDB_DUMP_INCOMPLETE (1<<0)	/* some vols omitted due to errors */
#define BUDB_DUMP_TAPEERROR  (1<<1)	/* tape error during dump */
#define BUDB_DUMP_INPROGRESS (1<<2)
#define BUDB_DUMP_ABORTED    (1<<3)	/* aborted: prob. dump unavailable */
#define BUDB_DUMP_XBSA_NSS   (1<<8)    /* dump was done with a client    */
#define BUDB_DUMP_BUTA       (1<<11)	/* (used by ADSM buta) == 0x800 */
#define BUDB_DUMP_ADSM	      (1<<12)	/* (used by XBSA/ADSM buta) == 0x1000 */

struct budb_tapeEntry {
	char name[32];
	afs_int32 flags;
	afs_uint32 written;
	afs_uint32 expires;
	afs_uint32 nMBytes;
	afs_uint32 nBytes;
	afs_int32 nFiles;
	afs_int32 nVolumes;
	afs_int32 seq;
	afs_int32 labelpos;
	afs_int32 useCount;
	afs_int32 useKBytes;
	afs_uint32 dump;
};
typedef struct budb_tapeEntry budb_tapeEntry;
bool_t xdr_budb_tapeEntry();

#define BUDB_TAPE_TAPEERROR    (1<<0)
#define BUDB_TAPE_DELETED      (1<<1)
#define BUDB_TAPE_BEINGWRITTEN (1<<2)	/* writing in progress */
#define BUDB_TAPE_ABORTED      (1<<3)	/* aborted: tape probably garbaged */
#define BUDB_TAPE_STAGED       (1<<4)	/* not yet on permanent media */
#define BUDB_TAPE_WRITTEN      (1<<5)	/* tape writing finished: all OK */

struct budb_volumeEntry {
	char name[32];
	afs_int32 flags;
	afs_int32 id;
	char server[32];
	afs_int32 partition;
	afs_int32 tapeSeq;
	afs_int32 position;
	afs_uint32 clone;
	afs_uint32 incTime;
	afs_int32 startByte;
	afs_uint32 nBytes;
	afs_int32 seq;
	afs_uint32 dump;
	char tape[32];
};
typedef struct budb_volumeEntry budb_volumeEntry;
bool_t xdr_budb_volumeEntry();

#define BUDB_VOL_TAPEERROR    (1<<0)	/* tape problem during dump */
#define BUDB_VOL_FILEERROR    (1<<1)	/* voldump aborted during dump */
#define BUDB_VOL_BEINGWRITTEN (1<<2)
#define BUDB_VOL_FIRSTFRAG    (1<<3)	/* same as low bits of tape position */
#define BUDB_VOL_LASTFRAG     (1<<4)
#define BUDB_VOL_ABORTED      (1<<5)	/* aborted: vol probably undumped */
#define BUDB_STATINDEX 17
#define BUDB_OP_NAMES	    (0x7)
#define BUDB_OP_STARTS	    (0x7<<3)
#define BUDB_OP_ENDS	    (0x7<<6)
#define BUDB_OP_TIMES	    (0x3<<9)
#define BUDB_OP_MISC	    (0xff<<16)
#define BUDB_OP_DUMPNAME   (1<<0)
#define BUDB_OP_VOLUMENAME (2<<0)
#define BUDB_OP_TAPENAME   (3<<0)
#define BUDB_OP_TAPESEQ    (4<<0)
#define BUDB_OP_STARTTIME  (1<<3)
#define BUDB_OP_RANGE      (1<<6)
#define BUDB_OP_NPREVIOUS  (2<<6)
#define BUDB_OP_NFOLLOWING (3<<6)
#define BUDB_OP_DUMPID     (2<<3)
#define BUDB_OP_CLONETIME  (1<<9)	/* use clone time */
#define BUDB_OP_DUMPTIME   (2<<9)	/* use dump time (create?) */
#define BUDB_OP_INCTIME    (3<<9)	/* use inc time */
#define BUDB_OP_FIRSTFRAG  (1<<16)
#define BUDB_MAX_RETURN_LIST 1000

typedef struct budb_volumeList {
	u_int budb_volumeList_len;
	budb_volumeEntry *budb_volumeList_val;
} budb_volumeList;
bool_t xdr_budb_volumeList();


typedef struct budb_dumpList {
	u_int budb_dumpList_len;
	budb_dumpEntry *budb_dumpList_val;
} budb_dumpList;
bool_t xdr_budb_dumpList();


typedef struct budb_tapeList {
	u_int budb_tapeList_len;
	budb_tapeEntry *budb_tapeList_val;
} budb_tapeList;
bool_t xdr_budb_tapeList();


typedef struct budb_dumpsList {
	u_int budb_dumpsList_len;
	afs_int32 *budb_dumpsList_val;
} budb_dumpsList;
bool_t xdr_budb_dumpsList();


typedef struct charListT {
	u_int charListT_len;
	char *charListT_val;
} charListT;
bool_t xdr_charListT();

#define BUDB_TEXT_COMPLETE	1
#define	SD_DBHEADER		1
#define	SD_DUMP			2
#define	SD_TAPE			3
#define	SD_VOLUME		4
#define	SD_TEXT_DUMPSCHEDULE	5
#define	SD_TEXT_VOLUMESET	6
#define	SD_TEXT_TAPEHOSTS	7
#define	SD_END			8
#define	BUDB_OP_DATES		(0x01)
#define	BUDB_OP_GROUPID		(0x02)
#define	BUDB_OP_APPDUMP		(0x01)
#define	BUDB_OP_DBDUMP		(0x02)

struct DbHeader {
	afs_int32 dbversion;
	afs_int32 created;
	char cell[256];
	afs_uint32 lastDumpId;
	afs_uint32 lastInstanceId;
	afs_uint32 lastTapeId;
};
typedef struct DbHeader DbHeader;
bool_t xdr_DbHeader();


struct structDumpHeader {
	afs_int32 type;
	afs_int32 structversion;
	afs_int32 size;
};
typedef struct structDumpHeader structDumpHeader;
bool_t xdr_structDumpHeader();


/* Opcode-related useful stats for package: BUDB_ */
#define BUDB_LOWEST_OPCODE   0
#define BUDB_HIGHEST_OPCODE	30
#define BUDB_NUMBER_OPCODES	31

#endif	/* _RXGEN_BUDB_ */
