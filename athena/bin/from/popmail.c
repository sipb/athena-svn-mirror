/* 
 * $Id: popmail.c,v 1.11 1999-09-21 01:40:07 danw Exp $
 *
 */

static const char rcsid[] = "$Id: popmail.c,v 1.11 1999-09-21 01:40:07 danw Exp $";

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>
#ifdef HAVE_HESIOD
#include <hesiod.h>
#endif
#ifdef HAVE_KRB4
#include <krb.h>
#endif

#include "from.h"

#define NOTOK (-1)
#define OK 0
#define DONE 1

extern FILE *sfi;
extern FILE *sfo;
extern char Errmsg[80];
#ifdef HAVE_KRB4
#define KPOP_PORT 1109
#endif
extern int popmail_debug;

int pop_init(char *host)
{
    register struct hostent *hp;
    register struct servent *sp;
    struct sockaddr_in sin;
    register int s;
#ifdef HAVE_KRB4
    KTEXT ticket = (KTEXT)NULL;
    int rem;
    long authopts;
    char *hostname;
#else
    int lport = IPPORT_RESERVED - 1;
#endif

    if (!host)
	return(NOTOK);
    hp = gethostbyname(host);
    if (hp == NULL) {
	(void) sprintf(Errmsg, "MAILHOST unknown: %s", host);
	return(NOTOK);
    }

#ifdef HAVE_KRB4
    sp = getservbyname("kpop", "tcp");
    if (sp == 0) 
        sin.sin_port = htons(KPOP_PORT);
    else
        sin.sin_port = sp->s_port;
#else
    sp = getservbyname("pop", "tcp");
    if (sp == 0) {
	(void) strcpy(Errmsg, "tcp/pop: unknown service");
	return(NOTOK);
    }
    sin.sin_port = sp->s_port;
#endif

    sin.sin_family = hp->h_addrtype;
    memcpy(&sin.sin_addr, hp->h_addr, hp->h_length);
#ifdef HAVE_KRB4
    s = socket(AF_INET, SOCK_STREAM, 0);
#else
    s = rresvport(&lport);
#endif
    if (s < 0) {
	(void) sprintf(Errmsg, "error creating socket: %s", strerror(errno));
	return(NOTOK);
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
	(void) sprintf(Errmsg, "error during connect: %s", strerror(errno));
	(void) close(s);
	return(NOTOK);
    }
#ifdef HAVE_KRB4
    ticket = (KTEXT)malloc( sizeof(KTEXT_ST) );
    if (ticket == NULL) {
	fprintf (stderr, "from: out of memory");
	exit (1);
    }
    rem=KSUCCESS;
    authopts = 0L;

    /* We stash hp->h_name as krb_realmofhost may stomp on the
       internal structures pointed by hp*/
    hostname = malloc(strlen(hp->h_name)+1);
    if (hostname == NULL) {
	fprintf(stderr, "from: out of memory");
	exit(1);
    }
    strcpy(hostname, hp->h_name);
    rem = krb_sendauth(authopts, s, ticket, "pop", hostname,
		       (char *) krb_realmofhost(hostname),
		       0, (MSG_DAT *) 0, (CREDENTIALS *) 0,
		       (bit_64 *) 0, (struct sockaddr_in *)0,
		       (struct sockaddr_in *)0,"ZMAIL0.0");
    free(hostname);
    if (rem != KSUCCESS) {
	(void) sprintf(Errmsg, "kerberos error: %s",krb_err_txt[rem]);
	(void) close(s);
	return(NOTOK);
    }
#endif

    sfi = fdopen(s, "r");
    sfo = fdopen(s, "w");
    if (sfi == NULL || sfo == NULL) {
	(void) sprintf(Errmsg, "error in fdopen\n");
	(void) close(s);
	return(NOTOK);
    }

    return(OK);
}

int pop_command(char *fmt, ...)
{
    char buf[4096];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    if (popmail_debug) fprintf(stderr, "---> %s\n", buf);
    if (putline(buf, Errmsg, sfo) == NOTOK) return(NOTOK);

    if (getline(buf, sizeof buf, sfi) != OK) {
	(void) strcpy(Errmsg, buf);
	return(NOTOK);
    }

    if (popmail_debug) fprintf(stderr, "<--- %s\n", buf);
    if (*buf != '+') {
	(void) strcpy(Errmsg, buf);
	return(NOTOK);
    } else {
	return(OK);
    }
}

    
int pop_stat(int *nmsgs, int *nbytes)
{
    char buf[4096];

    if (popmail_debug) fprintf(stderr, "---> STAT\n");
    if (putline("STAT", Errmsg, sfo) == NOTOK) return(NOTOK);

    if (getline(buf, sizeof buf, sfi) != OK) {
	(void) strcpy(Errmsg, buf);
	return(NOTOK);
    }

    if (popmail_debug) fprintf(stderr, "<--- %s\n", buf);
    if (*buf != '+') {
	(void) strcpy(Errmsg, buf);
	return(NOTOK);
    } else {
	(void) sscanf(buf, "+OK %d %d", nmsgs, nbytes);
	return(OK);
    }
}


int putline(char *buf, char *err, FILE *f)
{
    fprintf(f, "%s\r\n", buf);
    (void) fflush(f);
    if (ferror(f)) {
	(void) strcpy(err, "lost connection");
	return(NOTOK);
    }
    return(OK);
}

int getline(char *buf, int n, FILE *f)
{
    register char *p;
    int c;

    p = buf;
    while (--n > 0 && (c = fgetc(f)) != EOF)
      if ((*p++ = c) == '\n') break;

    if (ferror(f)) {
	(void) strcpy(buf, "error on connection");
	return (NOTOK);
    }

    if (c == EOF && p == buf) {
	(void) strcpy(buf, "connection closed by foreign host");
	return (DONE);
    }

    *p = '\0';
    if (*--p == '\n') *p = '\0';
    if (*--p == '\r') *p = '\0';
    return(OK);
}

int multiline(char *buf, int n, FILE *f)
{
    if (getline(buf, n, f) != OK) return (NOTOK);
    if (*buf == '.') {
	if (*(buf+1) == '\0') {
	    return (DONE);
	} else {
	    (void) strcpy(buf, buf+1);
	}
    }
    return(OK);
}
