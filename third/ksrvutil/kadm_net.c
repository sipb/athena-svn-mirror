/*
 * kadm_net.c
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * Kerberos administration server client-side network access routines
 * These routines do actual network traffic, in a machine dependent manner.
 */

#include <mit-copyright.h>

#define	DEFINE_SOCKADDR		/* Ask krb.h for struct sockaddr, etc */
#include "krb.h"

#include <errno.h>
#include <signal.h>
#include <kadm.h>
#include <kadm_err.h> 
#include <krbports.h>

#ifndef NULL
#define NULL 0
#endif

#ifdef _WINDOWS
#define SIGNAL(s, f) 0
#endif

#ifndef _WINDOWS
#define SIGNAL(s, f) signal(s, f)
extern int errno;
#endif


extern Kadm_Client client_parm;
extern int default_client_port;

static sigtype (*opipe)();

int kadm_cli_conn()
{					/* this connects and sets my_addr */
    int on = 1;

    if ((client_parm.admin_fd =
	 socket(client_parm.admin_addr.sin_family, SOCK_STREAM,0)) < 0)
	return KADM_NO_SOCK;		/* couldnt create the socket */
    if (connect(client_parm.admin_fd,
		(struct sockaddr *) & client_parm.admin_addr,
		sizeof(client_parm.admin_addr))) {
	(void) closesocket(client_parm.admin_fd);
	client_parm.admin_fd = -1;

        /* The V4 kadmind port number is 751.  The RFC assigned
	   number, for V5, is 749.  Sometimes the entry in
	   /etc/services on a client machine will say 749, but the
	   server may be listening on port 751.  We try to partially
	   cope by automatically falling back to try port 751 if we
	   don't get a reply on port we are using.  */
        if (client_parm.admin_addr.sin_port != htons(KADM_PORT)
	     && default_client_port) {
	    client_parm.admin_addr.sin_port = htons(KADM_PORT);
	    return kadm_cli_conn();
	}

	return KADM_NO_CONN;		/* couldnt get the connect */
    }
    opipe = SIGNAL(SIGPIPE, SIG_IGN);
    client_parm.my_addr_len = sizeof(client_parm.my_addr);
    if (getsockname(client_parm.admin_fd,
		    (struct sockaddr *) & client_parm.my_addr,
		    &client_parm.my_addr_len) < 0) {
	(void) closesocket(client_parm.admin_fd);
	client_parm.admin_fd = -1;
	(void) SIGNAL(SIGPIPE, opipe);
	return KADM_NO_HERE;		/* couldnt find out who we are */
    }
    if (setsockopt(client_parm.admin_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
		   sizeof(on)) < 0) {
	(void) closesocket(client_parm.admin_fd);
	client_parm.admin_fd = -1;
	(void) SIGNAL(SIGPIPE, opipe);
	return KADM_NO_CONN;		/* XXX */
    }
    return KADM_SUCCESS;
}

void kadm_cli_disconn()
{
    (void) closesocket(client_parm.admin_fd);
    (void) SIGNAL(SIGPIPE, opipe);
    return;
}

int kadm_cli_out(dat, dat_len, ret_dat, ret_siz)
u_char *dat;
int dat_len;
u_char **ret_dat;
int *ret_siz;
{
	u_short dlen;
	int retval;
	extern char *malloc();

	dlen = (u_short) dat_len;

	if (dat_len != (int)dlen)
		return (KADM_NO_ROOM);

	dlen = htons(dlen);
	if (krb_net_write(client_parm.admin_fd, (char *) &dlen,
			  sizeof(u_short)) < 0)
		return (SOCKET_ERRNO);	/* XXX */

	if (krb_net_write(client_parm.admin_fd, (char *) dat, dat_len) < 0)
		return (SOCKET_ERRNO);	/* XXX */

	if (retval = krb_net_read(client_parm.admin_fd, (char *) &dlen,
				  sizeof(u_short)) != sizeof(u_short)) {
	    if (retval < 0)
		return(SOCKET_ERRNO);	/* XXX */
	    else
		return(EPIPE);		/* short read ! */
	}

	dlen = ntohs(dlen);
	*ret_dat = (u_char *)malloc((unsigned)dlen);
	if (!*ret_dat)
	    return(KADM_NOMEM);

	if (retval = krb_net_read(client_parm.admin_fd, (char *) *ret_dat,
				  (int) dlen) != dlen) {
	    if (retval < 0)
		return(SOCKET_ERRNO);	/* XXX */
	    else
		return(EPIPE);		/* short read ! */
	}
	*ret_siz = (int) dlen;
	return KADM_SUCCESS;
}
