/* Machine generated file -- Do NOT edit */

#ifndef	_RXGEN_BUDB_
#define	_RXGEN_BUDB_

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

#ifndef NEVERDATE
#define NEVERDATE 037777777777		/* a date that will never come */
#endif
#ifndef Date
#define Date u_int32
#endif
#define BUDB_MAJORVERSION 2
#ifndef dumpId
#define dumpId u_int32
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
	int32 id;
	char tapeServer[32];
	char format[32];
	int32 maxTapes;
	int32 a;
	int32 b;
};
typedef struct budb_tapeSet budb_tapeSet;
bool_t xdr_budb_tapeSet();

#define budb_MakeTapeName(name,set,seq) sprintf (name, (set)->format, (set)->a*(seq) + (set)->b)

struct budb_dumpEntry {
	u_int32 id;
	u_int32 parent;
	int level;
	int32 flags;
	char volumeSetName[32];
	char dumpPath[256];
	char name[32];
	u_int32 created;
	u_int32 incTime;
	int nVolumes;
	struct budb_tapeSet tapes;
	struct budb_principal dumper;
	u_int32 initialDumpID;
	u_int32 appendedDumpID;
};
typedef struct budb_dumpEntry budb_dumpEntry;
bool_t xdr_budb_dumpEntry();

#define BUDB_DUMP_INCOMPLETE (1<<0)	/* some vols omitted due to errors */
#define BUDB_DUMP_TAPEERROR  (1<<1)	/* tape error during dump */
#define BUDB_DUMP_INPROGRESS (1<<2)
#define BUDB_DUMP_ABORTED    (1<<3)	/* aborted: prob. dump unavailable */

struct budb_tapeEntry {
	char name[32];
	int32 flags;
	u_int32 written;
	u_int32 expires;
	u_int32 nMBytes;
	u_int32 nBytes;
	int32 nFiles;
	int nVolumes;
	int seq;
	int32 tapeid;
	int useCount;
	int32 useKBytes;
	u_int32 dump;
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
	int32 flags;
	int32 id;
	char server[32];
	int32 partition;
	int tapeSeq;
	int32 position;
	u_int32 clone;
	u_int32 incTime;
	int32 startByte;
	u_int32 nBytes;
	int seq;
	u_int32 dump;
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
	int32 *budb_dumpsList_val;
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

struct DbHeader {
	int32 dbversion;
	int32 created;
	char cell[256];
	u_int32 lastDumpId;
	u_int32 lastInstanceId;
	u_int32 lastTapeId;
};
typedef struct DbHeader DbHeader;
bool_t xdr_DbHeader();


struct structDumpHeader {
	int32 type;
	int32 structversion;
	int32 size;
};
typedef struct structDumpHeader structDumpHeader;
bool_t xdr_structDumpHeader();


/* Opcode-related useful stats for package: BUDB_ */
#define BUDB_LOWEST_OPCODE   0
#define BUDB_HIGHEST_OPCODE	28
#define BUDB_NUMBER_OPCODES	29

#endif	/* _RXGEN_BUDB_ */
