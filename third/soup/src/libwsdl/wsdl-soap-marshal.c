/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-marshal.c: Runtime SOAP XML document construction
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <stdlib.h>

#include <libsoup/soup-parser.h>
#include <libsoup/soup-serializer.h>

#include "wsdl-soap-marshal.h"
#include "wsdl-typecodes.h"

static void wsdl_soap_marshal_param (SoupSerializer            *ser,
				     const guchar        *const name,
				     const wsdl_typecode *const typecode,
				     const gpointer             value);

static void
wsdl_soap_marshal_simple_param (SoupSerializer             *ser,
				const wsdl_typecode * const typecode,
				const gpointer              value)
{
	wsdl_typecode_kind_t kind;
	guchar *str = NULL;

	kind = wsdl_typecode_kind (typecode);

	g_return_if_fail (kind > WSDL_TK_GLIB_NULL &&
			  kind <= WSDL_TK_GLIB_STRING);

	switch (kind) {
	case WSDL_TK_GLIB_VOID:
		/* Nothing to do here */
		return;
		break;

	case WSDL_TK_GLIB_BOOLEAN:
		{
			gboolean bool = *(gboolean *) value;

			if (bool == TRUE) {
				str = g_strdup ("true");
			} else {
				str = g_strdup ("false");
			}
			break;
		}

	case WSDL_TK_GLIB_CHAR:
		{
			gchar ch = *(gchar *) value;

			str = g_strdup_printf ("%hhd", ch);
			break;
		}

	case WSDL_TK_GLIB_UCHAR:
		{
			guchar uch = *(guchar *) value;

			str = g_strdup_printf ("%hhu", uch);
			break;
		}

	case WSDL_TK_GLIB_INT:
		{
			gint intval = *(gint *) value;

			str = g_strdup_printf ("%d", intval);
			break;
		}

	case WSDL_TK_GLIB_UINT:
		{
			guint uintval = *(guint *) value;

			str = g_strdup_printf ("%u", uintval);
			break;
		}

	case WSDL_TK_GLIB_SHORT:
		{
			gshort shortval = *(gshort *) value;

			str = g_strdup_printf ("%hd", shortval);
			break;
		}

	case WSDL_TK_GLIB_USHORT:
		{
			gushort ushortval = *(gushort *) value;

			str = g_strdup_printf ("%hu", ushortval);
			break;
		}

	case WSDL_TK_GLIB_LONG:
		{
			glong longval = *(glong *) value;

			str = g_strdup_printf ("%ld", longval);
			break;
		}

	case WSDL_TK_GLIB_ULONG:
		{
			gulong ulongval = *(gulong *) value;

			str = g_strdup_printf ("%lu", ulongval);
			break;
		}

	case WSDL_TK_GLIB_INT8:
		{
			gint8 int8 = *(gint8 *) value;

			str = g_strdup_printf ("%hhd", int8);
			break;
		}

	case WSDL_TK_GLIB_UINT8:
		{
			guint8 uint8 = *(guint8 *) value;

			str = g_strdup_printf ("%hhu", uint8);
			break;
		}

	case WSDL_TK_GLIB_INT16:
		{
			gint16 int16 = *(gint16 *) value;

			str = g_strdup_printf ("%hd", int16);
			break;
		}

	case WSDL_TK_GLIB_UINT16:
		{
			guint16 uint16 = *(guint16 *) value;

			str = g_strdup_printf ("%hu", uint16);
			break;
		}

	case WSDL_TK_GLIB_INT32:
		{
			gint32 int32 = *(gint32 *) value;

			str = g_strdup_printf ("%d", int32);
			break;
		}

	case WSDL_TK_GLIB_UINT32:
		{
			guint32 uint32 = *(guint32 *) value;

			str = g_strdup_printf ("%u", uint32);
			break;
		}

	case WSDL_TK_GLIB_FLOAT:
		{
			gfloat floatval = *(gfloat *) value;

			str = g_strdup_printf ("%f", floatval);
			break;
		}

	case WSDL_TK_GLIB_DOUBLE:
		{
			gdouble doubleval = *(gdouble *) value;

			str = g_strdup_printf ("%f", doubleval);
			break;
		}

	case WSDL_TK_GLIB_STRING:
		{
			guchar *strval = *(guchar **) value;

			str = g_strdup_printf ("%s", strval);
			break;
		}

	case WSDL_TK_GLIB_NULL:
	case WSDL_TK_GLIB_ELEMENT:
	case WSDL_TK_GLIB_STRUCT:
	case WSDL_TK_GLIB_LIST:
	case WSDL_TK_GLIB_MAX:
		/* Handled elsewhere */
		break;
	}

	soup_serializer_write_string (ser, str);
	g_free (str);
}

static void
wsdl_soap_marshal_struct_param (SoupSerializer             *ser,
				const wsdl_typecode * const typecode,
				const gpointer              value)
{
	guint i;

	g_assert (typecode != NULL);
	g_assert (typecode->kind == WSDL_TK_GLIB_STRUCT);

	for (i = 0; i < typecode->sub_parts; i++) {
		guint offset;
		const wsdl_typecode *subtype;

		subtype = wsdl_typecode_offset (typecode, 
						typecode->subnames[i],
						&offset);

		/* For some reason gcc moans about taking
		 * offsets from void * pointers
		 */
		wsdl_soap_marshal_param (
			ser, 
			typecode->subnames[i], 
			subtype,
			ALIGN_ADDRESS (*(guchar **) value + offset,
				       wsdl_typecode_find_alignment (subtype)));
	}
}

static void
wsdl_soap_marshal_list_param (SoupSerializer             *ser, 
			      const guchar               *name,
			      const wsdl_typecode * const typecode,
			      const gpointer              value)
{
	/* value holds the pointer to a GSList */
	/* typecode defines what the list contains */
	GSList *iter = *(GSList **) value;

	while (iter != NULL) {
		soup_serializer_start_element (ser, name, NULL, NULL);

		if (wsdl_typecode_is_simple (typecode) == FALSE ||
		    wsdl_typecode_element_kind (typecode) ==
		    WSDL_TK_GLIB_STRING) {
			wsdl_soap_marshal_param (ser, 
						 typecode->name, 
						 typecode,
						 &iter->data);
		} else {
			wsdl_soap_marshal_param (ser, 
						 typecode->name, 
						 typecode,
						 iter->data);
		}

		soup_serializer_end_element (ser);
		iter = iter->next;
	}
}

static void
wsdl_soap_marshal_param (SoupSerializer             *ser, 
			 const guchar * const        name,
			 const wsdl_typecode * const typecode,
			 const gpointer              value)
{
	wsdl_typecode_kind_t kind;

	/* Check that value isnt pointing to NULL */
	if (value == NULL || *(guchar **) value == NULL) {
		return;
	}

	kind = wsdl_typecode_kind (typecode);
	if (kind == WSDL_TK_GLIB_ELEMENT) {
		/* The real type is stored in element 0 of subtypes[] */
		wsdl_soap_marshal_param (ser, 
					 name, 
					 typecode->subtypes[0],
					 value);
	} else if (kind == WSDL_TK_GLIB_STRUCT) {
		/* Fill in each structure element */
		soup_serializer_start_element (ser, name, NULL, NULL);
		wsdl_soap_marshal_struct_param (ser, typecode, value);
		soup_serializer_end_element (ser);
	} else if (kind == WSDL_TK_GLIB_LIST) {
		/* Fill in a list of the type stored in element 0 of
		 * subtypes[]
		 */
		wsdl_soap_marshal_list_param (ser, 
					      name, 
					      typecode->subtypes[0],
					      value);
	} else {
		soup_serializer_start_element (ser, name, NULL, NULL);
		wsdl_soap_marshal_simple_param (ser, typecode, value);
		soup_serializer_end_element (ser);
	}

}

/**
 * wsdl_soap_marshal:
 * @operation: a string containing the name of the operation being
 * marshalled.
 * @ns: a string containing a namespace reference
 * @ns_uri: a string containing a namespace URI
 * @params: a pointer to an array of #wsdl_param, terminated by a set
 * of #NULL elements
 * @buffer: a pointer to a #SoupDataBuffer
 * @env: a pointer to a #SoupEnv struct, which is expected to have
 * been initialised previously by soup_env_new()
 * @flags: an integer holding the bit flags #WSDL_SOAP_FLAGS_REQUEST
 * or #WSDL_SOAP_FLAGS_RESPONSE
 *
 * Marshals each element of @params into a SOAP message, and stores it
 * in @buffer.
 *
 * Returns: 0
 */
guint
wsdl_soap_marshal (const guchar * const     operation, 
		   const guchar * const     ns,
		   const guchar * const     ns_uri, 
		   const wsdl_param * const params,
		   SoupDataBuffer          *buffer, 
		   SoupEnv                 *env, 
		   gint                     flags)
{
	SoupSerializer *ser;
	const wsdl_param *param = params;
	SoupFault *fault;

	/* I'd use g_assert, but this is in a library :-) */
	if (params == NULL) {
		g_warning ("No params!");
		return (0);
	}

	ser = soup_serializer_new ();
	soup_serializer_start_envelope (ser);

	/* add headers to SOAP message */
	if (flags & WSDL_SOAP_FLAGS_REQUEST || 
	    flags & WSDL_SOAP_FLAGS_RESPONSE) {
		gboolean header_started = FALSE;
		const GSList *headers = soup_env_list_send_headers (env);
		const GSList *l;

		for (l = headers; l; l = g_slist_next (l)) {
			const SoupSOAPHeader *hdr = l->data;

			/* start header */
			if (!header_started) {
				soup_serializer_start_header (ser);
				header_started = TRUE;
			}

			/* add header element */
			soup_serializer_start_header_element (
				ser, 
				hdr->name, 
				hdr->must_understand,
				hdr->actor_uri, 
				NULL, 
				hdr->ns_uri);

			/* add header content */
			if (hdr->value)
				soup_serializer_write_string (ser,hdr->value);

			soup_serializer_end_header_element (ser);
		}

		if (header_started)
			soup_serializer_end_header (ser);
	}

	soup_serializer_start_body (ser);

	if ((fault = soup_env_get_fault (env))) {
		/* marshal the SOAP fault */
		soup_serializer_start_fault (ser, 
					     soup_fault_get_code (fault),
					     soup_fault_get_string (fault),
					     soup_fault_get_actor (fault));

		soup_serializer_start_fault_detail (ser);
		soup_serializer_write_string (ser,
					      soup_fault_get_detail (fault));
		soup_serializer_end_fault_detail (ser);

		soup_serializer_end_fault (ser);
	} else {
		/* marshal the return parameters */
		soup_serializer_start_element (ser, operation, ns, ns_uri);

		while (param->name != NULL) {
			wsdl_soap_marshal_param (ser, 
						 param->name,
						 param->typecode, 
						 param->param);
			param++;
		}

		soup_serializer_end_element (ser);
	}

	soup_serializer_end_body (ser);
	soup_serializer_end_envelope (ser);

	soup_serializer_persist (ser, buffer);

	soup_serializer_free (ser);

	return (0);
}
