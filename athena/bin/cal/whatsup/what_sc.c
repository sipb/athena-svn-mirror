/************************************************************************/
/*
/*			what_sc.c
/*
/*	Whatsup routines dealing with screen update, especially for
/*	main panel.
/*
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sc.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sc.c,v 1.1 1993-10-12 05:34:57 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*
/************************************************************************/

#ifndef lint
static char rcsid_what_sc_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sc.c,v 1.1 1993-10-12 05:34:57 probe Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <stdio.h>
#include "whatsup.h"

char *main_help_text[] = {
/**"",
"          Select a range of times and a range of dates,",
"          then use '*' to look up events in the calendar.",
"",
"     ==============================================================",
**/
"",
"                 KEY          PURPOSE",
"                 ---          -------",
"",
"                 TAB          Move to next field",
"                 RETURN       Field on new line",
"                 ARROWS       Field in direction of arrow",
"",
"                 *            Start the search for events",
"",
	    };

struct help_dat main_help_dat = {main_help_text, sizeof(main_help_text)/sizeof(char *)};

#define HELP_LINES min(LINES-HELP_LINE, sizeof(main_help_text)/sizeof(char *))


/*
 *			initialize_screen
 */
int
initialize_screen()
{
	pre_read_char = '\0';
	current_field = START_TIME_P;         /* put the cursor on the
					    starting time prompt */
	help_window = subwin(stdscr, HELP_LINES+2 , COLS-1, HELP_LINE, 0);
}


/*
 *			RebuildScreen
 *
 *	Called when screen is messed up, e.g. by system messages or
 *	talks, to get the screen cleaned up.
 */
int
RebuildScreen()
{
	make_dirty();
	clearok(curscr, TRUE);
	repaint();
}

/*
 *			repaint
 *
 *      Call to do a total refresh on the screen
 */
int
repaint()
{
	if (dirty_bits & MAIN_DIRTY) {
		main_repaint();
/*		help_repaint(&main_help_dat); */
	}
	if (dirty_bits & INPUT_DIRTY)
		input_repaint();
	if (dirty_bits & PLACE_DIRTY)
		place_repaint();
	if (dirty_bits & EVENT_DIRTY)
		event_repaint();
	draw_boxes();
	dirty_bits = 0;
	GO_TO_FIELD(i_field[current_field]);
	refresh();
}

/*
 *			main_repaint
 *
 *	Put up all the invariant boilerplate
 */
int
main_repaint()
{
	register int i;

	/*
	 * Write each field onto the screen
	 */
	for (i=0; i<vNOFIELDS; i++) {
		move(o_field[i].y, o_field[i].x);
		if (o_field[i].hilit) 
			standout();
		addstr(o_field[i].text);
		if (o_field[i].hilit) 
			standend();
	}
}
/*
 *			help_repaint
 *
 *	Put up all the invariant boilerplate
 */
#define HELP_BANNER " HINTS ON USING THE WHATSUP CALENDAR "
int
help_repaint(help_data)
struct help_dat *help_data;
{
	register int i;
	int pos;

	(void) wclear(help_window);

	for (i=0; i<help_data->count; i++) {
		(void) wmove(help_window, i+1, HELP_COL);
		(void) waddstr(help_window, help_data->text[i]);
	}
	box(help_window, '?', '?');
	pos = (80-sizeof(HELP_BANNER))/2;
	(void) wmove(help_window, 0, pos);
	(void) waddstr(help_window, HELP_BANNER);
}

/*
 *			input_repaint
 *
 *	Repaint time input fields--may generalize to repainting
 *      all input fields.
 */
int
input_repaint()
{
	register int i;

	for (i=0; i<NIFIELDS; i++) {
		switch (i_field[i].type) {
	    case SDAY:
	    case EDAY:
		    day_repaint(&i_field[i]);
		    break;
	    case SDATE:
	    case EDATE:
		    date_repaint(&i_field[i]);
		    break;
	    case STIME:
	    case ETIME:
		    time_repaint(&i_field[i]);
		    break;
	    case TYPE:
		    type_repaint(&i_field[i]);
		    break;
		}
	}
}

/*
 *			day_repaint
 *
 *	Display a value of type day.
 */
char *days[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

int
day_repaint(ipa)
struct ifl_def *ipa;
{
	register struct ifl_def *ip = ipa;   /* register copy of arg */

	GO_TO_FIELD((*ip));			/* move to the field */
	addstr(days[ip->value]);
}
/*
 *			type_repaint
 *
 *	Display a value of type type.
 */

int
type_repaint(ipa)
struct ifl_def *ipa;
{
	register struct ifl_def *ip = ipa;   /* register copy of arg */
	register char *cp = (char *)(ip->value);
	int len = ip->len;

	GO_TO_FIELD((*ip));			/* move to the field */

	if (cp != NULL)
		while (*cp != '\0') {
			addch(*cp++);
			len--;
		}

	while (len--)
		addch(' ');
}

/*
 *			date_repaint
 *
 *	Display a value of type date.
 */
int
date_repaint(ip)
struct ifl_def *ip;
{
	register int date = ip->value;       /* year/month/day 2 digits each */
	register int month = (date /100) % 100; /* month, 2 digit */
	register int day = date % 100; /* day of the month */

	GO_TO_FIELD((*ip));			/* move to the field */

	(void) wprintw(stdscr,"%2d", month);
	addch('/');
	add2digs(day);

}

/*
 *			time_repaint
 *
 *	Display a value of type time.
 */
int
time_repaint(ip)
struct ifl_def *ip;
{
	register int t = ip->value;		/* mins past midnite */
	register int hour = t / 100;             /* hours past midnight */
	register int mins = t % 100;		/* mins into hour */

	char ap;				/* AM or PM */
	GO_TO_FIELD((*ip));			/* move to the field */

	if (hour == 0 && mins == 0)
		ap = 'M';
	else if (hour == 12 && mins ==0)
		ap = 'N';
	else if (hour < 12)
	 	ap = 'A';
	else {
		ap = 'P';
		if (hour>12)
			hour -= 12;
	}
	(void) printw("%2d:%02d",hour,mins);
	addch(ap);
}

/*
 *			add2digs
 *
 *	Print a two digit number without supressing leading
 *      zero
 */
char nums[] = "0123456789";

int
add2digs(n)
int n;
{
	addch(nums[n/10]);
	addch(nums[n%10]);
}

/*
 *			place_repaint
 *
 *	Repaint place input fields
 */
int
place_repaint()
{
}

/*
 *			event_repaint
 *
 *	Repaint event input fields
 */
int
event_repaint()
{
}
/*
 *			make_dirty
 *
 *      Call to force refresh of all screen fields
 */
int
make_dirty()
{
	dirty_bits = ~0;		/* turn on all dirty bits */

}

