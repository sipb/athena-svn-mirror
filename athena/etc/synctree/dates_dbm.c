/* Copyright (C) 1988  Tim Shepard   All rights reserved. */

#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <ndbm.h>
#ifdef sparc
#include <alloca.h>
#endif
time_t get_date(dbmpp,fname)
     DBM **dbmpp;
     char *fname;
{
  extern char *date_db_name;
  datum key;
  datum datum_date_db;

  if (*dbmpp == NULL)
    { char *p = (char *) alloca(strlen(fname) + strlen(date_db_name) + 1);
      char *pp;
      (void) strcpy(p,fname);
      pp = rindex(p,'/');
      pp++;
      (void) strcpy(pp,date_db_name);

      *dbmpp = dbm_open(p, O_RDWR|O_CREAT, 00644);
      if (!*dbmpp) {
	      perror(p);
	      exit(1);
      }
    }
  if (!*dbmpp) abort();
  key.dptr = rindex(fname,'/');
  key.dptr++;
  key.dsize = strlen(key.dptr);
  
  datum_date_db = dbm_fetch(*dbmpp,key);
  if (datum_date_db.dptr == NULL) return (time_t) 0;  /* a 0 time_t */
  else if (datum_date_db.dsize != sizeof(time_t))
    { fprintf(stderr, "get_date: dsize != sizeof(time_t) in datum returned by dbm_fetch()");
      return (time_t) 0;
    }
  else {
	  time_t temp;
	  bcopy (datum_date_db.dptr, (char *)&temp, sizeof(temp));
	  return temp;
  }
}
	
set_date(dbmpp,fname,date)
     DBM **dbmpp;
     char *fname;
     time_t date;
{
  extern char *date_db_name;
  datum key;
  datum date_db_datum;
  if (*dbmpp == NULL)
    { char *p = (char *) alloca(strlen(fname) + strlen(date_db_name) + 1);
      char *pp;
      (void) strcpy(p,fname);
      pp = rindex(p,'/');
      pp++;
      (void) strcpy(pp,date_db_name);
      *dbmpp = dbm_open(p, O_RDWR|O_CREAT, 00644);
      if (!dbmpp) abort();
    }

  key.dptr = rindex(fname,'/');
  key.dptr++;
  key.dsize = strlen(key.dptr);
  date_db_datum.dptr = (char *) &date;
  date_db_datum.dsize = sizeof(date);
  (void) dbm_store(*dbmpp, key, date_db_datum, DBM_REPLACE);
}
	    






