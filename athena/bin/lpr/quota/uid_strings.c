/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/uid_strings.c,v 1.5 1993-05-10 13:42:50 vrt Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/uid_strings.c,v $ */
/* $Author: vrt $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "config.h"
#include <stdio.h>
#include <krb.h>
#include "quota_limits.h"
#include <strings.h>
#include <sys/param.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <gquota_db.h>

/* This is the Uid Database section of the code. */

#define UID_DB_MODE     0755    /* Mode for opening the uid database */

extern int errno;

#define MAX_RETRY 5	/* Try 5 times to open strings db file */
static char uid_dbname[MAXPATHLEN] = "";
static Uid_num numstrings=0;		/* Current num strings */
static Uid_num maxstrings=0;		/* Number available in cache */
static char **string[] = { NULL };
static int dbflags = 0;
static int dbopen = 0;
static int dbfd = 0;
static FILE *dbfno = NULL;

char *malloc(), *realloc();
off_t lseek();

/* Internal description of the database.  Strings will be allocated
   and pointers stored in one table. As it is inefficient to add strings
   one at a time in terms of realloc, we cheat and have a concept of the
   maximum number that exist in the database. We can realloc space for
   two more than we currently have and save on time in future additions.

   We take advantage of this fact and on string database reads, cheat and 
   allocate space for all the strings before read. Previous tests with another
   application have shown this to be a "win"
*/


/* uid_set_name sets the name used for the string database. */
int uid_string_set_name(name)
char *name;
{
    char *str = "00000\n"; /* Reserved start string */
    struct stat sbuf;

    (void) strncpy(uid_dbname, name, MAXPATHLEN);
    uid_dbname[MAXPATHLEN - 1] = '\0';

    if (!stat(uid_dbname, &sbuf)) {
	return 0;
    }
    if (uid_open_database(O_EXCL | O_WRONLY | O_CREAT)) {
	/* Cannot create the database. 
	   It can be a symlink or some other error */
	return -1;
    }    
    if (write(dbfd, str, strlen(str)) != strlen(str)) {
	/* Failed write */
	(void) uid_close_database();
	return -1;
    }
    return (uid_close_database());
}
    
int
uid_read_strings()
{
    register int i=0;
    Uid_num num;
    char buf[BUFSIZ];

    if (uid_open_database(O_RDONLY)) { 	
	/* Could not open the database - 
	   This is serious as the uid_string_set_name should have verified
	   existance, or created database 
	   */
	return -1;
    }

    if (uid_connect_stream("r")) {
	/* No resources mabe??? */
	return -1;
    }

    if(fscanf(dbfno, "%5d\n", &num) < 1) {
	/* Either end of file or something */
	(void) uid_close_database();
	return -1;
    }

    if (num == 0) goto done;

    /* Allocate space in our table... */
    if (uid_set_maxnum_entries(num + 10)) {
	/* No mem or something */
	(void) uid_close_database();
	return -1;
    }

    for(i = 1; i <= num; i++) {
	if(fscanf(dbfno, "%s\n", buf) < 1) {
	    (void) uid_close_database();
	    return -1;
	}
	if ( !((*string)[i] = malloc((unsigned) strlen(buf) + 1))) /* Enomem... */
	    {
		errno = ENOMEM;
		return 0;
	    }

	(void) strcpy((*string)[i], buf);
    }

    /* Make sure we are at the end of the file */
    if(fscanf(dbfno, "%c", buf) != EOF) {
	(void) uid_close_database();
	return -1;
    }

 done:
    if(uid_close_database()) return(-1);

    numstrings = num;
    return num;
}

/* 0 = sucess, -1 = failure */
static int uid_connect_stream(s)
char *s;
{
    if(!dbopen) return -1; /* Must be open already */
    if(!(dbfno = fdopen(dbfd, s))) return -1;
    return 0;
}

/* 0 = sucesss, -1 = failure */
static int uid_close_database()
{
    register int ret=0;
    if (dbopen && !dbfno) {
	ret = close(dbfd);
	dbopen = 0;
    }
    if (dbopen) {
	ret = fclose(dbfno);
	dbopen = 0;
	dbfno = 0;
    }
	    
    return ret;
}

/* 0 = sucess, !0 => error */       
static int uid_open_database(flags)
int flags;
{
    register int ret;
    register int retry_cnt=0;
    if (dbopen && (flags == dbflags)) return(0); /* Why do any work */
    if (dbopen && ((ret=uid_close_database()) < 0)) return(ret);

    dbfd = -1;
    while (dbfd < 0 && retry_cnt < MAX_RETRY) 
	if ((dbfd = open(uid_dbname, flags, UID_DB_MODE)) < 0) sleep(1);

    if(dbfd < 0 ) return(dbfd);
    
    dbflags = flags;
    dbopen = 1;
    return(0);
}

static int uid_set_maxnum_entries(num)
Uid_num num;
{
    register char *new;

    if (num <= maxstrings) return(0); /* Can't go down in size */
    if (maxstrings == 0) {
	if(!(new = malloc((unsigned) (num + 1) * sizeof(char*)))) {
	    errno = ENOMEM;
	    return(-1);
	}
    }
    else {
	
	if(!(new = realloc((char *) *string, (unsigned) (num + 1) * sizeof(char *)))) {
	    /* We are in big trouble. We are out of memory. We have
	       possibly corrupted the string table - we better abort, but
	       we leave it to upper layers to decide */
	    errno = ENOMEM;
	    return(-1);
	}
    }

    *string = (char **) new;
    maxstrings = num;
    return 0;
}

#ifdef __STDC__
char *uid_num_to_string(Uid_num n)
#else
char *uid_num_to_string(n)
Uid_num n;
#endif
{
    int i;
    i = (int) n;
    if (i <=0 || i > numstrings) return(NULL);
    return( (*string)[i]); /* XXX */
}

#ifdef __STDC__
Uid_num uid_string_to_num(char *str)
#else
Uid_num uid_string_to_num(str)
char *str;
#endif
{
    /* As we are currently storing the strings in a pointer array, a linear 
     *  search is as efficient as anything else for now
     */
    register int i=1;
    if (numstrings == 0 ) return 0;
    for (; i <= numstrings ; i++ )
	if(strcmp((*string)[i], str) == 0) return i;
    return 0;
}

Uid_num 
uid_add_string(str)
char *str;
{
    register Uid_num ret;
    char buf[BUFSIZ];

    /* Strategy:
       1) See if the string exists and return with value if it does
       2) Add string to file - if this fails then error condition should 
          be sent back. This is probably fatal...
       3) Add string to cache - if cannot, return fatal probably
    */

    /* If the string exists already, why do any work? */
    if (ret=uid_string_to_num(str)) return(ret);

    if( uid_open_database(O_EXCL | O_RDWR) ) return(0); /* Errored in opening */
    
    /* First add the entry to the end of the file. If we can't do that, we 
       are screwed */
#ifdef POSIX
    if ( lseek(dbfd, (off_t) 0, SEEK_END) < 0) {
#else
    if ( lseek(dbfd, (off_t) 0, L_XTND) < 0) {
#endif
	(void) uid_close_database();
	return(0); /* Error seeking to the end */
    }

    (void) strcpy(buf, str);
    (void) strcat(buf, "\n");
    PROTECT();
    if ( write(dbfd, buf, strlen(buf)) != strlen(buf)) {
	(void) uid_close_database();
	UNPROTECT();
	return(0); /* Error writing string */
    }

    if ( lseek(dbfd, (off_t) 0, L_SET) < 0) {
	(void) uid_close_database();
	UNPROTECT();
	return(0); /* Error going to beginning */
    }
    
    (void) sprintf(buf, "%5.5d\n", numstrings + 1);
    if( write(dbfd, buf, 5) != 5) {
	(void) uid_close_database();
	UNPROTECT();
	return(0); /* Error in writing new number */
    }

    if (uid_close_database()) {
	UNPROTECT();
	return(0); /* Error in the close */
    }

    UNPROTECT();
    /* The new entry is now in the strings database safely. Now to add it
       to our cache.
     */

    /* First, ensure we have space */
    if (numstrings + 1 >= maxstrings) 
	if( uid_set_maxnum_entries(maxstrings + 10) ) /* Some error */
	    return (0);

    /* Now allocate the new space */   

    if ( !((*string)[++numstrings] = malloc((unsigned) strlen(str) + 1))) /* Enomem... */
	    {

		errno = ENOMEM;
		return 0;
	    }

    (void) strcpy((*string)[numstrings], str);
    return((Uid_num) numstrings); /* Not = 0, remember */
}

