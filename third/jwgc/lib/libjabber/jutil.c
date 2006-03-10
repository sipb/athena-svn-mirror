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

/* $Id: jutil.c,v 1.1.1.1 2006-03-10 15:32:57 ghudson Exp $ */

#include "libjabber.h"
#include "libjwgc.h"
#include "libxode.h"

/* util for making presence packets */
xode 
jabutil_presnew(int type, char *to, char *status, int priority)
{
	xode pres;
	char pribuff[32];

	pres = xode_new("presence");
	switch (type) {
		case JABPACKET__SUBSCRIBE:
			xode_put_attrib(pres, "type", "subscribe");
			break;
		case JABPACKET__UNSUBSCRIBE:
			xode_put_attrib(pres, "type", "unsubscribe");
			break;
		case JABPACKET__SUBSCRIBED:
			xode_put_attrib(pres, "type", "subscribed");
			break;
		case JABPACKET__UNSUBSCRIBED:
			xode_put_attrib(pres, "type", "unsubscribed");
			break;
		case JABPACKET__PROBE:
			xode_put_attrib(pres, "type", "probe");
			break;
		case JABPACKET__UNAVAILABLE:
			xode_put_attrib(pres, "type", "unavailable");
			break;
		case JABPACKET__INVISIBLE:
			xode_put_attrib(pres, "type", "invisible");
			break;
	}
	if (to != NULL)
		xode_put_attrib(pres, "to", to);
#ifdef USE_GPGME
	if (*(int *)jVars_get(jVarUseGPG)) {
		char *signature;
		char *tmpstatus;

		if (status == NULL) {
			tmpstatus = "online";
		}
		else {
			tmpstatus = status;
		}

		signature = JTrimPGPMessage(JSign(tmpstatus));
		if (signature) {
			xode x;

			dprintf(dExecution, "Adding status signature.\n");
			x = xode_insert_tag(pres, "x");
			xode_put_attrib(x, "xmlns", NS_SIGNED);
			xode_insert_cdata(x, signature, strlen(signature));
			status = tmpstatus;
		}
	}
#endif /* USE_GPGME */
	if (status != NULL)
		xode_insert_cdata(xode_insert_tag(pres, "status"), status, strlen(status));
	if (priority >= 0) {
		snprintf(pribuff, sizeof(pribuff), "%d", priority);
		xode_insert_cdata(xode_insert_tag(pres, "priority"), pribuff, strlen(pribuff));
	}

	return pres;
}

/* util for making IQ packets */
xode 
jabutil_iqnew(int type, char *ns)
{
	xode iq;

	iq = xode_new("iq");
	switch (type) {
		case JABPACKET__GET:
			xode_put_attrib(iq, "type", "get");
			break;
		case JABPACKET__SET:
			xode_put_attrib(iq, "type", "set");
			break;
		case JABPACKET__RESULT:
			xode_put_attrib(iq, "type", "result");
			break;
		case JABPACKET__ERROR:
			xode_put_attrib(iq, "type", "error");
			break;
	}
	xode_put_attrib(xode_insert_tag(iq, "query"), "xmlns", ns);

	return iq;
}

/* util for making message packets */
xode 
jabutil_msgnew(char *type, char *to, char *subj, char *body, char *encrypt)
{
	xode msg;

	msg = xode_new("message");
	xode_put_attrib(msg, "type", type);
	xode_put_attrib(msg, "to", to);

	if (subj) {
		xode_insert_cdata(xode_insert_tag(msg, "subject"), subj, strlen(subj));
	}

	if (encrypt) {
		xode x;

		x = xode_insert_tag(msg, "x");
		xode_put_attrib(x, "xmlns", NS_ENCRYPTED);
		xode_insert_cdata(x, encrypt, strlen(encrypt));
	}

	{
		xode x;

		x = xode_insert_tag(msg, "x");
		xode_put_attrib(x, "xmlns", NS_EVENT);
		xode_insert_tag(x, "composing");
	}

	if (body) {
		xode_insert_cdata(xode_insert_tag(msg, "body"), body, strlen(body));
	}

	return msg;
}

/* util for making message ping/composing packets */
xode 
jabutil_pingnew(char *type, char *to)
{
	xode msg;
	xode x, y;

	msg = xode_new("message");
	xode_put_attrib(msg, "type", type);
	xode_put_attrib(msg, "to", to);

	x = xode_insert_tag(msg, "x");
	xode_put_attrib(x, "xmlns", NS_EVENT);
	xode_insert_tag(x, "composing");

	return msg;
}

/* util for making stream packets */
xode 
jabutil_header(char *xmlns, char *server)
{
	xode result;
	if ((xmlns == NULL) || (server == NULL))
		return NULL;
	result = xode_new("stream:stream");
	xode_put_attrib(result, "xmlns:stream", "http://etherx.jabber.org/streams");
	xode_put_attrib(result, "xmlns", xmlns);
	xode_put_attrib(result, "to", server);

	return result;
}

/* returns the priority on a presence packet */
int 
jabutil_priority(xode x)
{
	char *str;
	int p;

	if (x == NULL)
		return -1;

	if (xode_get_attrib(x, "type") != NULL)
		return -1;

	x = xode_get_tag(x, "priority");
	if (x == NULL)
		return 0;

	str = xode_get_data((x));
	if (str == NULL)
		return 0;

	p = atoi(str);
	if (p >= 0)
		return p;
	else
		return 0;
}

void 
jabutil_tofrom(xode x)
{
	char *to, *from;

	to = xode_get_attrib(x, "to");
	from = xode_get_attrib(x, "from");
	xode_put_attrib(x, "from", to);
	xode_put_attrib(x, "to", from);
}

xode 
jabutil_iqresult(xode x)
{
	xode cur;

	jabutil_tofrom(x);

	xode_put_attrib(x, "type", "result");

	/* hide all children of the iq, they go back empty */
	for (cur = xode_get_firstchild(x); cur != NULL; cur = xode_get_nextsibling(cur))
		xode_hide(cur);

	return x;
}

char *
jabutil_timestamp(void)
{
	time_t t;
	struct tm *new_time;
	static char timestamp[18];
	int ret;

	t = time(NULL);

	if (t == (time_t) - 1)
		return NULL;
	new_time = gmtime(&t);

	ret = snprintf(timestamp, 18, "%d%02d%02dT%02d:%02d:%02d", 1900 + new_time->tm_year,
		 new_time->tm_mon + 1, new_time->tm_mday, new_time->tm_hour,
		       new_time->tm_min, new_time->tm_sec);

	if (ret == -1)
		return NULL;

	return timestamp;
}

void 
jabutil_error(xode x, terror E)
{
	xode err;
	char code[4];

	xode_put_attrib(x, "type", "error");
	err = xode_insert_tag(x, "error");

	snprintf(code, 4, "%d", E.code);
	xode_put_attrib(err, "code", code);
	if (E.msg != NULL)
		xode_insert_cdata(err, E.msg, strlen(E.msg));

	jabutil_tofrom(x);
}

void 
jabutil_delay(xode msg, char *reason)
{
	xode delay;

	delay = xode_insert_tag(msg, "x");
	xode_put_attrib(delay, "xmlns", NS_DELAY);
	xode_put_attrib(delay, "from", xode_get_attrib(msg, "to"));
	xode_put_attrib(delay, "stamp", jabutil_timestamp());
	if (reason != NULL)
		xode_insert_cdata(delay, reason, strlen(reason));
}

#define KEYBUF 100

char *
jabutil_regkey(char *key, char *seed)
{
	static char keydb[KEYBUF][41];
	static char seeddb[KEYBUF][41];
	static int last = -1;
	char *str, strint[32];
	int i;

	/* blanket the keydb first time */
	if (last == -1) {
		last = 0;
		memset(&keydb, 0, KEYBUF * 41);
		memset(&seeddb, 0, KEYBUF * 41);
		srand(time(NULL));
	}

	/* creation phase */
	if (key == NULL && seed != NULL) {
		/* create a random key hash and store it */
		sprintf(strint, "%d", rand());
		strcpy(keydb[last], j_shahash(strint));

		/* store a hash for the seed associated w/ this key */
		strcpy(seeddb[last], j_shahash(seed));

		/* return it all */
		str = keydb[last];
		last++;
		if (last == KEYBUF)
			last = 0;
		return str;
	}

	/* validation phase */
	str = j_shahash(seed);
	for (i = 0; i < KEYBUF; i++)
		if (j_strcmp(keydb[i], key) == 0 && j_strcmp(seeddb[i], str) == 0) {
			seeddb[i][0] = '\0';	/* invalidate this key */
			return keydb[i];
		}

	return NULL;
}

char *
jab_type_to_ascii(j_type)
	int j_type;
{
	char *result;

	switch (j_type) {
		case JABPACKET_UNKNOWN:
			result = "unknown";
			break;

                case JABPACKET_MESSAGE:
			result = "message";
			break;

		case JABPACKET_PRESENCE:
			result = "presence";
			break;

		case JABPACKET_IQ:
			result = "iq";
			break;
                
		default:
			result = "<unknown type>";
			break;
	}
        
	return (strdup(result));
}

char *
jab_subtype_to_ascii(j_subtype)
	int j_subtype;
{       
	char *result;
        
	switch (j_subtype) {
		case JABPACKET__UNKNOWN:
			result = "unknown";
			break;
                
		case JABPACKET__NONE:
			result = "none";
			break;
                
		case JABPACKET__ERROR:
			result = "error";
			break;
                
		case JABPACKET__CHAT:
			result = "chat";
			break;
                
		case JABPACKET__GROUPCHAT:
			result = "groupchat";
			break;
                
		case JABPACKET__INTERNAL:
			result = "internal";
			break;

		case JABPACKET__GET:
			result = "get";
			break;

		case JABPACKET__SET:
			result = "set";
			break;

		case JABPACKET__RESULT:
			result = "result";
			break;

		case JABPACKET__SUBSCRIBE:
			result = "subscribe";
			break;

		case JABPACKET__UNSUBSCRIBE:
			result = "unsubscribe";
			break;

		case JABPACKET__UNSUBSCRIBED:
			result = "unsubscribed";
			break;

		case JABPACKET__AVAILABLE:
			result = "available";
			break;

		case JABPACKET__UNAVAILABLE:
			result = "unavailable";
			break;

		case JABPACKET__PROBE:
			result = "probe";
			break;

		case JABPACKET__HEADLINE:
			result = "headline";
			break;

		default:
			result = "<unknown subtype>";
			break;
	}

	return (strdup(result));
}

char *
jab_contype_to_ascii(j_contype)
	int j_contype;
{
	char *result;   
        
	switch (j_contype) {
		case JABCONN_STATE_OFF:
			result = "off";
			break;

		case JABCONN_STATE_CONNECTED:
			result = "connected";
			break;
        
		case JABCONN_STATE_ON:
			result = "on";
			break;
        
		case JABCONN_STATE_AUTH:
			result = "auth";
			break;

		default:
			result = "<unknown connection type>";
			break;
	}

	return (strdup(result));
}
