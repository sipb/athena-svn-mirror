/* GNet - Networking library
 * Copyright (C) 2000  David Helder
 * Copyright (C) 2000  Andrew Lanoix
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef _GNET_PRIVATE_H
#define _GNET_PRIVATE_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#include <glib.h>
#include "gnet.h"

#ifndef GNET_WIN32  /*********** Unix specific ***********/

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <netinet/in_systm.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>		/* Need for TOS */
#include <arpa/inet.h>

#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#ifndef socklen_t
#define socklen_t size_t
#endif

#define GNET_CLOSE_SOCKET(SOCKFD) close(SOCKFD)
#define GNET_SOCKET_IOCHANNEL_NEW(SOCKFD) g_io_channel_unix_new(SOCKFD)

#else	/*********** Windows specific ***********/

#include <windows.h>
#include <winbase.h>
#include <winuser.h>
#include <ws2tcpip.h>

#define socklen_t gint32

#define GNET_CLOSE_SOCKET(SOCKFD) closesocket(SOCKFD)
#define GNET_SOCKET_IOCHANNEL_NEW(SOCKFD) g_io_channel_win32_new_socket(SOCKFD)

#endif	/*********** End Windows specific ***********/

#ifndef INET_ADDRSTRLEN
#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46
#endif

#define GNET_SOCKADDR_IN(s) (*((struct sockaddr_in*) &s))
#define GNET_ANY_IO_CONDITION  (G_IO_IN|G_IO_OUT|G_IO_PRI|G_IO_ERR|G_IO_HUP|G_IO_NVAL)


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**

   This is the private declaration and definition file.  The things in
   here are to be used by GNet ONLY.  Use at your own risk.  If this
   file includes something that you absolutely must have, write the
   GNet developers.

*/


struct _GUdpSocket
{
  gint sockfd;			/* private */
  struct sockaddr sa;		/* private (Why not an InetAddr?) */
  guint ref_count;
  GIOChannel* iochannel;
};

struct _GTcpSocket
{
  gint sockfd;
  struct sockaddr sa;		/* Why not an InetAddr? */
  guint ref_count;
  GIOChannel* iochannel;
};

struct _GUnixSocket
{
  gint sockfd;
  struct sockaddr sa;
  guint ref_count;
  gboolean server;
  GIOChannel *iochannel;
};

struct _GMcastSocket
{
  gint sockfd;
  struct sockaddr sa;
  guint ref_count;
  GIOChannel* iochannel;
};

struct _GInetAddr
{
  gchar* name;
  struct sockaddr sa;
  guint ref_count;
};


/* **************************************** */
/* Async functions			*/

gboolean gnet_inetaddr_new_async_cb (GIOChannel* iochannel, 
				     GIOCondition condition, 
				     gpointer data);

typedef struct _GInetAddrAsyncState 
{
  GInetAddr* ia;
  GInetAddrNewAsyncFunc func;
  gpointer data;
#ifndef GNET_WIN32
#ifdef HAVE_LIBPTHREAD
  pthread_t pthread;
#else
  pid_t pid;
#endif
  int fd;
  guint watch;
  guchar buffer[16];
  int len;
#else
  int WSAhandle;
  char hostentBuffer[MAXGETHOSTSTRUCT];
  int errorcode;
#endif

} GInetAddrAsyncState;



gboolean gnet_inetaddr_get_name_async_cb (GIOChannel* iochannel, 
					  GIOCondition condition, 
					  gpointer data);

typedef struct _GInetAddrReverseAsyncState 
{
  GInetAddr* ia;
  GInetAddrGetNameAsyncFunc func;
  gpointer data;
#ifndef GNET_WIN32
  pid_t pid;
  int fd;
  guint watch;
  guchar buffer[256 + 1];/* I think a domain name can only be 256 characters... */
  int len;
#else
  int WSAhandle;
  char hostentBuffer[MAXGETHOSTSTRUCT];
  int errorcode;
#endif

} GInetAddrReverseAsyncState;


gboolean gnet_tcp_socket_new_async_cb (GIOChannel* iochannel, 
				       GIOCondition condition, 
				       gpointer data);

typedef struct _GTcpSocketAsyncState 
{
  GTcpSocket* socket;
  GTcpSocketNewAsyncFunc func;
  gpointer data;
  gint flags;
  guint connect_watch;
  GInetAddr* socks_addr;
#ifdef GNET_WIN32
  gint errorcode;
#endif

} GTcpSocketAsyncState;

#ifdef GNET_WIN32
/*
Used for:
-gnet_inetaddr_new_async
-gnet_inetaddr_get_name_asymc
*/
typedef struct _SocketWatchAsyncState 
{
	GIOChannel *channel;
	gint fd;
	long winevent;
	gint eventcode;
  gint errorcode;
	GSList* callbacklist;
} SocketWatchAsyncState;
#endif

void gnet_tcp_socket_connect_inetaddr_cb(GInetAddr* inetaddr, 
					 GInetAddrAsyncStatus status, 
					 gpointer data);

void gnet_tcp_socket_connect_tcp_cb(GTcpSocket* socket, 
				    GTcpSocketConnectAsyncStatus status, 
				    gpointer data);

typedef struct _GTcpSocketConnectState 
{
  GInetAddr* ia;
  GTcpSocketConnectAsyncFunc func;
  gpointer data;

  gpointer inetaddr_id;
  gpointer tcp_id;

} GTcpSocketConnectState;


/* **************************************** 	*/
/* More Windows specific stuff 			*/

#ifdef GNET_WIN32

extern WNDCLASSEX gnetWndClass;
extern HWND  gnet_hWnd; 
extern guint gnet_io_watch_ID;
extern GIOChannel *gnet_iochannel;
	
extern GHashTable *gnet_hash;
extern HANDLE gnet_Mutex; 
extern HANDLE gnet_hostent_Mutex;
	
#define IA_NEW_MSG 100		/* gnet_inetaddr_new_async */
#define GET_NAME_MSG 101	/* gnet_inetaddr_get_name_asymc */

#endif



/* ************************************************************ */

/* Private/Experimental functions */


GInetAddr* gnet_private_inetaddr_sockaddr_new(const struct sockaddr sa);

struct sockaddr gnet_private_inetaddr_get_sockaddr(const GInetAddr* ia);
/* gtk-doc doesn't like this function...  (but it will be fixed.) */


/* TODO: Need to port this to Solaris and Windows.  This also assumes
   eth0 (we need to get the name of the interface of the socket) */

/*gint gnet_udp_socket_get_MTU(GUdpSocket* us);*/




#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /* _GNET_PRIVATE_H */
