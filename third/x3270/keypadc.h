/*
 * Copyright 1995, 1996 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	keypadc.h
 *		Global declarations for keypad.c.
 */

extern Dimension keypad_qheight();
extern Dimension min_keypad_width();
extern void keypad_first_up();
extern Widget keypad_init();
extern void keypad_move();
extern void keypad_popup_init();
extern Dimension keypad_qheight();
extern void keypad_set_keymap();
extern void keypad_set_temp_keymap();
extern void keypad_shift();
extern void ReparentNotify_action();
