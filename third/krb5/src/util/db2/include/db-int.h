/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)compat.h	8.13 (Berkeley) 2/21/94
 */

#ifndef	_DB_INT_H_
#define	_DB_INT_H_

#include "db.h"

/* deal with autoconf-based stuff (db.h includes db-config.h) */

#ifndef HAVE_MEMMOVE
#define memmove my_memmove
#endif

#ifndef HAVE_MKSTEMP
#define mkstemp my_mkstemp
#endif

#ifndef HAVE_STRERROR
#define strerror my_strerror
#endif

#define DB_LITTLE_ENDIAN 1234
#define DB_BIG_ENDIAN 4321

#ifdef WORDS_BIGENDIAN
#define DB_BYTE_ORDER DB_BIG_ENDIAN
#else
#define DB_BYTE_ORDER DB_LITTLE_ENDIAN
#endif

/* end autoconf-based stuff */

/* include necessary system header files */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

/* types and constants used for database structure */

#define	MAX_PAGE_NUMBER	0xffffffff	/* >= # of pages in a file */
typedef u_int32_t	db_pgno_t;
#define	MAX_PAGE_OFFSET	65535		/* >= # of bytes in a page */
typedef u_int16_t	indx_t;
#define	MAX_REC_NUMBER	0xffffffff	/* >= # of records in a tree */
typedef u_int32_t	recno_t;

/*
 * Little endian <==> big endian 32-bit swap macros.
 *	M_32_SWAP	swap a memory location
 *	P_32_SWAP	swap a referenced memory location
 *	P_32_COPY	swap from one location to another
 */
#define	M_32_SWAP(a) {							\
	u_int32_t _tmp = a;						\
	((char *)&a)[0] = ((char *)&_tmp)[3];				\
	((char *)&a)[1] = ((char *)&_tmp)[2];				\
	((char *)&a)[2] = ((char *)&_tmp)[1];				\
	((char *)&a)[3] = ((char *)&_tmp)[0];				\
}
#define	P_32_SWAP(a) {							\
	u_int32_t _tmp = *(u_int32_t *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[3];				\
	((char *)a)[1] = ((char *)&_tmp)[2];				\
	((char *)a)[2] = ((char *)&_tmp)[1];				\
	((char *)a)[3] = ((char *)&_tmp)[0];				\
}
#define	P_32_COPY(a, b) {						\
	((char *)&(b))[0] = ((char *)&(a))[3];				\
	((char *)&(b))[1] = ((char *)&(a))[2];				\
	((char *)&(b))[2] = ((char *)&(a))[1];				\
	((char *)&(b))[3] = ((char *)&(a))[0];				\
}

/*
 * Little endian <==> big endian 16-bit swap macros.
 *	M_16_SWAP	swap a memory location
 *	P_16_SWAP	swap a referenced memory location
 *	P_16_COPY	swap from one location to another
 */
#define	M_16_SWAP(a) {							\
	u_int16_t _tmp = a;						\
	((char *)&a)[0] = ((char *)&_tmp)[1];				\
	((char *)&a)[1] = ((char *)&_tmp)[0];				\
}
#define	P_16_SWAP(a) {							\
	u_int16_t _tmp = *(u_int16_t *)a;				\
	((char *)a)[0] = ((char *)&_tmp)[1];				\
	((char *)a)[1] = ((char *)&_tmp)[0];				\
}
#define	P_16_COPY(a, b) {						\
	((char *)&(b))[0] = ((char *)&(a))[1];				\
	((char *)&(b))[1] = ((char *)&(a))[0];				\
}

/* open functions for each database type, used in dbopen() */

DB	*__bt_open __P((const char *, int, int, const BTREEINFO *, int));
DB	*__hash_open __P((const char *, int, int, const HASHINFO *, int));
DB	*__rec_open __P((const char *, int, int, const RECNOINFO *, int));
void	 __dbpanic __P((DB *dbp));

/*
 * There is no portable way to figure out the maximum value of a file
 * offset, so we put it here.
 */
#ifdef	OFF_T_MAX
#define	DB_OFF_T_MAX	OFF_T_MAX
#else
#define	DB_OFF_T_MAX	LONG_MAX
#endif

#ifndef O_ACCMODE			/* POSIX 1003.1 access mode mask. */
#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)
#endif

/*
 * If you can't provide lock values in the open(2) call.  Note, this
 * allows races to happen.
 */
#ifndef O_EXLOCK			/* 4.4BSD extension. */
#define	O_EXLOCK	0
#endif

#ifndef O_SHLOCK			/* 4.4BSD extension. */
#define	O_SHLOCK	0
#endif

#ifndef EFTYPE
#define	EFTYPE		EINVAL		/* POSIX 1003.1 format errno. */
#endif

#ifndef	STDERR_FILENO
#define	STDIN_FILENO	0		/* ANSI C #defines */
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2
#endif

#ifndef SEEK_END
#define	SEEK_SET	0		/* POSIX 1003.1 seek values */
#define	SEEK_CUR	1
#define	SEEK_END	2
#endif

#ifndef NULL				/* ANSI C #defines NULL everywhere. */
#define	NULL		0
#endif

#ifndef	MAX				/* Usually found in <sys/param.h>. */
#define	MAX(_a,_b)	((_a)<(_b)?(_b):(_a))
#endif
#ifndef	MIN				/* Usually found in <sys/param.h>. */
#define	MIN(_a,_b)	((_a)<(_b)?(_a):(_b))
#endif

#ifndef S_ISDIR				/* POSIX 1003.1 file type tests. */
#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000)	/* fifo */
#endif
#ifndef S_ISLNK				/* BSD POSIX 1003.1 extensions */
#define	S_ISLNK(m)	((m & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	((m & 0170000) == 0140000)	/* socket */
#endif

/* Athena kludge to stop exporting/using symbols in the OS namespace. */
#define __add_bigpage		db__add_bigpage
#define __add_ovflpage		db__add_ovflpage
#define __addel			db__addel
#define __big_delete		db__big_delete
#define __big_insert		db__big_insert
#define __big_keydata		db__big_keydata
#define __big_return		db__big_return
#define __bt_bdelete		db__bt_bdelete
#define __bt_close		db__bt_close
#define __bt_cmp		db__bt_cmp
#define __bt_curdel		db__bt_curdel
#define __bt_defcmp		db__bt_defcmp
#define __bt_defpfx		db__bt_defpfx
#define __bt_delete		db__bt_delete
#define __bt_dleaf		db__bt_dleaf
#define __bt_fd			db__bt_fd
#define __bt_first		db__bt_first
#define __bt_free		db__bt_free
#define __bt_get		db__bt_get
#define __bt_new		db__bt_new
#define __bt_open		db__bt_open
#define __bt_pdelete		db__bt_pdelete
#define __bt_pgin		db__bt_pgin
#define __bt_pgout		db__bt_pgout
#define __bt_put		db__bt_put
#define __bt_relink		db__bt_relink
#define __bt_ret		db__bt_ret
#define __bt_search		db__bt_search
#define __bt_seq		db__bt_seq
#define __bt_seqadv		db__bt_seqadv
#define __bt_seqset		db__bt_seqset
#define __bt_setcur		db__bt_setcur
#define __bt_snext		db__bt_snext
#define __bt_split		db__bt_split
#define __bt_sprev		db__bt_sprev
#define __bt_stkacq		db__bt_stkacq
#define __bt_sync		db__bt_sync
#define __call_hash		db__call_hash
#define __cur_db		db__cur_db
#define __cursor_creat		db__cursor_creat
#define __dberr			db__dberr
#define __dbpanic		db__dbpanic
#define __default_hash		db__default_hash
#define __delete_page		db__delete_page
#define __delpair		db__delpair
#define __expand_table		db__expand_table
#define __find_bigpair		db__find_bigpair
#define __free_ovflpage		db__free_ovflpage
#define __get_bigkey		db__get_bigkey
#define __get_item		db__get_item
#define __get_item_done		db__get_item_done
#define __get_item_first	db__get_item_first
#define __get_item_next		db__get_item_next
#define __get_item_reset	db__get_item_reset
#define __get_page		db__get_page
#define __hash_open		db__hash_open
#define __ibitmap		db__ibitmap
#define __log2			db__log2
#define __new_page		db__new_page
#define __ovfl_delete		db__ovfl_delete
#define __ovfl_get		db__ovfl_get
#define __ovfl_put		db__ovfl_put
#define __pgin_routine		db__pgin_routine
#define __pgout_routine		db__pgout_routine
#define __put_page		db__put_page
#define __rec_close		db__rec_close
#define __rec_delete		db__rec_delete
#define __rec_dleaf		db__rec_dleaf
#define __rec_fd		db__rec_fd
#define __rec_fmap		db__rec_fmap
#define __rec_fpipe		db__rec_fpipe
#define __rec_get		db__rec_get
#define __rec_iput		db__rec_iput
#define __rec_open		db__rec_open
#define __rec_put		db__rec_put
#define __rec_ret		db__rec_ret
#define __rec_search		db__rec_search
#define __rec_seq		db__rec_seq
#define __rec_sync		db__rec_sync
#define __rec_vmap		db__rec_vmap
#define __rec_vpipe		db__rec_vpipe
#define __split_page		db__split_page

#endif /* _DB_INT_H_ */
