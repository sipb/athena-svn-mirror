/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/rpd.c,v 1.3 1990-11-27 11:53:45 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

int sock;	/* the listening socket */
int fd;		/* the accepting socket */

main()
{
  int len;
  struct sockaddr_in sin,from;
  struct servent *sent;
  int onoff;

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

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &onoff, sizeof(int)) < 0) {
    perror("replayerd: setsockopt");
    exit(1);
  }

  if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
    perror("replayerd: bind");
    exit(1);
  }

  if (listen(sock,SOMAXCONN) == -1) {
    perror("replayerd: listen");
    exit(1);
  }

  /* Main loop... */

  while (1) {
    len = sizeof(from);
    if ((fd = accept(sock,(struct sockaddr *) &from, &len)) == -1) {
      if (errno == EINTR) {
	fprintf(stderr,"Interrupted by signal in accept; continuing\n");
	continue;
      }
      perror("replayerd: accept");
      exit(1);
    }
    handle_request(fd,from);
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
