/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-serializer.h: Asyncronous Callback-based SOAP Request Queue.
 *
 * Authors:
 *      Alex Graveley (alex@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */

#include "soup-serializer.h"
#include "soup-misc.h"

/* 
 * UNIT TESTS:
 * - test env_prefix/uri, xml encoding, standalone to _full
 * - element creation with prefix and uri
 * - fault creation
 * - fault detail
 * - header elem creation, with uri/prefix, mustUnderstand, and actorUri
 * - write base64, int, double, time
 * - set encoding style, multiple times on the same element, child overriding
 * - add attribute, adding multiple times with different uris 
 * - add attribute to children with same and different uris
 * - set default NS, multiple times on the same element, child overriding
 * - unbalanced element warning
 */

struct _SoupSerializer {
	xmlDocPtr  doc;
	xmlNodePtr last_node;
	xmlNsPtr   soap_ns;
	xmlNsPtr   xsi_ns;
	gchar      *env_prefix;
	gchar      *env_uri;
	gboolean    body_started;
	gchar      *action;
};

static xmlNsPtr
soup_serializer_fetch_ns (SoupSerializer *ser,
			  const gchar    *prefix,
			  const gchar    *ns_uri) 
{
	xmlNsPtr ns = NULL;

	if (prefix && ns_uri)
		ns = xmlNewNs (ser->last_node, ns_uri, prefix);
	else if (prefix && !ns_uri) {
		ns = xmlSearchNs (ser->doc, ser->last_node, prefix);
		if (!ns) ns = xmlNewNs (ser->last_node, "", prefix);
	}

	return ns;
}

/**
 * soup_serializer_new:
 * 
 * Creates a new %SoupSerializer.
 * 
 * Return value: The created %SoupSerializer.
 **/
SoupSerializer *
soup_serializer_new (void)
{
	SoupSerializer *ser = g_new0 (SoupSerializer, 1);
	ser->doc = xmlNewDoc ("1.0");
	ser->doc->standalone = FALSE;
	ser->doc->encoding = g_strdup ("UTF-8");
	return ser;
}

/**
 * soup_serializer_new_full:
 * @standalone: 
 * @xml_encoding: 
 * @env_prefix: 
 * @env_uri: 
 * 
 * Creates a new %SoupSerializer, using @xml_encoding as the encoding attribute
 * for the document, @env_prefix as the default XML Namespace prefix for all
 * SOAP elements, and @env_uri as the XML Namespace uri for SOAP elements.
 * 
 * Return value: The created %SoupSerializer.
 **/
SoupSerializer *
soup_serializer_new_full (gboolean     standalone, 
			  const gchar *xml_encoding,
			  const gchar *env_prefix,
			  const gchar *env_uri)
{
	SoupSerializer *ser = soup_serializer_new ();
	ser->doc->standalone = standalone;
	if (xml_encoding) {
		g_free ((char *) ser->doc->encoding);
		ser->doc->encoding = g_strdup (xml_encoding);
	}
	if (env_prefix || env_uri) {
		ser->env_prefix = g_strdup (env_prefix);
		ser->env_uri = g_strdup (env_uri);
	}
	return ser;
}

/**
 * soup_serializer_get_xml_doc:
 * @ser: the %SoupSerializer
 * 
 * Returns the internal xml representation tree of the %SoupSerializer pointed
 * to by @ser.
 * 
 * Return value: the xmlDocPtr representing the SOAP message.
 **/
xmlDocPtr
soup_serializer_get_xml_doc (SoupSerializer *ser)
{
	g_return_val_if_fail (ser != NULL, NULL);

	return ser->doc;
}

/**
 * soup_serializer_free:
 * @ser: the %SoupSerializer
 * 
 * Frees the %SoupSerializer pointed to by @ser, and the internal XML
 * represetation of the SOAP message.
 **/
void 
soup_serializer_free (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	soup_serializer_reset (ser);
	g_free (ser);
}


/**
 * soup_serializer_start_envelope:
 * @ser: the %SoupSerializer
 * 
 * Starts the top level SOAP Envelope element.
 **/
void 
soup_serializer_start_envelope (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = ser->doc->xmlRootNode = 
		xmlNewDocNode (ser->doc, NULL, "Envelope", NULL);

	ser->soap_ns = xmlNewNs (ser->doc->xmlRootNode, 
				 ser->env_uri ? ser->env_uri : 
				 "http://schemas.xmlsoap.org/soap/envelope/", 
				 ser->env_prefix ? ser->env_prefix : 
				 "SOAP-ENV");
	if (ser->env_uri) { 
		g_free (ser->env_uri); 
		ser->env_uri = NULL;
	}
	if (ser->env_prefix) { 
		g_free (ser->env_prefix); 
		ser->env_prefix = NULL;
	}

	xmlSetNs (ser->doc->xmlRootNode, ser->soap_ns);

	xmlNewNs (ser->doc->xmlRootNode, 
		  "http://schemas.xmlsoap.org/soap/encoding/", 
		  "SOAP-ENC");

	xmlNewNs (ser->doc->xmlRootNode, 
		  "http://www.w3.org/1999/XMLSchema", 
		  "xsd");

	ser->xsi_ns = xmlNewNs (ser->doc->xmlRootNode, 
				"http://www.w3.org/1999/XMLSchema-instance", 
				"xsi");
}

/**
 * soup_serializer_end_envelope:
 * @ser: the %SoupSerializer
 * 
 * Closes the top level SOAP Envelope element.
 **/
void 
soup_serializer_end_envelope (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_start_body:
 * @ser: the %SoupSerializer
 * 
 * Starts the SOAP Body element.
 **/
void 
soup_serializer_start_body (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = xmlNewChild (ser->last_node, 
				      ser->soap_ns, 
				      "Body", 
				      NULL);

	ser->body_started = TRUE;
}

/**
 * soup_serializer_end_body:
 * @ser: the %SoupSerializer
 * 
 * Closes the SOAP Body element.
 **/
void soup_serializer_end_body (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_start_element:
 * @ser: the %SoupSerializer
 * @name: the element name.
 * @prefix: the namespace prefix
 * @ns_uri: the namespace URI
 * 
 * Starts a new arbitrary message element, with @name as the element name,
 * @prefix as the XML Namespace prefix, and @ns_uri as the XML Namespace uri for
 * the created element.
 *
 * Passing @prefix with no @ns_uri will cause a recursive search for an 
 * existing namespace with the same prefix. Failing that a new ns will be 
 * created with an empty uri. 
 * 
 * Passing both @prefix and @ns_uri always causes new namespace attribute
 * creation.
 * 
 * Passing NULL for both @prefix and @ns_uri causes no prefix to be used, and
 * the element will be in the default namespace.
 **/
void 
soup_serializer_start_element (SoupSerializer *ser,
			       const gchar    *name, 
			       const gchar    *prefix,
			       const gchar    *ns_uri)
{
	g_return_if_fail (ser != NULL && name != NULL);

	ser->last_node = xmlNewChild (ser->last_node, NULL, name, NULL);

	xmlSetNs (ser->last_node, 
		  soup_serializer_fetch_ns (ser, prefix, ns_uri));

	if (ser->body_started && !ser->action)
		ser->action = g_strconcat (ns_uri ? ns_uri : "", 
					   "#", 
					   name, 
					   NULL);
}

/**
 * soup_serializer_end_element:
 * @ser: the %SoupSerializer
 * 
 * Close the current message element.
 **/
void 
soup_serializer_end_element (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = ser->last_node->parent;
}

/**
 * soup_serializer_start_fault:
 * @ser: the %SoupSerializer
 * @faultcode: faultcode element value
 * @faultstring: faultstring element value
 * @faultactor: faultactor element value
 * 
 * Starts a new SOAP Fault element, creating faultcode, faultstring, and
 * faultactor child elements.
 *
 * If you wish to add the faultdetail element, use
 * %soup_serializer_start_fault_detail, and then %soup_serializer_start_element
 * to add arbitrary sub-elements.
 **/
void 
soup_serializer_start_fault (SoupSerializer *ser,
			     const gchar    *faultcode,
			     const gchar    *faultstring,
			     const gchar    *faultactor)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = xmlNewChild (ser->last_node, 
				      ser->soap_ns, 
				      "Fault", 
				      NULL);

	xmlNewChild (ser->last_node, 
		     ser->soap_ns, 
		     "faultcode", 
		     faultcode);

	xmlNewChild (ser->last_node, 
		     ser->soap_ns, 
		     "faultstring", 
		     faultstring);

	ser->last_node = xmlNewChild (ser->last_node, 
				      ser->soap_ns, 
				      "faultactor", 
				      faultactor);

	if (!faultactor) soup_serializer_set_null (ser);

	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_end_fault:
 * @ser: the %SoupSerializer
 * 
 * Close the current SOAP Fault element. 
 **/
void 
soup_serializer_end_fault (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_start_fault_detail:
 * @ser: the %SoupSerializer
 * 
 * Start the faultdetail child element of the current SOAP Fault element. The
 * faultdetail element allows arbitrary data to be sent in a returned fault.
 **/
void 
soup_serializer_start_fault_detail (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = xmlNewChild (ser->last_node, 
				      ser->soap_ns, 
				      "detail", 
				      NULL);
}

/**
 * soup_serializer_end_fault_detail:
 * @ser: the %SoupSerializer
 * 
 * Closes the current SOAP faultdetail element.
 **/
void 
soup_serializer_end_fault_detail (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_start_header:
 * @ser: the %SoupSerializer
 * 
 * Creates a new SOAP Header element. You can call
 * %soup_serializer_start_header_element after this to add a new header child
 * element. SOAP Header elements allow out-of-band data to be transferred while
 * not interfering with the message body.
 *
 * This should be called after %soup_serializer_start_envelope and before
 * soup_serializer_start_body.
 **/
void 
soup_serializer_start_header (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	ser->last_node = xmlNewChild (ser->last_node, 
				      ser->soap_ns, 
				      "Header", 
				      NULL);
}

/**
 * soup_serializer_end_header:
 * @ser: the %SoupSerializer
 * 
 * Close the current SOAP Header element.
 **/
void 
soup_serializer_end_header (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}

/**
 * soup_serializer_start_header_element:
 * @ser: the %SoupSerializer
 * @name: name of the header element
 * @must_understand: whether the recipient must understand the header in order
 * to proceed with processing the message
 * @actor_uri: the URI which represents the destination actor for this header.
 * @prefix: the namespace prefix
 * @ns_uri: the namespace URI
 * 
 * Starts a new SOAP arbitrary header element.
 **/
void 
soup_serializer_start_header_element  (SoupSerializer *ser,
				       const gchar    *name,
				       gboolean        must_understand,
				       const gchar    *actor_uri,
				       const gchar    *prefix,
				       const gchar    *ns_uri)
{
	g_return_if_fail (ser != NULL);

	soup_serializer_start_element (ser, 
				       name, 
				       prefix, 
				       ns_uri);

	if (actor_uri) xmlNewNsProp (ser->last_node, 
				     ser->soap_ns, 
				     "actorUri", 
				     actor_uri);

	if (must_understand) xmlNewNsProp (ser->last_node, 
					   ser->soap_ns, 
					   "mustUnderstand", 
					   "1");
}

/**
 * soup_serializer_end_header_element:
 * @ser: the %SoupSerializer
 * 
 * Closes the current SOAP header element
 **/
void 
soup_serializer_end_header_element (SoupSerializer *ser)
{
	soup_serializer_end_element (ser);
}


/**
 * soup_serializer_write_int:
 * @ser: the %SoupSerializer
 * @i: the integer value to write.
 * 
 * Writes the stringified value of @i as the current element's content.
 **/
void 
soup_serializer_write_int (SoupSerializer *ser, glong i)
{
	gchar *str = g_strdup_printf ("%ld", i);
	soup_serializer_write_string (ser, str);
	g_free (str);
}

/**
 * soup_serializer_write_double:
 * @ser: the %SoupSerializer
 * @d: the double value to write
 * 
 * Writes the stringified value of @i as the current element's content.
 **/
void 
soup_serializer_write_double (SoupSerializer *ser, gdouble d)
{
	gchar *str = g_strdup_printf ("%f", d);
	soup_serializer_write_string (ser, str);
	g_free (str);	
}

/**
 * soup_serializer_write_base64:
 * @ser: the %SoupSerializer
 * @string: the binary data buffer to encode
 * @len: the length of data to encode
 * 
 * Writes the Base-64 encoded value of @string as the current element's content.
 **/
void 
soup_serializer_write_base64 (SoupSerializer *ser,
			      const gchar    *string,
			      guint           len)
{
	gchar *str = soup_base64_encode (string, len);
	soup_serializer_write_string (ser, str);
	g_free (str);
}

/**
 * soup_serializer_write_time:
 * @ser: the %SoupSerializer
 * @timeval: pointer to a time_t to encode
 * 
 * Writes the stringified value of @timeval as the current element's content.
 **/
void 
soup_serializer_write_time (SoupSerializer *ser, const time_t *timeval)
{
	gchar *str = g_strchomp (ctime (timeval));
	soup_serializer_write_string (ser, str);
}

/**
 * soup_serializer_write_string:
 * @ser: the %SoupSerializer
 * @string: string to write
 * 
 * Writes the @string as the current element's content.
 **/
void 
soup_serializer_write_string (SoupSerializer *ser, const gchar *string)
{
	g_return_if_fail (ser != NULL);

	xmlNodeAddContent (ser->last_node, string);
}

/**
 * soup_serializer_write_buffer:
 * @ser: the %SoupSerializer 
 * @buffer: the string data buffer to write
 * @length: length of @buffer
 * 
 * Writes the string buffer pointed to by @buffer and the current element's
 * content.
 **/
void 
soup_serializer_write_buffer (SoupSerializer *ser,
			      const gchar    *buffer, 
			      guint           length)
{
	g_return_if_fail (ser != NULL);

	xmlNodeAddContentLen (ser->last_node, buffer, length);
}

/**
 * soup_serializer_set_type:
 * @ser: the %SoupSerializer
 * @xsi_type: the type name for the element.
 * 
 * Sets the current element's XML Schema xsi:type attribute, which specifies the
 * element's type name.
 **/
void 
soup_serializer_set_type (SoupSerializer *ser,
			  const gchar    *xsi_type)
{
	g_return_if_fail (ser != NULL);

	xmlNewNsProp (ser->last_node, ser->xsi_ns, "type", xsi_type);
}

/**
 * soup_serializer_set_null:
 * @ser: the %SoupSerializer
 * 
 * Sets the current element's XML Schema xsi:null attribute.
 **/
void 
soup_serializer_set_null (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	xmlNewNsProp (ser->last_node, ser->xsi_ns, "null", "1");
}

/**
 * soup_serializer_add_attribute:
 * @ser: the %SoupSerializer
 * @name: name of the attribute
 * @value: value of the attribute
 * @prefix: the namespace prefix
 * @ns_uri: the namespace URI
 * 
 * Adds an XML attribute to the current element.
 **/
void 
soup_serializer_add_attribute (SoupSerializer *ser,
			       const gchar    *name,
			       const gchar    *value,
			       const gchar    *prefix,
			       const gchar    *ns_uri)
{
	g_return_if_fail (ser != NULL);

	xmlNewNsProp (ser->last_node, 
		      soup_serializer_fetch_ns (ser, prefix, ns_uri), 
		      name, 
		      value);
}

/**
 * soup_serializer_add_namespace:
 * @ser: the %SoupSerializer
 * @prefix: the namespace prefix
 * @ns_uri: the namespace URI, or NULL for empty namespace
 * 
 * Adds a new XML namespace to the current element.
 **/
void 
soup_serializer_add_namespace (SoupSerializer *ser,
			       const gchar    *prefix, 
			       const gchar    *ns_uri)
{
	g_return_if_fail (ser != NULL);

	xmlNewNs (ser->last_node, ns_uri ? ns_uri : "", prefix);
}

/**
 * soup_serializer_set_default_namespace:
 * @ser: the %SoupSerializer
 * @ns_uri: the namespace URI
 * 
 * Sets the default namespace to the URI specified in @ns_uri. The default
 * namespace becomes the namespace all non-explicitly namespaced child elements
 * fall into.
 **/
void 
soup_serializer_set_default_namespace (SoupSerializer *ser,
				       const gchar    *ns_uri)
{
	g_return_if_fail (ser != NULL);

	soup_serializer_add_namespace (ser, NULL, ns_uri);
}

/**
 * soup_serializer_get_namespace_prefix:
 * @ser: the %SoupSerializer
 * @ns_uri: the namespace URI
 * 
 * Return value: The namespace prefix for @ns_uri or an empty string if @ns_uri
 * is set to the default namespace. If no namespace exists for the URI given,
 * NULL is returned.
 **/
const gchar *
soup_serializer_get_namespace_prefix  (SoupSerializer *ser,
				       const gchar    *ns_uri)
{
	xmlNsPtr ns = NULL;

	g_return_val_if_fail (ser != NULL, NULL);
	g_return_val_if_fail (ns_uri != NULL, NULL);

	ns = xmlSearchNsByHref (ser->doc, ser->last_node, ns_uri);
	if (ns) {
		if (ns->prefix)
			return ns->prefix;
		else 
			return "";
	}

	return NULL;
}

/**
 * soup_serializer_set_encoding_style:
 * @ser: the %SoupSerializer
 * @enc_style: the new encodingStyle value
 * 
 * Sets the encodingStyle attribute on the current element to the value of
 * @enc_style.
 **/
void
soup_serializer_set_encoding_style (SoupSerializer *ser,
				    const gchar    *enc_style)
{
	g_return_if_fail (ser != NULL && enc_style != NULL);

	xmlNewNsProp (ser->last_node, ser->soap_ns, "encodingStyle", enc_style);
}

/**
 * soup_serializer_reset:
 * @ser: the %SoupSerializer
 * 
 * Reset the internal XML representation of the SOAP message.
 **/
void 
soup_serializer_reset (SoupSerializer *ser)
{
	g_return_if_fail (ser != NULL);

	xmlFreeDoc (ser->doc);
	ser->doc = xmlNewDoc ("1.0");
	ser->last_node = NULL;

	g_free (ser->action);
	ser->action = NULL;
	ser->body_started = FALSE;

	if (ser->env_uri) g_free (ser->env_uri); 
	ser->env_uri = NULL;

	if (ser->env_prefix) g_free (ser->env_prefix); 
	ser->env_prefix = NULL;
}

/**
 * soup_serializer_persist:
 * @ser: the %SoupSerializer
 * @dest: the destination %SoupDataBuffer
 * 
 * Writes the serialized XML tree to the %SoupDataBuffer pointed to by @dest.
 **/
void 
soup_serializer_persist (SoupSerializer *ser,
			 SoupDataBuffer *dest)
{
	g_return_if_fail (ser != NULL && dest != NULL);

	if (dest->body && dest->owner == SOUP_BUFFER_SYSTEM_OWNED)
		g_free (dest->body);

	xmlDocDumpMemory (ser->doc, (xmlChar **) &dest->body, &dest->length);
	dest->length *= sizeof (xmlChar);
	dest->owner = SOUP_BUFFER_SYSTEM_OWNED;
}
