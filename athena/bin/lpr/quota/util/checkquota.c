#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/file.h>
#include "quota_db.h"

#define PART 500 /* 1000 cents = 100 pages */
#define MB 2000
int bucket[MB];
#define ibuck(use) if((use)/PART > MB-1) \
    {fprintf(stderr,"Increase nbuckets %d\n", use); exit(4);} \
    bucket[(use+PART-1)/PART]++;
#define bval(n) bucket[(n)]

main(argc, argv)
int argc;
char **argv;
    {
	char *db;
	FILE *fp;
	quota_rec qrec;
	char temp[20], temp2[4];
	int nusers=0;
	int i=0;

	if(argc != 2) usage();
	bzero(bucket, sizeof(int)* MB);

	if(access((db = argv[1]), R_OK)) {
	    fprintf(stderr, "Cannot open dump db: %s\n", db);
	    exit(2);
	}
	fp = fopen(db, "r");
	
	if (fscanf(fp, "%s %s\n",temp, temp2) != 2 ||strcmp(temp2, "1.1")) {
	    fprintf(stderr, "Version mismatch in db\n");
	    exit(3);
	}
      while ((fscanf(fp, "%[^:]:%[^:]:%[^:]:%[^:]: %d %d %d %d %d %d %d %d %d %d\n",
		     qrec.name, qrec.instance, qrec.realm, qrec.service,
		     &qrec.uid, &qrec.quotaAmount, &qrec.quotaLimit,
		     &qrec.lastBilling, &qrec.lastCharge,
		     &qrec.pendingCharge, &qrec.lastQuotaAmount,
		     &qrec.yearToDateCharge, &qrec.allowedToPrint,
		     &qrec.deleted)) != EOF) {
	  nusers++;
	  ibuck(qrec.quotaAmount);
      }

	fclose(fp);
	if(!nusers) printf("No data to report on\n");
	else {
#if 0
	    printf("%d users in buckets of %d\n", nusers, PART);
	    printf("%6d - %6d %8d\n", 0, 0, bval(i));
#endif
	    for(i=1; i<MB; i++) {
#if 0
		if(bval(i)) printf("%6d - %6d %8d\n", (i-1)*PART+1, i*PART, bval(i));
#else
		if(bval(i)) printf("%8d %6d\n", bval(i), i*PART/10, bval(i));
#endif
	    }
	}
	exit(0);
    }

usage()
    {
	fprintf(stderr, "checkquota quota_dump\n");
	exit(1);
    }
