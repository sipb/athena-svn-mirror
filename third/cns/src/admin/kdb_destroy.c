/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Description.
 */

#include <mit-copyright.h>
#include <string.h>
#include <stdio.h>
#include "krb.h"
#include "krb_db.h"

main(argc, argv)
    int argc;
    char **argv;
{
    char    answer[10];		/* user input */
    char    dbm[256];		/* database path and name */
    char    dbm1[256];		/* database path and name */
    char   *file1, *file2;	/* database file names */

    if (argc > 1) {
        if (argc > 3) {
            fprintf(stderr, "Usage: %s [database name]\n", argv[0]);
            exit(1);
        }
	strcpy(dbm, argv[1]);
	strcpy(dbm1, argv[1]);
    } else {
        strcpy(dbm, DBM_FILE);
        strcpy(dbm1, DBM_FILE);
    }
    file1 = strcat(dbm, ".dir");
    file2 = strcat(dbm1, ".pag");

    printf("You are about to destroy the Kerberos database %s ",file1);
    printf("on this machine.\n");
    printf("Are you sure you want to do this (y/n)? ");
    fgets(answer, sizeof(answer), stdin);

    if (answer[0] == 'y' || answer[0] == 'Y') {
	if (unlink(file1) == 0 && unlink(file2) == 0)
	    fprintf(stderr, "Database deleted at %s\n", file1);
	else
	    fprintf(stderr, "Database cannot be deleted at %s\n",
		    DBM_FILE);
    } else
	fprintf(stderr, "Database not deleted.\n");
}
