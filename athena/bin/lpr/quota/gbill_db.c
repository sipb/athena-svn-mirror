/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/gbill_db.c,v $ */
/* $Author: danw $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <strings.h>
#include <krb.h>
#include <time.h>
#include <signal.h>
#include "gquota_db.h"
#include "quota.h"

/* These have to be defined here because quota_dba.o declares them */
/* extern.   Normally, the are declared in qmain.c. */
#ifdef DEBUG
char *progname = "gbill_db";
int quota_debug=1;
int gquota_debug=1;
#endif

/* Define all codes that that the bursar files will use. */
#define STUFF "SU1"
#define ATHCODE "385"
#define TITLE "Printing: "
#define ACC "24745"
#define OBJ "480"

/* file names*/
static char filename1[] = "/tmp/db_dump_groupXXXXXX";
static char filename2[] = "/tmp/bursar_groupsXXXXXX";

int clean1=0, clean2=0;     /* cleanup temp variables */

FILE *fp, *fp2;
char realm[REALM_SZ];

/* function declarations */
/* long gquota_start_update(), gquota_end_update(); */
int gquota_db_set_name(), gquota_db_iterate();
int gquota_db_get_group(), gquota_db_put_group();

struct group {
  int group_number;
  int account_number;
  struct group *next;
};

main(argc, argv)
int argc;
char *argv[];
{
  struct group *group_list, *read_group_list();
  int dump_groups(), cleanup();
  struct tm *time_str;
  int day, month, year;
  long given_time;
  /* long age; */
  char semester;

  signal(SIGINT, cleanup);

/* set time for files */
  given_time = time(0);
  time_str = localtime(&given_time);
  day = time_str->tm_mday;
  year = time_str->tm_year;
  month = time_str->tm_mon + 1;

  if(argc != 3) {
      fprintf(stderr, "gbill_db group_user_file quota_db\n");
      exit(1);
  }

/* ask user for semester */
  printf("Please enter the semester.\n");
  printf("    1 = Fall\n");
  printf("    2 = Spring\n");
  printf("    s = Summer\n");
  scanf("%c", &semester);

/* start */
  if (!(group_list = read_group_list(argv[1])))
    fprintf(stderr, "error in reading group list file %s\n", argv[1]);
  else if (gquota_db_set_name(argv[2]))
    fprintf(stderr, "error in setting db name %s\n", argv[2]);
  else {

/* generate unique file names */
    mktemp(filename1);
    clean1=1;
    mktemp(filename2);
    clean2=1;

/* dump database */
    fp = fopen(filename1, "w");
    /* age = gquota_start_update(argv[2]); */
    if (gquota_db_iterate(dump_groups, 0) < 0)
      exit(-1);
    /* gquota_end_update(argv[2], age); */
    (void)fclose(fp);

/* now use dump to generate bursar file and restore */
    fp = fopen(filename1, "r");
    fp2 = fopen(filename2, "w");
    /* age = gquota_start_update(argv[2]); */
    generate_bursar(group_list, month, day, year, semester);
    /* gquota_end_update(argv[2], age); */
    (void)fclose(fp);
    (void)fclose(fp2);
    unlink(filename1);
  }
  exit(0);
}

/*
 * Read_group_list function: Will read a group list file consisting of
 * "group_number:account_number" into memeory as a linked list.
 */

struct group *read_group_list(argv)
char* argv;
{
  int trip=0;
  struct group *current, *new, *group_list;
  int group, number;

  current = new = group_list = (struct group *)NULL;

  if(!(fp = fopen(argv, "r"))) return NULL;
  while (fscanf(fp, "%d:%d\n", &number, &group) != EOF) {
    if (trip == 1) 
      new = (struct group *)malloc((unsigned)sizeof(struct group));
    else {
      group_list = (struct group *)malloc((unsigned)sizeof(struct group));
      new = group_list;
    }
    if (new == NULL) {
      fprintf(stderr, "Out of memeory at %d", group);
      exit(-1);
    }
    new->group_number = group;
    new->account_number = number;
    new->next = NULL;
    if (trip == 1)
      current->next = new;
    current = new;
    trip = 1;
  }
  (void)fclose(fp);
  if (trip == 0)
    return(NULL);
  return(group_list);
}

/*
 * dump_groups function: will dump the database into a text file to be
 * used with the upcoming fscanf to get service.
 *
 * Only the account # and service are needed for the gquota_db_get_group
 * The first line is dumped for error purposes. The arrays of admin and
 * user are ignored.
 */

dump_groups(arg, qrec)
char *arg;
gquota_rec *qrec;
{
  fprintf(fp, "%d:%s:", qrec->account, qrec->service);
  fprintf(fp, "%8d:%8d:", qrec->admin[0], qrec->user[0]);
  fprintf(fp, "%d ", qrec->quotaAmount);
  fprintf(fp, "%d %d ", qrec->quotaLimit, qrec->lastBilling);
  fprintf(fp, "%d %d ", qrec->lastCharge, qrec->pendingCharge);
  fprintf(fp, "%d %d ", qrec->lastQuotaAmount, qrec->yearToDateCharge);
  fprintf(fp, "%d %d\n", qrec->allowedToPrint, qrec->deleted);
  return(0);
}

/*
 * Generate_bursar Function: will use dumped database file, with the linked
 * list in memory and generate a bursar file with appropriate format
 * and update the database using get and put group functions.
 *
 * The dump doesn't contain admin and user lists. That info is not needed
 * for billing purposes.
 */

generate_bursar(group_list, month, day, year, semester)
struct group *group_list;
int month, day, year;
char semester;
{
  gquota_rec qrec, temp;
  struct group *current;
  int billing_amount, found, more;

  while (fscanf(fp, "%d:%[^:]:%d:%d: %d %d %d %d %d %d %d %d %d\n", 
		&temp.account, temp.service, &temp.admin[0], &temp.user[0],
		&temp.quotaAmount, &temp.quotaLimit,
		&temp.lastBilling, &temp.lastCharge,
		&temp.pendingCharge, &temp.lastQuotaAmount,
		&temp.yearToDateCharge, &temp.allowedToPrint,
		&temp.deleted) != EOF) {
    found = 0;
    for (current = group_list; current && !found; current = current->next) {
      if (current->account_number == temp.account) {
	found = 1;

	/* do calculations */
	bzero(&qrec, sizeof(qrec));
	if (gquota_db_get_group(temp.account, temp.service, &qrec,
				(unsigned int)1, &more) == 1) {
	  billing_amount = qrec.quotaAmount - qrec.quotaLimit;
	  if (billing_amount < 500) {  /* restore and continue */
	    qrec.pendingCharge = billing_amount;
	    if (gquota_db_put_group(&qrec, (unsigned int)1) <= 0)
	      printf("database locked\n");
	    continue;
	  }
	  else if (billing_amount > 30000) {
	    qrec.pendingCharge = billing_amount - 30000;
	    billing_amount = 30000;
	  }
	  else
	    qrec.pendingCharge = 0;

	  /* now send to print routine */
	  print_bursar(current, month, day, year, semester, 
		       billing_amount);
	  
	  /* now restore */
	  qrec.lastBilling = (gquota_time)time(0);
	  qrec.yearToDateCharge += billing_amount;
	  /* The following is to cause the quotaLimit 
	     to reflect total allowed per managers request */
	  qrec.quotaLimit += billing_amount;
	  if (gquota_db_put_group(&qrec, (unsigned int)1) <= 0)
	    printf("database locked\n");
	}
	else
	  fprintf(stderr, "%d not found with get_group", temp.account);
      }
    }
    if (found)
      continue;

    /* else the temp.account was not found */
    if (temp.quotaAmount - temp.quotaLimit > 500) {
      fprintf(stderr, "Group %d was not found in list and ", temp.account);
      fprintf(stderr, "needs to be billed for %d.\n",
	      temp.quotaAmount - temp.quotaLimit);
    }
  }
  return(0);
}

/*
 * print_bursar function: prints to tmp file with proper formats
 */

print_bursar(group, month, day, year, semester, billing_amount)
struct group *group;
int month, day, year, billing_amount;
char semester;
{
  /* print students into bursar file with proper format */
  fprintf(fp2, "%3s", STUFF);             /* trans code */
  fprintf(fp2, "%09d", group->account_number);    /* ID number */
  fprintf(fp2, "%3s", ATHCODE);           /* Athena code */
  fprintf(fp2, "%2d%c", (year+(semester == '1' ? 1 : 0))%100, semester);  /* semester */
  fprintf(fp2, "%02d%02d%2d", month, day, year%100); /* billing date */
  fprintf(fp2, "%08d", billing_amount);   /* amount */
  fprintf(fp2, "%10s", TITLE);            /* title */
  fprintf(fp2, "%-20.20d", group->group_number);  /* real name */
  fprintf(fp2, "%5s", ACC);               /* account # */
  fprintf(fp2, "%3s", OBJ);               /* obj code */
  fprintf(fp2, "      ");                 /* space for Bursar code */
  fprintf(fp2, "ATHN\n");                 /* office code */
}

cleanup()
{
  if (clean1 == 1) {
    unlink(filename1);
  }
  if (clean2 == 1) {
    unlink(filename2);
    fprintf(stderr, "%s removed.\n", filename2);
  }
  exit(-1);
}

void PROTECT(){}
void UNPROTECT(){}


















