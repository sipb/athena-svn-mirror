/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-thread.c: Thread together all the pieces of the WSDL document
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "wsdl-thread.h"
#include "wsdl-parse.h"
#include "wsdl-trace.h"
#include "wsdl-locate.h"

static void
wsdl_thread_soap_parts (
		wsdl_binding_operation_inoutfault  *bind_iof,
		wsdl_porttype_operation_inoutfault *iof,
		int                                *errors G_GNUC_UNUSED)
{
	wsdl_soap_body *soap_body;
	wsdl_message *message;

	soap_body = bind_iof->soap_body;
	message = iof->thread_soap_message;

	if (soap_body->parts == NULL) {
		/* Thread all parts */
		iof->thread_soap_parts = g_slist_copy (message->parts);
	} else {
		/* Thread only those parts named */
		gchar **parts;
		wsdl_message_part *part;
		int i;

		parts = g_strsplit (soap_body->parts, " \t", 0);

		for (i = 0; parts[i] != NULL; i++) {
			part = wsdl_locate_message_part (parts[i], message);
			iof->thread_soap_parts =
				g_slist_append (iof->thread_soap_parts, part);
		}

		g_strfreev (parts);
	}
}

static void
wsdl_thread_soap_binding_operation (wsdl_binding_operation *op,
				    wsdl_porttype          *porttype,
				    wsdl_definitions       *definitions, 
				    int                    *errors)
{
	wsdl_binding_operation_inoutfault *input, *output;
	wsdl_porttype_operation_inoutfault *porttype_input, *porttype_output;
	wsdl_porttype_operation *porttype_op;
	wsdl_message *input_message, *output_message;

	g_assert (op->name != NULL);

	/* NB: This is optional for non-HTTP bindings! */
	g_assert (op->soap_operation != NULL);
	g_assert (op->soap_operation->soapAction != NULL);

	if (op->input == NULL) {
		wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
			    G_LOG_LEVEL_INFO,
			    "Notification operations not supported!");
		(*errors)++;
		return;
	}

	if (op->solicit == TRUE) {
		wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
			    G_LOG_LEVEL_INFO,
			    "Solicit-Response operations not supported!\n");
		(*errors)++;
		return;
	}

	wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
		    G_LOG_LEVEL_DEBUG,
		    "Binding Operation: [%s]", 
		    op->name);

	porttype_op = wsdl_locate_porttype_operation (op->name, porttype);
	if (porttype_op == NULL) {
		wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
			    G_LOG_LEVEL_INFO,
			    "Couldn't find operation %s in portType %s.\n",
			    op->name, 
			    porttype->name);
		(*errors)++;
		return;
	}

	op->thread_soap_porttype_operation = porttype_op;

	if (op->input != NULL) {
		input = op->input;
		g_assert (input->name != NULL);

		porttype_input = porttype_op->input;
		g_assert (porttype_input != NULL);

		if (strcmp (input->name, porttype_input->name) != 0) {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "Binding and portType input elements don't "
				    "match for operation %s (%s != %s).\n",
				    op->name, 
				    input->name,
				    porttype_input->name);
			(*errors)++;
			return;
		}

		input_message = wsdl_locate_message (porttype_input->message,
						     definitions);
		if (input_message == NULL) {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "Can't find message %s.\n",
				    porttype_input->message);
			(*errors)++;
			return;
		}
		porttype_input->thread_soap_message = input_message;

		if (input->soap_body != NULL) {
			wsdl_thread_soap_parts (input, porttype_input, errors);
		} else {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "No soap:body found for %s.", 
				    input->name);
			(*errors)++;
		}
	}

	if (op->output != NULL) {
		output = op->output;
		g_assert (output->name != NULL);

		porttype_output = porttype_op->output;
		g_assert (porttype_output != NULL);

		if (strcmp (output->name, porttype_output->name) != 0) {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "Binding and portType output elements "
				    "don't match for operation "
				    "%s (%s != %s).\n",
				    op->name, 
				    output->name,
				    porttype_output->name);
			(*errors)++;
			return;
		}

		output_message = wsdl_locate_message (porttype_output->message,
						      definitions);
		if (output_message == NULL) {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "Can't find message %s.\n",
				    porttype_output->message);
			(*errors)++;
			return;
		}
		porttype_output->thread_soap_message = output_message;

		if (output->soap_body != NULL) {
			wsdl_thread_soap_parts (output, 
						porttype_output,
						errors);
		} else {
			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_INFO,
				    "No soap:body found for %s.", 
				    output->name);
			(*errors)++;
		}
	}

	/* locate faults */
}

static void
wsdl_thread_soap_binding (wsdl_binding     *binding,
			  wsdl_definitions *definitions, 
			  int              *errors)
{
	GSList *iter;
	wsdl_porttype *porttype;

	g_assert (binding->name != NULL);
	g_assert (binding->type != NULL);
	g_assert (binding->operations != NULL);

	wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
		    G_LOG_LEVEL_DEBUG,
		    "Checking binding: [%s]", 
		    binding->name);

	porttype = wsdl_locate_porttype (binding->type, definitions);
	if (porttype == NULL) {
		wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
			    G_LOG_LEVEL_INFO,
			    "Couldn't find portType %s.\n", 
			    binding->type);
		(*errors)++;
	} else {
		binding->thread_soap_porttype = porttype;

		iter = binding->operations;
		while (iter != NULL) {
			wsdl_thread_soap_binding_operation (iter->data,
							    porttype,
							    definitions,
							    errors);

			iter = iter->next;
		}
	}
}

static void
wsdl_thread_soap_service (wsdl_service     *service,
			  wsdl_definitions *definitions, 
			  int              *errors)
{
	GSList *iter;
	gboolean threaded_service = FALSE;

	g_assert (service->name != NULL);

	wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
		    G_LOG_LEVEL_DEBUG,
		    "Checking service: [%s]", 
		    service->name);

	/* This is where we look for port definitions with soap addresses */
	iter = service->ports;
	while (iter != NULL) {
		wsdl_service_port *port = iter->data;
		wsdl_binding *binding;

		g_assert (port->name != NULL);
		g_assert (port->binding != NULL);

		wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
			    G_LOG_LEVEL_INFO,
			    "Scanning service port: [%s]", 
			    port->name);

		if (port->soap_address != NULL) {
			wsdl_soap_address *address = port->soap_address;

			wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
				    G_LOG_LEVEL_DEBUG,
				    "Found: [%s]", 
				    address->location);

			if (threaded_service == FALSE) {
				definitions->thread_soap_services =
					g_slist_append (definitions->
							thread_soap_services,
							service);
				threaded_service = TRUE;
			}

			service->thread_soap_ports =
				g_slist_append (service->thread_soap_ports,
						port);

			/* Now find the binding referenced by the port
			 * binding attribute
			 */
			binding = wsdl_locate_binding (port->binding,
						       definitions);
			if (binding == NULL) {
				wsdl_debug (WSDL_LOG_DOMAIN_THREAD,
					    G_LOG_LEVEL_INFO,
					    "Couldn't find binding: [%s]\n",
					    port->binding);
				(*errors)++;
			} else {
				port->thread_soap_binding = binding;
				wsdl_thread_soap_binding (binding, 
							  definitions,
							  errors);
			}
		}

		iter = iter->next;
	}
}

/**
 * wsdl_thread:
 * @definitions: a pointer to a wsdl_definitions structure, as
 * returned from wsdl_parse().
 *
 * Recurses through all the members of @definitions, threading
 * together all of the WSDL indirection.  The elements of @definitions
 * are updated in place.
 *
 * Returns: the number of inconsistencies (eg bindings that reference
 * a non-existing port binding, etc) .
 */
int
wsdl_thread (wsdl_definitions * definitions)
{
	GSList *iter;
	int errors = 0;

	wsdl_debug (WSDL_LOG_DOMAIN_THREAD, 
		    G_LOG_LEVEL_DEBUG,
		    "Threading together: [%s]", 
		    definitions->name);

	/*
	 * Scan services for port definitions with soap addresses.
	 *
	 * for each one found, find the binding referenced by
	 * the port binding attribute.
	 *
	 * For each binding operation, output a function
	 *
	 */
	iter = definitions->services;
	while (iter != NULL) {
		wsdl_thread_soap_service (iter->data, definitions, &errors);

		iter = iter->next;
	}

	/*
	 * Other (non-SOAP) bindings can be threaded here
	 */

	return (errors);
}
