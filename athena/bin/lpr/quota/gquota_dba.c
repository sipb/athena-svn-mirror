/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gquota_dba.c,v 1.4 1993-05-10 13:42:06 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gquota_dba.c,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#ifndef lint
static char rcs_id[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gquota_dba.c,v 1.4 1993-05-10 13:42:06 vrt Exp $";
#endif lint

#include "mit-copyright.h"
#include "quota.h"
#include "gquota_db.h"
#include "quota_limits.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
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

#if defined(ultrix) && defined(NULL)
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
static char default_gdb_name[] = DBM_GDEF_FILE;
static char *current_gdb_name = default_gdb_name;
static int non_blocking = 0;
static struct timeval timestamp;/* current time of request */
static void gquota_dbl_fini(), gquota_dbl_unlock();

#ifdef DEBUG
extern int debug;
extern long gquota_debug;
extern char *progname;
#endif

#define GQUOTA_DB_MAX_RETRY 5

/* In order to save time, much of this code has been stolen from 
   the kerberos admin server and modified accordingly.
*/


static void
encode_group_key(key, account, service)
    datum  *key;
    long   account;
    char   *service;
{
    static char keystring[4 + SERV_SZ];   /* Account and service */

    bzero(keystring, 4 + SERV_SZ);
    bcopy((char *) &account, keystring, 4);
    (void) strncpy(&keystring[4], service, SERV_SZ);
    key->dsize = 4 + SERV_SZ;
    key->dptr = keystring;
}

static void
decode_group_key(key, account, service)
    datum  *key;
    long   *account;
    char   *service;
{
    bcopy(key->dptr, (char *) account, 4);
    (void) strncpy(service, key->dptr + 4, SERV_SZ);
    
    service[SERV_SZ -1] = '\0';
}

static void
encode_group_contents(contents, qrec)
    datum  *contents;
    gquota_rec *qrec;
{
    contents->dsize = sizeof(*qrec);
    contents->dptr = (char *) qrec;
}

static void
decode_group_contents(contents, qrec)
    datum  *contents;
    gquota_rec *qrec;
{
    bcopy(contents->dptr, (char *) qrec, sizeof(*qrec));
}

/*
 * Utility routine: generate name of database file.
 */

static char *gen_gdbsuffix(db_name, sfx)
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

gquota_db_init()
{
    init = 1;
    return (0);
}

/*
 * gracefully shut down database--must be called by ANY program that does
 * a gquota_db_init 
 */

gquota_db_fini()
{
}

/*
 * Set the "name" of the current database to some alternate value.
 *
 * Passing a null pointer as "name" will set back to the default.
 * If the alternate database doesn't exist, nothing is changed.
 */

gquota_db_set_name(name)
	char *name;
{
    DBM *db;

    if (name == NULL)
	name = default_gdb_name;
    db = dbm_open(name, 0, 0);
    if (db == NULL)
	return errno;
    dbm_close(db);
    gquota_dbl_fini();
    current_gdb_name = name;
    return 0;
}

/*
 * Return the last modification time of the database.
 */

long gquota_get_db_age()
{
    struct stat st;
    char *okname;
    long age;
    
    okname = gen_gdbsuffix(current_gdb_name, ".ok");

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

long gquota_start_update(db_name)
    char *db_name;
{
    char *okname = gen_gdbsuffix(db_name, ".ok");
    long age = gquota_get_db_age();
    
    if (unlink(okname) < 0
	&& errno != ENOENT) {
	    age = -1;
    }
    free (okname);
    return age;
}

long gquota_end_update(db_name, age)
    char *db_name;
    long age;
{
    int fd;
    int retval = 0;
    char *new_okname = gen_gdbsuffix(db_name, ".ok#");
    char *okname = gen_gdbsuffix(db_name, ".ok");
    
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
		syslog(LOG_INFO, "gquota_dba: utimes() failed");
	    if(fsync(fd))
		syslog(LOG_INFO, "gquota_dba: fsync() failed");
	}
	(void) close(fd);
	if (rename (new_okname, okname) < 0)
	    retval = errno;
    }

    free (new_okname);
    free (okname);

    return retval;
}

static long gquota_start_read()
{
    return gquota_get_db_age();
}

static long gquota_end_read(age)
    u_long age;
{
    if (gquota_get_db_age() != age || age == -1) {
	return -1;
    }
    return 0;
}

/*
 * Create the database, assuming it's not there.
 */

gquota_db_create(db_name)
    char *db_name;
{
    char *okname = gen_gdbsuffix(db_name, ".ok");
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
    char *dirname = gen_gdbsuffix(db_name, ".dir");
    char *pagname = gen_gdbsuffix(db_name, ".pag");

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
	close(fd);
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

gquota_db_rename(from, to)
    char *from;
    char *to;
{
    char *fromdir = gen_gdbsuffix (from, ".dir");
    char *todir = gen_gdbsuffix (to, ".dir");
    char *frompag = gen_gdbsuffix (from , ".pag");
    char *topag = gen_gdbsuffix (to, ".pag");
    char *fromok = gen_gdbsuffix(from, ".ok");
    long trans = gquota_start_update(to);
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
	return gquota_end_update(to, trans);
    else
	return -1;
}

/*
 * look up a group in the data base returns number of groups
 * found , and whether there were more than requested. 
 */
gquota_db_get_group(account, serv, qrec, max, more)
    long   account;
    char   *serv;		/* could have wild card */     
    gquota_rec *qrec;
    unsigned int max;		/* max number of name structs to return */
    int    *more;		/* where there more than 'max' tuples? */

{
    int     found = 0;
    extern int errorproc();
    int     wilds;
    datum   key, contents;
    char    testserv[INST_SZ];
    u_long trans;
    int try;
    DBM    *db;

    if (!init)
	(void) gquota_db_init();	/* initialize database routines */

    for (try = 0; try < GQUOTA_DB_MAX_RETRY; try++) {
	trans = gquota_start_read();

	if (gquota_dbl_lock(GQUOTA_DBL_SHARED) != 0) {
	    gquota_dbl_fini(); 	/* Attempt to fix the problem of old locks */
	    return -1;
	}

	db = dbm_open(current_gdb_name, O_RDONLY, 0600);

	*more = 0;

#ifdef DEBUG
	if (gquota_debug & 2)
	    fprintf(stderr,
		    "%s: quota_gdb_get_group for %l %s max = %d",
		    account, serv, max);
#endif

	wilds = !strcmp(serv, "*");

	if (!wilds) {	/* nothing's wild */
	    encode_group_key(&key, account, serv);
	    contents = dbm_fetch(db, key);
	    if (contents.dptr == NULL) {
		found = 0;
		goto done;
	    }
	    decode_group_contents(&contents, qrec);
#ifdef DEBUG
	    if (gquota_debug & 1) {
		fprintf(stderr, "\t found %l %s s_n length %d \n",
			qrec->account,
			qrec->service,
			strlen(qrec->service));
	    }
#endif
	    found = 1;
	    goto done;
	}	/* process wild cards by looping through entire database */

	for (key = dbm_firstkey(db); key.dptr != NULL;
	     key = dbm_next(db, key)) {
	    decode_group_key(&key, &account, testserv);
	    if (wilds || !strcmp(testserv, serv)) { /* have a match */
		if (found >= max) {
		    *more = 1;
		    goto done;
		} else {
		    found++;
		    contents = dbm_fetch(db, key);
		    decode_group_contents(&contents, qrec);
#ifdef DEBUG
		    if (gquota_debug & 1) {
			fprintf(stderr, "\t found %l %s s_n length %d \n",
				qrec->account,
				qrec->service,
				strlen(qrec->service));
		    }
#endif
		    qrec++; /* point to next */
		}
	    }
	}

    done:
	gquota_dbl_unlock();	/* unlock read lock */
	dbm_close(db);
	if (gquota_end_read(trans) == 0)
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

gquota_db_put_group(qrec, max)
    gquota_rec *qrec;
    unsigned int max;		/* number of group structs to
				 * update */

{
    int     found = 0;
    u_long  i;
    extern int errorproc();
    datum   key, contents;
    DBM    *db;

    (void) gettimeofday(&timestamp, (struct timezone *) NULL);

    if (!init)
	(void) gquota_db_init();

    if (gquota_dbl_lock(GQUOTA_DBL_EXCLUSIVE) != 0)
	return -1;

    db = dbm_open(current_gdb_name, O_RDWR, 0600);

#ifdef DEBUG
    if (gquota_debug & 2)
	fprintf(stderr, "%s: gquota_db_put_group  max = %d",
	    progname, max);
#endif
    PROTECT();
    /* for each one, stuff temps, and do replace/append */
    for (i = 0; i < max; i++) {
	encode_group_contents(&contents, qrec);
	encode_group_key(&key, qrec->account, qrec->service);
	if (dbm_store(db, key, contents, DBM_REPLACE) < 0) 
	    syslog(LOG_ERR, "gquota_dba_put_principal: dbm_store failed");
#ifdef DEBUG
	if (gquota_debug & 1) {
	    fprintf(stderr, "\n put %l %s\n",
		qrec->account, qrec->service);
	}
#endif
	found++;
	qrec++;		/* bump to next struct			   */
    }

    dbm_close(db);
    gquota_dbl_unlock();		/* unlock database */
    UNPROTECT();
    return (found);
}


gquota_db_iterate (func, arg)
    int (*func)();
    char *arg;			/* void *, really */
{
    datum key, contents;
    gquota_rec *qrec;
    int code;
    DBM *db;
    
    (void) gquota_db_init();	/* initialize and open the database */
    if ((code = gquota_dbl_lock(GQUOTA_DBL_SHARED)) != 0)
	return code;

    db = dbm_open(current_gdb_name, O_RDONLY, 0600);

    for (key = dbm_firstkey (db); key.dptr != NULL; key = dbm_next(db, key)) {
	contents = dbm_fetch (db, key);
	/* XXX may not be properly aligned */
	qrec = (gquota_rec *) contents.dptr;
	if ((code = (*func)(arg, qrec)) != 0)
	    return code;
    }
    dbm_close(db);
    gquota_dbl_unlock();
    return 0;
}

static int dblfd = -1;
static int mylock = 0;
static int inited = 0;

static gquota_dbl_init()
{
    if (!inited) {
	char *filename = gen_gdbsuffix (current_gdb_name, ".ok");
	if ((dblfd = open(filename, 0)) < 0) {
	    fprintf(stderr, "gquota_dbl_init: couldn't open %s\n", filename);
	    (void) fflush(stderr);
	    perror("open");
	    exit(1);
	}
	free(filename);
	inited++;
    }
    return (0);
}

static void gquota_dbl_fini()
{
    if (dblfd != -1) (void) close(dblfd);
    dblfd = -1;
    inited = 0;
    mylock = 0;
}

static int gquota_dbl_lock(mode)
    int     mode;
{
    int flock_mode=0;
    
    if (!inited)
	(void) gquota_dbl_init();
    if (mylock) {		/* Detect lock call when lock already
				 * locked */
	fprintf(stderr, "gquota locking error (mylock)\n");
	(void) fflush(stderr);
	exit(1);
    }
    switch (mode) {
    case GQUOTA_DBL_EXCLUSIVE:
	flock_mode = LOCK_EX;
	break;
    case GQUOTA_DBL_SHARED:
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

static void gquota_dbl_unlock()
{
    if (!mylock) {		/* lock already unlocked */
	fprintf(stderr, "gquota database lock not locked when unlocking.\n");
	(void) fflush(stderr);
	exit(1);
    }
    if (flock(dblfd, LOCK_UN) < 0) {
	fprintf(stderr, "gquota database lock error. (unlocking)\n");
	fflush(stderr);
	perror("flock");
	exit(1);
    }
    mylock = 0;
}

int gquota_db_set_lockmode(mode)
    int mode;
{
    int old = non_blocking;
    non_blocking = mode;
    return old;
}


/**** Routines to manipulate/check the admin and user lists ****/
/**** Uses linear search for now, might need to change this ****/ /* XXXX */


int is_group_admin(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    return((gquota_db_find_admin(quid, qrec) < 0) ? 0 : 1);
}


int is_group_user(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    return((gquota_db_find_user(quid, qrec) < 0) ? 0 : 1);
}


int gquota_db_find_user(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    int no_user, i;

    no_user = (int) qrec->user[0];
    if (no_user == 0)
	return -1;

    /* Check user list to see if uid exists */
    for (i = 1 ; i <= no_user ; i++) {
	if (quid == qrec->user[i])
	    return i;
    }
    return -1;
}


int gquota_db_find_admin(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    int no_admin, i;

    no_admin = (int) qrec->admin[0];
    if (no_admin == 0)
	return -1;

    /* Check admin list to see if uid exists */
    for (i = 1 ; i <= no_admin ; i++) {
	if (quid == qrec->admin[i])
	    return i;
    }
    return -1;
}


gquota_db_insert_admin(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    int no_admin;

    no_admin = (int) qrec->admin[0];
    if (no_admin >= GQUOTA_MAX_ADMIN) return -1;

    no_admin++;
    qrec->admin[no_admin] = quid;
    qrec->admin[0] = (long) no_admin;
    return 0;
}


gquota_db_remove_admin(quid, qrec, ptr)
    long quid;
    gquota_rec *qrec;
    int ptr;
{
    int no_admin;

    if (ptr < 0) {
	if ((ptr = gquota_db_find_admin(quid, qrec)) < 0)
	    return -1;
    }

    no_admin = (int) qrec->admin[0];
    if (!no_admin || (ptr < 1)) return -1;

    if (no_admin > 1)
	qrec->admin[ptr] = qrec->admin[no_admin];

    no_admin--;
    qrec->admin[0] = (long) no_admin;
    return 0;
}


gquota_db_insert_user(quid, qrec)
    long quid;
    gquota_rec *qrec;
{
    int no_user;

    no_user = (int) qrec->user[0];
    if (no_user >= GQUOTA_MAX_USER) return -1;

    no_user++;
    qrec->user[no_user] = quid;
    qrec->user[0] = (long) no_user;
    return 0;
}


gquota_db_remove_user(quid, qrec, ptr)
    long quid;
    gquota_rec *qrec;
    int ptr;
{
    int no_user;

    if (ptr < 0) {
	if ((ptr = gquota_db_find_user(quid, qrec)) < 0)
	    return -1;
    }
    
    no_user = (int) qrec->user[0];
    if (!no_user || (ptr < 1)) return -1;

    if (no_user > 1)
	qrec->user[ptr] = qrec->user[no_user];

    no_user--;
    qrec->user[0] = (long) no_user;
    return 0;
}
