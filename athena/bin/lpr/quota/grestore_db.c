/* $Id: grestore_db.c,v 1.4 1999-01-22 23:11:00 ghudson Exp $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <strings.h>
#include "gquota_db.h"

/* These have to be defined here because quota_dba.o declares them */
/* extern.   Normally, the are declared in qmain.c. */
#ifdef DEBUG
char *progname = "grestore_db";
int quota_debug=1;
int gquota_debug=1;
#endif

int gquota_db_create(), gquota_db_set_name();

main(argc, argv)
int argc;
char *argv[];
{
  FILE *fp;
  gquota_rec qrec;
  char temp_string[10], title[20], title_num[4];
  long temp_account;
  int i, trip=0, temp_num;


  if (argc != 3) {
      fprintf(stderr, "Usage: grestore_db gdump_file gquota_db\n");
      exit(1);
  }

  fp = fopen(argv[1], "r");
  if (gquota_db_create(argv[2])) {
    printf("error in creating db %s\n", argv[2]);
    exit(-1);
  }
  if (gquota_db_set_name(argv[2])) {
    printf("error in setting db name %s\n", argv[2]);
    exit(-1);
  }
  fscanf(fp, "%s %s\n", title, title_num);
  if (strcmp(title_num, "1.1")) {
    printf("wrong database\n");
    exit(-1);
  }
  bzero(&qrec, sizeof(qrec));
  while (fscanf(fp, "%d:%[^:]:%d", &temp_account,
		temp_string, &temp_num) != EOF) {
    if (strcmp(temp_string, "admin")==0) {
      qrec.admin[0] = temp_num;
      for (i = 1; i <= temp_num; i++)
	fscanf(fp, ":%d", &(qrec.admin[i]));
      fscanf(fp, "\n");
    }
    else if (strcmp(temp_string, "user")==0) {
      qrec.user[0] = temp_num;
      for (i = 1; i <= temp_num; i++)
	fscanf(fp, ":%d", &(qrec.user[i]));
      fscanf(fp, "\n");
    }
    else {
      if (trip == 1) {
	if (gquota_db_put_group(&qrec, (unsigned int)1) <= 0) {
	  printf("database locked\n");
	  exit(-1);
	}
	bzero(&qrec, sizeof(qrec));
	qrec.account = temp_account;
	strcpy(qrec.service, temp_string);
	qrec.quotaAmount = temp_num;
	fscanf(fp, " %d %d %d %d %d %d %d %d\n", &qrec.quotaLimit,
	       &qrec.lastBilling, &qrec.lastCharge, &qrec.pendingCharge,
	       &qrec.lastQuotaAmount, &qrec.yearToDateCharge,
	       &qrec.allowedToPrint, &qrec.deleted);
      }
      else {
	trip = 1;
	qrec.account = temp_account;
	strcpy(qrec.service, temp_string);
	qrec.quotaAmount = temp_num;
	fscanf(fp, " %d %d %d %d %d %d %d %d\n", &qrec.quotaLimit,
	       &qrec.lastBilling, &qrec.lastCharge, &qrec.pendingCharge,
	       &qrec.lastQuotaAmount, &qrec.yearToDateCharge,
	       &qrec.allowedToPrint, &qrec.deleted);
      }
    }
  }
  if (trip && gquota_db_put_group(&qrec, (unsigned int)1) <= 0) {
    printf("database locked\n");
    exit(-1);
  }
  (void)fclose(fp);
  exit(0);
}

void PROTECT(){}
void UNPROTECT(){}



