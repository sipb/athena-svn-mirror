/*
 *	appl/tftp/udp.c
 */

/* udp.c */

/*  Copyright 1984 by the Massachusetts Institute of Technology  */
/*  See permission and disclaimer notice in file "notice.h"  */
#include	"notice.h"

/* This file contains the routines to open, close, and use udp connections.
 */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/ioctl.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#ifndef NO_SYSIO
#ifdef __svr4__
/* get FIONBIO from sys/filio.h, so what if it is a compatibility feature */
#include <sys/filio.h>
#endif
#endif

extern	char	*calloc();


udp_sock()

/* Nominally returns a unique udp socket number.  We return zero since
 * this will cause the kernel to allocate one for us.
 */

{
	return(0);
}


udp_open(fhost, fport, lport)

/* Open a udp socket to the specified foreign host/foreign port/local port.
 */

struct	in_addr	fhost;
int	fport;
int	lport;
{
	struct	sockaddr_in	lsock;		/* local sockaddr */
	int	onoff = 1;			/* arg for ioctl FIONBIO */
	int	s;				/* new socket */

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		return(s);

	if (ioctl(s, (int)FIONBIO, (char *)&onoff) < 0)
		return(-1);

	lsock.sin_family = AF_INET;
	lsock.sin_port = lport;
	lsock.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (struct sockaddr *)&lsock, sizeof(lsock)) < 0)
		return(-1);

	return(s);
}


caddr_t	udp_alloc(size, slop)

int	size, slop;
{
	return(calloc(size + slop, 1));
}


udp_breadfrom(s, buf, len, fhost, tmo)

int	s;
caddr_t	buf;
int	len;
struct	sockaddr_in	*fhost;
int	tmo;
{
	int	rfds = (1 << s);
	int	fhlen = sizeof(*fhost);
	struct	timeval	timeout;
	
	timeout.tv_sec = tmo;
	timeout.tv_usec = 0;

	if (select(32, &rfds, (int *)0, (int *)0, &timeout) <= 0)
		return(0);

	return(recvfrom(s, buf, len, 0, (struct sockaddr *) fhost, &fhlen));
}


udp_close(s)

int	s;
{
	close(s);
}


udp_write(s, buf, len, fhost, fport)

int	s;
caddr_t	buf;
int	len;
struct	in_addr	fhost;
int	fport;
{
	struct	sockaddr_in	fsock;

	fsock.sin_family = AF_INET;
	fsock.sin_port = fport;
	fsock.sin_addr = fhost;

	return(sendto(s, buf, len, 0,(struct sockaddr *)&fsock,sizeof(fsock)));
}


udp_free(buf)

caddr_t	buf;
{
	free(buf);
}
