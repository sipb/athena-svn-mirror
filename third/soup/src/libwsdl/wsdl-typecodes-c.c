/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-typecodes-c.c: Write C code from typecodes
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <stdio.h>
#include <stdarg.h>

#include "wsdl-typecodes.h"
#include "wsdl-typecodes-c.h"

static guint arraycount = 0;

static void
wsdl_typecode_write_c_definition_element (const wsdl_typecode * const tc,
					  FILE                       *out)
{
	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out,
		 "\n\nstatic const wsdl_typecode *wsdl_subtypes_array_%d[]={\n",
		 arraycount);
	fprintf (out, 
		 "\t&WSDL_TC_%s_%s_struct,\n", 
		 tc->subtypes[0]->ns,
		 tc->subtypes[0]->name);
	fprintf (out, "};\n");

	fprintf (out,
		 "const wsdl_typecode WSDL_TC_%s_%s_struct={\n"
		 "\tWSDL_TK_GLIB_ELEMENT, \"%s\", \"%s\", \"%s\", FALSE, 1,\n"
		 "\tNULL, wsdl_subtypes_array_%d, ",
		 tc->ns, 
		 tc->name, 
		 tc->name, 
		 tc->ns, 
		 tc->nsuri, 
		 arraycount);

	if (wsdl_typecode_is_simple (tc->subtypes[0]) == FALSE) {
		fprintf (out, "%s_%s_free", tc->ns, tc->name);
	} else if (wsdl_typecode_element_kind (tc->subtypes[0]) ==
		   WSDL_TK_GLIB_STRING) {
		fprintf (out, "g_free");
	} else {
		fprintf (out, "NULL");
	}

	fprintf (out, "\n};\n");

	arraycount++;
}

static void
wsdl_typecode_write_c_definition_struct (const wsdl_typecode * const tc,
					 FILE                       *out)
{
	guint i;
	guint nelems;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	nelems = wsdl_typecode_member_count (tc);

	fprintf (out, 
		 "\n\nstatic const guchar *wsdl_subnames_array_%d[]={\n",
		 arraycount);

	for (i = 0; i < nelems; i++) {
		fprintf (out, "\t\"%s\",\n", wsdl_typecode_member_name (tc, i));
	}
	fprintf (out, "};\n");

	fprintf (out,
		 "static const wsdl_typecode *wsdl_subtypes_array_%d[]={\n",
		 arraycount + 1);

	for (i = 0; i < nelems; i++) {
		const wsdl_typecode *const subtype =
			wsdl_typecode_member_type (tc, i);

		fprintf (out, 
			 "\t&WSDL_TC_%s_%s_struct,\n", 
			 subtype->ns,
			 subtype->name);
	}
	fprintf (out, "};\n");

	fprintf (out,
		 "const wsdl_typecode WSDL_TC_%s_%s_struct = {\n"
		 "\tWSDL_TK_GLIB_STRUCT, \"%s\", \"%s\", \"%s\", FALSE, %d,\n"
		 "\twsdl_subnames_array_%d, wsdl_subtypes_array_%d, "
		 "%s_%s_free\n"
		 "};\n",
		 tc->ns, 
		 tc->name, 
		 tc->name, 
		 tc->ns, 
		 tc->nsuri, 
		 nelems,
		 arraycount, 
		 arraycount + 1, 
		 tc->ns, 
		 tc->name);

	arraycount += 2;
}

static void
wsdl_typecode_write_c_definition_list (const wsdl_typecode * const tc,
				       FILE                       *out)
{
	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out,
		 "\n\nstatic const wsdl_typecode *wsdl_subtypes_array_%d[]={\n",
		 arraycount);
	fprintf (out, 
		 "\t&WSDL_TC_%s_%s_struct,\n", 
		 tc->subtypes[0]->ns,
		 tc->subtypes[0]->name);
	fprintf (out, "};\n");

	fprintf (out,
		 "const wsdl_typecode WSDL_TC_%s_%s_struct = {\n"
		   "\tWSDL_TK_GLIB_LIST, \"%s\", \"%s\", \"%s\", FALSE, 1, \n"
		   "\tNULL, wsdl_subtypes_array_%d, %s_%s_free\n"
		 "};\n",
		 tc->ns, 
		 tc->name, 
		 tc->name, 
		 tc->ns, 
		 tc->nsuri, 
		 arraycount,
		 tc->ns, 
		 tc->name);

	arraycount++;
}

/**
 * wsdl_typecode_write_c_definition:
 * @out: a stdio FILE pointer
 * @tc: a pointer to a typecode
 *
 * Writes to @out a C definition of a typecode struct.
 */
void
wsdl_typecode_write_c_definition (FILE *out, const wsdl_typecode * const tc)
{
	wsdl_typecode_kind_t kind;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	kind = wsdl_typecode_kind (tc);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		wsdl_typecode_write_c_definition_element (tc, out);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		wsdl_typecode_write_c_definition_struct (tc, out);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		wsdl_typecode_write_c_definition_list (tc, out);
	} else {
		/* Simple types are pre-defined */
	}
}

static void
wsdl_typecode_write_c_mm_element (const wsdl_typecode * const tc, FILE *out)
{
	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	if (wsdl_typecode_is_simple (tc->subtypes[0]) == FALSE) {
		/* element, struct or list type */
		fprintf (out, 
			 "\n\nvoid %s_%s_free (gpointer data)\n", 
			 tc->ns,
			 tc->name);
		fprintf (out, "{\n");
		fprintf (out, 
			 "\t%s_%s_free (data);\n", 
			 tc->subtypes[0]->ns,
			 tc->subtypes[0]->name);

		fprintf (out, "}\n\n");
	}
}

static void
wsdl_typecode_write_c_mm_struct (const wsdl_typecode * const tc, FILE *out)
{
	guint i;
	guint nelems;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	nelems = wsdl_typecode_member_count (tc);

	fprintf (out, 
		 "\n\nvoid %s_%s_free (gpointer data)\n", 
		 tc->ns, 
		 tc->name);
	fprintf (out, "{\n");
	fprintf (out, 
		 "\t%s item = (%s) data;\n", 
		 wsdl_typecode_param_type (tc),
		 wsdl_typecode_param_type (tc));

	for (i = 0; i < nelems; i++) {
		const wsdl_typecode *const subtype =
			wsdl_typecode_member_type (tc, i);

		if (wsdl_typecode_is_simple (subtype) == FALSE) {
			fprintf (out, 
				 "\tif (item->%s != NULL) {\n",
				 tc->subnames[i]);
			fprintf (out, 
				 "\t\t%s_%s_free (item->%s);\n",
				 subtype->ns, 
				 subtype->name, 
				 tc->subnames[i]);
			fprintf (out, "\t}\n");
		} else if (wsdl_typecode_element_kind (subtype) ==
			   WSDL_TK_GLIB_STRING) {
			fprintf (out, 
				 "\tif (item->%s != NULL) {\n",
				 tc->subnames[i]);
			fprintf (out, 
				 "\t\tg_free (item->%s);\n",
				 tc->subnames[i]);
			fprintf (out, "\t}\n");
		}
	}
	fprintf (out, "\tg_free (item);\n");
	fprintf (out, "}\n\n");
}

static void
wsdl_typecode_write_c_mm_list (const wsdl_typecode * const tc, FILE * out)
{
	guchar *type;

	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out, 
		 "\n\nvoid %s_%s_free (gpointer data)\n", 
		 tc->ns, 
		 tc->name);
	fprintf (out, "{\n");
	fprintf (out, "\tGSList *item = (GSList *) data;\n");
	fprintf (out, "\tGSList *iter = item;\n");
	fprintf (out, "\twhile (iter != NULL) {\n");

	type = wsdl_typecode_param_type (tc->subtypes[0]);

	if (wsdl_typecode_is_simple (tc->subtypes[0]) == FALSE) {
		fprintf (out, 
			 "\t\t%s subitem = (%s) iter->data;\n", 
			 type, 
			 type);
		fprintf (out, "\t\t%s_%s_free (subitem);\n", 
			 tc->subtypes[0]->ns,
			 tc->subtypes[0]->name);
	} else if (wsdl_typecode_element_kind (tc->subtypes[0]) ==
		   WSDL_TK_GLIB_STRING) {
		fprintf (out, 
			 "\t\t%s subitem = (%s) iter->data;\n", 
			 type, 
			 type);
		fprintf (out, "\t\tg_free (subitem);\n");
	} else {
		fprintf (out, 
			 "\t\t%s *subitem = (%s *) iter->data;\n", 
			 type,
			 type);
		fprintf (out, "\t\tg_free (subitem);\n");
	}
	g_free (type);

	fprintf (out, "\t\titer = iter->next;\n");
	fprintf (out, "\t}\n");
	fprintf (out, "\tg_slist_free (item);\n");
	fprintf (out, "}\n\n");
}


/**
 * wsdl_typecode_write_c_mm:
 * @out: a stdio FILE pointer
 * @tc: a pointer to a typecode
 *
 * Writes to @out a C function to free memory containing a C
 * representation of @tc.
 */
void
wsdl_typecode_write_c_mm (FILE * out, const wsdl_typecode * const tc)
{
	wsdl_typecode_kind_t kind;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	kind = wsdl_typecode_kind (tc);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		wsdl_typecode_write_c_mm_element (tc, out);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		wsdl_typecode_write_c_mm_struct (tc, out);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		wsdl_typecode_write_c_mm_list (tc, out);
	} else {
		/* Simple types are pre-defined */
	}
}

static void
wsdl_typecode_write_c_declaration_element (const wsdl_typecode * const tc,
					   FILE                       *out)
{
	guchar *type;

	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out, "\n#ifndef _WSDL_%s_%s_defined\n", tc->ns, tc->name);
	fprintf (out, "#define _WSDL_%s_%s_defined\n", tc->ns, tc->name);

	type = wsdl_typecode_type (tc->subtypes[0]);
	fprintf (out, "typedef %s %s_%s;\n\n", type, tc->ns, tc->name);
	g_free (type);

	fprintf (out, 
		 "extern const wsdl_typecode WSDL_TC_%s_%s_struct;\n",
		 tc->ns, 
		 tc->name);

	fprintf (out, "#endif /* _WSDL_%s_%s_defined */\n", tc->ns, tc->name);
}

static void
wsdl_typecode_write_c_declaration_struct (const wsdl_typecode * const tc,
					  FILE                       *out)
{
	guint i;
	guint nelems;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	nelems = wsdl_typecode_member_count (tc);

	fprintf (out, "\n#ifndef _WSDL_%s_%s_defined\n", tc->ns, tc->name);
	fprintf (out, "#define _WSDL_%s_%s_defined\n", tc->ns, tc->name);

	fprintf (out, 
		 "typedef struct _%s_%s %s_%s;\n\n", 
		 tc->ns, 
		 tc->name,
		 tc->ns, 
		 tc->name);
	fprintf (out, "struct _%s_%s {\n", tc->ns, tc->name);

	for (i = 0; i < nelems; i++) {
		const wsdl_typecode *const subtype =
			wsdl_typecode_member_type (tc, i);
		guchar *typename;

		typename = wsdl_typecode_param_type (subtype);

		fprintf (out, 
			 "\t%s %s;\n", 
			 typename,
			 wsdl_typecode_member_name (tc, i));
		g_free (typename);
	}

	fprintf (out, "};\n\n");

	fprintf (out, 
		 "extern const wsdl_typecode WSDL_TC_%s_%s_struct;\n",
		 tc->ns, 
		 tc->name);

	fprintf (out, "#endif /* _WSDL_%s_%s_defined */\n", tc->ns, tc->name);
}

static void
wsdl_typecode_write_c_declaration_list (const wsdl_typecode * const tc,
					FILE                       *out)
{
	guchar *type, *subtype;

	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out, "\n#ifndef _WSDL_%s_%s_defined\n", tc->ns, tc->name);
	fprintf (out, "#define _WSDL_%s_%s_defined\n", tc->ns, tc->name);

	type = wsdl_typecode_type (tc);
	subtype = wsdl_typecode_type (tc->subtypes[0]);

	fprintf (out, 
		 "typedef %s %s_%s;\t/* a list of %s */\n\n", 
		 type, 
		 tc->ns,
		 tc->name, 
		 subtype);
	g_free (type);
	g_free (subtype);

	fprintf (out, 
		 "extern const wsdl_typecode WSDL_TC_%s_%s_struct;\n",
		 tc->ns, 
		 tc->name);

	fprintf (out, "#endif /* _WSDL_%s_%s_defined */\n", tc->ns, tc->name);
}

/**
 * wsdl_typecode_write_c_declaration:
 * @out: a stdio FILE pointer
 * @tc: a pointer to a typecode
 *
 * Writes to @out a C prototype for a definition of a typecode struct.
 */
void
wsdl_typecode_write_c_declaration (FILE * out, const wsdl_typecode * const tc)
{
	wsdl_typecode_kind_t kind;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	kind = wsdl_typecode_kind (tc);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		wsdl_typecode_write_c_declaration_element (tc, out);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		wsdl_typecode_write_c_declaration_struct (tc, out);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		wsdl_typecode_write_c_declaration_list (tc, out);
	} else {
		/* Simple types are pre-defined */
	}
}

static void
wsdl_typecode_write_c_mm_decl_element (const wsdl_typecode * const tc,
				       FILE                       *out)
{
	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	if (wsdl_typecode_is_simple (tc->subtypes[0]) == FALSE) {
		/* element, struct or list type */
		fprintf (out,
			 "\n\nextern void %s_%s_free (gpointer);\n", 
			 tc->ns,
			 tc->name);
	}
}

static void
wsdl_typecode_write_c_mm_decl_struct (const wsdl_typecode * const tc,
				      FILE                       *out)
{
	g_assert (tc != NULL);
	g_assert (out != NULL);

	fprintf (out, 
		 "\n\nextern void %s_%s_free (gpointer);\n", 
		 tc->ns,
		 tc->name);
}

static void
wsdl_typecode_write_c_mm_decl_list (const wsdl_typecode * const tc, FILE * out)
{
	g_assert (tc != NULL);
	g_assert (tc->subtypes[0] != NULL);
	g_assert (out != NULL);

	fprintf (out, "\n\nextern void %s_%s_free (gpointer);\n", 
		 tc->ns,
		 tc->name);
}

/**
 * wsdl_typecode_write_c_mm_decl:
 * @out: a stdio FILE pointer
 * @tc: a pointer to a typecode
 *
 * Writes to @out a C function prototype for a function to free memory
 * containing a C representation of @tc.
 */
void
wsdl_typecode_write_c_mm_decl (FILE * out, const wsdl_typecode * const tc)
{
	wsdl_typecode_kind_t kind;

	g_assert (tc != NULL);
	g_assert (out != NULL);

	kind = wsdl_typecode_kind (tc);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		wsdl_typecode_write_c_mm_decl_element (tc, out);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		wsdl_typecode_write_c_mm_decl_struct (tc, out);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		wsdl_typecode_write_c_mm_decl_list (tc, out);
	} else {
		/* Simple types are pre-defined */
	}
}
