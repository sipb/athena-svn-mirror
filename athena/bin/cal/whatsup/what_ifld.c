/************************************************************************/
/*	
/*			what_ifld.c
/*	
/*	Prompting for various types of input fields for whatsup.c.
/*	
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_ifld.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_ifld.c,v 1.1 1993-10-12 05:34:53 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_ifld_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_ifld.c,v 1.1 1993-10-12 05:34:53 probe Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <stdio.h>
#include <ctype.h>
#include "whatsup.h"

char date_prompt_main_msg[] ="Enter MM/DD or MMDD for Mon/Day, or just DD. End with RETURN. ESC to undo.";
char time_prompt_main_msg[] ="HH:MM, or just HH. Use 24hour time or A/P. End with RETURN. ESC to undo.";
char type_prompt_main_msg[] ="Fill in specific type of event. This is optional! RETURN at end. ESC=undo.";


/*
 *			FillField
 *
 *	Called via psm table to fill in a prompt field
 */
int
FillField(field, typed_char, arg)
int	field;				/* where we were */
char	typed_char;			/* char typed to cause fill */
char	*arg;				/* arg from psm table */
{
	MsgClear();

	switch (i_field[field].type) {
      case SDAY:
      case EDAY:
		FillDayField(&i_field[field],typed_char);
		break;
      case SDATE:
      case EDATE:
		FillDateField(&i_field[field],typed_char);
		break;
      case STIME:
      case ETIME:
		FillTimeField(&i_field[field],typed_char);
		break;
      case TYPE:
		PendingMessage(type_prompt_main_msg);
		FillStringField(&i_field[field], typed_char);
		PendingClear();
		MsgClear();
	}		
	pre_read_char = '\0';		/* no pending input */

	/*
	 * If nothing was changed, then just return
	 */
	if (i_field[field].value == i_field[field].old_value)
		return;

       /*
        * If it's a string field, then there are no time dependencies
        */
	if (i_field[field].type == TYPE)
		return;
	/*
	 * A day, date, or time input field has been updated.
	 * When this happens, we may have to update other fields.
	 */
	FixOtherTimeFields(field);
}
/*
 *			FixOtherTimeFields
 *
 *	The indicated field has been affected by a keyboard update. 
 *	Handle any dependencies.
 */
#define IUPDATE(f, val) {i_field[f].old_value = i_field[f].value; i_field[f].value = val;}
int
FixOtherTimeFields(field)
int field;
{
	register unsigned int dirty = 1<<field;		/* bit mask of changed fields*/
	register unsigned int clean = (~((~0)<<NIFIELDS)) ^ dirty; 
						/* bit msk of 
						   unchanged fields*/
	int	day_diff;			/* used to calculate 
						   effective change in
						   day number when day
						   field is updated. The
						   week always begins 
						   today */
	register int i; 


	int cf;					/* index of clean field */
	int df;					/* index of dirty field */

	int Done = FALSE;



	/*
	 * i_dep encodes the dependency graph for a constraint machine
	 * controlling interactions among the time and date prompt
	 * fields.  The following while loop runs as long as further
	 * constraints remain to be acted upon.  
	 *
	 * A constraint is activated when a constraining field is dirty,
	 * i.e. its value is changing, and a corresponding constrained
	 * field is clean.  The inner for loop looks for one such
	 * occurrence.  Note that this machine is not fully general,
	 * since it presumes that each fields value may change only
	 * once.
	 */
	while (!Done) {
		Done = TRUE;
		
		for (i=0; i<NDEP; i++) {
			if (((1<<i_dep[i][0]) & dirty) &&
			    ((1<<i_dep[i][1]) & clean))
				break;
	        }

		if (i == NDEP)
			break;

		/*
		 * We've found an unmatched constraint.
		 * The following sections must be maintained in 
		 * conjunction with i_dep
		 */
		df = i_dep[i][0];	/* index of dirty field */
		cf = i_dep[i][1];	/*   "    " clean   " */

		if (cf == END_DATE_P && df == START_DATE_P) {
			i_field[cf].old_value = i_field[cf].value;
			guess_end_from_start(i_field[START_DATE_P].value, 
					     i_field[START_TIME_P].value, 
					     &i_field[END_DATE_P].value, 
					     &i_field[END_TIME_P].value,
					     FALSE);

			/*
			 * If  changing the time wrapped to a new date,
			 * mark that dirty.  It will affect the day.
			 */
			if ((cf != END_DATE_P) &&
			    (i_field[END_DATE_P].value != 
			     i_field[END_DATE_P].old_value)) {
				dirty |= 1<<END_DATE_P;
				clean &= ~(1<<END_DATE_P);
			}				  
			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			Done = FALSE;
			continue;
		}
		if (cf == END_TIME_P && df == START_TIME_P) {
			i_field[cf].old_value = i_field[cf].value;
			guess_end_time_from_start(i_field[START_DATE_P].value, 
					     i_field[START_TIME_P].value, 
					     &i_field[END_DATE_P].value, 
					     &i_field[END_TIME_P].value);

			/*
			 * If  changing the time wrapped to a new date,
			 * mark that dirty.  It will affect the day.
			 */
			if ((i_field[END_DATE_P].value != 
			     i_field[END_DATE_P].old_value)) {
				dirty |= 1<<END_DATE_P;
				clean &= ~(1<<END_DATE_P);
			}				  
			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			Done = FALSE;
			continue;
		}
		if ((cf == START_TIME_P && df == END_TIME_P) ||
		    (cf == START_DATE_P && df == END_DATE_P)) {
			i_field[cf].old_value = i_field[cf].value;
			i_field[START_DATE_P].old_value = 
			  		i_field[START_DATE_P].value;
						/* date can always change */
			guess_start_from_end(i_field[END_DATE_P].value, 
					     i_field[END_TIME_P].value, 
					     &i_field[START_DATE_P].value, 
					     &i_field[START_TIME_P].value);

			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			/*
			 * If  changing the time wrapped to a new date,
			 * mark that dirty.  It will affect the day.
			 */
			if ((cf != START_DATE_P) &&
			    (i_field[START_DATE_P].value != 
			     i_field[START_DATE_P].old_value)) {
				dirty |= 1<<START_DATE_P;
				clean &= ~(1<<START_DATE_P);
			}				  

			Done = FALSE;
			continue;
		}
		if (cf == START_DATE_P  && df == START_DAY_P) {
			i_field[cf].old_value = i_field[cf].value;
			/*
			 * Calculate number of days from today to
			 * desired date.  All days of week are interpreted
			 * to be in the future.
			 */
			day_diff= i_field[df].value - today;
			if (day_diff < 0)
				day_diff+=7;
			/*
			 * Get time and date from screen into tstruct 
			 */
			set_up_time(&t_struct, i_field[cf].value,
				    i_field[START_TIME_P].value);
			/*
			 * Set t_struct to current day adjusted for typed
			 * date, retaining time from screen
			 */
			t_struct.year = this_year;
			t_struct.month = this_month;
			t_struct.day = this_day + day_diff;
/* what's this??			i_field[cf].value += day_diff;  */
			/*
			 * day calculation may have given negative day.
			 * re-normalize and store as current
			 */			
			re_normalize_time(&t_struct);
			re_pack_time(&t_struct, &i_field[cf].value,
				    &i_field[START_TIME_P].value);
			
			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			Done = FALSE;
			continue;
		}
		if (cf == END_DATE_P  && df == END_DAY_P) {
			i_field[cf].old_value = i_field[cf].value;
			/*
			 * Ending days are always assumed to be in the 
			 * week following the start day. 
			 */
			day_diff= i_field[df].value - i_field[START_DAY_P].value;
			if (day_diff<0)
				day_diff+=7;
/* what's this ??	if (i_field[df].old_value < today)
				day_diff-=7;  */
			i_field[cf].value = i_field[START_DATE_P].value + 
			  		    day_diff;
			/*
			 * day calculation may have given negative day.
			 * re-normalize and store as current
			 */			
			set_up_time(&t_struct, i_field[cf].value,
				    i_field[END_TIME_P].value);
			re_normalize_time(&t_struct);
			re_pack_time(&t_struct, &i_field[cf].value,
				    &i_field[END_TIME_P].value);
			
			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			Done = FALSE;
			continue;
		}
		if ((cf == START_DAY_P  && df == START_DATE_P) ||
		    (cf == END_DAY_P && df == END_DATE_P )) {
			i_field[cf].old_value = i_field[cf].value;
			day_from_date(i_field[df].value,&i_field[cf].value);
			dirty |= 1<<cf;
			clean &= ~(1<<cf);
			Done = FALSE;
			continue;
		}		
	}		
	
}
#undef IUPDATE

/*
 *			FillDayField
 *
 *	Called when a non-command character has been typed in an
 *	input field of type Day.
 */
int
FillDayField(ipa, c)
struct ifl_def *ipa;
char c;
{
	register struct ifl_def *ip = ipa;

	ip->old_value = ip->value;      /* in case we want to back out
					   *** May be unnecessary */
	UPCASE(c);

	switch (c) {
    case 'S':				/* Sun or Sat */
		OneOfTwoDays(ip, 'S', 'U', 0, "SUN", 'A', 6, "SAT"); 
		break;
    case 'M':
		ip->value = 1;
		break;
    case 'T':
		OneOfTwoDays(ip, 'T', 'U', 2, "TUE", 'H', 4, "THU"); 
		break;
    case 'W':
		ip->value = 3;
		break;
    case 'F':
 		ip->value = 5;
		break;
    default:
		OuterError("Enter S M T W T F S to choose day.");
	}

	dirty_bits |= INPUT_DIRTY;

	
}
/*
 *			FillStringField
 *
 *	Called when a non-command character has been typed in an
 *	input field of type String.
 */
int
FillStringField(ipa, cdat)
struct ifl_def *ipa;
char cdat;
{
	register char c = cdat;
	register struct ifl_def *ip = ipa;
	register char *cp, *sp;
	char *ep;
	int row, col;
	int i;
	char *malloc();

	if ((char *)(ip->old_value) != NULL)
		free((char *)ip->old_value);
	
	ip->old_value = ip->value;      /* in case we want to back out
					   *** May be unnecessary */

/*NOSTRICT*/
	ip->value = (int)malloc( (unsigned) ip->len);

	sp = cp = (char *)ip->value;
	ep = sp + ip->len -1;			/* char past end */

	MsgClear();

	standout();
       /*
        * Blacken the input area then back up to start
        */
	for (i=0; i<ip->len-1; i++)
		addch(' ');
	refresh();
	GO_TO_FIELD(*ip);
       /*
        * Read the input and echo it, gathering the string
        */
	do {
		if (isupper(c))
			c=tolower(c);
               /*
                * Handle backspacing
                */
		if (c == BS || c == DEL) {
			getyx( stdscr, row, col);
			if (cp != sp) {
				move (row, col-1);
				addch(' ');
				move (row, col-1);
				cp--;
			} else {
				(void) putchar(BELL);
			}
			refresh();
			continue;
		}
		/*
		 * Handle a '?' for help
		 */
		else if (c == '?') {
			GetHelp(0,0,6);		/* 6 is for type help */
			continue;		       
               /*
                * Check for escape to restore old value
                */
		} else if (c == ESC) {
			free(sp);
			ip->value = ip->old_value;
			ip->old_value = 0;	/* avoid duplicate free later*/
			goto str_done;
		}
               /*
                * Not a backspace, make sure it fits
                */
		if (cp < ep) {
			addch(c);
			*cp++ = c;
		} else {
			(void) putchar(BELL);
		}
		refresh();
	} while ((c=getch()) != RETURN  && c != '\t');
	*cp = '\0';
	if (c == '\t')
		pre_read_char = c;
str_done:
	standend();

	dirty_bits |= INPUT_DIRTY;

	
}

/*
 *			OneOfTwoDays
 *
 *	Based on the next input char, pick one of two days.
 *	Assumes cursor already at start of field.
 */
int
OneOfTwoDays(ipa, first_char, c1, val1, prompt1, c2, val2, prompt2)
struct ifl_def *ipa;
char	first_char;		/* char already typed */
char	c1;			/* type this char to choose day 1 */
int 	val1;			/* if day1 is chosen ,this is the value */
char    *prompt1;		/* 3 letter name of day1 as string */
char	c2;			/* type this char to choose day 1 */
int 	val2;			/* if day1 is chosen ,this is the value */
char    *prompt2;		/* 3 letter name of day1 as string */
{
	register struct ifl_def *ip = ipa;
	register char c;
	char buffer[80];

	/*
	 * Highlight the field and show first char
	 */
	standout();
	addch(first_char);
	addch(' ');
	addch(' ');
	standend();
	GO_TO_FIELD_OFFSET((*ip), 3);

	/*
	 * Set up prompt
	 */
	(void) sprintf(buffer, "Enter %c for %s, %c for %s, or <ESC> to leave day unchanged.", c1, prompt1, c2, prompt2);

	/*
	 * Loop until meaningful input is received
	 */
	while (TRUE) {
		Message(buffer);		/* tell user what to do */
		c = getch();			/* read a char */
		while (c=='?') {
			GetHelp(0,0,5);
			c = getch();
		}
		UPCASE(c);

		if (c == c1) {
			ip->value = val1;
			break;
		}
		if (c == c2) {
			ip->value = val2;
			break;
		}
		if (c == ESC) {
			ip->value = ip->old_value;
			break;
		}
		(void) putchar(BELL);
	}

	Message("");			/* clear the message area */
	return;
}


/*
 *			FillDateField
 *
 *	Called when a non-command character has been typed in an
 *	input field of type Day.
 */

int
FillDateField(ipa, c)
struct ifl_def *ipa;
char c;
{
	register struct ifl_def *ip = ipa;
	register int off;
	int slash = (-1);		/* no slash parsed */
	int day;
	int month;
	int year;
	int ok;

	char typed[5];			/* data as typed */
	char buffer[80];

	ip->old_value = ip->value;      /* in case we want to back out
					   *** May be unnecessary */
	if (((c<'0') || (c>'9')) && (c!='/')) {
		OuterError("Start with a digit (0-9) or a / to enter date.");
		return;
	}
	/*
	 * Break the updated field into its components
	 */
	set_up_time(&t_struct, ip->value, i_field[ip->relative].value); 

	/*
         * Set up to prompt for rest of field
         */
	off = 0;			/* position of next char */
	month = (-1);			/* in case only day is spec'd */

	/*
	 * Put first char in buffer and paint screen (Message will refresh)
	 */
	typed[off++] = c;

	PendingMessage(date_prompt_main_msg);
	show_partial_date(ip, typed, off);
	refresh();

	/*
	 * Set up prompt
	 */
	do {
		c = getch();
		MsgClear();

		/*
		 * Check for ESC
		 */
		if (c == ESC){
			ip -> value = ip->old_value;
			dirty_bits |= INPUT_DIRTY;
			PendingClear();
			MsgClear();
			return;
		}

		/*
		 * Handle a backspace
		 */
		if (c == BS || c == DEL) {
		        if (off==0){
				ok = FALSE;
				continue;
			}
			if (typed[off-1] == '/')
				slash = (-1);  /* backing up over slash */
			off--;
		/*
		 * Handle a '?' for help
		 */
		} else if (c == '?') {
			GetHelp(0,0,4);		/* 4 is for time help */
			continue;		       
		} else if (off == 5 && c != RETURN && c!='\t') {
			Error("ERROR: Use RETURN to complete, BACKSPACE to correct, ESC to undo.");
			ok = FALSE;
			continue;		       
		/*
		 * A slash must be correctly placed
		 */
		} else if (c == '/') {
			if (off > 2 || slash >=0) {
				Error(date_prompt_main_msg);
				ok = FALSE;
				continue;
			}
			slash = off;
			typed[off++] = c;
		}
		/*
		 * Make it's either a digit or return
		 */
		else if ((c < '0' || c > '9') && c != RETURN && c!= '\t') {
			Error(date_prompt_main_msg);
			ok = FALSE;
			continue;
		} else if (c != RETURN && c != '\t') {      /* it's a digit */
			typed[off++] = c;
		}		
		/*
		 * Anything other than a RETURN has been appended
		 * to the typed buffer, and / is known to
		 * be only before position 2, with no multiple
		 * occurrences. Check the buffer and parse it.
		 */
		show_partial_date(ip, typed, off);
		if (off!=3 || c==RETURN || slash==2)
			parse_date(typed, off, slash, &t_struct, &month, &day, &year, &ok);
		else
			ok = TRUE;
	        refresh();
	} while ((c != RETURN && c != ESC && c!= '\t') || ok == FALSE);

	/*
	 * If terminated with a return or tab, update the field
	 */
	PendingClear();
	if ((c==RETURN || c=='\t') && ok) {
		t_struct.month = month;
		t_struct.day = day;
		t_struct.year = year;
		re_pack_time(&t_struct, &ip->value, &i_field[ip->relative].value);
		if  (year != this_year) {
			(void) sprintf(buffer,"Date presumed to be in 19%02d", year);
			OuterMessage(buffer);
		} else
			MsgClear();
				
	} else
		MsgClear();

	if (c=='\t')
		pre_read_char = c;
	dirty_bits |= INPUT_DIRTY;
}
/*
 *			show_partial_date
 *
 *	Put up the date field in reverse video
 */
int
show_partial_date(ip, typed, n)
struct ifl_def *ip;
char typed[];
int n;
{
	register int i=5;
	
	GO_TO_FIELD((*ip));
	standout();
	while (n--) {
		addch((*typed++));
		i--;
	}

	while (i--)
		addch(' ');
	standend();
	GO_TO_FIELD_OFFSET((*ip), 5);
}

/*
 *			parse_date
 *
 *	Check a typed in date string, make sure it looks plausible,
 *      and parse it as a date.
 */
int
parse_date(typed, n, slash, tp, month, day, year, ok)
char typed[];				/* chars as typed */
int n;					/* number of chars to parse */
int slash;				/* offset to s slash, if any */
struct t_struct *tp;			/* previous values of this field,
					   used for defaulting */
int *month, *day, *year;		/* returned parse */
int *ok;				/* returned true iff legal */
{
	int m, d, y;                    /* working values of month,
					   day, and year */
	register int i = 0;		/* next char to parse */
	char buffer[80];		/* error messages built here */

	/*
	 * If typed field is empty (probably because someone BS'd over it),
	 * just return
	 */
	if (n == 0) {
		*ok = FALSE;
		return;
	}
	/*
	 * Set up defaults in working values
	 */
	y = this_year;			/* real current year */
	m = tp->month;			/* last specified value of month */
	d = tp->day;
	
	/*
	 * Parse the supplied numbers - first, the case where a month
	 * is specified.  This can be only if the length is 4 (MMDD)
	 * or a slash exists somewhere to tell where to break month
	 * and day.
	 */
	if (slash > 0 || n ==4) {       /* there's a month */
		m = typed[i++]-'0';
		if (i<n && typed[i] != '/')
			m = m*10 + typed[i]-'0';
		i++;
		if (i<n && typed[i] == '/')
			i++;
		goto parseday;
	/*
	 * A leading slash just causes us to parse the rest as
 	 * a day.  Skip it.
	 */
	} else if (slash == 0) {
		i++;
		goto parseday;
	} else {			/* there's no slash and there are
					   less than 4 digits.  There's no
					   legal month specification */
		if (n==3) {
			Error("ERROR: illegal date specification, try MM/DD, MM, DD or <ESC> to undo.");
			*ok = FALSE;
			return;
		}
	}
	/*
	 * Parse the day
	 */
parseday:
	if (i<n)
		d = 0;
	while (i<n) {
		d = d*10 + typed[i++] - '0';
	}
		
	/*
	 * Check the values
	 */
	if (m<1 || m>12) {
		Error("ERROR: Month must be between 1 and 12");
		*ok = FALSE;
		return;
	}
	if ((m < this_month) || (m == this_month && d < this_day)) {
		y++;			/* covers 12 months starting with
					   current month */
	}
	if (d<1 || d > days_in_month(m,y)) {
		(void) sprintf(buffer, "ERROR: days must be between 1 and %d for selected month", days_in_month(m,y));
		Error(buffer);
		*ok = FALSE;
		return;
	}
	/*
	 * Looks ok, update the values and return
	 */
	*month = m;
	*day = d;
	*year = y;
	*ok = TRUE;
}
/*
 *			FillTimeField
 *
 *	Called when a non-command character has been typed in an
 *	input field of type Time.
 */

int
FillTimeField(ipa, c)
struct ifl_def *ipa;
char c;
{
	register struct ifl_def *ip = ipa;
	register int off;
	int colon = (-1);		/* no colon parsed */
	int hour;
	int minute;
	int ok;

	char typed[6];			/* data as typed */

	ip->old_value = ip->value;      /* in case we want to back out
					   *** May be unnecessary */
       /*
        * Handle noon and midnight
        */
	if (c=='N'||c=='n') {
		ip->value = 1200;
	}
	if (c=='M'||c=='m') {
		ip->value = 0;
	}
	if (((c<'0') || (c>'9')) && (c!=':')) {
		OuterError("Start with a digit (0-9) or a : to enter time.");
		return;
	}
	/*
	 * Break the updated field into its components
	 */
	set_up_time(&t_struct, i_field[ip->relative].value, ip->value); 

	/*
         * Set up to prompt for rest of field
         */
	off = 0;			/* position of next char */
	minute = (-1);			/* in case only hour is spec'd */

	/*
	 * Put first char in buffer and paint screen (Message will refresh)
	 */
	typed[off++] = c;


	/*
	 * Set up prompt
	 */
	PendingMessage(time_prompt_main_msg);
	show_partial_time(ip, typed, off);
	refresh();
	do {
		c = getch();
		UPCASE(c);
		MsgClear();

		/*
		 * Check for ESC
		 */
		if (c == ESC){
			ip -> value = ip->old_value;
			dirty_bits |= INPUT_DIRTY;
			PendingClear();
			MsgClear();
			return;
		}

		/*
		 * Handle a backspace
		 */
		if (c == BS || c == DEL) {
		        if (off==0){
				ok = FALSE;
				continue;
			}
			if (typed[off-1] == ':')
				colon = (-1);  /* backing up over colon */
			off--;
		} else if (off == 6 && c != RETURN && c!= '\t') {
			Error("ERROR: Use RETURN to complete, BACKSPACE to correct, ESC to undo.");
			ok = FALSE;
			continue;		       
		/*
		 * Handle a '?' for help
		 */
		} else if (c == '?') {
			GetHelp(0,0,3);		/* 4 is for time help */
			continue;		       
		/*
		 * A colon must be correctly placed
		 */
		} else if (c == ':') {
			if (off > 2 || colon >=0) {
				Error(time_prompt_main_msg);
				ok = FALSE;
				continue;
			}
			colon = off;
			typed[off++] = c;
		}
		/*
		 * Make sure it's either a digit or return or tab
		 */
		else if ((c < '0' || c > '9') && c != RETURN && c!= '\t' &&c !='A'
			 && c !='P' && c!='M' && c!='N') {
			Error(time_prompt_main_msg);
			ok = FALSE;
			continue;
		} else if (c != RETURN && c!= '\t') { /* it's a digit, A or P */
			typed[off++] = c;
		}		
		/*
		 * Anything other than a RETURN or tab has been appended
		 * to the typed buffer, and : is known to
		 * be only before position 2, with no multiple
		 * occurrences. Check the buffer and parse it.
		 */
		show_partial_time(ip, typed, off);
		if (c==RETURN || c=='\t' || colon==2 || c=='P' || c=='A' || c=='M' || c=='N')
			parse_time(typed, off, colon, &t_struct, &hour, &minute, &ok);
		else
			ok = TRUE;
	        refresh();
	} while ((c != RETURN && c!= '\t' && c != ESC) || ok == FALSE);

	/*
	 * If terminated with a return, update the field
	 */
	if ((c==RETURN || c=='\t')&& ok) {
		t_struct.hour = hour;
		t_struct.minute = minute;
		re_pack_time(&t_struct, &i_field[ip->relative].value, 
			     &ip->value);		
	}

	if (c=='\t')
		pre_read_char=c;
	dirty_bits |= INPUT_DIRTY;
	PendingClear();
	MsgClear();
}
/*
 *			show_partial_time
 *
 *	Put up the time field in reverse video
 */
int
show_partial_time(ip, typed, n)
struct ifl_def *ip;
char typed[];
int n;
{
	register int i=6;			/* init to width of field */
	
	GO_TO_FIELD((*ip));
	standout();
	while (n--) {
		addch((*typed++));
		i--;
	}

	while (i--)
		addch(' ');
	standend();
	GO_TO_FIELD_OFFSET((*ip), 6);
}

/*
 *			parse_time
 *
 *	Check a typed in time string, make sure it looks plausible,
 *      and parse it as a time.
 */
int
parse_time(typed, n, colon, tp, hour, minute, ok)
char typed[];				/* chars as typed */
int n;					/* number of chars to parse */
int colon;				/* offset to s colon, if any */
struct t_struct *tp;			/* previous values of this field,
					   used for defaulting */
int *hour, *minute;			/* returned parse */
int *ok;				/* returned true iff legal */
{
	int h, m;                       /* working values of hour, min */
	register int i = 0;		/* next char to parse */

	/*
	 * If typed field is empty (probably because someone BS'd over it),
	 * just return
	 */
	if (n == 0) {
		*ok = FALSE;
		return;
	}
	/*
	 * Set up defaults in working values
	 */
	h = tp->hour;			/* last specified value of hour */
	m = tp->minute;
	
	/*
	 * Parse the supplied numbers - first, the case where an hour
	 * is specified.  Only exception is a leading colon.
	 */
	if (colon != 0) {       	/* there's an : */
		m = 0;			/* min defaults to 0 when hour
					   specified */
		h = typed[i++]-'0';
		if (i==n)
		        goto hour_parse_done;
		if (typed[i] == 'P') {
			h += 12;
			i++;
			goto hour_parse_done;
		}
		if (typed[i] == 'A') {
			i++;
			goto hour_parse_done;
		}
		if (typed[i] != ':') {
			h = h*10 + typed[i++]-'0';
			if (i<n && typed[i] == 'M') {
				if (h==12 && m==0 && colon ==(-1)) {
					h=0;
					i++;
					goto all_done;
				} else {
					Error("Correct form is 12M (or just 0) for midnight.  Use BACKSPACE to fix.");
					*ok = FALSE;
					return;
				}
			}
			if (i<n && typed[i] == 'N') {
				if (h==12 && m==0 && colon == (-1)) {
					h=12;
					i++;
					goto all_done;
				} else {
					Error("Use either 12, 12:00, 12N, or 12:00N for noon.  BACKSPACE to fix.");
					*ok = FALSE;
					return;
				}
			}
			if (i<n &&typed[i] == 'P') {
				if (h<12)
					h += 12;
				i++;
				goto hour_parse_done;
			}
			if (i<n && typed[i] == 'A') {
				if (h==12)
				        h=0;
				i++;
				goto hour_parse_done;
			}
			if (i<n && typed[i] != ':') {
				Error("ERROR: a colon must separate hours from minutes.  BACKSPACE to fix.");
				*ok = FALSE;
				return;
			} else
				i++;    /* skip possible colon in position 3 */
		} else
			i++;		/* skip the colon in position 2 */
	/*
	 * A leading colon just causes us to parse the rest as
 	 * a minute.  Skip it.
	 */
	} else 
		i++;			/* skip colon in position 1 */
	/*
	 * Parse the minutes
	 */
hour_parse_done:
	if (i<n)
		m = 0;
	while (i<n) {
		if (typed[i] == 'P') {
			if (h<12)
				h += 12;
			break;
		}
		if (typed[i] == 'A') {
			if (h==12)
				h = 0;
			break;
		}
		if (typed[i] == 'M') {
			if (h==12 && m==0) {
				h=0;
				i++;
				goto all_done;
			} else {
				Error("Correct form is 12M (or just 0) for midnight.  Use BACKSPACE to correct.");
				*ok = FALSE;
				return;
			}
		}
		if (typed[i] == 'N') {
			if (h==12 && m==0) {
				i++;
				goto all_done;
			} else {
				Error("Correct form is  12, 12:00, 12N, or 12:00N for noon. Use BACKSPACE to fix.");
				*ok = FALSE;
				return;
			}
		}
		m = m*10 + typed[i++] - '0';
	}
		
	/*
	 * Check the values
	 */
	if (h>23) {
		Error("ERROR: Hour must be between 0 and 23.  Use BACKSPACE to correct.");
		*ok = FALSE;
		return;
	}
	if (m>59) {
		Error("ERROR: Minute must be between 0 and 59.  Use BACKSPACE to correct.");
		*ok = FALSE;
		return;
	}
all_done:

	/*
	 * Looks ok, update the values and return
	 */
	*hour = h;
	*minute = m;
	*ok = TRUE;
}

