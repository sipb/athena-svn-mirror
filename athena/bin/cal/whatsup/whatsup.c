/************************************************************************/
/*	
/*			whatsup
/*	
/*	An experimental online calendar of events for the MIT
/*	campus.
/*	
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/whatsup.c,v $
/*	$Author: miki $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/whatsup.c,v 1.2 1994-03-25 16:05:50 miki Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	Author: Noah Mendelsohn
/*	
/*	
/************************************************************************/
/*	
/*	
/*	Implementation notes:
/*	
/*	This program tries all kinds of fancy tricks in an attempt to
/*	see which ones may be useful in the long run.  Some of these
/*	have been grafted on to the program as they occured to me, so
/*	the code is a bit rambling.  In particular:  
/*	
/*		1) output only fields are all defined symbolically
/*		   along with their text in various defines and
/*		   compile time init's.  There is a regular (if messy)
/*		   procedure for adding new labels to the display.
/*	
/*		2) A FSM is used to control cursor motion among the
/*		   fields.  This is encoded as a two dimensional array
/*		   indexed by current input field and keystroke yielding
/*		   new input field for cursor.
/*	
/*		3) A constraint machine is used to represent interdependencies
/*		   among some of the prompting fields, particularly those
/*		   having to do with time.  This is one of the most 
/*		   questionable (albeit amusing) features of this program.
/*		   This is an array which basically says:  changes to
/*		   field X should cause re-evaluation of field Y.  The CSM
/*		   interpreter iterates until all the constraints are 
/*		   relaxed.
/*	
/*	One of these days I'll have to rearrange this huge source file
/*	into some reasonable modules.  One of the pitfalls of bottom-up
/*	design and development.  
/*	
/*	
/************************************************************************/

#ifndef lint
static char rcsid_whatsup_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/whatsup.c,v 1.2 1994-03-25 16:05:50 miki Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include "whatsup.h"
#define LOGINFO "/usr/unsupported/whatsup.logging"

/*
 *   Locator array for each output field.  These are referenced by
 *   the cursor motion state machine. Indexed by the symbolic names
 *   given above.
 */
struct ofl_def o_field[NOFIELDS] = {
	{START_L_X, START_L_Y, START_L_T},	/* starting time (START_L) */
	{END_L_X, END_L_Y, END_L_T},	  	/* ending time (END_L) */
	{DATE_L_X, DATE_L_Y, DATE_L_T},		/* date prompt */
	{TIME_L_X, TIME_L_Y, TIME_L_T},		/* time prompt */
	{HEADER_X, HEADER_Y, HEADER_T},		/* banner across top */
	{CTRLC_X, CTRLC_Y, CTRLC_T},		/* Ctrl-c prompt */
	{QMARK_X, QMARK_Y, QMARK_T},		/* question mark prompt */
	{DAY_L_X, DAY_L_Y, DAY_L_T},		/* day banner across top */
	{TYPE_L_X, TYPE_L_Y, TYPE_L_T},		/* event type prompt */
	{DATE2_L_X, DATE2_L_Y, DATE2_L_T},	/* event 1) prompt */
	{TIME2_L_X, TIME2_L_Y, TIME2_L_T},	/* event 1) prompt */
	{TYPE1_L_X, TYPE1_L_Y, TYPE1_L_T},	/* event 2) prompt */
	{PRIVACY_X, PRIVACY_Y, PRIVACY_T}	/* event 2) prompt */
};

int vNOFIELDS = NOFIELDS;			/* this is NOFIELDS */
						/* as a variable so */
						/* we can decrement */
						/* the privacy warning*/
/*
 *   Locator array for each input field.  These are referenced by
 *   the cursor motion state machine. Indexed by the symbolic names
 *   given above.
 */


struct ifl_def i_field[NIFIELDS] = {
/*    x		    y		len    typ v  nv   relative */
{START_DAY_P_X,	START_DAY_P_Y,	DAY_LEN, SDAY, 0, 0, START_DATE_P},/*START_DAY*/
{END_DAY_P_X,	END_DAY_P_Y,	DAY_LEN, EDAY, 0, 0, END_DATE_P}, /*END_DAY */
{START_DATE_P_X,START_DATE_P_Y,	DATE_LEN,SDATE,0, 0, START_TIME_P},/*START_DATE*/
{END_DATE_P_X,  END_DATE_P_Y,	DATE_LEN,EDATE,0, 0, END_TIME_P}, /*END_DATE */
{START_TIME_P_X,START_TIME_P_Y, TIME_LENG,STIME,0,0, END_TIME_P},  /*START_TIME*/
{END_TIME_P_X,	END_TIME_P_Y,	TIME_LENG,ETIME,0,0, START_TIME_P}, /*END_TIME */
{TYPE1_P_X,	TYPE1_P_Y,	TYPE_LEN,TYPE, 0,0, 0},           /*TYPE1 */
/****{TYPE2_P_X,	TYPE2_P_Y,	TYPE_LEN,TYPE, 0,0, 0},           /*TYPE2 */
};

/*----------------------------------------------------------------
 |                                                               |
 |                  Constraint machine governing                 |
 |   dependencies between input fields.  Change to first field   |
 |   implies change may be required in second.			 |
 |                                                               |
 *---------------------------------------------------------------*/
int i_dep[NDEP][2] = {
	START_DAY_P, START_DATE_P, /* OK */
	START_DATE_P, START_DAY_P, /* OK */
	START_DATE_P, END_DATE_P,  /* OK */
	START_TIME_P, END_TIME_P, /* OK */
	END_DAY_P, END_DATE_P,     /* OK */
	END_DATE_P, END_DAY_P,     /* OK */
	END_DATE_P, START_DATE_P, /* OK */
	END_TIME_P, START_TIME_P  /* OK */
};
	
      
       
struct k_map_entry k_map[SIZE_OF_KEYBOARD_MAP] = {
	{'\t',FTAB},
	{'\n',CR},
	{'\r',CR},
	{'*',AST},
	{'\014',REF},			/* CTL-L for refresh */
	{'\0',ALPHA},			/* dummy entry */
	{'\0',NUM},			/*   "    "    */
	{' ', BLANK},
        {DEL, BSP},
	{'\b', BSP},
	{'\0', UPARROW},
	{'\0', DOWNARROW},
	{'\0', LEFTARROW},
	{'\0', RIGHTARROW},
        {'?', QMARK},
	
};
	/*----------------------------------------------------------*/
	/*	
	/*	    Procedure Activation State Machine
	/*	
	/*	For a given input character, decide which procedure
	/*	to call and which arguments to give.
	/*	
	/*	Handler procedures are automatically passed the index
	/*	of the current ffield, typed char, and arg from below.
	/*	
	/*	There are several of these and we switch according
	/*	to where we are in the interaction cycle.
	/*	
	/*----------------------------------------------------------*/


int DoSearch();
int FillField();
int RebuildScreen();
int RebuildSelectScreen();
int SelectCmd();
int RebuildItemScreen();
int ItemCmd();
int GetHelp();

struct p_tr main_psm[NUMBER_OF_KEY_TYPES] = {
	{/*FTAB*/	NULL, NULL},
	{/*BSP*/		NULL, NULL},
	{/*CR*/		NULL, NULL},
	{/*UPARROW*/	NULL, NULL},
	{/*DOWNARROW*/		NULL, NULL},
	{/*LEFTARROW*/	NULL, NULL},
	{/*RIGHTARROW*/		NULL, NULL},
	{/*AST*/	DoSearch, NULL},        
	{/*REF*/	RebuildScreen, NULL},        
	{/*ALPHA*/      FillField, NULL},
	{/*NUM*/	FillField, NULL},
        {/*BLANK*/      FillField, NULL},
        {/*QMARK*/      GetHelp, (char *)0},
};

struct p_tr select_psm[NUMBER_OF_KEY_TYPES] = {
	{/*FTAB*/	NULL, NULL},
	{/*BSP*/		NULL, NULL},
	{/*CR*/		SelectCmd, NULL},
	{/*UPARROW*/	SelectCmd, (char *)UPARROW},
	{/*DOWNARROW*/	SelectCmd, (char *)DOWNARROW},
	{/*LEFTARROW*/	SelectCmd, (char *)LEFTARROW},
	{/*RIGHTARROW*/		SelectCmd, (char *)RIGHTARROW},
	{/*AST*/	NULL, NULL},        
	{/*REF*/	RebuildSelectScreen, NULL},        
	{/*ALPHA*/      SelectCmd, NULL},
	{/*NUM*/	NULL, NULL},
	{/*BLANK*/      NULL, NULL},

        {/*QMARK*/      GetHelp, (char *)1},
};
struct p_tr item_psm[NUMBER_OF_KEY_TYPES] = {
	{/*FTAB*/	NULL, NULL},
	{/*BSP*/		NULL, NULL},
	{/*CR*/		ItemCmd, NULL},
	{/*UPARROW*/	NULL, NULL},
	{/*DOWNARROW*/		NULL, NULL},
	{/*LEFTARROW*/	NULL, NULL},
	{/*RIGHTARROW*/		NULL, NULL},
	{/*AST*/	NULL, NULL},        
	{/*REF*/	RebuildItemScreen, NULL},        
	{/*ALPHA*/      ItemCmd, NULL},
	{/*NUM*/	NULL, NULL},
	{/*BLANK*/      ItemCmd, NULL},
        {/*QMARK*/      GetHelp, (char *)2},

};

struct p_tr *current_psm = main_psm;		/* initial one is main */

struct box_list {
     int x,y,w,h;				/* position and size */
     char *msg;					/* center in box */
     WINDOW *win;				/* pointer to the window */
} box_list[] = {
{TIME_BOX_X, TIME_BOX_Y, TIME_BOX_W, TIME_BOX_H, " TIME RANGE ", NULL},
{DATE_BOX_X, DATE_BOX_Y, DATE_BOX_W, DATE_BOX_H, " DATE RANGE ", NULL},
{TYPE_BOX_X, TYPE_BOX_Y, TYPE_BOX_W, TYPE_BOX_H, " TYPE OF EVENT (Optional) ", NULL},
};

int box_count = sizeof(box_list)/sizeof(struct box_list);


/*----------------------------------------------------------------
 |                                                               |
 |                  Cursor Motion State Machine                  |
 |                                                               |
 |      These presume a 24x80 screen, all we support right       |
 |      now.                                                     |
 |                                                               |
 *---------------------------------------------------------------*/


/*
 * state machine transition table
 * 
 * Major rows are for a current cursor position (as a field)
 * The entries in each row show where to go for a forward tab, 
 * newline, and carriage return.  It might not be unreasonable
 * to compute this during startup, or to alter it (or substitute
 * tables) during execution to change the cursor motion semantics
 * (i.e. modes!?!?!).  Note that the second, and fastest varying, 
 * subscript is determined by the keyboard mapping above.  For each
 * initial cursor position, there is a possible motion for each logical
 * keyboard input. 
 */


int tr_tab[NIFIELDS][SIZE_OF_CURSOR_KEYBOARD_MAP] = {
	{START_DATE_P, END_TIME_P, START_TIME_P, START_DAY_P, END_DAY_P,
             END_TIME_P, START_DATE_P},		/* START_DAY_P */
        {END_DATE_P, START_DATE_P, TYPE1_P, START_DAY_P, TYPE1_P, END_TIME_P,
            END_DATE_P},	     	/* END_DAY_P */
	{END_DAY_P, START_DAY_P, END_DAY_P, START_DATE_P, END_DATE_P, START_DAY_P, START_DATE_P},		/* START_DATE_P */
	{TYPE1_P, END_DAY_P, TYPE1_P, START_DATE_P, TYPE1_P, 
		END_DAY_P,  END_DATE_P},	     	/* END_DATE_P */
	{END_TIME_P, TYPE1_P, TYPE1_P, START_TIME_P, TYPE1_P, 
		START_TIME_P, END_TIME_P},		/* START_TIME_P */
	{START_DAY_P, START_TIME_P, TYPE1_P, END_TIME_P, TYPE1_P, 
		START_TIME_P, START_DAY_P},		/* END_TIME_P */
	{START_TIME_P, END_DATE_P, START_TIME_P, START_TIME_P, TYPE1_P,
		TYPE1_P, TYPE1_P},	        /* TYPE1_P */
/****	{START_DAY_P, START_DAY_P, START_DAY_P},            /* TYPE2_P */
};

int current_field;			/* field where the cursor is now */


/*----------------------------------------------------------------
 |                                                               |
 |                  Misc. Global Variables                       |
 |                                                               |
 *---------------------------------------------------------------*/

char	pre_read_char;			/* character to be processed as 
					   global input */
int     done = FALSE;			/* time to exit ? */
int	today;				/* number of day in week (0-6) */
int     this_day;			/* number of day in month */
int     this_month;			/* number of the current month */
int     this_year;			/* number of the current year (yy) */
int	time_window = 1;		/* number of hours to search
					   when not explicitly given */
short dirty_bits;				/* which areas need */
						/* repaint*/

char *PendMsg = NULL;				/* a message which is */
						/* to be shown continuously*/
char *OuterMsg = NULL;				/* a message which is */
						/* shown just once upon */
						/* return to outer loop */

short screen_format;

/* forward function references */

int	die();

/* a structure for playing with time */

struct t_struct t_struct, t_struct2;

TUPLE_DESCRIPTOR query_desc;			/* holds the descriptor */
						/* for the relations */
						/* we retrieve */

TUPLE_DESCRIPTOR MakeQueryDescriptor();

WINDOW *help_window;

int written = FALSE;
int limited = FALSE;				/* have we set the */
						/* retrieve limit yet */


int main(argc, argv)
int argc;
char *argv[];
{

	char whatsup_db[100]; 
	int first_time = TRUE;
#ifdef POSIX
	struct sigaction act;
#endif
       /*
        * handle the secret -db option
        */
	(void) strcpy(whatsup_db, "whatsup-mit");
	if (argc>1) {
		if(strcmp(argv[1], "-db") != 0) {
			fprintf(stderr, "No arguments should be supplied to whatsup\n\nWe'll keep going anyway!\n");
		} else {
                       /*
                        * site specification attempted
                        */
			if (argc != 3) {
				fprintf(stderr, "Correct form is whatsup -db dbname@site\n");
				exit(16);
			}
			(void) strcpy(whatsup_db, argv[2]);

		}
	}

       /*
        * Start talking to the user
        */
	gdb_init();
	hello_prompt();				/* welcome the user */
						/* and warn of recording*/

	printf("Attempting to connect to calendar database %s, hang on...\n", whatsup_db);
	(void) initscr();
#ifdef POSIX
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler= (void (*)()) die;
	(void) sigaction(SIGINT, &act, NULL);
#else
	(void) signal(SIGINT,die);
#endif
       /*
        * Limit screen size to 24, because curses takes too long calculating
        * Refreshes otherwise.
        */
	if (LINES<24 || COLS<80) {
		fprintf(stderr, "\n\nERROR: Your screen or window must be at least 24 lines long and 80 columns wide\nin order to run whatsup.\n\nIf you are running in an X window and decide to make it larger, be sure that\nyou get the size updated properly, or whatsup won't be able to detect\nthe change.  (See the discussion of resizing windows in the Project\nAthena \"Essential X\" guide for instructions.\n\n");
		endwin();
		exit(16);
	}  
	(void) noecho();
	(void) nonl();
	crmode();
	leaveok(stdscr, FALSE);
	scrollok(stdscr, FALSE);

/* 	gdb_log = fopen("whatsup.log", "w");  */

	PrimeDb(whatsup_db);

	init_boxes();

	time_initialize();		/* get today's date and time,
					   and initialize the start
					   and end */
	make_dirty();

	initialize_screen();		/* set up the state variables 
					   controlling the screen */

	done = FALSE;

	query_desc = MakeQueryDescriptor();	/* build the tuple */
						/* descriptor for our */
						/* queries */


	while (!done) {
		if (OuterMsg == NULL) 
			Message("Use TAB, ARROW keys, and RETURN to move cursor. Press '*' to find events.");
		else
			OuterMsg = NULL;
		repaint();
               /*
                * Kludge:  we want to have the CTRL-C to exit message
                * replace the you are not being recorded message.
                * 
                * We just put this at the end of the list, and by
                * decrementing NOFIELDS, we cause it to be ignored
                * next time!
                */
		if (first_time) {
			vNOFIELDS--;
			dirty_bits |= MAIN_DIRTY;
			first_time = FALSE;
		}
		process_outer_input(stdscr, TRUE /* do cursor movement */);
	}


}

/*
 * 			init_boxes
 * 
 * Build a subwindow for each box
 */
int
init_boxes()
{
	register int i;
	register struct box_list *bp;

	for (i=0; i<box_count; i++) {
		bp = box_list+i;
		bp->win = subwin(stdscr, bp->h, bp->w, bp->y, bp->x);
	}
}
/*
 * 			draw_boxes
 * 
 * Build a subwindow for each box
 */
int
draw_boxes()
{
	register int i;
	register struct box_list *bp;
	int pos;

	for (i=0; i<box_count; i++) {
		bp = box_list+i;
		box(bp->win, '*', '*');
		pos = (bp->w - strlen(bp->msg))/2;
		(void) wmove(bp->win, 0, pos);
		(void) waddstr(bp->win,bp->msg);
		
	}
}
/*
 * 			PendingMessage
 * 
 * 	Sets up a message which will be presented immediately, and which
 * 	will be shown whenever a message clear is done until a PendingClear
 *      is done.
 */

int
PendingMessage(msg)
char *msg;
{
	PendMsg = msg;
	MsgClear();
}
/*
 * 			OuterMessage
 * 
 * 	Sets up a message which will be presented immediately, and which
 * 	will be shown just once when returning to the outer interaction
 *      loop.  It will then be replaced by the regular outer message prompt.
 */

int
OuterMessage(msg)
char *msg;
{
	OuterMsg = msg;
	Message(OuterMsg);
}
/*
 * 			OuterError
 * 
 * 	Sets up a error which will be presented immediately, and which
 * 	will be shown just once when returning to the outer interaction
 *      loop.  It will then be replaced by the regular outer error prompt.
 */

int
OuterError(msg)
char *msg;
{
	OuterMsg = msg;
	Error(OuterMsg);
}
/*
 * 			PendingClear
 * 
 * 	Unset pending messages, but do not tear down the message field
 */

int
PendingClear()
{
	PendMsg = NULL;
}
/*
 *			Message
 *
 *	Write an informative or error message at bottom of screen, returning
 *	cursor to its original position when done.
 */
int
Message(str)
char *str;
{
	int row, col;

	getyx( stdscr, row, col);
	move(MESSAGE_Y, MESSAGE_X);
	addstr(str);
	clrtoeol();
	move(row, col);
	refresh();
}
/*
 *			MsgClear
 *
 *	Clear the message area--do not force refresh.
 */
int
MsgClear()
{
	int row, col;

	getyx( stdscr, row, col);
	move(MESSAGE_Y, MESSAGE_X);
	clrtoeol();
	if (PendMsg != NULL)
		Message(PendMsg);
	move(row, col);
}

/*
 *			Error
 *
 *	Write an informative or error message at bottom of screen, returning
 *	cursor to its original position when done.
 */
int
Error(str)
char *str;
{
	int row, col;

	getyx( stdscr, row, col);		/* remember where we were */
	move(MESSAGE_Y, MESSAGE_X);		/* go to message area */
	(void) putc(BELL, stderr);
	standout();				/* tell curses to highlight */
	addstr(str);
	standend();
	clrtoeol();
	move(row, col);				/* pop cursor to old location*/
	refresh();
}


/*
 *   Come here when killed 
 */
int 
die()
{
#ifdef POSIX
   struct sigaction act;
         sigemptyset(&act.sa_mask);
         act.sa_flags = 0;
         act.sa_handler= (void (*)()) SIG_IGN;
         (void) sigaction(SIGINT, &act, NULL);
#else
	(void) signal(SIGINT,SIG_IGN);
#endif
	clear();
	mvcur(0,COLS-1,LINES-1,0);
	refresh();
	endwin();
	if (written) {
		printf("\nThe items you filed with the \"F\" command have been stored\nin the file named \"whatsup.items\"\n\n");
	}
	exit(0);
}



char *help_text0[] = 
{
"The WHATSUP at MIT calendar program may be used to look for events",
"occurring within the coming 12 months.  To use the calendar:",
"",
"1) Select a range of times and a range of dates.  NOTE:  the time",
"   range applies to EACH day.  Therefore, a search from 3PM-5PM on",
"   MON through FRI searches 2 hours on each of 5 days.",
"",
"   TYPE OVER THE FIELDS TO CHANGE THEM.  If you don't pick a date or time,",
"   the rest of the current day is used as the default.",
"",
"   Don't search for too much;  a couple of days at a time or few hours",
"   each day for a week is about right, or else the search may take too long.",
"",
"2) OPTIONAL: choose a specific type of event.  Right now, the only way",
"   to learn the types is by looking at the data in the calendar.",
"",
"3) Press * to search.",
"",
"4) Choose items from the list of events and use 'S' to call up details.",
};

char *help_text1[] = 
{
"",
"",
"You have retrieved a list of events which match the dates, the times,",
"and the type (if you specified one) requested.",
"",
"Details are available for each of the events in the calendar.  You",
"may select an event by moving the cursor up or down.  When the cursor",
"is on the appropriate line, press the S key to call up details.",
"",
"To move the cursor, use the U (up a line),  D (down a line),",
"F (forward to next whole screen), B (back to previous screen), keys.",
"The UP and DOWN arrows also work on most terminals, and RETURN is",
"equivalent to D (down).",
"",
"",
"",
"          PRESS ANY KEY NOW TO RETURN TO THE EVENT LIST",
};

char *help_text2[] =
{
"The details shown are for the event you selected by pressing the",
"S key.",
"",
"If this event is of interest to you, you may wish to save the",
"details in a local Unix file.  To do this, return to the detail",
"display and press the F key.  An entry will be added to the file ",
"named \"whatsup.items\".  Pressing any other key will return you",
"to the event list.",
"",
"HINTS:",
"",
"In the case of information about IAP 1986, the event details were",
"adapted from the text found in the printed IAP guide, and the field",
"types don't always make sense.",
"",
"Make a note of the event types codes for things that interest you,",
"as these may be helpful in specifying further search criteria.",
"",
" PRESS CTRL-C (hold down CTRL and press C) ANYTIME TO EXIT WHATSUP",
};

char *help_text3[] =
{
"                          SELECTING TIMES OF DAY",
"",
"The starting and ending times you enter apply on each day being searched.",
"For example, a search from 3PM to 5PM on MON thru FRI searches 2 hours on",
"each of 5 days.",
"",
"The following are examples of legal time specifications as you would",
"type them:",
"",
"                  Specification            Refers to this time",
"                  -------------            -------------------",
"",
"                  5 or 5A or 5:00                5AM",
"                  5P or 5:00P or 17:00           5PM",
"                  8:15 or 8:15A               8:15AM",
"                  0 or 12M                  Midnight",
"                  12 or 12N or 12:00N           Noon",
"",
"Other variations are possible too.  Complete your entry by pressing RETURN.",
"BACKSPACE to correct or ESC to revert to previous value of the field.",
};

char *help_text4[] =
{
"                           SELECTING DATE RANGES",
"",
"You may specify the first and last day to be searched.  ALL DATES ARE",
"PRESUMED TO BE IN THE FUTURE;  BE CAREFUL, THE DATE YOU THINK OF AS",
"`YESTERDAY' THE CALENDAR WILL CONSIDER TO BE NEXT YEAR!",
"",
"The following are examples of legal date specifications as you would",
"type them:",
"",
"                  Specification            Refers to this date",
"                  -------------            -------------------",
"",
"                  3/15                         March 15th",
"                  15                           15th of same month as before",
"",
"You may also type over the day of the week, which is often a more",
"convenient way to specify dates.  Just type the first letter of the",
"correct day of the week.  Starting days are presumed to be within",
"the coming 7 days, ending days are presumed to follow starting days.",
};

char *help_text5[] =
{
"",
"                      COMPLETING A DAY OF WEEK SPECIFICATION",
"",
"You are in the middle of specifying a day of the week.  Most days",
"are uniquely identified by a single letter, such as \"M\" for",
"\"Monday\".",
"",
"In the cases of SAT, SUN, TUE, and THU a second letter is needed.",
"Press any key now to return to the prompt, and type the appropriate",
"second letter.  To give up, press the ESC key (which is F11 on",
"some of the DEC keyboards.)",
};

char *help_text6[] =
{
"                  SEARCHING FOR SPECIFIC TYPES OF EVENTS",
"",
"If you leave the type field blank, then the calendar will find",
"ALL events occurring within the range of dates and times you specify.",
"If you choose to fill in the type field, then only events of the",
"specified type will be retrieved.  Because searching for a specific",
"type tends to limit the amount of information returned, it is often",
"practical to search periods as long as a month or more when a type",
"is specified.",
"",
"There are several problems with type searching in this first version of",
"whatsup.  Each event has only one type, and you must guess it exactly",
"for whatsup to find it.  If you specify \"film\" and the event is listed",
"as a \"movie\", it will not be found.  Unfortunately, there isn't a list",
"of event types yet.  Still, when you find an event listed in the calendar",
"it's often useful to look for other events with the same type.",
"",
"For IAP events, the types are derived from the sections in the printed",
"IAP guide.  We hope to improve the handling of event types in future",
"releases of whatsup.",
};

char **helps[] = {help_text0, help_text1, help_text2, help_text3, help_text4, help_text5, help_text6};
int help_sizes[] = {sizeof(help_text0)/sizeof(char *),
		  sizeof(help_text1)/sizeof(char *),
		  sizeof(help_text2)/sizeof(char *),
		  sizeof(help_text3)/sizeof(char *),
		  sizeof(help_text4)/sizeof(char *),
		  sizeof(help_text5)/sizeof(char *),
		  sizeof(help_text6)/sizeof(char *),
	  };
#define HELP_HEAD " HOW TO USE WHATSUP "
#define HELP_FOOT " PRESS ANY KEY NOW TO CONTINUE "
/*
 * Called when the help key is pressed
 */
int
GetHelp(fld, c, arg)
int fld;
char c;
int arg;
{
	WINDOW *w;
	register int i;
	char **help = helps[arg];
	int size = help_sizes[arg];
	int height = min(LINES-2, 23);

       /*
        * Create a new window
        */
	w = newwin(height,COLS-1, 2, 0);

	for (i=0; i<size; i++) {
		(void) wmove(w, 2+i,2);
		(void) waddstr(w, help[i]);
	}

	box(w, '?', '?');

	(void) wmove(w, 0, (COLS-sizeof(HELP_HEAD))/2);
	(void) waddstr(w, HELP_HEAD);
	(void) wmove(w, height-1 , (COLS-sizeof(HELP_FOOT))/2);
	(void) waddstr(w, HELP_FOOT);

       /*
        * Present screen, wait for a character, then revert and exit
        */
	(void) wrefresh(w);
	getch();

	delwin(w);
	touchwin(stdscr);
	refresh();

}

/*
 * 			re_alloc_string
 * 
 * 	Frees an existing string and reallocates it to hold 
 * 	a new one.
 * 
 * 	Note: allocation and freeing are done using the GDB
 * 	db_alloc and db_free routines.
 */

int
re_alloc_string(targp, src)
char **targp;
char *src;
{
       /*
        * Only if there was a string there, de-allocate it
        */
	if (*targp != NULL) {
		db_free(*targp, strlen(*targp)+1);
	}

       /*
        * Allocate space for the new string.
        */
	*targp = db_alloc(strlen(src)+1);
       /*
        * Copy the new string
        */
	(void) strcpy(*targp, src);
}

/*
 *			hello_prompt
 *
 *	Before really getting going, tell the user what his/her
 *	options are.  Latest version doesn't give the user any options.
 */
int
hello_prompt()
{

	re_alloc_string(&gdb_uname, "**WHATSUP**");
	re_alloc_string(&gdb_host,  "**NOHOST**");
	return;

}
