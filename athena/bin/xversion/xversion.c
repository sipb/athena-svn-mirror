/* Copyright    Massachusetts Institute of Technology    1985	*/

/*
 * This program determines which X server is likely to be running by
 * attempting to connect to the X socket.
 *
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.4 1995-12-05 01:32:45 epeisach Exp $
 */

static char *rcsid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/xversion/xversion.c,v 1.4 1995-12-05 01:32:45 epeisach Exp $";

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
