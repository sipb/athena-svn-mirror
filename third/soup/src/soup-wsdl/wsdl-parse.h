/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-parse.h: Parse a WSDL file, building up a tree of structs
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_PARSE_H_
#define _WSDL_PARSE_H_

#include <glib.h>
#include <libxml/tree.h>

#include <libwsdl/wsdl-typecodes.h>

typedef struct {
	GString *documentation;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this types
					 * element
					 */
} wsdl_types;

typedef struct {
	guchar *name;
	guchar *targetNamespace;
	GString *documentation;

	wsdl_types *types;
	GSList *messages;		/* a list of wsdl_message */
	GSList *porttypes;		/* a list of wsdl_porttype */
	GSList *bindings;		/* a list of wsdl_binding */
	GSList *services;		/* a list of wsdl_service */

	/* Threaded structs: */
	GSList *thread_soap_services;	/* a list of wsdl_service */
	
	xmlDocPtr xml;			/* the entire XML document */
} wsdl_definitions;

typedef struct {
	guchar *name;
	GString *documentation;
	GSList *parts;			/* a list of wsdl_message_part */

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this message
					 * element
					 */
} wsdl_message;

typedef struct {
	guchar *name;
	const wsdl_typecode *typecode;

	/* Threaded structs: */

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this message part
					 * element
					 */
} wsdl_message_part;

typedef struct {
	guchar *name;
	GString *documentation;
	GSList *operations;		/* a list of wsdl_porttype_operation */

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this portType
					 * element
					 */
} wsdl_porttype;

typedef struct {
	guchar *name;
	guchar *message;
	GString *documentation;

	/* Threaded structs: */
	wsdl_message *thread_soap_message;
	GSList *thread_soap_parts;	/* a list of wsdl_message_part */
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this portType
					 * operation input, output or
					 * fault element
					 */
} wsdl_porttype_operation_inoutfault;

typedef struct {
	guchar *name;
	GString *documentation;
	gboolean solicit;
	wsdl_porttype_operation_inoutfault *input;
	wsdl_porttype_operation_inoutfault *output;
	GSList *faults;	/* a list of wsdl_porttype_operation_inoutfault */

	/* Threaded structs */
	GSList thread_soap_parts;	/* a list of wsdl_message_part */

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this portType
					 * operation element
					 */
} wsdl_porttype_operation;

typedef struct {
	guchar *style;
	guchar *transport;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * binding element
					 */
} wsdl_soap_binding;

typedef struct {
	guchar *name;
	guchar *type;
	GString *documentation;
	wsdl_soap_binding *soap_binding;
	GSList *operations;		/* a list of wsdl_binding_operation */

	/* Threaded structs: */
	wsdl_porttype *thread_soap_porttype;
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this binding
					 * element
					 */
} wsdl_binding;

typedef struct {
	guchar *parts;
	guchar *use;
	guchar *encodingStyle;
	guchar *namespace;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * body element
					 */
} wsdl_soap_body;

typedef struct {
	guchar *name;
	guchar *use;
	guchar *encodingStyle;
	guchar *namespace;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * fault element
					 */
} wsdl_soap_fault;

typedef struct {
	guchar *name;
	GString *documentation;
	wsdl_soap_body *soap_body;
	wsdl_soap_fault *soap_fault;
	GSList *soap_headers;		/* a list of wsdl_soap_header */
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this binding
					 * operation input, output or
					 * fault element
					 */
} wsdl_binding_operation_inoutfault;

typedef struct {
	guchar *element;
	guchar *fault;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * header element
					 */
} wsdl_soap_header;

typedef struct {
	guchar *soapAction;
	guchar *style;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * operation element
					 */
} wsdl_soap_operation;

typedef struct {
	guchar *name;
	GString *documentation;
	gboolean solicit;
	wsdl_soap_operation *soap_operation;
	wsdl_binding_operation_inoutfault *input;
	wsdl_binding_operation_inoutfault *output;
	GSList *faults;	/* a list of wsdl_binding_operation_inoutfault */

	/* Threaded structs: */
	wsdl_porttype_operation *thread_soap_porttype_operation;
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this binding
					 * operation element
					 */
} wsdl_binding_operation;

typedef struct {
	guchar *name;
	GString *documentation;
	GSList *ports;			/* a list of wsdl_service_port */

	/* Threaded structs: */
	GSList *thread_soap_ports;	/* a list of wsdl_service_port */
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this service
					 * element
					 */
} wsdl_service;

typedef struct {
	guchar *location;

	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this soap
					 * address element
					 */
} wsdl_soap_address;

typedef struct {
	guchar *name;
	guchar *binding;
	GString *documentation;
	wsdl_soap_address *soap_address;

	/* Threaded structs: */
	wsdl_binding *thread_soap_binding;
	
	xmlNodePtr xml_node;		/* the fragment of the xml doc
					 * that contains this service
					 * port element
					 */
} wsdl_service_port;

wsdl_definitions *wsdl_parse (const char *file);

#endif /* _WSDL_PARSE_H_ */
