/*
 * Contains the local configuration information for attach/detach/nfsid
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/attach/config.h,v $
 *	$Author: jfc $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/attach/config.h,v 1.3 1990-07-09 02:34:09 jfc Exp $
 */

/*
 * Configuration defines
 *
 * Warning:  attach may not compile if NFS is not defined... given
 * 	that attach was originally designed just for NFS filesystems,
 * 	this isn't so surprising.  Sigh.  This is true, to a lesser
 * 	extent, for KERBEROS.
 *
 * NEED_STRTOK means that we're on a system that does not have
 * strtok() support in libc.  In practice this usually means pre BSD
 * 4.3 systems.
 *
 * OLD-KERBEROS means we're compiling with the old, buggy kerberos
 * library.  This is necessary because of release skew.  Note that the
 * some gratuitous name changes took place between the new and old
 * kerberos libraries.
 */
#define NFS
#define AFS
#if !defined(AIX) && !defined(_AUX_SOURCE)
#define	RVD
#define UFS
#endif
#define ZEPHYR
#define HESIOD
#define KERBEROS

#ifdef AIX
#define	_BSD
#define	BSD
#define	BSD_INCLUDES
#endif

/*
 * Other external filenames
 */

#define ATTACHCONFFILE	"/etc/attach.conf"
#define ATTACHTAB	"/usr/tmp/attachtab"
#define MTAB		"/etc/mtab"
#define FSCK_FULLNAME "/etc/fsck"
#define FSCK_SHORTNAME "fsck"
#define RVDGETM_FULLNAME "/etc/athena/rvdgetm"
#define RVDGETM_SHORTNAME "rvdgetm"
#define NOSUID_FILENAME "/etc/nosuid"

/*
 * Kerberos instance
 */
#ifdef KERBEROS
#ifdef NFS
#define KERB_NFSID_INST	"rvdsrv"
#endif
#endif

/*
 * Default mount directory for afs
 */
#ifdef AFS
#define AFS_MOUNT_DIR "/mit"
#endif

/*
 * This is the type of function required by signal.  Since it changes
 * from system to system, it is declared here.
 */
#ifdef ultrix
typedef void	sig_catch;
#else
typedef int	sig_catch;
#endif

/*
 * Here is a definition for malloc(), which may or may not be required
 * for various systems.
 */
char *malloc();


/*
 *  This is a set of horrible hacks to make attach support Ultrix as
 * easily as possible.  Praise (and Blame) belongs to John Kohl.
 */
#ifdef ultrix
/* need to re-name some structures for Ultrix */
#define	ufs_args	ufs_specific
#define	nfs_args	nfs_gfs_mount

/* define a struct mntent for convenience of mount.c & unmount.c.  Used
   mainly as a convenient place to store pointers to stuff needed
   for the Ultrix mount() and umount() syscalls. */

struct mntent {
    char *mnt_fsname;
    char *mnt_dir;
    int mnt_type;
    char *mnt_opts;
    int mnt_freq;
    int mnt_passno;
};

/* hacks for filesystem type */
#define	MOUNT_NFS	GT_NFS
#define	MOUNT_UFS	GT_ULTRIX

/* hacks for M_ names */
#define	M_RDONLY	M_RONLY

#define PGUNITS		1024	/* to convert MINPGTHRESH to K */
#define DEFPGTHRESH	64	/* default page threshhold */

#endif /* ultrix compat stuff */

/* These are not defined or recognized by the system, but they are useful
   to allow common data structures with systems that do have these defines */
#ifdef AIX
#define	MOUNT_UFS	1
#define	MOUNT_NFS	2
#endif
