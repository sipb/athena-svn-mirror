/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-skels.c: Emit skeleton code for servers
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

#include <libsoup/soup-uri.h>

#include "wsdl-parse.h"
#include "wsdl-soap-skels.h"
#include "wsdl-trace.h"
#include "wsdl-soap-emit.h"

static void
wsdl_emit_soap_skels_binding_operation (FILE                         * out, 
					const guchar                 * opns,
					const guchar                 * opnsuri,
					const wsdl_binding_operation * const op)
{
	gchar *prefix;
	wsdl_porttype_operation *porttype_op;

	g_assert (op->name != NULL);
	g_assert (op->soap_operation != NULL);

	W ("/* BEGIN Binding Operation %s */\n\n", op->name);

	porttype_op = op->thread_soap_porttype_operation;

	g_assert (porttype_op != NULL);
	g_assert (porttype_op->input != NULL);

	if (op->documentation)
		W ("/* %s */\n\n", op->documentation->str);

	if (porttype_op->documentation)
		W ("/* %s */\n\n", porttype_op->documentation->str);

	prefix = g_strconcat (opns, "_", op->name, NULL);

	/* Emit the soup server callback */
	W ("static void \n"
	   "%s_servhandler (SoupServerContext \t*_context,\n"
	   "%*s              SoupMessage \t*_msg,\n"
	   "%*s              gpointer \t _user_data)\n{\n",
	   prefix,
	   (int) strlen (prefix), "",
	   (int) strlen (prefix), "");

	W ("\t_%s_skels_data_t *_cb_data = _user_data;\n", opns);
	W ("\tSoupEnv *_env;\n");	

	W ("\n");
	if (porttype_op->input) {
		W ("\t/* more args here based on input parts */\n");

		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t%t _in_%p;\n");
	}

	W ("\n");
	if (porttype_op->output) {
		W ("\t/* more args here based on output parts */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t%t _out_%p;\n");
	}

	W ("\n");
	if (porttype_op->input) {
		W ("\twsdl_param _in_params[]={\n");
		W ("\t\t/* params here based on input parts */\n");

		wsdl_emit_part_list (
			out, 
			porttype_op->input->thread_soap_parts,
			"\t\t{\"%p\", &_in_%p, &WSDL_TC_%n_%N_struct},\n");

		W ("\t\t{NULL, NULL, NULL},\n");
		W ("\t};\n");
	}

	W ("\n");
	if (porttype_op->output) {
		W ("\twsdl_param _out_params[]={\n");
		W ("\t\t/* params here based on output parts */\n");

		wsdl_emit_part_list (
			out,
			porttype_op->output->thread_soap_parts,
			"\t\t{\"%p\", &_out_%p, &WSDL_TC_%n_%N_struct},\n");

		W ("\t\t{NULL, NULL, NULL},\n");
		W ("\t};\n");
	}

	W ("\n");
	W ("\t_env = soup_env_new_server (_msg, _context);\n");
	W ("\n");

	W ("\tif (_cb_data->handlers.%s) {\n", op->name);

	if (porttype_op->input) {
		W ("\t\t/* parse msg->request.body into callback params */\n");
		W ("\t\twsdl_soap_parse (_msg->request.body,\n"
		   "\t\t\t\t \"%s\",\n"
		   "\t\t\t\t _in_params,\n"
		   "\t\t\t\t _env,\n"
		   "\t\t\t\t WSDL_SOAP_FLAGS_REQUEST);\n\n",
		   op->name);
	}

	if (porttype_op->output) {
		W ("\t\t/* Zero the output params */\n");
		W ("\t\twsdl_soap_initialise (_out_params);\n");
	}

	W ("\n");
	W ("\t\t/* Issue server callback */\n");
	W ("\t\t_cb_data->handlers.%s (\n"
	   "\t\t\t_env\n", op->name);

	if (porttype_op->input) {
		W ("\t\t\t\t\t/* input args go here */\n");

		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t\t, _in_%p\n");
	}

	if (porttype_op->output) {
		W ("\t\t\t\t\t/* output args go here */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t\t, &_out_%p\n");
	}

	W ("\t\t\t, _cb_data->user_data);\n");

	W ("\t} else {\n");
	W ("\t\t/* Now marshal a \"Not implemented\" fault */\n");
	W ("\t}\n\n");

	W ("\t/* Marshal output args into a SOAP reply */\n");
	W ("\twsdl_soap_marshal (\"%s\",\n"
	   "\t\t\t   \"%s\",\n"
	   "\t\t\t   \"%s\",\n",
	   op->name, 
	   opns, 
	   opnsuri);

	if (porttype_op->output) 
		W ("\t\t\t   _out_params,\n");
	else 
		W ("\t\t\t   NULL,\n");

	W ("\t\t\t   &_msg->response,\n"
	   "\t\t\t   _env,\n"
	   "\t\t\t   WSDL_SOAP_FLAGS_RESPONSE);\n\n");

	if (porttype_op->output) {
		W ("\t/* Free the returned memory */\n");
		W ("\twsdl_soap_free (_out_params);\n");
	}

	W ("\n");
	W ("\tsoup_env_free (_env);\n");

	W ("}\n");
	W ("\n");

	W ("/* END Binding Operation %s */\n\n", op->name);
}

static void
wsdl_emit_soap_skels_binding (FILE                      *out, 
			      const guchar              *opns,
			      const guchar              *opnsuri,
			      const wsdl_binding * const binding)
{
	GSList *iter;
	wsdl_porttype *porttype;
	gint opcnt = 0;

	g_assert (binding->name != NULL);

	W ("/* BEGIN Binding %s */\n\n", binding->name);

	if (binding->documentation)
		W ("/* %s */\n\n", binding->documentation->str);

	porttype = binding->thread_soap_porttype;

	g_assert (porttype != NULL);

	/* 
	 * Declare structure for passing callback data 
	 */
	W ("typedef struct {\n");
	W ("\tgint refcnt;\n");
	W ("\t%s_handlers handlers;\n", opns);
	W ("\tgpointer user_data;\n");
	W ("} _%s_skels_data_t;\n\n", opns);

	/* For each binding operation, output a server handler */
	iter = binding->operations;
	while (iter) {
		wsdl_emit_soap_skels_binding_operation (out, 
							opns, 
							opnsuri,
							iter->data);

		iter = iter->next;
	}

	/* 
	 * Handler cleanup. Unref the cb_data for this service, and free if it
	 * reaches 0 (i.e. no more handlers for this service exist) 
	 */
	W ("static void\n"
	   "%s_servcleanup (SoupServer \t\t*server,\n"
	   "\t\t\tSoupServerHandler \t*handler,\n"
	   "\t\t\tgpointer \t\t user_data)\n"
	   "{\n",
	   opns);
	W ("\t_%s_skels_data_t *_cb_data = user_data;\n", opns);
	W ("\n");
	W ("\t--(_cb_data->refcnt);\n");
	W ("\n");
	W ("\tif (_cb_data->refcnt == 0)\n");
	W ("\t\tg_free (_cb_data);\n");
	W ("}\n\n");

	W ("void\n"
	   "%s_register (SoupServer \t\t*server,\n"
	   "\t\t     SoupServerAuthContext \t*auth_ctx,\n"
	   "\t\t     %s_handlers \t*handlers,\n"
	   "\t\t     gpointer \t\t\t user_data)\n"
	   "{\n",
	   opns,
	   opns);

	W ("\t_%s_skels_data_t *_cb_data;\n", opns);
	W ("\n");

	W ("\t_cb_data = g_new (_%s_skels_data_t, 1);\n", opns);
	W ("\t_cb_data->user_data = user_data;\n");
	W ("\tmemcpy (&_cb_data->handlers, handlers, sizeof (%s_handlers));\n",
	   opns);
	W ("\n");

	for (iter = binding->operations; iter; iter = iter->next) {
		wsdl_binding_operation *op;
		wsdl_porttype_operation *porttype_op;
		SoupUri *uri;

		op = iter->data;
		porttype_op = op->thread_soap_porttype_operation;

		uri = soup_uri_new (op->soap_operation->soapAction);

		W ("\tsoup_server_register (server,\n"
		   "\t\t\t      \"%s\",\n"
		   "\t\t\t      auth_ctx,\n"
		   "\t\t\t      %s_%s_servhandler,\n"
		   "\t\t\t      %s_servcleanup,\n"
		   "\t\t\t      _cb_data);\n\n", 
		   uri->path, 
		   opns, 
		   op->name, 
		   opns);

		soup_uri_free (uri);
		opcnt++;
	}

	W ("\t_cb_data->refcnt = %d;\n", opcnt);

	W ("}\n");
	W ("\n");

	W ("/* END Binding %s */\n\n", binding->name);
}

static void
wsdl_emit_soap_skels_service (FILE                      *out, 
			      const guchar              *opns,
			      const guchar              *opnsuri,
			      const wsdl_service * const service)
{
	GSList *iter;

	g_assert (service->name != NULL);

	W ("/* BEGIN Service %s */\n", service->name);

	if (service->documentation)
		W ("/* %s */\n\n", service->documentation->str);

	iter = service->thread_soap_ports;
	while (iter) {
		wsdl_service_port *port = iter->data;

		g_assert (port->thread_soap_binding != NULL);

		wsdl_emit_soap_skels_binding (out, 
					      opns, 
					      opnsuri,
					      port->thread_soap_binding);

		if (port->soap_address) {
			SoupUri *loc_uri;

			loc_uri = soup_uri_new (port->soap_address->location);
			if (loc_uri) {
				const char *proto;

				switch (loc_uri->protocol) {
				case SOUP_PROTOCOL_HTTP:
					proto = "SOUP_PROTOCOL_HTTP";
					break;
				case SOUP_PROTOCOL_HTTPS:
					proto = "SOUP_PROTOCOL_HTTPS";
					break;
				default:
					proto = "SOUP_PROTOCOL_HTTP";
				}

				W ("SoupServer *\n"
				   "%s_get_server (void)\n"
				   "{\n"
				   "\treturn soup_server_new (%s, %d);\n"
				   "}\n\n", 
				   opns,
				   proto,
				   loc_uri->port);
			}
		}

		iter = iter->next;
	}

	W ("/* END Service %s */\n\n", service->name);
}

/**
 * wsdl_emit_soap_skels:
 * @outdir: a string containing the path to a directory.  This
 * function expects the string to have a trailing '/'.
 * @fileroot: a string containing the root of a filename.  "-skels.c"
 * will be appended to this name.
 * @definitions: a pointer to a #wsdl_definitions structure,
 * containing a set of WSDL elements.
 *
 * Creates the file @outdir/@fileroot-skels.c, and writes C code
 * containing server skeletons.
 */
void
wsdl_emit_soap_skels (const guchar                  *outdir, 
		      const guchar                  *fileroot,
		      const wsdl_definitions * const definitions)
{
	GSList *iter;
	guchar *filename;
	const guchar *opns;

	filename = g_strconcat (outdir, fileroot, "-skels.c", NULL);

	wsdl_debug (WSDL_LOG_DOMAIN_SKELS, 
		    G_LOG_LEVEL_DEBUG, 
		    "file: [%s]",
		    filename);

	wsdl_set_output_file (fopen (filename, "w"));
	g_free (filename);

	if (wsdl_get_output_file () == NULL) {
		g_warning ("Couldn't open %s for writing: %s", 
			   filename,
			   g_strerror (errno));
		return;
	}

	W ("/*\n");
	if (definitions->name) {
		W (" * %s\n", definitions->name);
		opns = definitions->name;
	} else
		opns = "m";

	W (" *\n");
	W (" * Automatically generated by soup-wsdl.\n");
	W (" */\n");
	W ("\n");
	W ("#include <glib.h>\n");
	W ("#include <string.h>\n");
	W ("#include <libsoup/soup.h>\n");
	W ("#include <libwsdl/wsdl.h>\n");
	W ("#include \"%s.h\"\n\n", fileroot);

	if (definitions->documentation)
		W ("/* %s */\n\n", definitions->documentation->str);

	iter = definitions->thread_soap_services;
	while (iter) {
		wsdl_emit_soap_skels_service (wsdl_get_output_file (), 
					      opns,
					      definitions->targetNamespace,
					      iter->data);

		iter = iter->next;
	}

	fclose (wsdl_get_output_file ());
	wsdl_set_output_file (NULL);
}
