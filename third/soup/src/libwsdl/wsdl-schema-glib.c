/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-schema-glib.c: Build typecodes from XML describing a simple 
 * glib-based type system.
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
#include "wsdl-typecodes.h"

typedef enum {
	GLIB_TOPLEVEL,
	GLIB_ELEMENT,
	GLIB_STRUCT,
	GLIB_STRUCT_ELEMENT,
	GLIB_LIST,
	GLIB_UNKNOWN,
	GLIB_STATE_MAX,
} wsdl_schema_glib_state_t;

static wsdl_typecode *tmptc = NULL;
static wsdl_schema_glib_state_t state, last_known_state;
static guint unknown_depth;

static gboolean
wsdl_schema_glib_parse_element_attrs (const xmlChar **attrs, 
				      guchar        **namep,
				      guchar        **typep)
{
	int i = 0;
	guchar *name = NULL, *type = NULL;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				name = g_strdup (attrs[i + 1]);
			} else if (!strcmp (attrs[i], "type") ||
				   !strcmp (attrs[i], "element")) {
				type = g_strdup (attrs[i + 1]);
			} else if (!strcmp (attrs[i], "xmlns") ||
				   !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				/* unknown element attribute */
			}

			i += 2;
		}
	}

	/* Check that name and type have values */
	if (name != NULL && type != NULL) {
		*namep = name;
		*typep = type;

		return (TRUE);
	} else {
		if (name != NULL) {
			g_free (name);
		}
		if (type != NULL) {
			g_free (type);
		}

		return (FALSE);
	}
}

static gboolean
wsdl_schema_glib_parse_struct_attrs (const xmlChar **attrs, guchar **namep)
{
	int i = 0;
	guchar *name = NULL;

	if (attrs != NULL) {
		while (attrs[i] != NULL) {
			if (!strcmp (attrs[i], "name")) {
				name = g_strdup (attrs[i + 1]);
			} else if (!strcmp (attrs[i], "xmlns") ||
				   !strncmp (attrs[i], "xmlns:", 6)) {
				/* Do nothing */
			} else {
				/* unknown element attribute */
			}

			i += 2;
		}
	}

	/* Check that name has a value */
	if (name != NULL) {
		*namep = name;
		return (TRUE);
	} else {
		return (FALSE);
	}
}

static void
wsdl_schema_glib_parse_struct (const xmlDocPtr    doc, 
			       const xmlNodePtr   node,
			       const xmlChar    **attrs,
			       WsdlErrorMsgFn     error_msg)
{
	if (error_msg == NULL) {
		error_msg = g_print;
	}

	if (wsdl_qnamecmp (node, GLIBNS, "element") == TRUE) {
		const wsdl_typecode *subtc;
		guchar *tcname = NULL, *type = NULL;
		const guchar *tnsuri;
		gboolean attrsok;

		g_assert (tmptc != NULL);

		attrsok = wsdl_schema_glib_parse_element_attrs (attrs, 
								&tcname,
								&type);
		if (attrsok == FALSE) {
			return;
		}

		tnsuri = wsdl_prefix_to_namespace (doc, node, type, FALSE);

		if ((subtc = wsdl_typecode_lookup (type, tnsuri)) == NULL) {
			/* Subtype not defined */

			if (tnsuri == NULL) {
				error_msg ("%s is not known", type);
			} else {
				error_msg ("%s is not known in the "
					   "%s namespace",
					   type, 
					   tnsuri);
			}

			g_free (tcname);
			g_free (type);
			return;
		}

		g_free (type);

		tmptc->subnames = g_renew (const guchar *, 
					   tmptc->subnames,
					   tmptc->sub_parts + 1);
		tmptc->subnames[tmptc->sub_parts] = tcname;

		tmptc->subtypes = g_renew (const wsdl_typecode *, 
					   tmptc->subtypes,
					   tmptc->sub_parts + 1);

		tmptc->subtypes[tmptc->sub_parts] = subtc;
		tmptc->sub_parts += 1;

		state = GLIB_STRUCT_ELEMENT;
	} else {
		last_known_state = state;
		state = GLIB_UNKNOWN;
		g_assert (unknown_depth == 0);
		unknown_depth++;
	}
}

/**
 * wsdl_schema_glib_start_element:
 * @doc: a pointer to an XML document tree
 * @node: a pointer to a node within the XML document tree
 * @attrs: an array of strings containing attributes of @node
 * @ns: a string containing a namespace reference
 * @nsuri: a string containing a namespace URI
 * @error_msg: a pointer to a function to be called with any error
 * messages to be displayed
 *
 * Implements the parsing of the simple Ximian glib schema, by being
 * called by wsdl_parse() via wsdl_schema_start_element() with each
 * @node in turn of a schema definition, and constructs a typecode
 * with each valid definition.  @ns and @nsuri are used to set the
 * namespace information for the typecode.
 *
 * The glib schema has 3 types of element: <element name="" type="">,
 * <struct name=""><element name="" type=""></struct> and <list
 * name="" type="">,
 */
void
wsdl_schema_glib_start_element (const xmlDocPtr    doc, 
				const xmlNodePtr   node,
				const xmlChar    **attrs, 
				const guchar      *ns,
				const guchar      *nsuri,
				WsdlErrorMsgFn     error_msg)
{
	if (error_msg == NULL) {
		error_msg = g_print;
	}

	switch (state) {
	case GLIB_TOPLEVEL:
		if (wsdl_qnamecmp (node, GLIBNS, "element") == TRUE) {
			wsdl_typecode *newtc;
			const wsdl_typecode *subtc;
			guchar *tcname = NULL, *type = NULL;
			const guchar *tnsuri;
			gboolean attrsok;

			attrsok =
				wsdl_schema_glib_parse_element_attrs (attrs,
								      &tcname,
								      &type);

			if (attrsok == FALSE) {
				return;
			}

			if (wsdl_typecode_lookup (tcname, nsuri) != NULL) {
				/* Already defined */

				if (nsuri == NULL) {
					error_msg ("%s is already defined",
						   tcname);
				} else {
					error_msg ("%s is already defined in "
						   "the %s namespace",
						   tcname, 
						   nsuri);
				}

				g_free (tcname);
				g_free (type);
				return;
			}

			tnsuri =
				wsdl_prefix_to_namespace (doc, node, type,
							  FALSE);

			if ((subtc = wsdl_typecode_lookup (type, tnsuri)) ==
			    NULL) {
				/* Subtype not defined */

				if (tnsuri == NULL) {
					error_msg ("%s is not known", type);
				} else {
					error_msg ("%s is not known in "
						   "the %s namespace",
						   type, 
						   tnsuri);
				}


				g_free (tcname);
				g_free (type);
				return;
			}

			g_free (type);

			newtc = g_new0 (wsdl_typecode, 1);
			newtc->kind = WSDL_TK_GLIB_ELEMENT;
			newtc->name = tcname;
			newtc->ns = g_strdup (ns);
			newtc->nsuri = g_strdup (nsuri);
			newtc->dynamic = TRUE;
			newtc->subtypes = g_new0 (const wsdl_typecode *, 1);
			newtc->sub_parts = 1;
			newtc->subtypes[0] = subtc;

			wsdl_typecode_register (newtc);

			state = GLIB_ELEMENT;
		} else if (wsdl_qnamecmp (node, GLIBNS, "struct") == TRUE) {
			guchar *tcname = NULL;
			gboolean attrsok;

			attrsok =
				wsdl_schema_glib_parse_struct_attrs (attrs,
								     &tcname);

			if (attrsok == FALSE) {
				return;
			}

			if (wsdl_typecode_lookup (tcname, nsuri) != NULL) {
				/* Already defined */

				if (nsuri == NULL) {
					error_msg ("%s is already defined",
						   tcname);
				} else {
					error_msg ("%s is already defined in "
						   "the %s namespace",
						   tcname, 
						   nsuri);
				}

				g_free (tcname);
				return;
			}

			tmptc = g_new0 (wsdl_typecode, 1);
			tmptc->kind = WSDL_TK_GLIB_STRUCT;
			tmptc->name = tcname;
			tmptc->ns = g_strdup (ns);
			tmptc->nsuri = g_strdup (nsuri);
			tmptc->dynamic = TRUE;

			wsdl_typecode_register (tmptc);

			state = GLIB_STRUCT;
		} else if (wsdl_qnamecmp (node, GLIBNS, "list") == TRUE) {
			wsdl_typecode *newtc;
			const wsdl_typecode *subtc;
			guchar *tcname = NULL, *type = NULL;
			const guchar *tnsuri;
			gboolean attrsok;

			attrsok = wsdl_schema_glib_parse_element_attrs (attrs,
									&tcname,
									&type);

			if (attrsok == FALSE) {
				return;
			}

			if (wsdl_typecode_lookup (tcname, nsuri) != NULL) {
				/* Already defined */

				if (nsuri == NULL) {
					error_msg ("%s is already defined",
						   tcname);
				} else {
					error_msg ("%s is already defined in "
						   "the %s namespace",
						   tcname, 
						   nsuri);
				}

				g_free (tcname);
				g_free (type);
				return;
			}

			tnsuri = wsdl_prefix_to_namespace (doc, 
							   node, 
							   type,
							   FALSE);

			subtc = wsdl_typecode_lookup (type, tnsuri);
			if (subtc == NULL) {
				/* Subtype not defined */

				if (tnsuri == NULL) {
					error_msg ("%s is not known", type);
				} else {
					error_msg ("%s is not known in "
						   "the %s namespace",
						   type, 
						   tnsuri);
				}

				g_free (tcname);
				g_free (type);
				return;
			}

			g_free (type);

			newtc = g_new0 (wsdl_typecode, 1);
			newtc->kind = WSDL_TK_GLIB_LIST;
			newtc->name = tcname;
			newtc->ns = g_strdup (ns);
			newtc->nsuri = g_strdup (nsuri);
			newtc->dynamic = TRUE;
			newtc->subtypes = g_new0 (const wsdl_typecode *, 1);
			newtc->sub_parts = 1;
			newtc->subtypes[0] = subtc;

			wsdl_typecode_register (newtc);

			state = GLIB_LIST;
		} else {
			last_known_state = state;
			state = GLIB_UNKNOWN;
			g_assert (unknown_depth == 0);
			unknown_depth++;
		}
		break;

	case GLIB_ELEMENT:
		/* may not contain elements */
		break;

	case GLIB_LIST:
		/* may not contain elements */
		break;

	case GLIB_STRUCT:
		wsdl_schema_glib_parse_struct (doc, node, attrs, error_msg);
		break;

	case GLIB_STRUCT_ELEMENT:
		/* may not contain elements */
		break;

	case GLIB_UNKNOWN:
		unknown_depth++;
		break;
	case GLIB_STATE_MAX:
		g_assert_not_reached ();
		break;
	}
}

/**
 * wsdl_schema_glib_end_element:
 * @node: a pointer to a node within an XML document tree.
 * @error_msg: a pointer to a function to be called with any error
 * messages to be displayed
 *
 * Implements the parsing of the simple Ximian glib schema, by being
 * called by wsdl_parse() via wsdl_schema_end_element() with each
 * @node in turn of a schema definition, to close the current typecode
 * that is being constructed.
 */
void
wsdl_schema_glib_end_element (const xmlNodePtr node G_GNUC_UNUSED,
			      WsdlErrorMsgFn   error_msg G_GNUC_UNUSED)
{
	switch (state) {
	case GLIB_TOPLEVEL:
		break;
	case GLIB_ELEMENT:
		state = GLIB_TOPLEVEL;
		break;
	case GLIB_STRUCT:
		tmptc = NULL;
		state = GLIB_TOPLEVEL;
		break;
	case GLIB_STRUCT_ELEMENT:
		state = GLIB_STRUCT;
		break;
	case GLIB_LIST:
		state = GLIB_TOPLEVEL;
		break;
	case GLIB_UNKNOWN:
		unknown_depth--;
		if (unknown_depth == 0) {
			state = last_known_state;
		}
		break;
	case GLIB_STATE_MAX:
		g_assert_not_reached ();
		break;
	}
}
