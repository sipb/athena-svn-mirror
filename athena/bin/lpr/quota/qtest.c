/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qtest.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qtest.c,v 1.1 1990-04-16 16:30:00 epeisach Exp $
 */

#if (!defined(lint) && !defined(SABER))
static char qmain_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/qtest.c,v 1.1 1990-04-16 16:30:00 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "quota.h"
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *progname;

main(argc, argv)
        int argc;
        char **argv;
{
    int fd;
    struct sockaddr_in sin_c;
    struct servent *servname;

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "Could not create socket - FATAL\n");
	return -1;
    }

    bzero((char *)&sin_c, sizeof(sin_c));
    sin_c.sin_family = AF_INET;
    servname = getservbyname("lpquota","udp");
    if(!servname) 
	sin_c.sin_port = htons(QUOTASERVENT);
    else 
	sin_c.sin_port = servname->s_port;


  /* Set the socket address */
	{
	    struct hostent *host;
	    host = gethostbyname("hal-2000");
	    bcopy(host->h_addr_list[0], &sin_c.sin_addr,host->h_length);
	}

    if(connect(fd, &sin_c, sizeof(sin_c)) < 0) {
	printf("Could not connect\n");
    }
    
    if(send(fd, argv[1], strlen(argv[1]),0) < 0 )
	printf("oops...\n");
    close(fd);

    /* Everything worked. Return */
    return 0;
}
	
