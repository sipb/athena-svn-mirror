/*
 *	$Id: qping.c,v 1.3 1999-01-22 23:11:07 ghudson Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */


#include "mit-copyright.h"
#include "quota.h"
#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>

#if (!defined(lint) && !defined(SABER))
static char qmain_rcsid[] = "$Id: qping.c,v 1.3 1999-01-22 23:11:07 ghudson Exp $";
#endif (!defined(lint) && !defined(SABER))

char *progname;

main(argc, argv)
        int argc;
        char **argv;
{
    int fd;
    struct sockaddr_in sin_c;
    struct servent *servname;
    int sequence,seq;

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	fprintf(stderr, "Could not create socket - FATAL\n");
	return -1;
    }

    if(argc != 2) usage(argv[0]);
    
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
	    host = gethostbyname(argv[1]);
	    if(!host) {
		fprintf(stderr, "%s: %s - not valid host\n", argv[0], argv[1]);
		exit(1);
	    }
	    bcopy(host->h_addr_list[0], &sin_c.sin_addr,host->h_length);
	}

    if(connect(fd, &sin_c, sizeof(sin_c)) < 0) {
	printf("Could not connect\n");
	exit(1);
    }
    
    sequence = 0;

    while(++sequence) {
	struct timeval tp;
	struct timezone tzp;
	fd_set set;
	int s1, s2;
	char buf[1024];

	buf[0] = 0; /* Protocol */
	s1=htonl(sequence);
	bcopy((char *)&s1, &buf[1], 4);
	gettimeofday(&tp, &tzp);
	s1=htonl(tp.tv_sec);
	bcopy((char *)&s1, &buf[5], 4);
	s1=htonl(tp.tv_usec);
	bcopy((char *)&s1, &buf[9], 4);
	if(send(fd, buf, 13,0)<1) printf("Sed failed\n");
	FD_ZERO(&set);
	FD_SET(fd, &set);
	tp.tv_sec = 3;
	tp.tv_usec = 0;

	if((s1=select(fd+1, &set, 0, 0, &tp))==0) {
	    /*Time to send another packet */
	    continue;
	}
	if(s1 < 0) {
	    printf("Oopssss...\n");
	    continue;
	}

	if(!FD_ISSET(fd, &set)) {
	    printf("Wrong socket response. Interesting.\n");
	    continue;
	}

	if((s1=recv(fd, buf, 13)) != 13) {
	    printf("Bogus response %d\n",s1);
	    continue;
	}
	/* Should check protocol match */
    	gettimeofday(&tp, &tzp);
	bcopy(&buf[5], (char *) &s1, 4);
	bcopy(&buf[9], (char *) &s2, 4);
	bcopy(&buf[1], (char *) &seq, 4);
	s1=ntohl(s1);
	s2=ntohl(s2);
	seq=ntohl(seq);
	printf("Return - %d - %d ms\n", seq,
	       (tp.tv_usec - s2 + 1000000 * (tp.tv_sec - s1))/1000);
	continue;
    }
}
	
usage(name)
char *name;
    {
	printf("%s: %s hostname\n", name, name);
	exit(1);
    }
