/* $Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/strings.c,v 1.7 1993-10-14 12:10:41 probe Exp $ */
/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/strings.c,v $ */
/* $Author: probe $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "quota.h"
#include "logger.h"
#include <stdio.h>
#include <krb.h>
#include "quota_limits.h"
#include <strings.h>
#ifdef ultrix
/* 'cause their broken */
#undef NULL
#endif
#include <sys/param.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/stat.h>
/* This is the Strings Database section of the code. */

extern int errno;

#define MAX_RETRY 5	/* Try 5 times to open strings db file */
static char str_dbname[MAXPATHLEN] = "";
static String_num numstrings=0;		/* Current num strings */
static String_num maxstrings=0;		/* Number available in cache */
static char **string[] = { NULL};
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


/* logger_set_name sets the name used for the string database. */

int logger_string_set_name(name)
char *name;
{
    char *str = "00000\n"; /* Reserved start string */
    struct stat sbuf;

    (void) strncpy(str_dbname, name, MAXPATHLEN);
    str_dbname[MAXPATHLEN - 1] = '\0';

/* Use stat!!! */
    if (!stat(str_dbname, &sbuf)) {
	return 0;
    }
    if (open_database(O_EXCL | O_WRONLY | O_CREAT)) {
	/* Cannot create the database. 
	   It can be a symlink or some other error */
	return -1;
    }
    
    if (write(dbfd, str, strlen(str)) != strlen(str)) {
	/* Failed write */
	(void) close_database();
	return -1;
    }
    
    return (close_database());
}
    
int
logger_read_strings()
{
    register int i=0;
    String_num num;
    char buf[BUFSIZ];

    if (open_database(O_RDONLY)) { 	
	/* Could not open the database - 
	   This is serious as the logger_string_set_name should have verified
	   existance, or created database 
	   */
	return -1;
    }

    if (connect_stream("r")) {
	/* No resources mabe??? */
	return -1;
    }

    if(fscanf(dbfno, "%5d\n", &num) < 1) {
	/* Either end of file or something */
	(void) close_database();
	return -1;
    }

    if (num == 0) goto done;


    /* Allocate space in our table... */
    if (set_maxnum_entries(num + 10)) {
	/* No mem or something */
	(void) close_database();
	return -1;
    }
	

    for(i=1; i <= num; i++) {
	if(fscanf(dbfno, "%s\n", buf) < 1) {
	    (void) close_database();
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
	(void) close_database();
	return -1;
    }

 done:
    if(close_database()) return(-1);

    numstrings = num;
    return num;
}

/* 0 = sucess, -1 = failure */
static int connect_stream(s)
char *s;
{
    if(!dbopen) return -1; /* Must be open already */
    if(!(dbfno = fdopen(dbfd, s))) return -1;
    return 0;
}

/* 0 = sucesss, -1 = failure */
static int close_database()
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

static int open_database(flags)
int flags;
{
    register int ret;
    register int retry_cnt=0;
    if (dbopen && (flags == dbflags)) return(0); /* Why do any work */
    if (dbopen && ((ret=close_database()) < 0)) return(ret);

    dbfd = -1;
    while (dbfd < 0 && retry_cnt < MAX_RETRY) 
	if ((dbfd = open(str_dbname, flags, STR_DB_MODE)) < 0) sleep(1);

    if(dbfd < 0 ) return(dbfd);
    
    dbflags = flags;
    dbopen = 1;
    return(0);
}

static int set_maxnum_entries(num)
String_num num;
{
    register char *new;

    if (num <= maxstrings) return(0); /* Can't go down in size */
    if (maxstrings == 0) {
	if(!(new = malloc((num + 1) * sizeof(char*)))) {
	    errno = ENOMEM;
	    return(-1);
	}
    }
    else {
	
	if(!(new = realloc((char *) *string, (num + 1) * sizeof(char *)))) {
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

char *logger_num_to_string(n)
String_num n;
{
    if (n == 0 || n > numstrings) return(NULL);
    return( (*string)[n]); /* XXX */
}

String_num
logger_string_to_num(str)
char *str;
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

String_num 
logger_add_string(str)
char *str;
{
    register String_num ret;
    char buf[BUFSIZ];

    /* Strategy:
       1) See if the string exists and return with value if it does
       2) Add string to file - if this fails then error condition should 
          be sent back. This is probably fatal...
       3) Add string to cache - if cannot, return fatal probably
    */

    /* If the string exists already, why do any work? */
    if (ret=logger_string_to_num(str)) return(ret);

    if( open_database(O_EXCL | O_RDWR) ) return(0); /* Errored in opening */
    
    /* First add the entry to the end of the file. If we can't do that, we 
       are screwed */
#ifdef POSIX
    if ( lseek(dbfd, (off_t) 0, SEEK_END) < 0) {
#else
    if ( lseek(dbfd, (off_t) 0, L_XTND) < 0) {
#endif
	(void) close_database();
	return(0); /* Error seeking to the end */
    }

    (void) strcpy(buf, str);
    (void) strcat(buf, "\n");
    PROTECT();
    if ( write(dbfd, buf, strlen(buf)) != strlen(buf)) {
	(void) close_database();
	UNPROTECT();
	return(0); /* Error writing string */
    }

    if ( lseek(dbfd, (off_t) 0, L_SET) < 0) {
	(void) close_database();
	UNPROTECT();
	return(0); /* Error going to beginning */
    }
    
    (void) sprintf(buf, "%5.5d\n", numstrings + 1);
    if( write(dbfd, buf, 5) != 5) {
	(void) close_database();
	UNPROTECT();
	return(0); /* Error in writing new number */
    }

    if (close_database()) {
	UNPROTECT();
	return(0); /* Error in the close */
    }

    UNPROTECT();
    /* The new entry is now in the strings database safely. Now to add it
       to our cache.
     */

    /* First, ensure we have space */
    if (numstrings + 1 >= maxstrings) 
	if( set_maxnum_entries(maxstrings + 10) ) /* Some error */
	    return (0);

    /* Now allocate the new space */   

    if ( !((*string)[++numstrings] = malloc((unsigned) strlen(str) + 1))) /* Enomem... */
	    {

		errno = ENOMEM;
		return 0;
	    }

    (void) strcpy((*string)[numstrings], str);
    return(numstrings); /* Not = 0, remember */
}

