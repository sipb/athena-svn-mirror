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


#ifndef _GNET_INETADDR_H
#define _GNET_INETADDR_H

#include <glib.h>

#ifdef   GNET_WIN32
#include <winsock2.h>	/* This needs to be here */
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*

  All fields in GInetAddr are private and should be accessed only by
  using the functions below.

 */
typedef struct _GInetAddr GInetAddr;




/* ********** */


/**
 *   GInetAddrAsyncStatus:
 * 
 *   Status of a asynchronous lookup (from gnet_inetaddr_new_async()
 *   or gnet_inetaddr_get_name_async()), passed by
 *   GInetAddrNewAsyncFunc or GInetAddrGetNameAsyncFunc.  More errors
 *   may be added in the future, so it's best to compare against
 *   %GINETADDR_ASYNC_STATUS_OK.
 *
 **/
typedef enum {
  GINETADDR_ASYNC_STATUS_OK,
  GINETADDR_ASYNC_STATUS_ERROR
} GInetAddrAsyncStatus;



/**
 *   GInetAddrNewAsyncID:
 * 
 *   ID of an asynchronous InetAddr creation started with
 *   gnet_inetaddr_new_async().  The creation can be canceled by
 *   calling gnet_inetaddr_new_async_cancel() with the ID.
 *
 **/
typedef gpointer GInetAddrNewAsyncID;



/**
 *   GInetAddrNewAsyncFunc:
 *   @inetaddr: InetAddr that was looked up
 *   @status: Status of the lookup
 *   @data: User data
 *   
 *   Callback for gnet_inetaddr_new_async().
 *
 **/
typedef void (*GInetAddrNewAsyncFunc)(GInetAddr* inetaddr, 
				      GInetAddrAsyncStatus status, 
				      gpointer data);



/**
 *   GInetAddrGetNameAsyncID:
 * 
 *   ID of an asynchronous InetAddr name lookup started with
 *   gnet_inetaddr_get_name_async().  The lookup can be canceled by
 *   calling gnet_inetaddr_get_name_async_cancel() with the ID.
 *
 **/
typedef gpointer GInetAddrGetNameAsyncID;



/**
 *   GInetAddrGetNameAsyncFunc:
 *   @inetaddr: InetAddr whose was looked up
 *   @status: Status of the lookup
 *   @name: Nice name of the address
 *   @data: User data
 *   
 *   Callback for gnet_inetaddr_get_name_async().  Delete the name
 *   when you're done with it.
 *
 **/
typedef void (*GInetAddrGetNameAsyncFunc)(GInetAddr* inetaddr, 
					  GInetAddrAsyncStatus status, 
					  gchar* name,
					  gpointer data);




/* ********** */

GInetAddr* gnet_inetaddr_new (const gchar* name, gint port);

GInetAddrNewAsyncID 
gnet_inetaddr_new_async (const gchar* name, gint port, 
			 GInetAddrNewAsyncFunc func, gpointer data);

void       gnet_inetaddr_new_async_cancel (GInetAddrNewAsyncID id);

GInetAddr* gnet_inetaddr_new_nonblock (const gchar* name, gint port);

GInetAddr* gnet_inetaddr_clone (const GInetAddr* ia);

void       gnet_inetaddr_delete (GInetAddr* ia);

void gnet_inetaddr_ref (GInetAddr* ia);
void gnet_inetaddr_unref (GInetAddr* ia);


/* ********** */

gchar* gnet_inetaddr_get_name (/* const */ GInetAddr* ia);

GInetAddrGetNameAsyncID
gnet_inetaddr_get_name_async (GInetAddr* ia, 
			      GInetAddrGetNameAsyncFunc func,
			      gpointer data);

void gnet_inetaddr_get_name_async_cancel (GInetAddrGetNameAsyncID id);

gchar* gnet_inetaddr_get_canonical_name (const GInetAddr* ia);

gint gnet_inetaddr_get_port (const GInetAddr* ia);

void gnet_inetaddr_set_port (const GInetAddr* ia, guint port);


/* ********** */

gboolean gnet_inetaddr_is_canonical (const gchar* name);

gboolean gnet_inetaddr_is_internet  (const GInetAddr* inetaddr);
gboolean gnet_inetaddr_is_private   (const GInetAddr* inetaddr);
gboolean gnet_inetaddr_is_reserved  (const GInetAddr* inetaddr);
gboolean gnet_inetaddr_is_loopback  (const GInetAddr* inetaddr);
gboolean gnet_inetaddr_is_multicast (const GInetAddr* inetaddr);
gboolean gnet_inetaddr_is_broadcast (const GInetAddr* inetaddr);


/* ********** */

guint gnet_inetaddr_hash (gconstpointer p);
gint  gnet_inetaddr_equal (gconstpointer p1, gconstpointer p2);
gint  gnet_inetaddr_noport_equal (gconstpointer p1, gconstpointer p2);


/* ********** */

gchar*     gnet_inetaddr_gethostname (void);
GInetAddr* gnet_inetaddr_gethostaddr (void);


/* ********** */

GInetAddr* gnet_inetaddr_new_any (void);
GInetAddr* gnet_inetaddr_autodetect_internet_interface (void);
GInetAddr* gnet_inetaddr_get_interface_to (const GInetAddr* addr);
GInetAddr* gnet_inetaddr_get_internet_interface (void);

gboolean   gnet_inetaddr_is_internet_domainname (const gchar* name);


/* ********** */

GList* gnet_inetaddr_list_interfaces (void);


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /* _GNET_INETADDR_H */
