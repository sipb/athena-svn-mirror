/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-storage-http.c: HTTP Storage implementation
 *
 * A libghttp-based Storage implementation. Based around the fs storage impl.
 *
 * Copyright (c) 2000 Helix Code, Inc.
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 */

#include <config.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/bonobo-storage-plugin.h>
#include "bonobo-storage-http.h"
#include "bonobo-stream-http.h"

static BonoboStorageClass *bonobo_storage_http_parent_class;

static void
bonobo_storage_http_destroy(GtkObject *object)
{
	BonoboStorageHTTP *storage = BONOBO_STORAGE_HTTP(object);

	g_free(storage->url);
	ghttp_request_destroy(storage->request);
} /* bonobo_storage_http_destroy */

static Bonobo_StorageInfo*
http_get_info (BonoboStorage *storage,
	     const CORBA_char *path,
	     const Bonobo_StorageInfoFields mask,
	     CORBA_Environment *ev)
{
	g_warning ("Not implemented");

	return CORBA_OBJECT_NIL;
} /* http_get_info */

static void
http_set_info(BonoboStorage *storage,
	      const CORBA_char *path,
	      const Bonobo_StorageInfo *info,
	      const Bonobo_StorageInfoFields mask,
	      CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
} /* http_set_info */

static BonoboStream *
http_open_stream(BonoboStorage *storage, const CORBA_char *url, 
		 Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStream *stream;

	stream = bonobo_stream_http_open(url, mode, 0644, ev);

	return stream;
} /* http_open_stream */

static BonoboStorage *
http_open_storage(BonoboStorage *storage, const CORBA_char *url, 
		  Bonobo_Storage_OpenMode mode, CORBA_Environment *ev)
{
	BonoboStorage *new_storage;

	new_storage = bonobo_storage_http_open (url, mode, 0644, ev);

	return new_storage;
} /* http_open_storage */

static void
http_copy_to(BonoboStorage *storage, Bonobo_Storage target, 
	     CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
} /* http_copy_to */

static void
http_rename(BonoboStorage *storage, const CORBA_char *path, 
	    const CORBA_char *new_path, CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
} /* http_rename */

static void
http_commit(BonoboStorage *storage, CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
} /* http_commit */

static void
http_revert(BonoboStorage *storage, CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);
} /* http_revert */

static Bonobo_Storage_DirectoryList *
http_list_contents(BonoboStorage *storage, const CORBA_char *path, 
		   Bonobo_StorageInfoFields mask, CORBA_Environment *ev)
{
	CORBA_exception_set(
		ev, CORBA_USER_EXCEPTION, ex_Bonobo_Stream_NotSupported, NULL);

	return NULL;
} /* http_list_contents */

static void
bonobo_storage_http_class_init(BonoboStorageHTTPClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	BonoboStorageClass *sclass = BONOBO_STORAGE_CLASS (klass);
	
	bonobo_storage_http_parent_class = 
		gtk_type_class (bonobo_storage_get_type ());

	sclass->get_info       = http_get_info;
	sclass->set_info       = http_set_info;
	sclass->open_stream    = http_open_stream;
	sclass->open_storage   = http_open_storage;
	sclass->copy_to        = http_copy_to;
	sclass->rename         = http_rename;
	sclass->commit         = http_commit;
	sclass->revert         = http_revert;
	sclass->list_contents  = http_list_contents;
	
	object_class->destroy = bonobo_storage_http_destroy;
} /* bonobo_storage_http_class_init */

static void
bonobo_storage_init(BonoboObject *object)
{
} /* bonobo_storage_init */

GtkType
bonobo_storage_http_get_type(void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/StorageHTTP:1.0",
			sizeof (BonoboStorageHTTP),
			sizeof (BonoboStorageHTTPClass),
			(GtkClassInitFunc) bonobo_storage_http_class_init,
			(GtkObjectInitFunc) bonobo_storage_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_storage_get_type (), &info);
	}

	return type;
} /* bonobo_storage_http_get_type */

/** 
 * bonobo_storage_http_open:
 * @url: URL that represents the storage
 * @flags: open flags.
 * @mode: mode used if @flags containst Bonobo_Storage_CREATE for the storage.
 * @ev: A corba environment for exception handling.
 *
 * Returns a BonoboStorage object that represents the storage at @path
 */
BonoboStorage *
bonobo_storage_http_open(const char *url, gint flags, gint mode, CORBA_Environment *ev)
{
	BonoboStorageHTTP *storage;
	Bonobo_Storage corba_storage;

	g_return_val_if_fail (url != NULL, NULL);

	if (flags & Bonobo_Storage_CREATE || flags & Bonobo_Storage_WRITE)
		return NULL;
	
	storage = gtk_type_new(bonobo_storage_http_get_type());
	storage->url = g_strdup(url);

	storage->request = ghttp_request_new();
	if (ghttp_set_uri(storage->request, storage->url))
		return NULL;
	ghttp_set_header(storage->request, http_hdr_Connection, "close");
	if (ghttp_prepare(storage->request))
		return NULL;

	corba_storage = bonobo_storage_corba_object_create(
		BONOBO_OBJECT(storage));
	if (corba_storage == CORBA_OBJECT_NIL) {
		bonobo_object_unref(BONOBO_OBJECT(storage));
		return NULL;
	}

	return bonobo_storage_construct(
		BONOBO_STORAGE(storage), corba_storage);
} /* bonobo_storage_http_open */

gint 
init_storage_plugin(StoragePlugin *plugin)
{
	g_return_val_if_fail (plugin != NULL, -1);

	plugin->name = "http";
	plugin->description = "HTTP driver";
	plugin->version = BONOBO_STORAGE_VERSION;
	
	plugin->storage_open = bonobo_storage_http_open; 
	plugin->stream_open = bonobo_stream_http_open; 

	return 0;
} /* init_storage_plugin */

