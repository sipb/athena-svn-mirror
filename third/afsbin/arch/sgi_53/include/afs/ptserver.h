/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/ptserver/RCS/ptserver.h,v 2.6 1996/12/10 19:09:40 thakur Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/ptserver/RCS/ptserver.h,v $ */

/* Copyright (C) 1989 Transarc Corporation - All rights reserved */

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1988
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

/*	
       Sherri Nichols
       Information Technology Center
       November, 1988
*/

#include "ptint.h"

#define	PRSRV		73

#define	ENTRYSIZE	192
#define	HASHSIZE	8191

#define	PRBADID		0x80000000

#define	SYSADMINID	-204
#define SYSBACKUPID     -205
#define	ANYUSERID	-101
#define	AUTHUSERID	-102
#define	ANONYMOUSID	32766

#define PRDBVERSION	0

struct prheader {
    int32 version;			/* database version number */
    int32 headerSize;			/* bytes in header (almost version#) */
    int32 freePtr;			/* first free entry in freelist */
    int32 eofPtr;			/* first free byte in file */
    int32 maxGroup;			/* most negative group id */
    int32 maxID;				/* largest user id allocated */
    int32 maxForeign;			/* largest foreign id allocated*/
    int32 maxInst;			/* largest sub/super id allocated */
    int32 orphan;			/* groups owned by deleted users */
    int32 usercount;			/* num users in system */
    int32 groupcount;			/* num groups in system */
    int32 foreigncount;			/* num registered foreign users NYI*/
    int32 instcount;			/* number of sub and super users NYI */
    int32 reserved[5];			/* just in case */
    int32 nameHash[HASHSIZE];		/* hash table for names */
    int32 idHash[HASHSIZE];		/* hash table for ids */
};

extern struct prheader cheader;

#define set_header_word(tt,field,value) \
  pr_Write ((tt), 0, ((char *)&(cheader.field) - (char *)&cheader),   \
	    ((cheader.field = (value)), (char *)&(cheader.field)),    \
	    sizeof(int32))

#define inc_header_word(tt,field,inc) \
  pr_Write ((tt), 0, ((char *)&(cheader.field) - (char *)&cheader), \
	    ((cheader.field = (htonl(ntohl(cheader.field)+(inc)))),	    \
	     (char *)&(cheader.field)),				    \
	    sizeof(int32))

#define	PRFREE		1		/* 1 if in free list */
#define	PRGRP		2		/* 1 if a group entry */
#define	PRCONT		4		/* 1 if an extension block */
#define	PRCELL		8		/* 1 if cell entry */
#define	PRFOREIGN	16		/* 1 if foreign user */
#define	PRINST		32		/* 1 if sub/super instance */

#define PRTYPE		0x3f		/* type bits: only one should be set */
#define PRUSER		0		/* all type bits 0 => user entry */

#define PRACCESS	(1<<6)		/* access checking enabled */
#define PRQUOTA		(1<<7)		/* group creation quota checking on */

/* define the access bits for entries, they are stored in the left half of the
 * entry's flags.  The SetFields interface takes them in the right half.  There
 * are eight bits altogether defining access rights for status, owned, member,
 * add, and remove operations.  For rights with two bits the values are defined
 * to be o=00, m=01, a=10, with 11 reserved.  As implemented, however, it is
 * o=00, m=01, a=1x. */
#define PRIVATE_SHIFT    16		/* move privacy bits to left half of flags */
#define PRP_STATUS_ANY (0x80 << PRIVATE_SHIFT)
#define PRP_STATUS_MEM (0x40 << PRIVATE_SHIFT)
#define PRP_OWNED_ANY  (0x20 << PRIVATE_SHIFT)
#define PRP_MEMBER_ANY (0x10 << PRIVATE_SHIFT)
#define PRP_MEMBER_MEM (0x08 << PRIVATE_SHIFT)
#define PRP_ADD_ANY    (0x04 << PRIVATE_SHIFT)
#define PRP_ADD_MEM    (0x02 << PRIVATE_SHIFT)
#define PRP_REMOVE_MEM (0x01 << PRIVATE_SHIFT)

#define PRP_GROUP_DEFAULT (PRP_STATUS_ANY | PRP_MEMBER_ANY)
#define PRP_USER_DEFAULT (PRP_STATUS_ANY)

#define PR_REMEMBER_TIMES 1

struct prentry {
    int32 flags;				/* random flags */
    int32 id;				/* user or group id*/
    int32 cellid;			/* reserved for cellID */
    int32 next;				/* next block same entry (or freelist) */
#ifdef PR_REMEMBER_TIMES
    u_int32 createTime, addTime, removeTime, changeTime;
    int32 reserved[1];
#else
    int32 reserved[5];
#endif
    int32 entries[PRSIZE];		/* groups a user is a member of (or list of members */
    int32 nextID;			/* id hash table next pointer */
    int32 nextName;			/* name has table next ptr */
    int32 owner;				/* id of owner of entry */
    int32 creator;			/* may differ from owner */
    int32 ngroups;			/* number of groups this user has created - 0 for group entries */
    int32 nusers;			/* number of foreign user entries this user has created - 0 for group entries NYI */
    int32 count;				/* number of members/groups for this group/user */
    int32 instance;			/* number of sub/super instances for this user NYI */
    int32 owned;			        /* chain of groups owned by this entry */
    int32 nextOwned;			/* chain of groups for owner of this entry */
    int32 parent;			/* ptr to super instance  NYI*/
    int32 sibling;			/* ptr to sibling instance  NYI*/
    int32 child;				/* ptr to first child  NYI*/
    char name[PR_MAXNAMELEN];		/* user or group name */
};

struct contentry {			/* continuation of entry */
    int32 flags;		    
    int32 id;
    int32 cellid;
    int32 next;
    int32 reserved[5];
    int32 entries[COSIZE];
};

#define	CROSS_CELL	1		/* Enable it by default */

#if	!defined(PR_START)
#define PR_START                0
#define PR_RESTART              1
#endif


