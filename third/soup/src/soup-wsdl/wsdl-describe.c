/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-describe.c: Print text description of WSDL tree
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>

#include "wsdl-parse.h"
#include "wsdl-describe.h"

#define OFFSET 4

static void
indent (guint ind)
{
	unsigned int i;

	for (i = 0; i < ind; i++) {
		g_print (" ");
	}
}

static void
wsdl_describe_typecode (const wsdl_typecode * const tc, gpointer user_data)
{
	guint ind = GPOINTER_TO_INT (user_data);

	indent (ind);
	g_print ("Typecode\n");
	indent (ind);
	g_print ("--------\n");

	wsdl_typecode_print (tc, ind);
}

/**
 * wsdl_describe_types:
 * @ind: an integer specifying the indent level to use
 * @types: a pointer to a #wsdl_types structure
 *
 * Produces a printable representation of @types on standard output.
 */
void
wsdl_describe_types (guint ind, wsdl_types * types)
{
	indent (ind);
	g_print ("Types\n");
	indent (ind);
	g_print ("-----\n");

	if (types->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", types->documentation->str);
	}

	/* call wsdl_typecode_print for all non-simple typecodes known */
	wsdl_typecode_foreach (FALSE, 
			       wsdl_describe_typecode,
			       GINT_TO_POINTER (ind));
}

/**
 * wsdl_describe_message_part:
 * @ind: an integer specifying the indent level to use
 * @part: a pointer to a #wsdl_message_part structure
 *
 * Produces a printable representation of @part on standard output.
 */
void
wsdl_describe_message_part (guint ind, wsdl_message_part * part)
{
	indent (ind);
	g_print ("Part\n");
	indent (ind);
	g_print ("----\n");

	if (part->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", part->name);
	}
	if (part->typecode != NULL) {
		wsdl_typecode_print (part->typecode, ind);
	}
}

/**
 * wsdl_describe_message:
 * @ind: an integer specifying the indent level to use
 * @message: a pointer to a #wsdl_types structure
 *
 * Produces a printable representation of @message on standard output.
 */
void
wsdl_describe_message (guint ind, wsdl_message * message)
{
	indent (ind);
	g_print ("Message\n");
	indent (ind);
	g_print ("-------\n");

	if (message->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", message->name);
	}

	if (message->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", message->documentation->str);
	}

	if (message->parts != NULL) {
		GSList *iter = message->parts;

		while (iter != NULL) {
			wsdl_describe_message_part (ind + OFFSET, iter->data);

			iter = iter->next;
		}
	}
}

/**
 * wsdl_describe_porttype_operation_iof:
 * @ind: an integer specifying the indent level to use
 * @iof: a pointer to a #wsdl_porttype_operation_inoutfault structure
 * @type: a #wsdl_describe_iof_t enum, containing either
 * #WSDL_DESCRIBE_INPUT, #WSDL_DESCRIBE_OUTPUT or
 * #WSDL_DESCRIBE_FAULT.
 *
 * Produces a printable representation of @iof on standard output.
 * @type controls whether @iof is listed as either "Input", "Output"
 * or "Fault".
 */
void
wsdl_describe_porttype_operation_iof (guint                               ind,
				      wsdl_porttype_operation_inoutfault *iof,
				      wsdl_describe_iof_t                 type)
{
	if (type == WSDL_DESCRIBE_INPUT) {
		indent (ind);
		g_print ("Input\n");
		indent (ind);
		g_print ("-----\n");
	} else if (type == WSDL_DESCRIBE_OUTPUT) {
		indent (ind);
		g_print ("Output\n");
		indent (ind);
		g_print ("------\n");
	} else if (type == WSDL_DESCRIBE_FAULT) {
		indent (ind);
		g_print ("Fault\n");
		indent (ind);
		g_print ("-----\n");
	} else {
		g_assert_not_reached ();
	}

	if (iof->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", iof->name);
	}

	if (iof->message != NULL) {
		indent (ind);
		g_print ("Message: [%s]\n", iof->message);
	}

	if (iof->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", iof->documentation->str);
	}
}

/**
 * wsdl_describe_porttype_operation:
 * @ind: an integer specifying the indent level to use
 * @op: a pointer to a #wsdl_porttype_operation structure
 *
 * Produces a printable representation of @op on standard output.
 */
void
wsdl_describe_porttype_operation (guint ind, wsdl_porttype_operation * op)
{
	indent (ind);
	g_print ("Operations\n");
	indent (ind);
	g_print ("----------\n");

	if (op->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", op->name);
	}

	if (op->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", op->documentation->str);
	}

	if (op->solicit == TRUE) {
		indent (ind);
		g_print ("(Solicit-response operation)\n");
	}

	if (op->input != NULL) {
		wsdl_describe_porttype_operation_iof (ind + OFFSET, 
						      op->input,
						      WSDL_DESCRIBE_INPUT);
	}

	if (op->output != NULL) {
		wsdl_describe_porttype_operation_iof (ind + OFFSET, 
						      op->output,
						      WSDL_DESCRIBE_OUTPUT);
	}

	if (op->faults != NULL) {
		GSList *faults = op->faults;

		while (faults != NULL) {
			wsdl_describe_porttype_operation_iof (
				ind + OFFSET,
				faults->data,
				WSDL_DESCRIBE_FAULT);
			faults = faults->next;
		}
	}
}

/**
 * wsdl_describe_porttype:
 * @ind: an integer specifying the indent level to use
 * @porttype: a pointer to a #wsdl_porttype structure
 *
 * Produces a printable representation of @porttype on standard
 * output.
 */
void
wsdl_describe_porttype (guint ind, wsdl_porttype * porttype)
{
	indent (ind);
	g_print ("PortType\n");
	indent (ind);
	g_print ("--------\n");

	if (porttype->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", porttype->name);
	}

	if (porttype->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", porttype->documentation->str);
	}

	if (porttype->operations != NULL) {
		GSList *opiter = porttype->operations;


		while (opiter != NULL) {
			wsdl_describe_porttype_operation (ind + OFFSET,
							  opiter->data);

			opiter = opiter->next;
		}
	}
}

/**
 * wsdl_describe_soap_operation:
 * @ind: an integer specifying the indent level to use
 * @soap_operation: a pointer to a #wsdl_soap_operation structure
 *
 * Produces a printable representation of @soap_operation on standard
 * output.
 */
void
wsdl_describe_soap_operation (guint ind, wsdl_soap_operation * soap_operation)
{
	indent (ind);
	g_print ("SOAP Operation\n");
	indent (ind);
	g_print ("--------------\n");

	if (soap_operation->soapAction != NULL) {
		indent (ind);
		g_print ("soapAction: [%s]\n", soap_operation->soapAction);
	}

	if (soap_operation->style != NULL) {
		indent (ind);
		g_print ("Style: [%s]\n", soap_operation->style);
	}
}

/**
 * wsdl_describe_soap_body:
 * @ind: an integer specifying the indent level to use
 * @soap_body: a pointer to a #wsdl_types structure
 *
 * Produces a printable representation of @soap_body on standard
 * output.
 */
void
wsdl_describe_soap_body (guint ind, wsdl_soap_body * soap_body)
{
	indent (ind);
	g_print ("SOAP Body\n");
	indent (ind);
	g_print ("---------\n");

	if (soap_body->parts != NULL) {
		indent (ind);
		g_print ("Parts: [%s]\n", soap_body->parts);
	}

	if (soap_body->use != NULL) {
		indent (ind);
		g_print ("Use: [%s]\n", soap_body->use);
	}

	if (soap_body->encodingStyle != NULL) {
		indent (ind);
		g_print ("encodingStyle: [%s]\n", soap_body->encodingStyle);
	}

	if (soap_body->namespace != NULL) {
		indent (ind);
		g_print ("Namespace: [%s]\n", soap_body->namespace);
	}
}

/**
 * wsdl_describe_soap_header:
 * @ind: an integer specifying the indent level to use
 * @soap_header: a pointer to a #wsdl_soap_header structure
 *
 * Produces a printable representation of @soap_header on standard
 * output.
 */
void
wsdl_describe_soap_header (guint ind, wsdl_soap_header * soap_header)
{
	indent (ind);
	g_print ("SOAP Header\n");
	indent (ind);
	g_print ("-----------\n");

	if (soap_header->element != NULL) {
		indent (ind);
		g_print ("Element: [%s]\n", soap_header->element);
	}

	if (soap_header->fault != NULL) {
		indent (ind);
		g_print ("Fault: [%s]\n", soap_header->fault);
	}
}

/**
 * wsdl_describe_soap_fault:
 * @ind: an integer specifying the indent level to use
 * @soap_fault: a pointer to a #wsdl_soap_fault structure
 *
 * Produces a printable representation of @soap_fault on standard
 * output.
 */
void
wsdl_describe_soap_fault (guint ind, wsdl_soap_fault * soap_fault)
{
	indent (ind);
	g_print ("SOAP Fault\n");
	indent (ind);
	g_print ("----------\n");

	if (soap_fault->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", soap_fault->name);
	}

	if (soap_fault->use != NULL) {
		indent (ind);
		g_print ("Use: [%s]\n", soap_fault->use);
	}

	if (soap_fault->encodingStyle != NULL) {
		indent (ind);
		g_print ("encodingStyle: [%s]\n", soap_fault->encodingStyle);
	}

	if (soap_fault->namespace != NULL) {
		indent (ind);
		g_print ("Namespace: [%s]\n", soap_fault->namespace);
	}
}

/**
 * wsdl_describe_binding_operation_iof:
 * @ind: an integer specifying the indent level to use
 * @iof: a pointer to a #wsdl_binding_operation_inoutfault structure
 * @type: a #wsdl_describe_iof_t enum, containing either
 * #WSDL_DESCRIBE_INPUT, #WSDL_DESCRIBE_OUTPUT or
 * #WSDL_DESCRIBE_FAULT.
 *
 *  Produces a printable representation of @iof on standard output.
 *  @type controls whether @iof is listed as either "Input", "Output"
 *  or "Fault".
 */
void
wsdl_describe_binding_operation_iof (guint                              ind,
				     wsdl_binding_operation_inoutfault *iof,
				     wsdl_describe_iof_t                type)
{
	if (type == WSDL_DESCRIBE_INPUT) {
		indent (ind);
		g_print ("Input\n");
		indent (ind);
		g_print ("-----\n");
	} else if (type == WSDL_DESCRIBE_OUTPUT) {
		indent (ind);
		g_print ("Output\n");
		indent (ind);
		g_print ("------\n");
	} else if (type == WSDL_DESCRIBE_FAULT) {
		indent (ind);
		g_print ("Fault\n");
		indent (ind);
		g_print ("-----\n");
	} else {
		g_assert_not_reached ();
	}

	if (iof->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", iof->name);
	}

	if (iof->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", iof->documentation->str);
	}

	if (iof->soap_body != NULL) {
		g_assert (type == WSDL_DESCRIBE_INPUT ||
			  type == WSDL_DESCRIBE_OUTPUT);

		wsdl_describe_soap_body (ind + OFFSET, iof->soap_body);
	}

	if (iof->soap_headers != NULL) {
		GSList *header = iof->soap_headers;

		g_assert (type == WSDL_DESCRIBE_INPUT ||
			  type == WSDL_DESCRIBE_OUTPUT);

		while (header != NULL) {
			wsdl_describe_soap_header (ind + OFFSET, header->data);
			header = header->next;
		}
	}

	if (iof->soap_fault != NULL) {
		g_assert (type == WSDL_DESCRIBE_FAULT);

		wsdl_describe_soap_fault (ind + OFFSET, iof->soap_fault);
	}
}

/**
 * wsdl_describe_binding_operation:
 * @ind: an integer specifying the indent level to use
 * @op: a pointer to a #wsdl_types structure
 *
 * Produces a printable representation of @op on standard output.
 */
void
wsdl_describe_binding_operation (guint ind, wsdl_binding_operation * op)
{
	indent (ind);
	g_print ("Operation\n");
	indent (ind);
	g_print ("---------\n");

	if (op->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", op->name);
	}

	if (op->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", op->documentation->str);
	}

	if (op->solicit == TRUE) {
		indent (ind);
		g_print ("(Solicit-response operation)\n");
	}

	if (op->soap_operation != NULL) {
		wsdl_describe_soap_operation (ind + OFFSET, op->soap_operation);
	}

	if (op->input != NULL) {
		wsdl_describe_binding_operation_iof (ind + OFFSET, 
						     op->input,
						     WSDL_DESCRIBE_INPUT);
	}

	if (op->output != NULL) {
		wsdl_describe_binding_operation_iof (ind + OFFSET, 
						     op->output,
						     WSDL_DESCRIBE_OUTPUT);
	}

	if (op->faults != NULL) {
		GSList *faults = op->faults;

		while (faults != NULL) {
			wsdl_describe_binding_operation_iof (
				ind + OFFSET,
				faults->data,
				WSDL_DESCRIBE_FAULT);
			faults = faults->next;
		}
	}
}

/**
 * wsdl_describe_soap_binding:
 * @ind: an integer specifying the indent level to use
 * @binding: a pointer to a #wsdl_types structure
 *
 * Produces a printable representation of @binding on standard output.
 */
void
wsdl_describe_soap_binding (guint ind, wsdl_soap_binding * binding)
{
	indent (ind);
	g_print ("SOAP Binding\n");
	indent (ind);
	g_print ("------------\n");

	if (binding->style != NULL) {
		indent (ind);
		g_print ("Style: [%s]\n", binding->style);
	}

	if (binding->transport != NULL) {
		indent (ind);
		g_print ("Transport: [%s]\n", binding->transport);
	}
}

/**
 * wsdl_describe_binding:
 * @ind: an integer specifying the indent level to use
 * @binding: a pointer to a #wsdl_binding structure
 *
 * Produces a printable representation of @binding on standard output.
 */
void
wsdl_describe_binding (guint ind, wsdl_binding * binding)
{
	indent (ind);
	g_print ("Binding\n");
	indent (ind);
	g_print ("-------\n");

	if (binding->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", binding->name);
	}
	if (binding->type != NULL) {
		indent (ind);
		g_print ("Type: [%s]\n", binding->type);
	}

	if (binding->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", binding->documentation->str);
	}

	if (binding->soap_binding != NULL) {
		wsdl_describe_soap_binding (ind + OFFSET,
					    binding->soap_binding);
	}

	if (binding->operations != NULL) {
		GSList *opiter = binding->operations;


		while (opiter != NULL) {
			wsdl_describe_binding_operation (ind + OFFSET,
							 opiter->data);

			opiter = opiter->next;
		}
	}
}

/**
 * wsdl_describe_soap_address:
 * @ind: an integer specifying the indent level to use
 * @address: a pointer to a #wsdl_soap_address structure
 *
 * Produces a printable representation of @address on standard output.
 */
void
wsdl_describe_soap_address (guint ind, wsdl_soap_address * address)
{
	indent (ind);
	g_print ("SOAP:Address\n");
	indent (ind);
	g_print ("------------\n");

	if (address->location != NULL) {
		indent (ind);
		g_print ("Location: [%s]\n", address->location);
	}
}

/**
 * wsdl_describe_service_port:
 * @ind: an integer specifying the indent level to use
 * @port: a pointer to a #wsdl_service_port structure
 *
 * Produces a printable representation of @port on standard output.
 */
void
wsdl_describe_service_port (guint ind, wsdl_service_port * port)
{
	indent (ind);
	g_print ("Port\n");
	indent (ind);
	g_print ("----\n");

	if (port->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", port->name);
	}
	if (port->binding != NULL) {
		indent (ind);
		g_print ("Binding: [%s]\n", port->binding);
	}

	if (port->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", port->documentation->str);
	}

	if (port->soap_address != NULL) {
		wsdl_describe_soap_address (ind + OFFSET, port->soap_address);
	}
}

/**
 * wsdl_describe_service:
 * @ind: an integer specifying the indent level to use
 * @service: a pointer to a #wsdl_service structure
 *
 * Produces a printable representation of @service on standard output.
 */
void
wsdl_describe_service (guint ind, wsdl_service * service)
{
	indent (ind);
	g_print ("Service\n");
	indent (ind);
	g_print ("-------\n");

	if (service->name != NULL) {
		indent (ind);
		g_print ("Name: [%s]\n", service->name);
	}

	if (service->documentation != NULL) {
		indent (ind);
		g_print ("Documentation: [%s]\n", service->documentation->str);
	}

	if (service->ports != NULL) {
		GSList *iter = service->ports;

		while (iter != NULL) {
			wsdl_describe_service_port (ind + OFFSET, iter->data);

			iter = iter->next;
		}
	}
}

/**
 * wsdl_describe_definitions:
 * @ind: an integer specifying the indent level to use
 * @definitions: a pointer to a #wsdl_definitions structure
 *
 * Produces a printable representation of @definitions on standard output,
 * recursing through all sub-types.
 */
void
wsdl_describe_definitions (guint ind, wsdl_definitions * definitions)
{
	GSList *iter;

	indent (ind);
	g_print ("Definitions\n");
	indent (ind);
	g_print ("-----------\n");

	if (definitions->name != NULL) {
		indent (ind);
		g_print ("Name [%s]\n", definitions->name);
	}
	if (definitions->targetNamespace != NULL) {
		indent (ind);
		g_print ("targetNamespace [%s]\n",
			 definitions->targetNamespace);
	}

	if (definitions->documentation != NULL) {
		indent (ind);
		g_print ("Definitions documentation: [%s]\n",
			 definitions->documentation->str);
	}

	if (definitions->types != NULL) {
		wsdl_describe_types (ind + OFFSET, definitions->types);
	}
	if (definitions->messages != NULL) {
		iter = definitions->messages;
		while (iter != NULL) {
			wsdl_describe_message (ind + OFFSET, iter->data);
			iter = iter->next;
		}
	}
	if (definitions->porttypes != NULL) {
		iter = definitions->porttypes;
		while (iter != NULL) {
			wsdl_describe_porttype (ind + OFFSET, iter->data);
			iter = iter->next;
		}
	}
	if (definitions->bindings != NULL) {
		iter = definitions->bindings;
		while (iter != NULL) {
			wsdl_describe_binding (ind + OFFSET, iter->data);
			iter = iter->next;
		}
	}
	if (definitions->services != NULL) {
		iter = definitions->services;
		while (iter != NULL) {
			wsdl_describe_service (ind + OFFSET, iter->data);
			iter = iter->next;
		}
	}
}
