/*
 * bonobo-config-utils.c: Utility functions
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */

#include <config.h>
#include <stdio.h>
#include <bonobo/bonobo-ui-node.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-xml.h>

#include "bonobo-config-utils.h"

/**
 * bonobo_config_dir_name:
 * @key: a configuration key 
 *
 * Gets the directory components of a configuration key. If @key does not 
 * contain a directory component it returns %NULL.
 *
 * Returns: the directory components of a configuration key.
 */
char *
bonobo_config_dir_name (const char *key)
{
	char *s;

	g_return_val_if_fail (key != NULL, NULL);
	
	if (!(s = strrchr (key, '/')))
		return NULL;
       
	while (s > key && *(s - 1) == '/')
		s--;

	if (s == key)
		return NULL;

	return g_strndup (key, s - key);
}

/**
 * bonobo_config_leaf_name:
 * @key: a configuration key 
 *
 * Gets the name of a configuration key without the leading directory 
 * component.
 *
 * Returns: the name of a configuration key without the leading directory 
 * component.
 */
char *
bonobo_config_leaf_name (const char *key)
{
	char *s;

	g_return_val_if_fail (key != NULL, NULL);
	
	if (!(s = strrchr (key, '/'))) {
		if (key [0])
			return g_strdup (key);
		return NULL;
	}

	if (s[1])
		return g_strdup (s + 1);
	
	return NULL;
}

#define ADD_STRING(text)                          G_STMT_START{         \
                l = strlen (text);                                      \
                memcpy (*buffer, text, l + 1);                          \
                *buffer += l; }G_STMT_END


#define ENCODE_TO_STRING(tckind,format,corbatype,value,align)		\
	case tckind:							\
		*value = ALIGN_ADDRESS (*value, align);			\
		snprintf (scratch, 127, format, *(corbatype *)*value);	\
                ADD_STRING(scratch);                                    \
		*value = ((guchar *)*value) + sizeof (corbatype);	\
		break;

#define UNPRINTABLE "XXX"

static void
encode_value (char             **buffer,
	      CORBA_TypeCode     tc,
	      gpointer          *value,
	      int                maxlen)
{
	char scratch [256] = "";
	int  i, l;

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		break;

		ENCODE_TO_STRING (CORBA_tk_short,  "%d", CORBA_short, value, 
				  ALIGNOF_CORBA_SHORT);
		ENCODE_TO_STRING (CORBA_tk_ushort, "%u", CORBA_unsigned_short,
				  value, ALIGNOF_CORBA_SHORT);
		ENCODE_TO_STRING (CORBA_tk_long, "%d", CORBA_long, value, 
				  ALIGNOF_CORBA_LONG);
		ENCODE_TO_STRING (CORBA_tk_ulong,  "%u", CORBA_unsigned_long, 
				  value, ALIGNOF_CORBA_LONG);
		ENCODE_TO_STRING (CORBA_tk_float, "%g", CORBA_float, value, 
				  ALIGNOF_CORBA_FLOAT);
		ENCODE_TO_STRING (CORBA_tk_double, "%g", CORBA_double, value, 
				  ALIGNOF_CORBA_DOUBLE);
		ENCODE_TO_STRING (CORBA_tk_boolean, "%d", CORBA_boolean, 
				  value, 1);
		ENCODE_TO_STRING (CORBA_tk_char, "%d", CORBA_char, value, 1);

		ENCODE_TO_STRING (CORBA_tk_octet, "%d", CORBA_octet, value, 1);

		ENCODE_TO_STRING (CORBA_tk_wchar, "%d", CORBA_wchar, value, 
				  ALIGNOF_CORBA_SHORT);

	case CORBA_tk_enum:
		*value = ALIGN_ADDRESS (*value,  ALIGNOF_CORBA_LONG);
		ADD_STRING (tc->subnames[*(CORBA_long *)*value]);
		*value = ((guchar *)*value) + sizeof(CORBA_long);
		break;
	case CORBA_tk_string:
	case CORBA_tk_wstring:
		*value = ALIGN_ADDRESS(*value, ALIGNOF_CORBA_POINTER);
		ADD_STRING ("\"");
		l = strlen (*(CORBA_char **) *value);
		memcpy (*buffer, *(CORBA_char **) *value, l + 1);
		*buffer += l;
		ADD_STRING ("\"");
		*value = ((guchar *)*value) + sizeof (CORBA_char *);
		break;

	case CORBA_tk_sequence: {
		CORBA_sequence_octet *sval;
		gpointer subval;

		*value = ALIGN_ADDRESS (*value, MAX(MAX(ALIGNOF_CORBA_LONG, 
							ALIGNOF_CORBA_STRUCT),
						    ALIGNOF_CORBA_POINTER));

		sval = *value;
		subval = sval->_buffer;

		ADD_STRING ("(");
		for (i = 0; i < sval->_length; i++) {
			if (i)
				ADD_STRING (", ");
			encode_value (buffer, tc->subtypes [0], &subval, 0);
		}
		ADD_STRING (")");

		*value = ((guchar *)*value) + sizeof (CORBA_sequence_octet);
		break;
	}

	case CORBA_tk_array:
		ADD_STRING ("[");
		for (i = 0; i < tc->length; i++) {
			if (i)
				ADD_STRING (", ");
			encode_value (buffer, tc->subtypes [0], value, 0);
		}
		ADD_STRING ("]");
		break;

 	case CORBA_tk_struct:
	case CORBA_tk_except:
		ADD_STRING ("{");
		for (i = 0; i < tc->sub_parts; i++) {
			if (i)
				ADD_STRING (", ");
			encode_value (buffer, tc->subtypes [i], value, 0);
		}
		ADD_STRING ("}");
		break;

	case CORBA_tk_alias:
		encode_value (buffer, tc->subtypes [0], value, 0);
		break;

	case CORBA_tk_union:
	case CORBA_tk_Principal:
	case CORBA_tk_fixed:
	case CORBA_tk_recursive:
	case CORBA_tk_objref:
	case CORBA_tk_TypeCode:
	case CORBA_tk_any:
	default:
		l = strlen (UNPRINTABLE);                               
                memcpy (*buffer, UNPRINTABLE, l + 1);                   
                *buffer += l; 
		break;
	}
}

/**
 * bonobo_config_any_to_string:
 * @any: the Any to encode
 *
 * Converts the @any into a string representation.
 *
 * Returns: a string representation of the any.
 */
char *
bonobo_config_any_to_string (CORBA_any *any)
{
	char buffer[4096];
	char *p = buffer;
	gpointer v;

	v = any->_value;

	encode_value (&p, any->_type, &v, 4096);

	return g_strdup (buffer);
}

static CORBA_TypeCode
string_to_type_code (const char *k)
{
	if (!strcmp (k, "short"))
		return TC_short;

	if (!strcmp (k, "ushort"))
		return TC_ushort;

	if (!strcmp (k, "long"))
		return TC_long;

	if (!strcmp (k, "ulong"))
		return TC_ulong;

	if (!strcmp (k, "float"))
		return TC_float;

	if (!strcmp (k, "double"))
		return TC_double;

	if (!strcmp (k, "boolean"))
		return TC_boolean;

	if (!strcmp (k, "char"))
		return TC_char;

	if (!strcmp (k, "string"))
		return TC_string;

	return TC_null;
}

static char *
is_simple (CORBA_TCKind my_kind)
{
	switch (my_kind) {

	case CORBA_tk_short:   return "short";
	case CORBA_tk_ushort:  return "ushort";
	case CORBA_tk_long:    return "long";
	case CORBA_tk_ulong:   return "ulong";
	case CORBA_tk_float:   return "float";
	case CORBA_tk_double:  return "double";
	case CORBA_tk_boolean: return "boolean";
	case CORBA_tk_char:    return "char";
	case CORBA_tk_string:  return "string";
				  
	default:               return NULL;
	}
}

#define DO_ENCODE(tckind,format,corbatype,value)			\
	case tckind:							\
		snprintf (scratch, 127, format,				\
			  * (corbatype *) value);			\
		bonobo_ui_node_set_attr (node, "value", scratch);       \
		break;

static void
encode_simple_value (BonoboUINode      *node,
		     const CORBA_any   *any,
		     CORBA_Environment *ev)
{
	gpointer v = any->_value;
	char scratch [256] = "";
	char *string;
	
	switch (any->_type->kind) {
	case CORBA_tk_string:
		string = bonobo_ui_util_encode_str (*(char **) v);
		bonobo_ui_node_set_attr (node, "value", string);
		g_free (string);
		break;

		DO_ENCODE (CORBA_tk_short, "%d", CORBA_short, v);
		DO_ENCODE (CORBA_tk_ushort, "%u", CORBA_unsigned_short, v); 
		DO_ENCODE (CORBA_tk_long, "%d", CORBA_long, v);
		DO_ENCODE (CORBA_tk_ulong, "%u", CORBA_unsigned_long, v);
		DO_ENCODE (CORBA_tk_float, "%g", CORBA_float, v);
		DO_ENCODE (CORBA_tk_double, "%g", CORBA_double,  v);
		DO_ENCODE (CORBA_tk_boolean, "%d", CORBA_boolean, v);
		DO_ENCODE (CORBA_tk_char, "%d", CORBA_char, v);
	default:
		g_warning ("unhandled enumeration value");
	}
}

/**
 * bonobo_config_xml_encode_any:
 * @any: the Any to encode
 * @name: the name of the entry
 * @ev: a corba exception environment
 *
 * This routine encodes @any into a XML tree. 
 * 
 * Return value: the XML tree representing the Any
 */
BonoboUINode *
bonobo_config_xml_encode_any (const CORBA_any   *any,
			      const char        *name,
			      CORBA_Environment *ev)
{
	BonoboUINode *node;
	char *kind;

	g_return_val_if_fail (any != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (ev != NULL, NULL);

	node = bonobo_ui_node_new ("entry");

	bonobo_ui_node_set_attr (node, "name", name);

	if ((kind = is_simple (any->_type->kind))) {
		bonobo_ui_node_set_attr (node, "type", kind);
		encode_simple_value (node, any, ev);
		return node;
	}

	bonobo_property_bag_xml_encode_any (node, any, ev);

	return node;
}

static char *
get_value_with_locale (BonoboUINode *node,
		       const char   *locale)
{
	BonoboUINode *child;
	char *v = NULL;
	char *l;

	child = bonobo_ui_node_children (node);

	while (child) {
		if (!strcmp (bonobo_ui_node_get_name (child), "value")) {
			l = bonobo_ui_node_get_attr (child, "xml:lang");

			if (!l && !v)
				v = bonobo_ui_node_get_content (child);
			
			if (l && locale && !strcmp (locale, l)) {
				if (v)
					bonobo_ui_node_free_string (v);
				
				v = bonobo_ui_node_get_content (child);

				break;
			}
		}

		child = bonobo_ui_node_next (child);
	}

	return v;
}

#define DO_DECODE(tckind,format,corbatype,value)			     \
	case tckind: {							     \
		if (sscanf (v, format, (corbatype *) (value->_value)) != 1)  \
			g_warning ("Broken scanf on '%s'", v);	             \
		break;							     \
	}

#define DO_DECODEI(tckind,format,corbatype,value)			     \
	case tckind: {							     \
		 CORBA_unsigned_long i;			                     \
		 if (sscanf (v, format, &i) != 1)                            \
			g_warning ("Broken scanf on '%s'", v);	             \
		  *(corbatype *) value->_value = i;     	             \
		  break;						     \
	}

static CORBA_any *
decode_simple_value (char *kind, char *v)
{
	CORBA_any *value;
	char *string;
	gboolean err;
	
	CORBA_TypeCode tc;
	
	if (!(tc = string_to_type_code (kind)))
		return NULL;
	
	value = bonobo_arg_new (tc);
	
	switch (tc->kind) {
	case CORBA_tk_string:
		string = bonobo_ui_util_decode_str (v, &err);
		if (err) {
			BONOBO_ARG_SET_STRING (value, v);
		} else {
			BONOBO_ARG_SET_STRING (value, string);
			g_free (string);
		}
		break;
		
		DO_DECODEI (CORBA_tk_short, "%d", CORBA_short, value); 
		DO_DECODEI (CORBA_tk_ushort, "%u",CORBA_unsigned_short, value);
		DO_DECODEI (CORBA_tk_long, "%d", CORBA_long, value); 
		DO_DECODEI (CORBA_tk_ulong, "%u", CORBA_unsigned_long, value); 
		DO_DECODE (CORBA_tk_float, "%g", CORBA_float, value);
		DO_DECODE (CORBA_tk_double, "%lg", CORBA_double, value);
		DO_DECODEI (CORBA_tk_boolean, "%d", CORBA_boolean, value);
		DO_DECODEI (CORBA_tk_char, "%d", CORBA_char, value);
	default:
		break;
	}

	return value;
}

/**
 * bonobo_config_xml_decode_any:
 * @node: the parsed XML representation of an any
 * @locale: an optional locale
 * @ev: a corba exception environment
 *
 * This routine is the converse of #bonobo_config_xml_encode_any.
 * It hydrates a serialized CORBA_any.
 * 
 * Return value: the CORBA_any or NULL on error
 */
CORBA_any *
bonobo_config_xml_decode_any (BonoboUINode      *node,
			      const char        *locale, 
			      CORBA_Environment *ev)
{
	CORBA_any *value = NULL;
	char *kind, *tmp;
	BonoboUINode *child;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (ev != NULL, NULL);

	if (strcmp (bonobo_ui_node_get_name (node), "entry"))
		return NULL;

	child = bonobo_ui_node_children (node);

	if (child && bonobo_ui_node_has_name (child, "any")) 
		return bonobo_property_bag_xml_decode_any (child, ev);
	
	if (!(kind = bonobo_ui_node_get_attr (node, "type")))
		return NULL;

	tmp = bonobo_ui_node_get_attr (node, "value");

	if (!tmp)
		tmp = get_value_with_locale (node, locale);

	if (tmp) {
		value = decode_simple_value (kind, tmp);
		bonobo_ui_node_free_string (tmp);
	}

	bonobo_ui_node_free_string (kind);
	return value;
}

