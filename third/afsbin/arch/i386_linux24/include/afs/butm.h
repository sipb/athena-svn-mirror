/*
 * butm.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including butm.p.h at beginning of butm.h file. */

/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 * 
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

#include <afs/auth.h>
#include <afs/param.h>
#include <afs/bubasics.h>

struct blockMark {
    int	count;	    /* actual number of bytes valid in the block */
    afs_int32 magic;	    
    afs_int32 spare1;
    afs_int32 spare2;
    afs_int32 spare3;
    afs_int32 spare4;
};

/* The size of a tapeblock is 16KB: contains header info and data */
#define BUTM_BLOCKSIZE 16384
#define BUTM_HDRSIZE   ((5*sizeof(afs_int32)) + sizeof(int))      /* sizeof blockMark */
#define	BUTM_BLKSIZE   (BUTM_BLOCKSIZE - BUTM_HDRSIZE)

struct butm_tapeInfo {
    afs_int32  structVersion;
    struct {
	afs_int32 (*mount)();
	afs_int32 (*dismount)();
	afs_int32 (*create)();
	afs_int32 (*readLabel)();
	afs_int32 (*seek)();
	afs_int32 (*seekEODump)();
	afs_int32 (*readFileBegin)();
	afs_int32 (*readFileData)();
	afs_int32 (*readFileEnd)();
	afs_int32 (*writeFileBegin)();
	afs_int32 (*writeFileData)();
	afs_int32 (*writeFileEnd)();
	afs_int32 (*writeEOT)();
	afs_int32 (*setSize)();
	afs_int32 (*getSize)();
    } ops;
    char  name[BU_MAXTAPELEN];
    afs_int32 position;			/* current position of tape */
    afs_uint32 posCount;		        /* position in bytes of the tape */

    /* the next three fields are used for modeling tape usage */
    afs_uint32  nBytes;		        /* number of bytes   written */
    afs_uint32  kBytes;		        /* number of Kbytes  written */
    afs_int32  nRecords;			/* number of records written */
    afs_int32  nFiles;			/* number of files   written */

    /* These fields provide the coefficients for the above variables */
    afs_int32  recordSize;			/* bytes per record */
    afs_uint32  tapeSize;		        /* length of tape */
    afs_int32  coefBytes;			/* length multiplier for bytes */
    afs_int32  coefRecords;			/*   ditto  records */
    afs_int32  coefFiles;			/*   ditto  files */
    int   simultaneousTapes;		/* number of tapes mounted simultaneously */
    int   status;			/* status of tape */
    int   flags;			/* e.g. read-only, sequential */
    char  *tcRock;			/* for random tape coordinator storage */
    char  *tmRock;			/* for random tape module storage */
    afs_int32  sizeflags;			/* What the units of tape size are. */

    afs_int32  debug;                       /* print debugging info */
    afs_int32  error;                       /* Error code from butm module */
    afs_int32  spare[8];
};

struct tapeConfig{
    char device[256];
    afs_uint32 capacity;
    afs_int32 fileMarkSize;			/* size of file marks, in bytes */
    afs_int32 portOffset;
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
    afs_int32 structVersion;		/* structure version number */
    Date creationTime;			/* when tape was first labeled */
    Date expirationDate;		/* when tape can be relabelled */
    char AFSName[BU_MAXTAPELEN];	/* AFS assigned tape name */
    struct ktc_principal creator;	/* person creating tape */
    char cell[BU_MAXCELLLEN];		/* cell which owns tape. */
    afs_uint32 dumpid;			/* which dump on this tape  */
    afs_int32 useCount;			/* # times written */
    afs_int32 spare[8];			
    char comment[96];
    char pName[BU_MAXTAPELEN];          /* permanent user assigned tape name */
    afs_uint32 size;
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