/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-stubs.c: Emit stub routines for clients
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

#include "wsdl-parse.h"
#include "wsdl-soap-stubs.h"
#include "wsdl-trace.h"
#include "wsdl-soap-emit.h"

static void 
emit_async_callback (FILE                                *out, 
		     const guchar                        *prefix,
		     const wsdl_binding_operation * const op)
{
	wsdl_porttype_operation *porttype_op;

	porttype_op = op->thread_soap_porttype_operation;

	W ("static void\n"
	   "%s_handler (SoupMessage *req, gpointer user_data)\n",
	   prefix);
	W ("{\n");
	W ("\t_wsdl_stubs_callback_data_t *cb_data = user_data;\n");
	W ("\t%s_cb callback;\n\n", prefix);

	if (porttype_op->output) {
		W ("\t/* more args here based on output parts */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t%t _out_%p;\n");
	}

	W ("\n");
	W ("\twsdl_param _out_params []={\n");
	if (porttype_op->output) {
		W ("\t\t/* params here based on output parts */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t{\"%p\", &_out_%p, "
				     "&WSDL_TC_%n_%N_struct},\n");
	}
	W ("\t\t{NULL, NULL, NULL},\n");
	W ("\t};\n");

	W ("\n");

	W ("\tif (!SOUP_MESSAGE_IS_ERROR (req)) {\n");
	W ("\t\t/* Handle results */\n");
	W ("\t\t/* parse req->response.body into callback params */\n");
	W ("\t\twsdl_soap_parse (req->response.body,\n"
	   "\t\t\t\t \"%s\",\n"
	   "\t\t\t\t _out_params,\n"
	   "\t\t\t\t cb_data->env,\n"
	   "\t\t\t\t WSDL_SOAP_FLAGS_RESPONSE);\n",
	   op->name);
	W ("\t}\n\n");

	W ("\tcallback = (%s_cb) cb_data->callback;\n", prefix);
	W ("\tif (callback) {\n");
	W ("\t\tcallback (cb_data->env,\n");

	if (porttype_op->output) {
		W ("\t\t\t  /* output args go here */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t\t  _out_%p,\n");
	}

	W ("\t\t\t  cb_data->user_data);\n");
	W ("\t}\n\n");

	W ("\tsoup_env_free (cb_data->env);\n");
	W ("\tg_free (cb_data);\n");

	W ("}\n");
	W ("\n");
}

static void 
emit_async_stub (FILE                                *out, 
		 const guchar                        *prefix,
		 const guchar                        *opns,
		 const guchar                        *opnsuri,
		 const wsdl_binding_operation * const op)
{
	wsdl_porttype_operation *porttype_op;

	porttype_op = op->thread_soap_porttype_operation;

	W ("void\n"
	   "%s_async (\n", prefix);
	W ("\t\tSoupEnv *env,\n");

	if (porttype_op->input) {
		W ("\t\t\t/* in, in/out parameters */\n");

		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t%t _in_%p,\n");
	}

	W ("\t\t\t/* callback & user_data */\n");
	W ("\t\t%s_cb user_callback,\n", prefix);
	W ("\t\tgpointer user_data)\n");

	W ("{\n");
	W ("\tSoupEnv *_env;\n");
	W ("\tSoupContext *_soup_context;\n");
	W ("\tSoupMessage *_message;\n");
	W ("\t_wsdl_stubs_callback_data_t *_cb_data;\n\n");

	W ("\twsdl_param _in_params[]={\n");
	if (porttype_op->input != NULL) {
		W ("\t\t/* params here based on input parts */\n");

		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t{\"%p\", &_in_%p, "
				     "&WSDL_TC_%n_%N_struct},\n");
	}
	W ("\t\t{NULL, NULL, NULL},\n");
	W ("\t};\n\n");

	W ("\t_env = soup_env_copy (env);\n\n");

#if 0
	/* Destination address override */
	W ("\tif (soup_env_get_address (_env))\n");
	W ("\t\t_soup_context = "
	   "soup_context_get (soup_env_get_address (_env));\n");
	W ("\telse\n");
	W ("\t\t_soup_context = soup_context_get (\"%s\");\n\n",
	   op->soap_operation->soapAction);
#endif

	W ("\t_soup_context = soup_context_get (\"%s\");\n\n",
	   op->soap_operation->soapAction);

	W ("\t_message = soup_env_get_message (_env);\n");
	W ("\t_message->method = SOUP_METHOD_POST;\n");
	W ("\tsoup_message_remove_header (_message->request_headers, "
	   "\"SOAPAction\");\n");
	W ("\tsoup_message_add_header (_message->request_headers,\n"
	   "\t\t\t\t \"SOAPAction\",\n"
	   "\t\t\t\t \"%s\");\n",
	   op->name);
	W ("\tsoup_message_set_context (_message, _soup_context);\n");
	W ("\tsoup_context_unref (_soup_context);\n\n");

	W ("\t_cb_data = g_new0 (_wsdl_stubs_callback_data_t, 1);\n");
	W ("\t_cb_data->callback = (gpointer) user_callback;\n");
	W ("\t_cb_data->user_data = user_data;\n");
	W ("\t_cb_data->env = _env;\n\n");

	W ("\twsdl_soap_marshal (\"%s\",\n"
	   "\t\t\t   \"%s\",\n"
	   "\t\t\t   \"%s\",\n"
	   "\t\t\t   _in_params,\n"
	   "\t\t\t   &_message->request,\n"
	   "\t\t\t   _env,\n"
	   "\t\t\t   WSDL_SOAP_FLAGS_REQUEST);\n\n",
	   op->name, 
	   opns, 
	   opnsuri);

	W ("\tsoup_message_queue (_message,\n"
	   "\t\t\t    %s_handler,\n"
	   "\t\t\t    _cb_data);\n",
	   prefix);

	W ("}\n\n");
}

static void
emit_sync_stub (FILE                                *out, 
		const guchar                        *prefix,
		const guchar                        *opns,
		const guchar                        *opnsuri,
		const wsdl_binding_operation * const op)
{
	wsdl_porttype_operation *porttype_op;

	porttype_op = op->thread_soap_porttype_operation;

	W ("void\n"
	   "%s (\n", prefix);
	W ("\t\tSoupEnv *env\n");

	if (porttype_op->input) {
		W ("\t\t\t/* in, in/out parameters */\n");
		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t,%t _in_%p\n");
	}
	if (porttype_op->output) {
		W ("\t\t\t/* out parameters */\n");
		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t,%t * _out_%p\n");
	}

	W ("\t\t)\n{\n");
	W ("\tSoupContext *_soup_context;\n");
	W ("\tSoupMessage *_message;\n\n");

	W ("\twsdl_param _in_params [] = {\n");
	if (porttype_op->input) {
		W ("\t\t/* params here based on input parts */\n");

		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t{\"%p\", &_in_%p, "
				     "&WSDL_TC_%n_%N_struct},\n");
	}
	W ("\t\t{NULL, NULL, NULL},\n");
	W ("\t};\n\n");

	W ("\twsdl_param _out_params [] = {\n");
	if (porttype_op->output) {
		W ("\t\t/* params here based on output parts */\n");

		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t{\"%p\", _out_%p, "
				     "&WSDL_TC_%n_%N_struct},\n");
	}
	W ("\t\t{NULL, NULL, NULL},\n");
	W ("\t};\n\n");

#if 0
	/* Destination address override */
	W ("\tif (soup_env_get_address (env))\n");
	W ("\t\t_soup_context = "
	   "soup_context_get (soup_env_get_address (env));\n");
	W ("\telse\n");
	W ("\t\t_soup_context = soup_context_get (\"%s\");\n\n",
	   op->soap_operation->soapAction);
#endif

	W ("\t_soup_context = soup_context_get (\"%s\");\n\n",
	   op->soap_operation->soapAction);

	W ("\t_message = soup_env_get_message (env);\n");
	W ("\t_message->method = SOUP_METHOD_POST;\n");
	W ("\tsoup_message_remove_header (_message->request_headers, "
	   "\"SOAPAction\");\n");
	W ("\tsoup_message_add_header (_message->request_headers,\n"
	   "\t\t\t\t \"SOAPAction\",\n"
	   "\t\t\t\t \"%s\");\n",
	   op->name);	
	W ("\tsoup_message_set_context (_message, _soup_context);\n");
	W ("\tsoup_context_unref (_soup_context);\n\n");

	W ("\twsdl_soap_marshal (\"%s\",\n"
	   "\t\t\t   \"%s\",\n"
	   "\t\t\t   \"%s\",\n"
	   "\t\t\t   _in_params,\n"
	   "\t\t\t   &_message->request,\n"
	   "\t\t\t   env,\n"
	   "\t\t\t   WSDL_SOAP_FLAGS_REQUEST);\n\n",
	   op->name, 
	   opns, 
	   opnsuri);

	W ("\tsoup_message_send (_message);\n\n");

	W ("\tif (!SOUP_MESSAGE_IS_ERROR (_message)) {\n");
	W ("\t\t/* Handle results */\n");
	W ("\t\t/* parse _message->response.body into callback params */\n");
	W ("\t\twsdl_soap_parse (_message->response.body,\n"
	   "\t\t\t\t \"%s\",\n"
	   "\t\t\t\t _out_params,\n"
	   "\t\t\t\t env,\n"
	   "\t\t\t\t WSDL_SOAP_FLAGS_RESPONSE);\n",
	   op->name);
	W ("\t}\n");

	W ("}\n\n");
}

static void
wsdl_emit_soap_stubs_binding_operation (
				FILE                                *out, 
				const guchar                        *opns,
				const guchar                        *opnsuri,
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

	/* Emit the async callback */
	emit_async_callback (out, prefix, op);

	/* Emit async client stub */
	emit_async_stub (out, prefix, opns, opnsuri, op);

	/* And now do the synchronous client stub */
	emit_sync_stub (out, prefix, opns, opnsuri, op);

	W ("/* END Binding Operation %s */\n\n", op->name);

	g_free (prefix);
}

static void
wsdl_emit_soap_stubs_binding (FILE                      *out, 
			      const guchar              *opns,
			      const guchar              *opnsuri,
			      const wsdl_binding * const binding)
{
	GSList *iter;
	wsdl_porttype *porttype;

	g_assert (binding->name != NULL);

	W ("/* BEGIN Binding %s */\n\n", binding->name);

	if (binding->documentation)
		W ("/* %s */\n\n", binding->documentation->str);

	porttype = binding->thread_soap_porttype;

	g_assert (porttype != NULL);

	/* For each binding operation, output a function */
	iter = binding->operations;
	while (iter) {
		wsdl_emit_soap_stubs_binding_operation (out, 
							opns, 
							opnsuri,
							iter->data);

		iter = iter->next;
	}

	W ("/* END Binding %s */\n\n", binding->name);
}

static void
wsdl_emit_soap_stubs_service (FILE                      *out, 
			      const guchar              *opns,
			      const guchar              *opnsuri,
			      const wsdl_service * const service)
{
	GSList *iter;

	g_assert (service->name != NULL);

	W ("/* BEGIN Service %s */\n\n", service->name);

	if (service->documentation)
		W ("/* %s */\n\n", service->documentation->str);

	iter = service->thread_soap_ports;
	while (iter) {
		wsdl_service_port *port = iter->data;

		g_assert (port->thread_soap_binding != NULL);

		wsdl_emit_soap_stubs_binding (out, 
					      opns, 
					      opnsuri,
					      port->thread_soap_binding);

		iter = iter->next;
	}

	W ("/* END Service %s */\n\n", service->name);
}

/**
 * wsdl_emit_soap_stubs:
 * @outdir: a string containing the path to a directory.  This
 * function expects the string to have a trailing '/'.
 * @fileroot: a string containing the root of a filename.  "-stubs.c"
 * will be appended to this name.
 * @definitions: a pointer to a #wsdl_definitions structure,
 * containing a set of WSDL elements.
 *
 * Creates the file @outdir/@fileroot-stubs.c, and writes C code
 * containing client stubs.
 */
void
wsdl_emit_soap_stubs (const guchar           * outdir, 
		      const guchar           * fileroot,
		      const wsdl_definitions * const definitions)
{
	GSList *iter;
	guchar *filename;
	const guchar *opns;

	filename = g_strconcat (outdir, fileroot, "-stubs.c", NULL);
	wsdl_debug (WSDL_LOG_DOMAIN_STUBS, 
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
	} else {
		opns = "m";
	}

	W (" *\n");
	W (" * Automatically generated by soup-wsdl.\n");
	W (" */\n");
	W ("\n");
	W ("#include <glib.h>\n");
	W ("#include <libsoup/soup.h>\n");
	W ("#include <libwsdl/wsdl.h>\n");
	W ("#include \"%s.h\"\n\n", fileroot);

	if (definitions->documentation)
		W ("/* %s */\n\n", definitions->documentation->str);

	/* Declare structure for passing callback data */
	W ("typedef struct {\n");
	W ("\tgpointer callback;\n");
	W ("\tgpointer user_data;\n");
	W ("\tSoupEnv *env;\n");
	W ("} _wsdl_stubs_callback_data_t;\n\n");

	iter = definitions->thread_soap_services;
	while (iter) {
		wsdl_emit_soap_stubs_service (wsdl_get_output_file (), 
					      opns,
					      definitions->targetNamespace,
					      iter->data);

		iter = iter->next;
	}

	fclose (wsdl_get_output_file ());
	wsdl_set_output_file (NULL);
}
