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

    Author: Ettore Perazzoli <ettore@helixcode.com>
*/

/* This file implements the Bonobo::PersistStream interface for the HTML editor
   control.  */

#include <config.h>

#include <gnome.h>
#include <bonobo.h>

#include "gtkhtml.h"

#include "persist-stream-impl.h"


/* Loading.  */

#define READ_CHUNK_SIZE 4096

static void
ps_impl_load (BonoboPersistStream *ps,
	      Bonobo_Stream stream,
	      Bonobo_Persist_ContentType type,
	      gpointer data,
	      CORBA_Environment *ev)
{
	GtkHTML *html;
	Bonobo_Stream_iobuf *buffer;
	GtkHTMLStream *handle;
	gboolean was_editable;

	if (strcmp (type, "text/html") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	html = GTK_HTML (data);

	was_editable = gtk_html_get_editable (html);
	if (was_editable)
		gtk_html_set_editable (html, FALSE);
	
	/* bonobo stream are _always_ utf-8 */
	handle = gtk_html_begin_content (html, "text/html; charset=utf-8");

	do {
		Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
				    &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			break;

		if (buffer->_length <= 0)
			break;
		gtk_html_write (html, handle, buffer->_buffer, buffer->_length);
		CORBA_free (buffer);
	} while (1);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		gtk_html_end (html, handle, GTK_HTML_STREAM_ERROR);
		bonobo_persist_stream_set_dirty (ps, TRUE);
	} else {
		CORBA_free (buffer);
		gtk_html_end (html, handle, GTK_HTML_STREAM_OK);
	}

	if (was_editable)
		gtk_html_set_editable (html, TRUE);
}


/* Saving.  */

struct _SaveState {
	Bonobo_Stream stream;
	CORBA_Environment *ev;
};
typedef struct _SaveState SaveState;

static gboolean
save_receiver (const HTMLEngine *engine,
	       const gchar *data,
	       guint length,
	       gpointer user_data)
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
ps_impl_save (BonoboPersistStream *ps,
	      Bonobo_Stream stream,
	      Bonobo_Persist_ContentType type,
	      gpointer data,
	      CORBA_Environment *ev)
{
	GtkHTML *html;
	SaveState save_state;
	
	if (strcmp (type, "text/html") == 0
	    || strcmp (type, "text/plain") == 0) {
		html = GTK_HTML (data);
		
		save_state.ev = ev;
		save_state.stream = CORBA_Object_duplicate (stream, ev);
		if (ev->_major == CORBA_NO_EXCEPTION)
			if (gtk_html_export (html, 
					     (char *)type, 
					     (GtkHTMLSaveReceiverFn)save_receiver,
					     &save_state))
				bonobo_persist_stream_set_dirty (ps, TRUE);
				
		CORBA_Object_release (save_state.stream, ev);

	} else {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		
	}
	return;
}

static Bonobo_Persist_ContentTypeList *
ps_impl_get_content_types (BonoboPersistStream *ps, gpointer data,
		   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (2, "text/html",
						      "text/plain");
}

static void
ps_destroy (BonoboPersistStream *stream, GtkHTML *html) 
{
	g_return_if_fail (GTK_IS_HTML (html));
	
	gtk_object_unref (GTK_OBJECT (html));
}


BonoboPersistStream *
persist_stream_impl_new (GtkHTML *html)
{
	BonoboPersistStream *stream;

	gtk_object_ref (GTK_OBJECT (html));

	stream = bonobo_persist_stream_new (ps_impl_load, ps_impl_save, NULL, 
					    ps_impl_get_content_types,
					    html);

	gtk_signal_connect (GTK_OBJECT (stream), "destroy",
			    ps_destroy, html);

	return stream;
}






