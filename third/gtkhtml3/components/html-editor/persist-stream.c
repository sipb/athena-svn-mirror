/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library

   Copyright (C) 2002 Ximian, Inc.
   Authors:           Radek Doulik (rodo@ximian.com)
                      Larry Ewing  (lewing@ximian.com)
		      Ettore Perazzoli (ettore@ximian.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHcANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include <string.h>
#include "gtkhtml.h"
#include "persist-stream.h"
#include "editor-control-factory.h"

static BonoboObjectClass *gtk_html_persist_stream_parent_class;

static void impl_save (PortableServer_Servant servant, const Bonobo_Stream stream, const CORBA_char *type,
		       CORBA_Environment * ev);
static void impl_load (PortableServer_Servant servant, const Bonobo_Stream stream, const CORBA_char *type,
		       CORBA_Environment * ev);

static void
finalize (GObject *object)
{
	GtkHTMLPersistStream *stream = GTK_HTML_PERSIST_STREAM (object);

	if (stream->html) {
		g_object_unref (stream->html);
		stream->html = NULL;
	}

	G_OBJECT_CLASS (gtk_html_persist_stream_parent_class)->finalize (object);
}

static Bonobo_Persist_ContentTypeList *
get_content_types (BonoboPersist *persist, CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (2, "text/html", "text/plain");
}

static void
gtk_html_persist_stream_class_init (GtkHTMLPersistStreamClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	BonoboPersistClass *persist_class = BONOBO_PERSIST_CLASS (klass);
	POA_Bonobo_PersistStream__epv *epv = &klass->epv;
	
	gtk_html_persist_stream_parent_class = g_type_class_peek_parent (klass);

	epv->load = impl_load;
	epv->save = impl_save;

	object_class->finalize = finalize;
	persist_class->get_content_types = get_content_types;
}

GType
gtk_html_persist_stream_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (GtkHTMLPersistStreamClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gtk_html_persist_stream_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (GtkHTMLPersistStream),
			0, /* n_preallocs */
			(GInstanceInitFunc) NULL
		};

		type = bonobo_type_unique (
			BONOBO_TYPE_PERSIST,
			POA_Bonobo_PersistStream__init, POA_Bonobo_PersistStream__fini,
			G_STRUCT_OFFSET (GtkHTMLPersistStreamClass, epv),
			&info, "GtkHTMLPersistStream");
	}

	return type;
}

BonoboObject *
gtk_html_persist_stream_new (GtkHTML *html)
{
	BonoboObject *stream;

	stream = g_object_new (gtk_html_persist_stream_get_type (), NULL);
	bonobo_persist_construct (BONOBO_PERSIST (stream), CONTROL_FACTORY_ID);

	g_object_ref (html);
	GTK_HTML_PERSIST_STREAM (stream)->html = html;

	return stream;
}

static void
impl_load (PortableServer_Servant servant, const Bonobo_Stream stream, const CORBA_char *type, CORBA_Environment * ev)
{
	GtkHTMLPersistStream *persist = GTK_HTML_PERSIST_STREAM (bonobo_object_from_servant (servant));
	Bonobo_Stream_iobuf *buffer;
	GtkHTMLStream *handle;
	gboolean was_editable;

	if (strcmp (type, "text/html") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	was_editable = gtk_html_get_editable (persist->html);
	if (was_editable)
		gtk_html_set_editable (persist->html, FALSE);
	
	/* bonobo streams are _always_ utf-8 */
	handle = gtk_html_begin_content (persist->html, "text/html; charset=utf-8");

	do {
		Bonobo_Stream_read (stream, 4096, &buffer, ev);

		if (ev->_major != CORBA_NO_EXCEPTION || buffer->_length <= 0) {
		        CORBA_free (buffer);
			break;
		}

		gtk_html_write (persist->html, handle, buffer->_buffer, buffer->_length);
		CORBA_free (buffer);
	} while (1);

	/* FIX2 bonobo_persist_stream_set_dirty (ps, TRUE); */
	gtk_html_end (persist->html, handle, ev->_major == CORBA_NO_EXCEPTION ? GTK_HTML_STREAM_OK : GTK_HTML_STREAM_ERROR);

	if (was_editable)
		gtk_html_set_editable (persist->html, TRUE);
}

struct _SaveState {
	Bonobo_Stream stream;
	CORBA_Environment *ev;
};
typedef struct _SaveState SaveState;

static gboolean
save_receiver (const HTMLEngine *engine, const gchar *data, guint length, gpointer user_data)
{
	Bonobo_Stream_iobuf buffer;
	SaveState *state;

	state = (SaveState *) user_data;
	if (state->stream == CORBA_OBJECT_NIL)
		CORBA_exception_set (state->ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_IOError, NULL);

	if (state->ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;

	buffer._maximum = length;
	buffer._length = length;
	buffer._buffer = (CORBA_char *) data; /* Should be safe.  */

	Bonobo_Stream_write (state->stream, &buffer, state->ev);

	if (state->ev->_major != CORBA_NO_EXCEPTION)
		return FALSE;


	return TRUE;
}

static void
impl_save (PortableServer_Servant servant, const Bonobo_Stream stream, const CORBA_char *type, CORBA_Environment * ev)
{
	GtkHTMLPersistStream *persist = GTK_HTML_PERSIST_STREAM (bonobo_object_from_servant (servant));
	SaveState save_state;
	
	if (strcmp (type, "text/html") == 0 || strcmp (type, "text/plain") == 0) {
		save_state.ev = ev;
		save_state.stream = CORBA_Object_duplicate (stream, ev);
		if (ev->_major == CORBA_NO_EXCEPTION)
			/* if ( */
			gtk_html_export (persist->html, (char *) type, (GtkHTMLSaveReceiverFn) save_receiver, &save_state);
		/* )
		   FIX2 bonobo_persist_stream_set_dirty (ps, TRUE); */
				
		CORBA_Object_release (save_state.stream, ev);
	} else {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
	}

	return;
}
