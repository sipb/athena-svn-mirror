#include <X11/copyright.h>
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 * stolen from X10/XOpenDisplay
 *
 * determine which server is likely to be running by attempting to
 *
 */
 
#include <X11/Xlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
 
#define X_UNIX_PATH	"/tmp/X0"	/* X10 UNIX socket! */

main()
{
    struct sockaddr_un addr;		/* UNIX socket address */
    int addrlen;
    int fd;
    Display *dpy;

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
        printf( "11\n" );
	XCloseDisplay( dpy );
	exit( 0 );
    }

    printf( "?unknown\n" );
    exit( 1 );
}
