/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vldb.h,v 2.2 1993/11/08 21:27:45 vasilis Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vldb.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidvldb = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vldb.h,v 2.2 1993/11/08 21:27:45 vasilis Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*

	System:		VICE-TWO
	Module:		vldb.h
	Institution:	The Information Technology Center, Carnegie-Mellon University

 */

/* Note: this structure happens to be 64 bytes long which isn't real important.  But it seemed like
   a nice number, and the code currently does use a shift. */

#define MAXVOLTYPES	4	/* Maximum number of different types of volumes, each of which can be
				   associated with the current volume */
#define MAXVOLSERVERS   8	/* Maximum number of servers that can be recorded in the vldb as serving
				   a single volume */
struct vldb {
    char key[33];		/* Name or volume id, in ascii, null terminated */
    byte obsolete_hashNext;
    byte volumeType;		/* Volume type, as defined in vice.h  (RWVOL, ROVOL, BACKVOL) */
    byte nServers;		/* Number of servers that have this volume */
    u_int32 volumeId[MAXVOLTYPES]; /* *NETORDER* Corresponding volume of each type + 1 extra unused */
    u_int32 nextEntry;	/* *NETORDER* offset to next entry with matching hash code */
    byte serverNumber[MAXVOLSERVERS];/* Server number for each server claiming to know about this volume */
};

#define LOG_VLDBSIZE	 6	/* Assume the structure is 64 bytes */

/* Header takes up entry #0.  0 is not a legit hash code */

struct vldbHeader {
    int32 magic;			/* *NETORDER* Magic number */
    int32 hashSize;		/* *NETORDER* Size to use for hash calculation (see HashString) */
};

#define VLDB_MAGIC 0xABCD4321

#define N_SERVERIDS 256		/* Not easy to change--maximum number of servers */

#define VLDB_PATH "/vice/db/VLDB"
#define VLDB_TEMP "/vice/db/VLDB.new"
#define BACKUPLIST_PATH "/vice/vol/BackupList"

struct vldb *VLDBLookup();
