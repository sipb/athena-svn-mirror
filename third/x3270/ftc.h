/*
 * Modifications Copyright 1996 by Paul Mattes.
 * Copyright October 1995 by Dick Altenbern
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	ftc.h
 *		Global declarations for ft.c.
 */

extern Boolean ascii_flag;
extern Boolean cr_flag;
extern unsigned long ft_length;
extern FILE *ft_local_file;
extern char *ft_local_filename;
enum ft_state {
	FT_NONE,	/* No transfer in progress */
	FT_AWAIT_ACK,	/* IND$FILE sent, awaiting acknowledgement message */
	FT_RUNNING,	/* Ack received, data flowing */
	FT_ABORT_WAIT,	/* Awaiting chance to send an abort */
	FT_ABORT_SENT	/* Abort sent; awaiting response */
	};
extern enum ft_state ft_state;

extern void popup_ft();

extern void dialog_focus_action();
extern void dialog_next_action();

extern void ft_aborting();
extern void ft_complete();
extern void ft_disconnected();
extern void ft_not3270();
extern void ft_running();
extern void ft_update_length();
