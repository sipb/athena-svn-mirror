/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-storage-http.h: HTTP storage backend
 *
 * Copyright (c) 2000 Helix Code, Inc.
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 *
 */

#ifndef _BONOBO_STORAGE_HTTP_H_
#define _BONOBO_STORAGE_HTTP_H_

#include <bonobo/bonobo-storage.h>
#include <ghttp.h>

BEGIN_GNOME_DECLS

#define BONOBO_STORAGE_HTTP_TYPE        (bonobo_storage_http_get_type ())
#define BONOBO_STORAGE_HTTP(o)          (GTK_CHECK_CAST ((o), BONOBO_STORAGE_HTTP_TYPE, BonoboStorageHTTP))
#define BONOBO_STORAGE_HTTP_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STORAGE_HTTP_TYPE, BonoboStorageHTTPClass))
#define BONOBO_IS_STORAGE_HTTP(o)       (GTK_CHECK_TYPE ((o), BONOBO_STORAGE_HTTP_TYPE))
#define BONOBO_IS_STORAGE_HTTP_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_HTTP_TYPE))

typedef struct {
	BonoboStorage storage;
	char *url;
	ghttp_request *request;
} BonoboStorageHTTP;

typedef struct {
	BonoboStorageClass parent_class;
} BonoboStorageHTTPClass;

GtkType        bonobo_storage_http_get_type  (void);
BonoboStorage *bonobo_storage_http_construct (BonoboStorageHTTP *storage,
					      Bonobo_Storage corba_storage,
					      const char *path, const char *open_mode);
BonoboStorage *bonobo_storage_http_open      (const char *path,
					      gint flags, gint mode,
					      CORBA_Environment *ev);
BonoboStorage *bonobo_storage_http_create    (BonoboStorageHTTP *storage,
					      const CORBA_char *path);

END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_HTTP_H_ */
