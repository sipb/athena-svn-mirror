/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_message_from_server.c,v $
 * $Author: ghudson $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_get_message_from_server_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_message_from_server.c,v 1.4 1996-09-19 22:39:17 ghudson Exp $";
#endif lint

#include "globalmessage.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <hesiod.h>
#include <sys/time.h>

Code_t get_message_from_server(ret_message, ret_message_size, server)
     char **ret_message;
     int *ret_message_size;
     char *server;
{
  struct sockaddr_in server_insocket;
  int sck, stat;
  char *message_data;
  int message_size;
  
  /* guard against NULL arguments */
  if ((!ret_message)||(!server)) {
    return(GMS_NULL_ARG_ERR);
  }

  /* contact the server, send the request name, get the message back */
  /* create the socket */
  sck = socket(AF_INET, SOCK_DGRAM, 0); /* 0 is VERY special... */
  if(sck == -1) {
    /* handle socket error */
    return(errno);
  }

  /* Set the socket family */
  server_insocket.sin_family = AF_INET;

  /* Set the socket port */
  {
    struct servent *gms_service, *hes_getservbyname();
    gms_service = getservbyname(GMS_SERV_NAME, GMS_SERV_PROTO);
    if(!gms_service) {
      /* getservbyname failed, fall back... */
      gms_service = hes_getservbyname(GMS_SERV_NAME, GMS_SERV_PROTO);
      if(!gms_service) {
	/* so did getservbyname, fall back to hard coded? */
	return(GMS_NO_SERVICE_NAME);
      }
    }
    server_insocket.sin_port = gms_service->s_port;
  }

  /* Set the socket address */
  {
    struct hostent *gms_host;
    gms_host = gethostbyname(server);
    if(!gms_host) {
      /* gethostbyname failed */
      return(gethost_error());
    }
    /* Copy in the first (preferred?) address of the server */
    memcpy(&server_insocket.sin_addr, gms_host->h_addr_list[0],
	  gms_host->h_length); 
  }

  /* Actually make the connection */
  {
    stat = connect(sck, &server_insocket, sizeof(server_insocket));
    if(stat == -1) {
      /* handle connect error */
      return(errno);
    }
  }

  /* send the version string as a datagram */
  stat = send(sck, GMS_VERSION_STRING, GMS_VERSION_STRING_LEN, 0);
  if (stat == -1) {
    /* handle send failed error */
    return(errno);
  }

  /* set up a timeout and select on the socket, to catch the return
   * packet */
  {
    fd_set reader;
    struct timeval timer;
    
    FD_ZERO(&reader);
    FD_SET(sck, &reader);

    timer.tv_sec = GMS_TIMEOUT_SEC;
    timer.tv_usec = GMS_TIMEOUT_USEC;

    stat = select(sck+1, &reader, 0, 0, &timer);
    if (stat == -1) {
      return(errno);
    }
    if (stat == 0) {
      return(GMS_TIMED_OUT);
    }
    /* since we only wait on reader, it must have arrived */
  }

  message_data = malloc(GMS_MAX_MESSAGE_LEN);
  stat = recv(sck, message_data, GMS_MAX_MESSAGE_LEN-1, 0);

  close(sck); /* regardless of any errors... */

  if(stat == -1) {
    free(message_data);
    return(errno);
  }

  message_data[stat] ='\0';
  *ret_message_size = stat;
  *ret_message = message_data;
  return(0);
}
