/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	selectc.h
 *		Global declarations for select.c.
 */

extern Boolean area_is_selected();
extern void Cut_action();
extern void MoveCursor_action();
extern void move_select_action();
extern void reclass();
extern void select_end_action();
extern void select_extend_action();
extern void select_start_action();
extern void set_select_action();
extern void start_extend_action();
extern void unselect();
