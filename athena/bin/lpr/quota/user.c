/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/user.c,v 1.6 1993-05-10 13:41:50 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/user.c,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "logger.h"
#include <strings.h>
#include <sys/types.h>
#if defined(ultrix) && defined(NULL)
#undef NULL
#endif
#include <sys/param.h> 
#include <errno.h>
#include <sys/file.h>
#include <fcntl.h>

/* This is the User Database section of the code. */

extern int errno;
static DBM *db = NULL;
static char current_db_name[MAXPATHLEN] = "";
#define MAX_RETRY 5
static int dbopen = 0;
static int dbflags = 0;

int
logger_user_set_name(name)
char *name;
{
    DBM *db1;

    db1 = dbm_open(name, 0, 0);
    if (db1 == NULL) {
	db1 = dbm_open(name, (O_RDWR | O_CREAT), USER_DB_MODE);
	if(db1 == NULL) return -1;
    }
    dbm_close(db1);
    (void) close_database();
    (void) strncpy(current_db_name, name, MAXPATHLEN);
    current_db_name[MAXPATHLEN - 1] = '\0';
    return 0;

}

User_db *logger_find_user(user)
User_str *user;
{
    static User_db db_ret;
    datum contents, key;

    key.dptr = (char *) user;
    key.dsize = sizeof (User_str);
    if (open_database(O_RDONLY)) 
	return (User_db *) NULL;
    contents = dbm_fetch(db, key);
    if(contents.dptr == NULL) {
	/* No record indicated, see if should create */
	(void) close_database();
	return (User_db *) NULL;
    }

    /* These better be the same or else. Let's ensure the first few */
    /* for bozo on other side. */
    db_ret.user.name = user->name;
    db_ret.user.instance = user->instance;
    db_ret.user.realm = user->realm;
    /* Real info */

#ifndef UNALIGNED
    bcopy(((char *) &(((User_db *) contents.dptr)->first)), (char *)
	  &db_ret.first, sizeof(Pointer));
    bcopy(((char *) &(((User_db *) contents.dptr)->last)), (char *)
	  &db_ret.last, sizeof(Pointer));
#else
    db_ret.first = ((User_db *) contents.dptr)->first;
    db_ret.last = ((User_db *) contents.dptr)->last;
#endif

    return(&db_ret);
}

int logger_set_user(user, ent)
User_str *user;
User_db *ent;
{
    datum key, contents;

    key.dptr = (char *) user;
    key.dsize = sizeof (User_str);
    contents.dptr = (char *) ent;
    contents.dsize = sizeof (User_db);
    if(open_database(O_RDWR)) return -1;
    PROTECT();
    if (dbm_store(db, key, contents, DBM_REPLACE)) {
	(void) close_database();
	UNPROTECT();
	return -1;
    }
    UNPROTECT();
    return(close_database());
}

logger_user_iterate(func, arg)
int (*func)();
char *arg;
{
    datum key, contents;
    User_db *u_db;
    int code;

    if(open_database(O_RDONLY)) return -1;
    for (key = dbm_firstkey (db); key.dptr != NULL; key = dbm_next(db, key)) {
	contents = dbm_fetch (db, key);
	u_db = (User_db *) contents.dptr;
	if ((code = (*func)(arg, u_db)) != 0)
            return code;
    }
    return(close_database());
}

static int close_database()
{
    if(dbopen) (void) dbm_close(db);
    dbopen = 0;
    db = NULL;
    return 0;
}

static int open_database(flags)
int flags;
{
    register int ret;
    register int retry_cnt=0;

    if (dbopen && (flags == dbflags)) return(0); /* Why do any work */
    if (dbopen && ((ret=close_database()) < 0)) return(ret);

    while (!db && retry_cnt < MAX_RETRY) 
	if ((db = dbm_open(current_db_name, flags, STR_DB_MODE)) == NULL) sleep(1);

    if(db == NULL ) return(-1);
    
    dbflags = flags;
    dbopen = 1;
    return(0);
}

