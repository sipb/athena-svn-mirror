#define _ELOADER_HTTP_C_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include "e-cache.h"
#include "eloader-http.h"

#define ELH_CACHE_SIZE (1024 * 1024)

#define EL_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str);

/* Cache for not yet connected clients */
static GHashTable * clients = NULL;
/* Cache for connected clients */
static ECache * ec = NULL;

static void eloader_http_class_init (GtkObjectClass * klass);
static void eloader_http_init (GtkObject * object);
static void eloader_http_destroy (GtkObject * object);

static gboolean setup_client (ELoaderHTTP * elh);
static gint connect_cached_client (gpointer data);
static void client_destroyed (GtkObject * object, gpointer data);
static void client_connect_main (EHTTPClient * client, gint result, gpointer data);
static void client_connect (EHTTPClient * client, gint result, gpointer data);
static void client_get_data (EHTTPClient * client, gint pos, gint length, gpointer data);
static void client_done (EHTTPClient * client, EHTTPClientStatus status, gpointer data);
static void client_set_status (EHTTPClient * client, const gchar * status, gpointer data);

static ELoaderClass * parent_class;

GtkType
eloader_http_get_type (void)
{
	static GtkType loader_type = 0;
	if (!loader_type) {
		GtkTypeInfo loader_info = {
			"ELoaderHTTP",
			sizeof (ELoaderHTTP),
			sizeof (ELoaderHTTPClass),
			(GtkClassInitFunc) eloader_http_class_init,
			(GtkObjectInitFunc) eloader_http_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		loader_type = gtk_type_unique (eloader_get_type (), &loader_info);
	}
	return loader_type;
}

static void
eloader_http_class_init (GtkObjectClass * klass)
{
	parent_class = gtk_type_class (eloader_get_type ());

	klass->destroy = eloader_http_destroy;
}

static void
eloader_http_init (GtkObject * object)
{
	ELoaderHTTP * elh;

	elh = ELOADER_HTTP (object);

	elh->cache = FALSE;
	elh->url = NULL;
	elh->hops = 0;
	elh->client = NULL;
	elh->iid = 0;
}

static void
eloader_http_destroy (GtkObject * object)
{
	ELoaderHTTP * elh;

	elh = ELOADER_HTTP (object);

	if (elh->iid) {
		gtk_idle_remove (elh->iid);
		elh->iid = 0;
	}

	if (elh->url) {
		g_free (elh->url);
		elh->url = NULL;
	}

	if (elh->client) {
		gtk_object_unref (GTK_OBJECT (elh->client));
		elh->client = 0;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

ELoader *
eloader_http_new_get (EBrowser * ebr, const gchar * url, GtkHTMLStream * stream)
{
	ELoaderHTTP * elh;

	g_return_val_if_fail (ebr != NULL, NULL);
	g_return_val_if_fail (IS_EBROWSER (ebr), NULL);
	g_return_val_if_fail (url != NULL, NULL);

	/* Create object */

	elh = gtk_type_new (ELOADER_HTTP_TYPE);
	eloader_construct (ELOADER (elh), ebr, stream);
	elh->cache = TRUE;
	elh->url = g_strdup (url);

	/* fixme: Process the right way */
	if (setup_client (elh)) {
		return ELOADER (elh);
	} else {
		return NULL;
	}
}

static gboolean
setup_client (ELoaderHTTP * elh)
{
	EHTTPClient * client;
	gpointer data;

	/* Query for connected client */

	if (!ec) ec = e_cache_new (g_str_hash, g_str_equal,
				   (ECacheDupFunc) g_strdup, (ECacheFreeFunc) g_free,
				   (ECacheFreeFunc) gtk_object_unref,
				   ELH_CACHE_SIZE, ELH_CACHE_SIZE);
	data = e_cache_lookup (ec, elh->url);
	if (data) {
		EHTTPClient * client;
		/* We have already connected client */
		/* fixme: test expiring */
		client = E_HTTP_CLIENT (data);
		gtk_object_ref (GTK_OBJECT (client));
		elh->client = client;
		elh->iid = gtk_idle_add (connect_cached_client, elh);
		return TRUE;
	}

	/* Query for connecting client */

	if (!clients) clients = g_hash_table_new (g_str_hash, g_str_equal);
	data = g_hash_table_lookup (clients, elh->url);
	if (data) {
		client = E_HTTP_CLIENT (data);
		e_http_client_ref (client);
	} else {
		client = e_http_client_new_get (elh->url, elh->loader.ebrowser->http_proxy);
		if (!client) {
			eloader_done (ELOADER (elh), ELOADER_ERROR);
			return FALSE;
		}
		g_hash_table_insert (clients, client->url, client);
		/* Client destroy handling */
		gtk_signal_connect (GTK_OBJECT (client), "destroy", GTK_SIGNAL_FUNC (client_destroyed), NULL);
		/* Our cache-managing handler */
		gtk_signal_connect (GTK_OBJECT (client), "connect", GTK_SIGNAL_FUNC (client_connect_main), elh);
	}

#if 0
	/* No, we do not want to ref it here */
	gtk_object_ref (GTK_OBJECT (client));
#endif
	elh->client = client;

	/* As we are not connected, we simply insert handlers here */

	gtk_signal_connect (GTK_OBJECT (client), "set_status", GTK_SIGNAL_FUNC (client_set_status), elh);
	gtk_signal_connect (GTK_OBJECT (client), "connect", GTK_SIGNAL_FUNC (client_connect), elh);
	gtk_signal_connect (GTK_OBJECT (client), "get_data", GTK_SIGNAL_FUNC (client_get_data), elh);
	gtk_signal_connect (GTK_OBJECT (client), "done", GTK_SIGNAL_FUNC (client_done), elh);

	return TRUE;
}

static gint
connect_cached_client (gpointer data)
{
	ELoaderHTTP * elh;
	const gchar * location;
	const gchar * content_type;

	elh = ELOADER_HTTP (data);

	if (elh->client->state < E_HTTP_CLIENT_CONNECTED) {
		/* Simply connect signals and wait */
		gtk_signal_connect (GTK_OBJECT (elh->client), "set_status", GTK_SIGNAL_FUNC (client_set_status), elh);
		gtk_signal_connect (GTK_OBJECT (elh->client), "connect", GTK_SIGNAL_FUNC (client_connect), elh);
		gtk_signal_connect (GTK_OBJECT (elh->client), "get_data", GTK_SIGNAL_FUNC (client_get_data), elh);
		gtk_signal_connect (GTK_OBJECT (elh->client), "done", GTK_SIGNAL_FUNC (client_done), elh);
		elh->iid = 0;
		return FALSE;
	}

	/* We are connected */

	switch (elh->client->result) {
	case 301: /* Moved Permanently */
	case 302: /* Found */
	case 303: /* See Other */
	case 307: /* Temporary Redirect */
		/* Automatic redirection */
		if (elh->hops > 8) break;
		location = e_http_client_get_header (elh->client, "Location");
		if (!location) break;
		/* Be cautious */
		elh->hops++;
		g_free (elh->url);
		elh->url = g_strdup (location);
		gtk_object_unref (GTK_OBJECT (elh->client));
		elh->client = NULL;
		elh->iid = 0;
		/* We do not have signals connected */
		setup_client (elh);
		return FALSE;
		break;
	case 300: /* Multiple Choices */
	case 304: /* Not Modified */
	case 305: /* Use Proxy */
	case 306: /* */
		break;
	default:
		break;
	}

	/* We are connected and not redirected */

	content_type = e_http_client_get_header (elh->client, "Content-Type");

	eloader_connect (ELOADER (elh), elh->url, content_type);

	if (elh->client->pos > 0) {
		/* We have data to write */
		if (elh->loader.stream) {
			gtk_html_stream_write (elh->loader.stream,
					       e_http_client_get_buffer (elh->client),
					       e_http_client_get_position (elh->client));
		}
	}

	if (elh->client->state < E_HTTP_CLIENT_DONE) {
		/* Simply connect signals and wait */
		gtk_signal_connect (GTK_OBJECT (elh->client), "set_status", GTK_SIGNAL_FUNC (client_set_status), elh);
		gtk_signal_connect (GTK_OBJECT (elh->client), "get_data", GTK_SIGNAL_FUNC (client_get_data), elh);
		gtk_signal_connect (GTK_OBJECT (elh->client), "done", GTK_SIGNAL_FUNC (client_done), elh);
		elh->iid = 0;
		return FALSE;
	}

	elh->iid = 0;
	eloader_done (ELOADER (elh), ELOADER_OK);

	return FALSE;
}

static void
client_destroyed (GtkObject * object, gpointer data)
{
	EHTTPClient * client;

	client = E_HTTP_CLIENT (object);

	if (g_hash_table_lookup (clients, client->url) == client) {
		g_hash_table_remove (clients, client->url);
	}
}

static void
client_connect_main (EHTTPClient * client, gint result, gpointer data)
{
	gint len;

	/* Remove from connection cache */

	g_hash_table_remove (clients, client->url);

	/* Add to permanent cache */
	/* Fixme: look results */

	len = e_http_client_get_content_length (client);

	if (len > 0) {
		e_http_client_ref (client);
		if (!e_cache_insert (ec, client->url, client, len)) {
			e_http_client_unref (client);
		}
	}
}

static void
client_connect (EHTTPClient * client, gint result, gpointer data)
{
	ELoaderHTTP * elh;
	const gchar * location;
	const gchar * content_type;

	elh = ELOADER_HTTP (data);

	switch (elh->client->result) {
	case 301: /* Moved Permanently */
	case 302: /* Found */
	case 303: /* See Other */
	case 307: /* Temporary Redirect */
		/* Automatic redirection */
		if (elh->hops > 8) break;
		location = e_http_client_get_header (elh->client, "Location");
		if (!location) break;
		/* Be cautious */
		g_free (elh->url);
		elh->url = g_strdup (location);
		elh->iid = 0;
		/* Disconnect signals (all, except destroy) */
		gtk_signal_disconnect_by_data (GTK_OBJECT (elh->client), elh);
		gtk_object_unref (GTK_OBJECT (elh->client));
		elh->client = NULL;
		setup_client (elh);
		return;
		break;
	case 300: /* Multiple Choices */
	case 304: /* Not Modified */
	case 305: /* Use Proxy */
	case 306: /* */
		break;
	default:
		break;
	}

	/* We are connected and not redirected */

	content_type = e_http_client_get_header (elh->client, "Content-Type");

	eloader_connect (ELOADER (elh), elh->url, content_type);
}

static void
client_get_data (EHTTPClient * client, gint pos, gint length, gpointer data)
{
	ELoaderHTTP * elh;

	elh = ELOADER_HTTP (data);

	if (elh->loader.stream) {
		gtk_html_stream_write (elh->loader.stream, e_http_client_get_buffer (elh->client) + pos, length);
	}
}

static void
client_done (EHTTPClient * client, EHTTPClientStatus status, gpointer data)
{
	ELoaderHTTP * elh;

	elh = ELOADER_HTTP (data);

#if 0
	e_http_client_unref (client);
	elh->client = NULL;
#endif

	eloader_done (ELOADER (elh), (status == E_HTTP_CLIENT_OK) ? ELOADER_OK : ELOADER_ERROR);
}

static void
client_set_status (EHTTPClient * client, const gchar * status, gpointer data)
{
	ELoaderHTTP * elh;

	elh = ELOADER_HTTP (data);

	eloader_set_status (ELOADER (elh), status);
}

ELoader *
eloader_http_new_post (EBrowser * ebr, const gchar * url, const gchar * encoding, GtkHTMLStream * stream)
{
	ELoaderHTTP * elh;
	EHTTPClient * client;

	g_return_val_if_fail (ebr != NULL, NULL);
	g_return_val_if_fail (IS_EBROWSER (ebr), NULL);
	g_return_val_if_fail (url != NULL, NULL);
	g_return_val_if_fail (encoding != NULL, NULL);
	g_return_val_if_fail (stream != NULL, NULL);

	/* Create object */

	elh = gtk_type_new (ELOADER_HTTP_TYPE);
	eloader_construct (ELOADER (elh), ebr, stream);
	elh->cache = FALSE;
	elh->url = g_strdup (url);

	client = e_http_client_new_post (elh->url, encoding, elh->loader.ebrowser->http_proxy);

	if (!client) {
		eloader_done (ELOADER (elh), ELOADER_ERROR);
		gtk_object_unref (GTK_OBJECT (elh));
		return NULL;
	}

#if 0
	gtk_object_ref (GTK_OBJECT (client));
#endif
	elh->client = client;

	/* As we are not connected, we simply insert handlers here */

	gtk_signal_connect (GTK_OBJECT (client), "set_status", GTK_SIGNAL_FUNC (client_set_status), elh);
	gtk_signal_connect (GTK_OBJECT (client), "connect", GTK_SIGNAL_FUNC (client_connect), elh);
	gtk_signal_connect (GTK_OBJECT (client), "get_data", GTK_SIGNAL_FUNC (client_get_data), elh);
	gtk_signal_connect (GTK_OBJECT (client), "done", GTK_SIGNAL_FUNC (client_done), elh);

	return ELOADER (elh);
}

