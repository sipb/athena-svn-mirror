/* conn.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* This file contains the routines which perform the network transfers
 * for the tftp protocol.
 */

#include	<stdio.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/ioctl.h>
#include	<netinet/in.h>
#include	<pwd.h>
#include	"tftp.h"
#include	"conn.h"

extern	char	*calloc();
extern	int	errno;

struct	conn	*cn_rq (dir, fhost, fhostname, file, mode, c_mode)

/* Open up the connection, make a request packet, and send the
 * packet out on it.  Allocate space for the connection control
 * block and fill it in. Allocate another packet for data and,
 * on writes, another to hold received packets.  Don't wait
 * for connection ack; it will be waited for in cn_rcv or cn_wrt.
 * Return pointer to the connection control block, or NULL on error.
 *
 * Arguments:
 */
int	dir;				/* connection direction */
struct	in_addr	fhost;			/* foreign host */
char	*fhostname;			/* foreign host name */
char	*file;				/* foreign file name */
int	mode;				/* transfer mode */
char	*c_mode;			/* transfer mode string */
{
	caddr_t	ppkt;			/* current packet */
	register struct	tftp	*ptftp;	/* tftp packet header */
	register char	*pdata;		/* packet data */
	int	len;			/* packet length */
	register struct	conn	*cn;	/* connection block */
	static KTEXT_ST		auth_st;/* Authenticator */
	KTEXT			authent	= &auth_st;
	
	if ((cn = (struct conn *)calloc (1, sizeof (struct conn))) == NULL) {
		cn_log ("Unable to alloc conn. block\n", 0, 0);
		return (NULL);
	}
	
	cn->type = USER;		/* user mode transfer */
	cn->synced = FALSE;		/* conn. unsynchronized yet */
	cn->file = strsave (file);	/* alloc space to save filename */
	cn->mode = mode;
	cn->c_mode = strsave (c_mode);	/* and mode string */
	cn->fhost = fhost;
	cn->locsock = udp_sock();
	cn->forsock = ntohs(TFTPSOCK);
	cn->timeout = ICPTIMEOUT;
	
	cn->dir = dir;
	if (dir == AREAD) cn->dir = READ;
	if (dir == AWRITE) cn->dir = WRITE;


	if ((cn->netfd = udp_open (0L, 0, cn->locsock)) < 0) {
		cn_log ("Unable to open connection\n", 0, 0);
		free (cn);
		return (NULL);
	}
		
	if ((cn->last_sent = udp_alloc (INETLEN, 0)) == NULL) {
		cn_log ("Couldn't alloc packet\n", 0, 0);
		cn_close (cn);
		return (NULL);
	}
	ptftp = (struct tftp *)(cn->last_sent);
	
	pdata = (char *)&ptftp->fp_data.f_rfc;	/* build rfc */
	ptftp->fp_opcode = dir;
	strcpy (pdata, file);
	len = strlen (pdata) + 1;
	pdata += len;
	strcpy (pdata, c_mode);
	len += strlen (pdata) + 1;
	pdata += strlen(pdata) + 1;

	if(dir == AREAD || dir == AWRITE) {
	    int rem;
	    char krb_realm[REALM_SZ];
	    rem = krb_get_lrealm(krb_realm,1);
	    if (rem == KSUCCESS) {
		krb_mk_req(authent,"rcmd",fhostname,krb_realm,0);
		*(pdata++) = (char) authent->length;
		memcpy(pdata, authent->dat, authent->length);
		len += authent->length + 1;
	    }
	}

	len += sizeof (ptftp->fp_opcode);
		
#ifndef	BIGINDIAN
	cn_swab (ptftp, OUTPKT);
#endif
	cn_send (cn, cn->last_sent, len);	/* send it out */
	
	cn->block_num =  ((dir == WRITE || dir == AWRITE) ? 0 : 1); /* write bno is smaller */
	cn->retrans = 0; 
	
	if ((cn->cur_pkt = udp_alloc (INETLEN, 0)) == NULL) {
		cn_log ("Couldn't alloc packet\n", 0, 0);
		cn_close (cn);
		return (FALSE);
	}
	if (dir == READ)
		cn->last_rcv = cn->cur_pkt; /* hack optimization */
	else if ((cn->last_rcv = udp_alloc (INETLEN, 0)) == NULL) {
		cn_log ("Couldn't alloc packet\n", 0, 0);
		cn_close (cn);
		return (FALSE);
	}
	return (cn);
}


#define	SRVR_TIME	60		/* sleep time per read */

struct	conn	*cn_lstn ()

/* Listen for a connection request on the TFTP ICP socket.  When a
 * valid request is received, fork a child process.  The child
 * forks again to perform the actual transfer, and the parent,
 * after waiting for the child to exit, goes back to listening.
 * The grandchile closes the ICP net connection, allocates a local port, and
 * opens a new net connection on that port.  It then fills in a
 * connection block (allocated in the parent) and returns it to
 * the caller to do the transfer.  Note that it does not send the
 * response (ack or data) to the initial connection request; this
 * must be done by the caller.  In the parent this routine never
 * returns.
 */
{
	register struct	conn	*cn;	/* connection control block */
	register int	pid;		/* process id */
	struct	sockaddr_in	fhost;	/* foreign host requesting conn */
	int	len;			/* packet length */
	extern	char	cmd_intrpt;	/* command interrupt flag */
	
	if ((cn = (struct conn *)calloc (1, sizeof (struct conn))) == NULL) {
		cn_log ("Unable to alloc conn block\n", 0, 0);
		return (NULL);
	}
	
/* Fill in conn block as much as possible now */
	
	cn->type = SERVER;		/* server mode transfer */
	cn->synced = FALSE;
	cn->timeout = TIMEOUT;
	cn->locsock = ntohs(TFTPSOCK);
	
#ifndef	INETD

/* Now open the server connection, if possible */
	
	if ((cn->netfd = udp_open (0L, 0, cn->locsock)) < 0) {
		cn_log ("Server port in use!\n", 0, 0);
		free (cn);
		return (NULL);
	}
	
#else
	cn->netfd = 0;			/* if inetd, arrives on stdin */

#endif /* INETD */

	if ((cn->last_rcv = udp_alloc (INETLEN, 0)) == NULL ||
	    (cn->last_sent = udp_alloc (INETLEN, 0)) == NULL ||
	    (cn->cur_pkt = udp_alloc (INETLEN, 0)) == NULL) {
		cn_log ("Couldn't alloc packet\n", 0, 0);
		cn_close (cn);
		return (NULL);
	}
	
/* Now the big loop.  Parent waits for ICP request, forks, and waits */
	
	for (;;) {			/* parent server loops forever */
		while ((len = udp_breadfrom (cn->netfd, cn->last_rcv, INETLEN,
			&fhost, SRVR_TIME)) == 0) { /* loop 'til pkt arrives */
			if (cmd_intrpt) { /* command waiting? */
				cmd_intrpt = 0;
				do_cmd (cn);
			}
		}
				
#ifndef	INETD

/* Got a packet.  Fork; child forks again and exits.  Parent waits for child */

		if ((pid = fork ()) < 0) { /* fork error; punt */
			cn_log ("Unable to fork server\n", 0, 0);
			continue;	/* back to loop */
		} else if (pid > 0) {	/* parent; turn net intrs on... */
			while (wait (0) != pid) ; /* wait for child to exit */
			continue;	/* back to loop */
		}

/* Child; fork again and exit.  This makes grandchild an orphan */
		
		if ((pid = fork ()) < 0) { /* fork error - punt */
			cn_log ("Unable to fork server\n", 0, 0);
			exit (1);
		} else if (pid > 0) { /* child; exit back to parent */
			exit (0);
		}

#else

		close(1);

/* Fork again and exit.  This makes grandchild an orphan */
		
		if ((pid = fork ()) < 0) { /* fork error - punt */
			cn_log ("Unable to fork server\n", 0, 0);
			exit (1);
		} else if (pid > 0) { /* child; exit back to parent */
			exit (0);
		}

#endif /* INETD */

/* Grandchild; finally do the transfer */
		
		udp_close (cn->netfd);
		cn->locsock = udp_sock (); /* get a unique socket */
		if ((cn->netfd = udp_open (0L, 0, cn->locsock)) < 0) {
			cn_log ("Unable to open connection\n", 0, 0);
			return (NULL);	/* punt... */
		}
		
		if (!cn_parse (cn, &fhost, cn->last_rcv)) /* good request? */
			return (NULL);	/* no, report lossage */
		else
			return (cn);	/* yes, give conn. block to user */
	}
}


struct	tftp	*cn_rcv (cn)

/* Receive a tftp packet into the packet buffer pointed to by cn->cur_pkt.
 * The packet to be received must be a packet of block number cn->block_num.
 * Returns a pointer to the tftp part of received packet.  Also performs
 * ack sending and retransmission.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection block */
{
	register struct	tftp	*ptftp;	/* tftp header */
	
	if ((ptftp = cn_wait (cn, DATA)) == NULL)
		return (NULL);
		
	cn->cur_pkt = cn->last_rcv; /* hack optimization */
	cn->cur_len = cn->rcv_len;
	
	cn_ack (cn);
	return (ptftp);
}


cn_wrt (cn, len)

/* Write the data packet contained in cn->cur_pkt, with data length len,
 * to the net.  Wait first for an ack for the previous packet to arrive,
 * retransmitting it as needed.  Then fill in the net headers, etc. and
 * send the packet out.  Return TRUE if the packet is sent successfully,
 * or FALSE if a timeout or error occurs.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection */
int	len;				/* data length of packet */
{
	register struct	tftp	*ptftp;	/* tftp header */
	caddr_t	temp;			/* temp for swap */
	
	ptftp = (struct tftp *)(cn->cur_pkt);
	ptftp->fp_opcode = DATA;
	ptftp->fp_data.f_data.f_blkno = cn->block_num + 1;
#ifndef	BIGINDIAN
	cn_swab (ptftp, OUTPKT);
#endif
	len += 4;
	
	if (cn->block_num != 0 || cn->type != SERVER) {
		if (cn_wait (cn, DACK) == NULL)
			return (FALSE);
	}
		
	cn->block_num++;		/* next expected block number */
	cn->retrans = 0;
	
	temp = cn->last_sent;		/* next write packet buffer */
	cn_send (cn, cn->cur_pkt, len);	/* sets up last_sent... */
	cn->cur_pkt = temp;		/* for next mkwrt */
	return (TRUE);
}


struct tftp *cn_wait (cn, opcode)

/* Wait for a valid tftp packet of the specified type to arrive on the
 * specified tftp connection, retransmitting the previous packet as needed up
 * to the timeout period.  When a packet comes in, check it out.
 * Return a pointer to the received packet or NULL if error or timeout.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection */
short	opcode;				/* expected pkt type (DATA or DACK) */
{
	time_t	now;			/* current time */
	int	tmo;			/* timeout time */
	struct	sockaddr_in	fhost;	/* foreign host */
	register struct	tftp	*ptftp;	/* tftp header */
	int	len;			/* packet length */
	

	for (;;) {
		time (&now);
		tmo = cn->nxt_retrans - now;
		if ((len = udp_breadfrom(cn->netfd, cn->last_rcv, INETLEN,
		    &fhost, (int)tmo)) == 0) {
			if (!cn_retrans (cn, TMO)) /* timeout */
				break;
			continue;
		}

/* Got a packet; check it out */
		
		ptftp = (struct tftp *)(cn->last_rcv);
		
		if (cn->intrace)
			logpkt (cn->last_rcv, len, fhost.sin_addr,
			    fhost.sin_port, INPKT);
		
#ifndef	BIGINDIAN
		cn_swab (ptftp, INPKT);
#endif

/* First, check the received length for validity */

		cn->rcv_len = len;
		if (cn->rcv_len < 2) {
			cn_log ("Received bad packet length %d\n",
				TEUNDEF, cn->rcv_len);
			cn_err (cn, fhost.sin_addr, fhost.sin_port, TEUNDEF,
				"bad tftp packet length");
			return (NULL);
		}

/* Next, check for correct foreign host */

		if (fhost.sin_addr.s_addr != cn->fhost.s_addr) {
			cn_inform("Received packet from bad foreign host %x\n",
			    fhost.sin_addr.s_addr);
			cn_err(cn, fhost.sin_addr, fhost.sin_port, TETID,
			    "Sorry, wasn't talking to you!");
			continue;
		}

/* Next, the foreign socket.  If still unsynchronized, use his socket */
		
		if (!(cn->synced) && ((ptftp->fp_opcode == opcode &&
		    ptftp->fp_data.f_data.f_blkno == cn->block_num) ||
		    (ptftp->fp_opcode == ERROR))) {
			cn->synced = TRUE;
			cn->forsock = fhost.sin_port;
			cn->timeout = TIMEOUT; /* normal data timeout */
		} else if (fhost.sin_port != cn->forsock) { /* bad port */
			cn_inform ("Received packet on bad foreign port %o\n",
			         fhost.sin_port);
			cn_err (cn, fhost.sin_addr, fhost.sin_port,
				TETID, "unexpected socket number");
			continue;
		}
		
/* Now check out the tftp opcode */
		
		if (ptftp->fp_opcode == opcode) {
			if (ptftp->fp_data.f_data.f_blkno == cn->block_num) {
				return (ptftp);
			} else if (ptftp->fp_data.f_data.f_blkno ==
				   (cn->block_num - 1) && opcode == DATA) {
				if (!cn_retrans (cn, DUP)) /* timeout */
					break;
			} else if (ptftp->fp_data.f_data.f_blkno >
			           cn->block_num) {
				cn_log ("Received packet with unexpected block no. %d\n",
				        TETFTP, ptftp->fp_data.f_data.f_blkno);
				cn_err (cn, cn->fhost, cn->forsock, TETFTP,
					"block num > expected");
				cn_close (cn);
				return (NULL);
			} else		/* old duplicate; ignore */
				continue;

		} else if (ptftp->fp_opcode == ERROR) {
			cn_log ("Error packet received: %s\n",
			       ptftp->fp_data.f_error.f_errcode,
			       ptftp->fp_data.f_error.f_errmsg);
			cn_close (cn);
			return (NULL);
		} else {		/* unexpected TFTP opcode */
			cn_log ("Bad opcode %d received\n", TETFTP, ptftp->fp_opcode);
			cn_err (cn, cn->fhost, cn->forsock, TETFTP,
				"bad opcode received");
			cn_close (cn);
			return (NULL);
		}
	}
	cn_log ("Long timeout occurred\n", TEUNDEF, 0);
	cn_err (cn, cn->fhost, cn->forsock, TEUNDEF, "timeout on receive");
	cn_close (cn);
	return (NULL);
}


cn_ack(cn)

/* Generate and send an ack packet for the specified connection.  Also
 * update the block number.  Use the packet stored in cn->last_sent to build
 * the ack in.
 */
register struct	conn	*cn;		/* the connection being acked */
{
	register struct	tftp	*ptftp;
	register int		len;

	ptftp = (struct tftp *)(cn->last_sent);
	
	len = 4;
	ptftp->fp_opcode = DACK;
	ptftp->fp_data.f_data.f_blkno = cn->block_num;

#ifndef	BIGINDIAN
	cn_swab (ptftp, OUTPKT);
#endif
	cn_send(cn, cn->last_sent, len);
	cn->retrans = 0;
	cn->block_num++;
}


cn_err (cn, fhost, fsock, ecode, emsg)

/* Make an error packet to send to the specified foreign host and socket
 * with the specified error code and error message.  This routine is
 * used to send error messages in response to packets received from
 * unexpected foreign hosts or tid's as well as those received for the
 * current connection.  It allocates a packet specially
 * for the error message because such error messages will not be
 * retransmitted.  Send it out on the connection.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection */
struct	in_addr	fhost;			/* foreign host */
int	fsock;				/* foreign socket */
int	ecode;				/* error code */
char	*emsg;				/* char. string error message */
{
	caddr_t	ppkt;			/* packet */
	register struct tftp	*ptftp;	/* tftp packet */
	register int		len;

	if ((ppkt = udp_alloc (DATALEN, 0)) == NULL) /* punt */
		return;
	ptftp = (struct tftp *)ppkt;
	
	len = 4;
	ptftp->fp_opcode = ERROR;
	ptftp->fp_data.f_error.f_errcode = ecode;
	strcpy(ptftp->fp_data.f_error.f_errmsg, emsg);
	len += (strlen(emsg) + 1);

#ifndef	BIGINDIAN
	cn_swab (ptftp, OUTPKT);
#endif
	
	if (udp_write (cn->netfd, ppkt, len, fhost, fsock) <= 0)
		cn_inform ("Net write error, errno = %d\n", errno);
	else if (cn->outtrace)
		logpkt (ppkt, len, fhost, fsock, OUTPKT);
	udp_free (ppkt);	
}


cn_send (cn, ppkt, len)

/* Send the specified packet, with the specified tftp length (length -
 * udp and ip headers) out on the current connection.  Fill in the
 * needed parts of the udp and ip headers, byte-swap the tftp packet,
 * etc; then write it out.  Then set up for retransmit.
 *
 * Arguments:
 */
register struct	conn	*cn;		/* connection */
caddr_t	ppkt;				/* pointer to packet to send */
int	len;				/* tftp length of packet */
{
	time_t	now;			/* current time */
	
	if (udp_write (cn->netfd, ppkt, len, cn->fhost, cn->forsock) <= 0)
		cn_inform ("Net write error, errno = %d\n", errno);
	else if (cn->outtrace)
		logpkt (ppkt, len, cn->fhost, cn->forsock, OUTPKT);
	
	cn->last_sent = ppkt;
	cn->last_len = len;
	time (&now);
	cn->nxt_retrans = now + cn->timeout;
}
	

struct tftp *cn_mkwrt (cn)

/* Return a pointer to the next tftp packet suitable for filling for
 * writes on the connection.
 *
 * Arguments:
 */

register struct	conn	*cn;
{
	return ((struct tftp *)(cn->cur_pkt));
}


cn_rcvf (cn)

/* Finish off a receive connection.  Just close the connection,
 * return.
 *
 * Arguments:
 */

struct	conn	*cn;			/* connection */
{
	cn_close (cn);
}


cn_wrtf (cn)

/* Finish off a write connection.  Wait for the last ack, then
 * close the connection and return.
 *
 * Arguments:
 */

struct	conn	*cn;			/* connection */
{
	register struct	tftp	*ptftp;	/* received packet */
	
	if ((ptftp = cn_wait (cn, DACK)) == NULL)
		return (FALSE);;

	cn_close (cn);
	return (TRUE);
}


cn_retrans (cn, dup)

/* Retransmit the last-sent packet, up to MAX_RETRANS times.  Exponentially
 * back off the timeout time up to a maximum of MAX_TIMEOUT.  This algorithm
 * may be replaced by a better one in which the timeout time is set from
 * the maximum round-trip time to date.
 * The second argument indicates whether the retransmission is due to the
 * arrival of a duplicate packet or a timeout.  If a duplicate, don't include
 * this retransmission in the maximum retransmission count.
 */

register struct	conn	*cn;		/* connection */
int	dup;				/* retransmit due to duplicate? */
{
	if ((dup != DUP) && (++cn->retrans >= MAX_RETRANS))
		return (FALSE);
	cn->timeout <<= 1;
	if (cn->timeout > MAX_TIMEOUT) cn->timeout = MAX_TIMEOUT;
	cn_send (cn, cn->last_sent, cn->last_len);
	return (TRUE);
}


cn_close (cn)

/* Close the specified connection.  Close the net connection, deallocate
 * all the packets and the connection control block, etc.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection */
{
	udp_close (cn->netfd);
	if (cn->cur_pkt != NULL)
		udp_free (cn->cur_pkt);
	if (cn->last_sent != NULL)
		udp_free (cn->last_sent);
	if (cn->last_rcv != NULL && cn->last_rcv != cn->cur_pkt)
		udp_free (cn->last_rcv);
	if (cn->file != NULL)
		free (cn->file);
	if (cn->c_mode != NULL)
		free (cn->c_mode);
	free (cn);
}

int	rd_ap_ret;			/* did read ap req succeed? used
					 by safetransfer() in tftpd.c */

cn_parse (cn, fh, pkt, len)

/* Parse the request packet pkt, to determine the request type, local
 * file name, and transfer mode, plus foreign host info.  If the
 * request packet is invalid respond with an appropriate error.  Set
 * up the connection block according to the request packet.  Return
 * TRUE if the request is valid, and FALSE otherwise.
 *
 * Arguments:
 */

register struct	conn	*cn;		/* connection block */
struct	sockaddr_in	*fh;		/* foreign sockaddr_in */
caddr_t	pkt;				/* received packet */
int	len;				/* received length */
{
	struct passwd	*pwd;
	register struct	tftp	*ptftp;	/* tftp header */
	char	*pdata;			/* tftp data pointer */
	static KTEXT_ST	auth_st;
	KTEXT		authent = &auth_st;
	AUTH_DAT 	ad;
	char	lname[ANAME_SZ];
	char	instance[INST_SZ];
	char	key[KKEY_SZ];

	ptftp = (struct tftp *)pkt;
	
	if (cn->intrace)
		logpkt (pkt, len, fh->sin_addr, fh->sin_port, INPKT);
	
#ifndef	BIGINDIAN
	cn_swab (ptftp, INPKT);
#endif
	krb_log ("Opcode: %d",ptftp->fp_opcode);
	if (ptftp->fp_opcode != RRQ && ptftp->fp_opcode != WRQ && ptftp->fp_opcode != ARRQ && ptftp->fp_opcode != AWRQ) {
		krb_log ("Aborting...");
		cn_log ("Bad ICP opcode %d received\n", TETFTP, ptftp->fp_opcode);
		cn_err (cn, fh->sin_addr, fh->sin_port, TETFTP,
			"bad opcode received");
		cn_close (cn);
		return (FALSE);
	}
	
	cn->fhost = fh->sin_addr;	/* set up conn block */
	cn->forsock = fh->sin_port;
	cn->dir = ptftp->fp_opcode;
	cn->synced = TRUE;
	cn->block_num = 0;		/* write blkno will be incremented */
	
	pdata = (char *)&ptftp->fp_data.f_rfc;	/* now parse up req. pkt */
	cn->file = strsave (pdata);	/* save off filename */
	pdata += strlen (pdata) + 1;
	cn->c_mode = strsave (pdata);	/* and mode */
	
	if (lwccmp (cn->c_mode, "netascii") == 0)
		cn->mode = NETASCII;
	else if (lwccmp (cn->c_mode, "image") == 0)
		cn->mode = IMAGE;
	else if (lwccmp (cn->c_mode, "octet") == 0) /* gotta support both */
		cn->mode = IMAGE;
	else {
		krb_log("Bad transfer mode: %s",cn->mode);
		cn_log("Bad transfer mode %s specified\n", TETFTP, cn->c_mode);
		cn_err (cn, fh->sin_addr, fh->sin_port, TETFTP, 
			"bad transfer mode specified");
		cn_close (cn);
		return (FALSE);
	}
	pdata += strlen(pdata) + 1;
	if (cn->dir == AREAD || cn->dir == AWRITE)
            {cn->dir = cn->dir - (AREAD - READ);
	     ptftp->fp_opcode = ((cn->dir == READ) ? RRQ : WRQ);
	     authent->length = (int) *(pdata++);
	     memcpy(authent->dat, pdata, authent->length);
	     strcpy(instance,"*");
	     rd_ap_ret = 1;
	     rd_ap_ret = krb_rd_req(authent,"rcmd",instance,
	     			fh->sin_addr.s_addr,&ad,"");
	     krb_log("Code %d: %suthenticated %s request from %s", 
	           rd_ap_ret, (rd_ap_ret ? "Bad a" : "A"),
		   ((cn->dir == READ) ? "read" : "write"), ad.pname);
	     if (rd_ap_ret || (krb_kntoln(&ad,lname) != KSUCCESS) || 
	     	 ((pwd = getpwnam(lname)) == NULL ))
		{setgid(DEFAULT_GID);
		 setuid(DEFAULT_UID);
	  	 if (rd_ap_ret && (krb_kntoln(&ad,lname) != KSUCCESS) &&
		 	((pwd = getpwnam(lname)) != NULL))
			chdir(pwd->pw_dir);}
	     else
	       {krb_log("Setting uid, gid, groups and home directory to %s",lname);
		setgid(pwd->pw_gid);
#ifndef __SCO__
		/* sco *has* no grouplist */
		initgroups(lname, pwd->pw_gid);
#endif
		setuid(pwd->pw_uid);
		chdir(pwd->pw_dir);}
	     }	
	else
		{setgid(DEFAULT_GID);
		 setuid(DEFAULT_UID);}
	return (TRUE);
}
	

#ifndef HAVE_STRSAVE
char	*strsave (str)

/* Save the string pointed to by str in a safe (allocated) place.
 *
 * Arguments:
 */

register char	*str;
{
	register char	*save;
	
	save = calloc (1, strlen (str) + 1);
	strcpy (save, str);
	return (save);
}
#endif

#define	tolower(c)	((c) >= 'A' && (c) <= 'Z' ? (c) + 'a' - 'A' : (c))


lwccmp (s1, s2)

/* Compare two strings ignoring case considerations.
 * Returns:
 *	<0 if s1 < s2
 *	=0 if s1 = s2
 *	>0 if s1 > s2
 *
 * Arguments:
 */

register char	*s1;			/* first string */
register char	*s2;			/* second string */
{
	while (*s1 == *s2 || (tolower (*s1) == tolower (*s2))) {
		if (*s1++ == '\0')
			return (0);
		else
			s2++;
	}
	return (tolower (*s1) - tolower (*s2));
}

	
#ifndef	BIGINDIAN
cn_swab (ptftp, pktdir)

/* Swap the bytes in integer fields of a tftp packet.  The only such
 * fields are the opcode, the block number in DATA and DACK packets,
 * and the error code field in ERROR packets.
 *
 * Arguments:
 */
register struct	tftp	*ptftp;		/* ptr. to tftp packet */
register int	pktdir;			/* packet direction for byteswap */
{
	register int	opcode;
	
	opcode = ptftp->fp_opcode;
	ptftp->fp_opcode = ntohs (ptftp->fp_opcode);
	if (pktdir == INPKT)
		opcode = ptftp->fp_opcode;
	switch (opcode) {
case DATA:
case DACK:
		ptftp->fp_data.f_data.f_blkno =
			ntohs (ptftp->fp_data.f_data.f_blkno);
		break;
case ERROR:
		ptftp->fp_data.f_error.f_errcode =
			ntohs (ptftp->fp_data.f_error.f_errcode);
		break;
default:
		break;
	}
}
#endif


#define	min(a, b)	((a) < (b) ? (a) : (b))

logpkt (ppkt, len, fhost, fsock, dir)

/* Log the specified packet out to the standard output.
 *
 * Arguments:
 */

register char	*ppkt;			/* ptr. to packet */
int	len;				/* length of data part in bytes */
struct	in_addr	fhost;			/* foreign host */
int	fsock;
int	dir;				/* packet direction:  */
					/* 	INPKT - input */
					/* 	OUTPKT - output */
{
	register int	i;		/* bytes per line counter */
	int	linelen;		/* temp for bytes per line */
	
	fprintf(stderr, "%s packet, fhost %X, fsock %d:\n", (dir == INPKT ?
	"Input" : "Output"), fhost.s_addr, fsock);
	
	while (len > 0) {
		linelen = min(16, len);
		for (i = 0; i < linelen; i++, len--)
			fprintf (stderr, "%3o ", (*ppkt++ & 0377));
		putc('\n', stderr);
	}
}
