/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-parser.h: Parser for SOAP requests/responses.
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef SOUP_PARSER_H
#define SOUP_PARSER_H

#include <libsoup/soup-serializer.h>
#include <libsoup/soup-fault.h>

typedef enum {
	SOUP_PARAM_TYPE_INVALID,
	SOUP_PARAM_TYPE_STRING,
	SOUP_PARAM_TYPE_STRUCT
} SoupParamType;

typedef struct {
	gchar         *name;
	SoupParamType  type;
	gpointer       data;
} SoupParam;

typedef struct _SoupParamList SoupParamList;

SoupParam     *soup_param_new                   (void);
SoupParam     *soup_param_new_full              (const gchar       *name, 
						 SoupParamType      type, 
						 gpointer           value);
const gchar   *soup_param_get_name              (SoupParam         *param);
void           soup_param_set_name              (SoupParam         *param, 
						 const gchar       *new_name);
SoupParamType  soup_param_get_type              (SoupParam         *param);
void           soup_param_set_type              (SoupParam         *param, 
						 SoupParamType      new_type);
gpointer       soup_param_get_data              (SoupParam         *param);
void           soup_param_set_data              (SoupParam         *param, 
						 gpointer           new_value);
void           soup_param_free                  (SoupParam         *param);

typedef void (*SoupParamListFunc) (const gchar *name, 
				   SoupParam   *param, 
				   gpointer     user_data);

SoupParamList *soup_param_list_new              (void);
void           soup_param_list_add              (SoupParamList     *plist, 
						 SoupParam         *param);
SoupParam     *soup_param_list_get_by_name      (SoupParamList     *plist, 
						 const gchar       *name);
void           soup_param_list_foreach          (SoupParamList     *plist,
						 SoupParamListFunc  func,
						 gpointer           user_data);
void           soup_param_list_free             (SoupParamList     *plist);

typedef struct _SoupParser SoupParser;

SoupParser    *soup_parser_new_from_string      (const gchar       *str);
SoupParser    *soup_parser_new_from_data_buffer (SoupDataBuffer    *buf);
SoupFault     *soup_parser_get_fault            (SoupParser        *parser);
const gchar   *soup_parser_get_method_name      (SoupParser        *parser);
SoupParamList *soup_parser_get_param_list       (SoupParser        *parser);
SoupParam     *soup_parser_get_param_by_name    (SoupParser        *parser, 
						 const gchar       *name);
void           soup_parser_free                 (SoupParser        *parser);

#endif /* SOUP_PARSER_H */
