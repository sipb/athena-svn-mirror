/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "quota_db.h"

#ifdef DEBUG
int quota_debug = 0;
#endif
char *progname;

main(argc, argv)
int argc;
char **argv;
    {
	int ret;

	if(argc != 2) {
	    printf("%s db_name\n",argv[0]);
	    exit(1);
	}

	if(ret=quota_db_create(argv[1])) {
	    perror("Error in creating db");
	    exit(1);
	}

	exit(0);
    }

/* DUMMY ROUTINES */
void PROTECT() {}
void UNPROTECT() {}
