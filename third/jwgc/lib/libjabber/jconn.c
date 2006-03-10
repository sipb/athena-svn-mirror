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

/* $Id: jconn.c,v 1.1.1.1 2006-03-10 15:33:02 ghudson Exp $ */

#include "libjabber.h"

/* local macros for launching event handlers */
#define STATE_EVT(arg) if(j->on_state) { (j->on_state)(j, (arg) ); }

/* prototypes of the local functions */
static void startElement(void *userdata, const char *name, const char **attribs);
static void endElement(void *userdata, const char *name);
static void charData(void *userdata, const char *s, int slen);

/*
 *  jab_new -- initialize a new jabber connection
 *
 *  parameters
 *      user -- jabber id of the user
 *      pass -- password of the user
 *
 *  results
 *      a pointer to the connection structure
 *      or NULL if allocations failed
 */
jabconn 
jab_new(char *user, char *pass)
{
	xode_pool p;
	jabconn j;

	if (!user)
		return (NULL);

	p = xode_pool_new();
	if (!p)
		return (NULL);
	j = xode_pool_mallocx(p, sizeof(jabconn_struct), 0);
	if (!j)
		return (NULL);
	j->p = p;

	j->user = jid_new(p, user);
	j->pass = xode_pool_strdup(p, pass);
	j->port = 5222;

	j->state = JABCONN_STATE_OFF;
	j->id = 1;
	j->fd = -1;
	j->dumpfd = -1;
	j->dumpid = NULL;
#ifdef USE_SSL
	j->ssl = NULL;
	j->ssl_ctx = NULL;
	j->sslfd = -1;
#endif /* USE_SSL */

	return j;
}

/*
 *  jab_delete -- free a jabber connection
 *
 *  parameters
 *      j -- connection
 *
 */
void 
jab_delete(jabconn j)
{
	if (!j)
		return;

	jab_stop(j);
	xode_pool_free(j->p);
}

/*
 *  jab_state_handler -- set callback handler for state change
 *
 *  parameters
 *      j -- connection
 *      h -- name of the handler function
 */
void 
jab_state_handler(jabconn j, jabconn_state_h h)
{
	if (!j)
		return;

	j->on_state = h;
}

/*
 *  jab_packet_handler -- set callback handler for incoming packets
 *
 *  parameters
 *      j -- connection
 *      h -- name of the handler function
 */
void 
jab_packet_handler(jabconn j, jabconn_packet_h h)
{
	if (!j)
		return;

	j->on_packet = h;
}

#ifdef USE_SSL
void 
init_prng()
{
	if (RAND_status() == 0) {
		char rand_file[256];
		time_t t;
		pid_t pid;
		long l, seed;

		t = time(NULL);
		pid = getpid();
		RAND_file_name((char *) &rand_file, 256);
		if (rand_file != NULL) {
			RAND_load_file(rand_file, 1024);
		}
		RAND_seed((unsigned char *) &t, sizeof(time_t));
		RAND_seed((unsigned char *) &pid, sizeof(pid_t));
		RAND_bytes((unsigned char *) &seed, sizeof(long));
		srand48(seed);
		while (RAND_status() == 0) {
			l = lrand48();
			RAND_seed((unsigned char *) &l, sizeof(long));
		}
		if (rand_file != NULL) {
			RAND_write_file(rand_file);
		}
	}
	return;
}
#endif /* USE_SSL */


/*
 *  jab_start -- start connection
 *
 *  parameters
 *      j -- connection
 *
 */
void 
jab_start(jabconn j, int ssl)
{
	xode x;
	char *t, *t2;

	if (!j || j->state != JABCONN_STATE_OFF)
		return;

	j->parser = XML_ParserCreate(NULL);
	XML_SetUserData(j->parser, (void *) j);
	XML_SetElementHandler(j->parser, startElement, endElement);
	XML_SetCharacterDataHandler(j->parser, charData);

	j->fd = make_netsocket(j->port, j->user->server, NETSOCKET_CLIENT);
	if (j->fd < 0) {
		STATE_EVT(JABCONN_STATE_OFF)
			return;
	}

#ifdef USE_SSL
	if (ssl && j->fd >= 0) {
		SSL_library_init();
		SSL_load_error_strings();
		j->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
		if (!j->ssl_ctx) {
			STATE_EVT(JABCONN_STATE_OFF)
				return;
		}
		j->ssl = (SSL *)SSL_new(j->ssl_ctx);
		if (!j->ssl) {
			STATE_EVT(JABCONN_STATE_OFF)
				return;
		}
		SSL_set_fd(j->ssl, j->fd);
		SSL_connect(j->ssl);
		j->sslfd = SSL_get_fd(j->ssl);
		if (j->sslfd < 0) {
			STATE_EVT(JABCONN_STATE_OFF)
				return;
		}
	}
#endif /* USE_SSL */

	j->state = JABCONN_STATE_CONNECTED;
	STATE_EVT(JABCONN_STATE_CONNECTED)
	/* start stream */
	x = jabutil_header(NS_CLIENT, j->user->server);
	t = xode_to_str(x);
	/*
	 * this is ugly, we can create the string here instead of
	 * jutil_header
	 */
	/* what do you think about it? -madcat */
	t2 = strstr(t, "/>");
	*t2++ = '>';
	*t2 = '\0';
	jab_send_raw(j, "<?xml version='1.0'?>");
	jab_send_raw(j, t);
	xode_free(x);

	jab_recv(j);

	j->state = JABCONN_STATE_AUTH;
	STATE_EVT(JABCONN_STATE_AUTH);
}

void
jab_startup(jabconn j) {
	j->state = JABCONN_STATE_ON;
	STATE_EVT(JABCONN_STATE_ON);
}

/*
 *  jab_stop -- stop connection
 *
 *  parameters
 *      j -- connection
 */
void
jab_stop(jabconn j)
{
	if (!j || j->state == JABCONN_STATE_OFF)
		return;

	j->state = JABCONN_STATE_OFF;
#ifdef USE_SSL
	SSL_CTX_free(j->ssl_ctx);
	SSL_free(j->ssl);
	j->ssl = NULL;
	j->sslfd = -1;
#endif /* USE_SSL */
	close(j->fd);
	j->fd = -1;
	j->dumpfd = -1;
	j->dumpid = NULL;
	XML_ParserFree(j->parser);
}

/*
 *  jab_getfd -- get file descriptor of connection socket
 *
 *  parameters
 *      j -- connection
 *
 *  returns
 *      fd of the socket or -1 if socket was not connected
 */
int
jab_getfd(jabconn j)
{
	if (j) {
#ifdef USE_SSL
		if (j->sslfd != -1) {
			return j->sslfd;
		}
		else {
#endif /* USE_SSL */
			return j->fd;
#ifdef USE_SSL
		}
#endif /* USE_SSL */
	}
	else {
		return -1;
	}
}

/*
 *  jab_getjid -- get jid structure of user
 *
 *  parameters
 *      j -- connection
 */
jid 
jab_getjid(jabconn j)
{
	if (j)
		return (j->user);
	else
		return NULL;
}

/*
 * jab_getsid -- get stream id This is the id of server's <stream:stream> tag
 * and used for digest authorization.
 * 
 * parameters j -- connection
 */
char *
jab_getsid(jabconn j)
{
	if (j)
		return (j->sid);
	else
		return NULL;
}

/*
 *  jab_getid -- get a unique id
 *
 *  parameters
 *      j -- connection
 */
char *
jab_getid(jabconn j)
{
	snprintf(j->idbuf, 8, "%d", j->id++);
	return &j->idbuf[0];
}

/*
 *  jab_send -- send xml data
 *
 *  parameters
 *      j -- connection
 *      x -- xode structure
 */
void 
jab_send(jabconn j, xode x)
{
	if (j && j->state != JABCONN_STATE_OFF) {
		char *buf = xode_to_str(x);
#ifdef USE_SSL
		if (j->sslfd == -1) {
#endif /* USE_SSL */
			if (buf)
				write(j->fd, buf, strlen(buf));
#ifdef USE_SSL
		}
		else {
			if (buf)
				SSL_write(j->ssl, buf, strlen(buf));
		}
#endif /* USE_SSL */
		dprintf(dJAB, "out: %s\n", buf);
	}
}

/*
 *  jab_send_raw -- send a string
 *
 *  parameters
 *      j -- connection
 *      str -- xml string
 */
void 
jab_send_raw(jabconn j, const char *str)
{
	if (j && j->state != JABCONN_STATE_OFF)
#ifdef USE_SSL
		if (j->sslfd == -1) {
#endif /* USE_SSL */
			write(j->fd, str, strlen(str));
#ifdef USE_SSL
		}
		else {
			SSL_write(j->ssl, str, strlen(str));
		}
#endif /* USE_SSL */
	dprintf(dJAB, "out: %s\n", str);
}

/*
 *  jab_recv -- read and parse incoming data
 *
 *  parameters
 *      j -- connection
 */
void 
jab_recv(jabconn j)
{
	static char buf[4096];
	int len;
	int errcode;

	if (!j || j->state == JABCONN_STATE_OFF)
		return;

#ifdef USE_SSL
	if (j->sslfd == -1) {
#endif /* USE_SSL */
		len = read(j->fd, buf, sizeof(buf) - 1);
#ifdef USE_SSL
	}
	else {
		len = SSL_read(j->ssl, buf, sizeof(buf) - 1);
	}
#endif /* USE_SSL */
	if (len > 0) {
		buf[len] = '\0';
		dprintf(dJAB, " in: %s\n", buf);
		errcode = XML_Parse(j->parser, buf, len, 0);
		dprintf(dParser, "parser index %d, line %d, col %d\n", XML_GetCurrentByteIndex(j->parser), XML_GetCurrentLineNumber(j->parser), XML_GetCurrentColumnNumber(j->parser));
		if (errcode == 0) {
			dprintf(dParser, "parser error %d at byte %d: %s\n", XML_GetErrorCode(j->parser), XML_GetCurrentByteIndex(j->parser), XML_ErrorString(XML_GetErrorCode(j->parser)));
		}
		else {
			dprintf(dParser, "parser complete\n");
		}
	}
	else if (len < 0) {
		STATE_EVT(JABCONN_STATE_OFF);
		jab_stop(j);
	}
}

/*
 *  jab_poll -- check socket for incoming data
 *
 *  parameters
 *      j -- connection
 *      timeout -- poll timeout
 */
void 
jab_poll(jabconn j, int timeout)
{
	fd_set fds;
	FILE *selfd;
	struct timeval tv;

	if (!j || j->state == JABCONN_STATE_OFF)
		return;

	FD_ZERO(&fds);
#ifdef USE_SSL
	if (j->sslfd == -1) {
#endif /* USE_SSL */
		FD_SET(j->fd, &fds);
		selfd = (FILE *) j->fd;
#ifdef USE_SSL
	}
	else {
		FD_SET(j->sslfd, &fds);
		selfd = (FILE *) j->sslfd;
	}
#endif /* USE_SSL */


	if (timeout < 0) {
		if (select((int) selfd + 1, &fds, NULL, NULL, NULL) > 0)
			jab_recv(j);
	}
	else {
		tv.tv_sec = 0;
		tv.tv_usec = timeout;
		if (select((int) selfd + 1, &fds, NULL, NULL, &tv) > 0)
			jab_recv(j);
	}
}

/*
 *  jab_auth -- authorize user
 *
 *  parameters
 *      j -- connection
 *
 *  returns
 *      id of the iq packet
 */
char *
jab_auth(jabconn j)
{
	xode x, y, z;
	char *user, *id;

	if (!j)
		return (NULL);

	user = j->user->user;
	id = jab_getid(j);

	x = jabutil_iqnew(JABPACKET__GET, NS_AUTH);
	xode_put_attrib(x, "id", "auth_1");
	xode_put_attrib(x, "to", j->user->server);
	y = xode_get_tag(x, "query");

	if (user) {
		z = xode_insert_tag(y, "username");
		xode_insert_cdata(z, user, -1);
	}

	jab_send(j, x);
	xode_free(x);
	jab_recv(j);

	x = jabutil_iqnew(JABPACKET__SET, NS_AUTH);
	xode_put_attrib(x, "id", "auth_2");
	xode_put_attrib(x, "to", j->user->server);
	y = xode_get_tag(x, "query");

	if (user) {
		dprintf(dExecution, "Got auth user %s...\n", user);
		z = xode_insert_tag(y, "username");
		xode_insert_cdata(z, user, -1);
	}

	z = xode_insert_tag(y, "resource");
	xode_insert_cdata(z, j->user->resource, -1);
	dprintf(dExecution, "Got auth resource %s...\n", j->user->resource);

	dprintf(dExecution, "Got auth password %s...\n", j->pass);
	if (j->sid && j->auth_digest) {
		char *hash;

		dprintf(dExecution, "Got digest auth id %s...\n", j->sid);
		z = xode_insert_tag(y, "digest");
		hash = xode_pool_malloc(x->p,
			strlen(j->sid) + strlen(j->pass) + 1);
		strcpy(hash, j->sid);
		strcat(hash, j->pass);
		hash = (char *)j_shahash(hash);
		xode_insert_cdata(z, hash, 40);
	}
	else if (j->auth_password) {
		z = xode_insert_tag(y, "password");
		xode_insert_cdata(z, j->pass, -1);
	}

	jab_send(j, x);
	xode_free(x);
	jab_recv(j);

	return id;
}

/*
 *  jab_reg -- register user
 *
 *  parameters
 *      j -- connection
 *
 *  returns
 *      id of the iq packet
 */
char *
jab_reg(jabconn j)
{
	xode x, y, z;
	char *user, *id;
	/* UNUSED char *hash; */

	if (!j)
		return (NULL);

	x = jabutil_iqnew(JABPACKET__SET, NS_REGISTER);
	id = jab_getid(j);
	xode_put_attrib(x, "id", id);
	y = xode_get_tag(x, "query");

	user = j->user->user;

	if (user) {
		z = xode_insert_tag(y, "username");
		xode_insert_cdata(z, user, -1);
	}

	z = xode_insert_tag(y, "resource");
	xode_insert_cdata(z, j->user->resource, -1);

	if (j->pass) {
		z = xode_insert_tag(y, "password");
		xode_insert_cdata(z, j->pass, -1);
	}

	jab_send(j, x);
	xode_free(x);

	j->state = JABCONN_STATE_AUTH;
	STATE_EVT(JABCONN_STATE_AUTH)

	return id;
}


/* local functions */

static void 
startElement(void *userdata, const char *name, const char **attribs)
{
	xode x;
	jabconn j = (jabconn) userdata;

	if (strcmp(name, "stream:stream") == 0) {
		/* special case: name == stream:stream */
		/* id attrib of stream is stored for digest auth */
		x = xode_new(name);
		xode_put_expat_attribs(x, attribs);
		j->sid = xode_get_attrib(x, "id");
		xode_free(x);
		dprintf(dXML, "Got stream:stream, retrieved stream id %s.\n", j->sid);
	}

	if (j->current) {
		/* Append the node to the current one */
		x = xode_insert_tag(j->current, name);
		xode_put_expat_attribs(x, attribs);

		j->current = x;
	}
	else {
		x = xode_new(name);
		xode_put_expat_attribs(x, attribs);
		if (strcmp(name, "stream:stream") != 0) {
			j->current = x;
		}
	}
}

static void 
endElement(void *userdata, const char *name)
{
	jabconn j = (jabconn) userdata;
	xode x;
	jabpacket p;

	if (j->current == NULL) {
		/* we got </stream:stream> */
		STATE_EVT(JABCONN_STATE_OFF)
			return;
	}

	x = xode_get_parent(j->current);

	if (x == NULL) {
		/* it is time to fire the event */
		p = jabpacket_new(j->current);

		if (j->on_packet)
			(j->on_packet) (j, p);
		xode_free(j->current);
	}

	j->current = x;
}

static void 
charData(void *userdata, const char *s, int slen)
{
	jabconn j = (jabconn) userdata;

	if (j->current)
		xode_insert_cdata(j->current, s, slen);
}
