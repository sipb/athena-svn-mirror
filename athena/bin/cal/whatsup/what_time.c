/************************************************************************/
/*	
/*			what_time.c
/*	
/*	Whatsup routines dealing with times and dates.
/*	
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_time.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_time.c,v 1.1 1992-11-08 19:03:21 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_time_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_time.c,v 1.1 1992-11-08 19:03:21 probe Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include "whatsup.h"


/*
 *			time_initialize()
 *
 *	Find today's date and time.  Start now and 
 *	and go for 3 hours thereafter.
 */

int
time_initialize()
{
 	struct tm *tm;
	long clock;
	long time();

	(void) time(&clock);

	tm = localtime(&clock);         /* get time from Unix(tm) */

       /*
        * Set up today's date and time for a start
        */
	i_field[START_DATE_P].value = ((tm->tm_year%100)*10000) + 
	  				((tm->tm_mon+1)*100) + tm->tm_mday;
	i_field[START_TIME_P].value = 100*tm->tm_hour+tm->tm_min;
	
       /*
        * Set the ending time and date to 0, then tell the system routines
        * to guess it.
        */
	i_field[END_DATE_P].value = 0;
	i_field[END_TIME_P].value = 0;

	guess_end_from_start(i_field[START_DATE_P].value, 
			     i_field[START_TIME_P].value, 
			     &i_field[END_DATE_P].value, 
			     &i_field[END_TIME_P].value,
			     TRUE /* force rest of day */);

	day_from_date(i_field[START_DATE_P].value, 
		      &i_field[START_DAY_P].value);
	today = i_field[START_DAY_P].value;	/* day of week*/
	this_month = tm->tm_mon+1;
	this_year = tm->tm_year % 100;
	this_day = tm->tm_mday;              	/* day in month */
	day_from_date(i_field[END_DATE_P].value, 
		      &i_field[END_DAY_P].value); 


	
}


/*
 *			guess_end_from_start
 *
 *	Given a starting date and time, come up with a reasonable
 *	guess for the ending date and time someone would want
 *	to search.  This routine works in two modes, and should really
 *  	be split into two routines.  With rest_of_day set, the time
 * 	forced to be 23:59.  Otherwise, the time is left alone.
 *
 *	WARNING: this algorithm, and all the time representations in
 *	this program, are lousy.  They are designed for debuggability.
 *      No other redeeming features.
 */
int
guess_end_from_start(sdate, stime, edate, etime, rest_of_day)
int sdate;
int stime;
int *edate;
int *etime;
int rest_of_day;				/* if TRUE, force end */
						/* to be rest of today */
{
	register struct t_struct *tp = &t_struct;
	int old_etime = *etime;

       /*
        * If the end date and time already look reasonable, then 
        * just return
        */
	if (sdate < *edate)
		return;
	if (sdate == *edate && stime <= *etime)
		return;
       /*
        * Looks like we've got to fix it
        */
	set_up_time(tp, sdate, stime);  /* break out dates and times in tp */
	tp->hour=23;
	tp->minute=59;

	re_normalize_time(tp);
	re_pack_time(tp, edate, etime);
	if (!rest_of_day)
		*etime = old_etime;
	return;	
}
/*
 *			guess_end_time_from_start
 *
 *	Given a starting date and time, ending date and time
 *       come up with a reasonable
 *	guess for the ending date and time someone would want
 *	to search.  Right now, we enforce a minimum of one hour.
 * 	less than the start.
 *
 *	WARNING: this algorithm, and all the time representations in
 *	this program, are lousy.  They are designed for debuggability.
 *      No other redeeming features.
 */
int
guess_end_time_from_start(sdate, stime, edate, etime)
int sdate;
int stime;
int *edate;
int *etime;
{
	register struct t_struct *tp = &t_struct;
	register struct t_struct *etp = &t_struct2;

       /*
        * See whether anything needs fixing
        */
	if (*etime -  stime >= 1)
		return;

	set_up_time(tp, sdate, stime);  /* break out dates and times in tp */
	set_up_time(etp, *edate, *etime);/* break out dates and times in tp */
	tp->hour += time_window;
	if (tp->hour>=24) {
		tp->hour=23;
		tp->minute=59;
	}
	re_normalize_time(tp);
       /*
        * The whole reason we're using this routine is that we want
        * to hold on to the ending date.
        */
	tp->year = etp->year;
	tp->month = etp->month;
	tp->day = etp->day;
	
	re_pack_time(tp, edate, etime);
	return;	
}
/*
 *			guess_start_from_end
 *
 *	Given an ending date and time, come up with a reasonable
 *	guess for the start date and time someone would want
 *	to search.  Right now, we only change things if the end is
 *      before the start.
 *	WARNING: this algorithm, and all the time representations in
 *	this program, are lousy.  They are designed for debuggability.
 *      No other redeeming features.
 */
int
guess_start_from_end(edate, etime, sdate, stime)
int edate;
int etime;
int *sdate;
int *stime;
{

	register struct t_struct *tp = &t_struct;
	register struct t_struct *stp = &t_struct2;

	/*
	 * If end time exceeds start, then everything's OK
	 * already.
	 */
	if (*sdate <= edate && *stime <= etime) {
	  	return;

	}
	
	/*
	 * Break out start and end date and time
	 */
	set_up_time(tp, edate, etime);
	set_up_time(stp, *sdate, *stime);

	if (*stime >etime) {
		stp->hour = tp->hour - time_window; /* guess a few hours */
						    /* before end */
		stp->minute = tp->minute;
		if (stp->hour < 0) {		/* but never wrap days */
			stp->hour = 0;
			stp->minute = 0;
		}
	}

	if (*sdate >edate) {
		stp->year = tp->year;
		stp->month = tp->month;
		stp->day = tp->day;
	}
	re_normalize_time(stp);
	re_pack_time(stp, sdate, stime);
	Message("Resetting start date and/or time.");
	return;	
}

/*
 *			set_up_time
 *
 *	Given a starting date and starting time as normally
 *	encoded field values, set up a unified time structure.
 */
int
set_up_time(tp, date, Time)
struct t_struct *tp;
int date;
int Time;
{
	tp->year = date / 10000;
	tp->month = (date%10000) /100;
	tp->day = date % 100;

	tp->hour = Time / 100;
	tp->minute = Time % 100;
}

/*
 *			re_pack_time
 *
 *	Given a normalized time structure, represent it as field
 *	values
 */
int
re_pack_time(tp, date, Time)
struct t_struct *tp;
int	*date;
int     *Time;
{
	*date = 10000*tp->year + 100*tp->month + tp->day;
	*Time = 100*tp->hour + tp->minute;
}

/*
 *			re_normalize_time
 *
 *	Given a time structure containing numbers of days, hours, etc.
 *      Normalize for no more than 24 hours in a day, and so on.
 *
 *	This routine also handles the special case where the number
 *      of days is negative by a small amount.  
 */
#define T_FLD_RENORM(f1,f2, m, d) {while(tp->f1 >m) {tp->f1-=d;tp->f2++;}}
int
re_normalize_time(tp)
struct t_struct *tp;
{
	int negdays;

	if (tp->day < 1) {
		negdays = tp->day;
		tp->day = 1;
		re_normalize_time(tp);     /* normalize for first of 
					      current month */
	        if (tp->month ==1) {
			tp->year--;
			tp->month = 12;
			tp->day = 31 + negdays;
		} else {
			tp->month--;
			tp->day = days_in_month(tp->month, tp->year) + negdays;
		}
	}

	T_FLD_RENORM(minute,hour,59,60)
	T_FLD_RENORM(hour,day,23,24)
	T_FLD_RENORM(day,month,((tp->month<=12)?days_in_month(tp->month, tp->year):100 /* a large number so we won't keep going when month reaches 13 */),
		     days_in_month(tp->month,tp->year))
	T_FLD_RENORM(month, year, 12, 12)
	tp->year %= 100;
}

/*
 *			days_in_month
 *
 *	Return the number of days in a month
 */
int daysinmonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int
days_in_month(month, year)
int month;
int year;
{
	if (month ==2 && year % 4 ==0)		/* wrong at century */
		return 29;

	return daysinmonth[month-1];
}

/*
 *			day_from_date
 *
 *	Return the number of a day given its date (in yymmdd)
 */
int daystomonth[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define DAYSIN4YEARS (365*4 + 1)
int
day_from_date(date, rday)
int date;
int *rday;
{
	register int relday;

        register int year = date / 10000 -80;    /* years since 1980 */
	register int month = (date /100) % 100;
	int day = date % 100;

	if (year < 0)
		year += 100;                     /* from 1980 to 2079 */

	relday = (year / 4) * DAYSIN4YEARS;

	year %= 4;

	/*
	 * We've expressed to within the last 4 years in days, and we
 	 * are now looking at the leap-year prior to the current date.
	 */
	if (year != 0 || month > 2)               /* after leap-day */
	        relday++;

	relday += year * 365 + daystomonth[month-1] + day -1;

	/*
	 * relday is 0 origin number of days since Jan. 1, 1980.
	 * That day is a Tuesday, which is index number 2 in our table
	 */

	*rday = ((relday + 2)%7);

	return;
	
}


