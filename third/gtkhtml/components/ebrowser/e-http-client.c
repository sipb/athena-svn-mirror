#define _E_HTTP_CLIENT_C_

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

#include <stdlib.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include "e-http-client.h"

#define noEHC_VERBOSE
#define noDEBUG_EHC_ALLOC

#ifdef __GNUC__
#define EHC_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str);
#else
#define EHC_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s)\n", __FILE__, __LINE__, str);
#endif

static void e_http_client_class_init (GtkObjectClass * klass);
static void e_http_client_init (GtkObject * object);
static void e_http_client_finalize (GtkObject * object);

static gint ehc_idle (gpointer data);
static void ehc_input (gpointer data, gint fd, GdkInputCondition condition);

static void ehc_headers (EHTTPClient * ehc, ghttp_status status);
static void ehc_body (EHTTPClient * ehc, ghttp_status status);
static void ehc_done (EHTTPClient * ehc, EHTTPClientStatus status);
static void ehc_set_status (EHTTPClient * ehc, const gchar * status);

enum {CONNECT, GET_DATA, DONE, SET_STATUS, LAST_SIGNAL};

static GtkObjectClass * parent_class;
static guint ehc_signals[LAST_SIGNAL] = {0};

#ifdef DEBUG_EHC_ALLOC
static gint ehc_num = 0;
#endif

GtkType
e_http_client_get_type (void)
{
	static GtkType client_type = 0;
	if (!client_type) {
		GtkTypeInfo client_info = {
			"EHTTPClient",
			sizeof (EHTTPClient),
			sizeof (EHTTPClientClass),
			(GtkClassInitFunc) e_http_client_class_init,
			(GtkObjectInitFunc) e_http_client_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		client_type = gtk_type_unique (gtk_object_get_type (), &client_info);
	}
	return client_type;
}

static void
e_http_client_class_init (GtkObjectClass * klass)
{
	parent_class = gtk_type_class (gtk_object_get_type ());

	ehc_signals[CONNECT] = gtk_signal_new ("connect",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EHTTPClientClass, connect),
					       gtk_marshal_NONE__INT,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_INT);
	ehc_signals[GET_DATA] = gtk_signal_new ("get_data",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EHTTPClientClass, get_data),
					       gtk_marshal_NONE__INT_INT,
					       GTK_TYPE_NONE, 2,
					       GTK_TYPE_INT, GTK_TYPE_INT);
	ehc_signals[DONE] = gtk_signal_new ("done",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EHTTPClientClass, done),
					       gtk_marshal_NONE__UINT,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_UINT);
	ehc_signals[SET_STATUS] = gtk_signal_new ("set_status",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EHTTPClientClass, set_status),
					       gtk_marshal_NONE__POINTER,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_POINTER);
	gtk_object_class_add_signals (klass, ehc_signals, LAST_SIGNAL);

	klass->finalize = e_http_client_finalize;
}

static void
e_http_client_init (GtkObject * object)
{
	EHTTPClient * ehc;

	ehc = E_HTTP_CLIENT (object);

	ehc->state = E_HTTP_CLIENT_NONE;
	ehc->url = NULL;
	ehc->result = 0;

	ehc->request = NULL;
	ehc->hops = 0;
	ehc->pos = 0;
	ehc->iid = 0;
	ehc->sid = 0;

#ifdef DEBUG_EHC_ALLOC
	ehc_num++;
	g_print ("EHTTPClients: %d\n", ehc_num);
#endif
}

static void
e_http_client_finalize (GtkObject * object)
{
	EHTTPClient * ehc;

	ehc = E_HTTP_CLIENT (object);

	if (ehc->url) {
		g_free (ehc->url);
		ehc->url = NULL;
	}

	if (ehc->body) {
		g_free (ehc->body);
		ehc->body = NULL;
	}

	if (ehc->iid) {
		gtk_idle_remove (ehc->iid);
		ehc->iid = 0;
	}

	if (ehc->sid) {
		gdk_input_remove (ehc->sid);
		ehc->sid = 0;
	}

	if (ehc->request) {
		ghttp_request_destroy (ehc->request);
		ehc->request = NULL;
		ehc->pos = 0;
	}

#if 0
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
#endif

#ifdef DEBUG_EHC_ALLOC
	ehc_num--;
	g_print ("EHTTPClients: %d\n", ehc_num);
#endif
}

EHTTPClient *
e_http_client_new_get (const gchar * url, const gchar * proxy)
{
	EHTTPClient * ehc;

	g_return_val_if_fail (url != NULL, NULL);

	if (ghttp_uri_validate ((char *) url) == -1) {
		EHC_DEBUG ("Invalid URI", LOADER);
		return NULL;
	}

	ehc = gtk_type_new (E_HTTP_CLIENT_TYPE);

	ehc->url = g_strdup (url);

	/* Setup request */
	ehc->request = ghttp_request_new ();
	ghttp_set_sync (ehc->request, ghttp_async);
	if (proxy) ghttp_set_proxy (ehc->request, (gchar *) proxy);
	ghttp_set_uri (ehc->request, (gchar *) ehc->url);
	ghttp_set_header (ehc->request, http_hdr_Connection, "close");
	ghttp_prepare (ehc->request);

	ehc->iid = gtk_idle_add (ehc_idle, ehc);

	return ehc;
}

EHTTPClient *
e_http_client_new_post (const gchar * url, const gchar * encoding, const gchar * proxy)
{
	EHTTPClient * ehc;

	g_return_val_if_fail (url != NULL, NULL);
	g_return_val_if_fail (encoding != NULL, NULL);

	if (ghttp_uri_validate ((char *) url) == -1) {
		EHC_DEBUG ("Invalid URI", LOADER);
		return NULL;
	}

	ehc = gtk_type_new (E_HTTP_CLIENT_TYPE);

	ehc->url = g_strdup (url);
	ehc->body = g_strdup (encoding);

	ehc->request = ghttp_request_new ();
	ghttp_set_type (ehc->request, ghttp_type_post);
	ghttp_set_header (ehc->request, http_hdr_Content_Type, "application/x-www-form-urlencoded");
	ghttp_set_uri (ehc->request, (gchar *) ehc->url);
	ghttp_set_body (ehc->request, ehc->body, strlen (ehc->body));
	ghttp_prepare (ehc->request);

	ehc->iid = gtk_idle_add (ehc_idle, ehc);

	return ehc;
}

const gchar *
e_http_client_get_header (EHTTPClient * client, const gchar * header)
{
	g_return_val_if_fail (client != NULL, NULL);
	g_return_val_if_fail (E_IS_HTTP_CLIENT (client), NULL);
	g_return_val_if_fail (client->request != NULL, NULL);
	g_return_val_if_fail (client->result >= E_HTTP_CLIENT_CONNECTED, NULL);

	return ghttp_get_header (client->request, header);
}

const gchar *
e_http_client_get_buffer (EHTTPClient * client)
{
	g_return_val_if_fail (client != NULL, NULL);
	g_return_val_if_fail (E_IS_HTTP_CLIENT (client), NULL);
	g_return_val_if_fail (client->request != NULL, NULL);
	g_return_val_if_fail (client->result >= E_HTTP_CLIENT_CONNECTED, NULL);

	return ghttp_get_body (client->request);
}

gint
e_http_client_get_content_length (EHTTPClient * client)
{
	const gchar * str;

	g_return_val_if_fail (client != NULL, -1);
	g_return_val_if_fail (E_IS_HTTP_CLIENT (client), -1);
	g_return_val_if_fail (client->request != NULL, -1);
	g_return_val_if_fail (client->result >= E_HTTP_CLIENT_CONNECTED, -1);

	str = ghttp_get_header (client->request, "Content-Length");
	if (str == NULL) return 0;
	return atoi (str);
}

gint
e_http_client_get_position (EHTTPClient * client)
{
	g_return_val_if_fail (client != NULL, -1);
	g_return_val_if_fail (E_IS_HTTP_CLIENT (client), -1);
	g_return_val_if_fail (client->request != NULL, -1);
	g_return_val_if_fail (client->result >= E_HTTP_CLIENT_CONNECTED, -1);

	return client->pos;
}

static gint
ehc_idle (gpointer data)
{
	EHTTPClient * ehc;
	ghttp_status status;
	ghttp_current_status current;
	gchar * str;

	ehc = E_HTTP_CLIENT (data);

	/* Process request */

	status = ghttp_process (ehc->request);

	switch (status) {
	case ghttp_error:
		EHC_DEBUG ("Error", LOADER);
		str = g_strdup_printf ("Error contacting %s", ehc->url);
		ehc_set_status (ehc, str);
		g_free (str);
		ehc->iid = 0;
		ehc_done (ehc, E_HTTP_CLIENT_ERROR);
		return FALSE;
		break;
	case ghttp_done:
		/* We have completed request */
		ehc_headers (ehc, status);
		ehc_body (ehc, status);
		ehc_done (ehc, E_HTTP_CLIENT_OK);
		return FALSE;
		break;
	case ghttp_not_done:
		/* test, whether we have succeded connecting to destination */
		if (ghttp_get_socket (ehc->request) < 0) {
			/* We have not yet opened socket, so keep idle recurring */
			return TRUE;
		}
		current = ghttp_get_status (ehc->request);
		switch (current.proc) {
		case ghttp_proc_request:
			/* We are still sending request */
			str = g_strdup_printf ("Contacting %s", ehc->url);
			ehc_set_status (ehc, str);
			g_free (str);
			return TRUE;
			break;
		case ghttp_proc_response:
			/* We have already got headers */
			ehc_headers (ehc, status);
			ehc_body (ehc, status);
		case ghttp_proc_response_hdrs:
			/* Request is sent - so switch to gdk_input */
			ehc->iid = 0;
			ehc->sid = gdk_input_add (ghttp_get_socket (ehc->request),
						  GDK_INPUT_READ | GDK_INPUT_EXCEPTION,
						  ehc_input, ehc);
			return FALSE;
			break;
		case ghttp_proc_none:
			g_warning ("Hmmm... processing 'none' while request is not done");
			return FALSE;
		default:
			g_assert_not_reached ();
			break;
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}
	ehc->iid = 0;
	return FALSE;
}

/*
 * We have sent request, and are waiting for input
 */

static void
ehc_input (gpointer data, gint fd, GdkInputCondition condition)
{
	EHTTPClient * ehc;
	ghttp_status status;
	ghttp_current_status current;
	gchar * str;

	ehc = E_HTTP_CLIENT (data);

	status = ghttp_process (ehc->request);

	switch (status) {
	case ghttp_error:
		EHC_DEBUG ("Error", LOADER);
		str = g_strdup_printf ("Error connecting %s", ehc->url);
		ehc_set_status (ehc, str);
		g_free (str);
		ehc_done (ehc, E_HTTP_CLIENT_ERROR);
		return;
		break;
	case ghttp_done:
		/* We have completed request */
		ehc_headers (ehc, status);
		ehc_body (ehc, status);
		ehc_done (ehc, E_HTTP_CLIENT_OK);
		return;
		break;
	case ghttp_not_done:
		current = ghttp_get_status (ehc->request);
		switch (current.proc) {
		case ghttp_proc_request:
			g_assert_not_reached ();
			break;
		case ghttp_proc_response:
			/* We have already got headers */
			ehc_headers (ehc, status);
			ehc_body (ehc, status);
		case ghttp_proc_response_hdrs:
			return;
			break;
		case ghttp_proc_none:
			g_warning ("Hmmm... processing 'none' while request is not done");
			return;
			break;
		default:
			g_assert_not_reached ();
			break;
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
ehc_headers (EHTTPClient * ehc, ghttp_status status)
{
	gchar * str;

	/* If we are already connected, simply return */
	if (ehc->state >= E_HTTP_CLIENT_CONNECTED) return;

	ehc->result = ghttp_status_code (ehc->request);

#ifdef EHC_VERBOSE
	g_print ("HTTP status code %d\n", ehc->result);
#endif

	str = g_strdup_printf ("Connected %s [%d]", ehc->url, ehc->result);
	ehc_set_status (ehc, str);
	g_free (str);

	ehc->state = E_HTTP_CLIENT_CONNECTED;

	e_http_client_ref (ehc);
	gtk_signal_emit (GTK_OBJECT (ehc), ehc_signals[CONNECT], ehc->result);
	e_http_client_unref (ehc);
}

static void
ehc_body (EHTTPClient * ehc, ghttp_status status)
{
	char * body;
	gint len;
	gchar * str;

	g_assert (ehc->state >= E_HTTP_CLIENT_CONNECTED);
	/* If we are already connected, simply return */
	if (ehc->state >= E_HTTP_CLIENT_DONE) return;

	body = ghttp_get_body (ehc->request);
	len = ghttp_get_body_len (ehc->request);

	if (len > ehc->pos) {
		gint pos;
		str = g_strdup_printf ("Read %d bytes from %s", len - ehc->pos, ehc->url);
		ehc_set_status (ehc, str);
		g_free (str);
		pos = ehc->pos;
		ehc->pos = len;
		e_http_client_ref (ehc);
		gtk_signal_emit (GTK_OBJECT (ehc), ehc_signals[GET_DATA], pos, len - pos);
		e_http_client_unref (ehc);
	}

	if (status == ghttp_done) {
		/* fixme: Should go to ehc_done? */
		ehc->state = E_HTTP_CLIENT_DONE;
	}
}

static void
ehc_done (EHTTPClient * ehc, EHTTPClientStatus status)
{
	/* We have to clear iid, in case we are invoked from idle */
	ehc->iid = 0;
	if (ehc->sid) {
		gdk_input_remove (ehc->sid);
		ehc->sid = 0;
	}

	e_http_client_ref (ehc);
	gtk_signal_emit (GTK_OBJECT (ehc), ehc_signals[DONE], status);
	e_http_client_unref (ehc);
}

static void
ehc_set_status (EHTTPClient * ehc, const gchar * status)
{
	e_http_client_ref (ehc);
	gtk_signal_emit (GTK_OBJECT (ehc), ehc_signals[SET_STATUS], status);
	e_http_client_unref (ehc);
}



