/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	ctlr.h
 *		External declarations for ctlr.c data structures.
 */

extern int		buffer_addr;	/* buffer address */
extern int		cursor_addr;	/* cursor address */
extern struct ea	*ea_buf;	/* 3270 extended attribute buffer */
extern Boolean		formatted;	/* contains at least one field? */
extern Boolean		is_altbuffer;	/* in alternate-buffer mode? */
extern unsigned char	*screen_buf;	/* 3270 display buffer */
