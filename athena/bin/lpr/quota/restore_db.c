/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/restore_db.c,v $ */
/* $Author: ilham $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <strings.h>
#include "quota_db.h"

int quota_db_create(), quota_db_set_name();

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp;
  quota_rec qrec;
  char temp[20], temp2[3];

  fp = fopen(argv[1], "r");
  if (quota_db_create(argv[1]))
    printf("error in creating db %s\n", argv[1]);
  else if (quota_db_set_name(argv[1]))
      printf("error in setting db name %s\n", argv[1]);
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



