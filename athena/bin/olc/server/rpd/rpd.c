/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/rpd.c,v 1.1 1990-11-18 18:52:34 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

  int sock;	/* the listening socket */
  int fd;	/* the accepting socket */

main()
{
  int len;
  int output_fd;
  long output_len;
  struct sockaddr_in sin,from;
  struct servent *sent;
  struct stat stat;
  char username[9];
  char pathname[128];
  char *buf;
  int buf_size;
  int instance;

  buf_size = 16384;
  if ((buf = malloc(16384)) == NULL) {
    fprintf(stderr,"replayerd: error malloc'ing inital buffer\n");
    exit(1);
  }
  
  signal(SIGHUP,clean_up);
  signal(SIGINT,clean_up);
  signal(SIGTERM,clean_up);
  signal(SIGPIPE,SIG_IGN);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("replayerd: socket");
    exit(1);
  }

  if ((sent = getservbyname("ols","tcp")) == NULL) {
    fprintf(stderr,"replayerd: ols/tcp unknown service\n");
    exit(1);
  }

  bzero(&sin,sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = sent->s_port;

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    perror("replayerd: bind");
    exit(1);
  }

  if (listen(sock,5) == -1) {
    perror("replayerd: listen");
    exit(1);
  }

  /* Main loop... */

  while (1) {
    if ((fd = accept(sock,(struct sockaddr *) &from, &len)) == -1) {
      perror("replayerd: accept");
      exit(1);
    }

    if ((len = read(fd,username,9)) != 9) {
      fprintf(stderr,"Wanted nine bytes of username, only got %d\n",len);
      perror("read username");
      exit(1);
    }

    if ((len = read(fd,&instance,sizeof(instance))) != sizeof(instance)) {
      fprintf(stderr,"Not enough bytes for instance %d\n",len);
      perror("read instance");
      exit(1);
    }

    instance = ntohs(instance);
    sprintf(pathname,"/usr/spool/olc/%s_%d.log",username,instance);
    if ((output_fd = open(pathname,O_RDONLY,0)) < 0) {
      output_len = htonl(-1);
      write(fd,&output_len,sizeof(long));
    }
    else {
      fstat(output_fd,&stat);
      if (buf_size < stat.st_size) {
	buf_size = stat.st_size*2;
	free(buf);
	if ((buf = malloc(buf_size)) == NULL) {
	  output_len = htonl(-2);
	  write(fd,&output_len,sizeof(long));
	  fprintf(stderr,"Error mallocing %d bytes\n",buf);
	  exit(1);
	}
      }
      read(output_fd,buf,stat.st_size);
      output_len = htonl(stat.st_size);
      write(fd,&output_len,sizeof(long));
      write(fd,buf,stat.st_size);
      close(fd);
    }
  }
}

int clean_up(signal)
     int signal;
{
  close(fd);
  close(sock);
  exit(0);
}
