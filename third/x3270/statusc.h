/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	statusc.h
 *		Global declarations for status.c.
 */

extern void status_compose();
extern void status_connect();
extern void status_ctlr_done();
extern void status_ctlr_init();
extern void status_cursor_pos();
extern void status_disp();
extern void status_init();
extern void status_insert_mode();
extern void status_kmap();
extern void status_kybdlock();
extern void status_oerr();
extern void status_reset();
extern void status_reverse_mode();
extern void status_script();
extern void status_scrolled();
extern void status_shift_mode();
extern void status_syswait();
extern void status_timing();
extern void status_touch();
extern void status_twait();
extern void status_typeahead();
extern void status_uncursor_pos();
extern void status_untiming();
