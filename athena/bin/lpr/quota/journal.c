/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/journal.c,v 1.7 1993-05-10 13:42:16 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/journal.c,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "logger.h"
#include <strings.h>
#if defined(ultrix) && defined(NULL)
#undef NULL
#endif
#include <sys/stat.h>
#include <sys/param.h> 
#include <errno.h>
#include <sys/file.h>
#include <fcntl.h>
#include <syslog.h>

extern int errno;

#define MAX_RETRY 5	/* Try 5 times to open strings db file */
static char str_dbname[MAXPATHLEN] = "";
static int dbflags = 0;
static int dbopen = 0;
static int dbfd = 0;
static int syncdb = 1;	/* sync after db writes */

char *malloc(), *realloc();
off_t lseek();

#define jpos(n) (off_t) ((n) * sizeof(log_entity))
int logger_journal_set_name(name)
char *name;
{
    struct stat sbuf;
    log_header newent;

    (void) strncpy(str_dbname, name, MAXPATHLEN);
    str_dbname[MAXPATHLEN - 1] = '\0';

    if (!stat(str_dbname, &sbuf)) {
	return 0;
    }

    if (open_database(O_EXCL | O_WRONLY | O_CREAT)) {
	/* Cannot create the database. 
	   It can be a symlink or some other error */
	return -1;
    }
    
/* Create the first entry in the database */
    newent.version = LOGGER_VERSION;
    newent.num_ent = 0;			/* Start w/ 0 entries */
    newent.last_q_time = 0;
    newent.quota_pos = 0;

    if (write(dbfd, (char *) &newent, sizeof(log_header)) 
	!= sizeof(log_header)) {
	/* Failed write */
	(void) close_database();
	return -1;
    }
    
    if(syncdb && fsync(dbfd)) {
	(void) close_database();
	return -1;
    }

    return (close_database());
}
    

/* 0 = sucesss, -1 = failure */
static int close_database()
{
    register int ret=0;
    if (dbopen) {
	ret = close(dbfd);
	dbopen = 0;
    }
	    
    return ret;
}

/* 0 = sucess, !0 => error */       

static int open_database(flags)
int flags;
{
    register int ret;
    register int retry_cnt=0;
    if (dbopen && (flags == dbflags)) return(0); /* Why do any work */
    if (dbopen && ((ret=close_database()) < 0)) return(ret);

    dbfd = -1;
    while (dbfd < 0 && retry_cnt < MAX_RETRY) 
	if ((dbfd = open(str_dbname, flags, JOUR_DB_MODE)) < 0) sleep(1);

    if(dbfd < 0 ) return(dbfd);
    
    dbflags = flags;
    dbopen = 1;
    return(0);
}


log_entity *logger_journal_get_line(n)
Pointer n;
{
    static log_entity ret;
    if(open_database(O_RDONLY)) return (log_entity *) NULL;
    /* This is for speed performance that we don't bother to check the 
       num_ent field as it means we must read in the record. We could in 
       theory cache the header record, but I don;t like it.
     */
    PROTECT();
    if(lseek(dbfd, jpos(n), L_SET) != jpos(n))
	    {
		(void) close_database();
		UNPROTECT();
		syslog(LOG_ERR, "get line %d", n);
		return (log_entity *) NULL;
	    }

    if(read(dbfd, (char *) &ret, sizeof(log_entity)) != sizeof(log_entity)) 
	    {
		(void) close_database();
		UNPROTECT();
		syslog(LOG_ERR, "get line read %d %d %d %d", n, jpos(n), errno,dbfd);
		return (log_entity *) NULL;
	    }
    UNPROTECT();
    if(close_database()) return (log_entity *) NULL;
    return &ret;
}

logger_journal_write_line(n, ent)
Pointer n;
log_entity *ent;
{
    int ret;

    if(n < 1) {
	errno = EIO;
	return -1;
    }
    if(open_database(O_WRONLY)) return -1;
    PROTECT();
    if(lseek(dbfd, jpos(n), L_SET) != jpos(n))
	    {
		(void) close_database();
		UNPROTECT();
		return -1;
	    }

    if(write(dbfd, (char *) ent, sizeof(log_entity)) != sizeof(log_entity)) 
	    {
		(void) close_database();
		UNPROTECT();
		syslog(LOG_ERR, "writing line %d", n);
		return -1;
	    }
    if(syncdb && fsync(dbfd))
	    {
		(void) close_database();
		UNPROTECT();
		syslog(LOG_ERR, "syncing line %d", n);
		return -1;
	    }
    ret=close_database();
    UNPROTECT();
    return ret;
}

int logger_journal_get_header(head)
log_header *head;
{
    if(open_database(O_RDONLY)) return -1;
    PROTECT();
    lseek(dbfd, (off_t) 0, L_SET);
    if(read(dbfd, (char *) head, sizeof(log_header)) != sizeof(log_header)) 
	    {
		(void) close_database();
		UNPROTECT();
		return -1;
	    }
    if(head->version != LOGGER_VERSION) 
	    {
		(void) close_database();
		UNPROTECT();
		syslog(LOG_ERR, "logger - get_header - vno not match %d", head->version);
		return -1;
	    }
    UNPROTECT();
    return(close_database());
}

int logger_journal_put_header(head)
log_header *head;
{
    int ret;
    if(head->version != LOGGER_VERSION) {
		syslog(LOG_ERR, "logger - put_header - vno not match %d", head->version);
		return -1;
	    }
    if(open_database(O_WRONLY)) return -1;
    PROTECT();
    lseek(dbfd, (off_t) 0, L_SET);
    if(write(dbfd, (char *) head, sizeof(log_header)) != sizeof(log_header)) 
	    {
		(void) close_database();
		UNPROTECT();
		return -1;
	    }
    if(syncdb && fsync(dbfd)) {
	(void) close_database();
	UNPROTECT();
	syslog(LOG_ERR, "sync after writing header failed");
	return -1;
    }
    ret=close_database();
    UNPROTECT();
    return(ret);
}

int
logger_journal_add_entry(ent, qt, qpos)
log_entity *ent;
Time qt;
Pointer qpos;
{
    /* Strategy:
       1) Lookup user structure, if doesn't exist create an entry for user
       2) Get the header
       3) Modify prev ref. in journal datbase
       4) Add new line to database, with prev ref set from user structure.
       5) Modify header with updated info and write out
       6) Modify user database pointers


       Failure modes based on steps above
       1) Continue
       2) Return -1
       3) Return -1 (Nothing really modified - we hope...)
       4) Back out step 3, Return -1.
       5) Back out step 3, return -1 (The data in 4 will not be referenced)
       6) back out 3 & 5. Return -1.
       
       Problem is what to do if backing out fails...
         
     */

    log_header oldhead, newhead;
    User_str user;
    User_db *uret, udb;
    Pointer num, oldlast;		/* journal table position*/
    log_entity *old_ent, new_ent;

    PROTECT();
    /* Step 1 - Get entry from user_db */

    user.name = ent->user.name;
    user.instance = ent->user.instance;
    user.realm = ent->user.realm;

    if((uret = logger_find_user(&user)) == (User_db *) NULL) {
	/* Failure 1 */
	uret = &udb;
	udb.user.name = user.name;
	udb.user.instance = user.instance;
	udb.user.realm = user.realm;
	udb.first = 0;
	udb.last = 0;
    }

    udb.user.name = uret->user.name;
    udb.user.instance = uret->user.instance;
    udb.user.realm = uret->user.realm;
    udb.first = uret->first;
    udb.last = uret->last;

    /* Step 2 - get the header */
    if (logger_journal_get_header(&oldhead)) {
	UNPROTECT();
	syslog(LOG_ERR, "Add entry - failure 2 %d", errno);
	return -1; /* Failure 2 */
    }

    (void) bcopy((char *) &oldhead, (char *) &newhead, sizeof(log_header));
    if(qt) newhead.last_q_time = qt;
    if (qpos) newhead.quota_pos = qpos;
    newhead.num_ent++;
    num = newhead.num_ent;	/* num contains new entry position */

    /* Step 3 - modify previous record */
    /* If user has old record then get it and modify */
    if (udb.last != 0) {
	if((old_ent = logger_journal_get_line(udb.last)) == NULL) {
	    UNPROTECT();
	    syslog(LOG_ERR, "Add entry - failure 3 %d", errno);
	    return -1; /* Mode 3 failure */
	}
	(void) bcopy((char *) old_ent, (char *) &new_ent, sizeof(log_entity));
	new_ent.next = num;
	if(logger_journal_write_line(udb.last, &new_ent)) {
	    UNPROTECT();
	    syslog(LOG_ERR, "Add entry - failure 3b %d", errno);
	    return -1; /* Failure */
	}
    }

    /* Step 4 */
    ent->prev = udb.last;
    ent->next = 0;
    if(logger_journal_write_line(num, ent)) {
	/* Failure mode 4 - back out 3 above */
	new_ent.next = 0;
	if(udb.last) (void) logger_journal_write_line(udb.last, &new_ent);
	UNPROTECT();
	syslog(LOG_ERR, "Add entry - failure 4 %d", errno);
	return -1;
    }

    /* Step 5 */
    newhead.last_q_time = qt;
    newhead.quota_pos = qpos;
    if(logger_journal_put_header(&newhead)) {
	/* Failure mode 5 - back out 3 above */
	new_ent.next = 0;
	if(udb.last) (void) logger_journal_write_line(udb.last, &new_ent);
	UNPROTECT();
	syslog(LOG_ERR, "Add entry - failure 5 %d", errno);
	return -1;
    }
	
    /* Step 6 */
	    
    if(udb.first == 0) udb.first = num;
    oldlast = udb.last;
    udb.last = num;
    if(logger_set_user(&user, &udb)) {
	/* Failure mode 6 - back out 3 and 5 */
	new_ent.next = 0;
	if(oldlast) (void) logger_journal_write_line(oldlast, &new_ent);
	(void) logger_journal_put_header(&oldhead);
	UNPROTECT();
	syslog(LOG_ERR, "Add entry - failure 6 %d", errno);
	return -1;
    }
       
    /* We made it ok */
    UNPROTECT();
    return 0;
}

int logger_journal_set_sync(new)
int new;
    {
	int old = syncdb;
	syncdb = new;
	return old;
    }
