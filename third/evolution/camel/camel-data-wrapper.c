/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; -*- */
/* camel-data-wrapper.c : Abstract class for a data_wrapper */

/*
 *
 * Authors: Bertrand Guiheneuf <bertrand@helixcode.com>
 *
 * Copyright 1999, 2000 Ximian, Inc. (www.ximian.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include "camel-data-wrapper.h"
#include "camel-mime-utils.h"
#include "camel-stream.h"
#include "camel-exception.h"
#include "camel-private.h"

#define d(x)

static CamelObjectClass *parent_class = NULL;

/* Returns the class for a CamelDataWrapper */
#define CDW_CLASS(so) CAMEL_DATA_WRAPPER_CLASS (CAMEL_OBJECT_GET_CLASS(so))

static int construct_from_stream(CamelDataWrapper *, CamelStream *);
static int write_to_stream (CamelDataWrapper *data_wrapper, CamelStream *stream);
static void set_mime_type (CamelDataWrapper *data_wrapper, const gchar *mime_type);
static gchar *get_mime_type (CamelDataWrapper *data_wrapper);
static CamelContentType *get_mime_type_field (CamelDataWrapper *data_wrapper);
static void set_mime_type_field (CamelDataWrapper *data_wrapper, CamelContentType *mime_type);
static gboolean is_offline (CamelDataWrapper *data_wrapper);

static void
camel_data_wrapper_class_init (CamelDataWrapperClass *camel_data_wrapper_class)
{
	parent_class = camel_type_get_global_classfuncs (camel_object_get_type ());

	/* virtual method definition */
	camel_data_wrapper_class->write_to_stream = write_to_stream;
	camel_data_wrapper_class->set_mime_type = set_mime_type;
	camel_data_wrapper_class->get_mime_type = get_mime_type;
	camel_data_wrapper_class->get_mime_type_field = get_mime_type_field;
	camel_data_wrapper_class->set_mime_type_field = set_mime_type_field;
	camel_data_wrapper_class->construct_from_stream = construct_from_stream;
	camel_data_wrapper_class->is_offline = is_offline;
}

static void
camel_data_wrapper_init (gpointer object, gpointer klass)
{
	CamelDataWrapper *camel_data_wrapper = CAMEL_DATA_WRAPPER (object);
	
	camel_data_wrapper->priv = g_malloc (sizeof (struct _CamelDataWrapperPrivate));
#ifdef ENABLE_THREADS
	pthread_mutex_init (&camel_data_wrapper->priv->stream_lock, NULL);
#endif
	
	camel_data_wrapper->mime_type = header_content_type_new ("application", "octet-stream");
	camel_data_wrapper->offline = FALSE;
	camel_data_wrapper->rawtext = TRUE;
}

static void
camel_data_wrapper_finalize (CamelObject *object)
{
	CamelDataWrapper *camel_data_wrapper = CAMEL_DATA_WRAPPER (object);
	
#ifdef ENABLE_THREADS
	pthread_mutex_destroy (&camel_data_wrapper->priv->stream_lock);
#endif
	g_free (camel_data_wrapper->priv);
	
	if (camel_data_wrapper->mime_type)
		header_content_type_unref (camel_data_wrapper->mime_type);
	
	if (camel_data_wrapper->stream)
		camel_object_unref (CAMEL_OBJECT (camel_data_wrapper->stream));
}

CamelType
camel_data_wrapper_get_type (void)
{
	static CamelType camel_data_wrapper_type = CAMEL_INVALID_TYPE;

	if (camel_data_wrapper_type == CAMEL_INVALID_TYPE) {
		camel_data_wrapper_type = camel_type_register (CAMEL_OBJECT_TYPE, "CamelDataWrapper",
							       sizeof (CamelDataWrapper),
							       sizeof (CamelDataWrapperClass),
							       (CamelObjectClassInitFunc) camel_data_wrapper_class_init,
							       NULL,
							       (CamelObjectInitFunc) camel_data_wrapper_init,
							       (CamelObjectFinalizeFunc) camel_data_wrapper_finalize);
	}

	return camel_data_wrapper_type;
}

static int
write_to_stream (CamelDataWrapper *data_wrapper, CamelStream *stream)
{
	int ret;
	
	if (data_wrapper->stream == NULL) {
		return -1;
	}
	
	CAMEL_DATA_WRAPPER_LOCK (data_wrapper, stream_lock);
	if (camel_stream_reset (data_wrapper->stream) == -1) {
		CAMEL_DATA_WRAPPER_UNLOCK (data_wrapper, stream_lock);
		return -1;
	}
	
	ret = camel_stream_write_to_stream (data_wrapper->stream, stream);
	CAMEL_DATA_WRAPPER_UNLOCK (data_wrapper, stream_lock);
	
	return ret;
}

CamelDataWrapper *
camel_data_wrapper_new(void)
{
	return (CamelDataWrapper *)camel_object_new(camel_data_wrapper_get_type());
}

/**
 * camel_data_wrapper_write_to_stream:
 * @data_wrapper: a data wrapper
 * @stream: stream for data to be written to
 * @ex: a CamelException
 *
 * Writes the data content to @stream in a machine-independent format
 * appropriate for the data. It should be possible to construct an
 * equivalent data wrapper object later by passing this stream to
 * camel_data_construct_from_stream().
 *
 * Return value: the number of bytes written, or -1 if an error occurs.
 **/
int
camel_data_wrapper_write_to_stream (CamelDataWrapper *data_wrapper,
				    CamelStream *stream)
{
	g_return_val_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper), -1);
	g_return_val_if_fail (CAMEL_IS_STREAM (stream), -1);

	return CDW_CLASS (data_wrapper)->write_to_stream (data_wrapper, stream);
}

static int
construct_from_stream (CamelDataWrapper *data_wrapper, CamelStream *stream)
{
	if (data_wrapper->stream)
		camel_object_unref((CamelObject *)data_wrapper->stream);

	data_wrapper->stream = stream;
	camel_object_ref (CAMEL_OBJECT (stream));
	return 0;
}

/**
 * camel_data_wrapper_construct_from_stream:
 * @data_wrapper: a data wrapper
 * @stream: A stream that can be read from.
 *
 * Constructs the content of the data wrapper from the
 * supplied @stream.
 *
 * Return value: -1 on error.
 **/
int
camel_data_wrapper_construct_from_stream (CamelDataWrapper *data_wrapper,
					  CamelStream *stream)
{
	g_return_val_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper), -1);
	g_return_val_if_fail (CAMEL_IS_STREAM (stream), -1);
	
	return CDW_CLASS (data_wrapper)->construct_from_stream (data_wrapper, stream);
}


static void
set_mime_type (CamelDataWrapper *data_wrapper, const gchar *mime_type)
{
	if (data_wrapper->mime_type)
		header_content_type_unref (data_wrapper->mime_type);
	data_wrapper->mime_type = header_content_type_decode (mime_type);
}

/**
 * camel_data_wrapper_set_mime_type:
 * @data_wrapper: a data wrapper
 * @mime_type: the text representation of a MIME type
 *
 * This sets the data wrapper's MIME type.
 * It might fail, but you won't know. It will allow you to set
 * Content-Type parameters on the data wrapper, which are meaningless.
 * You should not be allowed to change the MIME type of a data wrapper
 * that contains data, or at least, if you do, it should invalidate the
 * data.
 **/
void
camel_data_wrapper_set_mime_type (CamelDataWrapper *data_wrapper,
				  const gchar *mime_type)
{
	g_return_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper));
	g_return_if_fail (mime_type != NULL);

	CDW_CLASS (data_wrapper)->set_mime_type (data_wrapper, mime_type);
}

static gchar *
get_mime_type (CamelDataWrapper *data_wrapper)
{
	return header_content_type_simple (data_wrapper->mime_type);
}

/**
 * camel_data_wrapper_get_mime_type:
 * @data_wrapper: a data wrapper
 *
 * Return value: the text form of the data wrapper's MIME type,
 * which the caller must free.
 **/
gchar *
camel_data_wrapper_get_mime_type (CamelDataWrapper *data_wrapper)
{
	g_return_val_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper), NULL);

	return CDW_CLASS (data_wrapper)->get_mime_type (data_wrapper);
}


static CamelContentType *
get_mime_type_field (CamelDataWrapper *data_wrapper)
{
	return data_wrapper->mime_type;
}

/**
 * camel_data_wrapper_get_mime_type_field:
 * @data_wrapper: a data wrapper
 *
 * Return value: the parsed form of the data wrapper's MIME type
 **/
CamelContentType *
camel_data_wrapper_get_mime_type_field (CamelDataWrapper *data_wrapper)
{
	g_return_val_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper), NULL);

	return CDW_CLASS (data_wrapper)->get_mime_type_field (data_wrapper);
}

/**
 * camel_data_wrapper_set_mime_type_field:
 * @data_wrapper: a data wrapper
 * @mime_type: the parsed representation of a MIME type
 *
 * This sets the data wrapper's MIME type. It suffers from the same
 * flaws as camel_data_wrapper_set_mime_type.
 **/
static void
set_mime_type_field (CamelDataWrapper *data_wrapper,
		     CamelContentType *mime_type)
{
	g_return_if_fail (CAMEL_IS_DATA_WRAPPER (data_wrapper));
	g_return_if_fail (mime_type != NULL);

	if (data_wrapper->mime_type)
		header_content_type_unref (data_wrapper->mime_type);
	data_wrapper->mime_type = mime_type;
	if (mime_type)
		header_content_type_ref (data_wrapper->mime_type);
}

void
camel_data_wrapper_set_mime_type_field (CamelDataWrapper *data_wrapper,
					CamelContentType *mime_type)
{
	CDW_CLASS (data_wrapper)->set_mime_type_field (data_wrapper, mime_type);
}


static gboolean
is_offline (CamelDataWrapper *data_wrapper)
{
	return data_wrapper->offline;
}

/**
 * camel_data_wrapper_is_offline:
 * @data_wrapper: a data wrapper
 *
 * Return value: whether @data_wrapper is "offline" (data stored
 * remotely) or not. Some optional code paths may choose to not
 * operate on offline data.
 **/
gboolean
camel_data_wrapper_is_offline (CamelDataWrapper *data_wrapper)
{
	return CDW_CLASS (data_wrapper)->is_offline (data_wrapper);
}