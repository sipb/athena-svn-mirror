/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_dba.c,v 1.6 1993-05-10 13:42:12 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_dba.c,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#ifndef lint
static char rcs_id[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/quota_dba.c,v 1.6 1993-05-10 13:42:12 vrt Exp $";
#endif lint

#include "mit-copyright.h"
#include "quota.h"
#include "quota_db.h"
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef SOLARIS
/*
 * flock operations.
 */
#define LOCK_SH               1       /* shared lock */
#define LOCK_EX               2       /* exclusive lock */
#define LOCK_NB               4       /* don't block when locking */
#define LOCK_UN               8       /* unlock */
#endif

#ifdef ultrix
/* We are dumb and redefine NULL */
#undef NULL 
#endif

#ifdef NDBM
#include <ndbm.h>
#else /*NDBM*/
#include <dbm.h>
#endif /*NDBM*/
#include <strings.h>

/* Macros to convert ndbm names to dbm names.
 * Note that dbm_nextkey() cannot be simply converted using a macro, since
 * it is invoked giving the database, and nextkey() needs the previous key.
 *
 * Instead, all routines call "dbm_next" instead.
 */

#ifndef NDBM
typedef char DBM;

#define dbm_open(file, flags, mode) ((dbminit(file) == 0)?"":((char *)0))
#define dbm_fetch(db, key) fetch(key)
#define dbm_store(db, key, content, flag) store(key, content)
#define dbm_firstkey(db) firstkey()
#define dbm_next(db,key) nextkey(key)
#define dbm_close(db) dbmclose()
#else
#define dbm_next(db,key) dbm_nextkey(db)
#endif

extern char *malloc();
extern int errno;

static  init = 0;
static char default_db_name[] = DBM_DEF_FILE;
static char *current_db_name = default_db_name;
static int non_blocking = 0;
static struct timeval timestamp;/* current time of request */
static void quota_dbl_fini(), quota_dbl_unlock();

#ifdef DEBUG
extern int debug;
extern long quota_debug;
extern char *progname;
#endif

#define QUOTA_DB_MAX_RETRY 5
/* In order to save time, much of this code has been stolen from 
   the kerberos admin server and modified accordingly.
*/



static void
encode_princ_key(key, name, instance, service, realm)
    datum  *key;
    char   *name, *instance, *service, *realm;
{
    static char keystring[ANAME_SZ + INST_SZ + SERV_SZ + REALM_SZ];

    bzero(keystring, ANAME_SZ + INST_SZ + REALM_SZ);
    (void) strncpy(keystring, name, ANAME_SZ);
    (void) strncpy(&keystring[ANAME_SZ], instance, INST_SZ);
    (void) strncpy(&keystring[ANAME_SZ + INST_SZ], service, SERV_SZ);
    (void) strncpy(&keystring[ANAME_SZ + INST_SZ + SERV_SZ], realm, REALM_SZ);
    key->dsize = ANAME_SZ + INST_SZ + SERV_SZ + REALM_SZ;

    key->dptr = keystring;
}

static void
decode_princ_key(key, name, instance, service, realm)
    datum  *key;
    char   *name, *instance, *service, *realm;
{
    (void) strncpy(name, key->dptr, ANAME_SZ);
    (void) strncpy(instance, key->dptr + ANAME_SZ, INST_SZ);
    (void) strncpy(service, key->dptr + ANAME_SZ + INST_SZ, SERV_SZ);
    (void) strncpy(realm, key->dptr + ANAME_SZ + INST_SZ + SERV_SZ, REALM_SZ);
    
    name[ANAME_SZ - 1] = '\0';
    instance[INST_SZ - 1] = '\0';
    service[SERV_SZ -1] = '\0';
    realm[REALM_SZ - 1] = '\0';
}

static void
encode_princ_contents(contents, qrec)
    datum  *contents;
    quota_rec *qrec;
{
    contents->dsize = sizeof(*qrec);
    contents->dptr = (char *) qrec;
}

static void
decode_princ_contents(contents, qrec)
    datum  *contents;
    quota_rec *qrec;
{
    bcopy(contents->dptr, (char *) qrec, sizeof(*qrec));
}

/*
 * Utility routine: generate name of database file.
 */

static char *gen_dbsuffix(db_name, sfx)
    char *db_name;
    char *sfx;
{
    char *dbsuffix;
    
    if (sfx == NULL)
	sfx = ".ok";

    dbsuffix = malloc ((unsigned) strlen(db_name) + strlen(sfx) + 1);
    (void) strcpy(dbsuffix, db_name);
    (void) strcat(dbsuffix, sfx);
    return dbsuffix;
}

/*
 * initialization for data base routines.
 */

quota_db_init()
{
    init = 1;
    return (0);
}

/*
 * gracefully shut down database--must be called by ANY program that does
 * a quota_db_init 
 */

quota_db_fini()
{
}

/*
 * Set the "name" of the current database to some alternate value.
 *
 * Passing a null pointer as "name" will set back to the default.
 * If the alternate database doesn't exist, nothing is changed.
 */

quota_db_set_name(name)
	char *name;
{
    DBM *db;

    if (name == NULL)
	name = default_db_name;
    db = dbm_open(name, 0, 0);
    if (db == NULL)
	return errno;
    dbm_close(db);
    quota_dbl_fini();
    current_db_name = name;
    return 0;
}

/*
 * Return the last modification time of the database.
 */

long quota_get_db_age()
{
    struct stat st;
    char *okname;
    long age;
    
    okname = gen_dbsuffix(current_db_name, ".ok");

    if (stat (okname, &st) < 0)
	age = 0;
    else
	age = st.st_mtime;

    free (okname);
    return age;
}

/*
 * Remove the semaphore file; indicates that database is currently
 * under renovation.
 *
 * This is only for use when moving the database out from underneath
 * the server (for example, during slave updates).
 */

static long quota_start_update(db_name)
    char *db_name;
{
    char *okname = gen_dbsuffix(db_name, ".ok");
    long age = quota_get_db_age();
    
    if (unlink(okname) < 0
	&& errno != ENOENT) {
	    age = -1;
    }
    free (okname);
    return age;
}

static long quota_end_update(db_name, age)
    char *db_name;
    long age;
{
    int fd;
    int retval = 0;
    char *new_okname = gen_dbsuffix(db_name, ".ok#");
    char *okname = gen_dbsuffix(db_name, ".ok");
    
    fd = open (new_okname, O_CREAT|O_RDWR|O_TRUNC, 0600);
    if (fd < 0)
	retval = errno;
    else {
	struct stat st;
	struct timeval tv[2];
	/* make sure that semaphore is "after" previous value. */
	if (fstat (fd, &st) == 0
	    && st.st_mtime <= age) {
	    tv[0].tv_sec = st.st_atime;
	    tv[0].tv_usec = 0;
	    tv[1].tv_sec = age;
	    tv[1].tv_usec = 0;
	    /* set times.. */
	    if(utimes (new_okname, tv)) 
		syslog(LOG_INFO, "quota_dba: utimes() failed");
	    if(fsync(fd)) 
		syslog(LOG_INFO, "quota_dba: fsync() failed");
	}
	(void) close(fd);
	if (rename (new_okname, okname) < 0)
	    retval = errno;
    }

    free (new_okname);
    free (okname);

    return retval;
}

static long quota_start_read()
{
    return quota_get_db_age();
}

static long quota_end_read(age)
    u_long age;
{
    if (quota_get_db_age() != age || age == -1) {
	return -1;
    }
    return 0;
}

/*
 * Create the database, assuming it's not there.
 */

quota_db_create(db_name)
    char *db_name;
{
    char *okname = gen_dbsuffix(db_name, ".ok");
    int fd;
    register int ret = 0;
#ifdef NDBM
    DBM *db;

    db = dbm_open(db_name, O_RDWR|O_CREAT|O_EXCL, 0600);
    if (db == NULL)
	ret = errno;
    else
	dbm_close(db);
#else
    char *dirname = gen_dbsuffix(db_name, ".dir");
    char *pagname = gen_dbsuffix(db_name, ".pag");

    fd = open(dirname, O_RDWR|O_CREAT|O_EXCL, 0600);
    if (fd < 0)
	ret = errno;
    else {
	(void) close(fd);
	fd = open (pagname, O_RDWR|O_CREAT|O_EXCL, 0600);
	if (fd < 0)
	    ret = errno;
	else
	    (void) close(fd);
    }
    if (dbminit(db_name) < 0)
	ret = errno;
#endif
    if (ret == 0) {
	fd = open (okname, O_CREAT|O_RDWR|O_TRUNC, 0600);
	if (fd < 0)
	    ret = errno;
	(void) close(fd);
    }
    return ret;
}

/*
 * "Atomically" rename the database in a way that locks out read
 * access in the middle of the rename.
 *
 * Not perfect; if we crash in the middle of an update, we don't
 * necessarily know to complete the transaction the rename, but...
 */

quota_db_rename(from, to)
    char *from;
    char *to;
{
    char *fromdir = gen_dbsuffix (from, ".dir");
    char *todir = gen_dbsuffix (to, ".dir");
    char *frompag = gen_dbsuffix (from , ".pag");
    char *topag = gen_dbsuffix (to, ".pag");
    char *fromok = gen_dbsuffix(from, ".ok");
    long trans = quota_start_update(to);
    int ok=0;

    PROTECT();
    if ((rename (fromdir, todir) == 0)
	&& (rename (frompag, topag) == 0)) {
	(void) unlink (fromok);
	ok = 1;
    }
    UNPROTECT();

    free (fromok);
    free (fromdir);
    free (todir);
    free (frompag);
    free (topag);
    if (ok)
	return quota_end_update(to, trans);
    else
	return -1;
}

/*
 * look up a principal in the data base returns number of principals
 * found , and whether there were more than requested. 
 */
/* XXXXX - Needs proper support */
quota_db_get_principal(name, inst, serv, realm, qrec, max, more)
    char   *name;		/* could have wild card */
    char   *inst;		/* could have wild card */
    char   *serv;		/* could have wild card */     
    char   *realm;		/* could have wild card */
    quota_rec *qrec;
    unsigned int max;		/* max number of name structs to return */
    int    *more;		/* where there more than 'max' tuples? */

{
    int     found = 0;
    extern int errorproc();
    int     wildp, wildi, wilds, wildr;
    datum   key, contents;
    char    testname[ANAME_SZ], testinst[INST_SZ];
    char    testrealm[ANAME_SZ], testserv[INST_SZ];
    u_long trans;
    int try;
    DBM    *db;

    if (!init)
	(void) quota_db_init();		/* initialize database routines */

    for (try = 0; try < QUOTA_DB_MAX_RETRY; try++) {
	trans = quota_start_read();
	quota_dbl_fini(); 	/* Attempt to fix the problem of old locks */
	if (quota_dbl_lock(QUOTA_DBL_SHARED) != 0) {

	    return -1;
	}

	db = dbm_open(current_db_name, O_RDONLY, 0600);

	*more = 0;

#ifdef DEBUG
	if (quota_debug & 2)
	    fprintf(stderr,
		    "%s: db_get_principal for %s %s %s %s max = %d",
		    progname, name, inst, serv, realm, max);
#endif

	wildp = !strcmp(name, "*");
	wildi = !strcmp(inst, "*");
	wildr = !strcmp(realm, "*");
	wilds = !strcmp(serv, "*");

	if (!wildi && !wildp && !wilds && !wildr) {	/* nothing's wild */
	    encode_princ_key(&key, name, inst, serv, realm);
	    contents = dbm_fetch(db, key);
	    if (contents.dptr == NULL) {
		found = 0;
		goto done;
	    }
	    decode_princ_contents(&contents, qrec);
#ifdef DEBUG
	    if (quota_debug & 1) {
		fprintf(stderr, "\t found %s %s %s %s p_n length %d t_n length %d i_n length %d i_r length %d\n",
			qrec->name, qrec->instance,
			qrec->service, qrec->realm,
			strlen(qrec->name),
			strlen(qrec->instance),
			strlen(qrec->service),
			strlen(qrec->realm));
	    }
#endif
	    found = 1;
	    goto done;
	}	/* process wild cards by looping through entire database */

	for (key = dbm_firstkey(db); key.dptr != NULL;
	     key = dbm_next(db, key)) {
	    decode_princ_key(&key, testname, testinst, testserv, testrealm);
	    if ((wildp || !strcmp(testname, name)) &&
		(wildi || !strcmp(testinst, inst)) &&
		(wilds || !strcmp(testserv, serv)) &&
		(wildr || !strcmp(testrealm, realm))) { /* have a match */
		if (found >= max) {
		    *more = 1;
		    goto done;
		} else {
		    found++;
		    contents = dbm_fetch(db, key);
		    decode_princ_contents(&contents, qrec);
#ifdef DEBUG
		    if (quota_debug & 1) {
		      fprintf(stderr, "\t found %s %s %s %s p_n length %d t_n length %d i_n length %d i_r length %d\n",
			      qrec->name, qrec->instance,
			      qrec->service, qrec->realm,
			      strlen(qrec->name),
			      strlen(qrec->instance),
			      strlen(qrec->service),
			      strlen(qrec->realm));
		    }
#endif
		    qrec++; /* point to next */
		}
	    }
	}

    done:
	quota_dbl_unlock();	/* unlock read lock */
	dbm_close(db);
	if (quota_end_read(trans) == 0)
	    break;
	found = -2;
	if (!non_blocking)
	    sleep(1);
    }
    return (found);
}

/*
 * Update a name in the data base.  Returns number of names
 * successfully updated.
 */

quota_db_put_principal(qrec, max)
    quota_rec *qrec;
    unsigned int max;		/* number of principal structs to
				 * update */

{
    int     found = 0;
    u_long  i;
    extern int errorproc();
    datum   key, contents;
    DBM    *db;

    (void) gettimeofday(&timestamp, (struct timezone *) NULL);

    if (!init)
	(void) quota_db_init();

    if (quota_dbl_lock(QUOTA_DBL_EXCLUSIVE) != 0)
	return -1;

    db = dbm_open(current_db_name, O_RDWR, 0600);

#ifdef DEBUG
    if (quota_debug & 2)
	fprintf(stderr, "%s: quota_db_put_principal  max = %d",
	    progname, max);
#endif
    PROTECT();
    /* for each one, stuff temps, and do replace/append */
    for (i = 0; i < max; i++) {
	encode_princ_contents(&contents, qrec);
	encode_princ_key(&key, qrec->name, qrec->instance,
			 qrec->service, qrec->realm);
	if(dbm_store(db, key, contents, DBM_REPLACE)) 
	    syslog(LOG_ERR, "quota_dba_put_principal: dbm_store failed");
#ifdef DEBUG
	if (quota_debug & 1) {
	    fprintf(stderr, "\n put %s %s %s %s\n",
		qrec->name, qrec->instance,
		    qrec->service, qrec->realm);
	}
#endif
	found++;
	qrec++;		/* bump to next struct			   */
    }

    dbm_close(db);
    quota_dbl_unlock();		/* unlock database */
    UNPROTECT();
    return (found);
}


quota_db_iterate (func, arg)
    int (*func)();
    char *arg;			/* void *, really */
{
    datum key, contents;
    quota_rec *qrec;
    int code;
    DBM *db;
    
    (void) quota_db_init();		/* initialize and open the database */
    if ((code = quota_dbl_lock(QUOTA_DBL_SHARED)) != 0)
	return code;

    db = dbm_open(current_db_name, O_RDONLY, 0600);

    for (key = dbm_firstkey (db); key.dptr != NULL; key = dbm_next(db, key)) {
	contents = dbm_fetch (db, key);
	/* XXX may not be properly aligned */
	qrec = (quota_rec *) contents.dptr;
	if ((code = (*func)(arg, qrec)) != 0)
	    return code;
    }
    dbm_close(db);
    quota_dbl_unlock();
    return 0;
}

static int dblfd = -1;
static int mylock = 0;
static int inited = 0;

static quota_dbl_init()
{
    if (!inited) {
	char *filename = gen_dbsuffix (current_db_name, ".ok");
	if ((dblfd = open(filename, 0)) < 0) {
	    fprintf(stderr, "quota_dbl_init: couldn't open %s\n", filename);
	    (void) fflush(stderr);
	    perror("open");
	    exit(1);
	}
	free(filename);
	inited++;
    }
    return (0);
}

static void quota_dbl_fini()
{
    if(dblfd != -1) (void) close(dblfd);
    dblfd = -1;
    inited = 0;
    mylock = 0;
}

static int quota_dbl_lock(mode)
    int     mode;
{
    int flock_mode=0;
    
    if (!inited)
	(void) quota_dbl_init();
    if (mylock) {		/* Detect lock call when lock already
				 * locked */
	fprintf(stderr, "quota locking error (mylock)\n");
	(void) fflush(stderr);
	exit(1);
    }
    switch (mode) {
    case QUOTA_DBL_EXCLUSIVE:
	flock_mode = LOCK_EX;
	break;
    case QUOTA_DBL_SHARED:
	flock_mode = LOCK_SH;
	break;
    default:
	fprintf(stderr, "invalid lock mode %d\n", mode);
	abort();
    }
    if (non_blocking)
	flock_mode |= LOCK_NB;
    
    if (flock(dblfd, flock_mode) < 0) 
	return errno;
    mylock++;
    return 0;
}

static void quota_dbl_unlock()
{
    if (!mylock) {		/* lock already unlocked */
	fprintf(stderr, "quota database lock not locked when unlocking.\n");
	(void) fflush(stderr);
	exit(1);
    }
    if (flock(dblfd, LOCK_UN) < 0) {
	fprintf(stderr, "quota database lock error. (unlocking)\n");
	(void) fflush(stderr);
	perror("flock");
	exit(1);
    }
    mylock = 0;
}

int quota_db_set_lockmode(mode)
    int mode;
{
    int old = non_blocking;
    non_blocking = mode;
    return old;
}
