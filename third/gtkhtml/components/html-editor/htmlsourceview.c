/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

    Author: Larry Ewing <lewing@ximian.com>

*/

#include <config.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>

#include "gtkhtml.h"
#include "htmlengine.h"
#include "gtkhtml-stream.h"
#include "htmlsourceview.h"
#include "e-html-utils.h"

struct _HTMLSourceViewPrivate {
	GtkHTML   *html;
	CORBA_Object pstream;
	char    *content_type;
	gint     current_interval;
	gint     timer_id;
	gboolean as_html;
};

enum {
	UPDATE,
	LAST_SIGNAL
};

static GtkObject *parent_class;
static guint      signals [LAST_SIGNAL] = { 0 };

static void
html_source_view_real_update (HTMLSourceView *view)
{
}

static void
html_source_view_load (HTMLSourceView *view)
{
	BonoboStream *smem;
	GtkHTMLStream *hstream;
	CORBA_Environment ev;
	CORBA_Object pstream;
	const char *text;
	char *html;
	size_t len;

	CORBA_exception_init (&ev);

	pstream = view->priv->pstream;
	smem = bonobo_stream_mem_create (NULL, 1, FALSE, TRUE);
	Bonobo_PersistStream_save (pstream, BONOBO_OBJREF (smem), view->priv->content_type, &ev);
	/* Terminate the buffer for e_text_to_html */
	bonobo_stream_client_write (BONOBO_OBJREF (smem), "", 1, &ev);

        text = bonobo_stream_mem_get_buffer (BONOBO_STREAM_MEM (smem));
	len  = bonobo_stream_mem_get_size (BONOBO_STREAM_MEM (smem));

	if (!view->priv->as_html) {
		html = e_text_to_html_full (text, E_TEXT_TO_HTML_PRE | E_TEXT_TO_HTML_CONVERT_SPACES, 0);
	} else {
		html = g_strdup (text);
	}

	hstream = gtk_html_begin (view->priv->html);
	view->priv->html->engine->newPage = FALSE;

	gtk_html_stream_write (hstream, html, strlen (html));
	gtk_html_stream_close (hstream, GTK_HTML_STREAM_OK);

	bonobo_object_unref (BONOBO_OBJECT (smem));
	g_free (html);

	CORBA_exception_free (&ev);
}

static gint
html_source_view_timeout (gpointer *data)
{
	HTMLSourceView *view;
	
	g_return_val_if_fail (HTML_IS_SOURCE_VIEW (data), FALSE);

	view = HTML_SOURCE_VIEW (data);
	html_source_view_load (view);

	return TRUE;
}

void
html_source_view_set_timeout (HTMLSourceView *view, guint timeout)
{
	if (view->priv->timer_id)
		gtk_timeout_remove (view->priv->timer_id);
	
	view->priv->current_interval = timeout;
	view->priv->timer_id = gtk_timeout_add (timeout, (GtkFunction)html_source_view_timeout, view);
}

void
html_source_view_set_mode (HTMLSourceView *view, gboolean as_html)
{
	view->priv->as_html = as_html;
}

void
html_source_view_set_source (HTMLSourceView *view, BonoboWidget *control, char *content_type)
{
	BonoboObjectClient *object_client;

	g_return_if_fail (HTML_IS_SOURCE_VIEW (view));

	object_client = bonobo_widget_get_server (control);
	g_return_if_fail (object_client != NULL);
	
	g_free (view->priv->content_type);
	view->priv->content_type = g_strdup (content_type);

	view->priv->pstream = bonobo_object_client_query_interface (object_client, "IDL:Bonobo/PersistStream:1.0", NULL);

	if (view->priv->pstream == CORBA_OBJECT_NIL) {
		g_warning ("Couldn't find persist stream interface");
	} else {
		html_source_view_set_timeout (view, view->priv->current_interval);
	}
}

GtkWidget *
html_source_view_new (void)
{
	GtkWidget *view;

	view = GTK_WIDGET (gtk_type_new (html_source_view_get_type ()));
	
	return view;
}

static void
html_source_view_init (HTMLSourceView *view)
{
	GtkWidget *html;
	GtkWidget *scroll;

	view->priv = g_new0 (HTMLSourceViewPrivate, 1);

	view->priv->content_type = NULL;
	view->priv->html = GTK_HTML (html = gtk_html_new ());
	view->priv->current_interval = 500;
	view->priv->as_html = FALSE;
	
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	gtk_container_add (GTK_CONTAINER (scroll), html);

	gtk_box_pack_start (GTK_BOX (view), scroll,
			    TRUE, TRUE, 0);

	gtk_widget_show_all (scroll);
}

static void
html_source_view_destroy (GtkObject *object)
{
	HTMLSourceViewPrivate *priv = HTML_SOURCE_VIEW (object)->priv;

	if (priv->timer_id)
		gtk_timeout_remove (priv->timer_id);
	priv->timer_id = 0;

	if (priv->pstream != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;

		CORBA_exception_init (&ev);
		Bonobo_Unknown_unref (priv->pstream, &ev);
		CORBA_Object_release (priv->pstream, &ev);
		CORBA_exception_free (&ev);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy != NULL)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
html_source_view_finalize (GtkObject *object)
{
	HTMLSourceView *view = HTML_SOURCE_VIEW (object);
	
	g_free (view->priv);
	view->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->finalize != NULL)
		(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
html_source_view_class_init (HTMLSourceViewClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);
	parent_class = gtk_type_class (gtk_vbox_get_type ());

	signals [UPDATE] = gtk_signal_new ("update",
					   GTK_RUN_FIRST,
					   object_class->type,
					   GTK_SIGNAL_OFFSET (HTMLSourceViewClass, update),
					   gtk_marshal_NONE__NONE,
					   GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);
	
	object_class->destroy = html_source_view_destroy;
	object_class->finalize = html_source_view_finalize;
	klass->update = html_source_view_real_update;
}

GtkType
html_source_view_get_type ()
{
	static GtkType view_type = 0;
	
	if (!view_type) {
		GtkTypeInfo view_info = {
			"HTMLSourceView",
			sizeof (HTMLSourceView),
			sizeof (HTMLSourceViewClass),
			(GtkClassInitFunc) html_source_view_class_init,
			(GtkObjectInitFunc) html_source_view_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};

		view_type = gtk_type_unique (gtk_vbox_get_type (), & view_info);
	}
	
	return view_type;
}
