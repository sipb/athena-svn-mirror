/* acap.c -- ACAP API implementation
 *
 * Copyright (c) 2000 Carnegie Mellon University.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name "Carnegie Mellon University" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For permission or any other legal
 *    details, please contact  
 *	Office of Technology Transfer
 *	Carnegie Mellon University
 *	5000 Forbes Avenue
 *	Pittsburgh, PA  15213-3890
 *	(412) 268-4387, fax: (412) 268-7395
 *	tech-transfer@andrew.cmu.edu
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Computing Services
 *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* $Id: acap.c,v 1.1.1.1 2002-10-13 18:01:16 ghudson Exp $ */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>

#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sasl/sasl.h>
#include "../lib/prot.h" /* xxx should be <cyrus/prot.h> */
#include "../lib/iptostring.h"
#include "../lib/xmalloc.h"

#include "acap.h"

#define ACAP_DEFAULT_PORT (674)

/* types of callbacks */
typedef enum {
    CB_ANY,
    CB_CTX_ADDTO,
    CB_ALERT,
    CB_BYE,
    CB_CTX_CHANGE,
    CB_CONTINUATION,
    CB_DELETED,
    CB_ENTRY,
    CB_EXTENSION,
    CB_LANG,
    CB_LISTRIGHTS,
    CB_CTX_MODTIME,
    CB_MODTIME,
    CB_MYRIGHTS,
    CB_QUOTA,
    CB_REFER,
    CB_CTX_REMOVEFROM,
    CB_CMD_DONE
} acap_cb_type;

typedef void acap_cb_any(void *rock);
typedef void acap_cb_alert(char *str, void *rock);
typedef void acap_cb_bye(char *str, void *rock);
typedef void acap_cb_deleted(char *entry_name, void *rock);

typedef void acap_cb_continuation(acap_value_t *, void *);

typedef void acap_cb_generic(void);

struct acap_cb {
    acap_cb_type t;
    acap_cb_generic *cb;
    void *rock;

    struct acap_cb *next;
};

struct acap_cmd_s {
    char *tag;

    const struct acap_requested *ask;
    acap_entry_t *got;
    struct acap_cb *callbacks;

    acap_cmd_t *next;
};

struct acap_context_s {
    char *name;
    const struct acap_requested *ask;
    struct acap_cb *callbacks;

    acap_context_t *next;
};

struct acap_conn_s {
    char *host;
    int sock;
    struct protstream *pin, *pout;

    int next_tag;

    sasl_conn_t *sasl;
    int sasl_error;
    int authed;

    acap_cmd_t *commands;
    acap_context_t *contexts;
    struct acap_cb *callbacks; /* global callbacks */
};

/* prototypes */
int acap_cmd_start(acap_conn_t *conn, acap_cmd_t **ret, char *fmt, ...);
int acap_process_on_command(acap_conn_t *conn, acap_cmd_t *cmd,
			    acap_result_t *res);
static int acap_register_conn_callback(acap_conn_t *conn, acap_cb_type cbtype,
				      acap_cb_generic *f, void *rock);
static int acap_register_cmd_callback(acap_cmd_t *cmd, acap_cb_type cbtype,
				      acap_cb_generic *f, void *rock);
static int acap_register_context_callback(acap_context_t *context, 
					  acap_cb_type cbtype,
					  acap_cb_generic *f, void *rock);

int acap_init(void)
{
    initialize_acap_error_table();
    return ACAP_OK;
}

/* ----------------- opening connection stuff ----------------- */

/* this is an imperfect URL parser. */
static int acap_parse_url(const char *url,
			  char **user,
			  char **mech,
			  char **host,
			  char **port)
{
    char *p, *q, *myurl;
    char buf[1024];

    strncpy(buf, url, sizeof(buf));
    buf[1023] = '\0';
    myurl = buf;
    
    if (strncmp(myurl, "acap://", 7) != 0) {
	return ACAP_URL_NOT_RECOGNIZED;
    }

    myurl += 7;

    /* look for a user name */
    p = strchr(myurl, '@');
    if (p != NULL) {
	int user_len;
	    
	/* there's a username */
	
	if ((q = strstr(myurl, ";AUTH=")) != NULL) {
	    user_len = q - myurl;
	    *user = (char *) malloc(user_len + 1);
	    strncpy(*user, myurl, user_len);
	    (*user)[user_len] = '\0';

	    /* what authentication scheme? */
	    q += 6;
	    if (*q == '*' && *(q+1) == ';') {
		*mech = NULL; /* any */
	    } else {
		int mech_len = p - q;
		*mech = (char *) malloc(mech_len + 1);
		strncpy(*mech, q, mech_len);
		(*mech)[mech_len] = '\0';
	    }
	} else {
	    /* no AUTH=, so let's just grab the username */
	    user_len = p - myurl;
	    *user = (char *) malloc(user_len + 1);
	    strncpy(*user, myurl, user_len);
	    (*user)[user_len] = '\0';

	    *mech = NULL; /* any auth */
	}
	/* advance p beyond the '@' */
	p++;
    } else {
	/* no '@' sign; want anonymous authentication */
	*user = NULL;
	*mech = strdup("ANONYMOUS");
	p = myurl;
    }

    /* p now points to a hostport SLASH stuff */
    if ((q = strchr(p, '/')) != NULL) {
	/* ignore trailing URL foo */
	*q = '\0';
    }

    if ((q = strchr(p, ':')) != NULL) {
	/* there's a port */
	int host_len;
	host_len = q - p;
	*host = (char *) malloc(host_len + 1);
	strncpy(*host, p, host_len);
	(*host)[host_len] = '\0';
	*port = strdup(q+1);
    } else {
	/* no port */
	*host = strdup(p);
	*port = NULL;
    }

    return ACAP_OK;
}

static int acap_conn_do_connect(acap_conn_t *conn, char *host, char *port)
{
    struct sockaddr_in addr;
    struct hostent *hp;
    struct servent *service;
    int sock;

    if ((hp = gethostbyname(host)) == NULL) {
	return ACAP_NO_CONNECTION;
    }
    conn->host = strdup(hp->h_name);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	return ACAP_NO_CONNECTION;
    }

    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    service = getservbyname(port, "tcp");
    if (service == NULL) {
	int p = atoi(port);
	if (p == 0) p = ACAP_DEFAULT_PORT;
	addr.sin_port = htons(p);
    } else {
	addr.sin_port = service->s_port;
    }

    if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	return ACAP_NO_CONNECTION;
    }

    conn->sock = sock;
    
    conn->pin = prot_new(sock, 0);
    conn->pout = prot_new(sock, 1);
    prot_setflushonread(conn->pin, conn->pout);
    
    return ACAP_OK;
}

/* grab the * ACAP line, and if mech is non-NULL put the available mechs
   there.  set any capabilities we care about in the conn struct */
static int acap_conn_do_capability(acap_conn_t *conn, char **mech)
{
    char buf[4096];
    char *s;

    s = prot_fgets(buf, sizeof(buf), conn->pin);
    if (s == NULL) {
	return ACAP_NO_CONNECTION;
    }

    if (mech) {	/* find available mechanisms */
	char *p;

	p = buf;
	while (*p) {
	    if (*p == 's' || *p == 'S') {
		if (!strncasecmp(p, "SASL", 4)) {
		    break;
		}
	    }
	    p++;
	}
	if (!p) {
	    /* no mechanisms available? */
	    *mech = NULL;
	} else {
	    char *q = strchr(p, ')');
	    if (!q) {
		*mech = NULL;
		return ACAP_PROTOCOL_ERROR;
	    }
	    *mech = malloc(q-p+1);
	    strncpy(*mech, p, q-p);
	    (*mech)[q-p] = '\0';
	}
    }

    /* currently, there aren't any interesting capabilities */
    return ACAP_OK;
}

static void acap_auth_step(acap_value_t *arg, void *rock)
{
    int r;
    acap_conn_t *conn = (acap_conn_t *) rock;
    const char *out;
    unsigned int outlen;

    /* just got a response from the server; process it and generate our own */
    r = sasl_client_step(conn->sasl, arg->data, arg->len,
			 NULL, &out, &outlen);

    if (r == SASL_OK || r == SASL_CONTINUE) {
	if (outlen) {
	    prot_printf(conn->pout, "{%d+}\r\n", outlen);
	    prot_write(conn->pout, out, outlen);
	    prot_printf(conn->pout, "\r\n");
	} else {
	    prot_printf(conn->pout, "\"\"\r\n");
	}
    } else {
	/* cancel authentication */
	prot_printf(conn->pout, "*\r\n");
    }
}

static void acap_auth_done(acap_result_t res, void *rock)
{
    acap_conn_t *conn = (acap_conn_t *) rock;

    /* it would be nice if we could get the SASL keyword here */

    if (res == ACAP_RESULT_OK) {
	conn->authed = 1;
    } else {
	conn->authed = 0;
    }
}

static int mysasl_simple_cb(void *context, int id, const char **result,
			    unsigned int *len)
{
    if (!result) {
	return SASL_BADPARAM;
    }

    switch (id) {
    case SASL_CB_USER:
	*result = (char *)context;
	break;
    default:
	return SASL_BADPARAM;
    }
    if (len) {
	*len = *result ? strlen(*result) : 0;
    }

    return SASL_OK;
}


static sasl_security_properties_t *make_secprops(int min,int max)
{
  sasl_security_properties_t *ret=(sasl_security_properties_t *)
    malloc(sizeof(sasl_security_properties_t));

  ret->maxbufsize=1024;
  ret->min_ssf=min;
  ret->max_ssf=max;

  ret->security_flags=0;
  ret->property_names=NULL;
  ret->property_values=NULL;

  return ret;
}

static int acap_conn_do_auth(acap_conn_t *conn,
			     const char *user, 
			     const char *mechs,
			     sasl_callback_t *cb)
{
    struct sockaddr_in saddr_l, saddr_r;
    char localip[60], remoteip[60];
    sasl_security_properties_t *secprops=NULL;
    const char *mech;
    const char *str;
    socklen_t salen;
    unsigned len;
    int r;
    acap_cmd_t *cmd;
    sasl_callback_t *mycb;
    int didusercb, i;

    /* copy the sasl callbacks, to ours, add the CB_USER */
    i = 0;
    if (cb) {
	while (cb[i].id != SASL_CB_LIST_END) i++;
    }

    i += 2; /* one for SASL_CB_LIST_END, one for SASL_CB_USER */
    mycb = (sasl_callback_t *) malloc(i * sizeof(sasl_callback_t));

    i = 0;
    didusercb = 0;
    if (cb) {
	while (cb[i].id != SASL_CB_LIST_END) {
	    if (cb[i].id == SASL_CB_USER) {
		/* override */
		mycb[i].id = SASL_CB_USER;
		mycb[i].proc = &mysasl_simple_cb;
		mycb[i].context = (void *) user;
		
		didusercb = 1;
	    } else {
		/* copy */
		mycb[i].id = cb[i].id;
		mycb[i].proc = cb[i].proc;
		mycb[i].context = cb[i].context;
	    }
	    
	    i++;
	}
    }
    if (!didusercb) {		/* add on user callback */
	mycb[i].id = SASL_CB_USER;
	mycb[i].proc = &mysasl_simple_cb;
	mycb[i].context = (void *) user;

	i++;
    }
    /* add SASL_CB_LIST_END */
    mycb[i].id = SASL_CB_LIST_END;
    mycb[i].proc = NULL;
    mycb[i].context = NULL;

    salen = sizeof(saddr_r);
    if (getpeername(conn->sock, (struct sockaddr *) &saddr_r, &salen)) {
	free(mycb);
	return ACAP_NO_CONNECTION;
    }

    salen = sizeof(saddr_l);
    if (getsockname(conn->sock, (struct sockaddr *) &saddr_l, &salen)) {
	free(mycb);
	return ACAP_NO_CONNECTION;
    }

    if(iptostring((struct sockaddr *)&saddr_l, sizeof(struct sockaddr_in),
		  localip, 60))
	return ACAP_NO_CONNECTION;

    if(iptostring((struct sockaddr *)&saddr_r, sizeof(struct sockaddr_in),
		  remoteip, 60))
	return ACAP_NO_CONNECTION;

    r = sasl_client_new("acap", conn->host,
			localip, remoteip,
			mycb, 0, &conn->sasl);
    if (r != SASL_OK || !conn->sasl) {
	conn->sasl_error = r;
	free(mycb);
	return ACAP_NO_AUTHENTICATION;
    }

    secprops = make_secprops(0, 0); /* xxx */
    if (secprops != NULL) {
	sasl_setprop(conn->sasl, SASL_SEC_PROPS, secprops);
	free(secprops);
    }

    r = sasl_client_start(conn->sasl, mechs,
			  NULL, &str, &len, &mech);
    if (r == SASL_OK || r == SASL_CONTINUE) {
	if (str && len) {
	    r = acap_cmd_start(conn, &cmd, 
			       "Authenticate %s %S", mech, len, str);
	} else {
	    /* no initial response */
	    r = acap_cmd_start(conn, &cmd,
			       "Authenticate %s", mech);
	}

	if (r != ACAP_OK) {
	    return r;
	}
    } else {
	conn->sasl_error = r;
	free(mycb);
	return ACAP_NO_AUTHENTICATION;
    }

    /* register callbacks! */
    acap_register_conn_callback(conn, CB_CONTINUATION,
				(acap_cb_generic *) &acap_auth_step, conn);
    acap_register_cmd_callback(cmd, CB_CMD_DONE,
			       (acap_cb_generic *) &acap_auth_done, conn);

    r = acap_process_on_command(conn, cmd, NULL);
    if (r != ACAP_OK) {
	free(mycb);
	return r;
    }

    if (conn->authed) {
	prot_setsasl(conn->pin,  conn->sasl);
	prot_setsasl(conn->pout, conn->sasl);

	free(mycb);
	return ACAP_OK;
    } else {
	free(mycb);
	return ACAP_NO_AUTHENTICATION;
    }
}

int acap_conn_connect(const char *url, sasl_callback_t *cb, acap_conn_t **ret)
{
    char *user = NULL, *mech = NULL;
    char *host = NULL, *port = NULL;
    acap_conn_t *conn;
    int r;
    
    if (!url || !ret) return ACAP_BAD_PARAM;
    conn = (acap_conn_t *) malloc(sizeof(acap_conn_t));
    *ret = conn;
    memset(conn, 0, sizeof(acap_conn_t));

    r = acap_parse_url(url, &user, &mech, &host, &port);
    if (r != ACAP_OK) {
	return r;
    }
    
    r = acap_conn_do_connect(conn, host, port ? port : "acap");
    free(host);
    if (port) free(port);
    if (r != ACAP_OK) {
	if (user) free(user);
	if (mech) free(mech);
	return r;
    }

    /* if mech != NULL, we don't want to get another list of mechs */
    r = acap_conn_do_capability(conn, mech ? NULL : &mech);
    if (r != ACAP_OK) {
	if (user) free(user);
	if (mech) free(mech);
	return r;
    }

    r = acap_conn_do_auth(conn, user, mech, cb);
    if (user) free(user);
    if (mech) free(mech);
    if (r != ACAP_OK) {
	return r;
    }

    return ACAP_OK;
}

/* ----------- end of opening connection ----------- */

int acap_conn_get_sock(acap_conn_t *conn)
{
    return conn->sock;
}

/* closes & frees; if the connection is already closed, just frees */
int acap_conn_close(acap_conn_t *conn)
{
    if (!conn) return ACAP_BAD_PARAM;

    /* xxx the following values are not freed. not sure if they should be
    char *host;

    acap_cmd_t *commands;
    acap_context_t *contexts;
    struct acap_cb *callbacks; 
    */

    if (conn->sasl) sasl_dispose(&(conn->sasl));
    if (conn->pin) prot_free(conn->pin);
    if (conn->pout) prot_free(conn->pout);

    close(conn->sock);

    return ACAP_OK;
}

/* -------------------- commands ----------------------- */

static int send_quoted_p(int l, char *a)
{
    int i;

    if (l > 128) return 0;

    for (i = 0; i < l; i++) {
	if (a[i] == '*') continue;
	if (!isprint((int) a[i])) return 0;
	if (a[i] == '"') return 0;
	if (a[i] == '\\') return 0;
    }

    return 1;
}

acap_cmd_t *acap_cmd_new(acap_conn_t *conn)
{
    char tag[50];
    acap_cmd_t *ret;

    if (!conn) return NULL;
    ret = (acap_cmd_t *) malloc(sizeof(acap_cmd_t));
    sprintf(tag, "%d", conn->next_tag++);
    ret->tag = strdup(tag);

    ret->ask = NULL;
    ret->got = NULL;
    ret->callbacks = NULL;

    /* lock conn here */
    ret->next = conn->commands;
    conn->commands = ret;
    /* unlock conn here */

    return ret;
}

/* fmt is expected to be:
 * %% => send %
 * %s => C-style string, sent either quoted or literal
 * %S => length & char *, sent as literal
 * %v => acap_value_t, sent either quoted or literal
 * %d => int (sent unquoted)
 * %c => char (sent unquoted)
 */
int acap_cmd_start(acap_conn_t *conn, acap_cmd_t **ret,
		   char *fmt, ...)
{
    va_list pvar;
    char *percent;
    char tag[50], buf[50];
    struct protstream *out = conn->pout;
    acap_cmd_t *cmd;

    if (!conn) return ACAP_NO_CONNECTION;
    cmd = (acap_cmd_t *) malloc(sizeof(acap_cmd_t));

    /* lock conn here */
    /* generate tag */
    sprintf(tag, "%d", conn->next_tag++);

    cmd->callbacks = NULL;

    cmd->next = conn->commands;
    conn->commands = cmd;
    /* unlock conn here */

    prot_write(out, tag, strlen(tag));
    prot_putc(' ', out);

    /* setup cmd structure */
    cmd->tag = strdup(tag);
    *ret = cmd;

    /* send out command */
    va_start(pvar, fmt);
    while ((percent = strchr(fmt, '%')) != NULL) {
	prot_write(out, fmt, percent - fmt);
	switch (*++percent) {
	case '%':
	    prot_putc('%', out);
	    break;
	case 'd':
	{
	    int i = va_arg(pvar, int);
	    sprintf(buf, "%d", i);
	    prot_write(out, buf, strlen(buf));
	    break;
	}
	case 's':
	{
	    char *s = va_arg(pvar, char *);
	    int l = strlen(s);

	    if (send_quoted_p(l, s)) {
		prot_putc('"', out);
		prot_write(out, s, l);
		prot_putc('"', out);
	    } else {
		prot_printf(out, "{%d+}\r\n", l);
		prot_write(out, s, l);
	    }
	    break;
	}
	case 'S':
	{
	    int l = va_arg(pvar, int);
	    char *s = va_arg(pvar, char *);

	    prot_printf(out, "{%d+}\r\n", l);
	    prot_write(out, s, l);
	    break;
	}
	case 'v':
	{
	    acap_value_t *v = va_arg(pvar, acap_value_t *);
	    
	    if (send_quoted_p(v->len, v->data)) {
		prot_putc('"', out);
		prot_write(out, v->data, v->len);
		prot_putc('"', out);
	    } else {
		prot_printf(out, "{%d+}\r\n", v->len);
		prot_write(out, v->data, v->len);
	    }
	    break;
	}
	case 'c':
	{
	    int i = va_arg(pvar, int);
	    prot_putc(i, out);
	    break;
	}
	default:
	    abort();
	}
	fmt = percent+1;
    }
    prot_write(out, fmt, strlen(fmt));
    va_end(pvar);
    prot_putc('\r', out);
    prot_putc('\n', out);

    return ACAP_OK;
}

void acap_cmd_free(acap_conn_t *conn, acap_cmd_t *cmd)
{
    acap_cmd_t *p;

    if (!conn) return;
    p = conn->commands;
    if (p == cmd) {
	conn->commands = cmd->next;
    } else {
	while (p->next && (p->next != cmd)) {
	    p = p->next;
	}
	p->next = cmd->next;
    }

    free(cmd->tag);
    while (cmd->callbacks) {
	struct acap_cb *ptr = cmd->callbacks->next;
	free(cmd->callbacks);
	cmd->callbacks = ptr;
    }
    free(cmd);
}

/* ------------------ callbacks ------------------------------- */

static int acap_register_conn_callback(acap_conn_t *conn, acap_cb_type cbtype,
				      acap_cb_generic *f, void *rock)
{
    struct acap_cb *new = (struct acap_cb *) malloc(sizeof(struct acap_cb));

    new->t = cbtype;
    new->cb = f;
    new->rock = rock;

    /* lock conn here */
    new->next = conn->callbacks;
    conn->callbacks = new;
    /* unlock conn here */

    return ACAP_OK;
}

static int acap_register_cmd_callback(acap_cmd_t *cmd, acap_cb_type cbtype,
				      acap_cb_generic *f, void *rock)
{
    struct acap_cb *new = (struct acap_cb *) malloc(sizeof(struct acap_cb));

    new->t = cbtype;
    new->cb = f;
    new->rock = rock;

    /* lock conn here */
    new->next = cmd->callbacks;
    cmd->callbacks = new;
    /* unlock conn here */

    return ACAP_OK;
}

static int acap_register_context_callback(acap_context_t *context, 
					  acap_cb_type cbtype,
					  acap_cb_generic *f, void *rock)
{
    struct acap_cb *new = (struct acap_cb *) malloc(sizeof(struct acap_cb));

    new->t = cbtype;
    new->cb = f;
    new->rock = rock;

    /* lock conn here */
    new->next = context->callbacks;
    context->callbacks = new;
    /* unlock conn here */

    return ACAP_OK;
}

/* ----------------- datastructure stuff ------------------ */

acap_attribute_t *acap_attribute_new(char *name)
{
    acap_attribute_t *ret = 
	(acap_attribute_t *) malloc(sizeof(acap_attribute_t));

    ret->name = strdup(name);
    ret->v = NULL;
    
    return ret;
}

acap_attribute_t *acap_attribute_new_simple (char *attrname, char *attrvalue)
{
    acap_attribute_t *ret;

    /* call function to create new attribute */
    ret = acap_attribute_new(attrname);
    if (ret == NULL) return NULL;

    if (attrvalue) {
	int attrlen = strlen(attrvalue);

	/* fill in it's single string */
	ret->t = ACAP_VALUE_SINGLE;
	
	ret->v = (acap_value_t *) malloc(sizeof(acap_value_t) + attrlen + 1);

	ret->v->len = attrlen;
	ret->v->next = NULL;
	strcpy(ret->v->data, attrvalue);
    } else {
	ret->t = ACAP_VALUE_DEFAULT;
	ret->v = NULL;
    }

    return ret;
}

void acap_attribute_free(acap_attribute_t *a)
{
    if (!a) return;
		       
    if (a->name) free(a->name);
    while (a->v) {
	acap_value_t *w = a->v;
	a->v = w->next;
	free(w);
    }
    free(a);
}

static int attr_comp(const void *v1, const void *v2)
{
    /* this depends on the fact that the _name_ is the first
       element in the attribute structure!!! */
    const char **n1 = (const char **) v1;
    const char **n2 = (const char **) v2;

    return strcmp(*n1, *n2);
}

acap_entry_t *acap_entry_new(char *name)
{
    acap_entry_t *ret =
	(acap_entry_t *) malloc(sizeof(acap_entry_t));

    ret->name = strdup(name);
    ret->refcount = 1;
    ret->attrs = skiplist_new(10, 0.5, &attr_comp);

    return ret;
}

acap_entry_t *acap_entry_copy(acap_entry_t *e)
{
    e->refcount++;

    return e;
}

void acap_entry_free(acap_entry_t *e)
{
    if (!e) return;
    e->refcount--;

    if (e->refcount != 0) return;

    /* free it */
    free(e->name);
    /* free the attributes & free the list */
    skiplist_freeeach(e->attrs, (void (*)(const void *))acap_attribute_free);
    free(e);
}

char *acap_entry_getname(acap_entry_t *e)
{
    return e->name;
}

acap_value_t *acap_entry_getattr(acap_entry_t *e, char *attrname)
{
    acap_attribute_t *a = (acap_attribute_t *) ssearch(e->attrs, &attrname);

    if (a == NULL) return NULL;
    if (a->t == ACAP_VALUE_NIL || a->t == ACAP_VALUE_DEFAULT) return NULL;
    return a->v;
}

char *acap_entry_getattr_simple(acap_entry_t *e, char *attrname)
{
    acap_attribute_t *a = (acap_attribute_t *) ssearch(e->attrs, &attrname);

    if (a == NULL) return NULL;
    if (a->t == ACAP_VALUE_NIL || a->t == ACAP_VALUE_DEFAULT) return NULL;
    return a->v->data;
}

/* ----------------------- process stuff --------------------- */
static void eatline(acap_conn_t *conn, int c)
{
    struct protstream *in = conn->pin;
    int state = 0;
    char *statediagram = " {}\r";
    int size = -1;

    for (;;) {
	if (c == '\n') return;
	if (c == statediagram[state+1]) {
	    state++;
	    if (state == 1) size = 0;
	    else if (c == '\r') {
		/* Got a literal */
		c = prot_getc(in);/* Eat newline */
		while (size) {
		    c = prot_getc(in); /* Eat contents */
		}
		state = 0;	/* Go back to scanning for eol */
	    }
	}
	else if (state == 1 && isdigit(c)) {
	    size = size * 10 + c - '0';
	}
	else state = 0;

	c = prot_getc(in);
	if (c == EOF) return;
    }
}

struct sbuf {
    char *s;
    int alloc;
    int len;
};

#define BUFGROWSIZE 128
static int getstring(acap_conn_t *conn, struct sbuf *buf)
{
    int c;
    int i, len =0;
    int sawdigit = 0;
    struct protstream *in = conn->pin;

    if (buf->alloc == 0) {
	buf->alloc = BUFGROWSIZE - 1;
	buf->s = malloc(buf->alloc + 1);
    }

    c = prot_getc(in);
    switch (c) {
    case '\"':
	/* quoted string; max size is 1024 */
	for (;;) {
	    c = prot_getc(in);
	    if (c == '\\') {
		c = prot_getc(in);
	    } else if (c == '\"') {
		buf->s[len] = '\0';
		buf->len = len;
		return prot_getc(in);
	    } else if (c == EOF || c == '\r' || c == '\n') {
		buf->s[len] = '\0';
		if (c != EOF) prot_ungetc(c, in);
		return EOF;
	    }
	    if (len == buf->alloc) {
		buf->alloc += BUFGROWSIZE;
		buf->s = realloc(buf->s, buf->alloc+1);
	    }
	    buf->s[len++] = c;
	    if (len > 1024) {
		buf->s[len] = '\0';
		return EOF;
	    }
	}
    case '{':
	/* literal */
	buf->s[0] = '\0';
	while ((c = prot_getc(in)) != EOF && isdigit(c)) {
	    sawdigit = 1;
	    len = len * 10 + c - '0';
	}
	if (!sawdigit || c != '}') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	c = prot_getc(in);
	if (c != '\r') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	c = prot_getc(in);
	if (c != '\n') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	if (len >= buf->alloc) {
	    buf->alloc = len+1;
	    buf->s = realloc(buf->s, buf->alloc + 1);
	}
	for (i = 0; i < len; i++) {
	    c = prot_getc(in);
	    if (c == EOF) {
		buf->s[len] = '\0';
		return EOF;
	    }
	    buf->s[i] = c;
	}
	buf->s[len] = '\0';
	buf->len = len;
	return prot_getc(in);

    default:
	/* not a string! */
	buf->s[0] = '\0';
	if (c != EOF) prot_ungetc(c, in);
	return EOF;
    }
}

static int getvalstr(acap_conn_t *conn, acap_value_t **ret)
{
    int c;
    int i, len =0;
    int sawdigit = 0;
    struct protstream *in = conn->pin;
    acap_value_t *v;

    c = prot_getc(in);
    switch (c) {
    case '\"':
	/* quoted string; max size is 1024 */
	v = (acap_value_t *) malloc(sizeof(acap_value_t) + 1024);
	v->next = NULL;
	for (;;) {
	    c = prot_getc(in);
	    if (c == '\\') {
		c = prot_getc(in);
	    } else if (c == '\"') {
		v->data[len] = '\0';
		v->len = len;
		*ret = v;
		return prot_getc(in);
	    } else if (c == EOF || c == '\r' || c == '\n') {
		v->data[len] = '\0';
		if (c != EOF) prot_ungetc(c, in);
		free(v);
		return EOF;
	    }
	    v->data[len++] = c;
	    if (len == 1024) {
		free(v);
		return EOF;
	    }
	}
    case '{':
	/* literal */
	while ((c = prot_getc(in)) != EOF && isdigit(c)) {
	    sawdigit = 1;
	    len = len * 10 + c - '0';
	}
	if (!sawdigit || c != '}') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	c = prot_getc(in);
	if (c != '\r') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	c = prot_getc(in);
	if (c != '\n') {
	    if (c != EOF) prot_ungetc(c, in);
	    return EOF;
	}
	v = (acap_value_t *) malloc(sizeof(acap_value_t) + len + 1);
	v->next = NULL;
	for (i = 0; i < len; i++) {
	    c = prot_getc(in);
	    if (c == EOF) {
		free(v);
		return EOF;
	    }
	    v->data[i] = c;
	}
	v->data[len] = '\0';
	v->len = len;
	*ret = v;
	return prot_getc(in);

    default:
	/* not a string! */
	if (c != EOF) prot_ungetc(c, in);
	return EOF;
    }
}

static int getattr(acap_conn_t *conn, char *attrname, acap_attribute_t **ret)
{
    /* expecting either STRING or (STRING SP STRING ... STRING) or NIL */
    int ch;
    acap_attribute_t *attr = acap_attribute_new(attrname);

    ch = prot_getc(conn->pin);
    switch (ch) {
    case '(':
	ch = ' ';
	attr->t = ACAP_VALUE_LIST;
	attr->v = NULL;
	while (ch == ' ') {
	    acap_value_t *nv;
	    ch = getvalstr(conn, &nv);
	    if (ch != EOF) {
		nv->next = attr->v;
		attr->v = nv;
	    }
	}
	if (ch == EOF) ch = prot_getc(conn->pin);
	if (ch != ')') {
	    acap_attribute_free(attr);
	    return EOF;
	} else {
	    *ret = attr;
	    return prot_getc(conn->pin);
	}
	break;
    case 'N': case 'n':
	ch = prot_getc(conn->pin);
	if (ch == 'i' || ch == 'I') ch = prot_getc(conn->pin);
	if (ch == 'l' || ch == 'L') {
	    attr->v = NULL;
	    attr->t = ACAP_VALUE_NIL;
	    *ret = attr;
	    return prot_getc(conn->pin);
	} else {
	    if (ch != EOF) prot_ungetc(ch, conn->pin);
	    return EOF;
	}
	break;
    default:
	prot_ungetc(ch, conn->pin);
	/* maybe it's a string? */
	attr->t = ACAP_VALUE_SINGLE;
	ch = getvalstr(conn, &attr->v);
	if (ch == EOF) {
	    acap_attribute_free(attr);
	    return EOF;
	} else {
	    *ret = attr;
	    return ch;
	}
	break;
    }
}

static int get_integer(acap_conn_t *conn, unsigned int *ret)
{
    int c;
    unsigned int acc = 0;
    int saw = 0;

    c = prot_getc(conn->pin);
    while ((c != EOF) && isdigit(c)) {
	saw = 1;
	acc = acc * 10 + c - '0';
	c = prot_getc(conn->pin);
    }

    if (!saw || (c == EOF)) {
	if (c != EOF) prot_ungetc(c, conn->pin);
	return EOF;
    }

    *ret = acc;
    return c;
}

/* gets the entry describe in the cmd structure out */
static int get_entry(acap_conn_t *conn, const struct acap_requested *ask, 
		     char *name, acap_entry_t **ret)
{
    int a = 0; /* the attribute in "ask" are we scanning */
    acap_entry_t *e = acap_entry_new(name);
    int ch;
    static struct sbuf attrname;
    acap_attribute_t *attribute;

    *ret = NULL;

    ch = ' ';
    /* loop through ask, recording what we get in "got" */
    for (a = 0; a < ask->n_attrs; a++) {
	/* getting cmd->ask->attrname[a]; this can be multiple entries
	   if it ends in a star */
	char *curattr = ask->attrs[a].attrname;
	int curmeta = ask->attrs[a].ret;

	if (ch != ' ') { /* error ! */
	    if (ch != EOF) prot_ungetc(ch, conn->pin);
	    return EOF;
	}

	if (curattr[strlen(curattr) - 1] == '*') {
	    /* expecting a list of ATTR VALUE */

	    ch = prot_getc(conn->pin);
	    if (ch != '(') {
		if (ch != EOF) prot_ungetc(ch, conn->pin);
		return EOF;
	    }
	    ch = prot_getc(conn->pin);
	    while (ch != ')') {
		/* now expect: "(" "ATTR" SP "VALUE" ")" [SP / ")"] */
		if (curmeta == 0) {
		    if (ch != '(') {
			if (ch != EOF) prot_ungetc(ch, conn->pin);
			return EOF;
		    }
		    ch = getstring(conn, &attrname);
		    if (ch != ' ') {
			if (ch != EOF) prot_ungetc(ch, conn->pin);
			return EOF;
		    }
		    ch = getattr(conn, attrname.s, &attribute);
		    if (ch != ')') {
			if (ch != EOF) prot_ungetc(ch, conn->pin);
			return EOF;
		    }
		    ch = prot_getc(conn->pin);

		    /* insert (ATTR VALUE) into our entry */
		    sinsert(e->attrs, (void *) attribute);
		} else { /* we're looking at a list of metadata */
		    prot_ungetc(ch, conn->pin);

		    ch = getstring(conn, &attrname);
		    if ((ch != ' ') && (ch != ')')) {
			if (ch != EOF) prot_ungetc(ch, conn->pin);
			return EOF;
		    }
		    
		    attribute = acap_attribute_new(attrname.s);

		    /* insert (ATTR NULL) into our entry */
		    sinsert(e->attrs, (void *) attribute);
		}

		/* if we're looking at a SP, advance */
		if (ch == ' ') ch = prot_getc(conn->pin);
	    }
	    ch = prot_getc(conn->pin);
	} else {
	    /* expecting just a NIL or VALUE corresponding to curattr */
	    ch = getattr(conn, curattr, &attribute);

	    sinsert(e->attrs, (void *) attribute);
	}
    }

    *ret = e;
    return ch;
}


static int process_continuation(acap_conn_t *conn)
{
    int ch;
    struct acap_cb *ptr;
    acap_value_t *v;
    
    ch = getvalstr(conn, &v);
    if (ch != '\r') {
	if (ch != EOF) prot_ungetc(ch, conn->pin);
	return EOF;
    }
    
    /* look for callbacks */
    ptr = conn->callbacks;
    while (ptr) {
	if (CB_CONTINUATION == ptr->t) {
	    acap_cb_continuation *f = (acap_cb_continuation *) ptr->cb;
	    f(v, ptr->rock);
	}
	ptr = ptr->next;
    }

    /* free value */
    free(v);

    return ch;
}

static int process_atom(char *tag, char *atom, acap_conn_t *conn)
{
    acap_cb_type t;
    acap_cmd_t *cmd = NULL;
    acap_context_t *ctxt = NULL;
    struct acap_cb *ptr, *gb_ptr;
    int ch = EOF; /* xxx??? */
    static struct sbuf context; /* should be made connection-local ! */
    static struct sbuf buf;
    acap_result_t res;

    /* first, let's find out if this will trigger any callbacks */
    /* canonify tag ('*' => NULL) */
    if (!strcmp(tag, "*")) {
	tag = NULL;
    } else {
	/* find the command structure for this call */

	/* lock conn here */
	cmd = conn->commands;
	/* unlock conn here */
	while (cmd && strcmp(tag, cmd->tag)) {
	    cmd = cmd->next;
	}
	if (!cmd) { /* what sort of weird tag did i get back here? */
	    return EOF;
	}
    }

    /* identify atom */
    /* we'll assume it's an extension if we don't recognize it */
    t = CB_EXTENSION;
    res = ACAP_RESULT_NOTDONE;
    switch (atom[0]) {
    case 'a': case 'A':
	if (!strcasecmp(atom, "ADDTO")) {
	    /* ADDTO */
	    t = CB_CTX_ADDTO;
	} else if (!strcasecmp(atom, "ALERT")) {
	    /* ALERT */
	    t = CB_ALERT;
	}
	break;
    case 'b': case 'B':
	if (!strcasecmp(atom, "BAD")) {
	    /* BAD */
	    t = CB_CMD_DONE;
	    res = ACAP_RESULT_BAD;
	} else if (!strcasecmp(atom, "BYE")) {
	    /* BYE */
	    t = CB_BYE;
	}
	break;
    case 'c': case 'C':
	if (!strcasecmp(atom, "CHANGE")) {
	    /* CHANGE */
	    t = CB_CTX_CHANGE;
	}
	break;
    case 'd': case 'D':
	if (!strcasecmp(atom, "DELETED")) {
	    /* DELETED */
	    t = CB_DELETED;
	}
	break;
    case 'e': case 'E':
	if (!strcasecmp(atom, "ENTRY")) {
	    /* ENTRY */
	    t = CB_ENTRY;
	}
	break;
    case 'l': case 'L':
	if (!strcasecmp(atom, "LANG")) {
	    /* LANG */
	    t = CB_LANG;
	} else if (!strcasecmp(atom, "LISTRIGHTS")) {
	    /* LISTRIGHTS */
	    t = CB_LISTRIGHTS;
	}
	break;
    case 'm': case 'M':
	if (!strcasecmp(atom, "MODTIME")) {
	    /* MODTIME */
	    if (tag) {
		t = CB_MODTIME;
	    } else {
		t = CB_CTX_MODTIME;
	    }
	} else if (!strcasecmp(atom, "MYRIGHTS")) {
	    /* MYRIGHTS */
	    t = CB_MYRIGHTS;
	}
	break;
    case 'n': case 'N':
	if (!strcasecmp(atom, "NO")) {
	    /* NO */
	    t = CB_CMD_DONE;
	    res = ACAP_RESULT_NO;
	}
	break;
    case 'o': case 'O':
	if (!strcasecmp(atom, "OK")) {
	    /* OK */
	    t = CB_CMD_DONE;
	    res = ACAP_RESULT_OK;
	}
	break;
    case 'q': case 'Q':
	if (!strcasecmp(atom, "QUOTA")) {
	    /* QUOTA */
	    t = CB_QUOTA;
	}
	break;
    case 'r': case 'R':
	if (!strcasecmp(atom, "REFER")) {
	    /* REFER */
	    t = CB_REFER;
	} else if (!strcasecmp(atom, "REMOVEFROM")) {
	    /* REMOVEFROM */
	    t = CB_CTX_REMOVEFROM;
	}
	break;
    default:
	break;
    }

    /* do we need to get the context name? */
    if ((t == CB_CTX_ADDTO) || (t == CB_CTX_CHANGE)
	|| (t == CB_CTX_MODTIME) || (t == CB_CTX_REMOVEFROM)) {
	ch = getstring(conn, &context);
	if (ch != ' ') return ch;

	/* find the context */
	ctxt = conn->contexts;
	while (ctxt && strcmp(context.s, ctxt->name)) {
	    ctxt = ctxt->next;
	}
	if (!ctxt) { /* huh? */
	    return EOF;
	}
    }

    if (cmd) ptr = cmd->callbacks;
    else if (ctxt) ptr = ctxt->callbacks;
    else ptr = NULL;

    while (ptr && (t != ptr->t)) {
	ptr = ptr->next;
    }
    gb_ptr = conn->callbacks;
    while (gb_ptr && (t != gb_ptr->t)) {
	gb_ptr = gb_ptr->next;
    }
    
    /* no? return now (we eat the rest of the line when we return) */
    if (ptr == NULL && gb_ptr == NULL) {
	return prot_getc(conn->pin);
    }

    /* yes? process the line & make callbacks, but not the CRLF */
    switch (t) {
    case CB_CTX_ADDTO:		/* ADDTO & REMOVEFROM are identical */
    case CB_CTX_REMOVEFROM:
    {
	acap_entry_t *entry;
	unsigned position;

	ch = getstring(conn, &buf);
	if (ch != ' ') return ch;
	ch = get_integer(conn, &position);
	if (ch == EOF) return ch;

	/* grab entry information, if any */
	if (t == CB_CTX_ADDTO) {
	    if (ch != ' ') return ch;
	    ch = get_entry(conn, ctxt->ask, buf.s, &entry);
	    if (ch == EOF) return ch;
	} else { 
	    /* t == CB_CTX_REMOVEFROM, so no entry name */
	    entry = acap_entry_new(buf.s);
	}

	while (ptr != NULL) {
	    if (t == ptr->t) {
		void (*f)(acap_entry_t *, unsigned, void *) = 
		    (void (*)(acap_entry_t *, unsigned, void *)) ptr->cb;
		f(entry, position, ptr->rock);
	    }
	    ptr = ptr->next;
	}
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		void (*f)(acap_entry_t *, unsigned, void *) = 
		    (void (*)(acap_entry_t *, unsigned, void *)) gb_ptr->cb;
		f(entry, position, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}

	acap_entry_free(entry);
	break;
    }

    case CB_ALERT:
    {
	/* possible CODE here */
	ch = prot_getc(conn->pin);
	if (ch == '(') {
	    /* skip the code */
	    while (ch != ')') ch = prot_getc(conn->pin);
	    ch = prot_getc(conn->pin);
	} else {
	    prot_ungetc(ch, conn->pin);
	}
	ch = getstring(conn, &buf);
	if (ch == EOF) return ch;
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		acap_cb_alert *falert = (acap_cb_alert *) gb_ptr->cb;
		falert(buf.s, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}
	break;
    }

    case CB_BYE:
    {
	/* possible CODE here */
	ch = prot_getc(conn->pin);
	if (ch == '(') {
	    /* skip the code */
	    while (ch != ')') ch = prot_getc(conn->pin);
	    ch = prot_getc(conn->pin);
	} else {
	    prot_ungetc(ch, conn->pin);
	}
	ch = getstring(conn, &buf);
	if (ch == EOF) return ch;
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		acap_cb_bye *f = (acap_cb_bye *) gb_ptr->cb;
		f(buf.s, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}
	break;
    }

    case CB_CTX_CHANGE:
    {
	acap_entry_t *entry;
	unsigned old_position, new_position;

	ch = getstring(conn, &buf);
	if (ch != ' ') return ch;
	ch = get_integer(conn, &old_position);
	if (ch != ' ') return ch;
	ch = get_integer(conn, &new_position);
	if (ch != ' ') return ch;
	
	ch = get_entry(conn, ctxt->ask, buf.s, &entry);
	if (ch == EOF) return ch;
	while (ptr != NULL) {
	    if (t == ptr->t) {
		void (*f)(acap_entry_t *, unsigned, unsigned, void *) = 
		    (void (*)(acap_entry_t *, unsigned, unsigned, void *))
		            ptr->cb;
		f(entry, old_position, new_position, ptr->rock);
	    }
	    ptr = ptr->next;
	}
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		void (*f)(acap_entry_t *, unsigned, unsigned, void *) = 
		    (void (*)(acap_entry_t *, unsigned, unsigned, void *)) 
		            ptr->cb;
		f(entry, old_position, new_position, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}

	acap_entry_free(entry);
	break;
    }

    case CB_CONTINUATION:
	fatal("impossible",-1);
	break;

    case CB_DELETED:
    {
	ch = getstring(conn, &buf);
	if (ch == EOF) return ch;
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		acap_cb_deleted *f = (acap_cb_deleted *) gb_ptr->cb;
		f(buf.s, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}
	break;
    }

    case CB_ENTRY:
    {
	acap_entry_t *entry;

	ch = getstring(conn, &buf);
	if (ch != ' ')
	    return ch;
	ch = get_entry(conn, cmd->ask, buf.s, &entry);
	if (ch == EOF)
	    return ch;
	while (ptr != NULL) {
	    if (t == ptr->t) {
		void (*f)(acap_entry_t *, void *) = 
		    (void (*)(acap_entry_t *, void *)) ptr->cb;
		f(entry, ptr->rock);
	    }
	    ptr = ptr->next;
	}
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		void (*f)(acap_entry_t *, void *) = 
		    (void (*)(acap_entry_t *, void *)) gb_ptr->cb;
		f(entry, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}

	acap_entry_free(entry);
	break;
    }

    case CB_EXTENSION:
	break;

    case CB_LANG:
	fatal("not implemented", 3);
	break;

    case CB_LISTRIGHTS:
	fatal("not implemented", 3);
	break;

    case CB_CTX_MODTIME:	/* at this stage, these look the same */
    case CB_MODTIME:
    {
	ch = getstring(conn, &buf);
	while (ptr != NULL) {
	    if (t == ptr->t) {
		void (*f)(char *, void *) = (void (*)(char *, void *))ptr->cb;
		f(buf.s, ptr->rock);
	    }
	    ptr = ptr->next;
	}
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		void (*f)(char *, void *) = 
		    (void (*)(char *, void *))gb_ptr->cb;
		f(buf.s, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}
	break;
    }

    case CB_MYRIGHTS:
	fatal("not implemented", 3);
	break;

    case CB_QUOTA:
	fatal("not implemented", 3);
	break;

    case CB_REFER:
	fatal("not implemented", 3);
	break;

    case CB_CMD_DONE:
	/* possible CODE here */
	ch = prot_getc(conn->pin);
	if (ch == '(') {
	    /* skip the code */
	    while (ch != ')') ch = prot_getc(conn->pin);
	    ch = prot_getc(conn->pin);
	} else {
	    /* oops, not a code! */
	    prot_ungetc(ch, conn->pin);
	}
	ch = getstring(conn, &buf);
	while (ptr != NULL) {
	    if (t == ptr->t) {
		acap_cb_completion_t *f = (acap_cb_completion_t *) ptr->cb;
		f(res, ptr->rock);
	    }
	    ptr = ptr->next;
	}
	while (gb_ptr != NULL) {
	    if (t == gb_ptr->t) {
		acap_cb_completion_t *f = (acap_cb_completion_t *) gb_ptr->cb;
		f(res, gb_ptr->rock);
	    }
	    gb_ptr = gb_ptr->next;
	}
	/* now delete the command from our "outstanding commands" list */
	acap_cmd_free(conn, cmd);
	break;

    case CB_ANY:
	break;
    }

    /* now eat the rest of the line */
    return ch;
}

/* setting ACAP_PROCESS_NOBLOCK ensures that we don't block waiting for
   a new line.  however, we will block waiting for the completion of
   a response, which, depending on network/server conditions, could
   be a significant amount of time. let the user beware */
int acap_process_line(acap_conn_t *conn, int flag)
{
    char tag[33];
    char atom[1025];
    int taglen, atomlen, ch;
    int r;
    enum { TAG, ATOM, CRLF } s;

    if (!conn) return ACAP_NO_CONNECTION;
    if (flag & ACAP_PROCESS_NOBLOCK) {
	prot_NONBLOCK(conn->pin);
    } else {
	prot_BLOCK(conn->pin);
    }

    /* start state */
    r = ACAP_OK;
    taglen = 0;
    atomlen = 0;
    s = TAG;

    /* special case the first character */
    errno = 0;
    ch = prot_getc(conn->pin);
    if (ch == EOF) {
	if (errno == EAGAIN) {
	    return ACAP_WOULD_BLOCK;
	} else {
	    return ACAP_NO_CONNECTION;
	}
    }
    prot_BLOCK(conn->pin);

    taglen = 0;
    for (;;) {
	if (ch == EOF) {
	    if (conn->pin->eof) return ACAP_NO_CONNECTION;
	    else {
		eatline(conn, ch);
		return ACAP_PROTOCOL_ERROR;
	    }
	}

	switch (s) {
	case TAG:
	    if (ch != ' ') {
		if (taglen < 32) {
		    tag[taglen++] = ch;
		    ch = prot_getc(conn->pin);
		} else {
		    r = ACAP_PROTOCOL_ERROR;
		    ch = prot_getc(conn->pin);
		    s = CRLF;
		}
	    } else {
		tag[taglen] = '\0';
		/* check if tag represents a continuation */
		if (taglen == 1 && tag[0] == '+') {
		    /* ok, it's a continuation */
		    ch = process_continuation(conn);
		    s = CRLF;
		} else {
		    ch = prot_getc(conn->pin);
		    s = ATOM;
		}
	    }
	    break;

	case ATOM:
	    if (ch != ' ') {
		if (atomlen < 1024) {
		    atom[atomlen++] = ch;
		    ch = prot_getc(conn->pin);
		} else {
		    r = ACAP_PROTOCOL_ERROR;
		    ch = prot_getc(conn->pin);
		    s = CRLF;
		}
	    } else {
		atom[atomlen] = '\0';
		ch = process_atom(tag, atom, conn);
		s = CRLF;
	    }
	    break;

	case CRLF:
	    eatline(conn, ch);
	    return r;
	}
    }
}

int acap_process_outstanding(acap_conn_t *conn)
{
    int r = ACAP_OK;

    if (!conn) return ACAP_NO_CONNECTION;
    
    while (r == ACAP_OK) {
	r = acap_process_line(conn, ACAP_PROCESS_NOBLOCK);
    }
    if (r == ACAP_WOULD_BLOCK) r = ACAP_OK;

    return r;
}

static void cmd_done(acap_result_t res, void *rock)
{
    acap_result_t *p = (acap_result_t *) rock;

    *p = res;
}

int acap_process_on_command(acap_conn_t *conn, acap_cmd_t *cmd,
			    acap_result_t *res)
{
    acap_result_t cmd_fini = ACAP_RESULT_NOTDONE;
    int r;

    if (!conn) return ACAP_NO_CONNECTION;

    r = acap_register_cmd_callback(cmd, CB_CMD_DONE,
				   (acap_cb_generic *) &cmd_done, &cmd_fini);
    if (r != ACAP_OK) {
	return r;
    }
    
    while (cmd_fini == ACAP_RESULT_NOTDONE) {
	r = acap_process_line(conn, 0);
	if (r != ACAP_OK) {
	    return r;
	}
    }

    if (res) *res = cmd_fini;
    return ACAP_OK;
}


/* ------------------- search stuff ------------------- */
acap_context_t *acap_context_new(acap_conn_t *conn, char *name,
				 const struct acap_requested *ask)
{
    acap_context_t *ret = (acap_context_t *) malloc(sizeof(acap_context_t));

    ret->name = strdup(name);
    ret->callbacks = NULL;
    ret->ask = ask;

    /* lock conn here */
    ret->next = conn->contexts;
    conn->contexts = ret;
    /* unlock conn here */
   
    return ret;
}

void acap_context_free(acap_conn_t *conn, acap_context_t *context)
{
    acap_cmd_t *cmd;
    int r;
    acap_context_t *p;

    r = acap_cmd_start(conn, &cmd, "FREECONTEXT %s", context->name);
    if (r == ACAP_OK) r = acap_process_on_command(conn, cmd, NULL);

    p = conn->contexts;
    if (p == context) {
	conn->contexts = context->next;
    } else {
	while (p->next && (p->next != context)) {
	    p = p->next;
	}
	p->next = context->next;
    }

    while (context->callbacks) {
	struct acap_cb *ptr = context->callbacks->next;
	free(context->callbacks);
	context->callbacks = ptr;
    }

    free(context->name);
    free(context);
}

int acap_search_dataset(acap_conn_t *conn,
			const char *dataset, 
			char *spec,
			int depth,
			const struct acap_requested *ret_attrs,
			const struct acap_sort *sort_order,
			const acap_cb_completion_t *cmd_cb,
			const struct acap_search_callback *search_cb,
			acap_context_t **context_ptr,
			const struct acap_context_callback *context_cb,
			void *rock,
			acap_cmd_t **ret)
{
    char contextname[50];
    acap_cmd_t *cmd;
    acap_context_t *context = NULL;

    if (!conn) return ACAP_NO_CONNECTION;
    if (!dataset) return ACAP_BAD_PARAM;
    if (!spec) return ACAP_BAD_PARAM;
    if (!context_ptr && context_cb) return ACAP_BAD_PARAM;
    if (depth < 0) return ACAP_BAD_PARAM;

    cmd = acap_cmd_new(conn);

    /* lock conn here */
    cmd->ask = ret_attrs;
    if (ret) *ret = cmd;

    prot_printf(conn->pout, "%s SEARCH \"%s\" ", cmd->tag, dataset);
    if (search_cb && ret_attrs && (ret_attrs->n_attrs > 0)) {
	int i;

	prot_printf(conn->pout, "RETURN (");
	for (i = 0; i < ret_attrs->n_attrs; i++) {
	    if (i > 0) prot_putc(' ', conn->pout);
	    prot_printf(conn->pout, "{%d+}\r\n%s", 
			strlen(ret_attrs->attrs[i].attrname), 
			ret_attrs->attrs[i].attrname);

	    if (ret_attrs->attrs[i].ret & ATTRIBUTE) {
		prot_printf(conn->pout,"(\"attribute\")");
	    }
	    if (ret_attrs->attrs[i].ret & VALUE) {
		prot_printf(conn->pout,"(\"value\")");
	    }
	    if (ret_attrs->attrs[i].ret & SIZE) {
		prot_printf(conn->pout,"(\"size\")");
	    }
	}
	prot_printf(conn->pout, ") ");
    }

    prot_printf(conn->pout, "DEPTH %d ", depth);

    if (context_ptr) {
	sprintf(contextname, "%d", conn->next_tag++);
	context = acap_context_new(conn, contextname, ret_attrs);
	*context_ptr = context;
	prot_printf(conn->pout, "MAKECONTEXT ");
	if (sort_order) {
	    prot_printf(conn->pout, "ENUMERATE ");
	}
	if (context_cb) {
	    prot_printf(conn->pout, "NOTIFY ");
	}
	prot_printf(conn->pout, "\"%s\" ", contextname);
    }

    if (sort_order) {
	prot_printf(conn->pout, "SORT (");
	while (sort_order) {
	    prot_printf(conn->pout, "{%d+}\r\n%s {%d+}\r\n%s", 
			strlen(sort_order->attrname), sort_order->attrname,
			strlen(sort_order->comparator),sort_order->comparator);
	    if (sort_order->next) prot_putc(' ', conn->pout);
	    sort_order = sort_order->next;
	}
	prot_printf(conn->pout, ") ");
    }
    
    /* finish off the command */
    prot_printf(conn->pout, "%s\r\n", spec);

    /* add all the callbacks */
    if (cmd_cb) {
	acap_register_cmd_callback(cmd, CB_CMD_DONE,
				   (acap_cb_generic *) cmd_cb, rock);
    }
    if (search_cb) {
	if (search_cb->entry_data) {
	    acap_register_cmd_callback(cmd, CB_ENTRY,
				 (acap_cb_generic *) search_cb->entry_data, 
				       rock);
	}	    
	if (search_cb->modtime) {
	    acap_register_cmd_callback(cmd, CB_MODTIME,
				       (acap_cb_generic *) search_cb->modtime, 
				       rock);
	}
    }
    if (context_cb) {
	if (context_cb->addto) {
	    acap_register_context_callback(context, CB_CTX_ADDTO,
				   (acap_cb_generic *) context_cb->addto,
				   rock);
	}
	if (context_cb->removefrom) {
	    acap_register_context_callback(context, CB_CTX_REMOVEFROM, 
				   (acap_cb_generic *) context_cb->removefrom, 
				   rock);
	}
	if (context_cb->change) {
	    acap_register_context_callback(context, CB_CTX_CHANGE,
				   (acap_cb_generic *) context_cb->change, 
				   rock);
	}
	if (context_cb->modtime) {
	    acap_register_context_callback(context, CB_CTX_MODTIME,
				   (acap_cb_generic *) context_cb->modtime, 
				   rock);
	}
    }

    /* unlock conn here */

    return ACAP_OK;
}

int acap_search_context(acap_conn_t *conn,
			acap_context_t *context, char *spec,
			struct acap_requested *ret_attrs,
			struct acap_sort *sort_order,
			acap_cb_completion_t *cmd_cb,
			struct acap_search_callback *search_cb,
			void *rock,
			acap_cmd_t **ret)
{
    acap_cmd_t *cmd;

    if (!conn) return ACAP_NO_CONNECTION;
    if (!context) return ACAP_BAD_PARAM;
    if (!spec) return ACAP_BAD_PARAM;

    cmd = acap_cmd_new(conn);
	
    /* lock conn here */
    cmd->ask = ret_attrs;
    if (ret) *ret = cmd;

    prot_printf(conn->pout, "%s SEARCH \"%s\" ", cmd->tag, context->name);
    if (search_cb && ret_attrs && (ret_attrs->n_attrs > 0)) {
	int i;

	prot_printf(conn->pout, "RETURN (");
	for (i = 0; i < ret_attrs->n_attrs; i++) {
	    if (i > 0) prot_putc(' ', conn->pout);
	    prot_printf(conn->pout, "{%d+}\r\n%s", 
			strlen(ret_attrs->attrs[i].attrname), 
			ret_attrs->attrs[i].attrname);

	    if (ret_attrs->attrs[i].ret & ATTRIBUTE) {
		prot_printf(conn->pout,"(\"attribute\")");
	    }
	    if (ret_attrs->attrs[i].ret & VALUE) {
		prot_printf(conn->pout,"(\"value\")");
	    }
	    if (ret_attrs->attrs[i].ret & SIZE) {
		prot_printf(conn->pout,"(\"size\")");
	    }
	}
	prot_printf(conn->pout, ") ");
    }

    if (sort_order) {
	prot_printf(conn->pout, "SORT (");
	while (sort_order) {
	    prot_printf(conn->pout, "{%d+}\r\n%s {%d+}\r\n%s", 
			strlen(sort_order->attrname), sort_order->attrname,
			strlen(sort_order->comparator),sort_order->comparator);
	    if (sort_order->next) prot_putc(' ', conn->pout);
	    sort_order = sort_order->next;
	}
	prot_printf(conn->pout, ") ");
    }
    
    /* finish off the command */
    prot_printf(conn->pout, "%s\r\n", spec);

    /* add all the callbacks */
    if (cmd_cb) {
	acap_register_cmd_callback(cmd, CB_CMD_DONE,
				   (acap_cb_generic *) cmd_cb, rock);
    }
    if (search_cb) {
	if (search_cb->entry_data) {
	    acap_register_cmd_callback(cmd, CB_ENTRY,
				 (acap_cb_generic *) search_cb->entry_data, 
				       rock);
	}
	if (search_cb->modtime) {
	    acap_register_cmd_callback(cmd, CB_MODTIME,
				       (acap_cb_generic *) search_cb->modtime, 
				       rock);
	}
    }

    /* unlock conn here */

    return ACAP_OK;
}

int acap_updatecontext(acap_conn_t *conn,
		       acap_context_t *context,
		       acap_cb_completion_t *cmd_cb,
		       void *rock,
		       acap_cmd_t **ret)
{
    acap_cmd_t *cmd;

    if (!conn) return ACAP_NO_CONNECTION;
    if (!context) return ACAP_BAD_PARAM;

    cmd = acap_cmd_new(conn);
    
    /* lock conn here */
    if (ret) *ret = cmd;

    prot_printf(conn->pout, "%s UPDATECONTEXT \"%s\"\r\n", cmd->tag,
		context->name);

    if (cmd_cb) {
	acap_register_cmd_callback(cmd, CB_CMD_DONE,
				   (acap_cb_generic *) cmd_cb, rock);
    }

    /* unlock conn here */

    return ACAP_OK;
}

/* ------------------------- storing --------------------------- */
#define initial_time "20000121071919000000" /* xxx tim - what should this be? */

static void write_value(struct protstream *out, acap_value_t *v)
{
    if (send_quoted_p(v->len, v->data)) {
	prot_putc('"', out);
	prot_write(out, v->data, v->len);
	prot_putc('"', out);
    } else {
	prot_printf(out, "{%d+}\r\n", v->len);
	prot_write(out, v->data, v->len);
    }
}

int acap_store_entry(acap_conn_t *conn,
		     acap_entry_t *entry,
		     acap_cb_completion_t *cmd_cb,
		     void *rock,
		     int flags,
		     acap_cmd_t **ret)
{
    acap_cmd_t *cmd;
    acap_value_t *v;
    acap_attribute_t *attr;
    char *modtime = NULL;
    skipnode *node;
    int first;

    if (!conn) return ACAP_NO_CONNECTION;
    if ((flags & ACAP_STORE_INITIAL) && 
	(flags & ACAP_STORE_FORCE)) return ACAP_BAD_PARAM;
    if (!entry) return ACAP_BAD_PARAM;

    if (flags & ACAP_STORE_INITIAL) {
	modtime = initial_time;
    } else if (!(flags & ACAP_STORE_FORCE)) {
	v = acap_entry_getattr(entry, "modtime");
	if (v != NULL) {
	    modtime = v->data;
	}
    }

    cmd = acap_cmd_new(conn);

    /* lock conn */
    prot_printf(conn->pout, "%s STORE (\"%s\" ", cmd->tag, entry->name);
    if (modtime) {
	prot_printf(conn->pout, "UNCHANGEDSINCE \"%s\" ", modtime);
    }

    first = 1;
    for (attr = sfirst(entry->attrs, &node); attr != NULL; attr = snext(&node))
    {
	int l;

	if (!strcmp(attr->name, "modtime")) continue;
	if (!strcmp(attr->name, "entry")) continue;

	if (!first) {
	    prot_putc(' ', conn->pout);
	}
	first = 0;

	l = strlen(attr->name);
	if (send_quoted_p(l, attr->name)) {
	    prot_putc('"', conn->pout);
	    prot_write(conn->pout, attr->name, l);
	    prot_putc('"', conn->pout);
	} else {
	    prot_printf(conn->pout, "{%d+}\r\n", l);
	    prot_write(conn->pout, attr->name, l);
	}

	switch (attr->t) {
	case ACAP_VALUE_SINGLE:
	    prot_putc(' ', conn->pout);
	    write_value(conn->pout, attr->v);
	    break;

	case ACAP_VALUE_LIST:
	    prot_printf(conn->pout, " (\"value\" (");
	    for (v = attr->v; v != NULL; v = v->next) {
		write_value(conn->pout, v);
		if (v->next) { prot_putc(' ', conn->pout); }
	    }
	    prot_printf(conn->pout, "))");
	    break;

	case ACAP_VALUE_NIL:
	    prot_printf(conn->pout, " NIL");
	    break;

	case ACAP_VALUE_DEFAULT:
	    prot_printf(conn->pout, " DEFAULT");
	    break;
	}
    }

    prot_printf(conn->pout, ")\r\n");

    /* add all the callbacks */
    if (cmd_cb) {
	acap_register_cmd_callback(cmd, CB_CMD_DONE,
				   (acap_cb_generic *) cmd_cb, rock); 
    }

    /* unlock conn */

    if (ret) *ret = cmd;
    return ACAP_OK;
}

int acap_store_attribute(acap_conn_t *conn,
			 const char *entry,
			 acap_attribute_t *attr,
			 char *unchangedsince,
			 acap_cb_completion_t *cmd_cb,
			 void *rock,
			 acap_cmd_t **ret)
{
    acap_cmd_t *cmd;
    acap_value_t *v;
    int l;

    if (!conn) return ACAP_NO_CONNECTION;

    /* we can't explicitly set the modtime */
    if (!strcmp(attr->name, "modtime")) return ACAP_BAD_PARAM;
    if (!strcmp(attr->name, "entry") &&
	(attr->t == ACAP_VALUE_SINGLE || attr->t == ACAP_VALUE_LIST)) 
	return ACAP_NOT_SUPPORTED;

    cmd = acap_cmd_new(conn);
    /* lock conn */
    prot_printf(conn->pout, "%s STORE (\"%s\" ", cmd->tag, entry);
    if (unchangedsince) {
	prot_printf(conn->pout, "UNCHANGEDSINCE \"%s\" ", unchangedsince);
    }

    l = strlen(attr->name);
    prot_printf(conn->pout, "{%d+}\r\n", l);
    prot_write(conn->pout, attr->name, l);

    switch (attr->t) {
    case ACAP_VALUE_SINGLE:
	prot_putc(' ', conn->pout);
	write_value(conn->pout, attr->v);
	break;
	
    case ACAP_VALUE_LIST:
	prot_printf(conn->pout, " (\"value\" (");
	for (v = attr->v; v != NULL; v = v->next) {
	    write_value(conn->pout, v);
	    if (v->next) { prot_putc(' ', conn->pout); }
	}
	prot_printf(conn->pout, "))");
	
    case ACAP_VALUE_NIL:
	prot_printf(conn->pout, " NIL");
	break;
	
    case ACAP_VALUE_DEFAULT:
	prot_printf(conn->pout, " DEFAULT");
	break;
    }
    
    prot_printf(conn->pout, ")\r\n");

    /* unlock conn */

    if (ret) *ret = cmd;
    return ACAP_OK;

}


/* ---------------- deleting --------------------- */

int acap_delete_entry_name(acap_conn_t *conn,
			   const char *entryname,
			   acap_cb_completion_t *cmd_cb,
			   void *rock,
			   acap_cmd_t **ret)
{
    return acap_delete_attribute(conn,
				 (char *) entryname,
				 "entry",
				 cmd_cb,
				 rock,
				 ret);
}

int acap_delete_attribute(acap_conn_t *conn,
			  char *entryname,
			  char *attrname,
			  acap_cb_completion_t *cmd_cb,
			  void *rock,
			  acap_cmd_t **ret)
{
    int r;
    acap_attribute_t *attr;

    if (!conn) return ACAP_NO_CONNECTION;

    attr = acap_attribute_new (attrname);
    if (attr == NULL) return ACAP_NOMEM;
    
    attr->t = ACAP_VALUE_DEFAULT;

    r = acap_store_attribute(conn, entryname, attr, NULL,
			     cmd_cb, rock, ret);

    acap_attribute_free(attr);

    return r;    
}
