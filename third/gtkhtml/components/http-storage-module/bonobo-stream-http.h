/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream-http.c: HTTP Stream implementation
 *
 * libghttp-based Stream implementation.
 *
 * Copyright (c) 2000 Helix Code, Inc.
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 */
#ifndef _BONOBO_STREAM_HTTP_H_
#define _BONOBO_STREAM_HTTP_H_

#include <bonobo/bonobo-stream.h>
#include <ghttp.h>

BEGIN_GNOME_DECLS

#define BONOBO_STREAM_HTTP_TYPE        (bonobo_stream_http_get_type ())
#define BONOBO_STREAM_HTTP(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_HTTP_TYPE, BonoboStreamHTTP))
#define BONOBO_STREAM_HTTP_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_HTTP_TYPE, BonoboStreamHTTPClass))
#define BONOBO_IS_STREAM_HTTP(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_HTTP_TYPE))
#define BONOBO_IS_STREAM_HTTP_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_HTTP_TYPE))

typedef struct _BonoboStreamHTTP     BonoboStreamHTTP;

struct _BonoboStreamHTTP {
	BonoboStream stream;

	char *url;
	ghttp_request *request;
	int offset;
};

typedef struct {
	BonoboStreamClass parent_class;
} BonoboStreamHTTPClass;

GtkType          bonobo_stream_http_get_type     (void);
BonoboStream    *bonobo_stream_http_open         (const char *path, gint flags,
						  gint mode, CORBA_Environment *ev);
BonoboStream    *bonobo_stream_http_create       (const CORBA_char *path);
BonoboStream    *bonobo_stream_http_construct    (BonoboStreamHTTP *stream,
						  Bonobo_Stream corba_stream);
	
END_GNOME_DECLS

#endif /* _BONOBO_STREAM_HTTP_H_ */
