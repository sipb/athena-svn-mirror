/*
 * Log Replayer Daemon
 *
 * This replays question logs
 */

#ifndef lint
#ifndef SABER
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/server/rpd/handle_request.c,v 1.12 1991-04-08 21:20:16 lwvanels Exp $";
#endif
#endif

#include "rpd.h"

void
handle_request(fd, from)
     int fd;
     struct sockaddr_in from;
{

  int len;
  char username[9],tusername[9];
  int instance,tinstance;
  int version;
  int request;
  u_long output_len;
  char *buf;
  int result;

#ifdef KERBEROS
  KTEXT_ST their_auth;
  AUTH_DAT their_info;
  int ltr;
  int auth;
  char principal_buffer[ANAME_SZ+INST_SZ+REALM_SZ];
  static char instance_buffer[INST_SZ];

  if (instance_buffer[0] == '\0')
    instance_buffer[0] = '*';
#endif /* KERBEROS */

  if ((len = sread(fd,&version,sizeof(version))) != sizeof(version)) {
    if (len == -1)
      syslog(LOG_ERR,"reading version: %m");
    else
      syslog(LOG_WARNING,"Not enough bytes for version (%d received)",len);
    punt_connection(fd,from);
    return;
  }

  version = ntohl(version);
  if (version > VERSION) {
    syslog(LOG_WARNING,"Version skew from %s curr = %d, recvd = %d\n",
	    inet_ntoa(from.sin_addr),VERSION,version);
    punt_connection(fd,from);
    return;
  }

  if (version >= 1) {
    if ((len = sread(fd,&request,sizeof(request))) != sizeof(request)) {
      if (len == -1)
	syslog(LOG_ERR,"reading request: %m");
      else
	syslog(LOG_WARNING,"Not enough bytes for request (%d received)",len);
      punt_connection(fd,from);
      return;
    }
    request = ntohl(request);
  }
  else
    request = LIST_REQ;

  if ((len = sread(fd,username,9)) != 9) {
    if (len == -1)
      syslog(LOG_ERR,"reading username: %m");
    else
      syslog(LOG_WARNING,"Wanted nine bytes of username, only got %d\n",len);
    punt_connection(fd,from);
    return;
  }

  if ((len = sread(fd,&instance,sizeof(instance))) != sizeof(instance)) {
    if (len == -1)
      syslog(LOG_ERR,"reading instance: %m");
    else
      syslog(LOG_WARNING,"Not enough bytes for instance (%d)\n",len);
    punt_connection(fd,from);
    return;
  }

  instance = ntohl(instance);

  if (request == REPLAY_KILL_REQ) {
    /* both of these take a target username */
    if ((len = sread(fd,tusername,9)) != 9) {
      if (len == -1)
	syslog(LOG_ERR,"reading tusername: %m");
      else
	syslog(LOG_WARNING,"Wanted nine bytes of tusername, only got %d\n",
	       len);
      punt_connection(fd,from);
      return;
    }

    if ((len = sread(fd,&tinstance,sizeof(tinstance))) != sizeof(tinstance)) {
      if (len == -1)
	syslog(LOG_ERR,"reading tinstance: %m");
      else
	syslog(LOG_WARNING,"Not enough bytes for tinstance (%d)\n",len);
      punt_connection(fd,from);
      return;
    }

    tinstance = ntohl(tinstance);
  }

  
  if ((len = sread(fd,&their_auth.length, sizeof(their_auth.length))) !=
      sizeof(their_auth.length)) {
    if (len == -1)
      syslog(LOG_ERR,"reading kticket length: %m");
    else
      syslog(LOG_WARNING,"Not enought bytes for ticket (%d)\n",len);
    punt_connection(fd,from);
    return;
  }

  their_auth.length = ntohl(their_auth.length);

  if (their_auth.length != 0) {
    bzero(their_auth.dat,sizeof(their_auth.dat));
    ltr =MIN(sizeof(unsigned char)*their_auth.length,
	     sizeof(their_auth.dat));
    if ((len = sread(fd,their_auth.dat,ltr)) != ltr) {
      if (len == -1)
	syslog(LOG_ERR,"reading kticket: %m");
      else
	syslog(LOG_WARNING,"Error reading kerberos ticket (%d)\n",len);
      punt_connection(fd,from);
      return;
    }
  }

#ifdef KERBEROS
  auth = krb_rd_req(&their_auth,K_SERVICE,instance_buffer,
		    (unsigned long) from.sin_addr.s_addr,&their_info,"");
  if (auth != RD_AP_OK) {
    /* Twit! */
    syslog(LOG_WARNING,"Kerberos error: %s\n from %s",krb_err_txt[auth],
	   inet_ntoa(from.sin_addr));
    output_len = htonl(-auth);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  sprintf(principal_buffer,"%s.%s@%s",their_info.pname, their_info.pinst,
	  their_info.prealm);
  if ((strcmp(their_info.pname,username) != 0) &&
      !acl_check(MONITOR_ACL,principal_buffer)) {
    /* Twit! */
    syslog(LOG_WARNING,"Request from %s@%s who is not on the acl\n",
	    principal_buffer, inet_ntoa(from.sin_addr));
    output_len = htonl(ERR_NO_ACL);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

  syslog(LOG_DEBUG,"%s replays %s [%d]",principal_buffer, username,
	 instance);

  if (((request == SHOW_KILL_REQ) && strcmp(their_info.pname,username)) ||
      ((request == REPLAY_KILL_REQ) && strcmp(their_info.pname,tusername))) {
    syslog(LOG_WARNING, "Request to delete %s's new messages from %s@%s\n",
	   username,principal_buffer, inet_ntoa(from.sin_addr));
    output_len = htonl(ERR_OTHER_SHOW);
    write(fd,&output_len,sizeof(long));
    punt_connection(fd,from);
    return;
  }

#endif /* KERBEROS */

  switch(request) {
  case SHOW_KILL_REQ:
    buf = get_nm(username,instance,&result,1);
    break;
  case SHOW_NO_KILL_REQ:
    buf = get_nm(username,instance,&result,0);
    break;
  case LIST_REQ:
    buf = get_log(username,instance,&result);
    break;
  case REPLAY_KILL_REQ:
    buf = get_nm(tusername,tinstance,&result,1);
    if ((buf == NULL) && (result != 0))
      break;
    buf = get_log(username,instance,&result);
    break;
  default:
    /* Sorry, not here- */
    output_len = htonl(ERR_NOT_HERE);
    write(fd,&output_len,sizeof(u_long));
    return;
  }
  if (buf == NULL) {
    /* Didn't get response; determine if it's an error or simply that the */
    /* question just doesn't exist based on result */
    if (result == 0)
      output_len = htonl(ERR_NO_SUCH_Q);
    else
      output_len = htonl(ERR_SERV);
    write(fd,&output_len,sizeof(u_long));
  }
  else {
    /* All systems go, write response */
    output_len = htonl((u_long)result);
    write(fd,&output_len,sizeof(u_long));
    write(fd,buf,result);
  }
}

void
punt_connection(fd, from)
     int fd;
     struct sockaddr_in from;
{
  close(fd);
  syslog(LOG_INFO,"Punted connection from %s\n",inet_ntoa(from.sin_addr));
  return;
}
