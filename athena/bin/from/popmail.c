/* 
 * $Id: popmail.c,v 1.8 1997-11-22 19:23:21 ghudson Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/from/popmail.c,v $
 * $Author: ghudson $
 *
 */

#if !defined(lint) && !defined(SABER)
static char *rcsid = "$Id: popmail.c,v 1.8 1997-11-22 19:23:21 ghudson Exp $";
#endif /* lint || SABER */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#ifdef HAVE_HESIOD
#include <hesiod.h>
#endif
#ifdef HAVE_KRB4
#include <krb.h>
#endif

#define NOTOK (-1)
#define OK 0
#define DONE 1

extern FILE *sfi;
extern FILE *sfo;
extern char Errmsg[80];
#ifdef HAVE_KRB4
char *PrincipalHostname();
#define KPOP_PORT 1109
#endif
extern int popmail_debug;

pop_init(host)
char *host;
{
    register struct hostent *hp;
    register struct servent *sp;
    int lport = IPPORT_RESERVED - 1;
    struct sockaddr_in sin;
    register int s;
#ifdef HAVE_KRB4
    KTEXT ticket = (KTEXT)NULL;
    int rem;
    long authopts;
    char *hostname;
#endif

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

/*VARARGS1*/
pop_command(fmt, a, b, c, d)
char *fmt;
{
    char buf[4096];

    (void) sprintf(buf, fmt, a, b, c, d);

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

    
pop_stat(nmsgs, nbytes)
int *nmsgs, *nbytes;
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


putline(buf, err, f)
char *buf;
char *err;
FILE *f;
{
    fprintf(f, "%s\r\n", buf);
    (void) fflush(f);
    if (ferror(f)) {
	(void) strcpy(err, "lost connection");
	return(NOTOK);
    }
    return(OK);
}

getline(buf, n, f)
char *buf;
register int n;
FILE *f;
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

multiline(buf, n, f)
char *buf;
register int n;
FILE *f;
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
