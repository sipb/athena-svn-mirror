/* $Id: rkinitd.h,v 1.1 1999-10-05 17:09:59 danw Exp $ */

/* Copyright 1989,1999 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This header file contains function declarations for use for rkinitd. */

#ifndef __RKINITD_H__
#define __RKINITD_H__

int get_tickets(int);
void error(void);
int setup_rpc(int);
void rpc_exchange_version_info(int *, int *, int, int);
void rpc_get_rkinit_info(rkinit_info *);
void rpc_send_error(char *);
void rpc_send_success(void);
void rpc_exchange_tkt(KTEXT, MSG_DAT *);
void rpc_getauth(KTEXT, struct sockaddr_in *, struct sockaddr_in *);
int choose_version(int *);

#endif /* __RKINITD_H__ */
