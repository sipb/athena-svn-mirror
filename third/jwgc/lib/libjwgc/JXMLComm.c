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

/* This is a blatent ripoff of jconn.c from libjabber.  =)  */

/* $Id: JXMLComm.c,v 1.1.1.1 2006-03-10 15:32:57 ghudson Exp $ */

#include "libjwgc.h"

/* prototypes of the local functions */
static void jwg_startElement(void *userdata, const char *name, const char **attribs);
static void jwg_endElement(void *userdata, const char *name);
static void jwg_charData(void *userdata, const char *s, int slen);

/*
 *  jwg_new -- initialize a new jwgc connection
 *
 *  results
 *      a pointer to the connection structure
 *      or NULL if allocations failed
 */
jwgconn
jwg_new()
{
	xode_pool p;
	jwgconn jwg;

	p = xode_pool_new();
	if (!p)
		return (NULL);
	jwg = xode_pool_mallocx(p, sizeof(jwgconn_struct), 0);
	if (!jwg)
		return (NULL);
	jwg->p = p;

	jwg->state = JWGCONN_STATE_OFF;
	jwg->fd = -1;
	jwg->sckfd = -1;

	return jwg;
}


/*
 *  jwg_server -- initialize a new jwgc server port
 *
 *  results
 *      a pointer to the connection structure
 *      or NULL if allocations failed
 */
jwgconn
jwg_server()
{
	xode_pool p;
	jwgconn jwg;

	p = xode_pool_new();
	if (!p)
		return (NULL);
	jwg = xode_pool_mallocx(p, sizeof(jwgconn_struct), 0);
	if (!jwg)
		return (NULL);
	jwg->p = p;

	jwg->state = JWGCONN_STATE_OFF;
	jwg->fd = -1;
	jwg->sckfd = -1;

	return jwg;
}

/*
 *  jwg_delete -- free a jwgc connection
 *
 *  parameters
 *      j -- connection
 *
 */
void
jwg_delete(jwgconn jwg)
{
	if (!jwg)
		return;

	jwg_stop(jwg);
	xode_pool_free(jwg->p);
}

void
jwg_cleanup(jwgconn jwg)
{
	if (!jwg)
		return;
	if (jwg->sckfd == -1)
		return;

	close(jwg->sckfd);
	jwg->sckfd = -1;
}

/*
 *  jwg_event_handler -- set callback handler for client communication
 *
 *  parameters
 *      jwg -- connection
 *      h -- name of the handler function
 */
void
jwg_event_handler(jwgconn jwg, jwgconn_packet_h h)
{
	if (!jwg)
		return;

	jwg->on_packet = h;
}


/*
 *  jwg_start -- start connection
 *
 *  parameters
 *      jwg -- connection
 *
 */
void
jwg_start(jwgconn jwg)
{
	xode x;
	char *t, *t2;
	int len, fromlen, errcode;
	struct sockaddr_in from;

	if (!jwg || jwg->state != JWGCONN_STATE_OFF)
		return;

	jwg->parser = XML_ParserCreate(NULL);
	XML_SetUserData(jwg->parser, (void *) jwg);
	XML_SetElementHandler(jwg->parser, jwg_startElement, jwg_endElement);
	XML_SetCharacterDataHandler(jwg->parser, jwg_charData);

	jwg->fd = JConnect();
	if (jwg->fd < 0) {
		return;
	}
	jwg->state = JWGCONN_STATE_CONNECTED;
}


/*
 *  jwg_servstart -- start server
 *
 *  parameters
 *      jwg -- connection
 *
 */
void
jwg_servstart(jwgconn jwg)
{
	int flags = 0;

	if (!jwg || jwg->state != JWGCONN_STATE_OFF)
		return;

	jwg->parser = XML_ParserCreate(NULL);
	XML_SetUserData(jwg->parser, (void *) jwg);
	XML_SetElementHandler(jwg->parser, jwg_startElement, jwg_endElement);
	XML_SetCharacterDataHandler(jwg->parser, jwg_charData);

	jwg->fd = JSetupComm();
	if (jwg->fd < 0) {
		return;
	}
/*
	jwg->port = get_netport();
*/

	jwg->state = JWGCONN_STATE_CONNECTED;
}

/*
 *  jwg_reset -- reset parser
 *
 *  parameters
 *      jwg -- connection
 *
 */
void
jwg_reset(jwgconn jwg)
{
	XML_ParserFree(jwg->parser);
	jwg->parser = XML_ParserCreate(NULL);
	XML_SetUserData(jwg->parser, (void *) jwg);
	XML_SetElementHandler(jwg->parser, jwg_startElement, jwg_endElement);
	XML_SetCharacterDataHandler(jwg->parser, jwg_charData);
}

/*
 *  jwg_stop -- stop connection
 *
 *  parameters
 *      jwg -- connection
 */
void
jwg_stop(jwgconn jwg)
{
	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	jwg->state = JWGCONN_STATE_OFF;
	close(jwg->fd);
	jwg->fd = -1;
	jwg->sckfd = -1;
	XML_ParserFree(jwg->parser);
}

/*
 *  jwg_getfd -- get file descriptor of connection socket
 *
 *  parameters
 *      jwg -- connection
 *
 *  returns
 *      fd of the socket or -1 if socket was not connected
 */
int
jwg_getfd(jwgconn jwg)
{
	if (jwg) {
		return jwg->fd;
	}
	else {
		return -1;
	}
}

/*
 *  jwg_send -- send xml data
 *
 *  parameters
 *      jwg -- connection
 *      x -- xode structure
 */
void
jwg_send(jwgconn jwg, xode x)
{
	char *buf = xode_to_str(x);
	fd_set fds;
	FILE *selfd;
	struct timeval tv;
	int numwrote, totalsize, len, ret;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	if (!buf)
		return;

	FD_ZERO(&fds);
	FD_SET(jwg->fd, &fds);
	selfd = (FILE *) jwg->fd;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	ret = jwg_sockselect((int) selfd + 1, NULL, &fds, NULL, &tv);
	if (ret < 0) {
		dprintf(dJWG, "Error on select: %s\n", strerror(errno));
		return;
	}
	else if (ret == 0) {
		dprintf(dJWG, "Time expired while waiting to write.\n");
		return;
	}

	totalsize = strlen(buf) + 1;
	numwrote = 0;
	while (numwrote < totalsize) {
/*
		len = send(jwg->fd, buf, totalsize - numwrote, 0);
*/
		len = jwg_socksend(jwg->fd, buf, totalsize - numwrote);
		if (len < 0) {
			dprintf(dJWG, "Error writing to socket: %s\n", strerror(errno));
			break;
		}
		numwrote += len;
		dprintf(dJWG, "sent %d bytes, %d wrote out of %d\n", len, numwrote, totalsize);
		buf += len;
	}
	dprintf(dJWG, "out[%d]: %s\n", totalsize, buf);
}

/*
 *  jwg_servsend -- send xml data
 *
 *  parameters
 *      jwg -- connection
 *      x -- xode structure
 */
void
jwg_servsend(jwgconn jwg, xode x)
{
	char *buf = xode_to_str(x);
	int numwrote, totalsize, len, ret;
	FILE *selfd;
	struct timeval tv;
	fd_set fds;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	if (!buf)
		return;

	FD_ZERO(&fds);
	FD_SET(jwg->sckfd, &fds);
	selfd = (FILE *) jwg->sckfd;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	ret = jwg_sockselect((int) selfd + 1, NULL, &fds, NULL, &tv);
	if (ret < 0) {
		dprintf(dJWG, "Error on select: %s\n", strerror(errno));
		return;
	}
	else if (ret == 0) {
		dprintf(dJWG, "Time expired while waiting to write.\n");
		return;
	}

	totalsize = strlen(buf) + 1;
	numwrote = 0;
	dprintf(dJWG, "out[%d]: %s\n", totalsize, buf);
	while (numwrote < totalsize) {
/*
		len = send(jwg->sckfd, buf, totalsize - numwrote, 0);
*/
		len = jwg_socksend(jwg->sckfd, buf, totalsize - numwrote);
		if (len < 0) {
			dprintf(dJWG, "Error writing to socket: %s\n", strerror(errno));
			break;
		}
		numwrote += len;
		dprintf(dJWG, "sent %d bytes, %d wrote out of %d\n", len, numwrote, totalsize);
		buf += len;
	}
}

/*
 *  jwg_serverror -- send error msg
 *
 *  parameters
 *      jwg -- connection
 *      text -- xode structure
 */
void
jwg_serverror(jwgconn jwg, char *text)
{
	xode out;

	out = xode_new("error");
	xode_insert_cdata(out, text, strlen(text));
	jwg_servsend(jwg, out);
	xode_free(out);
}

/*
 *  jwg_servsuccess -- send success msg
 *
 *  parameters
 *      jwg -- connection
 *      text -- xode structure
 */
void
jwg_servsuccess(jwgconn jwg, char *text)
{
	xode out;

	out = xode_new("success");
	xode_insert_cdata(out, text, strlen(text));
	jwg_servsend(jwg, out);
	xode_free(out);
}

/*
 *  jwg_send_raw -- send a string
 *
 *  parameters
 *      jwg -- connection
 *      str -- xml string
 */
void
jwg_send_raw(jwgconn jwg, const char *str)
{
	if (jwg && jwg->state != JWGCONN_STATE_OFF)
		write(jwg->fd, str, strlen(str));
	dprintf(dJWG, "out: %s\n", str);
}

/*
 *  jwg_servsend_raw -- send a string
 *
 *  parameters
 *      jwg -- connection
 *      str -- xml string
 */
void
jwg_servsend_raw(jwgconn jwg, const char *str)
{
	if (jwg && jwg->state != JWGCONN_STATE_OFF)
		write(jwg->sckfd, str, strlen(str));
	dprintf(dJWG, "out: %s\n", str);
}

int
jwg_sockselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
		struct timeval *timeout)
{ 
	int ret; 
	struct timeval currenttv, parttv;

	if (!timeout) {
		while (1) {
			ret = select(nfds, readfds, writefds, exceptfds, NULL);
			if (ret < 0 && errno != EINTR) {
				return ret;
			}
			else if (ret >= 0) {
				return ret;
			}
		}
	}

	currenttv.tv_sec = 0;
	currenttv.tv_usec = timeout->tv_usec;

	while (currenttv.tv_usec > 0) {
		parttv.tv_sec = 0;
		parttv.tv_usec = 1;
		ret = select(nfds, readfds, writefds, exceptfds, &parttv);
		if (ret < 0 && errno != EINTR) {
			return ret;
		}
		else if (ret >= 0) {
			return ret;
		}
		currenttv.tv_sec = 0;
		currenttv.tv_usec -= 1;
	}

	return 0;
} 

int
jwg_sockrecv(int sockfd, char *buf, size_t maxsize)
{
	size_t totalread = 0;
	int ret;

	do {
		ret = recv(sockfd, buf, maxsize - totalread, 0);
		if (ret < 0 && errno != EINTR) {
			return ret;
		}
		else if (ret == 0) {
			return totalread;
		}
		else if (ret > 0) {
			totalread += ret;
			return totalread;
		}
		else if (ret == -1 && errno == EINTR) {
			if (recv(sockfd, buf, maxsize - totalread, MSG_PEEK)
					<= 0) {
				return totalread;
			}
		}
	} while (totalread < maxsize);

	return totalread;
}

int
jwg_socksend(int sockfd, char *buf, size_t len)
{
	size_t totalsent = 0;
	int ret;

	do {
		ret = send(sockfd, buf, len - totalsent, 0);
		if (ret < 0 && errno != EINTR) {
			return ret;
		}
		else if (ret == 0) {
			return totalsent;
		}
		else if (ret > 0) {
			totalsent += ret;
			return totalsent;
		}
	} while (totalsent < len);

	return totalsent;
}

/*
 *  jwg_recv -- read and parse incoming data
 *
 *  parameters
 *      jwg -- connection
 */
void
jwg_recv(jwgconn jwg)
{
	char *fullbuf;
	static char buf[4096];
	int len, totallen, fromlen, errcode;
	struct sockaddr_in from;
	fd_set fds;
	FILE *selfd;
	struct timeval tv;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	fullbuf = (char *) malloc(sizeof(char));
	fullbuf[0] = '\0';
	totallen = 0;

	FD_ZERO(&fds);
	FD_SET(jwg->fd, &fds);
	selfd = (FILE *) jwg->fd;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	while (jwg_sockselect((int) selfd + 1, &fds, NULL, NULL, &tv) > 0) {
		len = jwg_sockrecv(jwg->fd, buf, sizeof(buf) - 1);
		if (len <= 0) {
			break;
		}
		dprintf(dJWG, " in piece[%d]: %s\n", len, buf);
		fullbuf = (char *) realloc(fullbuf, strlen(fullbuf) + strlen(buf) + 1);
		strcat(fullbuf, buf);
		totallen += len;
	}
/*
	while ((len = jwg_sockrecv(jwg->fd, buf, sizeof(buf) - 1)) > 0) {
		dprintf(dJWG, " in piece[%d]: %s\n", len, buf);
		fullbuf = (char *) realloc(fullbuf, strlen(fullbuf) + strlen(buf) + 1);
		strcat(fullbuf, buf);
		totallen += len;
	}
*/

	if (totallen > 0) {
		dprintf(dJWG, " in[%d]: %s\n", totallen, fullbuf);
		errcode = XML_Parse(jwg->parser, fullbuf, totallen - 1, 0);
		dprintf(dParser, "parser index %d, line %d, col %d\n", XML_GetCurrentByteIndex(jwg->parser), XML_GetCurrentLineNumber(jwg->parser), XML_GetCurrentColumnNumber(jwg->parser));
		if (errcode == 0) {
			dprintf(dParser, "parser error %d at byte %d: %s\n", XML_GetErrorCode(jwg->parser), XML_GetCurrentByteIndex(jwg->parser), XML_ErrorString(XML_GetErrorCode(jwg->parser)));
		}
		else {
			dprintf(dParser, "parser complete\n");
		}
	}
	else {
		dprintf(dJWG, "jwg_recv: read failed, %d:%d:%s\n", totallen, errno, strerror(errno));
	}

	free(fullbuf);
}

/*
 *  jwg_servrecv -- read and parse incoming data
 *
 *  parameters
 *      jwg -- connection
 */
void
jwg_servrecv(jwgconn jwg)
{
	char *fullbuf;
	static char buf[4096];
	int len, totallen, fromlen, errcode;
	struct sockaddr_in from;
	fd_set fds;
	FILE *selfd;
	struct timeval tv;
	struct linger li;

	li.l_onoff = 1;
	li.l_linger = 900;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	fullbuf = (char *) malloc(sizeof(char));
	fullbuf[0] = '\0';
	totallen = 0;

	fromlen = sizeof(from);
	jwg->sckfd = accept(jwg->fd, (struct sockaddr *) & from, &fromlen);
	if (jwg->sckfd < 0) {
		dprintf(dJWG, "jwg_recv: accept failed, %d:%s\n", errno, strerror(errno));
		return;
	}

	if (setsockopt(jwg->sckfd, SOL_SOCKET, SO_LINGER, (char *)&li,
			sizeof(li)) == -1) {
		dprintf(dJWG, "jwg_recv: set linger failed, %d:%s\n", errno, strerror(errno));
		return;
	}

	FD_ZERO(&fds);
	FD_SET(jwg->sckfd, &fds);
	selfd = (FILE *) jwg->sckfd;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	while (jwg_sockselect((int) selfd + 1, &fds, NULL, NULL, &tv) > 0) {
		len = jwg_sockrecv(jwg->sckfd, buf, sizeof(buf) - 1);
		if (len <= 0) {
			break;
		}
		dprintf(dJWG, " in piece[%d]: %s\n", len, buf);
		fullbuf = (char *) realloc(fullbuf, strlen(fullbuf) + strlen(buf) + 1);
		strcat(fullbuf, buf);
		totallen += len;
	}

	if (totallen > 0) {
		dprintf(dJWG, " in[%d]: %s\n", totallen, fullbuf);
		errcode = XML_Parse(jwg->parser, fullbuf, totallen - 1, 0);
		dprintf(dParser, "parser index %d, line %d, col %d\n", XML_GetCurrentByteIndex(jwg->parser), XML_GetCurrentLineNumber(jwg->parser), XML_GetCurrentColumnNumber(jwg->parser));
		if (errcode == 0) {
			dprintf(dParser, "parser error %d at byte %d: %s\n", XML_GetErrorCode(jwg->parser), XML_GetCurrentByteIndex(jwg->parser), XML_ErrorString(XML_GetErrorCode(jwg->parser)));
		}
		else {
			dprintf(dParser, "parser complete\n");
		}
		jwg_reset(jwg);
	}
	else {
		dprintf(dJWG, "jwg_servrecv: read failed, %d:%d:%s\n", totallen, errno, strerror(errno));
	}

	free(fullbuf);
}

/*
 *  jwg_poll -- check socket for incoming data
 *
 *  parameters
 *      jwg -- connection
 *      timeout -- poll timeout
 */
void
jwg_poll(jwgconn jwg, int timeout)
{
	fd_set fds;
	FILE *selfd;
	struct timeval tv;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	FD_ZERO(&fds);
	FD_SET(jwg->fd, &fds);
	selfd = (FILE *) jwg->fd;

	if (timeout < 0) {
		if (jwg_sockselect((int) selfd + 1, &fds, NULL, NULL, NULL) > 0)
			jwg_recv(jwg);
	}
	else {
		tv.tv_sec = 0;
		tv.tv_usec = timeout;

		if (jwg_sockselect((int) selfd + 1, &fds, NULL, NULL, &tv) > 0)
			jwg_recv(jwg);
	}
}

/*
 *  jwg_servpoll -- check server socket for incoming data
 *
 *  parameters
 *      jwg -- connection
 *      timeout -- poll timeout
 */
void
jwg_servpoll(jwgconn jwg, int timeout)
{
	fd_set fds;
	FILE *selfd;
	struct timeval tv;

	if (!jwg || jwg->state == JWGCONN_STATE_OFF)
		return;

	FD_ZERO(&fds);
	FD_SET(jwg->fd, &fds);
	selfd = (FILE *) jwg->fd;


	if (timeout < 0) {
		if (select((int) selfd + 1, &fds, NULL, NULL, NULL) > 0)
			jwg_servrecv(jwg);
	}
	else {
		tv.tv_sec = 0;
		tv.tv_usec = timeout;

		if (jwg_sockselect((int) selfd + 1, &fds, NULL, NULL, &tv) > 0)
			jwg_servrecv(jwg);
	}
}

jwgpacket
jwgpacket_new(xode x)
{
	jwgpacket p;

	if (x == NULL)
		return NULL;

	p = xode_pool_malloc(xode_get_pool(x), sizeof(_jwgpacket));
	p->x = x;

	return jwgpacket_reset(p);
}

jwgpacket
jwgpacket_reset(jwgpacket p)
{
	xode x;

	x = p->x;
	memset(p, 0, sizeof(_jwgpacket));
	p->x = x;
	p->p = xode_get_pool(x);

	if (strncmp(xode_get_name(x), "message", 7) == 0) {
		p->type = JWGPACKET_MESSAGE;
	}
	else if (strncmp(xode_get_name(x), "locate", 6) == 0) {
		p->type = JWGPACKET_LOCATE;
	}
	else if (strncmp(xode_get_name(x), "status", 6) == 0) {
		p->type = JWGPACKET_STATUS;
	}
	else if (strncmp(xode_get_name(x), "shutdown", 8) == 0) {
		p->type = JWGPACKET_SHUTDOWN;
	}
	else if (strncmp(xode_get_name(x), "check", 5) == 0) {
		p->type = JWGPACKET_CHECK;
	}
	else if (strncmp(xode_get_name(x), "reread", 6) == 0) {
		p->type = JWGPACKET_REREAD;
	}
	else if (strncmp(xode_get_name(x), "showvar", 7) == 0) {
		p->type = JWGPACKET_SHOWVAR;
	}
	else if (strncmp(xode_get_name(x), "subscribe", 9) == 0) {
		p->type = JWGPACKET_SUBSCRIBE;
	}
	else if (strncmp(xode_get_name(x), "unsubscribe", 11) == 0) {
		p->type = JWGPACKET_UNSUBSCRIBE;
	}
	else if (strncmp(xode_get_name(x), "nickname", 8) == 0) {
		p->type = JWGPACKET_NICKNAME;
	}
	else if (strncmp(xode_get_name(x), "group", 5) == 0) {
		p->type = JWGPACKET_GROUP;
	}
	else if (strncmp(xode_get_name(x), "register", 8) == 0) {
		p->type = JWGPACKET_REGISTER;
	}
	else if (strncmp(xode_get_name(x), "search", 6) == 0) {
		p->type = JWGPACKET_SEARCH;
	}
	else if (strncmp(xode_get_name(x), "setvar", 6) == 0) {
		p->type = JWGPACKET_SETVAR;
	}
	else if (strncmp(xode_get_name(x), "join", 4) == 0) {
		p->type = JWGPACKET_JOIN;
	}
	else if (strncmp(xode_get_name(x), "leave", 5) == 0) {
		p->type = JWGPACKET_LEAVE;
	}
	else if (strncmp(xode_get_name(x), "debug", 5) == 0) {
		p->type = JWGPACKET_DEBUG;
	}
	else if (strncmp(xode_get_name(x), "ping", 4) == 0) {
		p->type = JWGPACKET_PING;
	}
	else {
		p->type = JWGPACKET_UNKNOWN;
	}

	return p;
}

/* local functions */

static void
jwg_startElement(void *userdata, const char *name, const char **attribs)
{
	xode x;
	jwgconn jwg = (jwgconn) userdata;

	if (jwg->current) {
		/* Append the node to the current one */
		x = xode_insert_tag(jwg->current, name);
		xode_put_expat_attribs(x, attribs);

		jwg->current = x;
	}
	else {
		x = xode_new(name);
		xode_put_expat_attribs(x, attribs);
		jwg->current = x;
	}
}

static void
jwg_endElement(void *userdata, const char *name)
{
	jwgconn jwg = (jwgconn) userdata;
	xode x;
	jwgpacket p;

	x = xode_get_parent(jwg->current);

	if (x == NULL) {
		p = jwgpacket_new(jwg->current);

		if (jwg->on_packet)
			(jwg->on_packet) (jwg, p);
		xode_free(jwg->current);
	}
	jwg->current = x;
}

static void
jwg_charData(void *userdata, const char *s, int slen)
{
	jwgconn jwg = (jwgconn) userdata;

	if (jwg->current)
		xode_insert_cdata(jwg->current, s, slen);
}
