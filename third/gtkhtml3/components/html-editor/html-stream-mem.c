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
#include <libgnome/gnome-i18n.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-exception.h>

#include "gtkhtml.h"
#include "gtkhtml-stream.h"
#include "html-stream-mem.h"

BonoboObjectClass *parent_class;

static void
html_stream_mem_write (PortableServer_Servant servant,
		       const Bonobo_Stream_iobuf *buffer,
		       CORBA_Environment *ev)
{
	HTMLStreamMem *bhtml = HTML_STREAM_MEM (bonobo_object_from_servant (servant));

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
html_stream_mem_finalize (GObject *object)
{
	HTMLStreamMem *bhtml = HTML_STREAM_MEM (object);

	if (bhtml->html_stream) {
		gtk_html_stream_close (bhtml->html_stream, GTK_HTML_STREAM_OK);
		bhtml->html_stream = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);		
}

HTMLStreamMem *
html_stream_mem_construct (HTMLStreamMem *bhtml,
			   GtkHTMLStream *html_stream)
{
	g_return_val_if_fail (HTML_IS_STREAM_MEM (bhtml), NULL);
	
	bhtml->html_stream = html_stream;

	return bhtml;
}

BonoboObject *
html_stream_mem_create (GtkHTMLStream *html_stream) 
{
	HTMLStreamMem *bhtml;

	bhtml = g_object_new (HTML_STREAM_MEM_TYPE, NULL);
	if (bhtml == NULL)
		return NULL;

	return BONOBO_OBJECT (html_stream_mem_construct (bhtml, html_stream));
}

static void
html_stream_mem_init (HTMLStreamMem *mem)
{
}

static void 
html_stream_mem_class_init (HTMLStreamMemClass *klass)
{
	GObjectClass *o_class = G_OBJECT_CLASS (klass);
	POA_Bonobo_Stream__epv *epv = &klass->epv;

	parent_class = g_type_class_peek_parent (klass);
	epv->write = html_stream_mem_write;
	
	o_class->finalize = html_stream_mem_finalize;

}

BONOBO_TYPE_FUNC_FULL (
	HTMLStreamMem,                 /* Glib class name */
	Bonobo_Stream,                 /* CORBA interface name */
	BONOBO_TYPE_OBJECT,            /* parent type */
	html_stream_mem);              /* local prefix ie. 'echo'_class_init */
