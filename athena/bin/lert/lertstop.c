/*
   file: lertstop.c
   basic use: lertstop type
     remove a single letter category from db

   ndbm apparently fails in traversing the data if you delete or
change keys while traversing.
   options--restart traversing or build a linked list of changes...

   q&d program...restart traverse!

 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ndbm.h>
#include <sys/file.h>

#include "lert.h"     

main(argc, argv)
int argc;
char ** argv;
{
  char buffer[512];
  DBM *db;
  datum key;
  datum new;
  datum data;
  register char *cp;
  register char *nd_p;
  int nd_c;
  int count = 0;
  int changed;
  int new_char;
  char * name_p;
  int name_c;
  char categ[128];

  if (argc != 2) {
     fprintf(stderr, "usage: %s type\n", argv[0]);
     fprintf(stderr, "   type is a single character category\n");
     fprintf(stderr, "   to be removed from the lertdata file\n");
     exit(1);
  }
    
  db = dbm_open(LERTS_DATA, O_RDWR, 0600);
  if (db == NULL) {
    fprintf(stderr, "Unable to open database file %s.\n", LERTS_DATA);
    exit (1);
  }
  key = dbm_firstkey(db);
  while (key.dptr != NULL) {
    data = dbm_fetch(db, key);
    if (!dbm_error(db)) {
      cp = categ;
      changed = FALSE;
      for(name_c = data.dsize, name_p = data.dptr; name_c > 0; name_c--) {
        if (*name_p != argv[1][0]) {
	  *cp = *name_p;
	  cp++;
	} else {
	  changed = TRUE;
	}
	name_p++;
      }

      if (changed) {
        if (data.dsize == 1) {
          if (dbm_delete(db, key) < 0) {
	    fprintf(stderr, "dbm_delete() failed\n");
	    (void) dbm_clearerr(db);
	  }
	} else {
	  new.dsize = data.dsize - 1;
	  new.dptr = categ;
	  if (dbm_store(db, key, new, DBM_REPLACE) < 0) {
	    fprintf(stderr, "dbm_store() failed\n");
	    (void) dbm_clearerr(db);
	  }
	}
	key = dbm_firstkey(db);
      } else {
	key = dbm_nextkey(db);
      }
    }
  }
  return (0);
}
