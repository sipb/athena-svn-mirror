/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/dump_db.c,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <signal.h>
#include "quota_db.h"

FILE *fp;
int clean;
static char *filename;

/* long quota_end_update(), quota_start_update(); */
int quota_db_iterate();

main(argc, argv)
int argc;
char *argv[];
{

  FILE *fopen();
  int print(), cleanup();
  /* long age; */

  signal(SIGINT, cleanup);

  if (argc != 3) {
      fprintf(stderr, "Usage: dump_db quota_db dump_file\n");
      exit(1);
  }

  quota_db_set_name(argv[1]);

  filename=argv[2];
  fp = fopen(argv[2], "w");
  clean = 1;

  /* age = quota_start_update(argv[1]); */
  fprintf(fp, "Version 1.1\n");
  quota_db_iterate(print, 0);
  /* quota_end_update(argv[1], age); */
  (void)fclose(fp);
  exit(0);
}

print(arg, qrec)
char *arg;
quota_rec *qrec;
{
  fprintf(fp, "%s:%s:", qrec->name, qrec->instance);
  fprintf(fp, "%s:%s:", qrec->realm, qrec->service);
  fprintf(fp, "%d %d ", qrec->uid, qrec->quotaAmount);
  fprintf(fp, "%d %d ", qrec->quotaLimit, qrec->lastBilling);
  fprintf(fp, "%d %d ", qrec->lastCharge, qrec->pendingCharge);
  fprintf(fp, "%d %d ", qrec->lastQuotaAmount, qrec->yearToDateCharge);
  fprintf(fp, "%d %d\n", qrec->allowedToPrint, qrec->deleted);
  return(0);
}

cleanup()
{
  if (clean == 1) {
    unlink(filename);
    fprintf(stderr, "%s removed.\n", filename);
  }
  exit(-1);
}

void PROTECT(){}
void UNPROTECT(){}



