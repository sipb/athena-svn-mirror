/* $Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/i386_linux22/include/afs/dir.h,v 1.1.1.2 2000-04-12 18:45:59 ghudson Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/third/afsbin/arch/i386_linux22/include/afs/dir.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsiddir = "$Header: /afs/dev.mit.edu/source/repository/third/afsbin/arch/i386_linux22/include/afs/dir.h,v 1.1.1.2 2000-04-12 18:45:59 ghudson Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/* File:			dir.hx
Author:			Mike Kazar
Quote:			Speed Kills
Quoted Celebrity:		Forest Baskett
Date:			Now */

#if	!defined(__AFS_DIR_H)

#define __AFS_DIR_H

#define AFS_PAGESIZE 2048	/* bytes per page */
#define NHASHENT 128	/* entries in the hash tbl */
#define MAXPAGES 128	/* max pages in a dir */
#define	BIGMAXPAGES 1023   /* new big max pages */
#define EPP 64		/* dir entries per page */
#define LEPP 6		/* log above */
/* When this next field changs, it is crucial to modify MakeDir, since the latter is responsible for marking these entries as allocated.  Also change the salvager. */
#define DHE 12		/* entries in a dir header above a pages header alone. */

#define FFIRST 1
#define FNEXT 2

struct MKFid
    {/* A file identifier. */
    afs_int32 vnode;	/* file's vnode slot */
    afs_int32 vunique;	/* the slot incarnation number */
    };

struct PageHeader
    {/* A page header entry. */
    unsigned short pgcount;	/* number of pages, or 0 if old-style */
    unsigned short tag;		/* 1234 in network byte order */
    char freecount;	/* unused, info in dirHeader structure */
    char freebitmap[EPP/8];
    char padding[32-(5+EPP/8)];
    };

struct DirHeader
    {/* A directory header object.
     */struct PageHeader header;
    char alloMap[MAXPAGES];    /* one byte per 2K page */
    unsigned short hashTable[NHASHENT];
    };

struct DirEntry
    {/* A directory entry */
    char flag;
    char length;	/* currently unused */
    unsigned short next;
    struct MKFid fid;
    char name[16];
    };

struct DirXEntry
    {/* A directory extension entry. */
    char name[32];
    };

struct DirPage0
    {/* A page in a directory. */
    struct DirHeader header;
    struct DirEntry entry[1];
    };

struct DirPage1
    {/* A page in a directory. */
    struct PageHeader header;
    struct DirEntry entry[1];
    };

/*
 * Note that this declaration is seen in both the kernel code and the
 * user space code.  One implementation is in afs/afs_buffer.c; the
 * other is in dir/buffer.c.
 */
extern int DVOffset(void *ap);

#endif /*	!defined(__AFS_DIR_H) */