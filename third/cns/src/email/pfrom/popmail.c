#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#ifdef HESIOD
#include <hesiod.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#ifdef KPOP
#include <krb.h>
#include <krbports.h>
#endif

extern char *malloc ();

#define NOTOK (-1)
#define OK 0
#define DONE 1

/* Defined in pfrom.c.  */
extern FILE *sfi;
extern FILE *sfo;
extern char Errmsg[80];

#ifdef KPOP
char *PrincipalHostname();
#endif
extern int popmail_debug;

pop_init(host)
char *host;
{
    register struct hostent *hp;
    register struct servent *sp;
    char *host_save;
    int lport = IPPORT_RESERVED - 1;
    struct sockaddr_in sin;
    register int s;
#ifdef KPOP
    KTEXT ticket = (KTEXT)NULL;
    int rem;
    long authopts;
#endif
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
      sin.sin_port = htons(KNETD_PORT); /* knetd/tcp */
#else
      sin.sin_port = htons(KPOP_PORT); /* kpop/tcp */
#endif
    }
#else /* !KPOP */
    sp = getservbyname("pop", "tcp");
    if (sp == 0) {
      sin.sin_port = htons(POP3_PORT); /* pop/tcp -- POP3 (POP2 is 109) */
    }
#endif /* KPOP */
    else
      sin.sin_port = sp->s_port;

    sin.sin_family = hp->h_addrtype;
    memcpy((char *)&sin.sin_addr, hp->h_addr, hp->h_length);
#ifdef KPOP
    s = socket(AF_INET, SOCK_STREAM, 0);
#else /* !KPOP */
    s = rresvport(&lport);
#endif /* KPOP */
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
	(void) sprintf(Errmsg, "kerberos error: %s", krb_get_err_text(rem));
	(void) close(s);
	return(NOTOK);
    }
#else
    authopts = 0L;
#endif
    host_save = malloc(strlen(hp->h_name)+1);
    strcpy(host_save, hp->h_name);
    rem = krb_sendauth(authopts, s, ticket, "pop", host_save,
		       (char *) krb_realmofhost(host_save),
		       0, (MSG_DAT *) 0, (CREDENTIALS *) 0,
		       (bit_64 *) 0, (struct sockaddr_in *)0,
		       (struct sockaddr_in *)0,"ZMAIL0.0");
    free(host_save);
    if (rem != KSUCCESS) {
	(void) sprintf(Errmsg, "kerberos error: %s",krb_get_err_text(rem));
	(void) close(s);
	return(NOTOK);
    }
#endif

    sfi = fdopen(s, "r");
    sfo = fdopen(s, "w");
#ifdef __SCO__
    setbuf(sfo,malloc(BUFSIZ));
#endif
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
     unsigned long a, b, c, d;
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

    *p = 0;
    if (*--p == '\n') *p = 0;
    if (*--p == '\r') *p = 0;
    return(OK);
}

multiline(buf, n, f)
char *buf;
register int n;
FILE *f;
{
    if (getline(buf, n, f) != OK) return (NOTOK);
    if (*buf == '.') {
	if (*(buf+1) == 0) {
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
#ifndef HAVE_SYS_ERRLIST_DECL
    extern char *sys_errlist[];
#endif
    char *s;

    if (errno < sys_nerr)
      s = sys_errlist[errno];
    else
      s = "unknown error";
    return(s);
}

