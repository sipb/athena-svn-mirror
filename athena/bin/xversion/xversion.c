#include <X11/copyright.h>
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 * This program determines which X server is likely to be running by
 * attempting to connect to the X socket.
 *
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.2 1989-06-10 11:48:01 probe Exp $
 */

static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.2 1989-06-10 11:48:01 probe Exp $";

#include <X11/Xlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
 
#define X_UNIX_PATH	"/tmp/X0"	/* X10 UNIX socket! */

main(argc,argv)
int argc;
char *argv[];
{
    struct sockaddr_un addr;		/* UNIX socket address */
    int full_version = 0;		/* Full protocol version number? */
    int addrlen;
    int fd;
    int i;
    Display *dpy;

    if (--argc)
	if (!strcmp(*++argv, "-r"))
	    full_version = 1;
    
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, X_UNIX_PATH);
    addrlen = strlen(addr.sun_path) + 2;

    if ((fd = socket(addr.sun_family, SOCK_STREAM, 0)) < 0) {
        /* Socket call failed! */
        /* errno set by system call. */
        perror( "couldn't open socket" );
        exit( 1 );
    }
 
    /* try connecting to X10 first */
    if (connect(fd, &addr, addrlen) == 0) {
        close(fd);
        printf( "10\n" );
	exit( 0 );
    }

    /* X11 requires connection information */
    if ((dpy = XOpenDisplay( "unix:0" )) != NULL) {
        printf((full_version ? "X%dR%d\n" : "%d\n" ),
	       ProtocolVersion(dpy),
	       VendorRelease(dpy));
	XCloseDisplay( dpy );
	exit( 0 );
    }

    printf( "?unknown\n" );
    exit( 1 );
}
