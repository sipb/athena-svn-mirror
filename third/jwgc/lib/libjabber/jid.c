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

/* $Id: jid.c,v 1.1.1.2 2006-03-24 16:59:41 ghudson Exp $ */

#include "libjabber.h"

jid 
jid_safe(jid id)
{
	unsigned char *str;

	if (strlen(id->server) == 0 || strlen(id->server) > 255)
		return NULL;

	/* lowercase the hostname, make sure it's valid characters */
	for (str = id->server; *str != '\0'; str++) {
		*str = tolower(*str);
		if (!(isalnum(*str) || *str == '.' || *str == '-' || *str == '_'))
			return NULL;
	}

	/* cut off the user */
	if (id->user != NULL && strlen(id->user) > 64)
		id->user[64] = '\0';

	/* check for low and invalid ascii characters in the username */
	if (id->user != NULL)
		for (str = id->user; *str != '\0'; str++)
			if (*str <= 32 || *str == ':' || *str == '@' || *str == '<' || *str == '>' || *str == '\'' || *str == '"' || *str == '&')
				return NULL;

	return id;
}

jid 
jid_new(xode_pool p, char *idstr)
{
	char *server, *resource, *type, *str;
	jid id;

	if (p == NULL || idstr == NULL || strlen(idstr) == 0)
		return NULL;

	/* user@server/resource */

	str = xode_pool_strdup(p, idstr);

	id = xode_pool_malloco(p, sizeof(struct jid_struct));
	id->p = p;
	id->full = xode_pool_strdup(p, idstr);

	resource = strstr(str, "/");
	if (resource != NULL) {
		*resource = '\0';
		++resource;
		if (strlen(resource) > 0)
			id->resource = resource;
	}
	else {
		resource = str + strlen(str);	/* point to end */
	}

	type = strstr(str, ":");
	if (type != NULL && type < resource) {
		*type = '\0';
		++type;
		str = type;	/* ignore the type: prefix */
	}

	server = strstr(str, "@");
	if (server == NULL || server > resource) {	/* if there's no @, it's
							 * just the server
							 * address */
		id->server = str;
	}
	else {
		*server = '\0';
		++server;
		id->server = server;
		if (strlen(str) > 0)
			id->user = str;
	}

	return jid_safe(id);
}

void 
jid_set(jid id, char *str, int item)
{
	char *old;

	if (id == NULL)
		return;

	/* invalidate the cached copy */
	id->full = NULL;

	switch (item) {
		case JID_RESOURCE:
			if (str != NULL && strlen(str) != 0)
				id->resource = xode_pool_strdup(id->p, str);
			else
				id->resource = NULL;
			break;
		case JID_USER:
			old = id->user;
			if (str != NULL && strlen(str) != 0)
				id->user = xode_pool_strdup(id->p, str);
			else
				id->user = NULL;
			if (jid_safe(id) == NULL)
				id->user = old;	/* revert if invalid */
			break;
		case JID_SERVER:
			old = id->server;
			id->server = xode_pool_strdup(id->p, str);
			if (jid_safe(id) == NULL)
				id->server = old;	/* revert if invalid */
			break;
	}

}

char *
jid_full(jid id)
{
	xode_spool s;

	if (id == NULL)
		return NULL;

	/* use cached copy */
	if (id->full != NULL)
		return id->full;

	s = xode_spool_newfrompool(id->p);

	if (id->user != NULL)
		xode_spooler(s, id->user, "@", s);

	xode_spool_add(s, id->server);

	if (id->resource != NULL)
		xode_spooler(s, "/", id->resource, s);

	id->full = xode_spool_tostr(s);
	return id->full;
}

char *
jid_bare(jid id)
{
	xode_spool s;

	if (id == NULL)
		return NULL;

	/* use cached copy */
	if (id->bare != NULL)
		return id->bare;

	s = xode_spool_newfrompool(id->p);

	if (id->user != NULL)
		xode_spooler(s, id->user, "@", s);

	xode_spool_add(s, id->server);

	id->bare = xode_spool_tostr(s);
	return id->bare;
}

/*
 * parses a /resource?name=value&foo=bar into an xode representing <resource
 * name="value" foo="bar"/>
 */
xode 
jid_xres(jid id)
{
	char *cur, *qmark, *amp, *eq;
	xode x;

	if (id == NULL || id->resource == NULL)
		return NULL;

	cur = xode_pool_strdup(id->p, id->resource);
	qmark = strstr(cur, "?");
	if (qmark == NULL)
		return NULL;
	*qmark = '\0';
	qmark++;

	x = xode_new_frompool(id->p, cur);

	cur = qmark;
	while (cur != '\0') {
		eq = strstr(cur, "=");
		if (eq == NULL)
			break;
		*eq = '\0';
		eq++;

		amp = strstr(eq, "&");
		if (amp != NULL) {
			*amp = '\0';
			amp++;
		}

		xode_put_attrib(x, cur, eq);

		if (amp != NULL)
			cur = amp;
		else
			break;
	}

	return x;
}

/* local utils */
int 
_jid_nullstrcmp(char *a, char *b)
{
	if (a == NULL && b == NULL)
		return 0;
	if (a == NULL || b == NULL)
		return -1;
	return strcmp(a, b);
}
int 
_jid_nullstrcasecmp(char *a, char *b)
{
	if (a == NULL && b == NULL)
		return 0;
	if (a == NULL || b == NULL)
		return -1;
	return strcasecmp(a, b);
}

int 
jid_cmp(jid a, jid b)
{
	if (a == NULL || b == NULL)
		return -1;

	if (_jid_nullstrcmp(a->resource, b->resource) != 0)
		return -1;
	if (_jid_nullstrcasecmp(a->user, b->user) != 0)
		return -1;
	if (_jid_nullstrcmp(a->server, b->server) != 0)
		return -1;

	return 0;
}

/* suggested by Anders Qvist <quest@valdez.netg.se> */
int 
jid_cmpx(jid a, jid b, int parts)
{
	if (a == NULL || b == NULL)
		return -1;

	if (parts & JID_RESOURCE && _jid_nullstrcmp(a->resource, b->resource) != 0)
		return -1;
	if (parts & JID_USER && _jid_nullstrcasecmp(a->user, b->user) != 0)
		return -1;
	if (parts & JID_SERVER && _jid_nullstrcmp(a->server, b->server) != 0)
		return -1;

	return 0;
}

/* makes a copy of b in a's pool, requires a valid a first! */
jid 
jid_append(jid a, jid b)
{
	jid next;

	if (a == NULL)
		return NULL;

	if (b == NULL)
		return a;

	next = a;
	while (next != NULL) {
		/* check for dups */
		if (jid_cmp(next, b) == 0)
			break;
		if (next->next == NULL)
			next->next = jid_new(a->p, jid_full(b));
		next = next->next;
	}
	return a;
}

xode 
jid_nodescan(jid id, xode x)
{
	xode cur;
	xode_pool p;
	jid tmp;

	if (id == NULL || xode_get_firstchild(x) == NULL)
		return NULL;

	p = xode_pool_new();
	for (cur = xode_get_firstchild(x); cur != NULL; cur = xode_get_nextsibling(cur)) {
		if (xode_get_type(cur) != XODE_TYPE_TAG)
			continue;

		tmp = jid_new(p, xode_get_attrib(cur, "jid"));
		if (tmp == NULL)
			continue;

		if (jid_cmp(tmp, id) == 0)
			break;
	}
	xode_pool_free(p);

	return cur;
}
