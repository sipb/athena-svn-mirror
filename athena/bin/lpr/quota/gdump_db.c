/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gdump_db.c,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <signal.h>
#include "gquota_db.h"

/* These have to be defined here because quota_dba.o declares them */
/* extern.   Normally, the are declared in qmain.c. */
#ifdef DEBUG
char *progname = "gdump_db";
int quota_debug=1;
int gquota_debug=1;
#endif

FILE *fp;
int clean;
static char *filename;

/* long gquota_start_update(), gquota_end_update(); */
int gquota_db_iterate();

main(argc, argv)
int argc;
char *argv[];
{

  FILE *fopen();
  int read_print(), cleanup();
  /* long age; */

  signal(SIGINT, cleanup);

  if (argc != 3) {
      fprintf(stderr, "Usage: gdump_db gquota_db dump_file\n");
      exit(1);
  }

  gquota_db_set_name(argv[1]);

  filename = argv[2];
  fp = fopen(filename, "w");
  clean = 1;

  /* age = gquota_start_update(argv[1]); */
  fprintf(fp, "Version 1.1\n");
  gquota_db_iterate(read_print, 0);
  /* gquota_end_update(argv[1], age); */
  (void)fclose(fp);
  exit(0);
}

read_print(arg, qrec)
char *arg;
gquota_rec *qrec;
{
  int i;

  fprintf(fp, "%d:%s:", qrec->account, qrec->service);
  fprintf(fp, "%d ", qrec->quotaAmount);
  fprintf(fp, "%d %d ", qrec->quotaLimit, qrec->lastBilling);
  fprintf(fp, "%d %d ", qrec->lastCharge, qrec->pendingCharge);
  fprintf(fp, "%d %d ", qrec->lastQuotaAmount, qrec->yearToDateCharge);
  fprintf(fp, "%d %d\n", qrec->allowedToPrint, qrec->deleted);

  fprintf(fp, "%d:admin:%d", qrec->account, qrec->admin[0]);
  for (i = 1; i <= qrec->admin[0]; i++)
    fprintf(fp, ":%d", qrec->admin[i]);
  fprintf(fp, "\n");
  fprintf(fp, "%d:user:%d", qrec->account, qrec->user[0]);
  for (i = 1; i <= qrec->user[0]; i++)
    fprintf(fp, ":%d", qrec->user[i]);
  fprintf(fp, "\n");
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

