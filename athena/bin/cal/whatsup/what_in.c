/************************************************************************/
/*	
/*			what_in.c
/*	
/*	Input processing for main screen of whatsup package.
/*	
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_in.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_in.c,v 1.1 1993-10-12 05:34:54 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_in_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/whatsup/what_in.c,v 1.1 1993-10-12 05:34:54 probe Exp $";
#endif

#include "mit-copyright.h"
#include <curses.h>
#include <stdio.h>
#include "whatsup.h"

/*
 *			process_outer_input
 *
 *	Handle one input event, generally a keystroke, and update
 *	the display.  This routine is not used in the middle of 
 *      prompting for structured input, only when doing general
 *	input to the panel.  Because some general input commands
 *	may be entered while in the middle of field prompting, this
 *      routine first checks for and processes a character of that
 *      sort, and only actually reads the keyboard if necessary.
 */
int
process_outer_input(win,docursors)
WINDOW *win;
int docursors;
{
	register char c;
	register int mapped_char;
	int y,x;
	/*
	 * If no input has been read yet, get it from the keyboard
	 */ 
	if ((c=pre_read_char)=='\0') {
		getyx(stdscr, y,x);		/* I wonder why I */
						/* put these here?*/
		c = getch();
		getyx(stdscr, y,x);
	}
	/*
	 * Find out what the character does and do it!
	 */
	mapped_char = map_char(c);

	if (mapped_char == MYSTERY_CHAR) {	/* char does nothing */
		addch('\007' /* BELL */);
		return;
	}

	/*
	 * Some characters activate procedures, if this is one of them,
	 * do it and get out.
	 */
	if (current_psm[mapped_char].f != NULL) {
		(*current_psm[mapped_char].f)(current_field, c, 
					      current_psm[mapped_char].arg);
		return;
	}

	/*
	 * Other characters move the cursor
	 */

	if (docursors)
		do_a_move(mapped_char);
	else 
		addch('\007' /* BELL */);
	return;		
}

/*
 *			map_char
 *
 *	Given a keyboard input character, return its logical definition
 */
int
map_char(c)
char c;
{
	register int i;
	register char cr=c;

	/*
         * Following tests locate alpha and numeric, presume ASCII
	 */
	if ((cr>='0') && (cr<='9'))
		return NUM;
	if (((cr>='a') && (cr<='z')) || ((cr>='A') && (cr<='Z')))
		return ALPHA;

       /*
        * Check for escapes
        */
	if (cr==ESC) {
		cr = getch();			/* get the arrow code */
		cr = getch();			/* get the arrow code */
		switch  (cr) {
		      case  'A':
			return UPARROW;
		      case 'B':
			return DOWNARROW;
		      case 'C':
			return RIGHTARROW;
		      case 'D':
			return LEFTARROW;
		      default:
			return MYSTERY_CHAR;
		}


	}
	/*
	 * Look up the rest in the map
	 */
	for (i=0; i<SIZE_OF_KEYBOARD_MAP; i++)
        	if (c==k_map[i].c)
			return (k_map[i].def);
	return MYSTERY_CHAR;
}

/*
 *			do_a_move
 *
 *	Given a mapped input character, do the cursor motion implied by 
 *	it.
 */
int
do_a_move(mapped_char)
int	mapped_char;
{
	register int target;

	target = tr_tab[current_field][mapped_char];
					/* from current field and input
					   find the field where cursor
					   will go */

	GO_TO_FIELD(i_field[target]);
					/* move the cursor on the screen */
	current_field = target;		/* remember this as the new 
					   current */
	pre_read_char = '\0';		/* clean input buffer */

	MsgClear();
}

