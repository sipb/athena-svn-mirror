/**
 * bonobo-property-bag-client.c: C sugar for property bags.
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <stdarg.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-client.h>

/**
 * bonobo_property_bag_client_get_properties:
 * @pb: A #Bonobo_PropertyBag      which is bound to a remote
 * #Bonobo_PropertyBag.
 *
 * Returns: A #GList filled with #Bonobo_Property CORBA object
 * references for all of the properties stored in the remote
 * #BonoboPropertyBag.
 */
GList *
bonobo_property_bag_client_get_properties (Bonobo_PropertyBag       pb,
					   CORBA_Environment       *ev)
{
	Bonobo_PropertyList  *props;
	GList		    *prop_list;
	int		     i;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	props = Bonobo_PropertyBag_getProperties (pb, real_ev);
	if (BONOBO_EX (real_ev)) {
		if (!ev)
			CORBA_exception_free (&tmp_ev);
		return NULL;
	}

	prop_list = NULL;
	for (i = 0; i < props->_length; i ++) {

		/*
		 * FIXME: Is it necessary to duplicate these?  I'm
		 * inclined to think that it isn't.
		 */
		prop_list = g_list_prepend (
			prop_list,
			CORBA_Object_duplicate (props->_buffer [i], real_ev));

		if (BONOBO_EX (real_ev)) {
			CORBA_Environment ev2;
			GList *curr;

			CORBA_exception_init (&ev2);

			for (curr = prop_list; curr != NULL; curr = curr->next) {
				CORBA_Object_release ((CORBA_Object) curr->data, &ev2);
				CORBA_exception_free (&ev2);
			}

			g_list_free (prop_list);

			if (!ev)
				CORBA_exception_free (&tmp_ev);
			return NULL;
		}
	}

	CORBA_free (props);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return prop_list;
}

/**
 * bonobo_property_bag_client_free_properties:
 * @list: A #GList containing Bonobo_Property corba objrefs (as
 * produced by bonobo_property_bag_client_get_properties(), for
 * example).
 *
 * Releases the CORBA Objrefs stored in @list and frees the list.
 */
void
bonobo_property_bag_client_free_properties (GList *list)
{
	GList *l;

	if (list == NULL)
		return;

	for (l = list; l != NULL; l = l->next) {
		CORBA_Environment ev;
		Bonobo_Property    prop;

		prop = (Bonobo_Property) l->data;

		CORBA_exception_init (&ev);

		CORBA_Object_release (prop, &ev);

		if (BONOBO_EX (&ev)) {
			g_warning ("bonobo_property_bag_client_free_properties: Exception releasing objref!");
			CORBA_exception_free (&ev);
			return;
		}

		CORBA_exception_free (&ev);
	}

	g_list_free (list);
}

/**
 * bonobo_property_bag_client_get_property_names:
 * @pb: A #Bonobo_PropertyBag      which is bound to a remote
 * #Bonobo_PropertyBag.
 *
 * This function exists as a convenience, so that you don't have to
 * iterate through all of the #Bonobo_Property objects in order to get
 * a list of their names.  It should be used in place of such an
 * iteration, as it uses fewer resources on the remote
 * #BonoboPropertyBag.
 *
 * Returns: A #GList filled with strings containing the names of all
 * the properties stored in the remote #BonoboPropertyBag.
 */
GList *
bonobo_property_bag_client_get_property_names (Bonobo_PropertyBag       pb,
					       CORBA_Environment       *ev)
{
	Bonobo_PropertyNames  *names;
	GList		     *name_list;
	int		      i;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	names = Bonobo_PropertyBag_getPropertyNames (pb, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			CORBA_exception_free (&tmp_ev);

		return NULL;
	}
	
	name_list = NULL;
	for (i = 0; i < names->_length; i ++) {
		 char *name;

		 name = g_strdup (names->_buffer [i]);
		 name_list = g_list_prepend (name_list, name);
	}

	CORBA_free (names);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return name_list;
}

/**
 * bonobo_property_bag_client_get_property:
 * @pb: A Bonobo_PropertyBag      which is associated with a remote
 * BonoboPropertyBag.
 * @name: A string containing the name of the property which is to
 * be fetched.
 *
 * Returns: A #Bonobo_Property CORBA object reference for the
 *
 */
Bonobo_Property
bonobo_property_bag_client_get_property (Bonobo_PropertyBag       pb,
					 const char              *property_name,
					 CORBA_Environment       *ev)
{
	Bonobo_Property prop;
	CORBA_Environment *real_ev, tmp_ev;

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);

	prop = Bonobo_PropertyBag_getPropertyByName (pb, property_name, ev);

	if (BONOBO_EX (real_ev))
		prop = CORBA_OBJECT_NIL;

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return prop;
}


/*
 * Bonobo Property streaming functions.
 */

/**
 * bonobo_property_bag_client_persist:
 * @pb: A #Bonobo_PropertyBag      object which is bound to a remote
 * #Bonobo_PropertyBag server.
 * @stream: A #BonoboStream into which the data in @pb will be written.
 *
 * Reads the property data stored in the #Bonobo_Property_bag to which
 * @pb is bound and streams it into @stream.  The typical use for
 * this function is to save the property data for a given Bonobo
 * Control into a persistent store to which @stream is attached.
 */
void
bonobo_property_bag_client_persist (Bonobo_PropertyBag       pb,
				    Bonobo_Stream            stream,
				    CORBA_Environment       *ev)
{
	Bonobo_PersistStream persist;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);
	g_return_if_fail (stream != NULL);
	g_return_if_fail (BONOBO_IS_STREAM (stream));

	persist = Bonobo_Unknown_queryInterface (pb, "IDL:Bonobo/PersistStream:1.0", ev);

	if (BONOBO_EX (ev) ||
	    persist   == CORBA_OBJECT_NIL) {
		g_warning ("Bonobo_PropertyBag     : No PersistStream interface "
			   "found on remote PropertyBag!");
		return;
	}

	Bonobo_PersistStream_save (persist, stream, "", ev);

	if (BONOBO_EX (ev)) {
		g_warning ("Bonobo_PropertyBag     : Exception caught while persisting "
			   "remote PropertyBag!");
		return;
	}

	bonobo_object_release_unref (persist, ev);
}

/**
 * bonobo_property_bag_client_depersist:
 */
void
bonobo_property_bag_client_depersist (Bonobo_PropertyBag       pb,
				      Bonobo_Stream            stream,
				      CORBA_Environment       *ev)
{
	Bonobo_PersistStream persist;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);
	g_return_if_fail (stream != NULL);
	g_return_if_fail (BONOBO_IS_STREAM (stream));

	persist = Bonobo_Unknown_queryInterface (
		pb, "IDL:Bonobo/PersistStream:1.0", ev);

	if (BONOBO_EX (ev) ||
	    persist    == CORBA_OBJECT_NIL) {
		g_warning ("Bonobo_PropertyBag     : No PersistStream interface "
			   "found on remote PropertyBag!");
		return;
	}

	Bonobo_PersistStream_load (persist, stream, "", ev);

	if (BONOBO_EX (ev)) {
		g_warning ("Bonobo_PropertyBag     : Exception caught while persisting "
			   "remote PropertyBag!");
		return;
	}

	bonobo_object_release_unref (persist, ev);
}


/*
 * Property convenience functions.
 */

/**
 * bonobo_property_bag_client_get_property_type:
 */
CORBA_TypeCode
bonobo_property_bag_client_get_property_type (Bonobo_PropertyBag       pb,
					      const char              *propname,
					      CORBA_Environment       *ev)
{
	Bonobo_Property prop;
	CORBA_TypeCode  tc;
	CORBA_Environment *real_ev, tmp_ev;

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	g_return_val_if_fail (propname != NULL, (CORBA_TypeCode) TC_null);
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, (CORBA_TypeCode) TC_null);

	prop = bonobo_property_bag_client_get_property (pb, propname, real_ev);

	if (prop == CORBA_OBJECT_NIL) {
		if (!ev) {
			g_warning ("prop is NIL");
			CORBA_exception_free (&tmp_ev);
		}
	}

	tc = Bonobo_Property_getType (prop, real_ev);

	if (BONOBO_EX (real_ev)) {
		g_warning ("bonobo_property_bag_client_get_property_type: Exception getting TypeCode!");

		CORBA_Object_release (prop, real_ev);

		if (!ev)
			CORBA_exception_free (&tmp_ev);

		return (CORBA_TypeCode) TC_null;
	}

	CORBA_Object_release (prop, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return tc;
}

typedef enum {
	FIELD_VALUE,
	FIELD_DEFAULT
} PropUtilFieldType;

static BonoboArg *
bonobo_property_bag_client_get_field_any (Bonobo_PropertyBag       pb,
					  const char              *propname,
					  PropUtilFieldType        field,
					  CORBA_Environment       *ev)

{
	Bonobo_Property prop;
	CORBA_any      *any;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (propname != NULL, NULL);
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	prop = bonobo_property_bag_client_get_property (pb, propname, real_ev);

	if (prop == CORBA_OBJECT_NIL) {
		if (!ev) {
			g_warning ("prop == NIL");
			CORBA_exception_free (&tmp_ev);
		}
		return NULL;
	}

	if (field == FIELD_VALUE)
		any = Bonobo_Property_getValue (prop, real_ev);
	else
		any = Bonobo_Property_getDefault (prop, real_ev);

	if (BONOBO_EX (real_ev)) {
		g_warning ("bonobo_property_bag_client_get_field_any: Exception getting property value!");
		CORBA_Object_release (prop, real_ev);
		if (!ev)
			CORBA_exception_free (&tmp_ev);

		return NULL;
	}

	CORBA_Object_release (prop, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return any;
}

#define MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD(type,def,corbatype,tk)	\
static type									\
bonobo_property_bag_client_get_field_##type (Bonobo_PropertyBag       pb,	\
					     const char              *propname,	\
					     PropUtilFieldType        field,	\
					     CORBA_Environment       *ev)	\
{										\
	CORBA_any *any;								\
	type       d;								\
										\
	g_return_val_if_fail (pb != NULL, (def));				\
	g_return_val_if_fail (propname != NULL, (def));				\
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, (def));		      	\
										\
	any = bonobo_property_bag_client_get_field_any (			\
		pb, propname, field, ev);					\
										\
	if (any == NULL)							\
		return 0.0;							\
										\
	g_return_val_if_fail (any->_type->kind == tk, (def));			\
										\
	d = *(corbatype *) any->_value;						\
										\
	CORBA_any__free (any, NULL, TRUE);					\
										\
	return d;								\
}

MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD (gboolean,  0, CORBA_boolean,        CORBA_tk_boolean);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD (gint  ,    0, CORBA_long,           CORBA_tk_long);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD (glong,     0, CORBA_long,           CORBA_tk_long);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD (gfloat,  0.0, CORBA_float,          CORBA_tk_float);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_GET_FIELD (gdouble, 0.0, CORBA_double,         CORBA_tk_double);

static char *
bonobo_property_bag_client_get_field_string (Bonobo_PropertyBag       pb,
					     const char              *propname,
					     PropUtilFieldType        field,
					     CORBA_Environment       *ev)
{
	CORBA_any *any;
	char      *str;

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (propname != NULL, NULL);

	any = bonobo_property_bag_client_get_field_any (
		pb, propname, field, ev);

	if (any == NULL)
		return NULL;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_string, NULL);

	str = g_strdup (*(char **) any->_value);

	CORBA_any__free (any, NULL, TRUE);

	return str;
}

/*
 *   This macro generates two functions; that to return the value
 * of a property and that to get its default; essentialy these
 * chain on to the shared bonobo_property_bag_client_get_field_gboolean
 * function.
 */
#define MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(type,rettype)					\
rettype												\
bonobo_property_bag_client_get_value_##type (Bonobo_PropertyBag       pb,			\
					     const char              *propname,			\
					     CORBA_Environment       *ev)			\
{												\
	return bonobo_property_bag_client_get_field_##type (pb, propname, FIELD_VALUE, ev);	\
}												\
												\
rettype						      						\
bonobo_property_bag_client_get_default_##type (Bonobo_PropertyBag       pb,			\
					       const char              *propname,		\
					       CORBA_Environment       *ev)			\
{												\
	return bonobo_property_bag_client_get_field_##type (pb, propname, FIELD_DEFAULT, ev);	\
}

MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(gboolean, gboolean);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(gint,     gint);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(glong,    glong);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(gfloat,   gfloat);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(gdouble,  gdouble);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(string,   char *);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_PAIR(any,      BonoboArg *);

/*
 * Setting property values.
 */
void
bonobo_property_bag_client_set_value_any (Bonobo_PropertyBag       pb,
					  const char              *propname,
					  BonoboArg               *value,
					  CORBA_Environment       *ev)
{
	Bonobo_Property   prop;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_if_fail (pb != CORBA_OBJECT_NIL);
	g_return_if_fail (propname != NULL);
	g_return_if_fail (value != NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	prop = bonobo_property_bag_client_get_property (pb, propname, real_ev);
	g_return_if_fail (prop != CORBA_OBJECT_NIL);

	Bonobo_Property_setValue (prop, value, real_ev);

	if (BONOBO_EX (real_ev))
		g_warning ("bonobo_property_bag_client_set_value_any: Exception setting property!");

	CORBA_Object_release (prop, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return;
}

#define MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(gtype,capstype)		\
										\
void										\
bonobo_property_bag_client_set_value_##gtype (Bonobo_PropertyBag       pb,	\
					      const char              *propname,\
					      gtype                    value,	\
					      CORBA_Environment       *ev)	\
{										\
	BonoboArg *arg;								\
										\
	g_return_if_fail (propname != NULL);					\
	g_return_if_fail (pb != CORBA_OBJECT_NIL);				\
										\
	arg = bonobo_arg_new (BONOBO_ARG_##capstype);		      		\
										\
	BONOBO_ARG_SET_##capstype (arg, value);					\
										\
	bonobo_property_bag_client_set_value_any (pb, propname, arg, ev);	\
										\
	bonobo_arg_release (arg);						\
}

MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(gboolean, BOOLEAN);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(gint,     INT);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(glong,    LONG);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(gfloat,   FLOAT);
MAKE_BONOBO_PROPERTY_BAG_CLIENT_SET_VALUE(gdouble,  DOUBLE);

void
bonobo_property_bag_client_set_value_string (Bonobo_PropertyBag       pb,
					     const char              *propname,
					     const gchar             *value,
					     CORBA_Environment       *ev)
{
	BonoboArg *arg;

	g_return_if_fail (propname != NULL);
	g_return_if_fail (pb != CORBA_OBJECT_NIL);

	arg = bonobo_arg_new (BONOBO_ARG_STRING);

	BONOBO_ARG_SET_STRING (arg, value);

	bonobo_property_bag_client_set_value_any (pb, propname, arg, ev);

	bonobo_arg_release (arg);
}

/*
 * Querying other fields and flags.
 */
char *
bonobo_property_bag_client_get_docstring (Bonobo_PropertyBag       pb,
					  const char              *propname,
					  CORBA_Environment       *ev)
{
	Bonobo_Property prop;
	CORBA_char     *docstr;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (propname != NULL, NULL);
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, NULL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	prop = bonobo_property_bag_client_get_property (pb, propname, real_ev);

	if (prop == CORBA_OBJECT_NIL) {
		if (!ev) {
			CORBA_exception_free (&tmp_ev);
			g_warning ("prop == NIL");
		}
		return NULL;
	}

	docstr = Bonobo_Property_getDocString (prop, real_ev);

	if (BONOBO_EX (real_ev)) {
		if (!ev)
			g_warning ("bonobo_property_bag_client_get_doc_string: "
				   "Exception getting doc string!");
		docstr = NULL;
	}

	CORBA_Object_release (prop, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return (char *) docstr;
}

BonoboPropertyFlags
bonobo_property_bag_client_get_flags (Bonobo_PropertyBag       pb,
				      const char              *propname,
				      CORBA_Environment       *ev)
{
	BonoboPropertyFlags flags;
	Bonobo_Property     prop;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, 0);
	g_return_val_if_fail (propname != NULL, 0);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	prop = bonobo_property_bag_client_get_property (pb, propname, real_ev);

	if (prop == CORBA_OBJECT_NIL) {
		if (!ev) {
			CORBA_exception_free (&tmp_ev);
			g_warning ("prop == NIL");
		}
		return 0;
	}

	flags = Bonobo_Property_getFlags (prop, real_ev);

	if (BONOBO_EX (real_ev))
		flags = 0;

	CORBA_Object_release (prop, real_ev);

	if (!ev)
		CORBA_exception_free (&tmp_ev);

	return flags;
}

#define SEND(pb,name,args,corbat,gt,ansip)									\
	case CORBA_tk##corbat:											\
		bonobo_property_bag_client_set_value##gt (pb, name, (CORBA##corbat) va_arg (args, ##ansip), ev);\
		break;

char *
bonobo_property_bag_client_setv (Bonobo_PropertyBag       pb,
				 CORBA_Environment       *ev,
				 const char              *first_arg,
				 va_list                  var_args)
{
	const char *arg_name;

	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, g_strdup ("No property bag"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = bonobo_property_bag_client_get_property_type (pb, arg_name, ev);

		if (type == TC_null)
			return g_strdup_printf ("No such arg '%s'", arg_name);

		switch (type->kind) {
			SEND (pb, arg_name, var_args, _boolean, _gboolean, int);
			SEND (pb, arg_name, var_args, _long,    _glong,    int);
			SEND (pb, arg_name, var_args, _float,   _gfloat,   double);
			SEND (pb, arg_name, var_args, _double,  _gdouble,  double);

		case CORBA_tk_string:
			bonobo_property_bag_client_set_value_string (pb, arg_name,
								     va_arg (var_args, CORBA_char *), ev);
			break;

		case CORBA_tk_any:
			bonobo_property_bag_client_set_value_any    (pb, arg_name,
								     va_arg (var_args, BonoboArg *), ev);
			break;

		default:
			return g_strdup_printf ("Unhandled setv arg '%s' type %d",
						arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);
	}

	return NULL;
}
#undef SEND

#define RECEIVE(pb,name,args,corbat,gt,ansip)					\
	case CORBA_tk##corbat:							\
		*((CORBA##corbat *)va_arg (args, ##ansip *)) =			\
		    bonobo_property_bag_client_get_value##gt (pb, name, ev);	\
		break;

char *
bonobo_property_bag_client_getv (Bonobo_PropertyBag pb,
				 CORBA_Environment *ev,
				 const char        *first_arg,
				 va_list            var_args)
{
	const char *arg_name;

	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));
	g_return_val_if_fail (pb != CORBA_OBJECT_NIL, g_strdup ("No property bag"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = bonobo_property_bag_client_get_property_type (pb, arg_name, ev);

		if (type == TC_null)
			return g_strdup_printf ("No such arg '%s'", arg_name);

		switch (type->kind) {

			RECEIVE (pb, arg_name, var_args, _boolean, _gboolean, int);
			RECEIVE (pb, arg_name, var_args, _long,    _glong,    int);
			RECEIVE (pb, arg_name, var_args, _float,   _gfloat,   double);
			RECEIVE (pb, arg_name, var_args, _double,  _gdouble,  double);

		case CORBA_tk_string:
			*((CORBA_char **)(va_arg (var_args, CORBA_char **))) =
				bonobo_property_bag_client_get_value_string (pb, arg_name, ev);
			break;

		case CORBA_tk_any:
			*((BonoboArg **)(va_arg (var_args, BonoboArg **))) =
				bonobo_property_bag_client_get_value_any (pb, arg_name, ev);
			break;

		default:
			return g_strdup_printf ("Unhandled getv arg '%s' type %d",
						arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);
	}

	return NULL;
}
#undef RECEIVE


