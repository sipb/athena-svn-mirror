/*
 * Log Replayer Client
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/clients/oreplay/oreplay.c,v 1.1 1990-11-18 18:52:25 lwvanels Exp $";
#endif
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>
#include <strings.h>

#include "oreplay.h"

main(argc,argv)
     int argc;
     char **argv;
{
  int sock;    /* the socket */
  long len;
  int fd;
  struct sockaddr_in sin;
  struct servent *sent;
  char username[9];
  int instance;
  int c;
  char **olc_servers;
  char *buf;
  char *bufp;
  int total_read;
  struct hostent *hp;
  int output_fd;
  extern char *optarg;
  extern int optind;

  output_fd = 1;
  hp = NULL;

  while ((c = getopt(argc, argv, "f:s:")) != EOF)
    switch(c) {
    case 'f':
      if ((output_fd = open(optarg, O_WRONLY|O_CREAT|O_TRUNC,
			    S_IREAD|S_IWRITE)) < 0) { 
	perror("oreplay: opening file");
	exit(1);
      }
      break;
    case 's':
      if ((hp = gethostbyname(optarg)) == NULL) {
	fprintf(stderr,"oreplay: Unknown host %s\n",optarg);
	exit(1);
      }
      break;
    case '?':
    default:
      usage();
      exit(1);
    }
  
  instance = 0;

  if (((argc - optind) > 2) || (argc == optind)) {
    usage();
    exit(1);
  }
    
  strcpy(username,argv[optind]);
  optind++;
  if ((argc - optind) == 1)
    instance = atoi(argv[optind]);

  if (hp == NULL) {
    if ((olc_servers = hes_resolve("OLC","SLOC")) == NULL) {
      fprintf(stderr,"oreplay: Unable to get hesiod infomration for OLC/SLOC\n");
      exit(1);
    }
    
    if ((hp = gethostbyname(olc_servers[0])) == NULL) {
      fprintf(stderr,"oreplay: Unknown host %s\n",olc_servers[0]);
      exit(1);
    }
  }

  if ((sent = getservbyname("ols","tcp")) == NULL) {
    fprintf(stderr,"oreplay: ols/tcp unknown service\n");
    exit(1);
  }

  bzero(&sin,sizeof(sin));
  bcopy(hp->h_addr,(char *)&sin.sin_addr,hp->h_length);
  sin.sin_family = hp->h_addrtype;
  sin.sin_port = sent->s_port;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("oreplay: socket");
    exit(1);
  }

  if (connect(sock,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
    perror("oreplay: connect");
    exit(1);
  }

  write(sock,username,9);
  instance = htons(instance);
  write(sock,&instance,sizeof(instance));
  read(sock,&len,sizeof(len));
  len = ntohl(len);
  if (len < 0) {
    switch (len) {
    case -1:
      fprintf(stderr,"No such question\n");
      break;
    case -2:
      fprintf(stderr,"Error on the server\n");
      break;
    default:
      fprintf(stderr,"Unknown error %d\n",len);
    }
    exit(1);
  }
  buf = malloc(len);
  total_read = 0;
  while (total_read < len) {
    c = read(sock,buf,(int)len);
    write(output_fd,buf,c);
    total_read += c;
  }
  close(sock);
  exit(0);
}

void usage()
{
  fprintf(stderr,"Usage: oreplay [-f filename] [-s server] username instance\n");
}
