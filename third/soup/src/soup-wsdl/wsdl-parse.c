/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-parse.c: Parse a WSDL file, building up a tree of structs
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlmemory.h>

#include <libwsdl/wsdl-schema.h>

#include "wsdl.h"
#include "wsdl-parse.h"
#include "wsdl-trace.h"

static int warnings;
static int errors;
static int fatalities;

static xmlParserCtxtPtr xml_ctxt;

/*
 * State diagram:
 *
 * WSDL_DEFINITIONS
 *	IMPORT
 *	WSDL_DOCUMENTATION
 *	WSDL_TYPES
 *		WSDL_DOCUMENTATION
 *		XSD_SCHEMA
 *		!!!extensibility element
 *	WSDL_MESSAGE
 *		WSDL_DOCUMENTATION
 *		PART
 *	WSDL_PORTTYPE
 *		WSDL_DOCUMENTATION
 *		WSDL_OPERATION
 *			WSDL_DOCUMENTATION
 *			WSDL_INPUT
 *				WSDL_DOCUMENTATION
 *			WSDL_OUTPUT
 *				WSDL_DOCUMENTATION
 *			WSDL_FAULT
 *				WSDL_DOCUMENTATION
 *	WSDL_BINDING
 *		WSDL_DOCUMENTATION
 *		!!!extensibility element
 *		WSDL_OPERATION
 *			WSDL_DOCUMENTATION
 *			!!!extensibility element
 *			WSDL_INPUT
 *				WSDL_DOCUMENTATION
 *				!!!extensibility element
 *			WSDL_OUTPUT
 *				WSDL_DOCUMENTATION
 *				!!!extensibility element
 *			WSDL_FAULT
 *				WSDL_DOCUMENTATION
 *				!!!extensibility element
 *	WSDL_SERVICE
 *		WSDL_DOCUMENTATION
 *		WSDL_PORT
 *			WSDL_DOCUMENTATION
 *			!!!extensibility element
 *		!!!extensibility element
 *	!!!extensibility element
 */

typedef enum {
	WSDL_START,
	WSDL_DEFINITIONS,
	WSDL_DOCUMENTATION,
	WSDL_TYPES,
	WSDL_TYPES_SCHEMA,
	WSDL_MESSAGE,
	WSDL_MESSAGE_PART,
	WSDL_PORTTYPE,
	WSDL_PORTTYPE_OPERATION,
	WSDL_PORTTYPE_OPERATION_INPUT,
	WSDL_PORTTYPE_OPERATION_OUTPUT,
	WSDL_PORTTYPE_OPERATION_FAULT,
	WSDL_BINDING,
	WSDL_BINDING_OPERATION,
	WSDL_BINDING_OPERATION_INPUT,
	WSDL_BINDING_OPERATION_OUTPUT,
	WSDL_SOAP_BODY,
	WSDL_SOAP_HEADER,
	WSDL_SOAP_FAULT,
	WSDL_BINDING_OPERATION_FAULT,
	WSDL_SOAP_OPERATION,
	WSDL_SOAP_BINDING,
	WSDL_SERVICE,
	WSDL_SERVICE_PORT,
	WSDL_SOAP_ADDRESS,
	WSDL_FINISH,
	WSDL_UNKNOWN,
	WSDL_STATE_MAX,
} wsdl_state_t;

/* Keep this synchronised with wsdl_state_t */
static const char *states[] = {
	"START",
	"DEFINITIONS",
	"DOCUMENTATION",
	"TYPES",
	"TYPES_SCHEMA",
	"MESSAGE",
	"MESSAGE_PART",
	"PORTTYPE",
	"PORTTYPE_OPERATION",
	"PORTTYPE_OPERATION_INPUT",
	"PORTTYPE_OPERATION_OUTPUT",
	"PORTTYPE_OPERATION_FAULT",
	"BINDING",
	"BINDING_OPERATION",
	"BINDING_OPERATION_INPUT",
	"BINDING_OPERATION_OUTPUT",
	"SOAP:BODY",
	"SOAP:HEADER",
	"SOAP:FAULT",
	"BINDING_OPERATION_FAULT",
	"SOAP:OPERATION",
	"SOAP:BINDING",
	"SERVICE",
	"SERVICE_PORT",
	"SOAP:ADDRESS",
	"FINISH",
	"UNKNOWN",
	"*Unknown*",
};

typedef struct {
	wsdl_state_t state, last_known_state, doc_prev_state,
		binding_operation_state;
	guint unknown_depth, types_depth;
	GString **documentation;
	wsdl_definitions *definitions;
	wsdl_types *types;
	wsdl_message *message;
	wsdl_message_part *message_part;
	wsdl_porttype *porttype;
	wsdl_porttype_operation *porttype_operation;
	wsdl_porttype_operation_inoutfault *porttype_operation_iof;
	wsdl_binding *binding;
	wsdl_binding_operation *binding_operation;
	wsdl_binding_operation_inoutfault *binding_operation_iof;
	wsdl_soap_operation *soap_operation;
	wsdl_soap_body *soap_body;
	wsdl_soap_header *soap_header;
	wsdl_soap_fault *soap_fault;
	wsdl_soap_binding *soap_binding;
	wsdl_service *service;
	wsdl_service_port *service_port;
	wsdl_soap_address *soap_address;
} wsdl_state;

static gchar *
wsdl_get_location (void)
{
	return (g_strdup_printf
		("[%s]: (line %d, column %d)",
		 xmlDefaultSAXLocator.getSystemId (xml_ctxt),
		 xmlDefaultSAXLocator.getLineNumber (xml_ctxt),
		 xmlDefaultSAXLocator.getColumnNumber (xml_ctxt)));
}

static void
wsdl_parse_warning (void *unused G_GNUC_UNUSED, const char *err, ...)
{
	va_list args;
	gchar *loc, *msg;

	loc = wsdl_get_location ();

	va_start (args, err);
	msg = g_strdup_vprintf (err, args);
	va_end (args);

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_WARNING, 
		    "%s: %s", 
		    loc,
		    msg);

	g_free (loc);
	g_free (msg);

	warnings++;
}

static void
wsdl_parse_error (void *unused G_GNUC_UNUSED, const char *err, ...)
{
	va_list args;
	gchar *loc, *msg;

	loc = wsdl_get_location ();

	va_start (args, err);
	msg = g_strdup_vprintf (err, args);
	va_end (args);

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_CRITICAL, 
		    "%s: %s", 
		    loc,
		    msg);

	g_free (loc);
	g_free (msg);

	errors++;
}

static void
wsdl_parse_fatal (void *unused G_GNUC_UNUSED, const char *err, ...)
{
	va_list args;
	gchar *loc, *msg;

	loc = wsdl_get_location ();

	va_start (args, err);
	msg = g_strdup_vprintf (err, args);
	va_end (args);

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_ERROR, 
		    "%s: %s", 
		    loc,
		    msg);

	g_free (loc);
	g_free (msg);

	/* Yes, glib will have already abort()ed, but just in case
	 * that behaviour ever changes... */
	fatalities++;
}

static void
wsdl_parse_schema_error (const gchar * fmt, ...)
{
	va_list args;
	gchar *msg;

	va_start (args, fmt);
	msg = g_strdup_vprintf (fmt, args);
	va_end (args);

	wsdl_parse_warning (NULL, "%s", msg);

	g_free (msg);
}

static void
wsdl_parse_definitions_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->definitions->name =
					g_strdup (attrs [i + 1]);
			} 
			else if (!strcmp (attrs[i], "targetNamespace")) {
				state->definitions->targetNamespace =
					g_strdup (attrs [i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown definitions "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}
}

static void
wsdl_parse_message_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->message->name = g_strdup (attrs [i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown message "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->message->name == NULL)
		wsdl_parse_error (state, "message requires a 'name' attribute");
}

static void
wsdl_parse_message_part_attrs (const xmlDocPtr doc, const xmlNodePtr node,
			       wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->message_part->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "element") ||
				 !strcmp (attrs[i], "type")) {
				const guchar *tnsuri;

				tnsuri =
					wsdl_prefix_to_namespace (doc, 
								  node,
								  attrs[i + 1],
								  FALSE);

				state->message_part->typecode =
					wsdl_typecode_lookup (attrs [i + 1],
							      tnsuri);

				if (state->message_part->typecode == NULL) {
					wsdl_parse_error (state,
							  "Unknown type [%s]",
							  attrs[i + 1]);
				}
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown message part "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->message_part->name == NULL)
		wsdl_parse_error (state,
				  "message part requires a 'name' attribute");

	if (state->message_part->typecode == NULL)
		wsdl_parse_error (state,
				  "message part requires one of either 'type' "
				  "or 'element' attributes");
}

static void
wsdl_parse_porttype_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->porttype->name = g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown portType "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->porttype->name == NULL)
		wsdl_parse_error (state,
				  "portType requires a 'name' attribute");
}

static void
wsdl_parse_porttype_operation_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->porttype_operation->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				g_warning
					("Unknown portType operation "
					 "attribute [%s]",
					 attrs[i]);
			}

			i += 2;
		}
	}

	if (state->porttype_operation->name == NULL)
		wsdl_parse_error (state,
				  "portType operation requires a "
				  "'name' attribute");
}

static void
wsdl_parse_porttype_operation_iof_attrs (wsdl_state * state,
					 const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->porttype_operation_iof->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "message")) {
				state->porttype_operation_iof->message =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown portType "
						    "operation input, output "
						    "or fault attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->porttype_operation_iof->message == NULL)
		wsdl_parse_error (state,
				  "portType operation input, output or fault "
				  "element requires a 'message' attribute");

	if (state->porttype_operation_iof->name == NULL &&
	    state->state == WSDL_PORTTYPE_OPERATION_FAULT) {
		wsdl_parse_error (state,
				  "portType operation fault element requires "
				  "a 'name' attribute");
	}
}

static void
wsdl_parse_binding_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->binding->name = g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "type")) {
				state->binding->type = g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown binding "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->binding->name == NULL)
		wsdl_parse_error (state,
				  "binding element requires a 'name' "
				  "attribute");

	if (state->binding->type == NULL)
		wsdl_parse_error (state,
				  "binding element requires a 'type' "
				  "attribute");
}

static void
wsdl_parse_binding_operation_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->binding_operation->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				g_warning ("Unknown binding operation "
					   "attribute [%s]",
					   attrs[i]);
			}

			i += 2;
		}
	}

	if (state->binding_operation->name == NULL)
		wsdl_parse_error (state,
				  "binding operation requires a 'name' "
				  "attribute");
}

static void
wsdl_parse_binding_operation_iof_attrs (wsdl_state * state,
					const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->binding_operation_iof->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown binding operation "
						    "input, output or fault "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->binding_operation_iof->name == NULL &&
	    state->state == WSDL_BINDING_OPERATION_FAULT) {
		wsdl_parse_error (state,
				  "binding operation fault element requires "
				  "a 'name' attribute");
	}
}

static void
wsdl_parse_soap_body_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "parts")) {
				state->soap_body->parts =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "use")) {
				state->soap_body->use = g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "encodingStyle")) {
				state->soap_body->encodingStyle =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "namespace")) {
				state->soap_body->namespace =
					g_strdup (attrs[i + 1]);
			}
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown soap:body "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}
}

static void
wsdl_parse_soap_header_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "element")) {
				state->soap_header->element =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "fault")) {
				state->soap_header->fault =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown soap:header "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->soap_header->element == NULL)
		wsdl_parse_error (state,
				  "soap:header element requires an 'element' "
				  "attribute");
}

static void
wsdl_parse_soap_fault_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->soap_fault->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "use")) {
				state->soap_fault->use =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "encodingStyle")) {
				state->soap_fault->encodingStyle =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "namespace")) {
				state->soap_fault->namespace =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown soap:fault "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}
}

static void
wsdl_parse_soap_operation_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "soapAction")) {
				state->soap_operation->soapAction =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "style")) {
				state->soap_operation->style =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown soap:operation "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	/* This becomes optional for non-HTTP protocol bindings! */
	if (state->soap_operation->soapAction == NULL)
		wsdl_parse_error (state,
				  "soap:operation element requires "
				  "a 'soapAction' attribute");
}

static void
wsdl_parse_soap_binding_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "style")) {
				state->soap_binding->style =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "transport")) {
				state->soap_binding->transport =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown soap:binding "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->soap_binding->style == NULL)
		wsdl_parse_error (state,
				  "soap:binding element requires a 'style' "
				  "attribute");

	if (state->soap_binding->transport == NULL)
		wsdl_parse_error (state,
				  "soap:binding element requires a 'transport' "
				  "attribute");
}

static void
wsdl_parse_service_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->service->name = g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} 
			else {
				wsdl_parse_warning (state,
						    "Unknown service "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->service->name == NULL)
		wsdl_parse_error (state,
				  "service element requires a 'name' "
				  "attribute");
}

static void
wsdl_parse_service_port_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				state->service_port->name =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "binding")) {
				state->service_port->binding =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown service port "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->service_port->name == NULL)
		wsdl_parse_error (state,
				  "service port element requires a 'name' "
				  "attribute");

	if (state->service_port->binding == NULL)
		wsdl_parse_error (state,
				  "service port element requires a 'binding' "
				  "attribute");
}

static void
wsdl_parse_soap_address_attrs (wsdl_state * state, const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "location")) {
				state->soap_address->location =
					g_strdup (attrs[i + 1]);
			} 
			else if (!strcmp (attrs[i], "xmlns") ||
				 !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				wsdl_parse_warning (state,
						    "Unknown soap:address "
						    "attribute [%s]",
						    attrs[i]);
			}

			i += 2;
		}
	}

	if (state->soap_address->location == NULL)
		wsdl_parse_error (state,
				  "soap:address element requires a 'location' "
				  "attribute");
}

static void
wsdl_start_document (void *user)
{
	wsdl_state *state = (wsdl_state *) user;

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, G_LOG_LEVEL_INFO,
		    "Start document (state %s)", states[state->state]);

	state->state = WSDL_START;
	/* initialise any other variables here */

	xmlDefaultSAXHandler.startDocument (xml_ctxt);
}

static void
wsdl_end_document (void *unused G_GNUC_UNUSED)
{
	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, G_LOG_LEVEL_INFO, "End document\n");

	xmlDefaultSAXHandler.endDocument (xml_ctxt);
}

static void
wsdl_parse_definitions (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "import") == TRUE) {
		/* should be able to recursively call
		 * xmlSAXUserParseFile here, hopefully
		 */
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") ==
		   TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->definitions->documentation;
		*state->documentation = g_string_new ("");
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "types") == TRUE) {
		state->types = g_new0 (wsdl_types, 1);
		state->definitions->types = state->types;
		/* no attrs here */
		state->state = WSDL_TYPES;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "message") == TRUE) {
		state->message = g_new0 (wsdl_message, 1);
		state->definitions->messages =
			g_slist_append (state->definitions->messages,
					state->message);

		wsdl_parse_message_attrs (state, attrs);

		state->state = WSDL_MESSAGE;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "portType") == TRUE) {
		state->porttype = g_new0 (wsdl_porttype, 1);
		state->definitions->porttypes =
			g_slist_append (state->definitions->porttypes,
					state->porttype);

		wsdl_parse_porttype_attrs (state, attrs);

		state->state = WSDL_PORTTYPE;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "binding") == TRUE) {
		state->binding = g_new0 (wsdl_binding, 1);
		state->definitions->bindings =
			g_slist_append (state->definitions->bindings,
					state->binding);

		wsdl_parse_binding_attrs (state, attrs);

		state->state = WSDL_BINDING;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "service") == TRUE) {
		state->service = g_new0 (wsdl_service, 1);
		state->definitions->services =
			g_slist_append (state->definitions->services,
					state->service);

		wsdl_parse_service_attrs (state, attrs);

		state->state = WSDL_SERVICE;
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_types (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->types->documentation;
		*state->documentation = g_string_new ("");
	} else {
		gboolean ok;

		ok = wsdl_schema_init (xml_ctxt->node, 
				       attrs,
				       wsdl_parse_schema_error);

		if (ok == FALSE) {
			state->last_known_state = state->state;
			state->state = WSDL_UNKNOWN;

			g_assert (state->unknown_depth == 0);

			state->unknown_depth++;
		} 
		else {
			g_assert (state->types_depth == 0);

			state->types_depth++;
			state->state = WSDL_TYPES_SCHEMA;
		}
	}
}

static void
wsdl_parse_message (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "part") == TRUE) {
		state->message_part = g_new0 (wsdl_message_part, 1);
		state->message->parts =
			g_slist_append (state->message->parts,
					state->message_part);

		wsdl_parse_message_part_attrs (state->definitions->xml,
					       xml_ctxt->node, 
					       state, 
					       attrs);

		state->state = WSDL_MESSAGE_PART;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") ==
		   TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->message->documentation;
		*state->documentation = g_string_new ("");
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_porttype (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "operation") == TRUE) {
		state->porttype_operation = g_new0 (wsdl_porttype_operation, 1);
		state->porttype->operations =
			g_slist_append (state->porttype->operations,
					state->porttype_operation);

		wsdl_parse_porttype_operation_attrs (state, attrs);

		state->state = WSDL_PORTTYPE_OPERATION;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, 
				WSDLNS, 
				"documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->porttype->documentation;
		*state->documentation = g_string_new ("");
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_porttype_operation (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "input") == TRUE) {
		if (state->porttype_operation->input != NULL) {
			wsdl_parse_error (state,
					  "portType already has an 'input' "
					  "element\n");
			return;
		}

		state->porttype_operation_iof =
			g_new0 (wsdl_porttype_operation_inoutfault, 1);

		state->porttype_operation->input =
			state->porttype_operation_iof;

		wsdl_parse_porttype_operation_iof_attrs (state, attrs);

		/* Tell the difference between a request-response
		 * operation and a solicit-response operation.
		 *
		 * (Request-response has input defined before output,
		 * solicit-response output first).
		 */
		if (state->porttype_operation->output != NULL) {
			state->porttype_operation->solicit = TRUE;
		}

		state->state = WSDL_PORTTYPE_OPERATION_INPUT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "output") == TRUE) {
		if (state->porttype_operation->output != NULL) {
			wsdl_parse_error (state,
					  "portType already has an 'output' "
					  "element\n");
			return;
		}

		state->porttype_operation_iof =
			g_new0 (wsdl_porttype_operation_inoutfault, 1);

		state->porttype_operation->output =
			state->porttype_operation_iof;

		wsdl_parse_porttype_operation_iof_attrs (state, attrs);

		state->state = WSDL_PORTTYPE_OPERATION_OUTPUT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "fault") == TRUE) {
		state->porttype_operation_iof =
			g_new0 (wsdl_porttype_operation_inoutfault, 1);
		state->porttype_operation->faults =
			g_slist_append (state->porttype_operation->faults,
					state->porttype_operation_iof);

		wsdl_parse_porttype_operation_iof_attrs (state, attrs);

		state->state = WSDL_PORTTYPE_OPERATION_FAULT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, 
				WSDLNS, 
				"documentation") == TRUE) {
		if (state->porttype_operation->documentation != NULL) {
			wsdl_parse_error (state,
					  "portType already has a "
					  "'documentation' element\n");
			return;
		}

		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation =
			&state->porttype_operation->documentation;
		*state->documentation = g_string_new ("");
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_porttype_operation_iof (wsdl_state * state)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation =
			&state->porttype_operation_iof->documentation;
		*state->documentation = g_string_new ("");
	} else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_binding (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "operation") == TRUE) {
		state->binding_operation = g_new0 (wsdl_binding_operation, 1);
		state->binding->operations =
			g_slist_append (state->binding->operations,
					state->binding_operation);

		wsdl_parse_binding_operation_attrs (state, attrs);

		state->state = WSDL_BINDING_OPERATION;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, 
				WSDLNS, 
				"documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->binding->documentation;
		*state->documentation = g_string_new ("");
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "binding") == TRUE) {
		/* SOAP extension */
		state->soap_binding = g_new0 (wsdl_soap_binding, 1);

		if (state->binding->soap_binding != NULL) {
			wsdl_parse_error (state,
					  "Only one soap:binding allowed per "
					  "binding");
		} else {
			state->binding->soap_binding = state->soap_binding;
		}

		wsdl_parse_soap_binding_attrs (state, attrs);
		state->state = WSDL_SOAP_BINDING;
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_binding_operation (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "input") == TRUE) {
		if (state->binding_operation->input != NULL) {
			wsdl_parse_error (state,
					  "binding operation already has "
					  "an 'input' element\n");
			return;
		}

		state->binding_operation_iof =
			g_new0 (wsdl_binding_operation_inoutfault, 1);
		state->binding_operation->input = state->binding_operation_iof;

		wsdl_parse_binding_operation_iof_attrs (state, attrs);

		/* Tell the difference between a request-response
		 * operation and a solicit-response operation.
		 *
		 * (Request-response has input defined before output,
		 * solicit-response output first).
		 */
		if (state->binding_operation->output != NULL) {
			state->binding_operation->solicit = TRUE;
		}

		state->state = WSDL_BINDING_OPERATION_INPUT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "output") == TRUE) {
		if (state->binding_operation->output != NULL) {
			wsdl_parse_error (state,
					  "binding operation already has "
					  "an 'output' element\n");
			return;
		}

		state->binding_operation_iof =
			g_new0 (wsdl_binding_operation_inoutfault, 1);
		state->binding_operation->output = state->binding_operation_iof;

		wsdl_parse_binding_operation_iof_attrs (state, attrs);

		state->state = WSDL_BINDING_OPERATION_OUTPUT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "fault") == TRUE) {
		state->binding_operation_iof =
			g_new0 (wsdl_binding_operation_inoutfault, 1);
		state->binding_operation->faults =
			g_slist_append (state->binding_operation->faults,
					state->binding_operation_iof);

		wsdl_parse_binding_operation_iof_attrs (state, attrs);

		state->state = WSDL_BINDING_OPERATION_FAULT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, 
				WSDLNS, 
				"documentation") == TRUE) {
		if (state->binding_operation->documentation != NULL) {
			wsdl_parse_error (state,
					  "binding operation already has "
					  "a 'documentation' element\n");
			return;
		}

		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->binding_operation->documentation;
		*state->documentation = g_string_new ("");
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "operation") == TRUE) {
		/* SOAP extension */
		state->soap_operation = g_new0 (wsdl_soap_operation, 1);
		if (state->binding_operation->soap_operation != NULL) {
			wsdl_parse_error (state,
					  "Only one soap:operation allowed "
					  "per operation");
		} else {
			state->binding_operation->soap_operation =
				state->soap_operation;
		}

		wsdl_parse_soap_operation_attrs (state, attrs);
		state->state = WSDL_SOAP_OPERATION;
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_binding_operation_iof (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation =
			&state->binding_operation_iof->documentation;
		*state->documentation = g_string_new ("");
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "body") == TRUE &&
		 (state->state == WSDL_BINDING_OPERATION_INPUT ||
		  state->state == WSDL_BINDING_OPERATION_OUTPUT)) {
		/* SOAP extension */
		state->soap_body = g_new0 (wsdl_soap_body, 1);

		if (state->binding_operation_iof->soap_body != NULL) {
			wsdl_parse_error (state,
					  "Only one soap:body allowed per "
					  "input or output element");
		} else {
			state->binding_operation_iof->soap_body =
				state->soap_body;
		}

		wsdl_parse_soap_body_attrs (state, attrs);
		state->binding_operation_state = state->state;
		state->state = WSDL_SOAP_BODY;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "header") == TRUE &&
		 (state->state == WSDL_BINDING_OPERATION_INPUT ||
		  state->state == WSDL_BINDING_OPERATION_OUTPUT)) {
		/* SOAP extension */
		state->soap_header = g_new0 (wsdl_soap_header, 1);

		state->binding_operation_iof->soap_headers =
			g_slist_append (state->binding_operation_iof->
					soap_headers, state->soap_header);

		wsdl_parse_soap_header_attrs (state, attrs);
		state->binding_operation_state = state->state;
		state->state = WSDL_SOAP_HEADER;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "fault") == TRUE &&
		 state->state == WSDL_BINDING_OPERATION_FAULT) {
		/* SOAP extension */
		state->soap_fault = g_new0 (wsdl_soap_fault, 1);

		if (state->binding_operation_iof->soap_fault != NULL) {
			wsdl_parse_error (state,
					  "Only one soap:fault allowed per "
					  "fault element");
		} else {
			state->binding_operation_iof->soap_fault =
				state->soap_fault;
		}

		wsdl_parse_soap_fault_attrs (state, attrs);
		state->state = WSDL_SOAP_FAULT;
	} else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_service (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "port") == TRUE) {
		state->service_port = g_new0 (wsdl_service_port, 1);
		state->service->ports =
			g_slist_append (state->service->ports,
					state->service_port);

		wsdl_parse_service_port_attrs (state, attrs);

		state->state = WSDL_SERVICE_PORT;
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, 
				WSDLNS, 
				"documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->service->documentation;
		*state->documentation = g_string_new ("");
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_parse_service_port (wsdl_state * state, const xmlChar ** attrs)
{
	if (wsdl_qnamecmp (xml_ctxt->node, WSDLNS, "documentation") == TRUE) {
		state->doc_prev_state = state->state;
		state->state = WSDL_DOCUMENTATION;

		state->documentation = &state->service_port->documentation;
		*state->documentation = g_string_new ("");
	} 
	else if (wsdl_qnamecmp (xml_ctxt->node, SOAPNS, "address") == TRUE) {
		/* SOAP extension */
		state->soap_address = g_new0 (wsdl_soap_address, 1);
		if (state->service_port->soap_address != NULL) {
			wsdl_parse_error (state,
					  "Only one soap:address allowed per "
					  "port element");
		} else {
			state->service_port->soap_address = state->soap_address;
		}

		wsdl_parse_soap_address_attrs (state, attrs);
		state->state = WSDL_SOAP_ADDRESS;
	} 
	else {
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
	}
}

static void
wsdl_start_element (void *user, const xmlChar * name, const xmlChar ** attrs)
{
	wsdl_state *state = (wsdl_state *) user;
	int i = 0;

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_INFO,
		    "Start: [%s] (state %s)", 
		    name, 
		    states[state->state]);

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
				    G_LOG_LEVEL_DEBUG,
				    "Attribute [%s] [%s]", 
				    attrs[i],
				    attrs[i + 1]);
			i += 2;
		}
	}

	/* Do the default tree build call first, to populate the
	 * context->node pointer.
	 */
	xmlDefaultSAXHandler.startElement (xml_ctxt, name, attrs);

	switch (state->state) {
	case WSDL_START:
		if (wsdl_qnamecmp (xml_ctxt->node, 
				   WSDLNS, 
				   "definitions") == FALSE) {
			wsdl_parse_error (state,
					  "Expected 'definitions', got %s",
					  name);
		}

		state->definitions = g_new0 (wsdl_definitions, 1);

		wsdl_parse_definitions_attrs (state, attrs);

		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_DEFINITIONS:
		wsdl_parse_definitions (state, attrs);
		break;

	case WSDL_DOCUMENTATION:
		/* The spec says that elements are allowed in the
		 * documentation, but not what to do with them.
		 */
		state->last_known_state = state->state;
		state->state = WSDL_UNKNOWN;

		g_assert (state->unknown_depth == 0);

		state->unknown_depth++;
		break;

	case WSDL_TYPES:
		wsdl_parse_types (state, attrs);
		break;

	case WSDL_TYPES_SCHEMA:
		g_assert (state->definitions != NULL);

		state->types_depth++;

		wsdl_schema_start_element (state->definitions->xml,
					   xml_ctxt->node, attrs,
					   state->definitions->name,
					   state->definitions->targetNamespace);
		break;

	case WSDL_MESSAGE:
		wsdl_parse_message (state, attrs);
		break;

	case WSDL_MESSAGE_PART:
		wsdl_parse_error (state,
				  "message part may not contain elements");
		break;

	case WSDL_PORTTYPE:
		wsdl_parse_porttype (state, attrs);
		break;

	case WSDL_PORTTYPE_OPERATION:
		wsdl_parse_porttype_operation (state, attrs);
		break;

	case WSDL_PORTTYPE_OPERATION_INPUT:
	case WSDL_PORTTYPE_OPERATION_OUTPUT:
	case WSDL_PORTTYPE_OPERATION_FAULT:
		wsdl_parse_porttype_operation_iof (state);
		break;

	case WSDL_BINDING:
		wsdl_parse_binding (state, attrs);
		break;

	case WSDL_BINDING_OPERATION:
		wsdl_parse_binding_operation (state, attrs);
		break;

	case WSDL_BINDING_OPERATION_INPUT:
	case WSDL_BINDING_OPERATION_OUTPUT:
	case WSDL_BINDING_OPERATION_FAULT:
		wsdl_parse_binding_operation_iof (state, attrs);
		break;

	case WSDL_SOAP_BODY:
		wsdl_parse_error (state, 
				  "soap:body may not contain elements");
		break;

	case WSDL_SOAP_HEADER:
		wsdl_parse_error (state,
				  "soap:header may not contain elements");
		break;

	case WSDL_SOAP_FAULT:
		wsdl_parse_error (state, 
				  "soap:fault may not contain elements");
		break;

	case WSDL_SOAP_OPERATION:
		wsdl_parse_error (state,
				  "soap:operation may not contain elements");
		break;

	case WSDL_SOAP_BINDING:
		wsdl_parse_error (state,
				  "soap:binding may not contain elements");
		break;

	case WSDL_SERVICE:
		wsdl_parse_service (state, attrs);
		break;

	case WSDL_SERVICE_PORT:
		wsdl_parse_service_port (state, attrs);
		break;

	case WSDL_SOAP_ADDRESS:
		wsdl_parse_error (state,
				  "soap:address may not contain elements");
		break;

	case WSDL_FINISH:
		/* Shouldn't see more new elements here */
		g_assert_not_reached ();
		break;

	case WSDL_UNKNOWN:
		state->unknown_depth++;
		break;

	case WSDL_STATE_MAX:	/* only here for the strict compile checking */
#if 0
	default:
#endif
		g_error ("Funny state! (%d/%s)", 
			 state->state,
			 state->state < WSDL_STATE_MAX ? 
			         states[state->state] : 
			         "*Unknown*");
		break;
	}

}

static void
wsdl_end_element (void *user, const xmlChar * name)
{
	wsdl_state *state = (wsdl_state *) user;

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_INFO,
		    "End: [%s] (state %s)", 
		    name, 
		    states[state->state]);

	switch (state->state) {
	case WSDL_DEFINITIONS:
		state->state = WSDL_FINISH;
		break;

	case WSDL_DOCUMENTATION:
		state->documentation = NULL;
		state->state = state->doc_prev_state;
		break;

	case WSDL_TYPES:
		state->types->xml_node = xml_ctxt->node;
		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_TYPES_SCHEMA:
		wsdl_schema_end_element (xml_ctxt->node);

		state->types_depth--;
		if (state->types_depth == 0) {
			state->state = WSDL_TYPES;
		}
		break;

	case WSDL_MESSAGE:
		state->message->xml_node = xml_ctxt->node;
		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_MESSAGE_PART:
		state->message_part->xml_node = xml_ctxt->node;
		state->state = WSDL_MESSAGE;
		break;

	case WSDL_PORTTYPE:
		state->porttype->xml_node = xml_ctxt->node;
		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_PORTTYPE_OPERATION:
		{
			/* Set default names on input and output elements, if
			 * necessary
			 */

			wsdl_porttype_operation_inoutfault *input, *output;
			guchar *opname = state->porttype_operation->name;

			g_assert (opname != NULL);

			input = state->porttype_operation->input;
			output = state->porttype_operation->output;

			if (input != NULL && output != NULL) {
				/* This is a request-response or a
				 * solicit-response operation.
				 */
				if (input->name == NULL &&
				    state->porttype_operation->solicit == TRUE)
					input->name = g_strconcat (opname, 
								   "Solicit",
								   NULL);

				if (input->name == NULL &&
				    state->porttype_operation->solicit == FALSE)
					input->name = g_strconcat (opname, 
								   "Request",
								   NULL);

				if (output->name == NULL)
					output->name = g_strconcat (opname, 
								    "Response",
								    NULL);
			} 
			else if (input != NULL) {
				/* This must be a one-way operation */
				input->name = g_strdup (opname);
			} 
			else if (output != NULL) {
				/* This must be a notification operation */
				output->name = g_strdup (opname);
			}

			state->porttype_operation->xml_node = xml_ctxt->node;
			state->state = WSDL_PORTTYPE;
		}

		break;

	case WSDL_PORTTYPE_OPERATION_INPUT:
	case WSDL_PORTTYPE_OPERATION_OUTPUT:
	case WSDL_PORTTYPE_OPERATION_FAULT:
		state->porttype_operation_iof->xml_node = xml_ctxt->node;
		state->state = WSDL_PORTTYPE_OPERATION;
		break;

	case WSDL_BINDING:
		if (state->binding->soap_binding == NULL) {
			wsdl_parse_error (state,
					  "WSDL:binding must include a "
					  "SOAP:binding element");
		}

		state->binding->xml_node = xml_ctxt->node;
		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_BINDING_OPERATION:
		{
			/* Set default names on input and output elements, if
			 * necessary
			 */

			wsdl_binding_operation_inoutfault *input, *output;
			guchar *opname = state->binding_operation->name;

			g_assert (opname != NULL);

			input = state->binding_operation->input;
			output = state->binding_operation->output;

			if (input != NULL && output != NULL) {
				/* This is a request-response or a
				 * solicit-response operation.
				 */
				if (input->name == NULL &&
				    state->binding_operation->solicit == TRUE)
					input->name = g_strconcat (opname, 
								   "Solicit",
								   NULL);

				if (input->name == NULL &&
				    state->binding_operation->solicit == FALSE)
					input->name = g_strconcat (opname, 
								   "Request",
								   NULL);

				if (output->name == NULL)
					output->name =
						g_strconcat (opname, 
							     "Response",
							     NULL);
			} else if (input != NULL) {
				/* This must be a one-way operation */
				input->name = g_strdup (opname);
			} else if (output != NULL) {
				/* This must be a notification operation */
				output->name = g_strdup (opname);
			}

			/* 
			 * This becomes optional for non-HTTP protocol bindings!
			 */
			if (state->binding_operation->soap_operation == NULL)
				wsdl_parse_error (state,
						  "WSDL:operation must include "
						  "a SOAP:operation element");

			state->binding_operation->xml_node = xml_ctxt->node;
			state->state = WSDL_BINDING;
		}

		break;

	case WSDL_BINDING_OPERATION_INPUT:
	case WSDL_BINDING_OPERATION_OUTPUT:
	case WSDL_BINDING_OPERATION_FAULT:
		state->binding_operation_iof->xml_node = xml_ctxt->node;
		state->state = WSDL_BINDING_OPERATION;
		break;

	case WSDL_SOAP_BODY:
		state->soap_body->xml_node = xml_ctxt->node;
		state->state = state->binding_operation_state;
		break;

	case WSDL_SOAP_HEADER:
		state->soap_header->xml_node = xml_ctxt->node;
		state->state = state->binding_operation_state;
		break;

	case WSDL_SOAP_FAULT:
		state->soap_fault->xml_node = xml_ctxt->node;
		state->state = WSDL_BINDING_OPERATION_FAULT;
		break;

	case WSDL_SOAP_OPERATION:
		state->soap_operation->xml_node = xml_ctxt->node;
		state->state = WSDL_BINDING_OPERATION;
		break;

	case WSDL_SOAP_BINDING:
		state->soap_binding->xml_node = xml_ctxt->node;
		state->state = WSDL_BINDING;
		break;

	case WSDL_SERVICE:
		state->service->xml_node = xml_ctxt->node;
		state->state = WSDL_DEFINITIONS;
		break;

	case WSDL_SERVICE_PORT:
		if (state->service_port->soap_address == NULL) {
			wsdl_parse_error (state,
					  "WSDL:port must include a "
					  "SOAP:address element");
		}

		state->service_port->xml_node = xml_ctxt->node;
		state->state = WSDL_SERVICE;
		break;

	case WSDL_SOAP_ADDRESS:
		state->soap_address->xml_node = xml_ctxt->node;
		state->state = WSDL_SERVICE_PORT;
		break;

	case WSDL_UNKNOWN:
		state->unknown_depth--;
		if (state->unknown_depth == 0) {
			state->state = state->last_known_state;
		}
		break;

	case WSDL_START:
		/* We shouldn't be ending any tags in this state */
	case WSDL_FINISH:
		/* We should not have any more tags in this state */
		g_assert_not_reached ();
		break;

	case WSDL_STATE_MAX:	/* only here for the strict compile checking */
#if 0
	default:
#endif
		g_error ("Funny state! (%d/%s)", 
			 state->state,
			 state->state < WSDL_STATE_MAX ? 
			         states[state->state] : 
			         "*Unknown*");
		break;
	}

	/* This needs to be at the end, so xml_ctxt points to the
	 * current node while we are recording xml_node pointers.
	 */
	xmlDefaultSAXHandler.endElement (xml_ctxt, name);
}

static void
wsdl_characters (void *user, const xmlChar * chars, signed int len)
{
	wsdl_state *state = (wsdl_state *) user;
	gchar buf[256];

	memset (buf, '\0', sizeof (buf));
	memcpy (buf, chars, (unsigned int) (len < 256 ? len : 255));

	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_INFO,
		    "Characters: [%s] (%d) (state %s)", 
		    buf, 
		    len,
		    states[state->state]);

	switch (state->state) {
	case WSDL_DOCUMENTATION:
		{
			gchar *str;

			g_assert (state->documentation != NULL &&
				  *state->documentation != NULL);

			str = g_strndup (chars, (unsigned int) len);
			g_string_append (*state->documentation, str);
			break;
		}

	default:
		/* Just ignore any junk data */
		break;
	}

	xmlDefaultSAXHandler.characters (xml_ctxt, chars, len);
}

static void
wsdl_whitespace (void *user G_GNUC_UNUSED, const xmlChar * chars,
		 signed int len)
{
	wsdl_debug (WSDL_LOG_DOMAIN_PARSER, 
		    G_LOG_LEVEL_DEBUG,
		    "Ignored %d whitespace chars", 
		    len);

	xmlDefaultSAXHandler.ignorableWhitespace (xml_ctxt, chars, len);
}


/* This is a combination of xmlSAXUserParseFile and xmlSAXParseFile,
 * needed so I can keep a handle on the xmlParserCtxtPtr xml_ctxt.
 */
static xmlDocPtr
wsdl_parse_xml (xmlSAXHandlerPtr sax, void *user_data, const char *file)
{
	xmlDocPtr ret;

	xml_ctxt = xmlCreateFileParserCtxt (file);

	if (xml_ctxt == NULL) {
		return (NULL);
	}

	if (xml_ctxt->sax != &xmlDefaultSAXHandler) {
		xmlFree (xml_ctxt->sax);
	}

	xml_ctxt->sax = sax;
	if (user_data != NULL) {
		xml_ctxt->userData = user_data;
	}

	xmlParseDocument (xml_ctxt);

	if (xml_ctxt->wellFormed) {
		ret = xml_ctxt->myDoc;
	} else {
		ret = NULL;
		xmlFreeDoc (xml_ctxt->myDoc);
		xml_ctxt->myDoc = NULL;
	}

	if (sax != NULL) {
		xml_ctxt->sax = NULL;
	}

	xmlFreeParserCtxt (xml_ctxt);

	return (ret);
}

/**
 * wsdl_parse:
 * @file: a string containing the full or relative path to a WSDL file
 *
 * Parses the file @file and constructs a set of WSDL elements.  The
 * #wsdl_definitions struct contains pointers to all WSDL elements
 * parsed from @file.  Any errors in the XML or the WSDL are logged by
 * calling g_log().  Serious errors (or warnings if --werror is
 * specified on the command line) cause this function to call exit(-1).
 *
 * Returns: a pointer to a #wsdl_definitions structure containing all
 * the information gleaned from the WSDL file, or NULL if parsing
 * failed.  The caller is expected to free the memory used by the
 * #wsdl_definitions structure, by calling wsdl_free_definitions().
 */
wsdl_definitions *
wsdl_parse (const char *file)
{
	xmlSAXHandler wsdlSAXParser;
	xmlDocPtr xml_doc;
	wsdl_state *state;
	wsdl_definitions *definitions = NULL;

	/* This palaver is so we can build an xmlDocPtr parse tree,
	 * while still having the SAX callbacks to build our own
	 * structures.
	 *
	 * (The tree is needed for XPath, validation, XInclude etc)
	 */

	memcpy (&wsdlSAXParser, &xmlDefaultSAXHandler, sizeof (xmlSAXHandler));

	/* These functions _must_ call the corresponding
	 * xmlDefaultSAXHandler functions themselves.
	 */
	wsdlSAXParser.startDocument = wsdl_start_document;
	wsdlSAXParser.endDocument = wsdl_end_document;
	wsdlSAXParser.startElement = wsdl_start_element;
	wsdlSAXParser.endElement = wsdl_end_element;
	wsdlSAXParser.characters = wsdl_characters;
	wsdlSAXParser.ignorableWhitespace = wsdl_whitespace;

	/* These don't have to */
	wsdlSAXParser.warning = wsdl_parse_warning;
	wsdlSAXParser.error = wsdl_parse_error;
	wsdlSAXParser.fatalError = wsdl_parse_fatal;

	warnings = errors = fatalities = 0;

	state = g_new0 (wsdl_state, 1);
	state->state = WSDL_START;

	if ((xml_doc = wsdl_parse_xml (&wsdlSAXParser, state, file)) == NULL) {
		g_warning ("Document not well formed!");
		errors++;
	} else {
		definitions = state->definitions;
		definitions->xml = xml_doc;
	}

	/* deliberately don't free the memory pointed to by state's
	 * members, they are pointing into definitions' children.
	 */
	g_free (state);

	if (warnings == 1) {
		g_warning ("There was 1 warning during parsing");
	} else if (warnings > 1) {
		g_warning ("There were %d warnings during parsing", warnings);
	}

	if (errors == 1) {
		g_warning ("There was 1 error during parsing");
	} else if (errors > 1) {
		g_warning ("There were %d errors during parsing", errors);
	}

	if (fatalities == 1) {
		g_warning ("There was 1 fatal error during parsing");
	} else if (fatalities > 1) {
		g_warning ("There were %d fatal errors during parsing",
			   fatalities);
	}

	if ((warnings && option_warnings_are_errors) || errors || fatalities) {
		exit (-1);
	}

	return (definitions);
}
