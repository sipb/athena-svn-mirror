/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/rpd.c,v 1.2 1990-11-18 21:07:40 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

  int sock;	/* the listening socket */
  int fd;	/* the accepting socket */

main()
{
  int len;
  long output_len;
  struct sockaddr_in sin,from;
  struct servent *sent;
  char username[9];
  char *buf;
  int instance;
  int result;

  signal(SIGHUP,clean_up);
  signal(SIGINT,clean_up);
  signal(SIGTERM,clean_up);
  signal(SIGPIPE,SIG_IGN);

  init_cache();

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
    buf = get_log(username,instance,&result);
    if (buf == NULL) {
      /* Didn't get response; determine if it's an error or simply that the */
      /* question just doesn't exist based on result */
      if (result == 0)
	output_len = htonl(-1L);
      else
	output_len = htonl(-2L);
      write(fd,&output_len,sizeof(long));
    }
    else {
      /* All systems go, write response */
      output_len = htonl((long)result);
      write(fd,&output_len,sizeof(long));
      write(fd,buf,result);
    }
    close(fd);
  }
}

int clean_up(signal)
     int signal;
{
  close(fd);
  shutdown(sock,2);
  close(sock);
  exit(0);
}
