#ifndef sgi
#include <X11/copyright.h>
#endif
/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 * This program determines which X server is likely to be running by
 * attempting to connect to the X socket.
 *
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.3 1995-07-21 01:40:39 cfields Exp $
 */

static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.3 1995-07-21 01:40:39 cfields Exp $";

#include <X11/Xlib.h>
#include <stdio.h>
 
main(argc,argv)
int argc;
char *argv[];
{
    int full_version = 0;		/* Full protocol version number? */
    Display *dpy;

    if (--argc)
	if (!strcmp(*++argv, "-r"))
	    full_version = 1;
    
    /* X11 requires connection information */
    if ((dpy = XOpenDisplay(NULL)) != NULL) {
        printf((full_version ? "X%dR%d\n" : "%d\n" ),
	       ProtocolVersion(dpy),
	       VendorRelease(dpy));
	XCloseDisplay( dpy );
	exit( 0 );
    }

    printf( "?unknown\n" );
    exit( 1 );
}
