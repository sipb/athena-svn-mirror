/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-parse.h: Runtime SOAP document parser
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_PARSE_H_
#define _WSDL_SOAP_PARSE_H_

#include <glib.h>
#include <libsoup/soup-env.h>
#include <libwsdl/wsdl-param.h>

guint wsdl_soap_parse (const guchar             *xmltext,
		       const guchar             *operation,
		       const wsdl_param * const  params,
		       SoupEnv                  *env,
		       gint                      flags);

#endif /* _WSDL_SOAP_PARSE_H_ */
