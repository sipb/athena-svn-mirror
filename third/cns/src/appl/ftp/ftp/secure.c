/*
 * Shared routines for client and server for
 * secure read(), write(), getc(), and putc().
 * Only one security context, thus only work on one fd at a time!
 */

#include "secure.h"	/* stuff which is specific to client or server */

#ifdef KERBEROS
#include <krb.h>

CRED_DECL
extern KTEXT_ST ticket;
extern MSG_DAT msg_data;
extern Key_schedule schedule;
#endif /* KERBEROS */

#include <arpa/ftp.h>

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
extern int errno;
#ifndef HAVE_SYS_ERRLIST_DECL
extern char *sys_errlist[];
#endif

extern int	level;
extern char	*auth_type;

#define MAX maxbuf
extern unsigned int maxbuf; 	/* maximum output buffer size */
extern unsigned char *ucbuf;	/* cleartext buffer */
static unsigned int nout, bufp;	/* number of chars in ucbuf,
				 * pointer into ucbuf */

#ifdef KERBEROS
#define FUDGE_FACTOR 32		/* Amount of growth
				 * from cleartext to ciphertext.
				 * krb_mk_priv adds this # bytes.
				 * Must be defined for each auth type.
				 */
#endif /* KERBEROS */

#ifndef FUDGE_FACTOR		/* In case no auth types define it. */
#define FUDGE_FACTOR 0
#endif

#ifndef KERBEROS
/* XXX - The following must be redefined if KERBEROS_V4 is not used
 * but some other auth type is.  They must have the same properties. */
#define krb_net_write write
#define krb_net_read read
#endif

#if defined(STDARG) || (defined(__STDC__) && ! defined(VARARGS))
extern secure_error(char *, ...);
#else
extern secure_error();
#endif

#define ERR	-2

static
secure_putbyte(fd, c)
int fd;
unsigned char c;
{
	int ret;

	ucbuf[nout++] = c;
	if (nout == MAX - FUDGE_FACTOR)
		if (ret = secure_putbuf(fd, ucbuf, nout))
			return(ret);
		else	nout = 0;
	return(c);
}

/* returns:
 *	 0  on success
 *	-1  on error (errno set)
 *	-2  on security error
 */
secure_flush(fd)
int fd;
{
	int ret;

	if (level == PROT_C)
		return(0);
	if (nout)
		if (ret = secure_putbuf(fd, ucbuf, nout))
			return(ret);
	return(secure_putbuf(fd, "", nout = 0));
}

/* returns:
 *	c>=0  on success
 *	-1    on error
 *	-2    on security error
 */
secure_putc(c, stream)
char c;
FILE *stream;
{
	if (level == PROT_C)
		return(putc(c,stream));
	return(secure_putbyte(fileno(stream), (unsigned char) c));
}

/* returns:
 *	nbyte on success
 *	-1  on error (errno set)
 *	-2  on security error
 */
secure_write(fd, buf, nbyte)
int fd;
unsigned char *buf;
unsigned int nbyte;
{
	unsigned int i;
	int c;

	if (level == PROT_C)
		return(write(fd,buf,nbyte));
	for (i=0; nbyte>0; nbyte--)
		if ((c = secure_putbyte(fd, buf[i++])) < 0)
			return(c);
	return(i);
}

/* returns:
 *	 0  on success
 *	-1  on error (errno set)
 *	-2  on security error
 */
secure_putbuf(fd, buf, nbyte)
int fd;
unsigned char *buf;
unsigned int nbyte;
{
	static char *outbuf;		/* output ciphertext */
	static unsigned int bufsize;	/* size of outbuf */
	extern char *malloc();
	long length;
	u_long net_len;

	if (bufsize < nbyte + FUDGE_FACTOR) {
		if (outbuf) (void) free(outbuf);
		if (outbuf = malloc((unsigned) (nbyte + FUDGE_FACTOR)))
			bufsize = nbyte + FUDGE_FACTOR;
		else {
			bufsize = 0;
			secure_error("%s (in malloc of PROT buffer)",
					sys_errlist[errno]);
			return(ERR);
		}
	}
	/* Other auth types go here ... */
#ifdef KERBEROS
	if (strcmp(auth_type, "KERBEROS_V4") == 0) {
	  struct sockaddr_in myaddr, hisaddr;
	  int len;
	  len = sizeof(myaddr);
	  if (getsockname(fd, (struct sockaddr*)&myaddr, &len) < 0) {
		  secure_error("secure_putbuf: getsockname failed");
		  return(ERR);
	  }
	  len = sizeof(hisaddr);
	  if (getpeername(fd, (struct sockaddr*)&hisaddr, &len) < 0) {
		  secure_error("secure_putbuf: getpeername failed");
		  return(ERR);
	  }
	  if ((length = level == PROT_P ?
	    krb_mk_priv(buf, (unsigned char *) outbuf, nbyte, schedule,
			SESSION, &myaddr, &hisaddr)
	  : krb_mk_safe(buf, (unsigned char *) outbuf, nbyte, SESSION,
			&myaddr, &hisaddr)) == -1) {
		secure_error("krb_mk_%s failed for KERBEROS_V4",
				level == PROT_P ? "priv" : "safe");
		return(ERR);
	  }
	}
#endif /* KERBEROS */
	net_len = htonl((u_long) length);
	if (krb_net_write(fd, &net_len, sizeof(net_len)) == -1) return(-1);
	if (krb_net_write(fd, outbuf, length) != length) return(-1);
	return(0);
}

static
secure_getbyte(fd)
int fd;
{
	/* number of chars in ucbuf, pointer into ucbuf */
	static unsigned int nin, bufp;
	int kerror;
	u_long length;

	if (nin == 0) {
		if ((kerror = krb_net_read(fd, &length, sizeof(length)))
				!= sizeof(length)) {
			secure_error("Couldn't read PROT buffer length: %s",
					kerror == -1 ? sys_errlist[errno]
							: "premature EOF");
			return(ERR);
		}
		if ((length = (u_long) ntohl(length)) > MAX) {
			secure_error("Length of PROT buffer > PBSZ=%u", MAX);
			return(ERR);
		}
		if ((kerror = krb_net_read(fd, ucbuf, length)) != length) {
			secure_error("Couldn't read %u byte PROT buffer: %s",
					length, kerror == -1 ?
					sys_errlist[errno] : "premature EOF");
			return(ERR);
		}
		/* Other auth types go here ... */
#ifdef KERBEROS
		if (strcmp(auth_type, "KERBEROS_V4") == 0) {
		  struct sockaddr_in myaddr, hisaddr;
		  int len;
		  len = sizeof(myaddr);
		  if (getsockname(fd, (struct sockaddr*)&myaddr, &len) < 0) {
			  secure_error("secure_getbyte: getsockname failed");
			  return(ERR);
		  }
		  len = sizeof(hisaddr);
		  if (getpeername(fd, (struct sockaddr*)&hisaddr, &len) < 0) {
			  secure_error("secure_getbyte: getpeername failed");
			  return(ERR);
		  }
		  if (kerror = level == PROT_P ?
		    krb_rd_priv(ucbuf, length, schedule, SESSION,
				&hisaddr, &myaddr, &msg_data)
		  : krb_rd_safe(ucbuf, length, SESSION,
				&hisaddr, &myaddr, &msg_data)) {
			secure_error("krb_rd_%s failed for KERBEROS_V4 (%s)",
					level == PROT_P ? "priv" : "safe",
					krb_get_err_text(kerror));
			return(ERR);
		  }
		  memcpy(ucbuf, msg_data.app_data, msg_data.app_length);
		  nin = bufp = msg_data.app_length;
		}
#endif /* KERBEROS */
		/* Other auth types go here ... */
	}
	if (nin == 0)
		return(EOF);
	else	return(ucbuf[bufp - nin--]);
}

/* returns:
 *	c>=0 on success
 *	-1   on EOF
 *	-2   on security error
 */
secure_getc(stream)
FILE *stream;
{
	if (level == PROT_C)
		return(getc(stream));
	return(secure_getbyte(fileno(stream)));
}

/* returns:
 *	n>0 on success (n == # of bytes read)
 *	0   on EOF
 *	-1  on error (errno set), only for PROT_C
 *	-2  on security error
 */
secure_read(fd, buf, nbyte)
int fd;
char *buf;
int nbyte;
{
	static int c;
	int i;

	if (level == PROT_C)
		return(read(fd,buf,nbyte));
	if (c == EOF)
		return(c = 0);
	for (i=0; nbyte>0; nbyte--)
		switch (c = secure_getbyte(fd)) {
			case ERR: return(c);
			case EOF: if (!i) c = 0;
				  return(i);
			default:  buf[i++] = c;
		}
	return(i);
}
