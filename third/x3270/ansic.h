/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	ansic.h
 *		Global declarations for ansi.c.
 */

extern void ansi_init();
extern void ansi_process();
extern void ansi_send_clear();
extern void ansi_send_down();
extern void ansi_send_home();
extern void ansi_send_left();
extern void ansi_send_pa();
extern void ansi_send_pf();
extern void ansi_send_right();
extern void ansi_send_up();
extern void toggle_lineWrap();
