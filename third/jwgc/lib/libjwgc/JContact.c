/* $Id: JContact.c,v 1.1.1.2 2006-03-24 16:59:39 ghudson Exp $ */

#include "libjwgc.h"

xode *contact_list = NULL;
int num_contacts = 0;

xode *agent_list = NULL;
int num_agents = 0;

void
insert_resource_into_contact(int pos, char *resource, char *status)
{
	int i, k;
	xode r, s;

	if (!resource) {
		return;
	}

	r = xode_get_tag(contact_list[pos], "resources");
	s = xode_insert_tag(r, "resource");
	xode_put_attrib(s, "name", resource);
	if (status) {
		xode_put_attrib(s, "status", status);
	}
}

void 
insert_into_contact_list(char *contact, char *status, char *resource)
{
	int i, k;

	num_contacts++;
	contact_list = realloc(contact_list, sizeof(xode) * (num_contacts));
	contact_list[num_contacts - 1] = malloc(sizeof(xode));

	for (i = 0; i < (num_contacts - 1) &&
	     (strcasecmp(contact, xode_get_attrib(contact_list[i], "jid")) > 0);
	     i++);

	for (k = (num_contacts - 1); k > i; k--) {
		contact_list[k] = contact_list[k - 1];
	}

	contact_list[k] = xode_new("contact");
	xode_put_attrib(contact_list[k], "jid", contact);
	xode_insert_tag(contact_list[k], "resources");

	insert_resource_into_contact(i, resource, status);

	dprintf(dExecution, "Inserted %s into contact list at position %d\n", contact, i);
}

void 
remove_from_contact_list(char *contact)
{
	int i;
	int k;

	for (i = 0; i < num_contacts &&
	    (strcasecmp(contact, xode_get_attrib(contact_list[i], "jid")) != 0);
	    i++);

	if (i == num_contacts) {
		return;
	}

	free(contact_list[i]);
	for (k = i; k < (num_contacts - 1); k++) {
		contact_list[k] = contact_list[k + 1];
	}
	num_contacts--;

	dprintf(dExecution, "Removed %s from contact list at position %d\n", contact, i);
}

void 
insert_into_agent_list(char *jid, char *name, char *service, int flags)
{
	int i, k;

	if (!jid) {
		return;
	}

	num_agents++;
	agent_list = realloc(agent_list, sizeof(xode) * (num_agents));
	agent_list[num_agents - 1] = malloc(sizeof(xode));

	for (i = 0; i < (num_agents - 1) &&
	     (strcasecmp(jid, xode_get_attrib(agent_list[i], "jid")) > 0);
	     i++);

	for (k = (num_agents - 1); k > i; k--) {
		agent_list[k] = agent_list[k - 1];
	}

	agent_list[k] = xode_new("agent");
	xode_put_attrib(agent_list[k], "jid", jid);
	if (name) {
		xode_put_attrib(agent_list[k], "name", name);
	}
	if (service) {
		xode_put_attrib(agent_list[k], "service", service);
	}
	if (flags & AGENT_TRANSPORT) {
		xode_insert_tag(agent_list[k], "transport");
	}
	if (flags & AGENT_GROUPCHAT) {
		xode_insert_tag(agent_list[k], "groupchat");
	}
	if (flags & AGENT_REGISTER) {
		xode_insert_tag(agent_list[k], "register");
	}
	if (flags & AGENT_SEARCH) {
		xode_insert_tag(agent_list[k], "search");
	}

	dprintf(dExecution, "Inserted %s into agent list at positiion %d\n", jid, i);
}

int 
update_contact_status(char *jid, char *status, char *resource)
{
	int contactfound = 0;
	int resourcefound = 0;
	int i;
	char *pos;
	xode x, y;
	int ret = 0;

	dprintf(dExecution, "Updating %s/%s status to %s.\n",
			jid,
			resource ? resource : "NULL",
			status ? status : "NULL");

	for (i = 0; i < num_contacts; i++) {
		if (!strcasecmp(xode_get_attrib(contact_list[i], "jid"),
							jid)) {
			contactfound = 1;
			if (!resource) {
				break;
			}
			x = xode_get_tag(contact_list[i], "resources");
			y = xode_get_firstchild(x);
			while (y) {
				if (!strcmp(xode_get_attrib(y, "name"),
								resource)) {
					char *curstatus;
					resourcefound = 1;

					curstatus = xode_get_attrib(y,
								"status");
					if (status && (!curstatus ||
						strcmp(curstatus, status))) {
						xode_put_attrib(y,
							"status",
							status);
						ret = 1;
					}
					break;
				}
				y = xode_get_nextsibling(y);
			}

			if (!resourcefound) {
				insert_resource_into_contact(i,
						resource, status);
				ret = 1;
			}

			break;
		}
	}

	if (!contactfound) {
		insert_into_contact_list(jid, status, resource);
		ret = 1;
	}

	return ret;
}

int 
contact_status_change(jabpacket packet)
{
	char *temp, *pos;
	xode x;
	char *from = xode_get_attrib(packet->x, "from");
	char *resource = "NULL";
	int ret = 0;
	if (!from) {
		return;
	}

	pos = (char *) strchr(from, '/');
	if (pos) {
		*pos = '\0';
		resource = pos + 1;
	}

	switch (packet->subtype) {
		case JABPACKET__AVAILABLE:
			x = xode_get_tag(packet->x, "show");
			if (x && (temp = xode_get_data(x))) {
				update_contact_status(from, temp, resource);
				ret = 1;
			}
			else {
				update_contact_status(from, "available", resource);
				ret = 1;
			}

			dprintf(dExecution, "%s is available\n", from);
			break;

		case JABPACKET__UNAVAILABLE:
			dprintf(dExecution, "%s is unavailable\n", from);
			update_contact_status(from, "unavailable", resource);
			ret = 1;
			break;

		default:
			dprintf(dExecution, "%s is unknown(?) -> %d\n", from, packet->subtype);
			update_contact_status(from, "unknown", resource);
			ret = 1;
			break;
	}
}

void 
list_contacts(jwg, matchstr, strictmatch, skipnotavail)
	jwgconn jwg;
	char *matchstr;
	int strictmatch;
	int skipnotavail;
{
	int i, k;
	xode x, y, ny, z, r, nr;

	x = xode_new("contacts");
	for (i = 0; i < num_contacts; i++) {
		y = xode_dup(contact_list[i]);
		xode_insert_node(x, y);
	}

	if (matchstr) {
		ny = y = xode_get_firstchild(x);
		while (ny) {
			ny = xode_get_nextsibling(ny);
			if (!test_match(matchstr, y, strictmatch)) {
				dprintf(dXML, "HIDE-A:\n%s\n",
					xode_to_prettystr(y));
				xode_hide(y);
			}
			y = ny;
		}
	}

	if (skipnotavail) {
		ny = y = xode_get_firstchild(x);
		while (ny) {
			z = xode_get_tag(y, "resources");
			nr = r = xode_get_firstchild(z);
			while (nr) {
				char *curstatus;
				nr = xode_get_nextsibling(nr);
				curstatus = xode_get_attrib(r, "status");
				if (!curstatus || !strcmp(curstatus,
						"unavailable")) {
					dprintf(dXML, "HIDE-B:\n%s\n",
						xode_to_prettystr(r));
					xode_hide(r);
				}
				r = nr;
			}
			ny = xode_get_nextsibling(ny);
			if (!xode_has_children(z)) {
				dprintf(dXML, "HIDE-C:\n%s\n",
					xode_to_prettystr(y));
				xode_hide(y);
			}
			y = ny;
		}
	}

	jwg_servsend(jwg, x);
	xode_free(x);
}

xode 
find_contact_group(base, target)
	xode base;
	char *target;
{
	xode x;
	char *name;

	if (!target) {
		return NULL;
	}

	x = xode_get_tag(base, "group");
	while (x) {
		name = xode_get_attrib(x, "name");
		if (name && !strcmp(name, target)) {
			return x;
		}
		x = xode_get_nextsibling(x);
	}

	return NULL;
}

void 
list_contacts_bygroup(jwg, matchstr, strictmatch, skipnotavail)
	jwgconn jwg;
	char *matchstr;
	int strictmatch;
	int skipnotavail;
{
	int i, k;
	xode x, y, ny, z, r, nr, g, ng;
	char *group;

	x = xode_new("contacts");
	for (i = 0; i < num_contacts; i++) {
		y = xode_dup(contact_list[i]);
		group = xode_get_attrib(y, "group");
		if (!group) {
			group = "Unknown";
		}
		z = find_contact_group(x, group);
		if (!z) {
			z = xode_insert_tag(x, "group");
			xode_put_attrib(z, "name", group);
		}
		xode_hide_attrib(y, "group");
		xode_insert_node(z, y);
	}

	if (matchstr) {
		ng = g = xode_get_firstchild(x);
		while (ng) {
			ny = y = xode_get_firstchild(g);
			while (ny) {
				ny = xode_get_nextsibling(y);
				if (!test_match(matchstr, y, strictmatch)) {
					dprintf(dXML,
						"HIDE-D:\n%s\n",
						xode_to_prettystr(y));
					xode_hide(y);
				}
				y = ny;
			}

			ng = xode_get_nextsibling(ng);
			if (!xode_has_children(g)) {
				dprintf(dXML, "HIDE-E:\n%s\n",
					xode_to_prettystr(g));
				xode_hide(g);
			}
			g = ng;
		}
	}

	if (skipnotavail) {
		ng = g = xode_get_firstchild(x);
		while (ng) {
			ny = y = xode_get_firstchild(g);
			while (ny) {
				z = xode_get_tag(y, "resources");
				nr = r = xode_get_firstchild(z);
				while (nr) {
					char *curstatus;
					nr = xode_get_nextsibling(nr);
					curstatus = xode_get_attrib(r,
								"status");
					if (!curstatus || !strcmp(curstatus,
							"unavailable")) {
						dprintf(dXML,
							"HIDE-F:\n%s\n",
							xode_to_prettystr(r));
						xode_hide(r);
					}
					r = nr;
				}

				ny = xode_get_nextsibling(ny);
				if (!xode_has_children(z)) {
					dprintf(dXML,
						"HIDE-G:\n%s\n",
						xode_to_prettystr(y));
					xode_hide(y);
				}
				y = ny;
			}

			ng = xode_get_nextsibling(ng);
			if (!xode_has_children(g)) {
				dprintf(dXML, "HIDE-H:\n%s\n",
					xode_to_prettystr(g));
				xode_hide(g);
			}
			g = ng;
		}
	}

	jwg_servsend(jwg, x);
	xode_free(x);
}

void 
list_agents(xode x)
{
	int i;
	xode y, z;

	y = xode_insert_tag(x, "agents");
	for (i = 0; i < num_agents; i++) {
		z = xode_dup(agent_list[i]);
		xode_insert_node(y, z);
	}
}

void 
update_nickname(char *target, char *nickname)
{
	int i;

	for (i = 0; i < num_contacts; i++) {
		if (!strcasecmp(xode_get_attrib(contact_list[i], "jid"),
								target)) {
			xode_put_attrib(contact_list[i], "nick", nickname);
			return;
		}
	}
}

void 
update_group(char *target, char *group)
{
	int i;

	for (i = 0; i < num_contacts; i++) {
		if (!strcasecmp(xode_get_attrib(contact_list[i], "jid"),
								target)) {
			xode_put_attrib(contact_list[i], "group", group);
			return;
		}
	}
}

char *
find_jid_from_nickname(char *nickname)
{
	int i;
	char *name;

	for (i = 0; i < num_contacts; i++) {
		name = xode_get_attrib(contact_list[i], "nick");
		if (name && !strcasecmp(name, nickname)) {
			return strdup(xode_get_attrib(contact_list[i], "jid"));
		}
	}

	return NULL;
}

char *
find_nickname_from_jid(char *jid)
{
	int i;
	char *name, *nick;

	for (i = 0; i < num_contacts; i++) {
		name = xode_get_attrib(contact_list[i], "jid");
		if (name && !strcasecmp(name, jid)) {
			nick = xode_get_attrib(contact_list[i], "nick");
			if (nick) {
				return strdup(nick);
			}
			else {
				return strdup("");
			}
		}
	}

	return NULL;
}

int 
test_match(matchstr, contact, exactmatch)
	char *matchstr;
	xode contact;
	int exactmatch;
{
	char *nick;

	if (!strcasecmp(xode_get_attrib(contact, "jid"), matchstr)) {
		return 1;
	}

	if ((nick = xode_get_attrib(contact, "nick")) != NULL &&
			!strcasecmp(nick, matchstr)) {
		return 1;
	}

	if (!exactmatch && !strncasecmp(xode_get_attrib(contact, "jid"),
					matchstr, strlen(matchstr))) {
		return 1;
	}

	return 0;
}

int
contact_exists(char *jid)
{
	int i;
	char *cjid;

	/* Try exact jid match */
	for (i = 0; i < num_contacts; i++) {
		cjid = xode_get_attrib(contact_list[i], "jid");
		if (!strcasecmp(jid, cjid)) {
			return 1;
		}
	}
	return 0;
}

char *
find_match(char *searchstr)
{
	int i;
	char *jid, *nick;

	/* Try exact jid match */
	for (i = 0; i < num_contacts; i++) {
		jid = xode_get_attrib(contact_list[i], "jid");
		dprintf(dMatch, "Exact Match: %s = %s\n", searchstr, jid);
		if (!strcasecmp(jid, searchstr)) {
			return strdup(jid);
		}
	}

	/* Try exact nickname match */
	for (i = 0; i < num_contacts; i++) {
		jid = xode_get_attrib(contact_list[i], "jid");
		nick = xode_get_attrib(contact_list[i], "nick");
		if (!nick) {
			continue;
		}
		dprintf(dMatch, "Nick Match: %s = %s\n", searchstr, nick);
		if (!strcasecmp(nick, searchstr)) {
			return strdup(jid);
		}
	}

	/* Try begging of jid match */
	for (i = 0; i < num_contacts; i++) {
		jid = xode_get_attrib(contact_list[i], "jid");
		dprintf(dMatch, "Part Match: %s = %s\n", searchstr, jid);
		if (!strncasecmp(jid, searchstr, strlen(searchstr))) {
			return strdup(jid);
		}
	}

	/* No matches, sorry */
	return NULL;
}
