/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-memory.c: Free memory allocated to WSDL structs
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>

#include <libwsdl/wsdl-typecodes.h>

#include "wsdl-memory.h"

/**
 * wsdl_free_types:
 * @types: a pointer to a #wsdl_types structure
 *
 * Frees the memory pointed to by @types, and any memory used by
 * members of the #wsdl_types structure.  Then calls
 * wsdl_typecode_free_all().
 */
void
wsdl_free_types (wsdl_types * types)
{
	g_assert (types != NULL);

	if (types->documentation != NULL) {
		g_string_free (types->documentation, TRUE);
	}

	wsdl_typecode_free_all ();

	g_free (types);
}


/**
 * wsdl_free_message_part:
 * @part: a pointer to a #wsdl_message_part structure
 *
 * Frees the memory pointed to by @part, and any memory used by
 * members of the #wsdl_message_part structure.
 */
void
wsdl_free_message_part (wsdl_message_part * part)
{
	g_assert (part != NULL);

	if (part->name != NULL) {
		g_free (part->name);
	}

	g_free (part);
}


/**
 * wsdl_free_message:
 * @message: a pointer to a #wsdl_message structure
 *
 * Frees the memory pointed to by @message, and any memory used by
 * members of the #wsdl_message structure.
 */
void
wsdl_free_message (wsdl_message * message)
{
	GSList *iter;

	g_assert (message != NULL);

	if (message->name != NULL) {
		g_free (message->name);
	}
	if (message->documentation != NULL) {
		g_string_free (message->documentation, TRUE);
	}

	if (message->parts != NULL) {
		iter = message->parts;
		while (iter != NULL) {
			wsdl_free_message_part (iter->data);
			iter = iter->next;
		}
		g_slist_free (message->parts);
	}

	g_free (message);
}


/**
 * wsdl_free_porttype_operation_iof:
 * @iof: a pointer to a #wsdl_porttype_operation_inoutfault structure
 *
 * Frees the memory pointed to by @iof, and any memory used by members
 * of the #wsdl_porttype_operation_inoutfault structure.
 */
void
wsdl_free_porttype_operation_iof (wsdl_porttype_operation_inoutfault * iof)
{
	g_assert (iof != NULL);

	if (iof->name != NULL) {
		g_free (iof->name);
	}
	if (iof->message != NULL) {
		g_free (iof->message);
	}
	if (iof->documentation != NULL) {
		g_string_free (iof->documentation, TRUE);
	}
	g_slist_free (iof->thread_soap_parts);

	g_free (iof);
}


/**
 * wsdl_free_porttype_operation:
 * @operation: a pointer to a #wsdl_porttype_operation structure
 *
 * Frees the memory pointed to by @operation, and any memory used by
 * members of the #wsdl_porttype_operation structure.
 */
void
wsdl_free_porttype_operation (wsdl_porttype_operation * operation)
{
	GSList *iter;

	g_assert (operation != NULL);

	if (operation->name != NULL) {
		g_free (operation->name);
	}
	if (operation->documentation != NULL) {
		g_string_free (operation->documentation, TRUE);
	}
	if (operation->input != NULL) {
		wsdl_free_porttype_operation_iof (operation->input);
	}
	if (operation->output != NULL) {
		wsdl_free_porttype_operation_iof (operation->output);
	}
	if (operation->faults != NULL) {
		iter = operation->faults;
		while (iter != NULL) {
			wsdl_free_porttype_operation_iof (iter->data);
			iter = iter->next;
		}
		g_slist_free (operation->faults);
	}

	g_free (operation);
}


/**
 * wsdl_free_porttype:
 * @porttype: a pointer to a #wsdl_porttype structure
 *
 * Frees the memory pointed to by @porttype, and any memory used by
 * members of the #wsdl_porttype structure.
 */
void
wsdl_free_porttype (wsdl_porttype * porttype)
{
	GSList *iter;

	g_assert (porttype != NULL);

	if (porttype->name != NULL) {
		g_free (porttype->name);
	}
	if (porttype->documentation != NULL) {
		g_string_free (porttype->documentation, TRUE);
	}

	if (porttype->operations != NULL) {
		iter = porttype->operations;
		while (iter != NULL) {
			wsdl_free_porttype_operation (iter->data);
			iter = iter->next;
		}
		g_slist_free (porttype->operations);
	}

	g_free (porttype);
}


/**
 * wsdl_free_soap_binding:
 * @binding: a pointer to a #wsdl_soap_binding structure
 *
 * Frees the memory pointed to by @binding, and any memory used by
 * members of the #wsdl_soap_binding structure.
 */
void
wsdl_free_soap_binding (wsdl_soap_binding * binding)
{
	g_assert (binding != NULL);

	if (binding->style != NULL) {
		g_free (binding->style);
	}
	if (binding->transport != NULL) {
		g_free (binding->transport);
	}

	g_free (binding);
}


/**
 * wsdl_free_soap_operation:
 * @operation: a pointer to a #wsdl_soap_operation structure
 *
 * Frees the memory pointed to by @operation, and any memory used by
 * members of the #wsdl_soap_operation structure.
 */
void
wsdl_free_soap_operation (wsdl_soap_operation * operation)
{
	g_assert (operation != NULL);

	if (operation->soapAction != NULL) {
		g_free (operation->soapAction);
	}
	if (operation->style != NULL) {
		g_free (operation->style);
	}

	g_free (operation);
}


/**
 * wsdl_free_soap_body:
 * @body: a pointer to a #wsdl_soap_body structure
 *
 * Frees the memory pointed to by @body, and any memory used by
 * members of the #wsdl_soap_body structure.
 */
void
wsdl_free_soap_body (wsdl_soap_body * body)
{
	g_assert (body != NULL);

	if (body->parts != NULL) {
		g_free (body->parts);
	}
	if (body->use != NULL) {
		g_free (body->use);
	}
	if (body->encodingStyle != NULL) {
		g_free (body->encodingStyle);
	}
	if (body->namespace != NULL) {
		g_free (body->namespace);
	}

	g_free (body);
}


/**
 * wsdl_free_soap_header:
 * @header: a pointer to a #wsdl_soap_header structure
 *
 * Frees the memory pointed to by @header, and any memory used by
 * members of the #wsdl_soap_header structure.
 */
void
wsdl_free_soap_header (wsdl_soap_header * header)
{
	g_assert (header != NULL);

	if (header->element != NULL) {
		g_free (header->element);
	}
	if (header->fault != NULL) {
		g_free (header->fault);
	}

	g_free (header);
}


/**
 * wsdl_free_soap_fault:
 * @fault: a pointer to a #wsdl_soap_fault structure
 *
 * Frees the memory pointed to by @fault, and any memory used by
 * members of the #wsdl_soap_fault structure.
 */
void
wsdl_free_soap_fault (wsdl_soap_fault * fault)
{
	g_assert (fault != NULL);

	if (fault->name != NULL) {
		g_free (fault->name);
	}
	if (fault->use != NULL) {
		g_free (fault->use);
	}
	if (fault->encodingStyle != NULL) {
		g_free (fault->encodingStyle);
	}
	if (fault->namespace != NULL) {
		g_free (fault->namespace);
	}

	g_free (fault);
}


/**
 * wsdl_free_binding_operation_iof:
 * @iof: a pointer to a #wsdl_binding_operation_inoutfault structure
 *
 * Frees the memory pointed to by @iof, and any memory used by members
 * of the #wsdl_binding_operation_inoutfault structure.
 */
void
wsdl_free_binding_operation_iof (wsdl_binding_operation_inoutfault * iof)
{
	GSList *iter;

	g_assert (iof != NULL);

	if (iof->name != NULL) {
		g_free (iof->name);
	}
	if (iof->documentation != NULL) {
		g_string_free (iof->documentation, TRUE);
	}
	if (iof->soap_body) {
		wsdl_free_soap_body (iof->soap_body);
	}
	if (iof->soap_headers) {
		iter = iof->soap_headers;
		while (iter != NULL) {
			wsdl_free_soap_header (iter->data);
			iter = iter->next;
		}
		g_slist_free (iof->soap_headers);
	}
	if (iof->soap_fault) {
		wsdl_free_soap_fault (iof->soap_fault);
	}

	g_free (iof);
}


/**
 * wsdl_free_binding_operation:
 * @operation: a pointer to a #wsdl_binding_operation structure
 *
 * Frees the memory pointed to by @operation, and any memory used by
 * members of the #wsdl_binding_operation structure.
 */
void
wsdl_free_binding_operation (wsdl_binding_operation * operation)
{
	GSList *iter;

	g_assert (operation != NULL);

	if (operation->name != NULL) {
		g_free (operation->name);
	}
	if (operation->documentation != NULL) {
		g_string_free (operation->documentation, TRUE);
	}
	if (operation->soap_operation != NULL) {
		wsdl_free_soap_operation (operation->soap_operation);
	}
	if (operation->input != NULL) {
		wsdl_free_binding_operation_iof (operation->input);
	}
	if (operation->output != NULL) {
		wsdl_free_binding_operation_iof (operation->output);
	}
	if (operation->faults != NULL) {
		iter = operation->faults;
		while (iter != NULL) {
			wsdl_free_binding_operation_iof (iter->data);
			iter = iter->next;
		}
		g_slist_free (operation->faults);
	}

	g_free (operation);
}


/**
 * wsdl_free_binding:
 * @binding: a pointer to a #wsdl_binding structure
 *
 * Frees the memory pointed to by @binding, and any memory used by
 * members of the #wsdl_binding structure.
 */
void
wsdl_free_binding (wsdl_binding * binding)
{
	GSList *iter;

	g_assert (binding != NULL);

	if (binding->name != NULL) {
		g_free (binding->name);
	}
	if (binding->type != NULL) {
		g_free (binding->type);
	}
	if (binding->documentation != NULL) {
		g_string_free (binding->documentation, TRUE);
	}
	if (binding->soap_binding != NULL) {
		wsdl_free_soap_binding (binding->soap_binding);
	}
	if (binding->operations != NULL) {
		iter = binding->operations;
		while (iter != NULL) {
			wsdl_free_binding_operation (iter->data);
			iter = iter->next;
		}
		g_slist_free (binding->operations);
	}

	g_free (binding);
}


/**
 * wsdl_free_soap_address:
 * @address: a pointer to a #wsdl_soap_address structure
 *
 * Frees the memory pointed to by @address, and any memory used by
 * members of the #wsdl_soap_address structure.
 */
void
wsdl_free_soap_address (wsdl_soap_address * address)
{
	g_assert (address != NULL);

	if (address->location != NULL) {
		g_free (address->location);
	}

	g_free (address);
}


/**
 * wsdl_free_service_port:
 * @port: a pointer to a #wsdl_service_port structure
 *
 * Frees the memory pointed to by @port, and any memory used by
 * members of the #wsdl_service_port structure.
 */
void
wsdl_free_service_port (wsdl_service_port * port)
{
	g_assert (port != NULL);

	if (port->name != NULL) {
		g_free (port->name);
	}
	if (port->binding != NULL) {
		g_free (port->binding);
	}
	if (port->documentation != NULL) {
		g_string_free (port->documentation, TRUE);
	}
	if (port->soap_address != NULL) {
		wsdl_free_soap_address (port->soap_address);
	}

	g_free (port);
}


/**
 * wsdl_free_service:
 * @service: a pointer to a #wsdl_service structure
 *
 * Frees the memory pointed to by @service, and any memory used by
 * members of the #wsdl_service structure.
 */
void
wsdl_free_service (wsdl_service * service)
{
	GSList *iter;

	g_assert (service != NULL);

	if (service->name != NULL) {
		g_free (service->name);
	}
	if (service->documentation != NULL) {
		g_string_free (service->documentation, TRUE);
	}
	if (service->ports != NULL) {
		iter = service->ports;
		while (iter != NULL) {
			wsdl_free_service_port (iter->data);
			iter = iter->next;
		}
		g_slist_free (service->ports);
	}
	g_slist_free (service->thread_soap_ports);

	g_free (service);
}


/**
 * wsdl_free_definitions:
 * @definitions: a pointer to a #wsdl_definitions structure
 *
 * Frees the memory pointed to by @definitions, and any memory used by
 * members of the #wsdl_definitions structure.
 */
void
wsdl_free_definitions (wsdl_definitions * definitions)
{
	GSList *iter;

	g_assert (definitions != NULL);

	if (definitions->name != NULL) {
		g_free (definitions->name);
	}
	if (definitions->targetNamespace != NULL) {
		g_free (definitions->targetNamespace);
	}
	if (definitions->documentation != NULL) {
		g_string_free (definitions->documentation, TRUE);
	}

	if (definitions->types != NULL) {
		wsdl_free_types (definitions->types);
	}

	if (definitions->messages != NULL) {
		iter = definitions->messages;
		while (iter != NULL) {
			wsdl_free_message (iter->data);
			iter = iter->next;
		}
		g_slist_free (definitions->messages);
	}
	if (definitions->porttypes != NULL) {
		iter = definitions->porttypes;
		while (iter != NULL) {
			wsdl_free_porttype (iter->data);
			iter = iter->next;
		}
		g_slist_free (definitions->porttypes);
	}
	if (definitions->bindings != NULL) {
		iter = definitions->bindings;
		while (iter != NULL) {
			wsdl_free_binding (iter->data);
			iter = iter->next;
		}
		g_slist_free (definitions->bindings);
	}
	if (definitions->services != NULL) {
		iter = definitions->services;
		while (iter != NULL) {
			wsdl_free_service (iter->data);
			iter = iter->next;
		}
		g_slist_free (definitions->services);
	}
	g_slist_free (definitions->thread_soap_services);

	if (definitions->xml != NULL) {
		xmlFreeDoc (definitions->xml);
	}

	g_free (definitions);
}
