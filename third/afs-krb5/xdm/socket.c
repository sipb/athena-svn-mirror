/* $XConsortium: socket.c,v 1.34 94/04/17 20:03:47 gildea Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * socket.c - Support for BSD sockets
 */

#include "dm.h"

#ifdef XDMCP
#ifndef STREAMSCONN

#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>

#ifdef X_NOT_STDC_ENV
extern int errno;
#endif


extern int	xdmcpFd;
extern int	chooserFd;

extern FD_TYPE	WellKnownSocketsMask;
extern int	WellKnownSocketsMax;

CreateWellKnownSockets ()
{
    struct sockaddr_in	sock_addr;
    char		*name, *localHostname();

    if (request_port == 0)
	    return;
    Debug ("creating socket %d\n", request_port);
    xdmcpFd = socket (AF_INET, SOCK_DGRAM, 0);
    if (xdmcpFd == -1) {
	LogError ("XDMCP socket creation failed, errno %d\n", errno);
	return;
    }
    name = localHostname ();
    registerHostname (name, strlen (name));
    RegisterCloseOnFork (xdmcpFd);
    /* zero out the entire structure; this avoids 4.4 incompatibilities */
    bzero ((char *) &sock_addr, sizeof (sock_addr));
#ifdef BSD44SOCKETS
    sock_addr.sin_len = sizeof(sock_addr);
#endif
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons ((short) request_port);
    sock_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (xdmcpFd, (struct sockaddr *)&sock_addr, sizeof (sock_addr)) == -1)
    {
	LogError ("error %d binding socket address %d\n", errno, request_port);
	close (xdmcpFd);
	xdmcpFd = -1;
	return;
    }
    WellKnownSocketsMax = xdmcpFd;
    FD_SET (xdmcpFd, &WellKnownSocketsMask);

    chooserFd = socket (AF_INET, SOCK_STREAM, 0);
    Debug ("Created chooser socket %d\n", chooserFd);
    if (chooserFd == -1)
    {
	LogError ("chooser socket creation failed, errno %d\n", errno);
	return;
    }
    listen (chooserFd, 5);
    if (chooserFd > WellKnownSocketsMax)
	WellKnownSocketsMax = chooserFd;
    FD_SET (chooserFd, &WellKnownSocketsMask);
}

GetChooserAddr (addr, lenp)
    char	*addr;
    int		*lenp;
{
    struct sockaddr_in	in_addr;
    int			len;

    len = sizeof in_addr;
    if (getsockname (chooserFd, (struct sockaddr *)&in_addr, &len) < 0)
	return -1;
    memmove( addr, (char *) &in_addr, len);
    *lenp = len;
    return 0;
}

#endif /* !STREAMSCONN */
#endif /* XDMCP */
