/* util.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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
 * Boston, MA 02111-1307, USA.
 */
#include <config.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include <string.h>

#include <glib.h>
#include "util.h"

#ifdef HAVE_SOCKADDR_SA_LEN
#define LINC_SET_SOCKADDR_LEN(saddr, len)                     \
		((struct sockaddr *)(saddr))->sa_len = (len)
#else 
#define LINC_SET_SOCKADDR_LEN(saddr, len)
#endif

#if defined(AF_INET6) && defined(RES_USE_INET6)
#define LINC_RESOLV_SET_IPV6     _res.options |= RES_USE_INET6
#define LINC_RESOLV_UNSET_IPV6   _res.options &= ~RES_USE_INET6
#else
#define LINC_RESOLV_SET_IPV6
#define LINC_RESOLV_UNSET_IPV6
#endif

/*
 * True if succeeded in mapping, else false.
 */
static gboolean
ipv4_addr_from_addr (struct in_addr *dest_addr,
		     guint8         *src_addr,
		     int             src_length)
{
	if (src_length == 4)
		memcpy (dest_addr, src_addr, 4);

	else if (src_length == 16) {
		int i;

#ifdef LOCAL_DEBUG
		g_warning ("Doing conversion ...");
#endif

		/* An ipv6 address, might be an IPv4 mapped though */
		for (i = 0; i < 10; i++)
			if (src_addr [i] != 0)
				return FALSE;

		if (src_addr [10] != 0xff ||
		    src_addr [11] != 0xff)
			return FALSE;

		memcpy (dest_addr, &src_addr[12], 4);
	} else
		return FALSE;

	return TRUE;
}

/*
 * get_sockaddr:
 * @hostname: the hostname.
 * @portnum: the port number.
 * @saddr_len: location in which to store the returned structure's length.
 *
 * Allocates and fills a #sockaddr_in with with the IPv4 address 
 * information.
 *
 * Return Value: a pointer to a valid #sockaddr_in structure if the call 
 *               succeeds, NULL otherwise.
 */
struct sockaddr *
get_sockaddr (const char             *hostname,
	      const char             *portnum,
	      LincSockLen            *saddr_len)
{
#ifdef INET6

	struct sockaddr_in6 *saddr;
	struct hostent      *host;

	g_assert (hostname);

	if (!portnum)
		portnum = "0";

	saddr = g_new0 (struct sockaddr_in6, 1);

	*saddr_len = sizeof (struct sockaddr_in6);

	LINC_SET_SOCKADDR_LEN (saddr, sizeof (struct sockaddr_in6));

	saddr->sin6_family = AF_INET6;
	saddr->sin6_port = htons (atoi (portnum));
#ifdef HAVE_INET_PTON
	if (inet_pton (AF_INET6, hostname, &saddr->sin6_addr) > 0)
		return (struct sockaddr *)saddr;
#endif

	if (!(_res.options & RES_INIT))
		res_init();

	LINC_RESOLV_SET_IPV6;
	host = gethostbyname (hostname);
	if (!host || host->h_addrtype != AF_INET6) {
		g_free (saddr);
		return NULL;
	}

	memcpy (&saddr->sin6_addr, host->h_addr_list[0], sizeof (struct in6_addr));

	return (struct sockaddr *)saddr;
}
#else
	struct sockaddr_in *saddr;
	struct hostent     *host;

	g_assert (hostname);

	if (!portnum)
		portnum = "0";

	saddr = g_new0 (struct sockaddr_in, 1);

	*saddr_len = sizeof (struct sockaddr_in);

	LINC_SET_SOCKADDR_LEN (saddr, sizeof (struct sockaddr_in));

	saddr->sin_family = AF_INET;
	saddr->sin_port   = htons (atoi (portnum));

	if ((saddr->sin_addr.s_addr = inet_addr (hostname)) == INADDR_NONE) {
	        int i;

		LINC_RESOLV_UNSET_IPV6;
		if (!(_res.options & RES_INIT))
			res_init();
		
		host = gethostbyname (hostname);
		if (!host) {
		  g_free (saddr);
		  return NULL;
		}

		for(i = 0; host->h_addr_list[i]; i++)
		    if(ipv4_addr_from_addr (&saddr->sin_addr,
					    (guint8 *)host->h_addr_list [i],
					    host->h_length))
		      break;

		if(!host->h_addr_list[i]) {
		  g_free (saddr);
		  return NULL;
		}
	}

	return (struct sockaddr *) saddr;
}
#endif
