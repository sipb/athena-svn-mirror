/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _DIRENT_H
#define	_DIRENT_H

#pragma ident	"@(#)dirent.h	1.16	93/08/20 SMI"	/* SVr4.0 1.6.1.5 */

#include <sys/feature_tests.h>

#include <sys/types.h>

#ifdef	__cplusplus
extern "C" {
#endif

#if !defined(_POSIX_C_SOURCE)
#define	MAXNAMLEN	512		/* maximum filename length */
#define	DIRBUF		1048		/* buffer size for fs-indep. dirs */
#endif /* !defined(_POSIX_C_SOURCE) */

typedef struct
	{
	int		dd_fd;		/* file descriptor */
	int		dd_loc;		/* offset in block */
	int		dd_size;	/* amount of valid data */
	char		*dd_buf;	/* directory block */
	}	DIR;			/* stream data from opendir() */

#if defined(__STDC__)

extern DIR		*opendir(const char *);
extern struct dirent	*readdir(DIR *);
#ifdef _REENTRANT
extern struct dirent    *readdir_r(DIR *, struct dirent *);
#endif /* _REENTRANT */
#if !defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
extern long		telldir(DIR *);
extern void		seekdir(DIR *, long);
#endif /* !defined(_POSIX_C_SOURCE) */
extern void		rewinddir(DIR *);
extern int		closedir(DIR *);

#else

extern DIR		*opendir();
extern struct dirent	*readdir();
#if !defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
extern long		telldir();
extern void		seekdir();
#endif /* !defined(_POSIX_C_SOURCE) */
extern void		rewinddir();
extern int		closedir();

#endif

#if !defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
#define	rewinddir(dirp)	seekdir(dirp, 0L)
#endif

#ifdef	__cplusplus
}
#endif

#ifndef _SYS_DIRENT_H
#define	_SYS_DIRENT_H

#include <sys/feature_tests.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * File-system independent directory entry.
 */
struct dirent {
	ino_t		d_ino;		/* "inode number" of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[MAXNAMELEN+1];	/* name of file */
};

typedef	struct	dirent	dirent_t;

#if !defined(_POSIX_C_SOURCE)
#if defined(__STDC__) && !defined(_KERNEL)
int getdents(int, struct dirent *, unsigned);
#else
int getdents();
#endif
#endif /* !defined(_POSIX_C_SOURCE) */

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_DIRENT_H */


#endif	/* _DIRENT_H */
