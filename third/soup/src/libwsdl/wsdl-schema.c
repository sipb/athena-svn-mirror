/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-schema.c: Build typecodes from XML describing one of the known
 * type systems
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>

#include <libxml/tree.h>

#include "wsdl-schema.h"
#include "wsdl-schema-glib.h"

static void (*schema_func_start) (const xmlDocPtr, 
				  const xmlNodePtr,
				  const xmlChar **, 
				  const guchar *,
				  const guchar *, 
				  WsdlErrorMsgFn err_msg) = NULL;
static void (*schema_func_end) (const xmlNodePtr,
				WsdlErrorMsgFn err_msg) = NULL;
static void (*schema_func_error) (const gchar * fmt, ...) = NULL;

/**
 * WSDLNS:
 *
 * The WSDL namespace URI.
 */

/**
 * SOAPNS:
 *
 * The SOAP namespace URI.
 */

/**
 * XSDNS:
 *
 * The XML Schema Description namespace URI.
 */

/**
 * GLIBNS:
 *
 * The Ximian simple glib schema namespace URI.
 */

/**
 * wsdl_qnamecmp:
 * @node: a node in an XML parse tree
 * @ns: a string containing a namespace URI
 * @localname: a string without a namespace prefix
 *
 * Checks that the node name is @localname and belongs to the
 * namespace @ns.
 *
 * Returns: #TRUE for "match", #FALSE otherwise.
 */
gboolean
wsdl_qnamecmp (const xmlNodePtr  node, 
	       const xmlChar    *ns,
	       const xmlChar    *localname)
{
	int ret;

	g_assert (node != NULL);
	g_assert (node->name != NULL);
	g_assert (node->ns != NULL);
	g_assert (node->ns->href != NULL);
	g_assert (ns != NULL);
	g_assert (localname != NULL);

	/* If the local parts don't match, return straight away */
	ret = strcmp (localname, node->name);
	if (ret != 0) {
		return (FALSE);
	}

	/* Now check that the node namespace matches ns */
	ret = strcmp (ns, node->ns->href);
	if (ret != 0) {
		return (FALSE);
	}

	return (TRUE);
}

/**
 * wsdl_attrnscmp:
 * @node: a node in an XML parse tree
 * @attr: a string with an optional prefix deliminated by a ':'
 * @ns_href: a string containing a namespace URI
 *
 * Checks that the namespace prefix of @attr matches the defined
 * namespace @ns_href, based on the XML namespaces in scope for @node.
 * If there is no prefix in @attr then a @ns_href of "" matches.
 *
 * Returns: #TRUE for "match", #FALSE otherwise.
 */
gboolean
wsdl_attrnscmp (const xmlNodePtr  node, 
		const guchar     *attr,
		const guchar     *ns_href)
{
	xmlNodePtr nptr;
	xmlNsPtr ns;
	guchar *colon;
	guchar *copy;
	gboolean ret = FALSE;

	g_assert (node != NULL);
	g_assert (attr != NULL);
	g_assert (ns_href != NULL);

	copy = g_strdup (attr);
	colon = strchr (copy, ':');
	if (colon != NULL) {
		/* There is a namespace prefix. Now 'copy' contains
		 * the prefix and 'colon' the local part.
		 */
		*colon++ = '\0';

		/* Need to find all namespace definitions from this
		 * node up the parent chain, to the root.  (If theres
		 * an easier way to find all the currently in-scope
		 * namespace definitions, I'd like to know)
		 */

		nptr = node;

		do {
			ns = nptr->nsDef;

			while (ns != NULL) {
				/* ns->prefix can be NULL, denoting
				 * the global namespace.
				 */
				if (ns->prefix != NULL &&
				    !strcmp (copy, ns->prefix) &&
				    !strcmp (ns_href, ns->href)) {
					ret = TRUE;
					goto end;
				}

				ns = ns->next;
			}

			nptr = nptr->parent;
		} while (nptr != NULL);
	} else {
		/* No namespace prefix, so only the blank namespace
		 * matches
		 * (should the global namespace match too? - RHP)
		 */
		if (!strcmp (ns_href, "")) {
			ret = TRUE;
			goto end;
		}
	}

      end:
	g_free (copy);
	return (ret);
}

/**
 * wsdl_prefix_to_namespace:
 * @doc: an XML document
 * @node: a node in the XML parse tree
 * @str: a string with an optional prefix deliminated by a ':'
 * @defns: whether the default namespace can match a missing prefix
 *
 * Finds the namespace prefix of @str in scope for @node.  If @defns is #TRUE
 * and the prefix is not specified, it uses the default namespace.
 *
 * Returns: a namespace URI corresponding to the prefix in @str.
 */
const guchar *
wsdl_prefix_to_namespace (const xmlDocPtr   doc, 
			  const xmlNodePtr  node,
			  const guchar     *str, 
			  gboolean          defns)
{
	xmlNsPtr nsptr;
	guchar *colon;
	guchar *ns;

	ns = g_strdup (str);

	if ((colon = strchr (ns, ':')) != NULL) {
		*colon = '\0';
	} else {
		g_free (ns);
		ns = NULL;
	}

	if (ns == NULL && defns == FALSE) {
		return (NULL);
	}

	nsptr = xmlSearchNs (doc, node, ns);

	if (ns != NULL) {
		g_free (ns);
	}


	if (nsptr != NULL) {
		return (nsptr->href);
	} else {
		return (NULL);
	}
}

static gboolean
wsdl_schema_glib_parse_attrs (const xmlChar ** attrs)
{
	int i = 0;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "fill-in-here")) {
				/* g_strdup(attrs[i+1]) */
			} else if (!strcmp (attrs[i], "xmlns") ||
				   !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				/* unknown element attribute */
			}

			i += 2;
		}
	}

	/* Check that all required attrs are set */
	if (1) {
		/* set pointer */
		return (TRUE);
	} else {
		/* free anything that has been alloced */
		return (FALSE);
	}
}

/**
 * wsdl_schema_init:
 * @node: a node in an XML parse tree
 * @attrs: an array of XML UTF-8 strings containing the XML node attributes
 * @error_msg: a pointer to a function to be called with any error
 * messages to be displayed
 *
 * Sets up the functions called by wsdl_schema_start_element() and
 * wsdl_schema_end_element() by comparing the @node name and namespace
 * to a list of known schemas.  Schema options are set from @attrs,
 * which vary depending on the schema.
 *
 * Returns: #TRUE if the namespace of @node is known, #FALSE otherwise.
 */
gboolean
wsdl_schema_init (const xmlNodePtr   node, 
		  const xmlChar    **attrs,
		  WsdlErrorMsgFn     error_msg)
{
	if (0 && wsdl_qnamecmp (node, XSDNS, "schema") == TRUE) {
		/* This section will be filled in sometime, honest! */
		return (TRUE);
	} else if (wsdl_qnamecmp (node, GLIBNS, "type") == TRUE) {
		schema_func_start = wsdl_schema_glib_start_element;
		schema_func_end = wsdl_schema_glib_end_element;
		schema_func_error = error_msg;

		wsdl_schema_glib_parse_attrs (attrs);

		return (TRUE);
	} else {
		return (FALSE);
	}
}

/**
 * wsdl_schema_start_element:
 * @doc: an XML document
 * @node: a node in the XML parse tree
 * @attrs: an array of XML UTF-8 strings containing the XML node attributes
 * @ns: a string containing a namespace reference
 * @nsuri: a string containing a namespace URI
 *
 * Calls the schema parser configured by wsdl_schema_init() with a new
 * element @node.  @ns and @nsuri are used to define the namespace of
 * any typecodes created.
 */
void
wsdl_schema_start_element (const xmlDocPtr    doc, 
			   const xmlNodePtr   node,
			   const xmlChar    **attrs, 
			   const guchar      *ns,
			   const guchar      *nsuri)
{
	if (schema_func_start != NULL) {
		schema_func_start (doc, node, attrs, ns, nsuri,
				   schema_func_error);
	}
}

/**
 * wsdl_schema_end_element:
 * @node: a node in an XML parse tree
 *
 * Calls the schema parser configured by wsdl_schema_init() to close
 * an element.
 */
void
wsdl_schema_end_element (const xmlNodePtr node)
{
	if (schema_func_end != NULL) {
		schema_func_end (node, schema_func_error);
	}
}
