/*
 * volser.h:
 * This file is automatically generated; please do not edit it.
 */
/* Including volser.p.h at beginning of volser.h file. */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
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






#ifndef _VOLSER_
#define _VOLSER_ 1
/* vflags, representing state of the volume */
#define	VTDeleteOnSalvage	1	/* delete on next salvage */
#define	VTOutOfService		2	/* never put this volume online */
#define	VTDeleted		4	/* deleted, don't do anything else */

/* iflags, representing "attach mode" for this volume at the start of this transaction */
#define	ITOffline	1	/* volume offline on server (returns VOFFLINE) */
#define	ITBusy		2	/* volume busy on server (returns VBUSY) */
#define	ITReadOnly	8	/* volume readonly on client, readwrite on server -DO NOT USE*/
#define	ITCreate	0x10	/* volume does not exist correctly yet */
#define	ITCreateVolID	0x1000	/* create volid */

/* tflags, representing transaction state */
#define	TTDeleted	1	/* delete transaction not yet freed due  to high refCount */

/* other names for volumes in voldefs.h */
#define volser_RW	0
#define volser_RO	1
#define	volser_BACK	2

#define	THOLD(tt)	((tt)->refCount++)

struct volser_trans {
    struct volser_trans	*next;	/* next ptr in active trans list */
    int32 tid;		    /* transaction id */
    int32 time;		    /* time transaction was last active (for timeouts) */
    int32 creationTime;	    /* time the transaction started */
    int32 returnCode;	    /* transaction error code */
    struct Volume *volume;  /* pointer to open volume */
    int32 volid;		    /* open volume's id */
    int32 partition;	    /* open volume's partition */
    int32 dumpTransId;	    /* other side's trans id during a dump */
    int32 dumpSeq;	    /* next sequence number to use during a dump */
    short refCount;	    /* reference count on this structure */
    short iflags;	    /* initial attach mode flags (IT*) */
    char vflags;	    /* current volume status flags (VT*) */
    char tflags;	    /* transaction flags (TT*) */
    char incremental;	    /* do an incremental restore */
    /* the fields below are useful for debugging */
    char lastProcName[30];  /* name of the last procedure which used transaction */
    struct rx_call *rxCallPtr; /* pointer to latest associated rx_call */
    
};

/* This is how often the garbage collection thread wakes up and 
 * checks for transactions that have timed out: BKGLoop()
 */
#define GCWAKEUP            30

struct volser_dest {
    int32 destHost;
    int32 destPort;
    int32 destSSID;
};

#define	MAXHELPERS	    10
/* flags for vol helper busyFlags array.  First, VHIdle goes on when a server
 * becomes idle.  Next, idle flag is cleared and VHRequest goes on when
 * trans is queued.  Finally, VHRequest goes off (but VHIdle stays off) when
 * helper is done.  VHIdle goes on again when an lwp waits for work.
 */
#define	VHIdle		    1	    /* vol helper is waiting for a request here */
#define	VHRequest	    2	    /* a request has been queued here */
extern struct volser_trans *QI_GlobalWriteTrans;

/* the stuff below is from errors.h in vol directory */
#define VICE_SPECIAL_ERRORS	101	/* Lowest special error code */

#define VSALVAGE	101	/* Volume needs salvage */
#define VNOVNODE	102	/* Bad vnode number quoted */
#define VNOVOL		103	/* Volume not attached, doesn't exist, 
				   not created or not online */
#define VVOLEXISTS	104	/* Volume already exists */
#define VNOSERVICE	105	/* Volume is not in service (i.e. it's
				   is out of funds, is obsolete, or somesuch) */
#define VOFFLINE	106	/* Volume is off line, for the reason
				   given in the offline message */
#define VONLINE		107	/* Volume is already on line */
#define VDISKFULL	108	/* Partition is "full", i.e. rougly within
				   n% of full */
#define VOVERQUOTA	109	/* Volume max quota exceeded */
#define VBUSY		110	/* Volume temporarily unavailable; try again.
				   The volume should be available again shortly; if
				   it isn't something is wrong.  Not normally to be
				   propagated to the application level */
#define VMOVED		111	/* Volume has moved to another server; do a VGetVolumeInfo
				   to THIS server to find out where */

#define MyPort 5003
#define NameLen 80
#define VLDB_MAXSERVERS 10
#define VOLSERVICE_ID 4
#define INVALID_BID 0
#define VOLSER_MAXVOLNAME 65
#define VOLSER_OLDMAXVOLNAME 32
#define	VOLMAXPARTS	255

int32 VldbServer; /* temporary hack */
char T_servername[80]; /*temporary hack */

/*flags used for interfacing with the  backup system */
struct volDescription {    /*used for interfacing with the backup system */
    char volName[VOLSER_MAXVOLNAME];/* should be VNAMESIZE as defined in volume.h*/
    int32 volId;
    int volSize;
    int32 volFlags;
    int32 volCloneId;
};

struct partList {   /*used by the backup system */
    int32 partId[VOLMAXPARTS];
    int32 partFlags[VOLMAXPARTS];
};

#define	STDERR	stderr
#define	STDOUT	stdout

#define ISNAMEVALID(name) (strlen(name) < (VOLSER_OLDMAXVOLNAME - 9))

/* values for flags in struct nvldbEntry */
#define RW_EXISTS 0x1000
#define RO_EXISTS 0x2000
#define BACK_EXISTS 0x4000

/* values for serverFlags in struct nvldbEntry */
#define NEW_REPSITE 0x01
#define ITSROVOL    0x02
#define ITSRWVOL    0x04
#define ITSBACKVOL  0x08
#define RO_DONTUSE  0x20

#define VLOP_RESTORE 0x100/*this is bogus, clashes with VLOP_DUMP */
#define VLOP_ADDSITE 0x80 /*this is bogus, clashes with VLOP_DELETE*/
#define PARTVALID 0x01
#define CLONEVALID 0x02
#define CLONEZAPPED 0x04
#define IDVALID 0x08
#define NAMEVALID 0x10
#define SIZEVALID 0x20
#define ENTRYVALID 0x40
#define REUSECLONEID 0x80
#define	VOK 0x02 

/* Values for the UV_RestoreVolume flags parameter */
#define RV_FULLRST 0x1
#define RV_OFFLINE 0x2

#endif /* _VOLSER_ */








/* End of prolog file volser.p.h. */

#define VOLSERTRELE_ERROR                        (1492325120L)
#define VOLSERNO_OP                              (1492325121L)
#define VOLSERREAD_DUMPERROR                     (1492325122L)
#define VOLSERDUMPERROR                          (1492325123L)
#define VOLSERATTACH_ERROR                       (1492325124L)
#define VOLSERILLEGAL_PARTITION                  (1492325125L)
#define VOLSERDETACH_ERROR                       (1492325126L)
#define VOLSERBAD_ACCESS                         (1492325127L)
#define VOLSERVLDB_ERROR                         (1492325128L)
#define VOLSERBADNAME                            (1492325129L)
#define VOLSERVOLMOVED                           (1492325130L)
#define VOLSERBADOP                              (1492325131L)
#define VOLSERBADRELEASE                         (1492325132L)
#define VOLSERVOLBUSY                            (1492325133L)
#define VOLSERNO_MEMORY                          (1492325134L)
#define VOLSERNOVOL                              (1492325135L)
#define VOLSERMULTIRWVOL                         (1492325136L)
#define VOLSERFAILEDOP                           (1492325137L)
extern void initialize_vols_error_table ();
#define ERROR_TABLE_BASE_vols (1492325120L)

/* for compatibility with older versions... */
#define init_vols_err_tbl initialize_vols_error_table
#define vols_err_base ERROR_TABLE_BASE_vols
