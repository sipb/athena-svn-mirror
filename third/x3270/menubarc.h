/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	menubarc.h
 *		Global declarations for menubar.c.
 */

extern void Connect_action();
extern void Disconnect_action();
extern void HandleMenu_action();
extern void hostfile_init();
extern int hostfile_lookup();
extern void menubar_as_set();
extern void menubar_connect();
extern void menubar_gone();
extern void menubar_init();
extern void menubar_keypad_changed();
extern void menubar_newmode();
extern Dimension menubar_qheight();
extern void menubar_resize();
extern void menubar_retoggle();
extern void Reconnect_action();
