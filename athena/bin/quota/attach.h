/*
 * $Id: attach.h,v 1.2 1991-07-18 22:46:59 probe Exp $
 *
 * Copyright (c) 1988,1991 by the Massachusetts Institute of Technology.
 *
 * For redistribution rights, see "mit-copyright.h"
 */

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

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
#if 0
    int		mount_type;
    int		flags;
    char	*good_flags;
    int		(*attach)();
    int		(*detach)();
    char **	(*explicit)();
    int		(*flush)();
#endif
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


extern void lock_attachtab(), unlock_attachtab();
extern void get_attachtab(), free_attachtab();
extern struct _attachtab *attachtab_lookup_mntpt(), *attachtab_first;
extern struct _fstypes *get_fs();
    

#if !defined(__STDC__) && !(defined(AIX) && defined(i386))
#define	const
#endif
