/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-marshal.h: Runtime SOAP XML document construction
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_MARSHAL_H_
#define _WSDL_SOAP_MARSHAL_H_

#include <glib.h>
#include <libsoup/soup-message.h>
#include <libsoup/soup-env.h>
#include <libwsdl/wsdl-param.h>

guint wsdl_soap_marshal (const guchar * const      operation,
			 const guchar * const      ns,
			 const guchar * const      ns_uri,
			 const wsdl_param * const  params,
			 SoupDataBuffer           *buffer,
			 SoupEnv                  *env,
			 gint                      flags);

#endif /* _WSDL_SOAP_MARSHAL_H_ */
