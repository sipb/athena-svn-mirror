/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-parse.c: Runtime SOAP document parser
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <libsoup/soup-parser.h>

#include "wsdl-soap-parse.h"
#include "wsdl-typecodes.h"
#include "wsdl-soap-memory.h"

static void wsdl_soap_set_param (const xmlDocPtr             xml_doc, 
				 const xmlNodePtr            node,
				 const wsdl_typecode * const typecode,
				 gpointer                    storage);

static void
wsdl_soap_set_simple_param (const xmlChar              *str,
			    const wsdl_typecode * const typecode,
			    gpointer                    storage_contents)
{
	g_return_if_fail (str != NULL);
	g_return_if_fail (typecode != NULL);
	g_return_if_fail (storage_contents != NULL);

	switch (wsdl_typecode_kind (typecode)) {
	case WSDL_TK_GLIB_NULL:
		g_warning ("Invalid typecode NULL in " G_GNUC_FUNCTION);
		break;

	case WSDL_TK_GLIB_VOID:
		/* Nothing to read here */
		break;

	case WSDL_TK_GLIB_BOOLEAN:
		{
			gboolean *boolp = (gboolean *) storage_contents;

			if (!g_strcasecmp (str, "true") ||
			    !g_strcasecmp (str, "yes")) {
				*boolp = TRUE;
			} else if (!g_strcasecmp (str, "false") ||
				   !g_strcasecmp (str, "no")) {
				*boolp = FALSE;
			} else {
				*boolp = strtol (str, NULL, 0);
			}

			break;
		}

	case WSDL_TK_GLIB_CHAR:
		{
			gchar *charp = (gchar *) storage_contents;

			*charp = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_UCHAR:
		{
			guchar *ucharp = (guchar *) storage_contents;

			*ucharp = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_INT:
		{
			gint *intp = (gint *) storage_contents;

			*intp = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_UINT:
		{
			guint *uintp = (guint *) storage_contents;

			*uintp = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_SHORT:
		{
			gshort *shortp = (gshort *) storage_contents;

			*shortp = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_USHORT:
		{
			gushort *ushortp = (gushort *) storage_contents;

			*ushortp = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_LONG:
		{
			glong *longp = (glong *) storage_contents;

			*longp = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_ULONG:
		{
			gulong *ulongp = (gulong *) storage_contents;

			*ulongp = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_INT8:
		{
			gint8 *int8p = (gint8 *) storage_contents;

			*int8p = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_UINT8:
		{
			guint8 *uint8p = (guint8 *) storage_contents;

			*uint8p = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_INT16:
		{
			gint16 *int16p = (gint16 *) storage_contents;

			*int16p = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_UINT16:
		{
			guint16 *uint16p = (guint16 *) storage_contents;

			*uint16p = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_INT32:
		{
			gint32 *int32p = (gint32 *) storage_contents;

			*int32p = strtol (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_UINT32:
		{
			guint32 *uint32p = (guint32 *) storage_contents;

			*uint32p = strtoul (str, NULL, 0);
			break;
		}

	case WSDL_TK_GLIB_FLOAT:
		{
			gfloat *floatp = (gfloat *) storage_contents;

			*floatp = strtod (str, NULL);
			break;
		}

	case WSDL_TK_GLIB_DOUBLE:
		{
			gdouble *doublep = (gdouble *) storage_contents;

			*doublep = strtod (str, NULL);
			break;
		}

	case WSDL_TK_GLIB_STRING:
		{
			guchar **strp = (guchar **) storage_contents;

			*strp = g_strdup (str);
			break;
		}

	case WSDL_TK_GLIB_ELEMENT:
	case WSDL_TK_GLIB_STRUCT:
	case WSDL_TK_GLIB_LIST:
		/* Handled elsewhere */
		break;
	case WSDL_TK_GLIB_MAX:
		g_warning ("Invalid typecode MAX in " G_GNUC_FUNCTION);
		break;
	}
}

static void
wsdl_soap_set_struct_param (const xmlDocPtr             xml_doc, 
			    const xmlNodePtr            strnode,
			    const wsdl_typecode * const typecode,
			    gpointer                    storage)
{
	xmlNodePtr node;
	gpointer item = wsdl_typecode_alloc (typecode);

	node = strnode->xmlChildrenNode;
	while (node != NULL) {
		const wsdl_typecode *subtype;
		guint offset;

		subtype = wsdl_typecode_offset (typecode, node->name, &offset);

		if (subtype != NULL) {
			/* For some reason gcc moans about taking
			 * offsets from void * pointers
			 */
			wsdl_soap_set_param (
				xml_doc, 
				node, 
				subtype,
				ALIGN_ADDRESS (
					(guchar *) item + offset,
					wsdl_typecode_find_alignment(subtype)));
		} else {
			g_warning ("Couldn't find [%s] in %s "
				   "typecode parameter list!",
				   node->name, 
				   typecode->name);
		}

		node = node->next;
	}

	*(guchar **) storage = item;
}

static void
wsdl_soap_set_list_param (const xmlDocPtr             xml_doc, 
			  const xmlNodePtr            node,
			  const wsdl_typecode * const typecode,
			  gpointer                    storage)
{
	/* storage holds the pointer to a GSList */
	/* typecode defines what we will be sticking in the list */
	/* node contains the XML for one element */
	gpointer item = wsdl_typecode_alloc (typecode);
	GSList **listp = (GSList **) storage;

	wsdl_soap_set_param (xml_doc, node, typecode, item);

	/* List items that are naturally pointers have the extra layer
	 * of indirection eliminated.
	 */
	if (wsdl_typecode_is_simple (typecode) == FALSE ||
	    wsdl_typecode_element_kind (typecode) == WSDL_TK_GLIB_STRING) {
		*listp = g_slist_append (*listp, *(guchar **) item);
	} else {
		*listp = g_slist_append (*listp, item);
	}
}

static void
wsdl_soap_set_param (const xmlDocPtr             xml_doc, 
		     const xmlNodePtr            node,
		     const wsdl_typecode * const typecode, 
		     gpointer                    storage)
{
	wsdl_typecode_kind_t kind;

	kind = wsdl_typecode_kind (typecode);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		/* The real type is stored in element 0 of subtypes[] */
		wsdl_soap_set_param (xml_doc, node, typecode->subtypes[0],
				     storage);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		/* Fill in each structure element */
		wsdl_soap_set_struct_param (xml_doc, node, typecode, storage);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		/* Fill in a list of the type stored in element 0 of
		 * subtypes[]
		 */
		wsdl_soap_set_list_param (xml_doc, node->xmlChildrenNode,
					  typecode->subtypes[0], storage);
	} else {
		xmlChar *str;

		str = xmlNodeListGetString (xml_doc, node->xmlChildrenNode, 1);
		wsdl_soap_set_simple_param (str, typecode, storage);
		xmlFree (str);
	}
}

static void
wsdl_soap_operation (const xmlDocPtr    xml_doc, 
		     const xmlNodePtr   body,
		     const guchar      *operation, 
		     const wsdl_param  *params,
		     SoupFault        **fault)
{
	xmlNodePtr node;
	const wsdl_param *param;

	node = body->xmlChildrenNode;

	/* check if the body contains a fault */
	if (!strcmp (body->name, "Fault")) {
		xmlChar *code = NULL, *string = NULL;
		xmlChar *actor = NULL, *detail = NULL;
		xmlNodePtr subnode;

		if (node == NULL) {
			g_warning ("Fault returned, but it is empty!");
			return;
		}

		/* retrieve all info about the fault */
		for (subnode = node; subnode; subnode = subnode->next) {
			xmlChar *str;

			str = xmlNodeListGetString (xml_doc,
						    subnode->xmlChildrenNode,
						    1);
			if (!strcmp (subnode->name, "faultcode"))
				code = str;
			else if (!strcmp (subnode->name, "faultstring"))
				string = str;
			else if (!strcmp (subnode->name, "faultactor"))
				actor = str;
			else if (!strcmp (subnode->name, "detail"))
				detail = str;	/* FIXME */
		}

		*fault = soup_fault_new (code, string, actor, detail);

		/* free memory */
		if (code)
			xmlFree (code);
		if (string)
			xmlFree (string);
		if (actor)
			xmlFree (actor);
		if (detail)
			xmlFree (detail);
		return;
	} else if (strcmp (body->name, operation) != 0) {
		g_warning ("Expecting operation [%s], got [%s]", operation,
			   body->name);
		return;
	}

	while (node != NULL) {
		param = params;

		while (param->name != NULL) {
			if (strcmp (param->name, node->name) == 0) {
				wsdl_soap_set_param (xml_doc, 
						     node,
						     param->typecode,
						     param->param);
				break;
			}
			param++;
		}

		if (param->name == NULL) {
			g_warning ("Couldn't find [%s] in "
				   "known parameter list!",
				   node->name);
		}

		node = node->next;
	}
}

static void
wsdl_soap_headers (const xmlDocPtr   xml_doc, 
		   const xmlNodePtr  header,
		   SoupEnv          *env, 
		   gint              flags)
{
	xmlNodePtr node;

	for (node = header; node; node = node->next) {
		SoupSOAPHeader hdr;
		xmlChar *mu;

		hdr.name = (xmlChar *) node->name;

		if (node->ns)
			hdr.ns_uri = (xmlChar *) node->ns->href;
		
		/* FIXME: Use xmlGetNsProp here */
		hdr.actor_uri = xmlGetProp (node, "actor");

		/* FIXME: Use xmlGetNsProp here */
		mu = xmlGetProp (node, "mustUnderstand");
		if (mu) {
			if (!strcmp (mu, "1"))
				hdr.must_understand = TRUE;
			else if (!strcmp (mu, "0"))
				hdr.must_understand = FALSE;
			xmlFree (mu);
		}

		hdr.value = xmlNodeListGetString (xml_doc, 
						  node->xmlChildrenNode,
						  1);

		soup_env_add_recv_header (env, &hdr);

		if (hdr.actor_uri)
			xmlFree (hdr.actor_uri);

		if (hdr.value)
			xmlFree (hdr.value);
	}
}

/**
 * wsdl_soap_parse:
 * @xmltext: a string containing an XML SOAP response
 * @operation: a string containing the name of the operation being
 * parsed
 * @params: a pointer to an array of #wsdl_param, terminated by a set
 * of #NULL elements
 * @env: a pointer to a #SoupEnv struct, which is expected to have
 * been initialised previously by soup_env_new()
 * @flags: an integer holding the bit flags #WSDL_SOAP_FLAGS_REQUEST
 * or #WSDL_SOAP_FLAGS_RESPONSE
 *
 * Parses @xmltext, filling in the memory pointed to by elements of
 * @params by constructing C instances of typecodes.
 *
 * Returns: 0
 */
guint
wsdl_soap_parse (const guchar            *xmltext, 
		 const guchar            *operation,
		 const wsdl_param * const params, 
		 SoupEnv                 *env, 
		 gint                     flags)
{
	xmlDocPtr xml_doc;
	xmlNsPtr env_ns;
	xmlNodePtr cur, body;
	SoupFault *fault = NULL;

	/* I'd use g_assert, but this is in a library :-) */
	if (xmltext == NULL || params == NULL) {
		g_warning ("No XML or params");
		return (0);
	}

	LIBXML_TEST_VERSION;
	xmlKeepBlanksDefault (0);

	/* Have to lose the const here :-( */
	xml_doc = xmlParseMemory ((guchar *) xmltext, (int) strlen (xmltext));
	if (xml_doc == NULL) {
		g_warning ("XML parse failed");
		return (0);
	}

	cur = xmlDocGetRootElement (xml_doc);
	if (cur == NULL) {
		g_warning ("Couldn't get root element");
		xmlFreeDoc (xml_doc);
		return (0);
	}
	env_ns = 
		xmlSearchNsByHref (xml_doc, 
				   cur,
				   "http://schemas.xmlsoap.org/soap/envelope/");
	if (env_ns == NULL) {
		/* wrong type, env ns not found */
		g_warning ("Wrong XML doc type, SOAP Envelope namespace "
			   "not found");
		xmlFreeDoc (xml_doc);
		return (0);
	}
	if (strcmp (cur->name, "Envelope") != 0) {
		/* wrong type, root node != Envelope */
		g_warning ("Wrong XML doc type, root node isn't SOAP:Envelope");
		xmlFreeDoc (xml_doc);
		return (0);
	}

	/* Zero the memory that will be filled in with type data */
	wsdl_soap_initialise (params);

	/* Inside the Envelope is an optional 'Header' and a mandatory
	 * 'Body'
	 */

	/* Locate the Body */
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		body = cur->xmlChildrenNode;
		if (strcmp (cur->name, "Header") == 0) {
			/* Now process headers */
			if (body != NULL) {
				wsdl_soap_headers (xml_doc, body, env, flags);
			}
		} else if (strcmp (cur->name, "Body") == 0) {
			/* Now walk the tree */
			if (body != NULL) {
				wsdl_soap_operation (xml_doc, 
						     body, 
						     operation,
						     params, 
						     &fault);
				if (fault) {
					soup_env_set_fault (env, fault);
					soup_fault_free (fault);
				}
			}
		}

		cur = cur->next;
	}

	xmlFreeDoc (xml_doc);

	return (0);
}
