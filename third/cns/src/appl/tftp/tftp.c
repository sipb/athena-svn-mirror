/* tftp.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* This is a quick-and-dirty simple user tftp program.  It attempts
 * to use the same syntax as the usual daemon-based one, but to
 * run faster.  It allows only one connection at a time and performs
 * minimal error checking, etc.  There are now a few extensions to
 * syntax: -o means to overwrite an existing file on a read; and
 * "-" may be used as the local file name to mean the standard
 * {input|output}.
 * This version is intended for use by humans, not other programs;
 * the difference is that it will prompt the user if he attempts to
 * overwrite an existing file on a read transfer.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<netinet/in.h>
#include	"tftp.h"
#include	"conn.h"

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

#define EBUFSIZ 120
char ebuffer[EBUFSIZ];

/* Global variables */

long	xfer_size;			/* transfer size in bytes */
extern	struct	sockaddr_in	*resolve_host();
extern 	char *PrincipalHostname();

FILE	*locfd;			/* local file descriptor */
int	imfd;			/* fd for image mode xfrs */

main (argc, argv)

int	argc;
char	**argv;
{
	time_t	start;			/* start time */
	time_t	finish;			/* finish time */
	struct	conn	*cn;		/* connection block */
	int	succ;			/* success code */
	struct	conn	*mk_conn();	/* make a connection */

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
	
	time (&start);

	if ((cn = mk_conn (argc, argv)) == NULL)
		exit (1);
	
	if (cn->dir == READ || cn->dir == AREAD) {
		succ = tftp_read (cn, argv[2]);
		if (!succ && strcmp(argv[2], "-") != 0)
			unlink (argv[2]);
	} else
		succ = tftp_write (cn, argv[2]);
	
	time (&finish);
	
	if (succ) {
		print_stats (xfer_size, finish - start);
		exit (0);
	} else
		exit (1);
}


static char usage[] = "usage: %s {-g|-ag|-o|-p|-ap} <local file> <host> <foreign file> [netascii|image]\n";

struct	conn	*mk_conn (argc, argv)

int	argc;
char	**argv;
{
	int	dir;			/* connection direction */
	struct	sockaddr_in *fhost;	/* foreign host */
	char	*fhostname;		/* foreign host name */
	char	*c_mode;		/* conn. mode string */
	int	mode;			/* conn. mode */
	struct	stat	stb;		/* file status buffer */
	int	ovrw = FALSE;		/* overwrite existing file? */
	register char	*arg;		/* arg pointer */
	
		
	if (argc < 5 || argc > 6) {
		fprintf (stderr, usage, argv[0]);
		return (NULL);
	}

	if (strcmp (argv[1], "-g") == 0 ||
	    strcmp (argv[1], "-r") == 0 ||
	    strcmp (argv[1], "get") == 0) {
		dir = READ;
	} else if (strcmp (argv[1], "-o") == 0) {
		dir = READ;
		ovrw = TRUE;
	} else if (strcmp (argv[1], "-p") == 0 ||
	    strcmp (argv[1], "-w") == 0 ||
	    strcmp (argv[1], "put") == 0) {
		dir = WRITE;
	} else if (strcmp (argv[1], "-ag") == 0) {
		dir = AREAD;
	} else if (strcmp (argv[1], "-ap") == 0) {
		dir = AWRITE;
	} else {
		fprintf (stderr, usage, argv[0]);
		return (NULL);
	}
	
	if (argc > 5) {
		c_mode = argv[5];
		if (strcmp (argv[5], "netascii") == 0)
			mode = NETASCII;
		else if (strcmp (argv[5], "image") == 0)
			mode = IMAGE;
		else
			mode = IMAGE;	/* using his weird mode */
	} else {
		c_mode = "netascii";
		mode = NETASCII;
	}

	if ((fhost = resolve_host(argv[3])) == NULL) {
		fprintf (stderr, "Don't know host %s\n", argv[3]);
		return (NULL);
	}
	fhostname = PrincipalHostname(argv[3]);

	if ((dir == READ || dir == AREAD) && !ovrw && stat (argv[2], &stb) >= 0 &&
	     strcmp (argv[2], "-") != 0) {
		fprintf (stderr, "File already exists; unlink? ");
		if (getchar () == 'y') {
			if (unlink (argv[2]) < 0) {
				fprintf (stderr, "Can't unlink; use '-o'\n");
				return (NULL);
			}
		} else
			return(NULL);
	}
	if(dir == READ || dir == AREAD) {
		if (mode == NETASCII) {
			if (strcmp (argv[2], "-") == 0) {
				locfd = fdopen(dup (fileno (stdout)), "w");
			}
			else if ((locfd = fopen (argv[2], "w")) == NULL) {
				fprintf (stderr,
					 "Local file error\nCode = 2\n\
Can't open local file\n");
				return (NULL);
			}
		}
	} else if (mode == NETASCII) {
		if (strcmp (argv[2], "-") == 0) {
			locfd = fdopen (dup (fileno (stdin)), "r");
		}
		else if ((locfd = fopen (argv[2], "r")) == NULL) {
			fprintf (stderr,
				 "Local file error\nCode = 1\n\
Can't open local file\n");
			return (NULL);
		}
	}
	return (cn_rq (dir, fhost->sin_addr, fhostname, argv[4], mode, c_mode));
}


tftp_read (cn, loc_file)

struct	conn	*cn;			/* this connection */
char	*loc_file;			/* local file name */
{
	register caddr_t	pdata;	/* data pointer */
	register struct	tftp	*ptftp;	/* packet being sent */
	short	len;			/* packet length */
	int	more;			/* more data flag */
	int	cr_seen;		/* for netascii */
	register int	ch;		/* next character read */
	
	printf("Read transfer started, mode %s\n", cn->c_mode);
	if(cn->mode == NETASCII) {
		cr_seen = FALSE;
		more = TRUE;
		while (more) {
			if ((ptftp = cn_rcv (cn)) == NULL)
				return (FALSE);
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
	} else {
		if (strcmp (loc_file, "-") == 0)
			imfd = dup (fileno (stdout));
		else if ((imfd = creat (loc_file, 0666)) < 0) {
			fprintf (stderr,
			"Local file error\nCode = 2\nCan't open local file\n");
			cn_err (cn, cn->fhost, cn->forsock, TEACESS,
				"unable to open file for write");
			return (FALSE);
		}
		more = TRUE;
		while (more) {
			if ((ptftp = cn_rcv (cn)) == NULL)
				return (FALSE);
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			len = cn->cur_len - 4;
			if (len == 0)
				break;
			more = (len == DATALEN);
			xfer_size += len;
			if (write (imfd, pdata, len) != len) {
werror:
				perror("Write error on local file");
				cn_err (cn, cn->fhost, cn->forsock, TEFULL,
					"disk write error occurred");
				return (FALSE);
			}
		}
		close (imfd);
	}
	cn_rcvf (cn);
	return (TRUE);
}


/* Netascii state defintions for write */

#define	NORM		0		/* normal character */
#define	NEEDLF		1		/* need a linefeed (for <CR><LF>) */
#define	NEEDNUL		2		/* need a null (for <CR><NUL>) */

tftp_write (cn, loc_file)

/* Perform a tftp write transfer from the specified local file on the
 * specified connection.  Return TRUE if the transfer completes
 * successfully and FALSE otherwise.
 *
 * Arguments:
 */

struct	conn	*cn;			/* the connection */
char	*loc_file;			/* local file name */
{
	struct	tftp	*ptftp;		/* tftp packet being written */
	register caddr_t	pdata;	/* data pointer */
	int	len;			/* packet length */
	register int	ch;		/* next character */
	int	more;			/* more data to send */
	register int	state;		/* netascii state */
	
	printf("Write transfer started, mode %s\n", cn->c_mode);
	if (cn->mode == NETASCII) {
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

		if (strcmp (loc_file, "-") == 0)
			imfd = dup (fileno (stdin));
		else if ((imfd = open (loc_file, 0)) < 0) {
			fprintf (stderr,
			"Local file error\nCode = 1\nCan't open local file\n");
			cn_err (cn, cn->fhost, cn->forsock, TEACESS,
				"unable to open file for read");
			return (FALSE);
		}
		
		more = TRUE;
		while (more) {
			ptftp = cn_mkwrt (cn);
			pdata = &ptftp->fp_data.f_data.f_blk[0];
			if ((len = read (imfd, pdata, DATALEN)) < 0) {
rerror:
				perror("Read error on local file");
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
	printf ("Transfer successful\n");
	printf ("%ld bytes in %ld seconds, %ld baud\n", size, elapsed,
		(elapsed > 0 ? ((size * 8) / elapsed) : 0) );
}
