#define _ELOADER_C_

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
#include "eloader.h"

#define noDEBUG_EL_ALLOC

#define EL_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str);

static void eloader_class_init (GtkObjectClass * klass);
static void eloader_init (GtkObject * object);
static void eloader_destroy (GtkObject * object);

enum {CONNECT, DONE, SET_STATUS, LAST_SIGNAL};

static GtkObjectClass * parent_class;
static guint el_signals[LAST_SIGNAL] = {0};

#ifdef DEBUG_EL_ALLOC
static gint el_num = 0;
#endif

GtkType
eloader_get_type (void)
{
	static GtkType loader_type = 0;
	if (!loader_type) {
		GtkTypeInfo loader_info = {
			"ELoader",
			sizeof (ELoader),
			sizeof (ELoaderClass),
			(GtkClassInitFunc) eloader_class_init,
			(GtkObjectInitFunc) eloader_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL
		};
		loader_type = gtk_type_unique (gtk_object_get_type (), &loader_info);
	}
	return loader_type;
}

static void
eloader_class_init (GtkObjectClass * klass)
{
	parent_class = gtk_type_class (gtk_object_get_type ());

	el_signals[CONNECT] = gtk_signal_new ("connect",
					      GTK_RUN_FIRST,
					      klass->type,
					      GTK_SIGNAL_OFFSET (ELoaderClass, connect),
					      gtk_marshal_NONE__POINTER_POINTER,
					      GTK_TYPE_NONE, 2,
					      GTK_TYPE_POINTER, GTK_TYPE_POINTER);
	el_signals[DONE]    = gtk_signal_new ("done",
					      GTK_RUN_FIRST,
					      klass->type,
					      GTK_SIGNAL_OFFSET (ELoaderClass, done),
					      gtk_marshal_NONE__UINT,
					      GTK_TYPE_NONE, 1,
					      GTK_TYPE_UINT);
	el_signals[SET_STATUS] = gtk_signal_new ("set_status",
					      GTK_RUN_FIRST,
					      klass->type,
					      GTK_SIGNAL_OFFSET (ELoaderClass, set_status),
					      gtk_marshal_NONE__POINTER,
					      GTK_TYPE_NONE, 1,
					      GTK_TYPE_POINTER);

	gtk_object_class_add_signals (klass, el_signals, LAST_SIGNAL);

	klass->destroy = eloader_destroy;
}

static void
eloader_init (GtkObject * object)
{
	ELoader * el;

	el = ELOADER (object);

	el->ebrowser = NULL;
	el->stream = NULL;
	el->sufix = NULL;

#ifdef DEBUG_EL_ALLOC
	el_num++;
	g_print ("ELoaders: %d\n", el_num);
#endif
}

static void
eloader_destroy (GtkObject * object)
{
	ELoader * el;

	el = ELOADER (object);

	if (el->sufix) {
		g_free (el->sufix);
		el->sufix = NULL;
	}

	if (el->stream) {
		gtk_html_stream_destroy (el->stream);
		el->stream = NULL;
	}

	if (el->ebrowser) {
		eloader_done (el, ELOADER_ERROR);
		el->ebrowser = NULL;
	}

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);

#ifdef DEBUG_EL_ALLOC
	el_num--;
	g_print ("ELoaders: %d\n", el_num);
#endif
}

void
eloader_construct (ELoader * eloader, EBrowser * ebrowser, GtkHTMLStream * stream)
{
	g_return_if_fail (eloader != NULL);
	g_return_if_fail (IS_ELOADER (eloader));
	g_return_if_fail (ebrowser != NULL);
	g_return_if_fail (IS_EBROWSER (ebrowser));

	eloader->ebrowser = ebrowser;

	eloader->stream = stream;
}

void
eloader_set_stream (ELoader * eloader, GtkHTMLStream * stream)
{
	g_return_if_fail (eloader != NULL);
	g_return_if_fail (IS_ELOADER (eloader));
	g_return_if_fail (stream);
        g_return_if_fail (!eloader->stream);

	eloader->stream = stream;
}

void
eloader_set_sufix (ELoader * eloader, const gchar * sufix)
{
	g_return_if_fail (eloader != NULL);
	g_return_if_fail (IS_ELOADER (eloader));

	if (eloader->sufix) {
		g_free (eloader->sufix);
		eloader->sufix = NULL;
	}

	if (sufix) eloader->sufix = g_strdup (sufix);
}

void
eloader_connect (ELoader * eloader, const gchar * url, const gchar * content_type)
{
	g_return_if_fail (eloader != NULL);
	g_return_if_fail (IS_ELOADER (eloader));

	gtk_object_ref (GTK_OBJECT (eloader));
	gtk_signal_emit (GTK_OBJECT (eloader), el_signals[CONNECT], url, content_type);
	gtk_object_unref (GTK_OBJECT (eloader));
}

void
eloader_done (ELoader * el, ELoaderStatus status)
{
	g_return_if_fail (el != NULL);
	g_return_if_fail (IS_ELOADER (el));

	if (el->stream) {
		if (el->sufix) {
			gtk_html_stream_write (el->stream, el->sufix, strlen (el->sufix));
		}
		gtk_html_stream_close (el->stream, (status == ELOADER_OK) ? GTK_HTML_STREAM_OK : GTK_HTML_STREAM_ERROR);
		el->stream = NULL;
	}

	gtk_object_ref (GTK_OBJECT (el));
	gtk_signal_emit (GTK_OBJECT (el), el_signals[DONE], status);
	el->ebrowser = NULL;
	gtk_object_unref (GTK_OBJECT (el));
}

void
eloader_set_status (ELoader * el, const gchar * status)
{
	g_return_if_fail (el != NULL);
	g_return_if_fail (IS_ELOADER (el));

	gtk_object_ref (GTK_OBJECT (el));
	gtk_signal_emit (GTK_OBJECT (el), el_signals[SET_STATUS], status);
	gtk_object_unref (GTK_OBJECT (el));
}






