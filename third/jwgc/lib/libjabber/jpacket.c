/*
 * --------------------------------------------------------------------------
 * 
 * License
 * 
 * The contents of this file are subject to the Jabber Open Source License
 * Version 1.0 (the "JOSL").  You may not copy or use this file, in either
 * source code or executable form, except in compliance with the JOSL. You
 * may obtain a copy of the JOSL at http://www.jabber.org/ or at
 * http://www.opensource.org/.
 * 
 * Software distributed under the JOSL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the JOSL for
 * the specific language governing rights and limitations under the JOSL.
 * 
 * Copyrights
 * 
 * Portions created by or assigned to Jabber.com, Inc. are Copyright (c)
 * 1999-2002 Jabber.com, Inc.  All Rights Reserved.  Contact information for
 * Jabber.com, Inc. is available at http://www.jabber.com/.
 * 
 * Portions Copyright (c) 1998-1999 Jeremie Miller.
 * 
 * Acknowledgements
 * 
 * Special thanks to the Jabber Open Source Contributors for their suggestions
 * and support of Jabber.
 * 
 * Alternatively, the contents of this file may be used under the terms of the
 * GNU General Public License Version 2 or later (the "GPL"), in which case
 * the provisions of the GPL are applicable instead of those above.  If you
 * wish to allow use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * JOSL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the GPL.  If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the JOSL or the GPL.
 * 
 * --------------------------------------------------------------------------
 */

/* $Id: jpacket.c,v 1.1.1.2 2006-03-10 15:35:16 ghudson Exp $ */

#include "libjabber.h"
#include "libxode.h"

jabpacket 
jabpacket_new(xode x)
{
	jabpacket p;

	if (x == NULL)
		return NULL;

	p = xode_pool_malloc(xode_get_pool(x), sizeof(_jabpacket));
	p->x = x;

	return jabpacket_reset(p);
}

jabpacket 
jabpacket_reset(jabpacket p)
{
	char *val;
	xode x;

	x = p->x;
	memset(p, 0, sizeof(_jabpacket));
	p->x = x;
	p->p = xode_get_pool(x);

	if (strncmp(xode_get_name(x), "message", 7) == 0) {
		p->type = JABPACKET_MESSAGE;
	}
	else if (strncmp(xode_get_name(x), "presence", 8) == 0) {
		p->type = JABPACKET_PRESENCE;
	}
	else if (strncmp(xode_get_name(x), "iq", 2) == 0) {
		p->type = JABPACKET_IQ;
		p->iq = xode_get_tag(x, "?xmlns");
		p->iqns = xode_get_attrib(p->iq, "xmlns");
	}
	else if (strncmp(xode_get_name(x), "challenge", 9) == 0) {
		p->type = JABPACKET_SASL_CHALLENGE;
	}
	else if (strncmp(xode_get_name(x), "success", 7) == 0) {
		p->type = JABPACKET_SASL_SUCCESS;
	}
	else if (strncmp(xode_get_name(x), "failure", 7) == 0) {
		p->type = JABPACKET_SASL_FAILURE;
	}

	val = xode_get_attrib(x, "to");
	if (val != NULL)
		if ((p->to = jid_new(p->p, val)) == NULL)
			p->type = JABPACKET_UNKNOWN;
	val = xode_get_attrib(x, "from");
	if (val != NULL)
		if ((p->from = jid_new(p->p, val)) == NULL)
			p->type = JABPACKET_UNKNOWN;

	p->subtype = JABPACKET__UNKNOWN;
	p->subtype = jabpacket_subtype(p);

	return p;
}


int 
jabpacket_subtype(jabpacket p)
{
	char *type;
	int ret = p->subtype;

	if (ret != JABPACKET__UNKNOWN)
		return ret;

	ret = JABPACKET__NONE;
	type = xode_get_attrib(p->x, "type");
	if (j_strcmp(type, "error") == 0)
		ret = JABPACKET__ERROR;
	else
		switch (p->type) {
			case JABPACKET_MESSAGE:
				if (j_strcmp(type, "chat") == 0)
					ret = JABPACKET__CHAT;
				else if (j_strcmp(type, "groupchat") == 0)
					ret = JABPACKET__GROUPCHAT;
				else if (j_strcmp(type, "headline") == 0)
					ret = JABPACKET__HEADLINE;
				else if (j_strcmp(type, "internal") == 0)
					ret = JABPACKET__INTERNAL;
				else
					ret = JABPACKET__UNKNOWN;
				break;

			case JABPACKET_PRESENCE:
				if (type == NULL)
					ret = JABPACKET__AVAILABLE;
				else if (j_strcmp(type, "unavailable") == 0)
					ret = JABPACKET__UNAVAILABLE;
				else if (j_strcmp(type, "probe") == 0)
					ret = JABPACKET__PROBE;
				else if (j_strcmp(type, "error") == 0)
					ret = JABPACKET__ERROR;
				else if (j_strcmp(type, "invisible") == 0)
					ret = JABPACKET__INVISIBLE;
				else if (j_strcmp(type, "available") == 0)
					ret = JABPACKET__AVAILABLE;
				else if (j_strcmp(type, "online") == 0)
					ret = JABPACKET__AVAILABLE;
				else if (j_strcmp(type, "subscribe") == 0)
					ret = JABPACKET__SUBSCRIBE;
				else if (j_strcmp(type, "subscribed") == 0)
					ret = JABPACKET__SUBSCRIBED;
				else if (j_strcmp(type, "unsubscribe") == 0)
					ret = JABPACKET__UNSUBSCRIBE;
				else if (j_strcmp(type, "unsubscribed") == 0)
					ret = JABPACKET__UNSUBSCRIBED;
				else
					ret = JABPACKET__UNKNOWN;
				break;

			case JABPACKET_IQ:
				if (j_strcmp(type, "get") == 0)
					ret = JABPACKET__GET;
				else if (j_strcmp(type, "set") == 0)
					ret = JABPACKET__SET;
				else if (j_strcmp(type, "result") == 0)
					ret = JABPACKET__RESULT;
				else
					ret = JABPACKET__UNKNOWN;
				break;
		}

	return ret;
}
