/*
   file: lertused.c
   basic use: lertused name
     removes a user from lert db
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
     fprintf(stderr, "usage: %s name\n", argv[0]);
     fprintf(stderr, "   lertused is a user name\n");
     fprintf(stderr, "   to be removed from the lertdata database\n");
     exit(1);
  }
    
  db = dbm_open(LERTS_DATA, O_RDWR, 0600);
  if (db == NULL) {
    fprintf(stderr, "Unable to open database file %s.\n", LERTS_DATA);
    exit (1);
  }
  for (key = dbm_firstkey(db); key.dptr != NULL; key = dbm_nextkey(db)) {
    if (!dbm_error(db)) {
      if (strncmp(argv[1], key.dptr, key.dsize) == 0) {
        if (dbm_delete(db, key) < 0) {
	  fprintf(stderr, "dbm_delete() failed\n");
	  (void) dbm_clearerr(db);
	}
      }
    }
  }
  return (0);
}
    
