/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-headers.c: Emit code for header files
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

#include <libwsdl/wsdl-typecodes.h>
#include <libwsdl/wsdl-typecodes-c.h>

#include "wsdl-parse.h"
#include "wsdl-soap-headers.h"
#include "wsdl-trace.h"
#include "wsdl-locate.h"
#include "wsdl-soap-emit.h"

static void
wsdl_emit_soap_header_typecodes (const wsdl_typecode * const tc,
				 gpointer                    user_data)
{
	FILE *out = (FILE *) user_data;

	wsdl_typecode_write_c_declaration (out, tc);
}

static void
wsdl_emit_soap_header_mm (const wsdl_typecode * const tc, gpointer user_data)
{
	FILE *out = (FILE *) user_data;

	wsdl_typecode_write_c_mm_decl (out, tc);
}

static void
wsdl_emit_soap_headers_binding_operation (
				FILE                               *out, 
				const guchar                       *opns,
				const wsdl_binding_operation *const op)
{
	gchar *prefix;
	wsdl_porttype_operation *porttype_op;

	g_assert (op->name != NULL);
	g_assert (op->soap_operation != NULL);

	W ("/* BEGIN Binding Operation %s */\n", op->name);

	porttype_op = op->thread_soap_porttype_operation;

	g_assert (porttype_op != NULL);
	g_assert (porttype_op->input != NULL);

	if (op->documentation)
		W ("/* %s */\n", op->documentation->str);
	else 
		W ("\n");

	if (porttype_op->documentation)
		W ("/* %s */\n", porttype_op->documentation->str);

	prefix = g_strconcat (opns, "_", op->name, NULL);

	/* 
	 * Emit the user callback function typedef 
	 */
	W ("typedef void (*%s_cb) \n"
	   "\t\t\t       (SoupEnv *env,\n", 
	   prefix);

	if (porttype_op->output)
		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t\t\t%t %p\t/* [out] */,\n");

	W ("\t\t\t\tgpointer user_data);\n");
	W ("\n");

	/* 
	 * Emit the client stub prototype 
	 */
	W ("void %s_async \n\t\t\t       (SoupEnv *env,\n", prefix);

	if (porttype_op->input)
		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t\t\t%t %p,\t/* [in] */\n");

	W ("\t\t\t\t%s_cb callback,\n", prefix);
	W ("\t\t\t\tgpointer user_data);\n");
	W ("\n");

	/* 
	 * Emit the client synchronous stub prototype 
	 */
	W ("void %s \n\t\t\t       (SoupEnv *env\n", prefix);

	if (porttype_op->input)
		wsdl_emit_part_list (out, 
				     porttype_op->input->thread_soap_parts,
				     "\t\t\t\t,%t _in_%p\t/* [in] */\n");

	if (porttype_op->output)
		wsdl_emit_part_list (out,
				     porttype_op->output->thread_soap_parts,
				     "\t\t\t\t,%t * _out_%p\t/* [out] */\n");

	W ("\t\t\t\t);\n\n");

	W ("/* END Binding Operation %s */\n\n", op->name);

	g_free (prefix);
}

static void
wsdl_emit_soap_headers_binding (FILE                      *out, 
				const guchar              *opns,
				const wsdl_binding * const binding)
{
	GSList *iter;
	wsdl_porttype *porttype;

	g_assert (binding->name != NULL);

	W ("/* BEGIN Binding %s */\n", binding->name);

	if (binding->documentation)
		W ("/* %s */\n\n", binding->documentation->str);

	porttype = binding->thread_soap_porttype;
	g_assert (porttype != NULL);

	/* 
	 * For each binding operation, output a function prototype and
	 * a context structure
	 */
	iter = binding->operations;
	while (iter) {
		wsdl_emit_soap_headers_binding_operation (out, 
							  opns,
							  iter->data);

		iter = iter->next;
	}

	/*
	 * Output a server callback structure and a server registration
	 * function.
	 */
	W ("typedef struct {\n");
	for (iter = binding->operations; iter; iter = iter->next) {
		wsdl_binding_operation *op;
		wsdl_porttype_operation *porttype_op;

		op = iter->data;
		porttype_op = op->thread_soap_porttype_operation;

		/* 
		 * Emit the server callback function type 
		 */
		W ("\tvoid (*%s) \t(SoupEnv *env,\n", op->name);

		if (porttype_op->input)
			wsdl_emit_part_list (
				out, 
				porttype_op->input->thread_soap_parts,
				"\t\t\t\t\t %t _in_%p,\t/* [in] */\n");

		if (porttype_op->output)
			wsdl_emit_part_list (
				out,
				porttype_op->output->thread_soap_parts,
				"\t\t\t\t\t %t * _out_%p,\t/* [out] */\n");

		W ("\t\t\t\t\t gpointer user_data);\n");
	}
	W ("} %s_handlers;\n\n", opns);

	W ("void %s_register (SoupServer \t\t\t*server,\n"
	   "\t\t\t  SoupServerAuthContext \t*auth_ctx,\n"
	   "\t\t\t  %s_handlers \t*handlers,\n"
	   "\t\t\t  gpointer \t\t\tuser_data);\n\n",
	   opns,
	   opns);

	W ("SoupServer *%s_get_server (void);\n\n", opns);

	W ("/* END Binding %s */\n\n", binding->name);
}

static void
wsdl_emit_soap_headers_service (FILE                      *out, 
				const guchar              *opns,
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

		wsdl_emit_soap_headers_binding (out, 
						opns,
						port->thread_soap_binding);

		iter = iter->next;
	}

	W ("/* END Service %s */\n\n", service->name);
}

/**
 * wsdl_emit_soap_headers:
 * @outdir: a string containing the path to a directory.  This
 * function expects the string to have a trailing '/'.
 * @fileroot: a string containing the root of a filename.  ".h" will
 * be appended to this name.
 * @definitions: a pointer to a #wsdl_definitions structure,
 * containing a set of WSDL elements.
 *
 * Creates the file @outdir/@fileroot.h, and writes C code containing
 * typecode declarations, and function prototypes for client stubs,
 * server skeletons and common code.
 */
void
wsdl_emit_soap_headers (const guchar                  *outdir, 
			const guchar                  *fileroot,
			const wsdl_definitions * const definitions)
{
	GSList *iter;
	guchar *filename;
	const guchar *opns;

	filename = g_strconcat (outdir, fileroot, ".h", NULL);

	wsdl_debug (WSDL_LOG_DOMAIN_HEADERS, 
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
	W ("#ifndef _%s_H_\n", opns);
	W ("#define _%s_H_\n\n", opns);

	W ("#include <glib.h>\n");
	W ("#include <libsoup/soup.h>\n");
	W ("#include <libsoup/soup-server.h>\n");
	W ("#include <libwsdl/wsdl.h>\n\n");

	if (definitions->documentation) {
		W ("/* %s */\n", definitions->documentation->str);
		W ("\n");
	}

	/* 
	 * call wsdl_typecode_write_c_declaration() for each non-simple
	 * typecode known.
	 */

	wsdl_typecode_foreach (FALSE, 
			       wsdl_emit_soap_header_typecodes, 
			       wsdl_get_output_file ());
	wsdl_typecode_foreach (FALSE, 
			       wsdl_emit_soap_header_mm, 
			       wsdl_get_output_file ());

	iter = definitions->thread_soap_services;
	while (iter) {
		wsdl_emit_soap_headers_service (wsdl_get_output_file (), 
						opns, 
						iter->data);

		iter = iter->next;
	}

	W ("#endif /* _%s_H_ */\n", opns);

	fclose (wsdl_get_output_file ());
	wsdl_set_output_file (NULL);
}
