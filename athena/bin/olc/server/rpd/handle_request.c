/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/handle_request.c,v 1.1 1990-11-27 11:51:34 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

void
handle_request(fd, from)
     int fd;
     struct sockaddr_in from;
{

  int len;
  char username[9];
  int instance;
  int version;
  long output_len;
  char *buf;
  int result;

#ifdef KERBEROS
  KTEXT_ST their_auth;
  AUTH_DAT their_info;
  int ltr;
  int auth;
  char instance_buffer[INST_SZ];
#endif /* KERBEROS */

  if ((len = sread(fd,&version,sizeof(version))) != sizeof(version)) {
    fprintf(stderr,"Not enough bytes for version (%d received)\n",len);
    perror("read version");
    punt_connection(fd,from);
    return;
  }

  version = ntohs(version);
  if (version != VERSION) {
    fprintf(stderr,"Version skew from %s\n curr = %d, recvd = %d\n",
	    inet_ntoa(from.sin_addr),VERSION,version);
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,username,9)) != 9) {
    fprintf(stderr,"Wanted nine bytes of username, only got %d\n",len);
    perror("read username");
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,&instance,sizeof(instance))) != sizeof(instance)) {
    fprintf(stderr,"Not enough bytes for instance (%d)\n",len);
    perror("read instance");
    punt_connection(fd,from);
    return;
  }

  instance = ntohs(instance);

#ifdef KERBEROS

  if ((len = sread(fd,&their_auth.length, sizeof(their_auth.length))) !=
      sizeof(their_auth.length)) {
    fprintf(stderr,"Not enought bytes for ticket (%d)\n",len);
    perror("read ticket length");
    punt_connection(fd,from);
    return;
  }

  their_auth.length = ntohs(their_auth.length);
  bzero(their_auth.dat,sizeof(their_auth.dat));
  ltr =MIN(sizeof(unsigned char)*their_auth.length,
	   sizeof(their_auth.dat));
  if ((len = sread(fd,&their_auth.dat,ltr)) != ltr) {
    fprintf(stderr,"Error reading kerberos ticket (%d)\n",len);
    perror("read: kdata");
    punt_connection(fd,from);
    return;
  }
  their_auth.mbz = 0;
  instance_buffer[0] = '*';
  instance_buffer[1] = '\0';
  auth = krb_rd_req(&their_auth,K_SERVICE,instance_buffer,
		    (unsigned long) from.sin_addr.s_addr,&their_info,"");
  if (auth != RD_AP_OK) {
    /* Twit! */
    fprintf(stderr,"Kerberos error: %s\n",krb_err_txt[auth]);
    output_len = htonl(-auth);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }
#endif /* KERBEROS */

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
}

void
punt_connection(fd, from)
     int fd;
     struct sockaddr_in from;
{
  shutdown(fd,2);   /* Not going to send or receive from this guy again.. */
  close(fd);
  fprintf(stderr,"Punted connection from %s\n",inet_ntoa(from.sin_addr));
  return;
}
