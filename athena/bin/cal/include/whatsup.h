/************************************************************************/
/*	
/*			whatsup.h
/*	
/*	Shared information for the whatsup calendar package.
/*	
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/whatsup.h,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/whatsup.h,v 1.1 1993-10-12 05:35:20 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/************************************************************************/

#include "gdb.h"

/*
 *			Data delcarations
 */

#define BELL '\007'
#define ESC  '\033'
#define BS   '\010'
#define DEL  '\177'
#define RETURN '\r'

#define UPCASE(c) {if (c>='a' && c<='z') c += 'A' - 'a';}


/*----------------------------------------------------------------
 |                                                               |
 |                  Dirty indicators for screen areas            |
 |                                                               |
 *---------------------------------------------------------------*/

extern short dirty_bits;	/* which screen areas need repainting */
#define MAIN_DIRTY	0x0001
#define INPUT_DIRTY 	0x0002
#define PLACE_DIRTY 	0x0004
#define EVENT_DIRTY	0x0008

/*----------------------------------------------------------------
 |                                                               |
 |                  Display format switches                      |
 |                                                               |
 *---------------------------------------------------------------*/

extern short screen_format;
#define SHOW_EVENTS	0x0001

/*----------------------------------------------------------------
 |                                                               |
 |                  Screen Layout Declarations                   |
 |                                                               |
 |      These presume a 24x80 screen, all we support right       |
 |      now.                                                     |
 |                                                               |
 *---------------------------------------------------------------*/

/*
 * start and end time prompt fields
 */
#define MESSAGE_X 5
#define MESSAGE_Y 4

#define TIME_BOX_X 0
#define TIME_BOX_Y 6
#define TIME_BOX_H 7
#define TIME_BOX_W 37

#define DATE_BOX_X 40
#define DATE_BOX_Y 6
#define DATE_BOX_H 7
#define DATE_BOX_W 37

#define TYPE_BOX_X 0
#define TYPE_BOX_Y 14
#define TYPE_BOX_H 7
#define TYPE_BOX_W 77

#define DATE2_L_X (DATE_BOX_X+4)
#define DATE2_L_Y (DATE_BOX_Y+2)
#define DATE2_L_T ""
#define DATE2_LEN sizeof(DATE2_L_T)
#define START_L_X DATE_BOX_X+8
#define START_L_Y DATE_BOX_Y+3
#define START_L_T "Start:"
#define END_L_X (START_L_X+sizeof(START_L_T)-sizeof(END_L_T))
#define END_L_Y START_L_Y+1
#define END_L_T "End:"
#define DAY_L_X (START_L_X + sizeof(START_L_T)+1)
#define DAY_L_Y (START_L_Y-1)
#define DAY_L_T "Day"
#define DAY_LEN 3
#define DATE_L_X (DAY_L_X + DAY_LEN+1)
#define DATE_L_Y (START_L_Y-1)
#define DATE_L_T "Date"
#define DATE_LEN 5
#define TIME2_L_X (TIME_BOX_X+4)
#define TIME2_L_Y (TIME_BOX_Y+2)
#define TIME2_L_T "Find events occuring between:"
#define TIME2_LEN sizeof(TIME2_L_T)
#define TIME_L_X (TIME_BOX_X+15)
#define TIME_L_Y (TIME_BOX_Y+4)
#define TIME_L_T "and"
#define TIME_LEN 3
#define HEADER_T "EXPERIMENTAL MIT/PROJECT ATHENA ONLINE EVENTS CALENDAR (Ver. 0.5)"
#define HEADER_X ((80-sizeof(HEADER_T))/2)
#define HEADER_Y 0
#define CTRLC_T "  To exit from `whatsup', type CTRL-C (hold down CTRL and press 'C') anytime   "
#define CTRLC_X (0)
#define CTRLC_Y 22
#define PRIVACY_T "   PRIVACY NOTICE: Whatsup no longer logs your name, identity or location.     "  
#define PRIVACY_X (0)
#define PRIVACY_Y 22
#define QMARK_T "PRESS '?' ANYTIME FOR INSTRUCTIONS"
#define QMARK_X ((80-sizeof(QMARK_T))/2)
#define QMARK_Y 2
#define TYPE_L_X (TYPE_BOX_X+3)
#define TYPE_L_Y (TYPE_BOX_Y+2)
#define TYPE_L_T "Find only events of this type.   Leave blank to search for all types."
#define TYPE1_L_X (TYPE_L_X+20)
#define TYPE1_L_Y (TYPE_L_Y+2)
#define TYPE1_L_T "=>"
#define TYPE2_L_X (TYPE1_L_X)
#define TYPE2_L_Y (TYPE1_L_Y+1)
#define TYPE2_L_T "2)"


#define START_DAY_P_X (START_L_X + sizeof(START_L_T))
#define START_DAY_P_Y START_L_Y
#define END_DAY_P_X (END_L_X + sizeof(END_L_T))
#define END_DAY_P_Y END_L_Y
#define START_DATE_P_X (DATE_L_X)
#define START_DATE_P_Y START_L_Y
#define END_DATE_P_X (START_DATE_P_X)
#define END_DATE_P_Y END_L_Y
#define START_TIME_P_X (TIME_L_X-7)
#define START_TIME_P_Y (TIME_L_Y)
#define END_TIME_P_X (TIME_L_X+TIME_LEN+1)
#define END_TIME_P_Y (START_TIME_P_Y)
#define TYPE1_P_X (TYPE1_L_X+3)
#define TYPE1_P_Y (TYPE1_L_Y)
#define TYPE2_P_X (TYPE_L_X)
#define TYPE2_P_Y (TYPE2_L_Y)

/*----------------------------------------------------------------
 |                                                               |
 |                  Output Field Declarations                    |
 |                                                               |
 *---------------------------------------------------------------*/
/*
 * Symbolic names for output fields
 */
#define START_L 0
#define END_L 1
#define DATE_L 2
#define TIME_L 3
#define HEADER 4
#define DAY_L 5
#define TYPE_L 6
#define TIME2_L 7
#define DATE2_L 8
#define TYPE1_L 7
/***** #define TYPE2_L 8   ***/
#define NOFIELDS 13

struct ofl_def {
	int x;				/* x coordinate (0 origin) */
	int y;				/* y    "        "    "    */
	char *text;			/* data with which to prompt */
	int hilit;				/* true to reverse video */
};

extern struct ofl_def o_field[NOFIELDS];
extern int vNOFIELDS;				/* variable form of */
						/* NOFIELDS
*/

/*----------------------------------------------------------------
 |                                                               |
 |                  Input field declarations                     |
 |                                                               |
 *---------------------------------------------------------------*/
/*
 * Symbolic names for input fields
 */
#define START_DAY_P 0
#define END_DAY_P 1
#define START_DATE_P 2
#define END_DATE_P 3
#define START_TIME_P 4
#define END_TIME_P 5
#define TYPE1_P 6
/**** #define TYPE2_P 7  **/
#define NIFIELDS 7

/*
 * Following should really use unions, but then we couldn't statically
 * initialize.
 */

struct ifl_def {
	int x;				/* x coordinate (0 origin) */
	int y;				/* y    "        "    "    */
	int len;			/* length of the prompt area */
	int type;			/* what kind of data (see below) */
	int value;			/* all input values are internally
					   coded as ints */
	int old_value;			/* temp value while decoding input,
					   maybe does not belong here */
	int relative;			/* index of field to be coordinated
					   with this one (e.g. day and date)*/
};

extern struct ifl_def i_field[NIFIELDS];
/*
 *	A Macro to Move Window Pointer to start of field
 */
#define GO_TO_FIELD(f) move((f).y, (f).x)
#define GO_TO_FIELD_OFFSET(f, off) move((f).y, (f).x + (off))

/*
 * Types of prompting fields
 */
#define SDAY 0
#define EDAY 1
#define SDATE 2
#define EDATE 3
#define STIME 4
#define ETIME 5
#define TYPE  6
/*
 * Lengths of various types of prompting fields
 */
#define DAY_LEN 3
#define DATE_LEN 5
#define TIME_LENG 5
#define TYPE_LEN 20


/*----------------------------------------------------------------
 |                                                               |
 |                  Results of mapping input char                |
 |   Cursor motion characters must come first in the table.      |
 |                                                               |
 *---------------------------------------------------------------*/

#define MYSTERY_CHAR (-1)		/* not a global command char */
#define FTAB 0				/* forward tab */
#define BSP 1				/* backspace */
#define CR 2				/* carriage return */
#define UPARROW 3				
#define DOWNARROW 4
#define LEFTARROW 5
#define RIGHTARROW 6
#define AST 7				/* asterisk */
#define REF 8				/* refresh character-generally
					   ctk-l */
#define ALPHA 9				/* any alpha (lower only)*/
#define NUM 10				/* any numeric */
#define BLANK 11					/* a blank char */
#define QMARK 12				/* question mark */

#define NUMBER_OF_KEY_TYPES 13
struct k_map_entry {
	char c;				/* char as input from keyboard */
	int def;			/* logical definition */
};

#define SIZE_OF_KEYBOARD_MAP 15
#define SIZE_OF_CURSOR_KEYBOARD_MAP 7

extern struct k_map_entry k_map[SIZE_OF_KEYBOARD_MAP];

extern char pre_read_char;			/* character to be processed */
						/* as global input */
extern int current_field;

extern int done;				/* time to exit? */

struct t_struct {
	int year;
	int month;
	int day;			/* 1-31 */
	int hour;
	int minute;
};

extern struct t_struct t_struct, t_struct2;	/* for playing with time */
extern int	today;			/* number of day in week (0-6) */
extern int     this_day;		/* number of day in month */
extern int     this_month;		/* number of the current month */
extern int     this_year;		/* number of the current year (yy) */
extern int	time_window;		/* number of hours to search
					   when not explicitly given */

struct p_tr {
	int ((*f)());			/* function to call */
	char *arg;			/* argument to pass */
};

extern struct p_tr *current_psm;		/* initial one is main */
extern struct p_tr main_psm[NUMBER_OF_KEY_TYPES];
extern struct p_tr select_psm[NUMBER_OF_KEY_TYPES];
extern struct p_tr item_psm[NUMBER_OF_KEY_TYPES];

extern int tr_tab[NIFIELDS][SIZE_OF_CURSOR_KEYBOARD_MAP];

#define NDEP 8
extern int i_dep[NDEP][2];

/************************************************************************/
/*	
/*			Selection Window Maintenance
/*	
/************************************************************************/

struct sel_state {
	WINDOW *w;
	RELATION rel;
	int length;				/* number of tuples */
	int top;				/* 0 based tuple number */
						/* on top line */
	int bump;				/* amount and direciton */
						/* of next scroll */
	int nlines;				/* number of lines */
						/* displayable */
	int selected;				/* TUPLE number (not line) */
						/* of last selected.  -1 */
						/* for none. */
	int curs_line;				/* 0 based tuple number */
						/* where we'd like the cursor */
						/* to be */
	int force;				/* true iff we are going */
						/* to force a repaint */
						/* of the tuple lines*/
};

#define SEL_LINE (TIME_BOX_Y)

/************************************************************************/
/*	
/*			Item Display Window Maintenance
/*	
/************************************************************************/

struct itm_state {
	WINDOW *w;
	TUPLE tup;
};

#define ITM_LINE SEL_LINE

/************************************************************************/
/*	
/*			query processing
/*	
/************************************************************************/

extern TUPLE_DESCRIPTOR query_desc;		/* holds the descriptor */
						/* for the relations */
						/* we retrieve */

/************************************************************************/
/*	
/*			Help Subwindow
/*	
/************************************************************************/

#define HELP_LINE 22
#define HELP_COL 3

struct help_dat {
	char **text;				/* lines to put in */
						/* window */
	int count;				/* number of lines */
};

extern WINDOW *help_window;


/************************************************************************/
/*	
/*			misc
/*	
/************************************************************************/

extern int written;				/* we appended something */
						/* to the items file */
#define RETRIEVE_LIMIT 100			/* don't get more tuples */
						/* than this */
extern int limited;				/* is retrieve limit set? */
