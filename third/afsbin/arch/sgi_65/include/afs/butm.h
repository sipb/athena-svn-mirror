/*
 * butm.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including butm.p.h at beginning of butm.h file. */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1989
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*===============================================================
 * Copyright (C) 1989 Transarc Corporation - All rights reserved 
 *===============================================================*/


/*==============================================================
 * 	Sailesh Chutani
 *	Transarc Corporation
 *	September 15, 1989
 *==============================================================
 */

#include <afs/auth.h>
#include <afs/param.h>
#include <afs/bubasics.h>

/* mt does not work in AIX, hence the stuff below */
#ifdef AFS_AIX_ENV
#include <sys/tape.h>

struct stop st_com;
#ifdef AFS_AIX32_ENV
/*
 * AIX31 device driver neglected to provide this functionality.
 * sigh.
 */
struct always_wonderful {
	int32	utterly_fantastic;
} st_status;
#else
struct stget st_status;
#endif

#define GENCOM st_com
#define GENGET st_status
#define GENOP st_op
#define GENCOUNT st_count
#define GENWEOF STWEOF
#define GENREW STREW
#define GENFSF STFSF
#define GENRETEN STRETEN
#define GENRESET STRESET
#define GENBSF STREW /* should be a no-op */ 
#define GENCALL(fid,com) ioctl(fid,STIOCTOP,com)

#else
#include <sys/mtio.h>

struct mtop mt_com;
struct mtget mt_status;

#define GENCOM mt_com
#define GENGET mt_status
#define GENOP  mt_op
#define GENCOUNT mt_count
#define GENWEOF MTWEOF
#define GENREW MTREW
#define GENFSF MTFSF
#define GENRETEN MTRETEN
#define GENBSF MTBSF
#define GENOFFL MTOFFL
#define GENCALL(fid,com) ioctl(fid,MTIOCTOP,com)

#endif

struct blockMark {
    int	count;	    /* actual number of bytes valid in the block */
    int32 magic;	    
    int32 spare1;
    int32 spare2;
    int32 spare3;
    int32 spare4;
};

/* the 16k limit is related to tc_EndMargin in dumps.c*/
#define BUTM_BLOCKSIZE 16384
#define	BUTM_BLKSIZE   (BUTM_BLOCKSIZE - ((5*sizeof(int32)) + sizeof(int)))   
                                                                  /* size of data portion of 
								     block written on tape:
  								     16k - sizeof(blockMark) */

struct butm_tapeInfo {
    int32  structVersion;
    struct {
	int32 (*mount)();
	int32 (*dismount)();
	int32 (*create)();
	int32 (*readLabel)();
	int32 (*seek)();
	int32 (*seekEODump)();
	int32 (*readFileBegin)();
	int32 (*readFileData)();
	int32 (*readFileEnd)();
	int32 (*writeFileBegin)();
	int32 (*writeFileData)();
	int32 (*writeFileEnd)();
	int32 (*writeEOT)();
	int32 (*setSize)();
	int32 (*getSize)();
    } ops;
    char  name[BU_MAXTAPELEN];
    int32 position;			/* current position of tape */
    u_int32 posCount;		        /* position in bytes of the tape */

    /* the next three fields are used for modeling tape usage */
    u_int32  nBytes;		        /* number of bytes   written */
    u_int32  kBytes;		        /* number of Kbytes  written */
    int32  nRecords;			/* number of records written */
    int32  nFiles;			/* number of files   written */

    /* These fields provide the coefficients for the above variables */
    int32  recordSize;			/* bytes per record */
    u_int32  tapeSize;		        /* length of tape */
    int32  coefBytes;			/* length multiplier for bytes */
    int32  coefRecords;			/*   ditto  records */
    int32  coefFiles;			/*   ditto  files */
    int   simultaneousTapes;		/* number of tapes mounted simultaneously */
    int   status;			/* status of tape */
    int   flags;			/* e.g. read-only, sequential */
    char  *tcRock;			/* for random tape coordinator storage */
    char  *tmRock;			/* for random tape module storage */
    int32  sizeflags;			/* What the units of tape size are. */

    int32  debug;                       /* print debugging info */
    int32  error;                       /* Error code from butm module */
    int32  spare[8];
};

struct tapeConfig{
    char device[256];
    u_int32 capacity;
    int32 fileMarkSize;			/* size of file marks, in bytes */
    int32 portOffset;
    int aixScsi;			/* are we using aix extended scsi drive */
};

/* returns answer in bytes */
#define butm_remainingSpace(i) (1024*((i)->tapeSize - (1024*(i)->kBytes*(i)->coefBytes + (i)->Bytes*(i)->coefBytes )))

/* returns answer in kbytes */
#define butm_remainingKSpace(i) ((i)->tapeSize - ((i)->kBytes*(i)->coefBytes ))

#define BUTM_STATUS_OFFLINE (1<<0)	/* tape not mounted */
#define BUTM_STATUS_TAPEERROR (1<<1)	/* tape error encountered */
#define BUTM_STATUS_WRITEERROR (1<<2)	/* tape error during write */
#define BUTM_STATUS_READERROR (1<<3)	/* tape error during read */
#define BUTM_STATUS_SEEKERROR (1<<4)	/* tape error during positioning */
#define BUTM_STATUS_EOF (1<<5)		/* tape at EOF */
#define BUTM_STATUS_EOD (1<<6)		/* end of tape reached */

#define BUTM_FLAGS_READONLY (1<<0)	/* tape mounted read-only */
#define BUTM_FLAGS_SEQUENTIAL (1<<1)	/* tape is sequential access: sort positions */

struct butm_tapeLabel {
    int32 structVersion;		/* structure version number */
    Date creationTime;			/* when tape was first labeled */
    Date expirationDate;		/* when tape can be relabelled */
    char AFSName[BU_MAXTAPELEN];	/* AFS assigned tape name */
    struct ktc_principal creator;	/* person creating tape */
    char cell[BU_MAXCELLLEN];		/* cell which owns tape. */
    u_int32 dumpid;			/* which dump on this tape  */
    int32 useCount;			/* # times written */
    int32 spare[8];			
    char comment[96];
    char pName[BU_MAXTAPELEN];          /* permanent user assigned tape name */
    u_int32 size;
    char dumpPath[BU_MAX_DUMP_PATH];	/* dump schedule path name */
};

#define TNAME(labelptr) \
   ( strcmp((labelptr)->pName,"") ? (labelptr)->pName : \
     ( strcmp((labelptr)->AFSName,"") ? (labelptr)->AFSName : "<NULL>" ) )

#define TAPENAME(tapename, name, dbDumpId) \
   if (!strcmp("", name)) \
     sprintf(tapename, "<NULL>"); \
   else if (dbDumpId == 0) \
     sprintf(tapename, "%s", name); \
   else \
     sprintf(tapename, "%s (%u)", name, dbDumpId);

#define LABELNAME(tapename, labelptr) \
   TAPENAME(tapename, TNAME(labelptr), (labelptr)->dumpid)

/* now the procedure macros */
#define butm_Mount(i,t) (*((i)->ops.mount))(i,t)
#define butm_Dismount(i) (*((i)->ops.dismount))(i)
#define butm_Create(i,l,r) (*((i)->ops.create))(i,l,r)
#define butm_ReadLabel(i,l,r) (*((i)->ops.readLabel))(i,l,r)
#define butm_Seek(i,p) (*((i)->ops.seek))(i,p)
#define butm_SeekEODump(i,p) (*((i)->ops.seekEODump))(i,p)
#define butm_ReadFileBegin(i) (*((i)->ops.readFileBegin))(i)
#define butm_ReadFileData(i,d,l,n) (*((i)->ops.readFileData))(i,d,l,n)
#define butm_ReadFileEnd(i) (*((i)->ops.readFileEnd))(i)
#define butm_WriteFileBegin(i) (*((i)->ops.writeFileBegin))(i)
#define butm_WriteFileData(i,d,b,l) (*((i)->ops.writeFileData))(i,d,b,l)
#define butm_WriteFileEnd(i) (*((i)->ops.writeFileEnd))(i)
#define butm_WriteEOT(i) (*((i)->ops.writeEOT))(i)
#define butm_SetSize(i,s) (*((i)->ops.setSize))(i,s)
#define butm_GetSize(i,s) (*((i)->ops.getSize))(i,s)



/* End of prolog file butm.p.h. */

#define BUTM_OLDINTERFACE                        (156568832L)
#define BUTM_NOMOUNT                             (156568833L)
#define BUTM_PARALLELMOUNTS                      (156568834L)
#define BUTM_MOUNTFAIL                           (156568835L)
#define BUTM_DISMOUNTFAIL                        (156568836L)
#define BUTM_IO                                  (156568837L)
#define BUTM_READONLY                            (156568838L)
#define BUTM_BADOP                               (156568839L)
#define BUTM_SHORTREAD                           (156568840L)
#define BUTM_SHORTWRITE                          (156568841L)
#define BUTM_EOT                                 (156568842L)
#define BUTM_BADCONFIG                           (156568843L)
#define BUTM_BADARGUMENT                         (156568844L)
#define BUTM_ENDVOLUME                           (156568845L)
#define BUTM_LABEL                               (156568846L)
#define BUTM_EOD                                 (156568847L)
#define BUTM_IOCTL                               (156568848L)
#define BUTM_EOF                                 (156568849L)
#define BUTM_BADBLOCK                            (156568850L)
#define BUTM_NOLABEL                             (156568851L)
#define BUTM_POSITION                            (156568852L)
extern void initialize_butm_error_table ();
#define ERROR_TABLE_BASE_butm (156568832L)

/* for compatibility with older versions... */
#define init_butm_err_tbl initialize_butm_error_table
#define butm_err_base ERROR_TABLE_BASE_butm
