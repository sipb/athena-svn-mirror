/*
 * Copyright 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 */

/*
 *	telnetc.h
 *		Global declarations for telnet.c.
 */

/* Output buffer. */
extern unsigned char *obuf, *obptr;

/* Spelled-out tty control character. */
struct ctl_char {
	char *name;
	char value[3];
};

extern void net_add_eor();
extern void net_break();
extern void net_charmode();
extern int net_connect();
extern void net_disconnect();
extern void net_exception();
extern void net_input();
extern void net_linemode();
extern struct ctl_char *net_linemode_chars();
extern void net_output();
extern void net_sendc();
extern void net_sends();
extern void net_send_erase();
extern void net_send_kill();
extern void net_send_werase();
extern Boolean net_snap_options();
extern void space3270out();
extern void trace_netdata();
