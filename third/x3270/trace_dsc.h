/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	trace_dsc.h
 *		Global declarations for trace_ds.c.
 */

extern char *rcba();
extern char *see_aid();
extern char *see_attr();
extern char *see_color();
extern char *see_ebc();
extern char *see_efa();
extern char *see_efa_only();
extern char *see_qcode();
extern void toggle_dsTrace();
extern void toggle_eventTrace();
extern void toggle_screenTrace();
extern FILE *tracef;
extern void trace_char();
#if defined(__STDC__)
extern void trace_ds(char *fmt, ...);
#else
extern void trace_ds();
#endif
extern void trace_screen();
extern void trace_ansi_disc();
