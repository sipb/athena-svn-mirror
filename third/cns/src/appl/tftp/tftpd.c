/* stftp.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"


/* This is a simple server tftp for the UNIX net system.  It runs
 * with one connection per process, forking as needed to handle
 * multiple connections.  In the fork, the parent services the
 * incoming request, while the child becomes the new listener.
 * This is necessary to avoid lots of zombie children lying around
 * (it's very inconvenient for the listener to wait for its children).
 */
#include	<string.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<netinet/in.h>
#include	"tftp.h"
#include	"conn.h"
#include        <netdb.h>

#ifdef _AUX_SOURCE
#define HAS_SETVBUF
#endif
#ifdef hpux
#define HAS_SETVBUF
#endif
#ifdef __SCO__
#define HAS_SETVBUF
#endif
#ifdef __svr4__
#define HAS_SETVBUF
#endif
/* Global variables */

#define	LOCKMODE	0444		/* lock file mode */
#define	NAMSIZ		1024
#define EBUFSIZ 120
#define MAXPATHLENGTH   500
#define RWX             077
#define EXECUTE         011
char ebuffer[EBUFSIZ];

long	xfer_size;			/* transfer size in bytes */
char	*tftpdir = "/tftpd";		/* tftp directory */
char	*lockfile = "/tftpd/lock";	/* the lock file */
time_t	start;				/* start time */
time_t	finish;				/* finish time */
extern	char	*tempfile();		/* to get temp file name */
extern	char	*calloc();

extern int rd_ap_ret;			/* in conn.c: did he authenticate? */
/*int	debug = 0;*/

main (argc, argv)

int	argc;
char	**argv;
{
	struct	conn	*cn;		/* connection block */
	int	succ;			/* success code */
	int	lockfd;			/* lock file descriptor */
	FILE	*lockptr;		/* lock file pointer */
	int	pid;			/* parent proc. id */
	int     sockname[20];
	int     namelen = 80;
#ifndef HAS_SETVBUF
	setbuffer(stderr, ebuffer, EBUFSIZ);
	setlinebuf(stderr);
#else
	setvbuf(stderr, ebuffer, _IOFBF, EBUFSIZ);
	{
	    static char buf[BUFSIZ];
	    setvbuf(stderr, buf, _IOLBF, BUFSIZ);
	}
#endif
	setbuf(stdout, NULL);
	umask(0);
#ifndef	INETD
	chdir (tftpdir);
	if ((lockfd = creat (lockfile, LOCKMODE)) < 0) {
		cn_inform ("Duplicate daemon exiting\n", 0);
		exit (1);
	}
	
	lockptr = fdopen (lockfd, "w");	/* set up lock file */
	pid = getpid ();
	fprintf (lockptr, "%d", pid);
	fclose (lockptr);
	
	time (&start);
	fprintf(stderr,"\nPid %d %sTFTP Daemon Initialized\n", pid,
	    ctime(&start));
	fflush (stderr);
	
#endif

	init_cmd ();			/* set up command file */
	
	if ((cn = cn_lstn ()) == NULL)
		exit (1);

/*	if(!geteuid()) {
		cn_log("Demon should not run as root\n", TEACESS, 0);
		cn_err(cn, cn->fhost, cn->forsock, TEACESS,
		       "EUID should not be root");
		cn_close(cn);
	} */
	time (&start);

	log_conn (cn, start);
		
	if ((cn->dir == READ) || (cn->dir == AREAD))
		succ = tftp_read (cn); /* their read is our write */
	else
		succ = tftp_write (cn);	/* and vice versa */
	
	time (&finish);
	
	if (succ) {
		print_stats (xfer_size, finish - start);
		exit (0);
	} else
		exit (1);
}


log_conn (cn, now)

/* Print appropriate logging information to the log file.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* the connection */
time_t	now;				/* current time */
{
	time (&now);
	fprintf(stderr,"\nPid %d, %s%s req for %s\nfhost %X fsock %o lsock %o\n",
		getpid (), ctime (&now), (cn->mode == MAIL ? "mail" :
		(((cn->dir == READ) || (cn->dir == AREAD)) ? "read" : "write")),
		cn->file, cn->fhost, cn->forsock, cn->locsock);
	fflush (stderr);
}


tftp_write (cn)

/* User asked for a tftp write operation.  Do it.  Return TRUE on success
 * and FALSE otherwise.
 */

struct	conn	*cn;			/* this connection */
{
	FILE	*locfd;			/* local file descriptor */
	int	imfd;			/* fd for image mode xfrs */
	register caddr_t	pdata;	/* data pointer */
	register struct	tftp	*ptftp;	/* packet being sent */
	short	len;			/* packet length */
	int	more;			/* more data flag */
	int	cr_seen;		/* for netascii */
	register int	ch;		/* next character read */
	struct	stat	stb;		/* for file status */
	char	*tmpnam;		/* temp file name */
	
	if(cn->dir == RRQ)
		;
	else
		;
	if(!safetransfer(cn->file, cn->fhost, cn->dir)) {
		cn_log("Security violation\n", TEACESS, 0);
		cn_err(cn, cn->fhost, cn->forsock, TEACESS,
		       "Subnet or directory security violation");
		cn_close(cn);
		return(FALSE);
	}
	if (stat (cn->file, &stb) >= 0) { /* already exists */
		cn_log ("Local file already exists\n", TEEXIST, 0);
		cn_err (cn, cn->fhost, cn->forsock, TEEXIST,
			"file already exists");
		cn_close (cn);
		return (FALSE);
	}
	
	tmpnam = tempfile (cn->file, cn->mode); /* get temp file name */

	if (cn->mode == NETASCII) {
		if ((locfd = fopen (tmpnam, "w")) == NULL) {
			cn_log ("Can't open local file\n", TEACESS, 0);
			cn_err (cn, cn->fhost, cn->forsock, TEACESS,
				"unable to open file for write");
			cn_close (cn);
			return (FALSE);
		}

		cn_ack (cn);		/* ack first packet */

		cr_seen = FALSE;
		more = TRUE;
		while (more) {
			if ((ptftp = cn_rcv (cn)) == NULL) {
				unlink (tmpnam);
				return (FALSE);
			}
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			len = cn->cur_len - 4;
			more = (len == DATALEN);
			xfer_size += len;
			
			while (len-- > 0) {
				ch = *pdata++;
				if (cr_seen) {
					cr_seen = FALSE;
					if (ch == '\n') {
						if (putc(ch, locfd) == EOF) {
							goto werror;
						}
					} else {
						if (putc('\r', locfd) == EOF) {
							goto werror;
						}
					}
				} else if (ch == '\r') {
					cr_seen = TRUE;
				} else {
					if (putc(ch, locfd) == EOF) {
						goto werror;
					}
				}
			}
		}
		fclose (locfd);
		link (tmpnam, cn->file);
		unlink (tmpnam);
		free (tmpnam);
	} else {
		if ((imfd = creat (tmpnam, 0666)) < 0) {
			cn_log ("Unable to open local file\n", TEACESS, 0);
			cn_err (cn, cn->fhost, cn->forsock, TEACESS,
				"unable to open file for write");
			cn_close (cn);
			return (FALSE);
		}

		cn_ack (cn);		/* ack initial packet */
		more = TRUE;
		while (more) {
			if ((ptftp = cn_rcv (cn)) == NULL) {
				unlink (tmpnam);
				return (FALSE);
			}
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			len = cn->cur_len - 4;
			if (len == 0)
				break;
			more = (len == DATALEN);
			xfer_size += len;
			if (write (imfd, pdata, len) != len) {
werror:
				cn_log ("Write error on local file\n", TEFULL,
				         0);
				cn_err (cn, cn->fhost, cn->forsock, TEFULL,
					"disk write error occurred");
				cn_close (cn);
				unlink(tmpnam);
				return (FALSE);
			}
		}
		close (imfd);
		link (tmpnam, cn->file);
		unlink (tmpnam);
		free (tmpnam);
	}
	cn_rcvf (cn);
	return (TRUE);
}


/* Netascii state defintions for write */

#define	NORM		0		/* normal character */
#define	NEEDLF		1		/* need a linefeed (for <CR><LF>) */
#define	NEEDNUL		2		/* need a null (for <CR><NUL>) */

tftp_read (cn)

/* User asked for a tftp read transaction.  Perform it.
 * Return TRUE if the transfer completes
 * successfully and FALSE otherwise.
 *
 * Arguments:
 */

struct	conn	*cn;			/* the connection */
{
	FILE	*locfd;			/* local file descriptor */
	int	imfd;			/* file desc. for image writes */
	struct	tftp	*ptftp;		/* tftp packet being written */
	register caddr_t	pdata;	/* data pointer */
	int	len;			/* packet length */
	register int	ch;		/* next character */
	int	more;			/* more data to send */
	register int	state;		/* netascii state */

	if(!safetransfer(cn->file, cn->fhost, cn->dir)) {
		cn_log("Security violation\n", TEACESS, 0);
		cn_err(cn, cn->fhost, cn->forsock, TEACESS,
		       "Subnet or directory security violation");
		cn_close(cn);
		return(FALSE);
	}
	
	if (cn->mode == NETASCII) {
		
		 if ((locfd = fopen (cn->file, "r")) == NULL) {
			cn_log ("Can't open local file\n", TEFNF, 0);
			cn_err (cn, cn->fhost, cn->forsock, TEFNF,
				"file not found");
			cn_close (cn);
			return (FALSE);
		}
		
		more = TRUE;
		state = NORM;
		while (more) {
			ptftp = cn_mkwrt (cn);	/* get write packet */
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			len = 0;
			do {
				if (state == NEEDLF) {
					*pdata++ = '\n';
					state = NORM;
				} else if (state == NEEDNUL) {
					*pdata++ = '\0';
					state = NORM;
				} else if ((ch = getc (locfd)) == EOF) {
					if (ferror(locfd))
						goto rerror;
					more = FALSE;
					break;
				} else if (ch == '\n') {
					*pdata++ = '\r';
					state = NEEDLF;
				} else if (ch == '\r') {
					*pdata++ = '\r';
					state = NEEDNUL;
				} else
					*pdata++ = ch;
			} while (++len < DATALEN);
			xfer_size += len;
			if (!cn_wrt (cn, len))
				return (FALSE);
		}
		fclose (locfd);
	} else {			/* image mode */

		if ((imfd = open (cn->file, 0)) < 0) {
			cn_log ("Can't open local file\n", TEFNF, 0);
			cn_err (cn, cn->fhost, cn->forsock, TEFNF,
				"file not found");
			return (FALSE);
		}
		
		more = TRUE;
		while (more) {
			ptftp = cn_mkwrt (cn);
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			if ((len = read (imfd, pdata, DATALEN)) < 0) {
rerror:
				cn_log ("Read error on local file\n", TEFULL,
				         0);
				cn_err (cn, cn->fhost, cn->forsock, TEFULL,
					"disk read error occurred");
				return (FALSE);
			}
			more = (len == DATALEN);
			xfer_size += len;
			if (!cn_wrt (cn, len))
				return (FALSE);
		}
		close (imfd);
	}
	return (cn_wrtf (cn));
}


print_stats (size, elapsed)

long	size;
long	elapsed;
{
	fprintf(stderr,"\nPid %d Transfer successful\n%ld bytes in %ld seconds, %ld baud\n", getpid (), size, elapsed, (size * 8) / elapsed);
}


char *tempfile (file, mode)

/* Create a temporary file name suitable for a write transfer to the specified
 * file name with the specified transfer type.  If the transfer type is not
 * MAIL, the temporary file must be in the same directory as the specified
 * file name.  If the mode is MAIL, the name is a user name, and the
 * temporary file must be in the mail daemon's directory.  In either
 * case the routine returns a pointer to a calloc'ed area containing
 * the temporary file name.
 *
 * Arguments:
 */

char	*file;				/* file name specified */
int	mode;				/* transfer mode specified */
{
	char	*tmp;			/* temp pointer to calloc'd area */
	int	len;			/* name length */
	char	name[NAMSIZ];		/* base name of temp file */
	register char	*p, *q, *s;	/* ptrs. to parse dir spec */
	
	tmpnam (name);			/* use stdio temp name routine */
	
	p = q = file;		/* q = ptr. after last / in name */
	while (*p != 0) {
		if (*p++ == '/')
			q = p;
	}
	len = q - file;		/* len. of dir spec */
	
	len += strlen (name) + 1;	/* get len to alloc */
	tmp = calloc (1, len);		/* alloc storage */
	for (p = file, s = tmp; p < q;)	/* copy in dir spec. */
		*s++ = *p++;
	*s++ = 0;			/* null terminate */
	strcat (tmp, name);		/* concatenate temp name */
	return (tmp);			/* that's it */
}

int
safetransfer(filename, forhost, direction)
register char *filename;
int forhost;
int direction;
{
	char hostname[100];
	struct hostent *he;
#ifdef notdef
/* punt the pathname checking */
	register int i;
	if(direction == WRITE) {
		if(filename[0] != '/') return(0);
		for(i = 0; filename[i] != '\0'; ++i) {
			if(i >= (MAXPATHLENGTH - 1)) return(0);
			if(!strncmp(&filename[i], "/tftp/", sizeof("/tftp/") - 1))
			{
				goto NEXTTEST;
			}
		}
		return(0);

	NEXTTEST:
		for(i = 0; filename[i] != '\0'; ++i) {
			if(!strncmp(&filename[i], "/../", (sizeof("/../") - 1))) {
				return(0);
			}
		}
	}
#endif
	/* authentication means he has access, else check subnet */
	if (rd_ap_ret == KSUCCESS)
	  return(1);
	else {
	    gethostname(hostname, 100);
	    hostname[99] = '\0';
	    he = gethostbyname(hostname);
	    if(*(u_short *)(he->h_addr) == *(u_short *)&forhost) {
		return(1);	/* net and subnet agree */
	    }
	    return(0);		/* not a safe transfer */
	}
}


