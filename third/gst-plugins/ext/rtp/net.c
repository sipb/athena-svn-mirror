/*
  Librtp - a library for the RTP/RTCP protocol
  Copyright (C) 2000  Roland Dreier
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  $Id: net.c,v 1.1.1.1 2003-01-29 21:51:31 ghudson Exp $
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gphone-lib.h"

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <glib.h>
#include <string.h>

#include "gphone.h"

/*int
find_host(char *host_name, struct in_addr *address)
{
  struct hostent *host;

  if (inet_aton(host_name, address)) {
    return 1;
  } else {
    host = gethostbyname(host_name);
  }
  
  if (!host) {
    herror("error looking up host");
    return 0;
  }

  memcpy(address, (struct in_addr *) host->h_addr_list[0],
         sizeof(struct in_addr));

  return 1;
}*/

int
recv_fd(int sock)
{
  struct iovec vector;
  struct msghdr msg;
  struct cmsghdr *cmsg;
  char buf;
  int fd;

  vector.iov_base = &buf;
  vector.iov_len = sizeof(buf);

  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &vector;
  msg.msg_iovlen = 1;

  cmsg = g_malloc(sizeof(struct cmsghdr) + sizeof(fd));
  cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
  msg.msg_control = cmsg;
  msg.msg_controllen = cmsg->cmsg_len;

  if (!recvmsg(sock, &msg, 0)) {
    gphone_perror_exit("*** recv_fd : recvmsg", 1);
  }

  memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));

  g_free(cmsg);

  return(fd);
}

void
send_fd(int sock, int fd)
{
  struct iovec vector;
  struct msghdr msg;
  struct cmsghdr *cmsg;
  char buf;

  vector.iov_base = &buf;
  vector.iov_len = 1;
  
  msg.msg_name = NULL;
  msg.msg_namelen = 0;
  msg.msg_iov = &vector;
  msg.msg_iovlen = 1;
  
  cmsg = g_malloc(sizeof(struct cmsghdr) + sizeof(fd));
  cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  
  memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
  
  msg.msg_control = cmsg;
  msg.msg_controllen = cmsg->cmsg_len;
  
  if (sendmsg(sock, &msg, 0) != vector.iov_len) {
    gphone_perror_exit("*** send_fd : sendmsg", 1);
  }
  
  g_free(cmsg);
}

char *
gphone_getpeername(int sock)
{
  char *sock_name, *ip_num;
  struct hostent *hostinfo;
  struct sockaddr address;
  struct sockaddr_in *address_in;
  int len = sizeof(address);

  getpeername(sock, &address, &len);
  address_in = (struct sockaddr_in *) &address;

  hostinfo = gethostbyaddr((char *) &(address_in->sin_addr),
                           sizeof(address_in->sin_addr), AF_INET);

  if (hostinfo != NULL) {
    sock_name = g_malloc(strlen(hostinfo->h_name) + 1); /* add space for '\0' */
    strcpy(sock_name, hostinfo->h_name);
  } else {
    ip_num = inet_ntoa(address_in->sin_addr);
    sock_name = g_malloc(strlen(ip_num) + 1); /* add space for '\0' */
    strcpy(sock_name, ip_num);
  }

  return(sock_name);
}



/*
 * Local variables:
 *  compile-command: "make -k libgphone.a"
 * End:
 */
