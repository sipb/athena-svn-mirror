/* GNet - Networking library
 * Copyright (C) 2001  Marius Eriksen, David Helder
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

#include "gnet-private.h"
#include "socks.h"
#include "socks-private.h"


int
gnet_private_negotiate_socks_server (GTcpSocket *s, const GInetAddr *dst)
{
  GIOChannel *ioc;
  int ver, ret;
  const char *verc;

  if ((verc = g_getenv("SOCKS_VERSION"))) 
    ver = atoi(verc);
  else
    ver = GNET_DEFAULT_SOCKS_VERSION;

  ioc = gnet_tcp_socket_get_iochannel(s);
  if (ver == 5)
    ret = gnet_private_negotiate_socks5 (ioc, dst);
  else if (ver == 4)
    ret = gnet_private_negotiate_socks4 (ioc, dst);
  else
    ret = -1;
  g_io_channel_unref(ioc);

  return ret;
}


int
gnet_private_negotiate_socks5 (GIOChannel *ioc, const GInetAddr *dst)
{
  unsigned char s5r[3];
  struct socks5_h s5h;
  struct sockaddr_in *sa_in;
  int len;

  s5r[0] = 5;
  s5r[1] = 1;	/* XXX no authentication yet */
  s5r[2] = 0;	

  if (gnet_io_channel_writen(ioc, s5r, 3, &len) != G_IO_ERROR_NONE)
    return -1;
  if (gnet_io_channel_readn(ioc, s5r, 2, &len) != G_IO_ERROR_NONE)
    return -1;
  if ((s5r[0] != 5) || (s5r[1] != 0))
    return -1;
	
  sa_in = (struct sockaddr_in*)&dst->sa;

  /* fill in SOCKS5 request */
  s5h.vn    = 5;
  s5h.cd    = 1;
  s5h.rsv   = 0;
  s5h.atyp  = 1;
  s5h.dip   = (long)sa_in->sin_addr.s_addr; 
  s5h.dport = (short)sa_in->sin_port;

  if (gnet_io_channel_writen(ioc, (gchar*)&s5h, 10, &len) != G_IO_ERROR_NONE)
    return -1;
  if (gnet_io_channel_readn(ioc, (gchar*)&s5h, 10, &len) != G_IO_ERROR_NONE)
    return -1;
  if (s5h.cd != 0)
    return -1;

  return 0;
}


int
gnet_private_negotiate_socks4 (GIOChannel *ioc, const GInetAddr *dst)
{
  struct socks4_h s4h;
  struct sockaddr_in *sa_in;
  int len;

  sa_in = (struct sockaddr_in*) &dst->sa;

  s4h.vn     = 4;
  s4h.cd     = 1;
  s4h.dport  = (short) sa_in->sin_port;
  s4h.dip    = (long) sa_in->sin_addr.s_addr;
  s4h.userid = 0;

  if (gnet_io_channel_writen(ioc, &s4h, 9, &len) != G_IO_ERROR_NONE)
    return -1;
  if (gnet_io_channel_readn(ioc, &s4h, 8, &len) != G_IO_ERROR_NONE)
    return -1;

  if ((s4h.cd != 90) || (s4h.vn != 0))
    return -1;

  return 0;
}
