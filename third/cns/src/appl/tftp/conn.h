/* conn.h */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */


/* This file contains the definitions for the TFTP connection control
 * block, which contains all the information pertaining to a connection.
 * A conn structure is allocated at connection open time and retained
 * until the connection is closed.  The routines in the file conn.c
 * are sufficient for dealing with connections.
 */
#include <krb.h>


#define	TIMEOUT		2		/* initial retransmit timeout */
#define	ICPTIMEOUT	6		/* initial connection timeout */
#define	MAX_TIMEOUT	30		/* max. retransmit timeout */
#define	MAX_RETRANS	5		/* max. retransmits */
#define	SERVER		0		/* a server connection */
#define	USER		1		/* a user connection */
#define	TMO		0		/* retransmitting due to timeout */
#define	DUP		1		/* retransmitting due to duplicate */
#define LOGFILE		"/etc/tftplog"


/* A connection control block */

struct	conn	{
	int	netfd;			/* network file descriptor */
	int	type;			/* user or server connection */
	int	synced;			/* conn synchronized flag */
	int	block_num;		/* next block number */
	caddr_t	last_sent;		/* previous packet sent */
	int	last_len;		/* size of previous packet */
	time_t	nxt_retrans;		/* when to retransmit */
	int	retrans;		/* number of retransmits */
	int	timeout;		/* retransmit timeout */
	caddr_t	cur_pkt;		/* current packet (send or rcv) */
	int	cur_len;		/* current packet len */
	caddr_t	last_rcv;		/* last received packet */
	int	rcv_len;		/* size of last rcvd. packet */
	char	*file;			/* file name */
	int	dir;			/* direction */
	int	mode;			/* transfer mode */
	char	*c_mode;		/* char. string mode */
	struct	in_addr	fhost;		/* foreign host */
	int	locsock;		/* local socket for connection */
	int	forsock;		/* foreign socket for connection */
	int	intrace;		/* input packet trace flag */
	int	outtrace;		/* output packet trace flag */
	KTEXT	authent;		/* Authenticator for connection */
};

extern	struct	conn	*cn_rq ();
extern	struct	conn	*cn_lstn();
extern	struct	tftp	*cn_rcv ();
extern	struct	tftp	*cn_wait ();
extern	struct	tftp	*cn_mkwrt ();
extern	char	*strsave ();
extern	caddr_t	udp_alloc();
