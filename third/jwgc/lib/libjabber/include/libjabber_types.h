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

/* $Id: libjabber_types.h,v 1.1.1.2 2006-03-10 15:35:16 ghudson Exp $ */

#ifndef _LIBJABBER_TYPES_H_
#define _LIBJABBER_TYPES_H_ 1

#include <gssapi.h>

/* --------------------------------------------------------- */
/* jid.c                                                     */
/* JID structures & constants                                */
/* --------------------------------------------------------- */
#define JID_RESOURCE 1
#define JID_USER     2
#define JID_SERVER   4

typedef struct jid_struct { 
	xode_pool		p;
	char			*resource;
	char			*user;
	char			*server;
	char			*full;
	struct jid_struct	*next;	/* for lists of jids */
} *jid;
  


/* --------------------------------------------------------- */
/* jpacket.c                                                 */
/* JPacket structures & constants                            */
/* --------------------------------------------------------- */
#define JABPACKET_UNKNOWN		0
#define JABPACKET_MESSAGE		1
#define JABPACKET_PRESENCE		2
#define JABPACKET_IQ			3
#define JABPACKET_SASL_CHALLENGE	4
#define JABPACKET_SASL_SUCCESS		5
#define JABPACKET_SASL_FAILURE		6

#define JABPACKET__UNKNOWN	0
#define JABPACKET__NONE		1
#define JABPACKET__ERROR	2
#define JABPACKET__CHAT		3
#define JABPACKET__GROUPCHAT	4
#define JABPACKET__GET		5
#define JABPACKET__SET		6
#define JABPACKET__RESULT	7
#define JABPACKET__SUBSCRIBE	8
#define JABPACKET__SUBSCRIBED	9
#define JABPACKET__UNSUBSCRIBE	10
#define JABPACKET__UNSUBSCRIBED	11
#define JABPACKET__AVAILABLE	12
#define JABPACKET__UNAVAILABLE	13
#define JABPACKET__PROBE	14
#define JABPACKET__HEADLINE	15
#define JABPACKET__INVISIBLE	16
#define JABPACKET__INTERNAL	17

typedef struct jabpacket_struct {
	int			type;
	int			subtype;
	int			flag;
	void			*aux1;
	xode			x;
	jid			to;
	jid			from;
	char			*iqns;
	xode			iq;
	xode_pool		p;
} *jabpacket, _jabpacket;
 


/* --------------------------------------------------------- */
/* jconn.c                                                   */
/* JConn structures & functions                              */
/* --------------------------------------------------------- */
#define JABCONN_STATE_OFF	0
#define JABCONN_STATE_CONNECTED	1
#define JABCONN_STATE_ON	2
#define JABCONN_STATE_AUTH	3

typedef struct jabconn_struct {
	/* Core structure */
	xode_pool		p;		/* Memory allocation pool */
	int			state;		/* Connection state flag */
	int			fd;		/* Connection file descriptor */
	int			dumpfd;		/* FD to dump output to */
	char			*dumpid;	/* ID to dump output of */
#ifdef USE_SSL
	int			sslfd;		/* SSL file descriptor */
	SSL			*ssl;		/* SSL connection */
	SSL_CTX			*ssl_ctx;	/* SSL context */
#endif /* USE_SSL */
	jid			user;		/* User info */
	char			*server;	/* Server hostname */
	int			port;		/* Port to connect to */

	/* Capability tracking */
	int			auth_password;	/* Password auth supported */
	int			auth_digest;	/* Digest auth supported */
	int			auth_0k;	/* 0k auth supported */

	/* Stream stuff */
	int			id;		/* Id counter */
	char			idbuf[9];	/* Temporary storage */
	char			*sid;		/* Stream id */
	int			sequence;	/* Sequence id */
	char			*token;		/* Token id */
	XML_Parser		parser;		/* Parser instance */
	xode			current;	/* Current parsing node */
	int			endcount;	/* Count of stream end tags */

	/* GSSAPI authentication */
	gss_ctx_id_t		gsscontext;	/* GSSAPI context */
	gss_name_t		gssserver;	/* GSSAPI server name */
	char			*gsstoken;	/* Challenge token */
	int			gsscomplete;	/* Completed flag */
	int			gsssuccess;	/* Success flag */

	/* Event callback ptrs */
	void (*on_state)(struct jabconn_struct *j, int state);
	void (*on_packet)(struct jabconn_struct *j, jabpacket p);
} *jabconn, jabconn_struct;

typedef void (*jabconn_state_h)(jabconn j, int state);
typedef void (*jabconn_packet_h)(jabconn j, jabpacket p);



/* --------------------------------------------------------- */
/* jutil.c                                                   */
/* JUtil functions, types, and defines                       */
/* --------------------------------------------------------- */

/* Error structures & constants                              */
typedef struct terror_struct
{
	int			code;
	char			msg[64];
} terror;

#define TERROR_BAD		(terror){400,"Bad Request"}
#define TERROR_AUTH		(terror){401,"Unauthorized"}
#define TERROR_PAY		(terror){402,"Payment Required"}
#define TERROR_FORBIDDEN	(terror){403,"Forbidden"}
#define TERROR_NOTFOUND		(terror){404,"Not Found"}
#define TERROR_NOTALLOWED	(terror){405,"Not Allowed"}
#define TERROR_NOTACCEPTABLE	(terror){406,"Not Acceptable"}
#define TERROR_REGISTER		(terror){407,"Registration Required"}
#define TERROR_REQTIMEOUT	(terror){408,"Request Timeout"}
#define TERROR_CONFLICT		(terror){409,"Conflict"}
#define TERROR_INTERNAL		(terror){500,"Internal Server Error"}
#define TERROR_NOTIMPL		(terror){501,"Not Implemented"}
#define TERROR_EXTERNAL		(terror){502,"Remote Server Error"}
#define TERROR_UNAVAIL		(terror){503,"Service Unavailable"}
#define TERROR_EXTTIMEOUT	(terror){504,"Remote Server Timeout"}
#define TERROR_DISCONNECTED	(terror){510,"Disconnected"}

/* Namespace constants                                       */
#define NSCHECK(x,n) (j_strcmp(xmlnode_get_attrib(x,"xmlns"),n) == 0)

#define NS_CLIENT	"jabber:client"
#define NS_SERVER	"jabber:server"
#define NS_AUTH		"jabber:iq:auth"
#define NS_REGISTER	"jabber:iq:register"
#define NS_ROSTER	"jabber:iq:roster"
#define NS_OFFLINE	"jabber:x:offline"
#define NS_AGENT	"jabber:iq:agent"
#define NS_AGENTS	"jabber:iq:agents"
#define NS_DELAY	"jabber:x:delay"
#define NS_VERSION	"jabber:iq:version"
#define NS_TIME		"jabber:iq:time"
#define NS_VCARD	"vcard-temp"
#define NS_PRIVATE	"jabber:iq:private"
#define NS_SEARCH	"jabber:iq:search"
#define NS_OOB		"jabber:iq:oob"
#define NS_XOOB		"jabber:x:oob"
#define NS_ADMIN	"jabber:iq:admin"
#define NS_FILTER	"jabber:iq:filter"
#define NS_AUTH_0K	"jabber:iq:auth:0k"
#define NS_BROWSE	"jabber:iq:browse"
#define NS_EVENT	"jabber:x:event"
#define NS_CONFERENCE	"jabber:iq:conference"
#define NS_SIGNED	"jabber:x:signed"
#define NS_ENCRYPTED	"jabber:x:encrypted"
#define NS_GATEWAY	"jabber:iq:gateway"
#define NS_LAST		"jabber:iq:last"
#define NS_ENVELOPE	"jabber:x:envelope"
#define NS_EXPIRE	"jabber:x:expire"
#define NS_XHTML	"http://www.w3.org/1999/xhtml"
#define NS_XDBGINSERT	"jabber:xdb:ginsert"
#define NS_XDBNSLIST	"jabber:xdb:nslist"
#define NS_DATA		"jabber:x:data"
#define NS_SASL		"urn:ietf:params:xml:ns:xmpp-sasl"
#define NS_BIND		"urn:ietf:params:xml:ns:xmpp-bind"
#define NS_SESSION	"urn:ietf:params:xml:ns:xmpp-session"

/* Message Types                                             */
#define TMSG_NORMAL	"normal"
#define TMSG_ERROR	"error"
#define TMSG_CHAT	"chat"
#define TMSG_GROUPCHAT	"groupchat"
#define TMSG_HEADLINE	"headline"


#endif /* _LIBJABBER_TYPES_H_ */
