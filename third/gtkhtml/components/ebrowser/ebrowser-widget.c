#define _EBROWSER_WIDGET_C_

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

#include <string.h>
#include "eloader-http.h"
#include "eloader-file.h"
#include "eloader-moniker.h"
#include "ebrowser-widget.h"

enum {ARG_0, ARG_HTTP_PROXY, ARG_URL, ARG_FOLLOW_LINKS, ARG_FOLLOW_REDIRECT, ARG_ALLOW_SUBMIT, ARG_DEFAULT_BGCOLOR, ARG_DEFAULT_FONT, ARG_HISTORY_SIZE};

static void ebrowser_class_init (GtkObjectClass * klass);
static void ebrowser_init (GtkObject * object);
static void ebrowser_destroy (GtkObject * object);
static void ebrowser_set_arg (GtkObject * object, GtkArg * arg, guint arg_id);

static void ebrowser_title_changed (GtkHTML * html, const gchar * title);
static void ebrowser_url_requested (GtkHTML * html, const gchar * url, GtkHTMLStream * handle);
static void ebrowser_load_done (GtkHTML * html);
static void ebrowser_link_clicked (GtkHTML * html, const gchar * url);
static void ebrowser_set_base (GtkHTML * html, const gchar * base);
static void ebrowser_set_base_target (GtkHTML * html, const gchar * base);
static void ebrowser_on_url (GtkHTML * html, const gchar * url);
static void ebrowser_redirect (GtkHTML * html, const gchar * url, int delay);
static void ebrowser_submit (GtkHTML * html, const gchar * method, const gchar * url, const gchar * encoding);

/* Helpers */

static void ebrowser_load (EBrowser * ebr, const gchar * uri);
static void ebrowser_stop_loading (EBrowser * ebr);
static void ebrowser_loader_done (ELoader * el, ELoaderStatus status, gpointer data);
static void ebrowser_loader_set_status (ELoader * el, const gchar * status, gpointer data);
static void ebrowser_body_connect (ELoader * el, const gchar * url, const gchar * content_type, gpointer data);
static void ebrowser_status_set (EBrowser * ebr, const gchar * status);

static EBrowserProtocol ebrowser_find_protocol (const gchar * uri, const gchar ** location);
static gboolean ebrowser_http_base (const gchar * location, gchar ** root, gchar ** dir);
static gboolean ebrowser_file_base (const gchar * location, gchar ** root, gchar ** dir);

static gchar * ebrowser_concat (const gchar * url, const gchar * relative);

enum {URL_SET, STATUS_SET, REQUEST, DONE, LAST_SIGNAL};

static GtkHTMLClass * parent_class;
static guint ebr_signals[LAST_SIGNAL] = {0};

GtkType
ebrowser_get_type (void)
{
	static GtkType ebrowser_type = 0;
	if (!ebrowser_type) {
		GtkTypeInfo ebrowser_info = {
			"EBrowser",
			sizeof (EBrowser),
			sizeof (EBrowserClass),
			(GtkClassInitFunc) ebrowser_class_init,
			(GtkObjectInitFunc) ebrowser_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		ebrowser_type = gtk_type_unique (gtk_html_get_type (), &ebrowser_info);
	}
	return ebrowser_type;
}

static void
ebrowser_class_init (GtkObjectClass * klass)
{
	GtkHTMLClass * html_class;

	html_class = GTK_HTML_CLASS (klass);

	parent_class = gtk_type_class (gtk_html_get_type ());

	gtk_object_add_arg_type ("EBrowser::url", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_URL);
	gtk_object_add_arg_type ("EBrowser::http_proxy", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_HTTP_PROXY);
	gtk_object_add_arg_type ("EBrowser::follow_links", GTK_TYPE_BOOL, GTK_ARG_WRITABLE, ARG_FOLLOW_LINKS);
	gtk_object_add_arg_type ("EBrowser::follow_redirect", GTK_TYPE_BOOL, GTK_ARG_WRITABLE, ARG_FOLLOW_REDIRECT);
	gtk_object_add_arg_type ("EBrowser::allow_submit", GTK_TYPE_BOOL, GTK_ARG_WRITABLE, ARG_ALLOW_SUBMIT);
	gtk_object_add_arg_type ("EBrowser::default_bgcolor", GTK_TYPE_UINT, GTK_ARG_WRITABLE, ARG_DEFAULT_BGCOLOR);
	gtk_object_add_arg_type ("EBrowser::default_font", GTK_TYPE_STRING, GTK_ARG_WRITABLE, ARG_DEFAULT_FONT);
	gtk_object_add_arg_type ("EBrowser::history_size", GTK_TYPE_UINT, GTK_ARG_WRITABLE, ARG_HISTORY_SIZE);

	ebr_signals[URL_SET] = gtk_signal_new ("url_set",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EBrowserClass, url_set),
					       gtk_marshal_NONE__POINTER,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_POINTER);
	ebr_signals[STATUS_SET] = gtk_signal_new ("status_set",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EBrowserClass, status_set),
					       gtk_marshal_NONE__POINTER,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_POINTER);
	ebr_signals[REQUEST] = gtk_signal_new ("request",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EBrowserClass, request),
					       gtk_marshal_NONE__POINTER,
					       GTK_TYPE_NONE, 1,
					       GTK_TYPE_POINTER);
	ebr_signals[DONE] = gtk_signal_new    ("done",
					       GTK_RUN_FIRST,
					       klass->type,
					       GTK_SIGNAL_OFFSET (EBrowserClass, done),
					       gtk_marshal_NONE__NONE,
					       GTK_TYPE_NONE, 0);
	gtk_object_class_add_signals (klass, ebr_signals, LAST_SIGNAL);

	klass->destroy = ebrowser_destroy;
	klass->set_arg = ebrowser_set_arg;

	html_class->title_changed = ebrowser_title_changed;
	html_class->url_requested = ebrowser_url_requested;
	html_class->load_done = ebrowser_load_done;
	html_class->link_clicked = ebrowser_link_clicked;
	html_class->set_base = ebrowser_set_base;
	html_class->set_base_target = ebrowser_set_base_target;
	html_class->on_url = ebrowser_on_url;
	html_class->redirect = ebrowser_redirect;
	html_class->submit = ebrowser_submit;
}

static void
ebrowser_init (GtkObject * object)
{
	EBrowser * ebr;

	ebr = EBROWSER (object);

	ebr->url = NULL;
	ebr->http_proxy = NULL;
	ebr->baseroot = NULL;
	ebr->basedir = NULL;
	ebr->baseprotocol = EBROWSER_PROTOCOL_UNKNOWN;
	ebr->followlinks = FALSE;
	ebr->followredirect = FALSE;
	ebr->allowsubmit = FALSE;
	ebr->defaultbgcolor = 0xffffffff;
	ebr->defaultfont = NULL;

	ebr->loaders = NULL;

	ebr->history_size = 50;
	
	ebr->history = ebrowser_history_new (ebr->history_size);
}

static void
ebrowser_destroy (GtkObject * object)
{
	EBrowser * ebr;

	ebr = EBROWSER (object);

	ebrowser_stop_loading (ebr);

	if (ebr->url) {
		g_free (ebr->url);
		ebr->url = NULL;
	}
	if (ebr->http_proxy) {
		g_free (ebr->http_proxy);
		ebr->http_proxy = NULL;
	}
	if (ebr->baseroot) {
		g_free (ebr->baseroot);
		ebr->baseroot = NULL;
	}
	if (ebr->basedir) {
		g_free (ebr->basedir);
		ebr->basedir = NULL;
	}
	if (ebr->defaultfont) {
		g_free (ebr->defaultfont);
		ebr->defaultfont = NULL;
	}
	if (ebr->history){
		ebrowser_history_destroy (ebr->history);
		ebr->history = NULL;
	}
	
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
ebrowser_set_arg (GtkObject * object, GtkArg * arg, guint arg_id)
{
	EBrowser * ebr;
	gchar * str;

	ebr = EBROWSER (object);

	switch (arg_id) {
	case ARG_URL:
		ebrowser_load (ebr, GTK_VALUE_STRING (* arg));
		break;
	case ARG_HTTP_PROXY:
		if (ebr->http_proxy) {
			g_free (ebr->http_proxy);
			ebr->http_proxy = NULL;
		}
		str = GTK_VALUE_STRING (* arg);
		if (str) {
			ebr->http_proxy = g_strdup (str);
		}
		break;
	case ARG_FOLLOW_LINKS:
		ebr->followlinks = GTK_VALUE_BOOL (* arg);
		break;
	case ARG_FOLLOW_REDIRECT:
		ebr->followredirect = GTK_VALUE_BOOL (* arg);
		break;
	case ARG_ALLOW_SUBMIT:
		ebr->allowsubmit = GTK_VALUE_BOOL (* arg);
		break;
	case ARG_DEFAULT_BGCOLOR:
		ebr->defaultbgcolor = GTK_VALUE_UINT (* arg);
		break;
	case ARG_DEFAULT_FONT:
		if (ebr->defaultfont) {
			g_free (ebr->defaultfont);
			ebr->defaultfont = NULL;
		}
		str = GTK_VALUE_STRING (* arg);
		if (str) {
			ebr->defaultfont = g_strdup (str);
		}
		break;

	case ARG_HISTORY_SIZE:
		ebr->history_size = GTK_VALUE_UINT (*arg);
		if (ebr->history_size == 0){
			if (ebr->history){
				ebrowser_history_destroy (ebr->history);
			}
		} else {
			if (!ebr->history){
				ebr->history = ebrowser_history_new (ebr->history_size);
			} else
				ebrowser_history_set_size (ebr->history, ebr->history_size);
		}
		break;
		
	default:
		g_assert_not_reached ();
		break;
	}
}

GtkWidget *
ebrowser_new (void)
{
	EBrowser * ebrowser;

	ebrowser = gtk_type_new (EBROWSER_TYPE);
	gtk_html_construct (GTK_WIDGET (ebrowser));
	gtk_html_load_empty (GTK_HTML (ebrowser));

	return GTK_WIDGET (ebrowser);
}

static void
ebrowser_register_loader (EBrowser * ebr, ELoader *loader)
{
	g_assert (ebr != NULL);
	g_assert (loader != NULL);
      
	ebr->loaders = g_slist_prepend (ebr->loaders, loader);
	gtk_signal_connect (GTK_OBJECT (loader), "done",
			    GTK_SIGNAL_FUNC (ebrowser_loader_done), ebr);
	gtk_signal_connect (GTK_OBJECT (loader), "set_status",
			    GTK_SIGNAL_FUNC (ebrowser_loader_set_status), ebr);
}

static void
ebrowser_unregister_loader (EBrowser * ebr, ELoader *loader)
{
	ebr->loaders = g_slist_remove (ebr->loaders, loader);

	/*
	 * If this becomes NULL, then it means we are done loading the page
	 */
	if (ebr->loaders == NULL)
		gtk_signal_emit (GTK_OBJECT (ebr), ebr_signals [DONE]);
}

void
ebrowser_stop (EBrowser * ebrowser)
{
	ebrowser_stop_loading (ebrowser);
}

static void
ebrowser_title_changed (GtkHTML * html, const gchar * title)
{
	g_print ("title_changed: %s\n", title);
}

static void
ebrowser_url_requested (GtkHTML * html, const gchar * url, GtkHTMLStream * handle)
{
	EBrowser * ebr;
	EBrowserProtocol proto;
	const gchar * location;
	gchar * full;
	ELoader * el;

	ebr = EBROWSER (html);

	g_print ("url_requested: %s handle: %p\n", url, handle);

	el = NULL;

	proto = ebrowser_find_protocol (url, &location);

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
		el = eloader_http_new_get (ebr, url, handle);
		break;
	case EBROWSER_PROTOCOL_FILE:
		el = eloader_file_new (ebr, location, handle);
		break;
	case EBROWSER_PROTOCOL_RELATIVE:
		if (*location == '/') {
			if (ebr->baseroot) {
				full = g_strconcat (ebr->baseroot, location + 1, NULL);
			} else {
				full = g_strdup (location);
			}
		} else {
			full = ebrowser_concat (ebr->url, url);
		}
		if (ebr->baseprotocol == EBROWSER_PROTOCOL_HTTP) {
			el = eloader_http_new_get (ebr, full, handle);
		} if (ebr->baseprotocol == EBROWSER_PROTOCOL_FILE) {
			/* fixme: Do paths correctly */
			el = eloader_file_new (ebr, full + 5, handle);
		}
		g_free (full);
		break;
	case EBROWSER_PROTOCOL_INTERNAL:
		/* We are unsing hacked main page, so find saved eloader */
		el = gtk_object_get_data (GTK_OBJECT (ebr), "InternalLoader");
		if (el) {
			/* fixme: This is hackish - can we be sure that streams are processed immediately? */
			eloader_set_stream (el, handle);
			return;
		}
	case EBROWSER_PROTOCOL_UNKNOWN:
		el = eloader_moniker_new (ebr, location, handle);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	if (el) 
		ebrowser_register_loader (ebr, el);
}

static void
ebrowser_load_done (GtkHTML * html)
{
	g_print ("load_done\n");
}

static void
ebrowser_link_clicked (GtkHTML * html, const gchar * url)
{
	EBrowser * ebr;
	EBrowserProtocol proto;
	const gchar * location;
	gchar * full;

	g_print ("link clicked: %s\n", url);

	ebr = EBROWSER (html);

	if (!ebr->followlinks) return;

	proto = ebrowser_find_protocol (url, &location);

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
	case EBROWSER_PROTOCOL_FILE:
	case EBROWSER_PROTOCOL_UNKNOWN:
		ebrowser_load (ebr, url);
		return;
		break;
	case EBROWSER_PROTOCOL_RELATIVE:
		if (*location == '/') {
			if (ebr->baseroot) {
				full = g_strconcat (ebr->baseroot, location + 1, NULL);
			} else {
				full = g_strdup (location);
			}
		} else {
			full = ebrowser_concat (ebr->url, url);
		}
		ebrowser_load (ebr, full);
		g_free (full);
		return;
		break;
	case EBROWSER_PROTOCOL_INTERNAL:
		gtk_signal_emit (GTK_OBJECT (ebr), ebr_signals[REQUEST], url);
		return;
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

static void
ebrowser_set_base (GtkHTML * html, const gchar * base)
{
	g_print ("set_base: %s\n", base);
}

static void
ebrowser_set_base_target (GtkHTML * html, const gchar * base)
{
	g_print ("set_base_target: %s\n", base);
}

static void
ebrowser_on_url (GtkHTML * html, const gchar * url)
{
	EBrowser * ebr;
	gchar * str;

	if (!url) return;

	ebr = EBROWSER (html);

	str = g_strdup_printf ("%s%s", ebr->followlinks ? "" : "[Blocked] ", url);
	ebrowser_status_set (ebr, str);
	g_free (str);
}

static void
ebrowser_redirect (GtkHTML * html, const gchar * url, int delay)
{
	g_print ("redirect: %s %d\n", url, delay);
}

static void
ebrowser_submit (GtkHTML * html, const gchar * method, const gchar * url, const gchar * encoding)
{
	EBrowser * ebr;
	EBrowserProtocol proto;
	const gchar * location;
	gchar * full;

	g_print ("submit: %s %s %s\n", method, url, encoding);

	ebr = EBROWSER (html);

	if (!ebr->allowsubmit) return;

	proto = ebrowser_find_protocol (url, &location);
	full = NULL;

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
		full = g_strdup (url);
		break;
	case EBROWSER_PROTOCOL_RELATIVE:
		if (ebr->baseprotocol == EBROWSER_PROTOCOL_HTTP) {
			if (*location == '/') {
				if (ebr->baseroot) {
					full = g_strconcat (ebr->baseroot, location + 1, NULL);
				} else {
					return;
				}
			} else {
				full = ebrowser_concat (ebr->url, url);
			}
			break;
		}
		return;
		break;
	case EBROWSER_PROTOCOL_FILE:
	case EBROWSER_PROTOCOL_INTERNAL:
	case EBROWSER_PROTOCOL_UNKNOWN:
		return;
		break;
	default:
		g_assert_not_reached ();
		return;
		break;
	}
	/* fixme: */
	{
		ELoader * el;
		GtkHTMLStream * stream;
		stream = gtk_html_begin (GTK_HTML (ebr));
		el = eloader_http_new_post (ebr, full, encoding, stream);

		if (el) {
			ebrowser_register_loader (ebr, el);
			gtk_signal_connect (GTK_OBJECT (el), "connect",
					    GTK_SIGNAL_FUNC (ebrowser_body_connect), ebr);
		}
	}
	g_free (full);
}

/*
 * Helpers
 */

/*
 * ebrowser_load
 *
 * Requests loading main page
 * We do not invalidate old page now, but instead in "connect" handler
 */

static void
ebrowser_load (EBrowser * ebr, const gchar * uri)
{
	EBrowserProtocol proto;
	const gchar * location;
	gchar * new = NULL;
	ELoader * el;

	if (uri) {
		new = g_strdup (uri);
	} else {
		/* Clean page */

		ebrowser_stop_loading (ebr);

		if (ebr->url) {
			g_free (ebr->url);
			ebr->url = NULL;
		}

		if (ebr->baseroot) {
			g_free (ebr->baseroot);
			ebr->baseroot = NULL;
		}

		if (ebr->basedir) {
			g_free (ebr->basedir);
			ebr->basedir = NULL;
		}

		ebr->baseprotocol = EBROWSER_PROTOCOL_UNKNOWN;

		gtk_html_load_empty (GTK_HTML (ebr));
		return;
	}

	el = NULL;
	ebr->url = new;

	proto = ebrowser_find_protocol (new, &location);

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
		if (!ebrowser_http_base (location, &ebr->baseroot, &ebr->basedir)) return;
		ebr->baseprotocol = proto;
#if 0
		stream = gtk_html_begin (GTK_HTML (ebr));
#endif
		el = eloader_http_new_get (ebr, new, NULL);
		break;
	case EBROWSER_PROTOCOL_FILE:
		if (!ebrowser_file_base (location, &ebr->baseroot, &ebr->basedir)) return;
		ebr->baseprotocol = proto;
#if 0
		stream = gtk_html_begin (GTK_HTML (ebr));
#endif
		el = eloader_file_new (ebr, location, NULL);
		break;
	case EBROWSER_PROTOCOL_RELATIVE:
		if (!ebrowser_file_base (location, &ebr->baseroot, &ebr->basedir)) return;
		ebr->url = g_strdup_printf ("file:%s", new);
		ebr->baseprotocol = EBROWSER_PROTOCOL_FILE;
#if 0
		stream = gtk_html_begin (GTK_HTML (ebr));
#endif
		el = eloader_file_new (ebr, location, NULL);
		g_free (new);
		break;
	case EBROWSER_PROTOCOL_INTERNAL:
		break;
	case EBROWSER_PROTOCOL_UNKNOWN:
		ebr->baseprotocol = proto;
		el = eloader_moniker_new (ebr, location, NULL);
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	if (el) {
		ebrowser_register_loader (ebr, el);
		
		gtk_signal_connect (GTK_OBJECT (el), "connect",
				    GTK_SIGNAL_FUNC (ebrowser_body_connect), ebr);
	}
}

static void
ebrowser_stop_loading (EBrowser * ebr)
{
	while (ebr->loaders) {
		ELoader *loader = ebr->loaders->data;
		
		gtk_object_unref (GTK_OBJECT (loader));
		ebr->loaders = g_slist_remove (ebr->loaders, loader);
	}
}

static void
ebrowser_loader_done (ELoader * el, ELoaderStatus status, gpointer data)
{
	EBrowser * ebr;

	ebr = EBROWSER (data);
	
	ebrowser_unregister_loader (ebr, el);
	gtk_object_unref (GTK_OBJECT (el));
}

static void
ebrowser_loader_set_status (ELoader * el, const gchar * status, gpointer data)
{
	ebrowser_status_set (EBROWSER (data), status);
}

static void
ebrowser_status_set (EBrowser * ebr, const gchar * status)
{
	gtk_signal_emit (GTK_OBJECT (ebr), ebr_signals[STATUS_SET], status);
}

/*
 * fixme: handle all cases more nicely
 */

static void
ebrowser_body_connect (ELoader * el, const gchar * url, const gchar * content_type, gpointer data)
{
	EBrowser * ebr;
	EBrowserProtocol proto;
	const gchar * location;

	ebr = EBROWSER (data);

	if (ebr->url) {
		g_free (ebr->url);
		ebr->url = NULL;
	}

	if (ebr->baseroot) {
		g_free (ebr->baseroot);
		ebr->baseroot = NULL;
	}

	if (ebr->basedir) {
		g_free (ebr->basedir);
		ebr->basedir = NULL;
	}

	proto = ebrowser_find_protocol (url, &location);

	if (strncmp (content_type, "text/html", 9) == 0) {
		GtkHTMLStream * stream;
		/* We are std HTML, so give main stream to loader */
		stream = gtk_html_begin (GTK_HTML (ebr));
		eloader_set_stream (el, stream);
	} else {
		/* We are not the simplest case */
		if ((strncmp (content_type, "image/jpeg", 10) == 0) ||
		    (strncmp (content_type, "image/x-bmp", 11) == 0) ||
		    (strncmp (content_type, "image/x-png", 11) == 0) ||
		    (strncmp (content_type, "image/x-pixmap", 14) == 0) ||
		    (strncmp (content_type, "image/gif", 9) == 0)) {
			GtkHTMLStream * stream;
			gchar * str = g_strdup_printf ("<html><head><title>%s</title></head>"
						       "<body><img src=\"internal:%s\"></body></html>",
						       url,
						       location);
			gtk_object_set_data (GTK_OBJECT (ebr), "InternalLoader", el);
			stream = gtk_html_begin (GTK_HTML (ebr));
			gtk_html_stream_write (stream, str, strlen (str));
			gtk_html_stream_close (stream, GTK_HTML_STREAM_OK);
		} else if (strncmp (content_type, "text/plain", 10) == 0) {
			GtkHTMLStream * stream;
			gchar * str = g_strdup_printf ("<html><head><title>%s</title></head>"
						       "<body><pre>\n",
						       url);
			stream = gtk_html_begin (GTK_HTML (ebr));
			gtk_html_stream_write (stream, str, strlen (str));
			eloader_set_sufix (el, "\n</pre></html>");
			eloader_set_stream (el, stream);
		} else {
			/* Unhandled type */
			eloader_set_stream (el, gtk_html_begin (GTK_HTML (ebr)));
		}
	}

	if (url) {
		ebr->url = g_strdup (url);
	}

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
		ebrowser_http_base (location, &ebr->baseroot, &ebr->basedir);
		break;
	case EBROWSER_PROTOCOL_FILE:
	case EBROWSER_PROTOCOL_RELATIVE:
		ebrowser_file_base (location, &ebr->baseroot, &ebr->basedir);
		break;
	case EBROWSER_PROTOCOL_UNKNOWN:
		ebr->baseroot = g_strdup ("");
		ebr->basedir  = g_strdup ("");
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	g_print ("Connected: %s\n", url);

	gtk_signal_emit (GTK_OBJECT (ebr), ebr_signals[URL_SET], ebr->url);
}

static EBrowserProtocol
ebrowser_find_protocol (const gchar * uri, const gchar ** location)
{
	const gchar * p;

	if (!uri) return EBROWSER_PROTOCOL_UNKNOWN;

	while (*uri && *uri <= ' ') uri++;

	if (!*uri) return EBROWSER_PROTOCOL_UNKNOWN;

	if (!strncmp (uri, "internal:", 9)) {
		if (location) *location = uri + 9;
		return EBROWSER_PROTOCOL_INTERNAL;
	}

	if (!strncmp (uri, "http://", 7)) {
		if (location) *location = uri + 7;
		return EBROWSER_PROTOCOL_HTTP;
	}

	if (!strncmp (uri, "file:", 5)) {
		if (location) *location = uri + 5;
		return EBROWSER_PROTOCOL_FILE;
	}

	if (location) *location = uri;

	for (p = uri; *p; p++) {
		if (*p == '/') return EBROWSER_PROTOCOL_RELATIVE; /* Slash before colon */
		if (*p == ':') return EBROWSER_PROTOCOL_UNKNOWN; /* Colon before slash */
	}

	/* neither colon nor slash */

	return EBROWSER_PROTOCOL_RELATIVE;
}

static gboolean
ebrowser_http_base (const gchar * location, gchar ** root, gchar ** dir)
{
	gchar * slash1, * slash2;

	if (!location) return FALSE;

	slash1 = strchr (location, '/');

	if (!slash1) {
		*root = g_strdup_printf ("http://%s/", location);
		*dir = g_strdup_printf ("http://%s/", location);
		return TRUE;
	}

	if (*(slash1 + 1) == '~') {
		slash1 = strchr (slash1, '/');
		if (!slash1) {
			*root = g_strdup_printf ("http://%s/", location);
			*dir = g_strdup_printf ("http://%s/", location);
			return TRUE;
		}
	}

	slash2 = strrchr (location, '/');
	g_assert (slash2);

	*root = g_new (gchar, 7 + (slash1 - location) + 1 + 1);
	memcpy (*root, "http://\0", 8);
	strncat (*root, location, (slash1 - location));
	strcat (*root, "/");

	*dir = g_new (gchar, 7 + (slash2 - location) + 1 + 1);
	memcpy (*dir, "http://\0", 8);
	strncat (*dir, location, (slash2 - location));
	strcat (*dir, "/");

	return TRUE;
}

/*
 * File protocol base
 */

static gboolean
ebrowser_file_base (const gchar * location, gchar ** root, gchar ** dir)
{
	gchar * slash2;
	gchar * str;
	const gchar * trail;

	if (!location) return FALSE;

	if (*location == '/') {
		/* Start from root */
		*root = g_strdup ("file:/");
		trail = location + 1;
	} else if ((*location == '~') && (location[1] == '/')) {
		/* Homedir */
		*root = g_strdup_printf ("file:%s/", g_get_home_dir ());
		trail = location + 2;
	} else {
		str = g_get_current_dir ();
		*root = g_strdup_printf ("file:%s/", str);
		g_free (str);
		trail = location;
	}

	slash2 = strrchr (trail, '/');

	if (!slash2) {
		*dir = g_strdup (*root);
	} else {
		*dir = g_new (gchar, strlen (*root) + (slash2 - trail) + 1 + 1);
		strcpy (*dir, *root);
		strncat (*dir, trail, (slash2 - trail));
		strcat (*dir, "/");
	}

	return TRUE;
}

static gchar *
ebrowser_concat (const gchar * url, const gchar * relative)
{
	EBrowserProtocol proto;
	const gchar * location;
	gchar * root, * dir;
	gchar * fresh, * start, * b;
	gint len;

	proto = ebrowser_find_protocol (url, &location);

	switch (proto) {
	case EBROWSER_PROTOCOL_HTTP:
		if (!ebrowser_http_base (location, &root, &dir)) return NULL;
		break;
	case EBROWSER_PROTOCOL_FILE:
		if (!ebrowser_file_base (location, &root, &dir)) return NULL;
		break;
	default:
		return NULL;
		break;
	}

	fresh = g_strconcat (dir, relative, NULL);
	start = fresh + strlen (root);
	g_free (root);
	g_free (dir);
	len = strlen (fresh);

	g_print ("URL: %s\nRelative: %s\n", url, relative);

	while ((b = strstr (fresh, ".."))) {
		if ((*(b - 1) == '/') && (*(b + 2) == '/') && (b > start)) {
			/* file:foo/../bar */
			/* file:/foo/../bar */
			gchar * s;
			s = b - 2;
			while ((s >= start - 1) && (*s != '/') && (*s != ':')) s--;
			memmove (s + 1, b + 3, len - (b - fresh) - 2);
			g_print ("URL step: %s\n", fresh);
		} else {
			/* Remove these completely */
			memmove (b, b + 2, len - (b - fresh) - 1);
			g_print ("URL remove: %s\n", fresh);
		}
	}

	g_print ("Final: %s\n", fresh);

	return fresh;
}

gpointer
ebrowser_base_stream (EBrowser * ebr)
{
	GtkHTMLStream * stream;

	ebrowser_stop_loading (ebr);

	if (ebr->url) {
		g_free (ebr->url);
		ebr->url = NULL;
	}

	if (ebr->baseroot) {
		g_free (ebr->baseroot);
		ebr->baseroot = NULL;
	}

	if (ebr->basedir) {
		g_free (ebr->basedir);
		ebr->basedir = NULL;
	}

	ebr->baseprotocol = EBROWSER_PROTOCOL_UNKNOWN;

	ebr->url = g_strdup ("Bonobo:");

	stream = gtk_html_begin (GTK_HTML (ebr));

	return stream;
}


