#include <stdio.h>
#include <sys/time.h>
#include <tzfile.h>
#include <ctype.h>
#include "config.h"
#include "logger.h"
#include "logger_ncs.h"

/* BUGS: Credits are not properly handled in regards to billing.... */

static struct timeval	tv;
static int	dmsize[] =
	{ -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


struct _day_record {
    int njobs;
    int npages;
    int start;
    int month;
} rec[370];

int pages[1000];

main(argc, argv)
int argc;
char **argv;
    {
	char *journal_dump;
	FILE *f;
	char in_buf[BUFSIZ];
	log_entity log;
	int line = 0, ncharges = 0, maxent=0, maxpage=0, totpages=0;
	int numdays, basesec, ent;
	struct timezone	tz;
	int i;
	
	if(argc != 5) usage();

	for(ent=0; ent<1000; ent++) pages[ent]=0;

	journal_dump = argv[1];

	if (gettimeofday(&tv,&tz)) {
		perror("gettimeofday");
		exit(1);
	}

	if(gtime("9009010101")) usage();

	if((numdays = atoi(argv[4])) <= 0) usage();

	tv.tv_sec += (long)tz.tz_minuteswest * SECS_PER_MIN;
			/* now fix up local daylight time */

	basesec = tv.tv_sec;
	printf("Starting for %d days\n", numdays);
        rec[0].start = basesec;
	rec[0].month = 9;
	for(i=1; i<numdays; i++) {
		rec[i].start=rec[i-1].start+86400*dmsize[rec[i-1].month];
		rec[i].month=rec[i-1].month+1;
		if(rec[i].month == 13) rec[i].month = 1;
	}



	if(!(f=fopen(journal_dump, "r"))) {
	    fprintf(stderr, "Could not open journal dump");
	    exit(2);
	}

	if(logger_string_set_name(argv[2])) {
	    fprintf(stderr, "Could not set string table");
	    exit(3);
	}

	while(fgets(in_buf, BUFSIZ, f)) {
	    line++;
	    if(logger_parse_quota(in_buf, &log)) {
		fprintf(stderr, "Could not parse line %d %s\n", line, in_buf);
	    }
	    if(!(line%1000)) printf("At line %d\n",line);
	    if(log.time < basesec || log.time > basesec+numdays*SECS_PER_DAY)
		continue;
	    if(log.func != LO_CHARGE) continue;
	    ncharges++;
	    ent=99;
	    for(i = 0; i <= numdays; i++) {
		    if(log.time >= rec[i].start && log.time <= rec[i+1].start){
			    ent=i;
			    break;
		    }
	    }
/*	    ent = (log.time - basesec) / SECS_PER_DAY;*/
	    if(ent>maxent) maxent=ent;
	    rec[ent].njobs++;
	    rec[ent].npages += log.trans.charge.npages;
	    totpages += log.trans.charge.npages;
	    pages[log.trans.charge.npages]++;
	    if(maxpage < log.trans.charge.npages) 
		maxpage = log.trans.charge.npages;
	}

	printf("%d lines parsed %d charges\n", line, ncharges);
	for(ent=0; ent<=maxent; ent++) {
	    printf("%d\t%d\t%d\n", ent, rec[ent].njobs, rec[ent].npages);
	}

	printf("Page count stats %d total\n", totpages);
	for(ent=0; ent<=maxpage; ent++) 
	    if(pages[ent]) printf("%d\t%d\n", ent, pages[ent]);
       
	exit(0);

    }


	
	


usage() 
	{
	    fprintf(stderr, "countjobs journal_dump tmp_string_table startday #days\n");
	    fprintf(stderr, "Date in form: [yymmddhhmm[.ss]]\n");
	    exit(1);
	}

void PROTECT(){}
void UNPROTECT() {}

#define	ATOI2(ar)	(ar[0] - '0') * 10 + (ar[1] - '0'); ar += 2;

/*
 * gtime --
 *	convert user's time into number of seconds
 */
static
gtime(ap)
	register char	*ap;		/* user argument */
{
	register int	year, month;
	register char	*C;		/* pointer into time argument */
	struct tm	*L;
	int	day, hour, mins, secs;

	for (secs = 0, C = ap;*C;++C) {
		if (*C == '.') {		/* seconds provided */
			if (strlen(C) != 3)
				return(1);
			*C = NULL;
			secs = (C[1] - '0') * 10 + (C[2] - '0');
			break;
		}
		if (!isdigit(*C))
			return(-1);
	}

	L = localtime((time_t *)&tv.tv_sec);
	year = L->tm_year;			/* defaults */
	month = L->tm_mon + 1;
	day = L->tm_mday;

	switch ((int)(C - ap)) {		/* length */
		case 10:			/* yymmddhhmm */
			year = ATOI2(ap);
		case 8:				/* mmddhhmm */
			month = ATOI2(ap);
		case 6:				/* ddhhmm */
			day = ATOI2(ap);
		case 4:				/* hhmm */
			hour = ATOI2(ap);
			mins = ATOI2(ap);
			break;
		default:
			return(1);
	}

	if (*ap || month < 1 || month > 12 || day < 1 || day > 31 ||
	     mins < 0 || mins > 59 || secs < 0 || secs > 59)
		return(1);
	if (hour == 24) {
		++day;
		hour = 0;
	}
	else if (hour < 0 || hour > 23)
		return(1);

	tv.tv_sec = 0;
	year += TM_YEAR_BASE;
	if (isleap(year) && month > 2)
		++tv.tv_sec;
	for (--year;year >= EPOCH_YEAR;--year)
		tv.tv_sec += isleap(year) ? DAYS_PER_LYEAR : DAYS_PER_NYEAR;
	while (--month)
		tv.tv_sec += dmsize[month];
	tv.tv_sec += day - 1;
	tv.tv_sec = HOURS_PER_DAY * tv.tv_sec + hour;
	tv.tv_sec = MINS_PER_HOUR * tv.tv_sec + mins;
	tv.tv_sec = SECS_PER_MIN * tv.tv_sec + secs;
	return(0);
}
