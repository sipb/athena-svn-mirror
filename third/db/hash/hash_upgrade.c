/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */
#include "db_config.h"

#ifndef lint
static const char revid[] = "$Id: hash_upgrade.c,v 1.1.1.1 2002-02-11 16:29:15 ghudson Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_swap.h"
#include "hash.h"
#include "db_upgrade.h"

/*
 * __ham_30_hashmeta --
 *      Upgrade the database from version 4/5 to version 6.
 *
 * PUBLIC: int __ham_30_hashmeta __P((DB *, char *, u_int8_t *));
 */
int
__ham_30_hashmeta(dbp, real_name, obuf)
	DB *dbp;
	char *real_name;
	u_int8_t *obuf;
{
	DB_ENV *dbenv;
	HASHHDR *oldmeta;
	HMETA30 newmeta;
	u_int32_t *o_spares, *n_spares;
	u_int32_t fillf, maxb, nelem;
	int i, non_zero, ret;

	dbenv = dbp->dbenv;
	memset(&newmeta, 0, sizeof(newmeta));

	oldmeta = (HASHHDR *)obuf;

	/*
	 * The first 32 bytes are similar.  The only change is the version
	 * and that we removed the ovfl_point and have the page type now.
	 */

	newmeta.dbmeta.lsn = oldmeta->lsn;
	newmeta.dbmeta.pgno = oldmeta->pgno;
	newmeta.dbmeta.magic = oldmeta->magic;
	newmeta.dbmeta.version = 6;
	newmeta.dbmeta.pagesize = oldmeta->pagesize;
	newmeta.dbmeta.type = P_HASHMETA;

	/* Move flags */
	newmeta.dbmeta.flags = oldmeta->flags;

	/* Copy: max_bucket, high_mask, low-mask, ffactor, nelem, h_charkey */
	newmeta.max_bucket = oldmeta->max_bucket;
	newmeta.high_mask = oldmeta->high_mask;
	newmeta.low_mask = oldmeta->low_mask;
	newmeta.ffactor = oldmeta->ffactor;
	newmeta.nelem = oldmeta->nelem;
	newmeta.h_charkey = oldmeta->h_charkey;

	/*
	 * There was a bug in 2.X versions where the nelem could go negative.
	 * In general, this is considered "bad."  If it does go negative
	 * (that is, very large and positive), we'll die trying to dump and
	 * load this database.  So, let's see if we can fix it here.
	 */
	nelem = newmeta.nelem;
	fillf = newmeta.ffactor;
	maxb = newmeta.max_bucket;

	if ((fillf != 0 && fillf * maxb < 2 * nelem) ||
	    (fillf == 0 && nelem > 0x8000000))
		newmeta.nelem = 0;

	/*
	 * We now have to convert the spares array.  The old spares array
	 * contained the total number of extra pages allocated prior to
	 * the bucket that begins the next doubling.  The new spares array
	 * contains the page number of the first bucket in the next doubling
	 * MINUS the bucket number of that bucket.
	 */
	o_spares = oldmeta->spares;
	n_spares = newmeta.spares;
	non_zero = 0;
	n_spares[0] = 1;
	for (i = 1; i < NCACHED; i++) {
		non_zero = non_zero || o_spares[i - 1] != 0;
		if (o_spares[i - 1] == 0 && non_zero)
			n_spares[i] = 0;
		else
			n_spares[i] = 1 + o_spares[i - 1];
	}

					/* Replace the unique ID. */
	if ((ret = __os_fileid(dbenv, real_name, 1, newmeta.dbmeta.uid)) != 0)
		return (ret);

	/* Overwrite the original. */
	memcpy(oldmeta, &newmeta, sizeof(newmeta));

	return (0);
}

/*
 * __ham_31_hashmeta --
 *      Upgrade the database from version 6 to version 7.
 *
 * PUBLIC: int __ham_31_hashmeta
 * PUBLIC:      __P((DB *, char *, u_int32_t, DB_FH *, PAGE *, int *));
 */
int
__ham_31_hashmeta(dbp, real_name, flags, fhp, h, dirtyp)
	DB *dbp;
	char *real_name;
	u_int32_t flags;
	DB_FH *fhp;
	PAGE *h;
	int *dirtyp;
{
	HMETA31 *newmeta;
	HMETA30 *oldmeta;

	COMPQUIET(dbp, NULL);
	COMPQUIET(real_name, NULL);
	COMPQUIET(fhp, NULL);

	newmeta = (HMETA31 *)h;
	oldmeta = (HMETA30 *)h;

	/*
	 * Copy the fields down the page.
	 * The fields may overlap so start at the bottom and use memmove().
	 */
	memmove(newmeta->spares, oldmeta->spares, sizeof(oldmeta->spares));
	newmeta->h_charkey = oldmeta->h_charkey;
	newmeta->nelem = oldmeta->nelem;
	newmeta->ffactor = oldmeta->ffactor;
	newmeta->low_mask = oldmeta->low_mask;
	newmeta->high_mask = oldmeta->high_mask;
	newmeta->max_bucket = oldmeta->max_bucket;
	memmove(newmeta->dbmeta.uid,
	    oldmeta->dbmeta.uid, sizeof(oldmeta->dbmeta.uid));
	newmeta->dbmeta.flags = oldmeta->dbmeta.flags;
	newmeta->dbmeta.record_count = 0;
	newmeta->dbmeta.key_count = 0;
	ZERO_LSN(newmeta->dbmeta.alloc_lsn);

	/* Update the version. */
	newmeta->dbmeta.version = 7;

	/* Upgrade the flags. */
	if (LF_ISSET(DB_DUPSORT))
		F_SET(&newmeta->dbmeta, DB_HASH_DUPSORT);

	*dirtyp = 1;
	return (0);
}

/*
 * __ham_31_hash --
 *      Upgrade the database hash leaf pages.
 *
 * PUBLIC: int __ham_31_hash
 * PUBLIC:      __P((DB *, char *, u_int32_t, DB_FH *, PAGE *, int *));
 */
int
__ham_31_hash(dbp, real_name, flags, fhp, h, dirtyp)
	DB *dbp;
	char *real_name;
	u_int32_t flags;
	DB_FH *fhp;
	PAGE *h;
	int *dirtyp;
{
	HKEYDATA *hk;
	db_pgno_t pgno, tpgno;
	db_indx_t indx;
	int ret;

	COMPQUIET(flags, 0);

	ret = 0;
	for (indx = 0; indx < NUM_ENT(h); indx += 2) {
		hk = (HKEYDATA *)H_PAIRDATA(h, indx);
		if (HPAGE_PTYPE(hk) == H_OFFDUP) {
			memcpy(&pgno, HOFFDUP_PGNO(hk), sizeof(db_pgno_t));
			tpgno = pgno;
			if ((ret = __db_31_offdup(dbp, real_name, fhp,
			    LF_ISSET(DB_DUPSORT) ? 1 : 0, &tpgno)) != 0)
				break;
			if (pgno != tpgno) {
				*dirtyp = 1;
				memcpy(HOFFDUP_PGNO(hk),
				    &tpgno, sizeof(db_pgno_t));
			}
		}
	}

	return (ret);
}
