/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream-http.c: HTTP Stream implementation
 *
 * A libghttp-based Stream implementation.
 *
 * Copyright (c) 2000 Helix Code, Inc.
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 */

#include <config.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include "bonobo-stream-http.h"

static BonoboStreamClass *bonobo_stream_http_parent_class;

static Bonobo_StorageInfo*
http_get_info(BonoboStream *stream,
	      const Bonobo_StorageInfoFields mask,
	      CORBA_Environment *ev)
{
	g_warning ("Not implemented");

	return CORBA_OBJECT_NIL;
} /* http_get_info */

static void
http_set_info(BonoboStream *stream,
	      const Bonobo_StorageInfo *info,
	      const Bonobo_StorageInfoFields mask,
	      CORBA_Environment *ev)
{
	g_warning ("Not implemented");
} /* http_set_info */

static void
http_write(BonoboStream *stream, const Bonobo_Stream_iobuf *buffer,
	   CORBA_Environment *ev)
{
        CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, 
		ex_Bonobo_Stream_NotSupported, NULL);
} /* http_write */

static void
http_read (BonoboStream         *stream,
	   CORBA_long            count,
	   Bonobo_Stream_iobuf **buffer,
	   CORBA_Environment    *ev)
{
	BonoboStreamHTTP *http_stream = BONOBO_STREAM_HTTP(stream);
	CORBA_octet *data;
	char *body;
	int len;
	int v;

	*buffer = Bonobo_Stream_iobuf__alloc();
	CORBA_sequence_set_release(*buffer, TRUE);
	data = CORBA_sequence_CORBA_octet_allocbuf(count);

	body = ghttp_get_body(http_stream->request);
	len = ghttp_get_body_len(http_stream->request);
	
	body += http_stream->offset;
	len -= http_stream->offset;
	
	if (len > count) {
		memcpy(data, body, count);
		v = count;
	}
	else {
		memcpy(data, body, len);
		v = len;
	}
	
	http_stream->offset += v;
	
	(*buffer)->_buffer = data;
	(*buffer)->_length = v;
} /* http_read */

static CORBA_long
http_seek(BonoboStream *stream,
	  CORBA_long offset, Bonobo_Stream_SeekType whence,
	  CORBA_Environment *ev)
{
	BonoboStreamHTTP *http_stream = BONOBO_STREAM_HTTP(stream);

	switch (whence) {
	case Bonobo_Stream_SEEK_CUR:
		http_stream->offset += offset;
		break;
	case Bonobo_Stream_SEEK_SET:
		http_stream->offset = offset;
		break;
	case Bonobo_Stream_SEEK_END:
	default:
		CORBA_exception_set(
			ev, CORBA_USER_EXCEPTION, 
			ex_Bonobo_Stream_NotSupported, NULL);		
		return -1;
	}

	return http_stream->offset;
} /* http_seek */

static void
http_truncate(BonoboStream *stream,
	      const CORBA_long new_size, 
	      CORBA_Environment *ev)
{
        CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, 
		ex_Bonobo_Stream_NotSupported, NULL);
} /* http_truncate */

static void
http_copy_to(BonoboStream *stream,
	     const CORBA_char *dest,
	     const CORBA_long bytes,
	     CORBA_long *read_bytes,
	     CORBA_long *written_bytes,
	     CORBA_Environment *ev)
{
        CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, 
		ex_Bonobo_Stream_NotSupported, NULL);
} /* http_copy_to */

static void
http_commit(BonoboStream *stream,
	    CORBA_Environment *ev)
{
        CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, 
		ex_Bonobo_Stream_NotSupported, NULL);
} /* http_commit */

static void
http_revert(BonoboStream *stream,
	    CORBA_Environment *ev)
{
        CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, 
		ex_Bonobo_Stream_NotSupported, NULL);
} /* http_revert */


static void
http_destroy(GtkObject *object)
{
	BonoboStreamHTTP *http_stream = BONOBO_STREAM_HTTP(object);
	
	g_free(http_stream->url);
	ghttp_request_destroy(http_stream->request);
	http_stream->offset = 0;
} /* http_destroy */

static void
bonobo_stream_http_class_init (BonoboStreamHTTPClass *klass)
{
	GtkObjectClass    *oclass = (GtkObjectClass *) klass;
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS(klass);
	
	bonobo_stream_http_parent_class = gtk_type_class(
		bonobo_stream_get_type ());

	sclass->get_info = http_get_info;
	sclass->set_info = http_set_info;
	sclass->write    = http_write;
	sclass->read     = http_read;
	sclass->seek     = http_seek;
	sclass->truncate = http_truncate;
	sclass->copy_to  = http_copy_to;
        sclass->commit   = http_commit;
        sclass->revert   = http_revert;

	oclass->destroy = http_destroy;
} /* bonobo_stream_http_class_init */

/**
 * bonobo_stream_http_get_type:
 *
 * Returns the GtkType for the BonoboStreamHTTP class.
 */
GtkType
bonobo_stream_http_get_type(void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStreamHTTP",
			sizeof (BonoboStreamHTTP),
			sizeof (BonoboStreamHTTPClass),
			(GtkClassInitFunc) bonobo_stream_http_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique(bonobo_stream_get_type(), &info);
	}

	return type;
} /* bonobo_stream_http_get_type */

/**
 * bonobo_stream_http_construct:
 * @stream: The BonoboStreamHTTP object to initialize.
 * @corba_stream: The CORBA server which implements the BonoboStreamHTTP service.
 *
 * This function initializes an object of type BonoboStreamHTTP using the
 * provided CORBA server @corba_stream.
 *
 * Returns the constructed BonoboStreamHTTP @stream.
 */
BonoboStream *
bonobo_stream_http_construct(BonoboStreamHTTP *stream,
			     Bonobo_Stream corba_stream)
{
	g_return_val_if_fail(stream != NULL, NULL);
	g_return_val_if_fail(BONOBO_IS_STREAM(stream), NULL);
	g_return_val_if_fail(corba_stream != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct(
		BONOBO_OBJECT(stream), corba_stream);

	return BONOBO_STREAM(stream);
} /* bonobo_stream_http_construct */

/**
 * bonobo_stream_http_create:
 * @path: The path to the file to be opened.
 *
 * Creates a new BonoboStreamHTTP object which is bound to the URL
 * specified by @url.
 *
 * When data is read out of the returned BonoboStream object, the read() 
 * operations are mapped to the corresponding operations on the specified file.
 *
 * Returns: the constructed BonoboStream object which is bound to the 
 * specified file.
 */
BonoboStream *
bonobo_stream_http_create(const CORBA_char *url)
{
	BonoboStreamHTTP *stream;
	Bonobo_Stream corba_stream;
	ghttp_status status;

	g_return_val_if_fail (url != NULL, NULL);

	stream = gtk_type_new(bonobo_stream_http_get_type());
	if (stream == NULL)
		return NULL;

	corba_stream = bonobo_stream_corba_object_create(
		BONOBO_OBJECT(stream));

	if (corba_stream == CORBA_OBJECT_NIL) {
		bonobo_object_unref(BONOBO_OBJECT(stream));
		return NULL;
	}

	stream->url = g_strdup(url);
	stream->request = ghttp_request_new();
	if (ghttp_set_uri(stream->request, stream->url))
		goto request_failed;
	ghttp_set_header(stream->request, http_hdr_Connection, "close");
	if (ghttp_prepare(stream->request))
		goto request_failed;
	status = ghttp_process(stream->request);
	if (status == ghttp_error)
		goto request_failed;
	
	return bonobo_stream_http_construct(stream, corba_stream);

request_failed:
	g_free(stream->url);
	ghttp_request_destroy(stream->request);
	return NULL;
} /* bonobo_stream_http_create */

/**
 * bonobo_stream_http_open:
 * @url: The URL to be opened.
 * @flags: The flags with which the file should be opened.
 * @ev: A corba environment for exception handling.
 *
 * Creates a new BonoboStream object for the URL specified by
 * @url.  
 */
BonoboStream *
bonobo_stream_http_open(const char *url, gint flags, gint mode, CORBA_Environment *ev)
{
	return bonobo_stream_http_create(url);
} /* bonobo_stream_http_open */
