/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	actionsc.h
 *		Global declarations for actions.c.
 */

/* types of internal actions */
enum iaction {
	IA_STRING, IA_PASTE, IA_REDRAW,
	IA_KEYPAD, IA_DEFAULT, IA_KEY,
	IA_MACRO, IA_SCRIPT, IA_PEEK,
	IA_TYPEAHEAD, IA_FT
};
extern enum iaction ia_cause;

extern int              actioncount;
extern XtActionsRec     actions[];

extern char	       *ia_name[];

extern void action_debug();
extern void action_internal();
extern char *action_name();
extern int check_usage();
