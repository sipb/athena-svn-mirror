/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-serializer.h: Asyncronous Callback-based SOAP Request Queue.
 *
 * Authors:
 *      Alex Graveley (alex@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */

#ifndef SOUP_SERIALIZER_H
#define SOUP_SERIALIZER_H 1

#include <time.h>
#include <glib.h>
#include <libxml/tree.h>

#include <libsoup/soup-message.h>

typedef struct _SoupSerializer SoupSerializer;

SoupSerializer *soup_serializer_new                   (void);

SoupSerializer *soup_serializer_new_full              (gboolean        standalone,
						       const gchar    *xml_encoding,
						       const gchar    *env_prefix,
						       const gchar    *env_uri);

void            soup_serializer_free                  (SoupSerializer *ser);

xmlDocPtr       soup_serializer_get_xml_doc           (SoupSerializer *ser);

void            soup_serializer_start_envelope        (SoupSerializer *ser);
void            soup_serializer_end_envelope          (SoupSerializer *ser);

void            soup_serializer_start_body            (SoupSerializer *ser);
void            soup_serializer_end_body              (SoupSerializer *ser);

void            soup_serializer_start_element         (SoupSerializer *ser,
						       const gchar    *name,
						       const gchar    *prefix,
						       const gchar    *ns_uri);
void            soup_serializer_end_element           (SoupSerializer *ser);

void            soup_serializer_start_fault           (SoupSerializer *ser,
						       const gchar    *faultcode,
						       const gchar    *faultstring,
						       const gchar    *faultactor);
void            soup_serializer_end_fault             (SoupSerializer *ser);

void            soup_serializer_start_fault_detail    (SoupSerializer *ser);
void            soup_serializer_end_fault_detail      (SoupSerializer *ser);

void            soup_serializer_start_header          (SoupSerializer *ser);
void            soup_serializer_end_header            (SoupSerializer *ser);

void            soup_serializer_start_header_element  (SoupSerializer *ser,
						       const gchar    *name,
						       gboolean        must_understand,
						       const gchar    *actor_uri,
						       const gchar    *prefix,
						       const gchar    *ns_uri);
void            soup_serializer_end_header_element    (SoupSerializer *ser);

void            soup_serializer_write_int             (SoupSerializer *ser,
						       glong           i);

void            soup_serializer_write_double          (SoupSerializer *ser,
						       gdouble         d);

void            soup_serializer_write_base64          (SoupSerializer *ser,
						       const gchar    *string,
						       guint           len);

void            soup_serializer_write_time            (SoupSerializer *ser,
						       const time_t   *timeval);

void            soup_serializer_write_string          (SoupSerializer *ser,
						       const gchar    *string);

void            soup_serializer_write_buffer          (SoupSerializer *ser,
						       const gchar    *buffer,
						       guint           length);

void            soup_serializer_set_type              (SoupSerializer *ser,
						       const gchar    *xsi_type);

void            soup_serializer_set_null              (SoupSerializer *ser);

void            soup_serializer_add_attribute         (SoupSerializer *ser,
						       const gchar    *name,
						       const gchar    *value,
						       const gchar    *prefix,
						       const gchar    *ns_uri);

void            soup_serializer_add_namespace         (SoupSerializer *ser,
						       const gchar    *prefix,
						       const gchar    *ns_uri);

void            soup_serializer_set_default_namespace (SoupSerializer *ser,
						       const gchar    *ns_uri);

const gchar *   soup_serializer_get_namespace_prefix  (SoupSerializer *ser,
						       const gchar    *ns_uri);

void            soup_serializer_set_encoding_style    (SoupSerializer *ser,
						       const gchar    *enc_style);

void            soup_serializer_reset                 (SoupSerializer *ser);

void            soup_serializer_persist               (SoupSerializer *ser,
						       SoupDataBuffer *dest);

#endif /* SOUP_SERIALIZER_H */
