#define _EBROWSER_STREAM_C_

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

#include "gtkhtml-stream.h"
#include "ebrowser-widget.h"
#include "ebrowser-stream.h"

#define ES_DEBUG(str,section) if (FALSE) g_print ("%s:%d (%s) %s\n", __FILE__, __LINE__, __FUNCTION__, str);

#define READ_CHUNK_SIZE 4096

void ebrowser_ps_load (BonoboPersistStream * ps,
		       Bonobo_Stream stream,
		       Bonobo_Persist_ContentType type,
		       gpointer data,
		       CORBA_Environment * ev)
{
	EBrowser * ebr;
	GtkHTMLStream * handle;
	Bonobo_Stream_iobuf * buffer;

	if (strcmp (type, "text/html") != 0) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Persist_WrongDataType, NULL);
		return;
	}

	ebr = EBROWSER (data);

	handle = ebrowser_base_stream (ebr);

	do {
		Bonobo_Stream_read (stream, READ_CHUNK_SIZE,
				    &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION) break;
		if (buffer->_length <= 0) break;
		gtk_html_stream_write (handle, buffer->_buffer, buffer->_length);
		CORBA_free (buffer);
	} while (1);

	if (ev->_major != CORBA_NO_EXCEPTION) {
		gtk_html_stream_close (handle, GTK_HTML_STREAM_ERROR);
	} else {
		CORBA_free (buffer);
		gtk_html_stream_close (handle, GTK_HTML_STREAM_OK);
	}
}

Bonobo_Persist_ContentTypeList *
ebrowser_ps_types (BonoboPersistStream * ps,
		   gpointer data,
		   CORBA_Environment * ev)
{
	return bonobo_persist_generate_content_types (1, "text/html");
}


