/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
static char *const _id = "$Id: open_device.c,v 1.1.1.1 1999-02-17 15:31:05 ghudson Exp $";

#include "ifhp.h"

void Do_stty();

void Open_device( char *device )
{
	char *s, *end;
	int i, fd = -1, port, option, attempt;
	struct hostent *hostent;
	struct sockaddr_in sin;
	int src_port = 0;
	static char *dcopy;

	DEBUG1("Open_device: device '%s'", device);
	if( dcopy ) free(dcopy);
	dcopy = 0;
	dcopy = safestrdup( device,__FILE__,__LINE__ );

	/* we need to close fd 1 */
	close(1);
	Errorcode = 0;
	if( (dcopy[0] == '/') ){
		attempt = 1;
		while( fd < 0 ){
			DEBUG1("Open_device: device '%s', attempt %d",
				device, attempt);
			fd = open( dcopy, O_WRONLY|O_APPEND|O_CREAT, 0600 );
			if( fd < 0 ){
				if( Dev_retries == 0 || (Dev_retries >0 && attempt++ >= Dev_retries) ){
					Errorcode = JABORT;
					logerr_die("Open_device: open '%s' failed", device);
				}
				if( Dev_sleep > 0 ) sleep(Dev_sleep);
			}
		}
		if( isatty( fd ) && Stty_args){
			Do_stty( fd, Stty_args );
		}
	} else {
		if( (end = strchr( dcopy, '%' )) == 0 ){
			Errorcode = JABORT;
			fatal("Open_device: missing port number '%s'",device );
		}
		*end++ = 0; 
		s = end;
		port = strtol( end, &s, 0 );
		if( port <= 0 ){
			Errorcode = JABORT;
			fatal("Open_device: bad port number '%s'",end );
		}
		end = s;
		if( end && *end ){
			*end++ = 0;
			src_port = strtol( end, 0, 0 );
		}
		memset( &sin, 0, sizeof(sin) );
		sin.sin_family = AF_INET;
		if( (hostent = gethostbyname(dcopy)) ){
			/*
			 * set up the address information
			 */
			if( hostent->h_addrtype != AF_INET ){
				Errorcode = JABORT;
				fatal("Open_device: bad address type for host '%s'", device);
				cleanup(0);
			}
			memcpy( &sin.sin_addr, hostent->h_addr, hostent->h_length );
		} else {
			sin.sin_addr.s_addr = inet_addr(dcopy);
			if( sin.sin_addr.s_addr == -1){
				Errorcode = JABORT;
				fatal("Open_device: getconnection: unknown host '%s'", device);
				cleanup(0);
			}
		}
		sin.sin_port = htons( port );
		attempt = 1;
		fd = -1;
		for( fd = -1; fd < 0; ++attempt ){
			if( attempt > 1 ){
				if( Dev_retries == 0 || (Dev_retries >0 && attempt >= Dev_retries) ){
					Errorcode = JABORT;
					logerr_die("Open_device: open '%s' failed", device);
				}
				if( Dev_sleep > 0 ) sleep(Dev_sleep);
			}
			DEBUG1("Open_device: destination '%s' port %d, attempt %d",
				inet_ntoa( sin.sin_addr ), ntohs( sin.sin_port ), attempt );
			fd = socket (AF_INET, SOCK_STREAM, 0);
			if( fd < 0 ){
				logerr( "Open_device: socket() failed" ); 
			}
			option = 1;
			if( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
					(char *)&option, sizeof(option) ) ){
				Errorcode = JABORT;
				logerr_die( "Open_device: setsockopt failed" );
			}
			if( src_port ){
				struct sockaddr_in src_sin;
				memset( &src_sin, 0, sizeof(src_sin) );
				src_sin.sin_family = AF_INET;
				src_sin.sin_port = htons( src_port );
				if( bind( fd, (struct sockaddr *)&src_sin,
					sizeof( src_sin )) < 0 ){
					logerr("Open_device: bind failed");
					close(fd);
					fd = -1;
					continue;
				}
			}
			i = connect (fd, (struct sockaddr *) & sin, sizeof (sin));
			if( i < 0 ){
				Errorcode = JFAIL;
				logerr("Open_device: connect to '%s port %d' failed",
					inet_ntoa( sin.sin_addr ), ntohs( sin.sin_port ) );
				close(fd);
				fd = -1;
				continue;
			}
		}
	}
	if( fd != 1 ){
		if( dup2( fd, 1 ) < 0 ){
			Errorcode = JABORT;
			logerr_die("Open_device: open '%s' failed",device);
		}
		close( fd );
	}
	DEBUG1("Open_device: success");
}
