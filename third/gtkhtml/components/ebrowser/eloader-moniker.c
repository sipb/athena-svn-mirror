/*  This file is part of the GtkHTML library.

    Copyright (C) 2001 Ximian, Inc.

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

    Author: Radek Doulik  <rodo@ximian.com>
*/

#include "eloader-moniker.h"

static void eloader_moniker_class_init (GtkObjectClass * klass);
static void eloader_moniker_destroy (GtkObject * object);

static gint eloader_moniker_idle (gpointer data);

static ELoaderClass * parent_class;

GtkType
eloader_moniker_get_type (void)
{
	static GtkType loader_type = 0;
	if (!loader_type) {
		GtkTypeInfo loader_info = {
			"ELoaderMoniker",
			sizeof (ELoaderMoniker),
			sizeof (ELoaderMonikerClass),
			(GtkClassInitFunc) eloader_moniker_class_init,
			NULL, NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		loader_type = gtk_type_unique (eloader_get_type (), &loader_info);
	}
	return loader_type;
}

static void
eloader_moniker_class_init (GtkObjectClass * klass)
{
	parent_class = gtk_type_class (eloader_get_type ());

	klass->destroy = eloader_moniker_destroy;
}

static void
eloader_moniker_destroy (GtkObject * object)
{
	ELoaderMoniker * el;

	el = ELOADER_MONIKER (object);

	g_free (el->url);

	if (el->idle_id) {
		gtk_idle_remove (el->idle_id);
		el->idle_id = 0;
	}

	if (el->stream != CORBA_OBJECT_NIL) {
		CORBA_Environment ev;
		CORBA_exception_init (&ev);
		bonobo_object_release_unref (el->stream, &ev);
		CORBA_exception_free (&ev);
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

ELoader *
eloader_moniker_new (EBrowser * ebr, const gchar * path, GtkHTMLStream * stream)
{
	ELoaderMoniker *el;
	Bonobo_Stream  corba_stream;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	corba_stream = bonobo_get_object (path, "IDL:Bonobo/Stream:1.0", &ev);
	if (ev._major != CORBA_NO_EXCEPTION || corba_stream == CORBA_OBJECT_NIL) {
		gtk_html_stream_close (stream, GTK_HTML_STREAM_ERROR);
		CORBA_exception_free (&ev);

		return NULL;
	}

	el = gtk_type_new (ELOADER_MONIKER_TYPE);

	eloader_construct (ELOADER (el),  ebr, stream);
	/* eloader_connect   (ELOADER (el), path, "text/html"); */

	el->url     = g_strdup (path);
	el->stream  = corba_stream;
	el->idle_id = gtk_idle_add (eloader_moniker_idle, el);
	/* gtk_signal_connect (GTK_OBJECT (el), "connect",
	   GTK_SIGNAL_FUNC (body_connect), el); */

	CORBA_exception_free (&ev);

	return ELOADER (el);
}

static gint
eloader_moniker_idle (gpointer data)
{
        Bonobo_Stream_iobuf *stream_iobuf;
	CORBA_Environment ev;
	ELoaderMoniker * el;
	gint rv = TRUE;

	el = ELOADER_MONIKER (data);

	if (!el->loader.stream)
		eloader_connect   (ELOADER (el), el->url, "text/html");

	CORBA_exception_init (&ev);
        Bonobo_Stream_read (el->stream, 512, &stream_iobuf, &ev);
        if (ev._major != CORBA_NO_EXCEPTION)
		rv = FALSE;
	CORBA_exception_free (&ev);

        if (stream_iobuf->_length == 0) {
		CORBA_free (stream_iobuf);
		eloader_done (ELOADER (el), ELOADER_OK);
		return FALSE;
        }

	if (rv) {
		gtk_html_stream_write (el->loader.stream, stream_iobuf->_buffer, stream_iobuf->_length);
		CORBA_free (stream_iobuf);
	} else {
		if (el->stream != CORBA_OBJECT_NIL) {
			bonobo_object_release_unref (el->stream, NULL);
			el->stream  = CORBA_OBJECT_NIL;
		}
		el->idle_id = 0;
	}

	return rv;
}
