/* 
 * $Id: popmail.c,v 1.1 1991-05-29 10:04:37 akajerry Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/from/popmail.c,v $
 * $Author: akajerry $
 *
 */

#if !defined(lint) && !defined(SABER) && defined(RCS_HDRS)
static char *rcsid = "$Id: popmail.c,v 1.1 1991-05-29 10:04:37 akajerry Exp $";
#endif /* lint || SABER || RCS_HDRS */

#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#ifdef HESIOD
#include <hesiod.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#ifdef KPOP
#include <krb.h>
#endif

#define NOTOK (-1)
#define OK 0
#define DONE 1

FILE *sfi;
FILE *sfo;
char Errmsg[80];
#ifdef KPOP
char *PrincipalHostname(), *index();
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
#ifdef KPOP
    KTEXT ticket = (KTEXT)NULL;
    int rem;
    long authopts;
#endif KPOP
    char *get_errmsg();

    hp = gethostbyname(host);
    if (hp == NULL) {
	(void) sprintf(Errmsg, "MAILHOST unknown: %s", host);
	return(NOTOK);
    }

#ifdef KPOP
#ifdef ATHENA_COMPAT
    sp = getservbyname("knetd", "tcp");
#else
    sp = getservbyname("kpop", "tcp");
#endif
    if (sp == 0) {
#ifdef ATHENA_COMPAT
	(void) strcpy(Errmsg, "tcp/knetd: unknown service");
#else
	(void) strcpy(Errmsg, "tcp/kpop: unknown service");
#endif
	return(NOTOK);
    }
#else !KPOP
    sp = getservbyname("pop", "tcp");
    if (sp == 0) {
	(void) strcpy(Errmsg, "tcp/pop: unknown service");
	return(NOTOK);
    }
#endif KPOP

    sin.sin_family = hp->h_addrtype;
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = sp->s_port;
#ifdef KPOP
    s = socket(AF_INET, SOCK_STREAM, 0);
#else !KPOP
    s = rresvport(&lport);
#endif KPOP
    if (s < 0) {
	(void) sprintf(Errmsg, "error creating socket: %s", get_errmsg());
	return(NOTOK);
    }

    if (connect(s, (struct sockaddr *)&sin, sizeof sin) < 0) {
	(void) sprintf(Errmsg, "error during connect: %s", get_errmsg());
	(void) close(s);
	return(NOTOK);
    }
#ifdef KPOP
    ticket = (KTEXT)malloc( sizeof(KTEXT_ST) );
    rem=KSUCCESS;
#ifdef ATHENA_COMPAT
    authopts = KOPT_DO_OLDSTYLE;
    rem = krb_sendsvc(s,"pop");
    if (rem != KSUCCESS) {
	(void) sprintf(Errmsg, "kerberos error: %s", krb_err_txt[rem]);
	(void) close(s);
	return(NOTOK);
    }
#else
    authopts = 0L;
#endif
    rem = krb_sendauth(authopts, s, ticket, "pop", hp->h_name, (char *)0,
		       0, (MSG_DAT *) 0, (CREDENTIALS *) 0,
		       (bit_64 *) 0, (struct sockaddr_in *)0,
		       (struct sockaddr_in *)0,"ZMAIL0.0");
    if (rem != KSUCCESS) {
	(void) sprintf(Errmsg, "kerberos error: %s",krb_err_txt[rem]);
	(void) close(s);
	return(NOTOK);
    }
#endif KPOP

    sfi = fdopen(s, "r");
    sfo = fdopen(s, "w");
    if (sfi == NULL || sfo == NULL) {
	(void) sprintf(Errmsg, "error in fdopen: %s", get_errmsg());
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

    *p = NULL;
    if (*--p == '\n') *p = NULL;
    if (*--p == '\r') *p = NULL;
    return(OK);
}

multiline(buf, n, f)
char *buf;
register int n;
FILE *f;
{
    if (getline(buf, n, f) != OK) return (NOTOK);
    if (*buf == '.') {
	if (*(buf+1) == NULL) {
	    return (DONE);
	} else {
	    (void) strcpy(buf, buf+1);
	}
    }
    return(OK);
}

char *get_errmsg()
{
    extern int errno, sys_nerr;
    extern char *sys_errlist[];
    char *s;

    if (errno < sys_nerr)
      s = sys_errlist[errno];
    else
      s = "unknown error";
    return(s);
}

