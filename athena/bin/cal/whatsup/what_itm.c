/************************************************************************/
/*	
/*			what_itm.c
/*	
/*	Routines to display a selected item in the whatsup package.
/*	
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_itm.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_itm.c,v 1.1 1993-10-12 05:34:55 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/*	
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_itm_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_itm.c,v 1.1 1993-10-12 05:34:55 probe Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <stdio.h>
#include <strings.h>
#include "whatsup.h"

#define BANNER_LINES 2

int quit_item;


/*
 *			DoItem
 *
 *	Called from selection mode when an item has  been selected
 */
int
DoItem(global_state)
struct sel_state *global_state;
{
	struct itm_state state;

	struct p_tr *old_psm = current_psm;	/* remember the proc */
						/* state machine */
	register struct itm_state *sp;		/* pointer to our state */

	int tup_num = global_state->curs_line;
	TUPLE tup;


	current_psm = item_psm;			/* what we call will */
						/* from now on be */
						/* according to selection */
						/* mode when a key is */
						/* pressed */

       /*
        * Set t to the first tuple we'll actually write, if any
        */
	tup = FIRST_TUPLE_IN_RELATION(global_state->rel);


	while (tup != NULL && tup_num>0) {
		tup = NEXT_TUPLE_IN_RELATION(global_state->rel, tup);
		tup_num--;
	}
	
	sp = &state;

       /*
        * put in some plausible values for state
        */
	sp->tup = tup;				/* rell the world which */
						/* tuple we're using */
       /*
        * Create the selection window and bind the tuple data to it.
        */
	create_itm_window(sp);
	sync_itm_window(sp);			/* put initial contents */
						/* in window */
	do_item_interactions(sp);		/* stay here as long */
						/* as we are in selection */
						/* mode */

	destroy_itm_window(sp);
	current_psm = old_psm;
}
	/*----------------------------------------------------------*/
	/*	
	/*			create_itm_window
	/*	
	/*	create the subwindow in which the selections will
	/*	be shown.  Clear it and display the banner.
	/*	
	/*----------------------------------------------------------*/

int
create_itm_window(statep)
struct itm_state *statep;
{
	register struct itm_state *sp = statep;

	sp->w = stdscr;
	move(ITM_LINE, 0);
	clrtobot();
	itm_banner(sp);
}
	/*----------------------------------------------------------*/
	/*	
	/*			sync_itm_window
	/*	
	/*	Synchronize the contents of the item display window
	/*	with the data according to contents.  
	/*	
	/*----------------------------------------------------------*/
int
sync_itm_window(statep)
struct itm_state *statep;
{
	struct itm_state *sp = statep;
	register TUPLE_DESCRIPTOR desc;
	 TUPLE t = sp->tup;
	register int Index;


	desc = DESCRIPTOR_FROM_TUPLE(t);
	/*
	 * Write each field onto the screen
	 */
	Index = field_index(desc, "date");
	i_date_fld(ITM_LINE+3,1,"Date: ",(int *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "uniq");
	i_int_fld(ITM_LINE+3,50,"Event number: ",(int *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "time");
	i_time_fld(ITM_LINE+4,1,"Time: ",(int *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "place");
	i_text_fld(ITM_LINE+6,0,"Place: ",(STRING *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "type");
	i_text_fld(ITM_LINE+6,47,"   Type of event: ", (STRING *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "title");
	i_text_fld(ITM_LINE+7,0,"Title: ",(STRING *)FIELD_FROM_TUPLE(t,Index));
	Index = field_index(desc, "comments");
	i_comments(ITM_LINE+9,0,"Comments: ",(STRING *)FIELD_FROM_TUPLE(t,Index));

	(void) wrefresh(sp->w);
	iput_cursor(sp);
	
}

	/*----------------------------------------------------------*/
	/*	
	/*			format_string
	/*	
	/*	Given a  single contiguous string with
	/*	blanks and \n characters.  Insert a \n at every
	/*	point where the line would be too long for the screen,
	/*	always doing this between words.  
	/*	
	/*	The first line can be forced artificially to be
	/*	shorter than the others.
	/*	
	/*	Also, try to uppercase the first character of each
	/*	sentence.
	/*	
	/*	Note that the string may be shortened by the blank
	/*	removal done at the beginning of lines, so watch
	/*	out for dynamically allocated memory.
	/*	
	/*----------------------------------------------------------*/

int
format_string(sp, first_len, other_len)
char *sp;
int  first_len;					/* length of 1st line */
int  other_len;					/* length of all others */
{
	register char *blank = NULL;		/* ptr to previous blank */
	register char *next = sp;		/* next char to check */
	register int len = 0;			/* length of the current */
						/* line so far */
	int target = first_len;			/* length of line we're */
						/* trying to build */
	int start_sentence = FALSE;		/* Next non-blank wants */
						/* to be uppercased. */
	int flush_blanks = FALSE;		/* true if we're getting */
						/* rid of blanks.  Generally */
						/* at the start of a line */
						/* that we split */

       /*
        * Flush leading blanks
        */
	while (*next==' ')
		next++;
       /*
        * Return if string is null
        */
	if (*next == '\0')
		return;



	UPCASE(*next);				/* first char is upper */

       /*
        * Loop through the whole input string
        */
	while (*(++next) != '\0') {
		len++;
		if (*next == ' ') {
			blank = next; 
			if (flush_blanks) {
				(void) strcpy(next, next+1);
				next--;		/* because loop increments */
						/* it.  UGH */
				continue;
			}
		}
		else {
			flush_blanks = FALSE;
			if (*next == '\n') {
				len = 0;
				continue;
			} else
				if (start_sentence) {
					UPCASE(*next);
					start_sentence = FALSE;
				}
		}
		if (*next == '.') {
			start_sentence = TRUE;
		}
               /*
                * See if we have to try and split line.  If we fail, then
                * just keep going.  Sooner or later we'll hit a blank
                * and split there, which is the best we can do.
                */
		if (len > target) {
			if (blank != NULL) {	/* is there a place for \n */
				*blank = '\n';
				len = next - blank;
				blank = NULL;
				flush_blanks = FALSE;
			} 
			target = other_len;	/* we're not on first */
						/* line anymore */
		}
		
	}
}


	/*----------------------------------------------------------*/
	/*	
	/*			do_comment
	/*	
	/*	Put up the comment which precedes a field and 
	/*	leave the cursor positioned ready for the data.
	/*	
	/*----------------------------------------------------------*/
int
do_comment(y,x, comment)
    int y;
int x;
char *comment;
{
	move(y,x);
	addstr(comment);
	
}

	/*----------------------------------------------------------*/
	/*	
	/*			i_text_fld
	/*	
	/*	Put up a text field in the position indicated
	/*	
	/*----------------------------------------------------------*/

int
i_text_fld(y, x, comment, dp)
int y;
int x;
char *comment;
STRING *dp;
{

	char *cp;
	do_comment(y,x, comment);
	cp=STRING_DATA(*dp);
	if (cp != NULL && *cp != '\0') {
		addstr(cp);
	}
}
	/*----------------------------------------------------------*/
	/*	
	/*			i_int_fld
	/*	
	/*	Put up a int field in the position indicated
	/*	
	/*----------------------------------------------------------*/

int
i_int_fld(y, x, comment, dp)
int y;
int x;
char *comment;
int *dp;
{
	do_comment(y,x, comment);
	(void) wprintw(stdscr, "%d", *dp);
}
	/*----------------------------------------------------------*/
	/*	
	/*			i_comments
	/*	
	/*	Put up a text field in the position indicated
	/*	
	/*----------------------------------------------------------*/

int
i_comments(y, x, comment, dp)
int y;
int x;
char *comment;
STRING *dp;
{
	char *cp;

	do_comment(y,x, comment);
	cp=STRING_DATA(*dp);
	format_string(cp, COLS - 12, COLS-3); /* avoid word wraps */
						   /* on display */
	if (cp != NULL && *cp != '\0') {
		addstr(cp);
	}
}
	/*----------------------------------------------------------*/
	/*	
	/*			i_time_fld
	/*	
	/*	Put up a time field in the position indicated
	/*	
	/*----------------------------------------------------------*/

int
i_time_fld(y, x, comment, dp)
int y;
int x;
char *comment;
int *dp;
{
        struct ifl_def ifl;			/* field descriptor for */
						/* the data */
	int newy, newx;
	do_comment(y,x, comment);
	getyx(stdscr, newy, newx);
	make_ifld(&ifl, newy, newx, TIME_LENG, STIME, *dp);
	time_repaint(&ifl);
}

	/*----------------------------------------------------------*/
	/*	
	/*			i_date_fld
	/*	
	/*	Put up a date field in the position indicated
	/*	
	/*----------------------------------------------------------*/

extern char *days[];
int
i_date_fld(y, x, comment, dp)
int y;
int x;
char *comment;
int *dp;
{
        struct ifl_def ifl;			/* field descriptor for */
						/* the data */
	int daynumber;
	int newy, newx;

	do_comment(y,x, comment);
	getyx(stdscr, newy, newx);
       
	make_ifld(&ifl, newy, newx, DATE_LEN, SDATE, *dp);
	date_repaint(&ifl);

	day_from_date(*dp, &daynumber);
	(void) wprintw(stdscr, "(%s)", days[daynumber]);
}

	/*----------------------------------------------------------*/
	/*	
	/*			make_ifld
	/*	
	/*	Fill in an ifl structure so that the date and time
	/*	formatting routines will believe it.
	/*	
	/*----------------------------------------------------------*/

int
make_ifld(ip, y, x, len, type, val)
struct ifl_def *ip;
int y;
int x;
int len;
int type;
int val;
{
	register struct ifl_def *rip = ip;

	rip->y = y;
	rip->x = x;
	rip->len = len;
	rip->type = type;
	rip->value = val;
	
}

	/*----------------------------------------------------------*/
	/*	
	/*		DoSelectionInteractions
	/*	
	/*	Stay here until we're done with selection mode
	/*	
	/*----------------------------------------------------------*/
struct itm_state *iglobal_state;			/* ugh- we can't */
						/* get this to the */
						/* semantic routines */
						/* otherwise*/
int
do_item_interactions(statep)
struct itm_state *statep;
{
	register struct itm_state *sp = statep;

	iglobal_state = statep;

	quit_item = FALSE;
	do {
		(void) wrefresh(sp->w);		/* put up the display */
		process_outer_input(sp->w, FALSE /* no cursor movement */);
	} while (!quit_item);
}


int
RebuildItemScreen()
{
	(void) wrefresh(curscr);
}
	/*----------------------------------------------------------*/
	/*	
	/*			ItemCmd
	/*	
	/*	Called when an alpha character is typed in item display
	/*	mode.
	/*	
	/*----------------------------------------------------------*/

int
ItemCmd(fld, c, arg)
int fld;
char c;
int arg;
{
	MsgClear();
	UPCASE(c);
	switch (c) {
	      case 'F':
		itm_file(iglobal_state);
		break;
	      default:
		quit_item = TRUE;
		break;
	}
}

int
itm_file(statep)
struct itm_state *statep;
{
	FILE *file;
	struct itm_state *sp = statep;
	register TUPLE_DESCRIPTOR desc;
	TUPLE t = sp->tup;
	register int Index;
	int *dummy;
	int hour, mins;
	char ap;
	STRING *stp;


	desc = DESCRIPTOR_FROM_TUPLE(t);

	file = fopen("whatsup.items", "a");

	if (file==NULL) {
		Message("Sorry, could not open file named \"whatsup.log\"");
		return;
	}

       /*
        * Whatsup calendar unique event i.d.
        */
	Index = field_index(desc, "uniq");
	fprintf(file, "\nWhatsup calender unique event i.d. number: %d\n",
		*(int *)FIELD_FROM_TUPLE(t,Index));
       /*
        * event type
        */
	Index = field_index(desc, "type");
	stp = (STRING *)FIELD_FROM_TUPLE(t,Index);
	fprintf(file,"\nEvent type: %s\n", STRING_DATA(*stp));
       /*
        * date
        */
	Index = field_index(desc, "date");
	dummy = (int *)FIELD_FROM_TUPLE(t,Index);
	fprintf(file, "\nDate: %d/%d/%d\t", (*dummy / 100) %100, *dummy %100,
		*dummy/10000);
	Index = field_index(desc, "time");
       /*
        * time
        */
	dummy = (int *)FIELD_FROM_TUPLE(t,Index);
	hour = *dummy/100;
	mins = *dummy%100;
	if (*dummy == 0)
		ap = 'M';
	else if (*dummy == 1200) {
		ap = 'N';
	} else if (hour>=12)  {
		ap = 'P';
		if (hour > 12) 
			hour-=12;		
	} else
		ap = 'A';
     	fprintf(file, "Time: %d:%02d%c\n", hour, mins, ap);
       /*
        * title
        */
	Index = field_index(desc, "title");
	stp = (STRING *)FIELD_FROM_TUPLE(t,Index);
	fprintf(file,"\nTitle: %s\n", STRING_DATA(*stp));
       /*
        * place
        */
	Index = field_index(desc, "place");
	stp = (STRING *)FIELD_FROM_TUPLE(t,Index);
	fprintf(file,"\nPlace: %s\n", STRING_DATA(*stp));
       /*
        * comments
        */
	Index = field_index(desc, "comments");
	stp = (STRING *)FIELD_FROM_TUPLE(t,Index);
	fprintf(file,"\nComments: %s\n\n", STRING_DATA(*stp));

       /*
        * closing bar
        */

	fprintf(file,"\n\t\t***************************************\n");

	Message("Item appended to \"whatsup.items\" file.");

	(void) fclose(file);

	written = TRUE;

}



char ibanner_msg[] = "SELECTED EVENT DETAILS";
char ibanner_msg2[] = "F = Write to File;  Any other key to return to list";
int
itm_banner(statep)
struct itm_state *statep;
{
	register struct itm_state *sp = statep;
	register int i;
	int y;					/* dummy */
	int x;

       /*
        * Top banner line
        */
	x = max(0, (COLS-strlen(ibanner_msg)-2)/2);
	(void) wmove(sp->w, ITM_LINE, 0);
	(void) wstandout(sp->w);
	for (i=0; i<x; i++)
		(void) waddch(sp->w,' ');			/* dark leader */
	(void) waddstr(sp->w, ibanner_msg);
	getyx(sp->w, y, i);
	while (i<COLS-2) {
		(void) waddch(sp->w,' ');
		i++;
	}
	
       /*
        * second banner line
        */
	x = max(0, (COLS-strlen(ibanner_msg2)-2)/2);
	(void) wmove(sp->w, ITM_LINE+1, 0);
	for (i=0; i<x; i++)
		(void) waddch(sp->w,' ');			/* dark leader */
	(void) waddstr(sp->w, ibanner_msg2);
	getyx(sp->w, y, i);
	while (i<COLS-2) {
		(void) waddch(sp->w, ' ');
		i++;
	}
	
	(void) wstandend(sp->w);
}

int
destroy_itm_window(statep)
struct itm_state *statep;
{
	move(ITM_LINE, 0);
	clrtobot();
	refresh();
}
int
iput_cursor(statep)
struct itm_state *statep;
{
	register struct itm_state *sp = statep;
	register int cursor_line = ITM_LINE + 2;
	(void) wmove(sp->w, cursor_line,0);
}
