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

    Author: Larry Ewing <lewing@helixcode.com>

*/

#include <config.h>

#include <gnome.h>
#include <bonobo.h>
#include "gtkhtml.h"
#include "resolver-progressive-impl.h"

typedef struct _ResolverSinkData ResolverSinkData;
struct _ResolverSinkData {
	GtkHTML *html;
	gchar *url;
	GtkHTMLStream *handle;
};

#define SINK_PRINT(s) g_warning("%s", s);

static int
resolver_sink_start (BonoboProgressiveDataSink *psink, void *closure)
{
	SINK_PRINT ("Sink START");
	
	return 0;
}

static int
resolver_sink_end (BonoboProgressiveDataSink *psink, void *closure)
{
	ResolverSinkData *data = (ResolverSinkData *)closure;
	gtk_html_end (data->html, data->handle, GTK_HTML_STREAM_OK);	
	
	SINK_PRINT ("Sink END");

	g_free (data->url);
	g_free (data);

	return 0;
}

static int
resolver_sink_add_data (BonoboProgressiveDataSink *psink,
			const Bonobo_ProgressiveDataSink_iobuf *buffer,
			void *closure)
{
	ResolverSinkData *data = (ResolverSinkData *)closure;

	SINK_PRINT ("Sink ADD_DATA");

	if (buffer->_length >0) {
		gtk_html_write (data->html, data->handle, 
				buffer->_buffer, buffer->_length);
	}

	return 0;
}

static int
resolver_sink_set_size (BonoboProgressiveDataSink *psink,
			const CORBA_long count,
			void *closure)
{
  	SINK_PRINT ("Sink SET_SIZE");

	return 0;
}

BonoboProgressiveDataSink *
resolver_sink (GtkHTML *html, const char *url, GtkHTMLStream *handle)
{
	BonoboProgressiveDataSink *sink;
	ResolverSinkData *closure = g_new0 (ResolverSinkData, 1);

	closure->html = html;
	closure->url = g_strdup (url);
	closure->handle = handle;

	sink = bonobo_progressive_data_sink_new (resolver_sink_start,
						 resolver_sink_end,
						 resolver_sink_add_data,
						 resolver_sink_set_size,
						 (void *)closure);

	return sink;
}





