/*
   file: lertsaid.c
   dump the dbm data file where I'm keeping records

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
  datum old;
  datum data;
  register char *cp;
  register char *name_p;
  int name_c;
  char name[128];
  char categ[128];

  if (argc != 1) {
     fprintf(stderr, "usage: %s\n", argv[0]);
     fprintf(stderr, "   dumps the file of people who have responded -n\n");
     exit(1);
  }
    
  db = dbm_open(LERTS_LOG, O_RDONLY, 0600);
  if (db == NULL) {
    fprintf(stderr, "Unable to open database file %s.\n", LERTS_LOG);
    exit (1);
  }

  for (key = dbm_firstkey(db); key.dptr != NULL; key = dbm_nextkey(db)) {
    data = dbm_fetch(db, key);
    if (!dbm_error(db)) {
      cp = name;
      for(name_c = key.dsize, name_p = key.dptr; name_c > 0; name_c--) {
	*cp = *name_p;
	cp++;
	name_p++;
      }
      cp = categ;
      for(name_c = data.dsize, name_p = data.dptr; name_c > 0; name_c--) {
	*cp = *name_p;
	cp++;
	name_p++;
      }
      *cp = '\0';
      printf("name: %s  categories: %s\n", name, categ);
    }
  }
  return (0);
}
    
