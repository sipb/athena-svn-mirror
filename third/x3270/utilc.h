/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	utilc.h
 *		Global declarations for util.c.
 */

extern char *ctl_see();
extern char *do_subst();
extern char *get_message();
extern char *get_resource();
extern char *local_strerror();
extern int split_dresource();
extern int split_lresource();
#if defined(__STDC__)
extern char *xs_buffer(char *fmt, ...);
extern void xs_warning(char *fmt, ...);
extern void xs_error(char *fmt, ...);
#else
extern char *xs_buffer();
extern void xs_warning();
extern void xs_error();
#endif
