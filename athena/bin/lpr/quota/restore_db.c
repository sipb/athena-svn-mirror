/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/restore_db.c,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <strings.h>
#include "quota_db.h"

/* These have to be defined here because quota_dba.o declares them */
/* extern.   Normally, the are declared in qmain.c. */
#ifdef DEBUG
char *progname = "restore_db";
int quota_debug=1;
#endif

int quota_db_create(), quota_db_set_name();

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp;
  quota_rec qrec;
  char temp[20], temp2[4];

  if (argc != 3) {
      fprintf(stderr, "Usage: restore_db dump_file quota_db\n");
      exit(1);
  }

  fp = fopen(argv[1], "r");
  if (quota_db_create(argv[2]))
    printf("error in creating db %s\n", argv[2]);
  else if (quota_db_set_name(argv[2]))
      printf("error in setting db name %s\n", argv[2]);
  else {
    fscanf(fp, "%s %s\n", temp, temp2);
    if (strcmp(temp2, "1.1"))
      printf("wrong database\n");
    else
      while ((fscanf(fp, "%[^:]:%[^:]:%[^:]:%[^:]: %d %d %d %d %d %d %d %d %d %d\n",
		     qrec.name, qrec.instance, qrec.realm, qrec.service,
		     &qrec.uid, &qrec.quotaAmount, &qrec.quotaLimit,
		     &qrec.lastBilling, &qrec.lastCharge,
		     &qrec.pendingCharge, &qrec.lastQuotaAmount,
		     &qrec.yearToDateCharge, &qrec.allowedToPrint,
		     &qrec.deleted)) != EOF) {
	if (quota_db_put_principal(&qrec, (unsigned int)1) <= 0)
	  printf("database locked\n");
	bzero(&qrec, sizeof(qrec));
      }
  }
  (void)fclose(fp);
  exit(0);
}

void PROTECT(){}
void UNPROTECT(){}



