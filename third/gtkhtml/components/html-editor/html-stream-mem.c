/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream-memory.c: Memory based stream
 *
 * Author:
 *   Larry Ewing <lewing@ximian.com>
 *
 * Copyright 2001, Ximian, Inc.
 */
#include <config.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>

#include "gtkhtml.h"
#include "gtkhtml-stream.h"
#include "html-stream-mem.h"

BonoboStreamMemClass *parent_class;

static void
html_stream_mem_write (BonoboStream *stream,
		       const Bonobo_Stream_iobuf *buffer,
		       CORBA_Environment *ev)
{
	HTMLStreamMem *bhtml = HTML_STREAM_MEM (stream);

	BONOBO_STREAM_CLASS (parent_class)->write (stream, buffer, ev);
	
	if (bhtml->html_stream) {
		if (ev->_major == CORBA_NO_EXCEPTION) {
			gtk_html_stream_write (bhtml->html_stream, buffer->_buffer, buffer->_length);
		} else {
			gtk_html_stream_close (bhtml->html_stream, GTK_HTML_STREAM_OK);
			bhtml->html_stream = NULL;
		}
	}
}

static void 
html_stream_mem_destroy (GtkObject *object)
{
	HTMLStreamMem *bhtml = HTML_STREAM_MEM (object);

	if (bhtml->html_stream) {
		gtk_html_stream_close (bhtml->html_stream, GTK_HTML_STREAM_OK);
		bhtml->html_stream = NULL;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);		
}

HTMLStreamMem *
html_stream_mem_constuct (HTMLStreamMem *bhtml,
			  Bonobo_Stream corba_stream,
			  GtkHTMLStream *html_stream)
{
	g_return_val_if_fail (corba_stream != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (BONOBO_IS_STREAM_MEM (bhtml), NULL);
	
	bhtml->html_stream = html_stream;

	return HTML_STREAM_MEM (bonobo_stream_mem_construct (BONOBO_STREAM_MEM (bhtml),
							     corba_stream,
							     NULL, 0,
							     FALSE, TRUE));
}

BonoboStream *
html_stream_mem_create (GtkHTMLStream *html_stream) 
{
	HTMLStreamMem *bhtml;
	Bonobo_Stream corba_stream;

	bhtml = gtk_type_new (html_stream_mem_get_type ());
	if (bhtml == NULL)
		return NULL;

	corba_stream = bonobo_stream_corba_object_create (BONOBO_OBJECT (bhtml));

	if (corba_stream == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (bhtml));
		return NULL;
	}

	return BONOBO_STREAM (html_stream_mem_constuct (bhtml,
							corba_stream,
							html_stream));
}	

static void 
html_stream_mem_class_init (HTMLStreamMemClass *klass)
{
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (klass);
	GtkObjectClass *oclass = GTK_OBJECT_CLASS (klass);

	parent_class = gtk_type_class (bonobo_stream_mem_get_type ());
	
	sclass->write = html_stream_mem_write;
	
	oclass->destroy = html_stream_mem_destroy;	

}

GtkType
html_stream_mem_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"HTMLStreamMem",
			sizeof (HTMLStreamMem),
			sizeof (HTMLStreamMemClass),
			(GtkClassInitFunc) html_stream_mem_class_init,
			(GtkObjectInitFunc) NULL,
			NULL,
			NULL,
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_stream_mem_get_type (), &info);
	}
	
	return type;
}
		

