/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-schema.h: Build typecodes from XML describing one of the known
 * type systems.
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SCHEMA_H_
#define _WSDL_SCHEMA_H_

#include <glib.h>
#include <libxml/tree.h>

#define WSDLNS "http://schemas.xmlsoap.org/wsdl/"
#define SOAPNS "http://schemas.xmlsoap.org/wsdl/soap/"
#define XSDNS  "http://www.w3.org/1999/XMLSchema"
#define GLIBNS "http://ximian.com/soup/glib/1.0/"

gboolean      wsdl_qnamecmp             (const xmlNodePtr   node,
					 const xmlChar     *ns,
					 const xmlChar     *localname);
gboolean      wsdl_attrnscmp            (const xmlNodePtr   node,
					 const guchar      *attr,
					 const guchar      *ns_href);
const guchar *wsdl_prefix_to_namespace  (const xmlDocPtr    doc,
					 const xmlNodePtr   node,
					 const guchar      *str,
					 gboolean           defns);

typedef void (*WsdlErrorMsgFn) (const gchar *fmt, ...);

gboolean      wsdl_schema_init          (const xmlNodePtr   node,
					 const xmlChar    **attrs,
					 WsdlErrorMsgFn     error_msg);
void          wsdl_schema_start_element (const xmlDocPtr    doc,
					 const xmlNodePtr   node,
					 const xmlChar    **attrs,
					 const guchar      *ns,
					 const guchar      *nsuri);
void          wsdl_schema_end_element   (const xmlNodePtr   node);

#endif /* _WSDL_SCHEMA_H_ */
