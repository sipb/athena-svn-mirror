/*
 * $Id: attach.h,v 1.18 1995-07-21 00:36:43 cfields Exp $
 *
 * Copyright (c) 1988,1991 by the Massachusetts Institute of Technology.
 *
 * For redistribution rights, see "mit-copyright.h"
 */

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifdef _AIX
#undef geterrno
#endif
#include <netdb.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#ifdef NFS
#include <rpc/rpc.h>
#ifndef i386
#ifndef sgi
#include <nfs/nfs.h>
#else
#include <sys/fs/nfs.h>
#endif /* sgi */
#else
#include <rpc/nfs.h>
#endif
#ifdef NeXT
#include <nfs/nfs_mount.h>		/* Newer versions of NFS (?) */
#endif /* NeXT */
#include <rpcsvc/mount.h>
#ifdef _AUX_SOURCE
#include <nfs/mount.h>
#endif
#ifdef SOLARIS
#include <nfs/mount.h>
#include <sys/fs/ufs_mount.h>
#endif
#ifdef sgi
#include <sys/fs/nfs_clnt.h>
#include <sys/mount.h>
#endif
#endif /* NFS */

#ifdef ultrix
#include <ufs/ufs_mount.h>
#ifdef NFS
#include <nfs/nfs_gfs.h>
#endif /* NFS */
#define KERNEL
/* AACK!  this @#OU@#)($(#) file defines and initializes an array which
   loses on multiple includes, but it's surrounded by #ifdef KERNEL. */
#include <sys/fs_types.h>
#undef KERNEL
#endif /* ultrix */

#include <sys/mount.h>
#ifdef _AIX
#include <sys/vmount.h>
#define	M_RDONLY	MNT_READONLY
#endif

#if defined(_AUX_SOURCE) || defined(NeXT) || defined(_AIX) || defined(sgi)
#define	vfork	fork
#endif

/*
 * If MOUNT_CMD or UMOUNT_CMD are defined, it will run the program
 * specified rather than trying to use in-line code.
 */
#if defined(_IBMR2)
#define MOUNT_CMD	"/etc/mount"
#endif
#if defined(SOLARIS)
#define MOUNT_CMD "/etc/fs/nfs/mount"
#define UMOUNT_CMD "/usr/sbin/umount"
#endif
#if defined(sgi)
#define MOUNT_CMD "/sbin/mount"
#define UMOUNT_CMD "/sbin/umount"
#endif

#define MAXOWNERS 64
#define MAXHOSTS 64

/*
 * We don't really want to deal with malloc'ing and free'ing stuff
 * in this structure...
 */

struct _attachtab {
	struct _attachtab	*next, *prev;
	char		version[3];
	char		explicit;
	char		status;
	char		mode;
	struct _fstypes	*fs;
	struct		in_addr hostaddr[MAXHOSTS];
	int		rmdir;
	int		drivenum;
	int		flags;
	int		nowners;
	uid_t		owners[MAXOWNERS];
	char		hesiodname[BUFSIZ];
	char		host[BUFSIZ];
	char		hostdir[MAXPATHLEN];
	char		mntpt[MAXPATHLEN];
};

/*
 * Attach flags defines
 *
 * FLAG_NOSETUID --- this filesystem was mounted nosetuid (no meaning
 * 	for afs filesystems)
 * FLAG_LOCKED --- this filesystem is passed over by detach -a, and
 * 	you must be the owner of the filesystem to detach it.
 * FLAG_ANYONE --- anyone can detach this filesystem  (not yet implemented)
 * FLAG_PERMANENT --- when this filesytem is detached, don't do
 * 	actually unmount it; just deauthenticate, if necessary.  attach
 * 	sets this flag if it finds the filesystem already mounted but
 * 	not in attachtab.
 */
#define FLAG_NOSETUID	1
#define FLAG_LOCKED	2
#define FLAG_ANYONE	4
#define FLAG_PERMANENT	8

#define ATTACH_VERSION	"A1"

#define ATTACHTABMODE	644

#define STATUS_ATTACHED	       	'+'
#define STATUS_ATTACHING	'*'
#define STATUS_DETACHING	'-'

#define TYPE_NFS	001
#define TYPE_RVD	002
#define TYPE_UFS	004
#define TYPE_ERR	010
#define TYPE_AFS	020
#define TYPE_MUL	040
#define ALL_TYPES	067

/*
 * Attach configuration defines
 */
#define MAXFILTAB	100
#define MAXTRUIDTAB	100

/*
 * Type table
 */

struct _fstypes {
    char	*name;
    int		type;
    int		mount_type;
    int		flags;
    char	*good_flags;
    int		(*attach)();
    int		(*detach)();
    char **	(*explicit)();
    int		(*flush)();
};

/*
 * Flags for _fstypes.flags
 */
#define AT_FS_MNTPT		1
#define AT_FS_REMOTE		2
#define AT_FS_PARENTMNTPT	4
#define AT_FS_MNTPT_CANON	8

extern struct _fstypes fstypes[];

/*
 * Mount options
 */
#ifndef M_RDONLY
#define M_RDONLY	0x01		/* mount fs read-only */
#endif
#ifndef M_NOSUID
#define M_NOSUID	0x02		/* mount fs without setuid perms */
#endif

/*
 * Mount option table
 */
struct mntopts {
	int	type;		/* File system type */
	int	flags;		/* Mount flags */
#ifdef NFS
	int	nfs_port;	/* Valid only for NFS, port for rpc.mountd */
#endif
	union tsa {
#ifdef UFS
		struct ufs_args	ufs;
#endif
#ifdef NFS
		struct nfs_args nfs;
#endif
	} tsa;
};

/*
 * Command option lists
 */

struct command_list {
    char	*large;
    char	*small;
};

/*
 * RVD defines
 */

#define RVD_ATTACH_TIMEOUT	30

/*
 * RPC caching
 */

#define RPC_MAXCACHE 10

struct cache_ent {
    struct	in_addr addr;
    CLIENT	*handle;
    struct	sockaddr_in sin;
    int		fd;
    int		error;
};

/*
 * Calls to RPC.MOUNTD
 */

#ifndef MOUNTPROC_KUIDMAP
#define MOUNTPROC_KUIDMAP	7
#define MOUNTPROC_KUIDUMAP	8
#define MOUNTPROC_KUIDPURGE	9
#define MOUNTPROC_KUIDUPURGE	10
#endif

/*
 * Command names
 */

#define ATTACH_CMD	"attach"
#define DETACH_CMD	"detach"
#define NFSID_CMD	"nfsid"
#define FSID_CMD	"fsid"
#ifdef ZEPHYR
#define ZINIT_CMD	"zinit"
#endif /* ZEPHYR */
    
/*
 * Generic defines
 */

#define SUCCESS 0
#define FAILURE 1

/*
 * Error status defininitions
 */

#define ERR_NONE	0	/* No error */
#define ERR_BADARGS	1	/* Bad arguments */
#define ERR_SOMETHING	2	/* Something wrong - > 1 args */
#define ERR_FATAL	3	/* Internal failure */
#define ERR_INTERRUPT	4	/* Program externally aborted */
#define ERR_BADCONF	5	/* Bad configuration file */
#define ERR_BADFSDSC	6	/* Bad filesystem description */
#define ERR_BADFSFLAG	7	/* Bad filsys flag */

#define ERR_KERBEROS	10	/* Kerberos failure */
#define ERR_HOST	11	/* General host communication failure */
#define ERR_AUTHFAIL	12	/* Authentication failure */
#define ERR_NOPORTS	13	/* Out of reserved ports */

#define ERR_NFSIDNOTATTACHED 20	/* Filesystem with -f not attached */
#define ERR_NFSIDBADHOST 21	/* Can't resolve hostname */
#define	ERR_NFSIDPERM	22	/* unauthorized nfsid -p */

#define ERR_ATTACHBADFILSYS 20	/* Bad filesystem name */
#define ERR_ATTACHINUSE	21	/* Filesystem in use by another proc */
#define ERR_ATTACHNEEDPW 22	/* RVD spinup needs a password */
#define ERR_ATTACHFSCK	23	/* FSCK returned error on RVD */
#define ERR_ATTACHNOTALLOWED 24	/* User not allowed to do operation */
#define ERR_ATTACHBADMNTPT 25	/* User not allowed to mount a */
				/* filesystem here */
#define ERR_ATTACHNOFILSYS 26	/* The remote filesystem doesn't exist */
#define ERR_ATTACHDIRINUSE 27 /* Some other filesystem is using the */
                              /* mountpoint directory */

#define ERR_DETACHNOTATTACHED 20 /* Filesystem not attached */
#define ERR_DETACHINUSE 21	/* Filesystem in use by another proc */
#define ERR_DETACHNOTALLOWED 22	/* User not allowed to do operations */

#define	ERR_ZINITZLOSING	20	/* Random zephyr lossage */

/*
 * Zephyr definitions
 */

#ifdef ZEPHYR
#define ZEPHYR_CLASS "filsrv"
#define ZEPHYR_MAXSUBS 100	/* 50 filesystems... */
#define ZEPHYR_TIMEOUT  60	/* 1 minute timeout */
#endif /* ZEPHYR */

/* AFS */
#ifdef AFS
#ifdef __STDC__
extern int afs_auth(const char *, const char *), afs_auth_to_cell(const char *);
extern int afs_zinit(const char *, const char *);
#else
extern int afs_auth(), afs_auth_to_cell();
#endif
#endif

/*
 * Externals
 */

extern	char	*errstr();	/* convert errno to string */

AUTH	*spoofunix_create_default();
CLIENT	*rpc_create();
extern char *strdup(), *strtok();
extern int errno;
extern unsigned long rvderrno;
extern char *sys_errlist[];
extern char **build_hesiod_line(), **conf_filsys_resolve();
extern struct _fstypes *get_fs();
extern char *filsys_options();
extern char *stropt(), *struid(), *path_canon();
extern struct _attachtab *attachtab_lookup(), *attachtab_lookup_mntpt();

extern int verbose, debug_flag, map_anyway, do_nfsid, print_path, explicit;
extern int owner_check, owner_list, override, keep_mount;
extern int error_status, force, lock_filesystem, lookup, euid, clean_detach;
extern int exp_mntpt, exp_allow;
#ifdef ZEPHYR
extern int use_zephyr;
#endif /* ZEPHYR */
extern char override_mode, *mount_options, *filsys_type;
extern char *mntpt;
extern int override_suid, default_suid, skip_fsck, nfs_root_hack;
extern char *spoofhost, *attachtab_fn, *mtab_fn, *nfs_mount_dir;
#ifdef AFS
extern char *aklog_fn, *afs_mount_dir;
#endif
extern char *fsck_fn;

extern char *ownerlist();
extern void add_an_owner();
extern int is_an_owner(), real_uid, effective_uid, owner_uid;

extern char internal_getopt();
extern void mark_in_use(), add_options(), check_root_privs();

extern char exp_hesline[BUFSIZ];	/* Place to store explicit */
extern char *exp_hesptr[2];		/* ``hesiod'' entry */
extern char *abort_msg;

/* High C 2.1 can optimize small bcopys such as are used to copy 4
   byte IP addrs */
#ifdef __HIGHC__
#define bcopy(src, dest, cnt)	memcpy(dest, src, cnt)
extern char *memcpy();
#endif
#ifdef __STDC__
#ifdef NFS
extern int	nfsid(const char *, struct in_addr, int, int, const char *, int, int);
extern AUTH	*spoofunix_create_default(char *, int);
#endif
extern int	attach(const char *), detach(const char *);
extern	void	detach_all(void), detach_host(const char *);
extern int	read_config_file(const char *);
extern int	parse_username(const char *);
extern int	trusted_user(int);
extern void	lock_attachtab(void), unlock_attachtab(void);
extern void	lint_attachtab(void), get_attachtab(void), free_attachtab(void);
#ifdef ZEPHYR
extern	int	zephyr_sub(int), zephyr_unsub(int);
extern	void	zephyr_addsub(const char *);
#endif
#else
#ifdef NFS
extern int	nfsid();
extern AUTH	*spoofunix_create_default();
#endif
extern	int	attach(), detach();
extern	void	detach_all(), detach_host();
extern	int	read_config_file(), parse_username(), trusted_user();
extern	void	lock_attachtab(), unlock_attachtab();
extern	void	get_attachtab(), free_attachtab();
#ifdef ZEPHYR
extern	int	zephyr_sub(), zephyr_unsub();
extern	void	zephyr_addsub();
#endif
#endif

extern	char	*progname;

/*
 * Instead of ifdef-ing all const declarations, make one global definition.
 * The AIX PS/2 compiler understands "const", but does not define __STDC__.
 */
#if !defined(__STDC__) && !(defined(AIX) && defined(i386))
#define	const
#endif
