/************************************************************************/
/*	
/*			what_sel.c
/*	
/*	Selection routines for the whatsup calendar package.
/*	
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sel.c,v $
/*	$Author: ghudson $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sel.c,v 1.2 1996-09-19 22:16:31 ghudson Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/*	
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_sel_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_sel.c,v 1.2 1996-09-19 22:16:31 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <stdio.h>
#include <string.h>
#include "whatsup.h"

#define BANNER_LINES 5

int quit_select;

DATABASE calendar_data;
OPERATION open_db;

	/*----------------------------------------------------------*/
	/*	
	/*			GetSearchData
	/*	
	/*	Looks through the input fields to get the parameters
	/*	for the search.
	/*	
	/*----------------------------------------------------------*/

int
GetSearchData(sdatep, stimep, edatep, etimep, etypepp)
int *sdatep;
int *stimep;
int *edatep;
int *etimep;
char **etypepp;
{
	register struct ifl_def *fld_array = i_field;
	register int i;
	register int v;

       /*
        * Loop through fields, picking out the values
        */

	for (i=0; i<NIFIELDS; i++) {
		v = fld_array[i].value;
		switch (fld_array[i].type) {
		      case SDATE:
			*sdatep= v;
			break;
		      case STIME:
			*stimep= v;
			break;
		      case EDATE:
			*edatep= v;
			break;
		      case ETIME:
			*etimep= v;
			break;
		      case TYPE:
			*etypepp = (char *)v;
		      default:
			break;
			/* skip it if no match */
		}
	}
}
	/*----------------------------------------------------------*/
	/*	
	/*			move_if_needed
	/*	
	/*	given a source pointer, target pointer, and count,
	/*	do a counted move, but only if the move would
	/*	do some real work.
	/*	
	/*----------------------------------------------------------*/


int
move_if_needed(target, source, len)
char *target;
char *source;
int len;
{
	if (target != source && len >0) 
		(void) strncpy(target, source, len);
}

	/*----------------------------------------------------------*/
	/*	
	/*	`	   compress_blanks
	/*	
	/*	Given a pointer to a string, remove leading blanks
	/*	and turn all multiple blanks to singles.
	/*	
	/*----------------------------------------------------------*/

int
compress_blanks(stp)
char *stp;
{
	register char *cp=stp;			/* next byte to scan */
	register char *target=stp;		/* place to put next */
						/* move  */
	char *source=stp;			/* next move starts here */
	register int count=0;			/* number of bytes in */
						/* next move */
	int blanks=TRUE;			/* true iff this is */
						/* a blank field */

	while (*cp != '\0') {
		if (*cp++ == ' ') {
			if(blanks) {
				move_if_needed(target, source,count);
				target = target + count;
				while (*cp == ' ')
					cp++;
				source  = cp;
				count = 0;
				if (*cp == '\0')
					break;	/* blanks at end */
				blanks  = FALSE;
				continue;
			} else {
				blanks= TRUE;
			}
		} else
			blanks = FALSE;
		count++;
	}

	move_if_needed(target, source, count+1); /* +1 gets the null */
	  
	
}
	/*----------------------------------------------------------*/
	/*	
	/*			FormatQuery
	/*	
	/*	Prepare the QUEL string that will do the query.
	/*	
	/*----------------------------------------------------------*/

int
FormatQuery(buffer, sdate, stime, edate, etime, type)
char *buffer;
int sdate;
int stime;
int edate;
int etime;
char *type;
{
	register char *bp = buffer;

	(void) sprintf(bp, "(>*date*<=caltimes.date, >*time*<=caltimes.time, >*uniq*<=calitems.uniq, >*place*<=calitems.place, >*type*<=trim(calitems.type), >*title*<=calitems.title, >*comments*<=calitems.comments) where caltimes.uniq=calitems.uniq and caltimes.date>=%d and caltimes.date<=%d and caltimes.time>=%d and caltimes.time<=%d ",sdate, edate, stime, etime);
	bp += strlen(bp);
	if (type != NULL) {
		compress_blanks(type);
		if (*type != '\0') {
			(void) sprintf(bp," and calitems.type=\"%s\" ", type);
			bp += strlen(bp);
		}
	}
	(void) strcpy(bp," sort by RET_VAR1, RET_VAR2"); 

}


/*
 *			DoSearch
 *
 *	Called via psm table to do a database search.  Finds the data
 * 	and enters selection mode so user can choose an event of interest.
 */
int
DoSearch(field, typed_char, arg)
int	field;				/* where we were */
char	typed_char;			/* char typed to cause search */
char	*arg;				/* arg from psm table */
{
	RELATION found_data = NULL;
	int rc;
	int sdate, stime, edate, etime;		/* date and time */
						/* bounds on search*/
	char *event_type;
	char query_buffer[500];
	register TUPLE t;
	int count=0;
	char msg_buf[80];

	move(MESSAGE_Y+1,0);
	Message("Now searching... (use CTRL-C to give up and exit program)");
	
	GetSearchData(&sdate, &stime, &edate, &etime, &event_type);
	FormatQuery(query_buffer, sdate, stime, edate, etime, event_type);

       /*
        * Set max tuples to be retrieved
        */
	if (!limited) {
		(void) limit_retrieves(RETRIEVE_LIMIT+1);
		limited = TRUE;
	}
	
	rc = DoQuery(&found_data, query_buffer, query_desc);
       /*
        * Handle error
        */
	if (rc != 0) {
		clear();
		refresh();
		endwin();
		fprintf(stderr, "Error accessing calendar database\n");
		exit(64);
	}
       /*
        * Handle failure of search
        */
	if (found_data == NULL ||
	    FIRST_TUPLE_IN_RELATION(found_data) == NULL) {
		OuterMessage("Could not find any matching events\n");
		if (found_data != NULL) {
			delete_relation(found_data);
			found_data = NULL;
		}
		return;
	}
       /*
        * We seem to have found something, count 'em and report
        */

	for (t=FIRST_TUPLE_IN_RELATION(found_data); t!=NULL;
	     t=NEXT_TUPLE_IN_RELATION(found_data, t))
		count++;

/*****	if (count>RETRIEVE_LIMIT) {
		t  = PREV_TUPLE_IN_RELATION(found_data, ((TUPLE) found_data));
		REMOVE_TUPLE_FROM_RELATION(found_data, t);
		delete_tuple(t);
		count--;
	  	(void) sprintf(msg_buf,"Too many matches, only the first %d have been retrieved.", count);
	} else   *****SKIPPED DUE TO BUGS IN LIMIT SETTING ***/
	  	(void) sprintf(msg_buf,"%d events were found...", count);

	Message(msg_buf);

	SelectionMode(found_data);

	dirty_bits |= ~0;		/* force redisplay of */
						/* help data */
}
	/*----------------------------------------------------------*/
	/*	
	/*			SelectionMode
	/*	
	/*	The query found some data.  Now present a selection
	/*	to the user.
	/*	
	/*----------------------------------------------------------*/

int
SelectionMode(rel)
RELATION rel;
{
	struct sel_state state;

	struct p_tr *old_psm = current_psm;	/* remember the proc */
						/* state machine */
	register struct sel_state *sp;		/* pointer to our state */

	current_psm = select_psm;		/* what we call will */
						/* from now on be */
						/* according to selection */
						/* mode when a key is */
						/* pressed */

	sp = &state;

       /*
        * put in some plausible values for state
        */
	sp->rel = rel;				/* rell the world which */
						/* relation we're using */
	sp->length = tuples_in_relation(rel);	/* number of tuples in */
						/* the relation */
	sp->top = 0;				/* tuple 0 is top line */
	sp->bump = 0;				/* don't try any fancy */
						/* scrolling, just write */
						/* it all */
	sp->nlines = LINES - SEL_LINE;		/* number of lines */
	sp->selected = -1;			/* nothing selected  */
	sp->curs_line = 0;			/* cursor starts on first */
						/* tuple */
	sp->force = TRUE;			/* force an update */
       /*
        * Create the selection window and bind the tuple data to it.
        */
	create_sel_window(sp);
	sync_sel_window(sp);			/* put initial contents */
						/* in window */
	do_selection_interactions(sp);		/* stay here as long */
						/* as we are in selection */
						/* mode */
	destroy_sel_data(sp);

	destroy_sel_window(sp);
	current_psm = old_psm;
}

	/*----------------------------------------------------------*/
	/*	
	/*			create_sel_window
	/*	
	/*	create the subwindow in which the selections will
	/*	be shown.  Clear it and display the banner.
	/*	
	/*----------------------------------------------------------*/

int
create_sel_window(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;

	sp->w = stdscr;
	move(SEL_LINE, 0);
	clrtobot();
	sel_banner(sp);
}
	/*----------------------------------------------------------*/
	/*	
	/*			sync_sel_window
	/*	
	/*	Synchronize the contents of the selection window
	/*	with the data according to contents.  The bump variable
	/*	is used to optimize scrolling.  Positive numbers mean
	/*	that the data is moving up on the screen, negative
	/*	mean it is moving down.
	/*	
	/*	For now, we won't optimize at all, later maybe we will.
	/*	
	/*----------------------------------------------------------*/
int
sync_sel_window(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	register int i;
	int first;				/* we will go here */
						/* before writing */
	int count;				/* number of lines */
						/* we'll actually */
						/* write */
	int tup_num;				/* number of the */
						/* tuple we'll write */
						/* first */
	int curline;				/* next line we'll write */
	int top;				/* working copy of top */
	int scroll, uscroll;			/* (un)signed amount we */
						/* will actuall scroll*/
	TUPLE t;
	int oldbump = sp->bump;			/* remember the bump */

	first = BANNER_LINES;			/* skip banner line */
	count = sp->nlines - BANNER_LINES;	/* leave off one for banner */
	tup_num = sp->top;
	top = sp->top + sp->bump;
	
       /*
        * Handle possible bump (scroll)
        */
	if (sp->bump !=0) {
               /*
                * count now contains the number of writeable lines on the
                * screen.  Figure out the new top tuple.
                */
		if (sp->length - top < count) 
		      top = sp->length - count;
		if (top<0)
			top = 0;
               /*
                * top - sp->top now tells us how much we're really going to
                * scroll (signed).  If it's less than the size of the writeable
                * area, then we can do optimizations.
                */
		scroll = top - sp->top;
		uscroll = (scroll>0)?scroll: (-scroll);
		if (uscroll<count) {
                       /*
                        * Optimize
                        */
			if (scroll > 0) {
                               /*
                                * down
                                */
				move(first+SEL_LINE,0);	
				for (i=0; i<scroll; i++)
					deleteln();
				move(first+SEL_LINE+count-scroll,0);	
				clrtobot();	/* redundant? */
				tup_num = top+count-scroll;
				first = first + count - scroll;
				count = scroll;
				
			} else {
                               /*
                                * up
                                */
				tup_num = top;
			}
		}
		
		sp->top = top;
		sp->bump = 0;
	}

	if (oldbump || sp->force) {

		/*
		 * Set t to the first tuple we'll actually write, if any
		 */
		t = FIRST_TUPLE_IN_RELATION(sp->rel);
		
		while (t != NULL && tup_num>0) {
			t = NEXT_TUPLE_IN_RELATION(sp->rel, t);
			tup_num--;
		}
	
		/*
		 * Write each line in place--for now, we don't mess with the bump
		 */
		
		curline = first;
		
		while (count--) {
			write_tuple(sp->w, t, curline);
			curline++;
			t = NEXT_TUPLE_IN_RELATION(sp->rel, t);
			if (t==NULL) {
				(void) wclrtobot(sp->w);
				break;
			}
		}
	}

	sp->force = FALSE;
	put_cursor(sp);
	
}

	/*----------------------------------------------------------*/
	/*	
	/*		write_tuple
	/*	
	/*	Format the supplied tuple on the indicated line
	/*	
	/*----------------------------------------------------------*/

extern char *days[];
int
write_tuple(win, tup, line)
WINDOW *win;
TUPLE tup;
int line;
{
	TUPLE_DESCRIPTOR desc;
	char buffer[160];
	int Index;
	STRING *title;
	STRING *type;
	int date;
	int month, day;
	int time;
	int hour, minute;
	char ap;
	int daynumber;
	char *dayid;
	int y,x;

	desc = DESCRIPTOR_FROM_TUPLE(tup);
	Index = field_index(desc, "date");
	date = *(int *)FIELD_FROM_TUPLE(tup, Index);
	month = (date % 10000) / 100 ;
	day = date % 100;
	day_from_date(date, &daynumber);
	dayid = days[daynumber];

	Index = field_index(desc, "time");
	time = *(int *)FIELD_FROM_TUPLE(tup, Index);
	hour = time / 100;
	minute =  time % 100;
	if (hour == 0 && minute == 0) 
		ap = 'M';
	else if (hour == 12 && minute ==0)
		ap = 'N';
	else if (hour < 12)
	 	ap = 'A';
	else {
		ap = 'P';
		if (hour>12)
			hour -= 12;
	}
	
	Index = field_index(desc, "title");
	title = (STRING *)FIELD_FROM_TUPLE(tup, Index);
	Index = field_index(desc, "type");
	type = (STRING *)FIELD_FROM_TUPLE(tup, Index);
	(void) wmove(win, line+SEL_LINE, 0);
	(void) wprintw(win, "%s %2d/%-d %2d:%02d%c (%s) ", dayid, month, day, hour, minute, ap, STRING_DATA(*type));
	getyx(win, y,x);
      	(void) strncpy(buffer,STRING_DATA(*title), 150);
	*(buffer+COLS-2-x) = '\0';		/* set max len of title */
						/* to avoid wrap */
	(void) waddstr(win, buffer);
	(void) wclrtoeol(win);
}

	/*----------------------------------------------------------*/
	/*	
	/*		DoSelectionInteractions
	/*	
	/*	Stay here until we're done with selection mode
	/*	
	/*----------------------------------------------------------*/
struct sel_state *global_state;			/* ugh- we can't */
						/* get this to the */
						/* semantic routines */
						/* otherwise*/
int
do_selection_interactions(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;

	global_state = statep;

	quit_select = FALSE;
	do {
		(void) wrefresh(sp->w);		/* put up the display */
		process_outer_input(sp->w, FALSE /* no cursor movement */);
	} while (!quit_select);
}


int
RebuildSelectScreen()
{
	(void) wrefresh(curscr);
}
	/*----------------------------------------------------------*/
	/*	
	/*			SelectCmd
	/*	
	/*	Called when an alpha character is typed in selection
	/*	mode.
	/*	
	/*----------------------------------------------------------*/

int
SelectCmd(fld, c, arg)
int fld;
char c;
int arg;
{

       /*
        * allow arrows for up and down
        */
	switch (arg) {
	      case UPARROW:
		c = 'U';
		break;
	      case DOWNARROW:
		c = 'D';
		break;
	      default:
		break;
	}

	UPCASE(c);
	switch (c) {
	      case 'U':
	      case 'P':
		sel_up(global_state);
		break;
	      case 'F':
		sel_fwd(global_state);
		break;
	      case 'B':
		sel_bwd(global_state);
		break;
	      case 'S':
		DoItem(global_state);
		global_state->force = TRUE;	/* force repaint of */
						/* selection list */
		sel_banner(global_state);
		sync_sel_window(global_state);
		break;
	      case 'D':
	      case'N': 
	      case RETURN:
		sel_down(global_state);
		break;
	      case 'Q':
		quit_select = TRUE;
		break;
	      default:
		(void) putc('\007', stderr);
	}
}

int
sel_up(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	register int prev;

       /*
        * Compute desired previous line
        */
	prev = sp->curs_line -1 ;
	if (prev <0) {
		(void) putc('\007', stderr);
		sync_sel_window(sp);
		return;
	}
       /*
        * Prev is tuple number we're moving to see if it's off screen
        */
	if (prev < sp->top)
		sp->bump = -1;			/* scroll up */

	sp->curs_line = prev;
       
	sync_sel_window(sp);
	
}

int
sel_bwd(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	int lines = sp->nlines - BANNER_LINES;

       /*
        * Make sure not already at top
        */
	if (sp->top <= 0) {
		sp->bump = 0;
		(void) putc('\007', stderr);
		sync_sel_window(sp);
		return;
	}
       /*
        * Prev is tuple number we're moving to see 
        */
	sp->bump = 2-lines;			/* scroll up */

       /*
        * never scroll before first item
        */
	if (sp->top +sp->bump < 0) {
		sp->bump = -sp->top;
		sp->curs_line = 0;
	}
	

	sync_sel_window(sp);
	
}

int
sel_fwd(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	int lines = sp->nlines - BANNER_LINES;

	if (sp->length - sp->top <= lines) {
		(void) putc('\007', stderr);
		sp->curs_line = sp->length-1;
		sync_sel_window(sp);
		return;
	}

	sp->bump = lines-2;			/* scroll down */

       /*
        * never scroll past last item
        */
	if (sp->top +sp->bump >= sp->length-lines) {
		sp->bump = sp->length-lines-sp->top;
		sp->curs_line = sp->length-1;
	}

	sync_sel_window(sp);
	
}

int
sel_down(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	register int next;

       /*
        * Compute desired nextious line
        */
	next = sp->curs_line +1 ;
	if (next >= sp->length) {
		(void) putc('\007', stderr);
		sync_sel_window(sp);
		return;
	}
       /*
        * Next is tuple number we're moving to see if it's off screen
        */
	if (next >= sp->top+sp->nlines - BANNER_LINES)
		sp->bump = 1;			/* scroll up */

	sp->curs_line = next;

	sync_sel_window(sp);
       
	
}


char banner_msg[] = "EVENT LIST";
char banner_msg2[] = "S = Select; U = Up; D = Down; F = Fwd Page; B = Back Page; Q = Quit";
int
sel_banner(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	register int i;
	int y;					/* dummy */
	int x;
	char ap;
	int sdate, stime, edate, etime;
	char *event_type;
	int year, month, day, daynumber, hour, minute;
	char *dayid;

       /*
        * Get the search criteria
        */
	GetSearchData(&sdate, &stime, &edate, &etime, &event_type);


       /*
        * Top banner line
        */
	x = max(0, (COLS-strlen(banner_msg)-2)/2);
	(void) wmove(sp->w, SEL_LINE, 0);
	(void) wstandout(sp->w);
	for (i=0; i<x; i++)
		(void) waddch(sp->w,' ');			/* dark leader */
	(void) waddstr(sp->w, banner_msg);
	getyx(sp->w, y, i);
	while (i<COLS-2) {
		(void) waddch(sp->w,' ');
		i++;
	}

       /*
        * Blank banner line (2nd)
        */
	(void) wmove(sp->w, SEL_LINE+1, 0);
	for (i=0; i<COLS-2; i++)
		(void) waddch(sp->w,' ');
       /*
        * Print criteria (3rd line)
        */
	(void) wmove(sp->w, SEL_LINE+2, 0);
	if (sdate == edate)
		x=12;
	else
		x=6;
	for (i=0; i<x; i++)
		(void) waddch(sp->w,' ');			/* dark leader */
	hour = stime / 100;
	minute =  stime % 100;
	if (hour == 0 && minute == 0) 
		ap = 'M';
	else if (hour == 12 && minute ==0)
		ap = 'N';
	else if (hour < 12)
	 	ap = 'A';
	else {
		ap = 'P';
		if (hour>12)
			hour -= 12;
	}
	(void) wprintw(sp->w, "Search criteria: %d:%02d%c to ",  hour, minute, ap);
	hour = etime / 100;
	minute =  etime % 100;
	if (hour == 0 && minute == 0) 
		ap = 'M';
	else if (hour == 12 && minute ==0)
		ap = 'N';
	else if (hour < 12)
	 	ap = 'A';
	else {
		ap = 'P';
		if (hour>12)
			hour -= 12;
	}
	(void) wprintw(sp->w, "%d:%02d%c on ",  hour, minute, ap);
	month = (sdate % 10000) / 100 ;
	day = sdate % 100;
	day_from_date(sdate, &daynumber);
	dayid = days[daynumber];
	(void) wprintw(sp->w, "(%s) %d/%-d", dayid, month, day);
	if (sdate != edate) {
		year = edate / 10000;
		month = (edate % 10000) / 100 ;
		day = edate % 100;
		day_from_date(edate, &daynumber);
		dayid = days[daynumber];
		(void) wprintw(sp->w, " through (%s) %d/%-d", dayid, month, day);
	}

	getyx(sp->w, y, i);
	while (i<COLS-2) {
		(void) waddch(sp->w, ' ');
		i++;
	}
	
       /*
        * Blank banner line or event type (3rd)
        */
	(void) wmove(sp->w, SEL_LINE+3, 0);
	if (event_type != NULL && *event_type != ' ' && *event_type != '\0') {
		for (i=0; i<15; i++) 
		  	(void) waddch(sp->w,' ');
		(void) wprintw(sp->w, "Search limited to events of type: %s", event_type);
		getyx(sp->w, y, i);
		while (i<COLS-2) {
			(void) waddch(sp->w, ' ');
			i++;
		}
	}else {
		for (i=0; i<COLS-2; i++)
			(void) waddch(sp->w,' ');
	}
       /*
        * fourth banner line
        */
	x = max(0, (COLS-strlen(banner_msg2)-2)/2);
	(void) wmove(sp->w, SEL_LINE+4, 0);
	for (i=0; i<x; i++)
		(void) waddch(sp->w,' ');			/* dark leader */
	(void) waddstr(sp->w, banner_msg2);
	getyx(sp->w, y, i);
	while (i<COLS-2) {
		(void) waddch(sp->w, ' ');
		i++;
	}
	
	(void) wstandend(sp->w);
}

int
destroy_sel_data(statep)
struct sel_state *statep;
{
       if (statep->rel != NULL)
		delete_relation(statep->rel);
       statep->rel = NULL;
}
int
destroy_sel_window(statep)
struct sel_state *statep;
{
	move(SEL_LINE, 0);
	clrtobot();
	refresh();
}
int
put_cursor(statep)
struct sel_state *statep;
{
	register struct sel_state *sp = statep;
	register int cursor_line;

       /*
        * Get sp->curs_line into the window... it should already be
        * in most cases.  Also make sure it's not off the end of the
        * relation.
        */
	if (sp->curs_line<sp->top || sp->curs_line >= sp->top+sp->nlines-
	    					      BANNER_LINES)
		if (sp->curs_line<sp->top)
			sp->curs_line = sp->top;
		else
			sp->curs_line = sp->top +sp->nlines -BANNER_LINES;;
	if (sp->curs_line >=  sp->length)
		sp->curs_line = sp->length-1;
       /*
        * set cursor_line to the actual screen line for the cursor
        * and move it.
        */

	cursor_line = sp->curs_line +SEL_LINE+BANNER_LINES-sp->top;

	(void) wmove(sp->w, cursor_line,0);
}
