/************************************************************************/
/*      
/*                      postmisc.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmisc.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmisc.c,v 1.1 1992-11-08 19:05:01 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*	Miscellaneous routines used by postit and bulkpost
/*      
/************************************************************************/

#ifndef lint
static char rcsid_postmisc_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmisc.c,v 1.1 1992-11-08 19:05:01 probe Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <curses.h>
#include "gdb.h"
#include "postit.h"

WINDOW *stdscr;					/* just to keep */
						/* the linker happy */
	/*----------------------------------------------------------*/
	/*	
	/*			init_times
	/*	
	/*	Initialize for handline of times.
	/*	
	/*	At the moment, this just makes a global record of 
	/*	today's date and time.
	/*	
	/*----------------------------------------------------------*/

int
init_times()
{
	long clock;
	extern long time();

       /*
        * Get the time of day clock in internal form
        */

	clock = time((long *)0);

       /*
        * Unpack it into a structure
        */
	
	bcopy((char *)localtime(&clock), (char *)&today, sizeof(struct tm));
}

	/*----------------------------------------------------------*/
	/*	
	/*			check_date
	/*	
	/*	Make sure the date on a new entry is plausible.
	/*	Warn for more than 2 months in the future.
	/*	Error for past or more than a year in the future.
	/*	
	/*----------------------------------------------------------*/

int
check_date(year, month, day)
{
       /*
        * Error if in the past
        */
	if (compare_dates(year, month, day, &today) < 0) {
		if (mode != POSTIT)
			line_tag();
		fprintf(stderr,"\nERROR: you cannot post listings for events which were in the past.\nPlease check the date in your listing.\n");
		return FALSE;
	}
       /*
        * Error if more than a year in the future
        */
	if (compare_dates(year-1, month, day, &today) >0) {
		if (mode != POSTIT)
			line_tag();
		fprintf(stderr,"\nERROR: you cannot post listings for events more than a year in the future.\nPlease check the date in your listing.\n");
		return FALSE;
	}
       /*
        * Warning if event is more than 2 months off
        */
	if (compare_dates(year, month-2, day, &today) >0) {
               /*
                * Don't repeat the warning if we're in the update phase of
                * a bulk-post.  User already got warned during checking phase.
                */
		if (mode == BULK_UPDATING)
			return TRUE;
		if (mode != POSTIT)
			line_tag();
		fprintf(stderr,"\nWARNING:  Date is more than two months in the future.\n          Please make sure it is correct.\n\n");
		return TRUE;
	}

       /*
        * Looks OK
        */

	return TRUE;
}

	/*----------------------------------------------------------*/
	/*	
	/*			compare_dates
	/*	
	/*	Compare a given year month and day with the date
	/*	in a tm style structure.  Returns 1 for date is later,
	/*	0 for equal, -1 for earlier.
	/*	
	/*----------------------------------------------------------*/

#define SGN(a,b) ((a)>(b)?1:((a)==(b)?0:-1))
int
compare_dates(year, month, day, tmp)
int year;
int month;
int day;
struct tm *tmp;
{
	register int sgn;			/* sign of a difference */
       /*
        * Check for mismatch in years
        */
	sgn = SGN(year,tmp->tm_year);
	if (sgn)
		return sgn;
       /*
        * Check for mismatch in months
        */
	sgn = SGN(month,tmp->tm_mon+1);
	if (sgn)
		return sgn;
       /*
        * Check for mismatch in years
        */
	sgn = SGN(day,tmp->tm_mday);
	return sgn;
		
}

