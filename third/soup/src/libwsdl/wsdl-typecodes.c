/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-typecodes.c: Type code definitions for WSDL types
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <string.h>

#include "wsdl-typecodes.h"
#include "wsdl-schema.h"

/*
 * Default alignment for windows
 */
#ifdef SOUP_WIN32
#define ALIGNOF_GBOOLEAN 4
#define ALIGNOF_GCHAR 1
#define ALIGNOF_GUCHAR 1
#define ALIGNOF_GINT 4
#define ALIGNOF_GUINT 4
#define ALIGNOF_GSHORT 2
#define ALIGNOF_GUSHORT 2
#define ALIGNOF_GLONG 4
#define ALIGNOF_GULONG 4
#define ALIGNOF_GINT8 1
#define ALIGNOF_GUINT8 1
#define ALIGNOF_GINT16 2
#define ALIGNOF_GUINT16 2
#define ALIGNOF_GINT32 4
#define ALIGNOF_GUINT32 4
#define ALIGNOF_GFLOAT 4
#define ALIGNOF_GDOUBLE 4
#define ALIGNOF_GPOINTER 4
#define ALIGNOF_GSTRUCT 1
#endif

/**
 * ALIGN_ADDRESS:
 * @this: a pointer to anything
 * @boundary: an integer containing the requested alignment
 *
 * Finds a pointer to the next memory location after @this that can
 * accept an alignment of @boundary.
 *
 */

/**
 * WSDL_TC_glib_null_struct:
 *
 * The typecode for the #NULL glib type. (Not used anywhere, sort of
 * an error condition)
 */
const wsdl_typecode WSDL_TC_glib_null_struct = { 
	WSDL_TK_GLIB_NULL, "null", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_void_struct:
 *
 * The typecode for the #void glib type. (Not currently used anywhere)
 */
const wsdl_typecode WSDL_TC_glib_void_struct = { 
	WSDL_TK_GLIB_VOID, "void", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_boolean_struct:
 *
 * The typecode for the #gboolean glib type.
 */
const wsdl_typecode WSDL_TC_glib_boolean_struct = { 
	WSDL_TK_GLIB_BOOLEAN, "boolean", "glib", GLIBNS, FALSE, 0,NULL,NULL,NULL
};

/**
 * WSDL_TC_glib_char_struct:
 *
 * The typecode for the #gchar glib type.
 */
const wsdl_typecode WSDL_TC_glib_char_struct = { 
	WSDL_TK_GLIB_CHAR, "char", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_uchar_struct:
 *
 * The typecode for the #guchar glib type.
 */
const wsdl_typecode WSDL_TC_glib_uchar_struct = { 
	WSDL_TK_GLIB_UCHAR, "uchar", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_int_struct:
 *
 * The typecode for the #gint glib type.
 */
const wsdl_typecode WSDL_TC_glib_int_struct = { 
	WSDL_TK_GLIB_INT, "int", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_uint_struct:
 *
 * The typecode for the #guint glib type.
 */
const wsdl_typecode WSDL_TC_glib_uint_struct = { 
	WSDL_TK_GLIB_UINT, "uint", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_short_struct:
 *
 * The typecode for the #gshort glib type.
 */
const wsdl_typecode WSDL_TC_glib_short_struct = { 
	WSDL_TK_GLIB_SHORT, "short", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_ushort_struct:
 *
 * The typecode for the #gushort glib type.
 */
const wsdl_typecode WSDL_TC_glib_ushort_struct = { 
	WSDL_TK_GLIB_USHORT, "ushort", "glib", GLIBNS, FALSE, 0, NULL, NULL,NULL
};

/**
 * WSDL_TC_glib_long_struct:
 *
 * The typecode for the #glong glib type.
 */
const wsdl_typecode WSDL_TC_glib_long_struct = { 
	WSDL_TK_GLIB_LONG, "long", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_ulong_struct:
 *
 * The typecode for the #gulong glib type.
 */
const wsdl_typecode WSDL_TC_glib_ulong_struct = { 
	WSDL_TK_GLIB_ULONG, "ulong", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_int8_struct:
 *
 * The typecode for the #gint8 glib type.
 */
const wsdl_typecode WSDL_TC_glib_int8_struct = { 
	WSDL_TK_GLIB_INT8, "int8", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_uint8_struct:
 *
 * The typecode for the #guint8 glib type.
 */
const wsdl_typecode WSDL_TC_glib_uint8_struct = { 
	WSDL_TK_GLIB_UINT8, "uint8", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_int16_struct:
 *
 * The typecode for the #gint16 glib type.
 */
const wsdl_typecode WSDL_TC_glib_int16_struct = { 
	WSDL_TK_GLIB_INT16, "int16", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_uint16_struct:
 *
 * The typecode for the #guint16 glib type.
 */
const wsdl_typecode WSDL_TC_glib_uint16_struct = { 
	WSDL_TK_GLIB_UINT16, "uint16", "glib", GLIBNS, FALSE, 0, NULL, NULL,NULL
};

/**
 * WSDL_TC_glib_int32_struct:
 *
 * The typecode for the #gint32 glib type.
 */
const wsdl_typecode WSDL_TC_glib_int32_struct = { 
	WSDL_TK_GLIB_INT32, "int32", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_uint32_struct:
 *
 * The typecode for the #guint32 glib type.
 */
const wsdl_typecode WSDL_TC_glib_uint32_struct = { 
	WSDL_TK_GLIB_UINT32, "uint32", "glib", GLIBNS, FALSE, 0, NULL, NULL,NULL
};

/**
 * WSDL_TC_glib_float_struct:
 *
 * The typecode for the #gfloat glib type.
 */
const wsdl_typecode WSDL_TC_glib_float_struct = { 
	WSDL_TK_GLIB_FLOAT, "float", "glib", GLIBNS, FALSE, 0, NULL, NULL, NULL
};

/**
 * WSDL_TC_glib_double_struct:
 *
 * The typecode for the #gdouble glib type.
 */
const wsdl_typecode WSDL_TC_glib_double_struct = { 
	WSDL_TK_GLIB_DOUBLE, "double", "glib", GLIBNS, FALSE, 0, NULL, NULL,NULL
};

/**
 * WSDL_TC_glib_string_struct:
 *
 * The typecode for the #guchar * glib type.
 */
const wsdl_typecode WSDL_TC_glib_string_struct = { 
	WSDL_TK_GLIB_STRING, "string", "glib", GLIBNS, FALSE, 0, NULL, NULL,NULL
};


/* Keep this synchronised with wsdl_typecode_kind_t in
 * wsdl-typecodes.h
 */
static const guchar *wsdl_typecode_kind_names[] = {
	"WSDL_TK_GLIB_NULL",
	"WSDL_TK_GLIB_VOID",
	"WSDL_TK_GLIB_BOOLEAN",
	"WSDL_TK_GLIB_CHAR",
	"WSDL_TK_GLIB_UCHAR",
	"WSDL_TK_GLIB_INT",
	"WSDL_TK_GLIB_UINT",
	"WSDL_TK_GLIB_SHORT",
	"WSDL_TK_GLIB_USHORT",
	"WSDL_TK_GLIB_LONG",
	"WSDL_TK_GLIB_ULONG",
	"WSDL_TK_GLIB_INT8",
	"WSDL_TK_GLIB_UINT8",
	"WSDL_TK_GLIB_INT16",
	"WSDL_TK_GLIB_UINT16",
	"WSDL_TK_GLIB_INT32",
	"WSDL_TK_GLIB_UINT32",
	"WSDL_TK_GLIB_FLOAT",
	"WSDL_TK_GLIB_DOUBLE",
	"WSDL_TK_GLIB_STRING",
	"WSDL_TK_GLIB_ELEMENT",
	"WSDL_TK_GLIB_STRUCT",
	"WSDL_TK_GLIB_LIST",
	"Not a valid WSDL_TK_GLIB type"
};

static GSList *wsdl_typecodes = NULL;

static const guchar *
wsdl_typecode_kind_name (wsdl_typecode_kind_t kind)
{
	g_assert (kind < WSDL_TK_GLIB_MAX);

	return (wsdl_typecode_kind_names[kind]);
}

/**
 * wsdl_typecode_kind:
 * @tc: a typecode pointer
 *
 * Finds the typecode kind of @tc.
 *
 * Returns: the typecode kind of @tc
 */
wsdl_typecode_kind_t
wsdl_typecode_kind (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);
	g_assert (tc->kind < WSDL_TK_GLIB_MAX);

	return (tc->kind);
}

/**
 * wsdl_typecode_element_kind:
 * @tc: a typecode pointer
 *
 * Finds the typecode kind of @tc, recursing through
 * #WSDL_TK_GLIB_ELEMENT typecodes.
 *
 * Returns: the typecode kind of @tc, recursing through
 * #WSDL_TK_GLIB_ELEMENT typecodes.
 */
wsdl_typecode_kind_t
wsdl_typecode_element_kind (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);
	g_assert (tc->kind < WSDL_TK_GLIB_MAX);

	if (tc->kind == WSDL_TK_GLIB_ELEMENT) {
		return (wsdl_typecode_element_kind (tc->subtypes[0]));
	} else {
		return (tc->kind);
	}
}

/**
 * wsdl_typecode_is_simple:
 * @tc: a typecode pointer
 *
 * Checks the kind of @tc, recursing through #WSDL_TK_GLIB_ELEMENT
 * typecodes.
 *
 * Returns: #TRUE if the typecode is for a predefined type, or #FALSE
 * otherwise.
 */
gboolean
wsdl_typecode_is_simple (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);
	g_assert (tc->kind < WSDL_TK_GLIB_MAX);

	if (tc->kind < WSDL_TK_GLIB_ELEMENT) {
		return (TRUE);
	} else if (tc->kind == WSDL_TK_GLIB_ELEMENT) {
		return (wsdl_typecode_is_simple (tc->subtypes[0]));
	} else {
		return (FALSE);
	}
}

/**
 * wsdl_typecode_member_count:
 * @tc: a typecode pointer
 *
 * Counts the members in a #WSDL_TK_GLIB_STRUCT typecode.
 *
 * Returns: the number of members, or 0 if @tc isn't a
 * #WSDL_TK_GLIB_STRUCT kind.
 */
guint
wsdl_typecode_member_count (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);

	if (tc->kind != WSDL_TK_GLIB_STRUCT) {
		return (0);
	}

	return (tc->sub_parts);
}

/**
 * wsdl_typecode_member_name:
 * @tc: a typecode pointer
 * @member: an integer index, starting from 0
 *
 * Looks up the name of the member of @tc at index position @member.
 *
 * Returns: the name of the member of @tc, or #NULL if @tc isn't a
 * #WSDL_TK_GLIB_STRUCT kind.
 */
const guchar *
wsdl_typecode_member_name (const wsdl_typecode * const tc, guint member)
{
	g_assert (tc != NULL);

	if (tc->kind != WSDL_TK_GLIB_STRUCT || member >= tc->sub_parts) {
		return (NULL);
	}

	return (tc->subnames[member]);
}

/**
 * wsdl_typecode_member_type:
 * @tc: a typecode pointer
 * @member: an integer index, starting from 0
 *
 * Looks up the type of the member of @tc at index position @member.
 *
 * Returns: the name of the member of @tc, or #NULL if @tc isn't a
 * #WSDL_TK_GLIB_STRUCT kind.
 */
const wsdl_typecode *
wsdl_typecode_member_type (const wsdl_typecode * const tc, guint member)
{
	g_assert (tc != NULL);

	if (tc->kind != WSDL_TK_GLIB_STRUCT || member >= tc->sub_parts) {
		return (NULL);
	}

	return (tc->subtypes[member]);
}

/**
 * wsdl_typecode_content_type:
 * @tc: a typecode pointer
 *
 * Looks up the typecode that is either the aliased type of a
 * #WSDL_TK_GLIB_ELEMENT kind or the content of a #WSDL_TK_GLIB_LIST
 * kind.
 *
 * Returns: the typecode, or NULL if @tc isn't a #WSDL_TK_GLIB_ELEMENT
 * or #WSDL_TK_GLIB_LIST kind.
 */
const wsdl_typecode *
wsdl_typecode_content_type (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);

	if (tc->kind != WSDL_TK_GLIB_LIST || tc->kind != WSDL_TK_GLIB_ELEMENT) {
		return (NULL);
	}

	return (tc->subtypes[0]);
}

/**
 * wsdl_typecode_name:
 * @tc: a typecode pointer
 *
 * Finds the name of the typecode @tc.
 *
 * Returns: the name of the typecode @tc.
 */
const guchar *
wsdl_typecode_name (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);

	return (tc->name);
}

/**
 * wsdl_typecode_ns:
 * @tc: a typecode pointer
 *
 * Finds the namespace prefix of the typecode @tc.
 *
 * Returns: the namespace prefix of the typecode @tc.
 */
const guchar *
wsdl_typecode_ns (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);

	return (tc->ns);
}

/**
 * wsdl_typecode_nsuri:
 * @tc: a typecode pointer
 *
 * Finds the namespace URI of the typecode @tc.
 *
 * Returns: the namespace URI of the typecode @tc.
 */
const guchar *
wsdl_typecode_nsuri (const wsdl_typecode * const tc)
{
	g_assert (tc != NULL);

	return (tc->nsuri);
}

/**
 * wsdl_typecode_type:
 * @tc: a typecode pointer
 *
 * Constructs a string containing the C mapping of the type
 * represented by typecode @tc.
 *
 * Returns: a string containing the C mapping of the type represented
 * by typecode @tc.  This string should be freed by the caller.
 */
guchar *
wsdl_typecode_type (const wsdl_typecode * const tc)
{
	guchar *str = NULL;

	g_assert (tc != NULL);

	switch (tc->kind) {
	case WSDL_TK_GLIB_NULL:
		break;
	case WSDL_TK_GLIB_VOID:
		break;
	case WSDL_TK_GLIB_BOOLEAN:
		str = g_strdup ("gboolean");
		break;
	case WSDL_TK_GLIB_CHAR:
		str = g_strdup ("gchar");
		break;
	case WSDL_TK_GLIB_UCHAR:
		str = g_strdup ("guchar");
		break;
	case WSDL_TK_GLIB_INT:
		str = g_strdup ("gint");
		break;
	case WSDL_TK_GLIB_UINT:
		str = g_strdup ("guint");
		break;
	case WSDL_TK_GLIB_SHORT:
		str = g_strdup ("gshort");
		break;
	case WSDL_TK_GLIB_USHORT:
		str = g_strdup ("gushort");
		break;
	case WSDL_TK_GLIB_LONG:
		str = g_strdup ("glong");
		break;
	case WSDL_TK_GLIB_ULONG:
		str = g_strdup ("gulong");
		break;
	case WSDL_TK_GLIB_INT8:
		str = g_strdup ("gint8");
		break;
	case WSDL_TK_GLIB_UINT8:
		str = g_strdup ("guint8");
		break;
	case WSDL_TK_GLIB_INT16:
		str = g_strdup ("gint16");
		break;
	case WSDL_TK_GLIB_UINT16:
		str = g_strdup ("guint16");
		break;
	case WSDL_TK_GLIB_INT32:
		str = g_strdup ("gint32");
		break;
	case WSDL_TK_GLIB_UINT32:
		str = g_strdup ("guint32");
		break;
	case WSDL_TK_GLIB_FLOAT:
		str = g_strdup ("gfloat");
		break;
	case WSDL_TK_GLIB_DOUBLE:
		str = g_strdup ("gdouble");
		break;
	case WSDL_TK_GLIB_STRING:
		str = g_strdup ("guchar *");
		break;
	case WSDL_TK_GLIB_ELEMENT:
		str = g_strdup_printf ("%s_%s", tc->ns, tc->name);
		break;
	case WSDL_TK_GLIB_STRUCT:
		str = g_strdup_printf ("%s_%s", tc->ns, tc->name);
		break;
	case WSDL_TK_GLIB_LIST:
		str = g_strdup ("GSList *");
		break;
	case WSDL_TK_GLIB_MAX:
		break;
	}

	if (str == NULL) {
		str = g_strdup ("NULL");
	}

	return (str);
}

/**
 * wsdl_typecode_param_type:
 * @tc: a typecode pointer
 *
 * Constructs a string containing the C mapping of the type
 * represented by typecode @tc, in a form suitable for function
 * parameter types.  The difference with wsdl_typecode_type() is that
 * '*' is appended to C types that represent #WSDL_TK_GLIB_STRUCT
 * typecodes.
 *
 * Returns: a string containing the C mapping of the type represented
 * by typecode @tc, in a form suitable for function parameter types.
 * The difference with wsdl_typecode_type() is that '*' is appended to
 * C types that represent #WSDL_TK_GLIB_STRUCT typecodes.  This string
 * should be freed by the caller.
 */
guchar *
wsdl_typecode_param_type (const wsdl_typecode * const tc)
{
	guchar *str = NULL;

	g_assert (tc != NULL);

	switch (tc->kind) {
	case WSDL_TK_GLIB_ELEMENT:
		{
			const wsdl_typecode *subtc = tc->subtypes[0];

			/* 
			 * This one gets tricky. If the aliased type is a struct
			 * we need to add the '*' 
			 */
			while (subtc != NULL) {
				if (subtc->kind == WSDL_TK_GLIB_ELEMENT) {
					subtc = subtc->subtypes[0];
				} else if (subtc->kind == WSDL_TK_GLIB_STRUCT) {
					str = g_strdup_printf ("%s_%s *",
							       tc->ns,
							       tc->name);
					break;
				} else {
					str = g_strdup_printf ("%s_%s",
							       tc->ns,
							       tc->name);
					break;
				}
			}

			break;
		}

	case WSDL_TK_GLIB_STRUCT:
		str = g_strdup_printf ("%s_%s *", tc->ns, tc->name);
		break;
	case WSDL_TK_GLIB_NULL:
	case WSDL_TK_GLIB_VOID:
	case WSDL_TK_GLIB_BOOLEAN:
	case WSDL_TK_GLIB_CHAR:
	case WSDL_TK_GLIB_UCHAR:
	case WSDL_TK_GLIB_INT:
	case WSDL_TK_GLIB_UINT:
	case WSDL_TK_GLIB_SHORT:
	case WSDL_TK_GLIB_USHORT:
	case WSDL_TK_GLIB_LONG:
	case WSDL_TK_GLIB_ULONG:
	case WSDL_TK_GLIB_INT8:
	case WSDL_TK_GLIB_UINT8:
	case WSDL_TK_GLIB_INT16:
	case WSDL_TK_GLIB_UINT16:
	case WSDL_TK_GLIB_INT32:
	case WSDL_TK_GLIB_UINT32:
	case WSDL_TK_GLIB_FLOAT:
	case WSDL_TK_GLIB_DOUBLE:
	case WSDL_TK_GLIB_STRING:
	case WSDL_TK_GLIB_LIST:
	case WSDL_TK_GLIB_MAX:
		str = wsdl_typecode_type (tc);
		break;
	}

	if (str == NULL) {
		str = g_strdup ("NULL");
	}

	return (str);
}

static void
tc_indent (guint ind)
{
	unsigned int i;

	for (i = 0; i < ind; i++) {
		g_print (" ");
	}
}

/**
 * wsdl_typecode_print:
 * @tc: a typecode pointer
 * @ind: an integer specifying the indent level to use
 *
 * Produces a printable representation of typecode @tc on standard
 * output.
 */
void
wsdl_typecode_print (const wsdl_typecode * const tc, guint ind)
{
	guint i;

	g_assert (tc != NULL);

	tc_indent (ind);
	g_print ("%s ", wsdl_typecode_kind_name (tc->kind));
	g_print ("%s (%s:%s):\n", tc->name, tc->ns, tc->nsuri);

	switch (tc->kind) {
	default:
		g_print ("\n");
		break;

	case WSDL_TK_GLIB_ELEMENT:
		wsdl_typecode_print (tc->subtypes[0], ind + 4);
		break;

	case WSDL_TK_GLIB_STRUCT:
		for (i = 0; i < tc->sub_parts; i++) {
			tc_indent (ind + 2);
			g_print ("%s:\n", tc->subnames[i]);

			wsdl_typecode_print (tc->subtypes[i], ind + 4);
		}
		break;

	case WSDL_TK_GLIB_LIST:
		wsdl_typecode_print (tc->subtypes[0], ind + 4);
		break;
	}
}

static void
wsdl_typecode_init (void)
{
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_void_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_boolean_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_char_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_uchar_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_int_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_uint_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_short_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_ushort_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_long_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_ulong_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_int8_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_uint8_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_int16_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_uint16_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_int32_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_uint32_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_float_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_double_struct);
	wsdl_typecodes =
		g_slist_append (wsdl_typecodes,
				(gpointer) & WSDL_TC_glib_string_struct);
}

/**
 * wsdl_typecode_lookup:
 * @name: a string containing typecode name, which may have an
 * optional namespace prefix delimited by a ':'
 * @nsuri: a string containing a namespace URI, which may be #NULL.
 *
 * Looks up a typecode by comparing the @name with any prefix removed
 * with the known typecode names.  If @nsuri is not #NULL, the
 * typecode namespace URI must also match @nsuri.
 *
 * Returns: a typecode pointer, or #NULL if a matching typecode could
 * not be found.
 */
const wsdl_typecode *
wsdl_typecode_lookup (const guchar * name, const guchar * nsuri)
{
	GSList *iter;
	guchar *colon;
	const guchar *compare;

	if (wsdl_typecodes == NULL) {
		wsdl_typecode_init ();
	}

	if ((colon = strchr (name, ':')) != NULL) {
		compare = colon + 1;
	} else {
		compare = name;
	}

	iter = wsdl_typecodes;
	while (iter != NULL) {
		wsdl_typecode *tc = (wsdl_typecode *) iter->data;

		/* Check for namespace-qualified names */
		if (nsuri != NULL && !strcmp (compare, tc->name) &&
		    !strcmp (nsuri, tc->nsuri)) {
			return (tc);
		}

		/* Check for any match */
		if (nsuri == NULL && !strcmp (compare, tc->name)) {
			return (tc);
		}

		iter = iter->next;
	}

	return (NULL);
}

/**
 * wsdl_typecode_register:
 * @tc: a typecode pointer
 *
 * Stores the typecode pointer @tc in the internal list of known typecodes.
 */
void
wsdl_typecode_register (const wsdl_typecode * const tc)
{
	if (wsdl_typecodes == NULL) {
		wsdl_typecode_init ();
	}

	/* g_slist_append really should declare the data arg const */
	wsdl_typecodes = g_slist_append (wsdl_typecodes, (gpointer) tc);
}

/**
 * wsdl_typecode_unregister:
 * @name: a string containing typecode name, which may have an
 * optional namespace prefix delimited by a ':'
 * @nsuri: a string containing a namespace URI, which may be #NULL.
 *
 * Finds a typecode by calling wsdl_typecode_lookup(), removes it from
 * the list of known typecodes and then passes it to
 * wsdl_typecode_free().
 */
void
wsdl_typecode_unregister (const guchar * name, const guchar * nsuri)
{
	const wsdl_typecode *tc;

	g_return_if_fail (name == NULL);

	tc = wsdl_typecode_lookup (name, nsuri);
	if (tc == NULL) {
		return;
	}

	wsdl_typecodes = g_slist_remove (wsdl_typecodes, (gpointer) tc);

	if (tc->dynamic) {
		wsdl_typecode_free ((gpointer) tc);
	}
}

/**
 * wsdl_typecode_free:
 * @tc: a typecode pointer
 *
 * Recursively frees all memory used by a typecode.
 */
void
wsdl_typecode_free (wsdl_typecode * tc)
{
	gulong i;

	g_return_if_fail (tc == NULL);
	g_return_if_fail (tc->dynamic == FALSE);

	if (tc->name != NULL) {
		g_free ((gpointer) tc->name);
	}
	if (tc->ns != NULL) {
		g_free ((gpointer) tc->ns);
	}
	if (tc->nsuri != NULL) {
		g_free ((gpointer) tc->nsuri);
	}

	if (tc->kind == WSDL_TK_GLIB_ELEMENT || tc->kind == WSDL_TK_GLIB_STRUCT
	    || tc->kind == WSDL_TK_GLIB_LIST) {
		for (i = 0; i < tc->sub_parts; i++) {
			if (tc->subnames[i] != NULL) {
				g_free ((gpointer) tc->subnames[i]);
			}
			if (tc->kind == WSDL_TK_GLIB_STRUCT &&
			    tc->subtypes[i] != NULL) {
				wsdl_typecode_free ((gpointer) tc->subtypes[i]);
			}
		}
	}

	g_free (tc);
}

/**
 * wsdl_typecode_free_all:
 *
 * Unregisters all known typecodes, freeing any memory used.
 */
void
wsdl_typecode_free_all (void)
{
	GSList *iter = wsdl_typecodes;

	while (iter != NULL) {
		wsdl_typecode *tc = (wsdl_typecode *) iter->data;


		wsdl_typecodes = g_slist_remove (wsdl_typecodes, tc);

		if (tc->dynamic) {
			wsdl_typecode_free (tc);
		}

		iter = iter->next;
	}
}

/**
 * wsdl_typecode_foreach:
 * @predefined: a boolean, selecting whether to include predefined typecodes
 * @callback: a function to call for each typecode
 * @user_data: a generic pointer to anything
 *
 * For each typecode known (including predefined ones if @predefined is
 * #TRUE), @callback is called with arguments of the typecode, and
 * @user_data.
 */
void
wsdl_typecode_foreach (gboolean              predefined,
		       WsdlTypecodeForeachFn callback,
		       gpointer              user_data)
{
	GSList *iter = wsdl_typecodes;

	while (iter != NULL) {
		const wsdl_typecode *const tc = (wsdl_typecode *) iter->data;

		if (predefined == TRUE || tc->kind >= WSDL_TK_GLIB_ELEMENT) {
			callback (tc, user_data);
		}

		iter = iter->next;
	}
}

/**
 * wsdl_typecode_find_alignment:
 * @tc: a typecode pointer
 *
 * Works out the C type alignment required for typecode @tc, using
 * data supplied by %configure.
 *
 * Returns: the alignment
 */
guint
wsdl_typecode_find_alignment (const wsdl_typecode * const tc)
{
	switch (tc->kind) {
	case WSDL_TK_GLIB_NULL:
		g_warning ("Invalid typecode NULL in " G_GNUC_FUNCTION);
		return (0);
		break;

	case WSDL_TK_GLIB_VOID:
		return (0);
		break;

	case WSDL_TK_GLIB_BOOLEAN:
		return (ALIGNOF_GBOOLEAN);
		break;

	case WSDL_TK_GLIB_CHAR:
		return (ALIGNOF_GCHAR);
		break;

	case WSDL_TK_GLIB_UCHAR:
		return (ALIGNOF_GUCHAR);
		break;

	case WSDL_TK_GLIB_INT:
		return (ALIGNOF_GINT);
		break;

	case WSDL_TK_GLIB_UINT:
		return (ALIGNOF_GUINT);
		break;

	case WSDL_TK_GLIB_SHORT:
		return (ALIGNOF_GSHORT);
		break;

	case WSDL_TK_GLIB_USHORT:
		return (ALIGNOF_GUSHORT);
		break;

	case WSDL_TK_GLIB_LONG:
		return (ALIGNOF_GLONG);
		break;

	case WSDL_TK_GLIB_ULONG:
		return (ALIGNOF_GULONG);
		break;

	case WSDL_TK_GLIB_INT8:
		return (ALIGNOF_GINT8);
		break;

	case WSDL_TK_GLIB_UINT8:
		return (ALIGNOF_GUINT8);
		break;

	case WSDL_TK_GLIB_INT16:
		return (ALIGNOF_GINT16);
		break;

	case WSDL_TK_GLIB_UINT16:
		return (ALIGNOF_GUINT16);
		break;

	case WSDL_TK_GLIB_INT32:
		return (ALIGNOF_GINT32);
		break;

	case WSDL_TK_GLIB_UINT32:
		return (ALIGNOF_GUINT32);
		break;

	case WSDL_TK_GLIB_FLOAT:
		return (ALIGNOF_GFLOAT);
		break;

	case WSDL_TK_GLIB_DOUBLE:
		return (ALIGNOF_GDOUBLE);
		break;

	case WSDL_TK_GLIB_STRING:
		return (ALIGNOF_GPOINTER);
		break;

	case WSDL_TK_GLIB_ELEMENT:
		return (MAX
			(1, wsdl_typecode_find_alignment (tc->subtypes[0])));
		break;

	case WSDL_TK_GLIB_STRUCT:
		{
			gulong i;
			guint align = 1;

			for (i = 0; i < tc->sub_parts; i++) {
				align = MAX (align,
					     wsdl_typecode_find_alignment (
					              tc->subtypes [i]));
			}

			return (align);
			break;
		}

	case WSDL_TK_GLIB_LIST:
		return (ALIGNOF_GPOINTER);
		break;

	case WSDL_TK_GLIB_MAX:
		g_warning ("Invalid typecode MAX in " G_GNUC_FUNCTION);
		return (0);
		break;
	}

	/* gcc should know that it can't reach here */
	g_assert_not_reached ();
	return (0);
}

/**
 * wsdl_typecode_size:
 * @tc: a typecode pointer
 *
 * Works out the size in bytes needed for the C representation of a
 * typecode, taking alignment into account.
 *
 * Returns: the size in bytes
 */
guint
wsdl_typecode_size (const wsdl_typecode * const tc)
{
	switch (tc->kind) {
	case WSDL_TK_GLIB_NULL:
		g_warning ("Invalid typecode NULL in " G_GNUC_FUNCTION);
		return (0);
		break;

	case WSDL_TK_GLIB_VOID:
		return (0);
		break;

	case WSDL_TK_GLIB_BOOLEAN:
		return (sizeof (gboolean));
		break;

	case WSDL_TK_GLIB_CHAR:
		return (sizeof (gchar));
		break;

	case WSDL_TK_GLIB_UCHAR:
		return (sizeof (guchar));
		break;

	case WSDL_TK_GLIB_INT:
		return (sizeof (gint));
		break;

	case WSDL_TK_GLIB_UINT:
		return (sizeof (guint));
		break;

	case WSDL_TK_GLIB_SHORT:
		return (sizeof (gshort));
		break;

	case WSDL_TK_GLIB_USHORT:
		return (sizeof (gushort));
		break;

	case WSDL_TK_GLIB_LONG:
		return (sizeof (glong));
		break;

	case WSDL_TK_GLIB_ULONG:
		return (sizeof (gulong));
		break;

	case WSDL_TK_GLIB_INT8:
		return (sizeof (gint8));
		break;

	case WSDL_TK_GLIB_UINT8:
		return (sizeof (guint8));
		break;

	case WSDL_TK_GLIB_INT16:
		return (sizeof (gint16));
		break;

	case WSDL_TK_GLIB_UINT16:
		return (sizeof (guint16));
		break;

	case WSDL_TK_GLIB_INT32:
		return (sizeof (gint32));
		break;

	case WSDL_TK_GLIB_UINT32:
		return (sizeof (guint32));
		break;

	case WSDL_TK_GLIB_FLOAT:
		return (sizeof (gfloat));
		break;

	case WSDL_TK_GLIB_DOUBLE:
		return (sizeof (gdouble));
		break;

	case WSDL_TK_GLIB_STRING:
		return (sizeof (guchar *));
		break;

	case WSDL_TK_GLIB_ELEMENT:
		return (wsdl_typecode_size (tc->subtypes[0]));
		break;

	case WSDL_TK_GLIB_STRUCT:
		{
			gulong i;
			guint sum = 0;

			for (i = 0; i < tc->sub_parts; i++) {
				sum = 
					GPOINTER_TO_INT (ALIGN_ADDRESS (
						sum,
						wsdl_typecode_find_alignment (
							tc->subtypes[i])));
				sum += wsdl_typecode_size (tc->subtypes[i]);
			}

			sum =
				GPOINTER_TO_INT (ALIGN_ADDRESS (
					sum,
					wsdl_typecode_find_alignment (tc)));

			return (sum);
			break;
		}

	case WSDL_TK_GLIB_LIST:
		return (sizeof (GSList *));
		break;

	case WSDL_TK_GLIB_MAX:
		g_warning ("Invalid typecode MAX in " G_GNUC_FUNCTION);
		return (0);
		break;
	}

	/* gcc should know that it can't reach here */
	g_assert_not_reached ();
	return (0);
}

/**
 * wsdl_typecode_alloc:
 * @tc: a typecode pointer
 *
 * Allocated memory for the C representation of the typecode @tc.
 *
 * Returns: allocated memory, which must be freed by the caller.
 */
gpointer
wsdl_typecode_alloc (const wsdl_typecode * const tc)
{
	guint size;
	gpointer ret;

	size = wsdl_typecode_size (tc);
	ret = g_malloc0 (size);

	return (ret);
}

/**
 * wsdl_typecode_offset:
 * @tc: a typecode pointer
 * @name: a string containing a typecode member name
 * @offset: a pointer to an integer
 *
 * If @tc is a typecode of kind #WSDL_TK_GLIB_STRUCT, finds the member
 * named @name and works out the offset to this member from the start
 * of the C representation of this typecode.  The offset is stored in
 * the integer pointed to by @offset.
 *
 * Returns: the typecode of the member @name, or #NULL if the member
 * isn't found or if the typecode @tc isn't of kind
 * #WSDL_TK_GLIB_STRUCT.
 */
const wsdl_typecode *
wsdl_typecode_offset (const wsdl_typecode * const tc, const guchar * name,
		      guint * offset)
{
	const wsdl_typecode *ret = NULL;
	gulong i;

	g_assert (tc != NULL);

	*offset = 0;

	if (tc->kind != WSDL_TK_GLIB_STRUCT) {
		return (NULL);
	}

	for (i = 0; i < tc->sub_parts; i++) {
		if (!strcmp (name, tc->subnames[i])) {
			ret = tc->subtypes[i];
			break;
		}
		*offset += wsdl_typecode_size (tc->subtypes[i]);
	}

	return (ret);
}
