/* $Id: request.h,v 1.1.1.1 2001-01-16 15:25:56 ghudson Exp $
 *
 * Copyright (C) 2000  Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "sock.h"
#include "session.h"

/*
 * Types
 */
 

/* proxy_tunnel functions */
int socket_connect_proxy_tunnel(Socket *sock, const char *name, int port, gboolean setup_ssl, SocketConnectFn connectfn);

unsigned short proxy_listen (unsigned short port, gpointer user_data, const ProxyCallbackInfo * callbacks, const char *target_path);
void proxy_listen_close (unsigned short port); 

#endif /*_REQUEST_H_*/
