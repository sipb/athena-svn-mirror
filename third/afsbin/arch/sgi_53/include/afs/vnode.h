/* $Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vnode.h,v 2.3 1996/01/04 20:09:48 zumach Exp $ */
/* $Source: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vnode.h,v $ */

#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsidvnode = "$Header: /afs/transarc.com/project/fs/dev/afs/rcs/vol/RCS/vnode.h,v 2.3 1996/01/04 20:09:48 zumach Exp $";
#endif

/*
 * P_R_P_Q_# (C) COPYRIGHT IBM CORPORATION 1987
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */
/*

	System:		VICE-TWO
	Module:		vnode.h
	Institution:	The Information Technology Center, Carnegie-Mellon University

 */

typedef struct ViceLock
{
    int		lockCount;
    int		lockTime;
} ViceLock;

#define ViceLockCheckLocked(vptr) ((vptr)->lockTime == 0)
#define ViceLockClear(vptr) ((vptr)->lockCount = (vptr)->lockTime = 0)

#define ROOTVNODE 1

/*typedef enum {vNull=0, vFile=1, vDirectory=2, vSymlink=3} VnodeType;*/
typedef int VnodeType;
#define vNull 0
#define vFile 1
#define vDirectory 2
#define vSymlink 3

/*typedef enum {vLarge=0,vSmall=1} VnodeClass;*/
#define vLarge	0
#define vSmall	1
typedef int VnodeClass;
#define VNODECLASSWIDTH 1
#define VNODECLASSMASK	((1<<VNODECLASSWIDTH)-1)
#define nVNODECLASSES	(VNODECLASSMASK+1)

struct VnodeClassInfo {
    struct Vnode *lruHead;	/* Head of list of vnodes of this class */
    int diskSize;		/* size of vnode disk object, power of 2 */
    int logSize;		/* log 2 diskSize */
    int residentSize;		/* resident size of vnode */
    int cacheSize;		/* Vnode cache size */
    bit32 magic;		/* Magic number for this type of vnode,
    				   for as long as we're using vnode magic
				   numbers */
    int	allocs;			/* Total number of successful allocation
    				   requests; this is the same as the number
				   of sanity checks on the vnode index */
    int gets,reads;		/* Number of VGetVnodes and corresponding
    				   reads */
    int writes;			/* Number of vnode writes */
} VnodeClassInfo[nVNODECLASSES];

#define vnodeTypeToClass(type)  ((type) == vDirectory? vLarge: vSmall)
#define vnodeIdToClass(vnodeId) ((vnodeId-1)&VNODECLASSMASK)
#define vnodeIdToBitNumber(v) (((v)-1)>>VNODECLASSWIDTH)
/* The following calculation allows for a header record at the beginning
   of the index.  The header record is the same size as a vnode */
#define vnodeIndexOffset(vcp,vnodeNumber) \
    ((vnodeIdToBitNumber(vnodeNumber)+1)<<(vcp)->logSize)
#define bitNumberToVnodeNumber(b,class) (((b)<<VNODECLASSWIDTH)+(class)+1)
#define vnodeIsDirectory(vnodeNumber) (vnodeIdToClass(vnodeNumber) == vLarge)

typedef struct VnodeDiskObject {
    VnodeType	  type:3;	/* Vnode is file, directory, symbolic link
    				   or not allocated */
    unsigned	  cloned:1;	/* This vnode was cloned--therefore the inode
    				   is copy-on-write; only set for directories*/
    unsigned	  modeBits:12;	/* Unix mode bits */
    bit16	  linkCount;	/* Number of directory references to vnode
    				   (from single directory only!) */
    bit32	  length;	/* Number of bytes in this file */
    Unique	  uniquifier;	/* Uniquifier for the vnode; assigned
				   from the volume uniquifier (actually
				   from nextVnodeUnique in the Volume
				   structure) */
    FileVersion   dataVersion;	/* version number of the data */
    Inode	  inodeNumber;	/* inode number of the data attached to
    				   this vnode */
    Date	  unixModifyTime;/* set by user */
    UserId	  author;	/* Userid of the last user storing the file */
    UserId	  owner;	/* Userid of the user who created the file */
    VnodeId	  parent;	/* Parent directory vnode */
    bit32	  vnodeMagic;	/* Magic number--mainly for file server
    				   paranoia checks */
#   define	  SMALLVNODEMAGIC	0xda8c041F
#   define	  LARGEVNODEMAGIC	0xad8765fe
    /* Vnode magic can be removed, someday, if we run need the room.  Simply
       have to be sure that the thing we replace can be VNODEMAGIC, rather
       than 0 (in an old file system).  Or go through and zero the fields,
       when we notice a version change (the index version number) */
    ViceLock	  lock;		/* Advisory lock */
    Date	  serverModifyTime;	/* Used only by the server; for incremental
    				   backup purposes */
    int32	  group;	    /* unix group */
    bit32	  reserved5;
    bit32	  reserved6;
    /* Missing:
       archiving/migration
       encryption key
     */
} VnodeDiskObject;

#define SIZEOF_SMALLDISKVNODE	64
#define CHECKSIZE_SMALLVNODE\
	(sizeof(VnodeDiskObject) == SIZEOF_SMALLDISKVNODE)
#define SIZEOF_LARGEDISKVNODE	256

typedef struct Vnode {
    struct	Vnode *hashNext;/* Next vnode on hash conflict chain */
    struct	Vnode *lruNext;	/* Less recently used vnode than this one */
    struct	Vnode *lruPrev;	/* More recently used vnode than this one */
				/* The lruNext, lruPrev fields are not
				   meaningful if the vnode is in use */
    bit16	hashIndex;	/* Hash table index */
#ifdef	AFS_AIX_ENV
    unsigned	changed_newTime:1;	/* 1 if vnode changed, write time */
    unsigned	changed_oldTime:1; /* 1 changed, don't update time. */
    unsigned	delete:1;	/* 1 if the vnode should be deleted; in
    				   this case, changed must also be 1 */
#else
    byte	changed_newTime:1;	/* 1 if vnode changed, write time */
    byte	changed_oldTime:1; /* 1 changed, don't update time. */
    byte	delete:1;	/* 1 if the vnode should be deleted; in
    				   this case, changed must also be 1 */
#endif
    VnodeId	vnodeNumber;
    struct Volume
    		*volumePtr;	/* Pointer to the volume containing this file*/
    byte	nUsers;		/* Number of lwp's who have done a VGetVnode */
    bit16	cacheCheck;	/* Must equal the value in the volume Header
    				   for the cache entry to be valid */
    struct	Lock lock;	/* Internal lock */
    PROCESS	writer;		/* Process id having write lock */
    VnodeDiskObject disk;	/* The actual disk data for the vnode */
} Vnode;

#define SIZEOF_LARGEVNODE \
	(sizeof(struct Vnode) - sizeof(VnodeDiskObject) + SIZEOF_LARGEDISKVNODE)
#define SIZEOF_SMALLVNODE	(sizeof (struct Vnode))

#define VVnodeDiskACL(v)     /* Only call this with large (dir) vnode!! */ \
	((AL_AccessList *) (((byte *)(v))+SIZEOF_SMALLDISKVNODE))
#define  VVnodeACL(vnp) (VVnodeDiskACL(&(vnp)->disk))
/* VAclSize is defined this way to allow information in the vnode header
   to grow, in a POSSIBLY upward compatible manner.  SIZEOF_SMALLDISKVNODE
   is the maximum size of the basic vnode.  The vnode header of either type
   can actually grow to this size without conflicting with the ACL on larger
   vnodes */
#define VAclSize(vnp)		(SIZEOF_LARGEDISKVNODE - SIZEOF_SMALLDISKVNODE)
#define VAclDiskSize(v)		(SIZEOF_LARGEDISKVNODE - SIZEOF_SMALLDISKVNODE)
extern int VolumeHashOffset();
extern VInitVnodes();
extern Vnode *VGetVnode();
extern VputVnode();
extern Vnode *VAllocVnode();
extern VFreeVnode();
