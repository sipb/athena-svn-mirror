/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#include "mit-copyright.h"
#include "config.h"
#include "quota_db.h"
#include "gquota_db.h"

#ifdef DEBUG
int quota_debug = 0;
int gquota_debug = 0;
#endif
char *progname;

main(argc, argv)
int argc;
char **argv;
    {
	int ret;

	if(argc != 3) {
	    printf("%s db_name group_db_name\n",argv[0]);
	    exit(1);
	}

	if(ret=quota_db_create(argv[1])) {
	    perror("Error in creating db");
	    exit(1);
	}

	if(ret=gquota_db_create(argv[2])) {
	    perror("Error in creating group db");
	    exit(1);
	}

	exit(0);
    }

/* DUMMY ROUTINES */
void PROTECT() {}
void UNPROTECT() {}
