/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Jabber
 *  Copyright (C) 1998-1999 The Jabber Team http://jabber.org/
 *
 */

/* $Id: libjabber.h,v 1.1.1.1 2006-03-10 15:32:57 ghudson Exp $ */

#ifndef _LIBJABBER_H_
#define _LIBJABBER_H_ 1

#include <sysdep.h>
#include "libxode.h"
#include "libjabber_types.h"
#include "libjwgc_types.h"



/* --------------------------------------------------------- */
/* jid.c                                                     */
/* JID structures & constants                                */
/* --------------------------------------------------------- */
jid	jid_new(xode_pool p, char *idstr);
	/* Creates a jabber id from the idstr */
void	jid_set(jid id, char *str, int item);
	/* Individually sets jid components */
char	*jid_full(jid id);
	/* Builds a string type=user/resource@server from the jid data */
jid	jid_user(jid a);
int	jid_cmp(jid a, jid b);
	/* Compares two jid's, returns 0 for perfect match */
int	jid_cmpx(jid a, jid b, int parts);
	/* Compares just the parts specified as JID_|JID_ */
jid	jid_append(jid a, jid b);
	/* Appending b to a (list), no dups */
xode	jid_xres(jid id);
	/* Returns xode representation of the resource?query=string */
xode	jid_nodescan(jid id, xode x);
	/* Scans the children of the node for a matching jid attribute */



/* --------------------------------------------------------- */
/* jpacket.c                                                 */
/* JPacket structures & constants                            */
/* --------------------------------------------------------- */
jabpacket	jabpacket_new(xode x);
	/* Creates a jabber packet from the xode */
jabpacket	jabpacket_reset(jabpacket p);
	/* Resets the jpacket values based on the xode */
int		jabpacket_subtype(jabpacket p);
	/* Returns the subtype value (looks at xode for it) */



/* --------------------------------------------------------- */
/* jconn.c                                                   */
/* JConn structures & functions                              */
/* --------------------------------------------------------- */
jabconn	jab_new(char *user, char *pass);
void	jab_delete(jabconn j);
void	jab_state_handler(jabconn j, jabconn_state_h h);
void	jab_packet_handler(jabconn j, jabconn_packet_h h);
void	jab_start(jabconn j, int ssl);
void	jab_startup(jabconn j);
void	jab_stop(jabconn j);
int	jab_getfd(jabconn j);
jid	jab_getjid(jabconn j);
char	*jab_getsid(jabconn j);
char	*jab_getid(jabconn j);
void	jab_send(jabconn j, xode x);
void	jab_send_raw(jabconn j, const char *str);
void	jab_recv(jabconn j);
void	jab_poll(jabconn j, int timeout);
char	*jab_auth(jabconn j);
char	*jab_reg(jabconn j);



/* --------------------------------------------------------- */
/* jutil.c                                                   */
/* JUtil functions, types, and defines                       */
/* --------------------------------------------------------- */
xode	jabutil_presnew(int type, char *to, char *status, int priority);
	/* Create a skeleton presence packet */
xode	jabutil_iqnew(int type, char *ns);
	/* Create a skeleton iq packet */
xode	jabutil_msgnew(char *type, char *to, char *subj, char *body, char *encrypt);
	/* Create a skeleton message packet */
xode	jabutil_pingnew(char *type, char *to);
	/* Create a skeleton composing packet */
xode	jabutil_header(char* xmlns, char* server);
	/* Create a skeleton stream packet */
int	jabutil_priority(xode x);
	/* Determine priority of this packet */
void	jabutil_tofrom(xode x);
	/* Swaps to/from fields on a packet */
xode	jabutil_iqresult(xode x);
	/* Generate a skeleton iq/result, given a iq/query */
char	*jabutil_timestamp(void);
	/* Get stringified timestamp */
void	jabutil_error(xode x, terror E);
	/* Append an <error> node to x */
void	jabutil_delay(xode msg, char *reason);
	/* Append a delay packet to msg */
char	*jabutil_regkey(char *key, char *seed);
	/* pass a seed to generate a key, pass the key again to */
	/* validate (returns it) */
char	*jab_type_to_ascii(int j_type);
	/* converts a numeric type to an ascii string */
char	*jab_subtype_to_ascii(int j_subtype);
	/* converts a numeric subtype to an ascii string */
char	*jab_contype_to_ascii(int j_contype);
	/* converts a numeric connection type to an ascii string */


#endif /* _LIBJABBER_H_ */
