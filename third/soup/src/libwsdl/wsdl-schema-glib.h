/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-schema-glib.h: Build typecodes from XML describing a simple
 * glib-based type system
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SCHEMA_GLIB_H_
#define _WSDL_SCHEMA_GLIB_H_

#include <glib.h>
#include <libxml/tree.h>
#include <libwsdl/wsdl-typecodes.h>

void wsdl_schema_glib_start_element (const xmlDocPtr    doc,
				     const xmlNodePtr   node,
				     const xmlChar    **attrs,
				     const guchar      *ns,
				     const guchar      *nsuri,
				     WsdlErrorMsgFn     error_msg);

void wsdl_schema_glib_end_element   (const xmlNodePtr   node,
				     WsdlErrorMsgFn     error_msg);

#endif /* _WSDL_SCHEMA_GLIB_H_ */
