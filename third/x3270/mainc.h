/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	mainc.h
 *		Global declarations for main.c.
 */

extern void invert_icon();
extern void lock_icon();
extern void relabel();
extern void setup_keymaps();
extern void x3270_exit();
extern int x_connect();
extern void x_connected();
extern void x_disconnect();
extern void x_except_off();
extern void x_except_on();
extern Status x_get_window_attributes();
extern void x_in3270();
extern void x_reconnect();
