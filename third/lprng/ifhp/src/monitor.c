/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: monitor.c,v 1.1.1.1 1999-02-17 15:31:04 ghudson Exp $";

#include "ifhp.h"

/**** ENDINCLUDE ****/
/*
 * Monitor for udp information
 *  Opens a UDP socket and waits for data to be sent to it.  This is a
 * sample for diagnostic purposes.
 *
 *  monitor port
 *   port is an integer number or a service name in the services database
 */

#include "portable.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


#if defined(HAVE_STDARGS)
int plp_snprintf (char *str, size_t count, const char *fmt, ...);
int plp_vsnprintf (char *str, size_t count, const char *fmt, va_list arg);
#else
int plp_snprintf ();
int plp_vsnprintf ();
#endif

extern int errno;

int udp_open( char *portname );
const char * Errormsg ( int err );

char buffer[1024];

int main( int argc, char *argv[] )
{
	int fd;
	int n, i, cnt;
	if( argc != 2 ){
		fprintf( stderr, "usage: monitor port\n" );
		exit(1);
	}
	fd = udp_open( argv[1] );
	while( (n = read( fd, buffer, sizeof(buffer)-1) ) > 0 ){
		for( i = 0; i < n; i += cnt ){
			cnt = write(1, buffer+i, n-i );
		}
	}
	return(0);
}

int udp_open( char *portname )
{
	int port, i, fd, err;
	struct sockaddr_in sin;
	struct servent *servent;

	port = atoi( portname );
	if( port <= 0 ){
		servent = getservbyname( portname, "udp" );
		if( servent ){
			port = ntohs( servent->s_port );
		}
	}
	if( port <= 0 ){
		fprintf( stderr, "udp_open: bad port number '%s'\n",portname );
		return( -1 );
	}
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons( port );

	fd = socket( AF_INET, SOCK_DGRAM, 0 );
	err = errno;
	if( fd < 0 ){
		fprintf(stderr,"udp_open: socket call failed - %s\n", Errormsg(err) );
		return( -1 );
	}
	i = -1;
	i = bind( fd, (struct sockaddr *) & sin, sizeof (sin) );
	err = errno;

	if( i < 0 ){
		fprintf(stderr,"udp_open: connect to '%s port %d' failed - %s\n",
			inet_ntoa( sin.sin_addr ), ntohs( sin.sin_port ),
			Errormsg(errno) );
		close(fd);
		fd = -1;
	}
	return( fd );
}


/****************************************************************************
 * Extract the necessary definitions for error message reporting
 ****************************************************************************/

#if !defined(HAVE_STRERROR)
# if defined(HAVE_SYS_NERR)
#   if !defined(HAVE_SYS_NERR_DEF)
      extern int sys_nerr;
#   endif
#   define num_errors    (sys_nerr)
# else
#  	define num_errors    (-1)            /* always use "errno=%d" */
# endif
# if defined(HAVE_SYS_ERRLIST)
#  if !defined(HAVE_SYS_ERRLIST_DEF)
    extern const char *const sys_errlist[];
#  endif
# else
#  undef  num_errors
#  define num_errors   (-1)            /* always use "errno=%d" */
# endif
#endif

const char * Errormsg ( int err )
{
    const char *cp;

#if defined(HAVE_STRERROR)
	cp = strerror(err);
#else
# if defined(HAVE_SYS_ERRLIST)
    if (err >= 0 && err <= num_errors) {
		cp = sys_errlist[err];
    } else
# endif
	{
		static char msgbuf[32];     /* holds "errno=%d". */
		(void) plp_snprintf (msgbuf,sizeof(msgbuf), "errno=%d", err);
		cp = msgbuf;
    }
#endif
    return (cp);
}
