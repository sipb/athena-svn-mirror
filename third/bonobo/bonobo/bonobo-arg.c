/*
 * bonobo-arg.c Bonobo argument support:
 *
 *  A thin wrapper of CORBA_any's with macros
 * to assist in handling values safely.
 *
 * Author:
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-arg.h>

BonoboArg *
bonobo_arg_new (BonoboArgType t)
{
	CORBA_any *any = NULL;
	DynamicAny_DynAny dyn = NULL;
	CORBA_ORB orb;
	CORBA_Environment ev;

	g_return_val_if_fail (t != NULL, NULL);

	CORBA_exception_init (&ev);
	
	orb = bonobo_orb ();

	switch (t->kind) {

	case CORBA_tk_null: 
	case CORBA_tk_void: 
	case CORBA_tk_short:
	case CORBA_tk_long:
	case CORBA_tk_ushort:
	case CORBA_tk_ulong:
	case CORBA_tk_float:
	case CORBA_tk_double:
	case CORBA_tk_boolean:
	case CORBA_tk_char:
	case CORBA_tk_octet:
	case CORBA_tk_string:
	case CORBA_tk_longlong:
	case CORBA_tk_ulonglong:
	case CORBA_tk_longdouble:
	case CORBA_tk_wchar:
	case CORBA_tk_wstring:
	case CORBA_tk_objref:
	case CORBA_tk_any:
	case CORBA_tk_TypeCode:
	case CORBA_tk_Principal:

		dyn = CORBA_ORB_create_basic_dyn_any (orb, t, &ev);
		break;

 	case CORBA_tk_sequence:

		dyn = CORBA_ORB_create_dyn_sequence (orb, t, &ev);
		break;

	case CORBA_tk_array:

		dyn = CORBA_ORB_create_dyn_array (orb, t, &ev);
		break;
		
	case CORBA_tk_except:
	case CORBA_tk_struct:

		dyn = CORBA_ORB_create_dyn_struct (orb, t, &ev);
		break;

	case CORBA_tk_union:

		dyn = CORBA_ORB_create_dyn_union (orb, t, &ev);
		break;
       
	case CORBA_tk_enum:

		dyn = CORBA_ORB_create_dyn_enum (orb, t, &ev);
		break;
       
	default:

		g_warning ("Unhandled typecode");
		break;
	}

	if (!BONOBO_EX (&ev) && dyn != NULL) {
		any = DynamicAny_DynAny_to_any (dyn, &ev);
		DynamicAny_DynAny_destroy (dyn, &ev);
		CORBA_Object_release ((CORBA_Object) dyn, &ev);
	}

	CORBA_exception_free (&ev);

	return any;
}

void
bonobo_arg_release (BonoboArg *a)
{
	if (a)
		CORBA_free (a);	
}

BonoboArg *
bonobo_arg_copy (BonoboArg *a)
{
	BonoboArg *copy = CORBA_any__alloc ();

	if (!a) {
		copy->_type = TC_null;
		g_warning ("Duplicating a NULL Bonobo Arg");
	} else
		CORBA_any__copy (copy, a);

	return copy;
}

BonoboArgType
bonobo_arg_type_from_gtk (GtkType id)
{
	switch (id) {
	case GTK_TYPE_CHAR:   return TC_char;
	case GTK_TYPE_UCHAR:  return TC_char;
	case GTK_TYPE_BOOL:   return TC_boolean;
	case GTK_TYPE_INT:    return TC_short;
	case GTK_TYPE_UINT:   return TC_ushort;
	case GTK_TYPE_LONG:   return TC_long;
	case GTK_TYPE_ULONG:  return TC_ulong;
	case GTK_TYPE_FLOAT:  return TC_float;
	case GTK_TYPE_DOUBLE: return TC_double;
	case GTK_TYPE_STRING: return TC_string;
	default:
		g_warning ("Unmapped arg type '%d'", id);
		break;
	}

	return NULL;
}

GtkType
bonobo_arg_type_to_gtk (BonoboArgType id)
{
	CORBA_Environment ev;
	GtkType gtk_type = GTK_TYPE_NONE;

	CORBA_exception_init (&ev);

	if (bonobo_arg_type_is_equal (TC_char, id, &ev))
		gtk_type = GTK_TYPE_CHAR;
	else if (bonobo_arg_type_is_equal (TC_boolean, id, &ev))
		gtk_type = GTK_TYPE_BOOL;
	else if (bonobo_arg_type_is_equal (TC_short,   id, &ev))
		gtk_type = GTK_TYPE_INT;
	else if (bonobo_arg_type_is_equal (TC_ushort,  id, &ev))
		gtk_type = GTK_TYPE_UINT;
	else if (bonobo_arg_type_is_equal (TC_long,    id, &ev))
		gtk_type = GTK_TYPE_LONG;
	else if (bonobo_arg_type_is_equal (TC_ulong,   id, &ev))
		gtk_type = GTK_TYPE_ULONG;
	else if (bonobo_arg_type_is_equal (TC_float,   id, &ev))
		gtk_type = GTK_TYPE_FLOAT;
	else if (bonobo_arg_type_is_equal (TC_double,  id, &ev))
		gtk_type = GTK_TYPE_DOUBLE;
	else if (bonobo_arg_type_is_equal (TC_string,  id, &ev))
		gtk_type = GTK_TYPE_STRING;
	else
		g_warning ("Unmapped bonobo arg type");

	CORBA_exception_free (&ev);

	return gtk_type;
}

#define MAKE_FROM_GTK_CASE(gtktype,tcid,unionid,corbatype,corbakind)			\
	case GTK_TYPE_##gtktype:							\
		*((corbatype *)a->_value) = (corbatype)GTK_VALUE_##gtktype(*arg);	\
		break;

void
bonobo_arg_from_gtk (BonoboArg *a, const GtkArg *arg)
{
	int        id;

	g_return_if_fail (a != NULL);
	g_return_if_fail (arg != NULL);

	id = arg->type;

	switch (id) {

	case GTK_TYPE_INVALID:
	case GTK_TYPE_NONE:
		g_warning ("Strange gtk arg type %d", id);
		break;
		
		MAKE_FROM_GTK_CASE (CHAR,   TC_char,     char_data, CORBA_char,           CORBA_tk_char);
		MAKE_FROM_GTK_CASE (UCHAR,  TC_char,    uchar_data, CORBA_char,           CORBA_tk_char);
		MAKE_FROM_GTK_CASE (BOOL,   TC_boolean,  bool_data, CORBA_boolean,        CORBA_tk_boolean);
		MAKE_FROM_GTK_CASE (INT,    TC_short,     int_data, CORBA_short,          CORBA_tk_short);
		MAKE_FROM_GTK_CASE (UINT,   TC_ushort,   uint_data, CORBA_unsigned_short, CORBA_tk_ushort);
		MAKE_FROM_GTK_CASE (LONG,   TC_long,     long_data, CORBA_long,           CORBA_tk_long);
		MAKE_FROM_GTK_CASE (ULONG,  TC_ulong,   ulong_data, CORBA_unsigned_long,  CORBA_tk_ulong);
		MAKE_FROM_GTK_CASE (FLOAT,  TC_float,   float_data, CORBA_float,          CORBA_tk_float);
		MAKE_FROM_GTK_CASE (DOUBLE, TC_double, double_data, CORBA_double,         CORBA_tk_double);

	case GTK_TYPE_STRING:
		/* Orbit really doesn't like NULL string's in anys: why ? ... */
		if (GTK_VALUE_STRING (*arg))
			*((CORBA_char **)a->_value) = CORBA_string_dup (GTK_VALUE_STRING (*arg));
		else
			*((CORBA_char **)a->_value) = CORBA_string_dup ("");
		break;

	case GTK_TYPE_POINTER:
		g_warning ("FIXME: we can map user data callbacks locally");
		break;

	case GTK_TYPE_OBJECT:
		g_warning ("All objects can be mapped to base gtk types"
			   "in due course");
		break;

	case GTK_TYPE_SIGNAL:
	case GTK_TYPE_ARGS:
	case GTK_TYPE_CALLBACK:
	case GTK_TYPE_C_CALLBACK:
		g_warning ("Clever things can be done for these");
		break;

	case GTK_TYPE_ENUM:
	case GTK_TYPE_FLAGS:
	case GTK_TYPE_BOXED:
	case GTK_TYPE_FOREIGN:
	default:
		g_warning ("Unmapped gtk arg type %d", id);
		break;
	}
}

#define MAKE_TO_GTK_CASE(gtktype,tcid,unionid,corbatype,corbakind)		\
	case corbakind:								\
		GTK_VALUE_##gtktype(*a) = *((corbatype *)arg->_value);		\
		break;

void
bonobo_arg_to_gtk (GtkArg *a, const BonoboArg *arg)
{
	int     id;

	g_return_if_fail (a != NULL);
	g_return_if_fail (arg != NULL);
	g_return_if_fail (arg->_type != NULL);

	id = arg->_type->kind;
	switch (id) {

	case CORBA_tk_null:
	case CORBA_tk_void:
		g_warning ("Strange corba arg type %d", id);
		break;
		
		MAKE_TO_GTK_CASE (CHAR,   TC_char,     char_data, CORBA_char,           CORBA_tk_char);
/*		MAKE_TO_GTK_CASE (UCHAR,  TC_char,    uchar_data, CORBA_char,           CORBA_tk_char);*/
		MAKE_TO_GTK_CASE (BOOL,   TC_boolean,  bool_data, CORBA_boolean,        CORBA_tk_boolean);
		MAKE_TO_GTK_CASE (INT,    TC_short,     int_data, CORBA_short,          CORBA_tk_short);
		MAKE_TO_GTK_CASE (UINT,   TC_ushort,   uint_data, CORBA_unsigned_short, CORBA_tk_ushort);
		MAKE_TO_GTK_CASE (LONG,   TC_long,     long_data, CORBA_long,           CORBA_tk_long);
		MAKE_TO_GTK_CASE (ULONG,  TC_ulong,   ulong_data, CORBA_unsigned_long,  CORBA_tk_ulong);
		MAKE_TO_GTK_CASE (FLOAT,  TC_float,   float_data, CORBA_float,          CORBA_tk_float);
		MAKE_TO_GTK_CASE (DOUBLE, TC_double, double_data, CORBA_double,         CORBA_tk_double);

	case CORBA_tk_string:
		GTK_VALUE_STRING (*a) = g_strdup (BONOBO_ARG_GET_STRING (arg));
		break;

	case CORBA_tk_objref:
		g_warning ("All objects can be mapped to base gtk types"
			   "in due course");
		break;

	case CORBA_tk_sequence:
	case CORBA_tk_alias:
	case CORBA_tk_enum:
	case CORBA_tk_array:
	case CORBA_tk_union:
		g_warning ("Clever things can be done for these");
		break;

	default:
		g_warning ("Unmapped corba arg type %d", id);
		break;
	}
}

gboolean
bonobo_arg_type_is_equal (BonoboArgType a, BonoboArgType b, CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	gboolean retval;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	retval = CORBA_TypeCode_equal (a, b, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

gboolean
bonobo_arg_is_equal (BonoboArg *a, BonoboArg *b, CORBA_Environment *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	gboolean retval;

	if (!opt_ev) {

		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	retval = ORBit_any_equivalent (a, b, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}
