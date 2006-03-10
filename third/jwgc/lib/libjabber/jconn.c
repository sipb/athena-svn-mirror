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

/* $Id: jconn.c,v 1.1.1.2 2006-03-10 15:35:16 ghudson Exp $ */

#include "libjabber.h"

/* local macros for launching event handlers */
#define STATE_EVT(arg) if(j->on_state) { (j->on_state)(j, (arg) ); }

/* prototypes of the local functions */
static void startElement(void *userdata, const char *name, const char **attribs);
static void endElement(void *userdata, const char *name);
static void charData(void *userdata, const char *s, int slen);
static unsigned char *base64_encode(const unsigned char *in, size_t inlen);
static void base64_decode(const char *text, char **data, int *size);

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
jab_new(char *user, char *server)
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
	j->server = xode_pool_strdup(p, server);
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
	j->endcount = 0;

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
	char *t;

	if (!j || j->state != JABCONN_STATE_OFF)
		return;

	j->parser = XML_ParserCreate(NULL);
	XML_SetUserData(j->parser, (void *) j);
	XML_SetElementHandler(j->parser, startElement, endElement);
	XML_SetCharacterDataHandler(j->parser, charData);

	j->fd = make_netsocket(j->port, j->server, NETSOCKET_CLIENT);
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
	t = jabutil_header(NS_CLIENT, j->user->server);
	jab_send_raw(j, "<?xml version='1.0'?>");
	jab_send_raw(j, t);
	free(t);

	/* Receive back a stream header and stream:features packet. */
	jab_recv_packet(j, 1);

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

void
jab_recv_packet(jabconn j, int npkt)
{
	j->endcount = 0;
	while (j->endcount < npkt)
		jab_recv(j);
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

/* Written by Simon Wilkinson for Gaim; adapted for jwgc. */
static int
gssapi_step(jabconn j, char *indata, char **outdata) {
	gss_buffer_t in_token_ptr = NULL;
	gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
	gss_buffer_desc	output_token = GSS_C_EMPTY_BUFFER;
	unsigned int len;
	OM_uint32 major, minor;

	if (indata) {
		base64_decode(indata, (char **) &input_token.value, &len);
		input_token.length = len;
		in_token_ptr = &input_token;
	}

	if (j->gsscomplete) {
		if (input_token.length == 0) {
			/* We've received an empty token from the server,
			 * so we should send an empty one back */
			*outdata = NULL;
		} else {
			char *response;
			major = gss_unwrap(&minor, j->gsscontext,
				   	   &input_token, &output_token, 
					   NULL, NULL);
			if (GSS_ERROR(major)) {
				gss_release_buffer(&minor, &output_token);
				return 0;
			}
			gss_release_buffer(&minor, &output_token);
			input_token.length = strlen(j->user->user) + 4;
			response = malloc(input_token.length);
			response[0] = 1;
			response[1] = 0;
			response[2] = 0;
			response[3] = 0;
			memcpy(response + 4, j->user->user,
			       input_token.length - 4);
			input_token.value = response;
			major = gss_wrap(&minor, j->gsscontext,
					 0, GSS_C_QOP_DEFAULT,
					 &input_token, NULL,
					 &output_token);

			*outdata = base64_encode(output_token.value, 
						 output_token.length);
			gss_release_buffer(&minor, &output_token);
			free(response);
		}
	} else {
	
		gss_OID_desc mech_oid_desc =
			{ 9, (void *) "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02" };

		major = gss_init_sec_context(&minor,
					     GSS_C_NO_CREDENTIAL,
					     &j->gsscontext,
					     j->gssserver,
					     &mech_oid_desc,
					     GSS_C_MUTUAL_FLAG,
					     GSS_C_INDEFINITE,
					     GSS_C_NO_CHANNEL_BINDINGS,
					     in_token_ptr,
					     NULL,
					     &output_token,
					     NULL,
					     NULL);

		if (input_token.value)
			free(input_token.value);

		if (GSS_ERROR(major)) {
			gss_release_buffer(&minor, &output_token);
			return 0;
		}

		if (major == GSS_S_COMPLETE)
			j->gsscomplete = 1;

		if (output_token.length > 0)
			*outdata = base64_encode(output_token.value, 
						 output_token.length);
		else
			*outdata = NULL;

		gss_release_buffer(&minor, &output_token);
	}

	return 1;
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
	char *outdata, *t;
	gss_buffer_desc server_desc, output_token;
	OM_uint32 major, minor;
	gss_OID_desc gss_c_nt_hostbased_service =
		{ 10, (void *) "\x2a\x86\x48\x86\xf7\x12\x01\x02\x01\x04" };
	int status;

	if (!j)
		return (NULL);

	server_desc.value = malloc(strlen(j->server) + 6);
	sprintf(server_desc.value, "xmpp@%s", j->server);
	server_desc.length = strlen(server_desc.value);
	major = gss_import_name(&minor, &server_desc,
				&gss_c_nt_hostbased_service, &j->gssserver);
	free(server_desc.value);
	if (GSS_ERROR(major))
		return NULL;

	j->gsscontext = GSS_C_NO_CONTEXT;
	j->gsstoken = NULL;
	j->gsscomplete = 0;
	j->gsssuccess = 0;

	if (!gssapi_step(j, NULL, &outdata))
		return NULL;

	/* Kick off the authentication. */
	x = xode_new("auth");
	xode_put_attrib(x, "xmlns", NS_SASL);
	xode_put_attrib(x, "mechanism", "GSSAPI");
	xode_insert_cdata(x, outdata, -1);
	free(outdata);
	jab_send(j, x);
	xode_free(x);

	/* Respond to challenges until we receive a success notification. */
	jab_recv_packet(j, 1);
	while (j->gsssuccess == 0) {
		if (!j->gsstoken)
			return NULL;
		status = gssapi_step(j, j->gsstoken, &outdata);
		free(j->gsstoken);
		j->gsstoken = NULL;
		if (!status)
			return NULL;

		x = xode_new("response");
		xode_put_attrib(x, "xmlns", NS_SASL);
		xode_insert_cdata(x, outdata, -1);
		free(outdata);
		jab_send(j, x);
		xode_free(x);
		jab_recv_packet(j, 1);
	}

	/* The XML stream restarts at this point, so make a new parser. */
	XML_ParserFree(j->parser);
	j->parser = XML_ParserCreate(NULL);
	XML_SetUserData(j->parser, (void *) j);
	XML_SetElementHandler(j->parser, startElement, endElement);
	XML_SetCharacterDataHandler(j->parser, charData);

	/* Send a new stream header. */
	t = jabutil_header(NS_CLIENT, j->user->server);
	jab_send_raw(j, t);
	free(t);

	/* Receive back a stream header and stream:features packet. */
	jab_recv_packet(j, 1);

	/* Perform resource binding. */
	x = xode_new("iq");
	xode_put_attrib(x, "type", "set");
	y = xode_insert_tag(x, "bind");
	xode_put_attrib(y, "xmlns", NS_BIND);
	if (j->user->resource) {
		z = xode_insert_tag(y, "resource");
		xode_insert_cdata(z, j->user->resource, -1);
	}
	jab_send(j, x);
	xode_free(x);
	jab_recv_packet(j, 1);

	/* Request a session. */
	x = xode_new("iq");
	xode_put_attrib(x, "type", "set");
	y = xode_insert_tag(x, "session");
	xode_put_attrib(y, "xmlns", NS_SESSION);
	jab_send(j, x);
	xode_free(x);
	jab_recv_packet(j, 1);

	return NULL;
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
		j->endcount++;

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

/**************************************************************************
 * Base64 Functions (taken from Gaim)
 **************************************************************************/
static const char alphabet[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

static const char xdigits[] =
	"0123456789abcdef";

static unsigned char *
base64_encode(const unsigned char *in, size_t inlen)
{
	char *out, *rv;

	rv = out = malloc(((inlen/3)+1)*4 + 1);

	for (; inlen >= 3; inlen -= 3)
	{
		*out++ = alphabet[in[0] >> 2];
		*out++ = alphabet[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*out++ = alphabet[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*out++ = alphabet[in[2] & 0x3f];
		in += 3;
	}

	if (inlen > 0)
	{
		unsigned char fragment;

		*out++ = alphabet[in[0] >> 2];
		fragment = (in[0] << 4) & 0x30;

		if (inlen > 1)
			fragment |= in[1] >> 4;

		*out++ = alphabet[fragment];
		*out++ = (inlen < 2) ? '=' : alphabet[(in[1] << 2) & 0x3c];
		*out++ = '=';
	}

	*out = '\0';

	return rv;
}

static void
base64_decode(const char *text, char **data, int *size)
{
	char *out = NULL;
	char tmp = 0;
	const char *c;
	int tmp2 = 0, len = 0, n = 0;

	c = text;

	while (*c) {
		if (*c >= 'A' && *c <= 'Z') {
			tmp = *c - 'A';
		} else if (*c >= 'a' && *c <= 'z') {
			tmp = 26 + (*c - 'a');
		} else if (*c >= '0' && *c <= 57) {
			tmp = 52 + (*c - '0');
		} else if (*c == '+') {
			tmp = 62;
		} else if (*c == '/') {
			tmp = 63;
		} else if (*c == '\r' || *c == '\n') {
			c++;
			continue;
		} else if (*c == '=') {
			if (n == 3) {
				out = realloc(out, len + 2);
				out[len] = (char)(tmp2 >> 10) & 0xff;
				len++;
				out[len] = (char)(tmp2 >> 2) & 0xff;
				len++;
			} else if (n == 2) {
				out = realloc(out, len + 1);
				out[len] = (char)(tmp2 >> 4) & 0xff;
				len++;
			}
			break;
		}
		tmp2 = ((tmp2 << 6) | (tmp & 0xff));
		n++;
		if (n == 4) {
			out = realloc(out, len + 3);
			out[len] = (char)((tmp2 >> 16) & 0xff);
			len++;
			out[len] = (char)((tmp2 >> 8) & 0xff);
			len++;
			out[len] = (char)(tmp2 & 0xff);
			len++;
			tmp2 = 0;
			n = 0;
		}
		c++;
	}

	out = realloc(out, len + 1);
	out[len] = 0;

	*data = out;

	if (size)
		*size = len;
}
