/* $Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/bill_db.c,v $ */
/* $Author: epeisach $ */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */
#include <stdio.h>
#include <strings.h>
#include <krb.h>
#include <time.h>
#include <signal.h>
#include "quota_db.h"
#include "quota.h"

/* Define all codes that that the bursar files will use. */
#define STUFF "SU1"
#define ATHCODE "385"
#define TITLE "Printing: "
#define TITLE1 "Printing Charges"
#define ACC "24745"
#define OBJ "480"

/* Global Variables */
static char filename1[] = "/tmp/db_dumpXXXXXX";
static char filename2[] = "/tmp/bursar_studentsXXXXXX";
static char filename3[] = "/tmp/bursar_facultyXXXXXX";

int clean1=0, clean2=0, clean3=0;

FILE *fp, *fp2, *fp3;
char realm[REALM_SZ];
/* long quota_start_update(), quota_end_update(); */

struct person {
  char username[9];       /* NULL terminated username */
  int number;
  char flag;
  char real_name[33];
  struct person *next;
};

main(argc, argv)
int argc;
char *argv[];
{
  struct person *name_list, *read_name_list();
  int dump(), kresult, cleanup();
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
      fprintf(stderr, "bill_db user_file quota_db\n");
      exit(1);
  }
/* ask user for semester */
  printf("Please enter the semester.\n");
  printf("    1 = Fall\n");
  printf("    2 = Spring\n");
  printf("    s = Summer\n");
  scanf("%c", &semester);

/* start */
  kresult = krb_get_lrealm(realm, 1);
  if (kresult != KSUCCESS)
    fprintf(stderr, "bill_db: error in obtaining realm\n");
  else if (!(name_list = read_name_list(argv[1])))
    fprintf(stderr, "error in reading name list file %s\n", argv[1]);
  else if (quota_db_set_name(argv[2]))
    fprintf(stderr, "error in setting db name %s\n", argv[2]);
  else {

/* generate unique file names */
    mktemp(filename1);
    clean1=1;
    mktemp(filename2);
    clean2=1;
    mktemp(filename3);
    clean3=1;

/* dump database */
    fp = fopen(filename1, "w");
    /* age = quota_start_update(argv[2]); */
    if (quota_db_iterate(dump, 0) < 0)
      exit(-1);
    /* quota_end_update(argv[2], age); */
    (void)fclose(fp);

/* now use dump to generate bursar file and restore */
    fp = fopen(filename1, "r");
    fp2 = fopen(filename2, "w");
    fp3 = fopen(filename3, "w");
    /* age = quota_start_update(argv[2]); */
    generate_bursar(name_list, month, day, year, semester);
    /* quota_end_update(argv[2], age); */
    (void)fclose(fp);
    (void)fclose(fp2);
    (void)fclose(fp3);
    unlink(filename1);
  }
  exit(0);
}

/*
 * Read_name_list function: Will read a name list file consisting of
 * "username:social_security:flag:real_name" into memeory as a linked list.
 */

struct person *read_name_list(argv)
char* argv;
{
  int trip=0;
  struct person *current, *new, *name_list;
  char user_name[9], flag, real_name[33];     /* NULL terminated username */
  int number;

  current = new = name_list = (struct person *)NULL;

  if(!(fp = fopen(argv, "r"))) return NULL;
  while (fscanf(fp, "%[^:]:%d:%c:%[^\n]\n", user_name, &number, 
		&flag, real_name) != EOF) {
    if (trip == 1) 
      new = (struct person *)malloc((unsigned)sizeof(struct person));
    else {
      name_list = (struct person *)malloc((unsigned)sizeof(struct person));
      new = name_list;
    }
    if (new == NULL) {
      fprintf(stderr, "Out of memeory at %s", user_name);
      exit(-1);
    }
    strcpy(new->username, user_name);
    new->number = number;
    new->flag = flag;
    strcpy(new->real_name, real_name);
    new->next = NULL;
    if (trip == 1)
      current->next = new;
    current = new;
    trip = 1;
  }
  (void)fclose(fp);
  if (trip == 0)
    return(NULL);
  return(name_list);
}

/*
 * dump function: will dump the database into a text file to be used
 * with the upcoming fscanf to get user, realm, instance, and service.
 */

dump(arg, qrec)
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

/*
 * Generate_bursar Function: will use dumped database file, with the linked
 * list in memory and generate two bursar files with appropriate formats
 * and update the database using get and put principal functions.
 */

generate_bursar(name_list, month, day, year, semester)
struct person *name_list;
int month, day, year;
char semester;
{
  quota_rec qrec, temp;
  int billing_amount, found;
  struct person *current;
  int more;

  bzero(&temp, sizeof(temp));
  while (fscanf(fp, "%[^:]:%[^:]:%[^:]:%[^:]: %d %d %d %d %d %d %d %d %d %d\n",
		 temp.name, temp.instance, temp.realm, temp.service,
		 &temp.uid, &temp.quotaAmount, &temp.quotaLimit,
		 &temp.lastBilling, &temp.lastCharge,
		 &temp.pendingCharge, &temp.lastQuotaAmount,
		 &temp.yearToDateCharge, &temp.allowedToPrint,
		 &temp.deleted) != EOF) {
    found = 0;
    for (current = name_list; current && !found; current = current->next) {
      if ((strcmp(realm, temp.realm) == 0) &&   
	  (strcmp(current->username, temp.name) == 0)) {
	found = 1;

	/* do calculations */
	bzero(&qrec, sizeof(qrec));
	if (quota_db_get_principal(temp.name, temp.instance, temp.service,
				   temp.realm, &qrec, (unsigned int)1,
				   &more) == 1) { 
	  billing_amount = qrec.quotaAmount - qrec.quotaLimit
	    - qrec.yearToDateCharge;
	  if (billing_amount < 500) {  /* restore and continue */
	    qrec.pendingCharge = billing_amount;
	    if (quota_db_put_principal(&qrec, (unsigned int)1) <= 0)
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
	  print_bursar(current, month, day, year, semester, billing_amount);

	  /* now restore */
	  qrec.lastBilling = (quota_time)time(0);
	  qrec.lastCharge = billing_amount;
	  qrec.yearToDateCharge += billing_amount;
	  /* The following is to cause the quotaLimit 
	     to reflect total allowed per managers request */
	  qrec.quotaLimit += billing_amount;
	  if (quota_db_put_principal(&qrec, (unsigned int)1) <= 0)
	    printf("database locked\n");
	}
	else
	  fprintf(stderr, "%s not found with get_principal", temp.name);
      }
    }
    if (found) {
      bzero(&temp, sizeof(temp));
      continue;
    }
    /* else the username was not found */
    if (temp.quotaAmount - temp.quotaLimit - temp.yearToDateCharge > 500) {
      fprintf(stderr, "%s was not found in list and ", temp.name);
      fprintf(stderr, "needs to be billed for %d.\n",
	      temp.quotaAmount - temp.quotaLimit - temp.yearToDateCharge);
    }
    bzero(&temp, sizeof(temp));
  }
  return(0);
}

/*
 * print_bursar function: prints depending on flag of structure with
 * proper formats
 */

print_bursar(user, month, day, year, semester, billing_amount)
struct person *user;
int month, day, year, billing_amount;
char semester;
{
  if (user->flag == 's') {
    /* print students into bursar file with proper format */
    fprintf(fp2, "%3s", STUFF);             /* trans code */
    fprintf(fp2, "%09d", user->number);  /* ID number */
    fprintf(fp2, "%3s", ATHCODE);           /* Athena code */
    fprintf(fp2, "%2d%c", year+(semester == '1' ? 1 : 0), semester);  /* semester */
    fprintf(fp2, "%02d%02d%2d", month, day, year); /* billing date */
    fprintf(fp2, "%08d", billing_amount);   /* amount */
    fprintf(fp2, "%-30.30s", TITLE1);            /* title */
    fprintf(fp2, "%5s", ACC);               /* account # */
    fprintf(fp2, "%3s", OBJ);               /* obj code */
    fprintf(fp2, "      ");                 /* space for Bursar code */
    fprintf(fp2, "ATHN\n");                 /* office code */
  }
  else {
    /* print faculty into bursar file with proper format */
    fprintf(fp3, "         ");              /* blank */
    fprintf(fp3, "%5s", ACC);               /* acount# */
    fprintf(fp3, "%3s", OBJ);               /* obj code */
    fprintf(fp3, "%3s", STUFF);             /* trans code */
    fprintf(fp3, " ");                      /* blank */
    fprintf(fp3, "%10s", TITLE);            /* title */
    fprintf(fp3, "%8s:", user->username);   /* username */
    fprintf(fp3, "%-23.23s", user->real_name);  /* real name */
    fprintf(fp3, "%02d%02d%2d", month, day, year); /* billing date */
    fprintf(fp3, "%011d", billing_amount);  /* amount */
    fprintf(fp3, "J\n");                    /* weird char */
  }
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
  if (clean3 == 1) {
    unlink(filename3);
    fprintf(stderr, "%s removed.\n", filename3);
  }
  exit(-1);
}

void PROTECT(){}
void UNPROTECT(){}


