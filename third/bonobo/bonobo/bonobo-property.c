/**
 * Bonobo property object implementation.
 *
 * Author:
 *    Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag.h>

typedef struct {
	POA_Bonobo_Property		 prop;
	BonoboPropertyBag		*pb;
	char				*property_name;
} BonoboPropertyServant;

static CORBA_char *
impl_Bonobo_Property_getName (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	return CORBA_string_dup (pservant->property_name);
}

static CORBA_TypeCode
impl_Bonobo_Property_getType (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;
	BonoboArgType          type;

	type = bonobo_property_bag_get_type (pservant->pb, pservant->property_name);
	/* FIXME: we need to handle obscure cases like non existance of the property */

	return (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) (type), ev);
}

static CORBA_any *
impl_Bonobo_Property_getValue (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_value (pservant->pb, pservant->property_name);
}

static void
impl_Bonobo_Property_setValue (PortableServer_Servant servant,
				const CORBA_any       *any,
				CORBA_Environment     *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	bonobo_property_bag_set_value (pservant->pb, pservant->property_name, any, ev);
}

static CORBA_any *
impl_Bonobo_Property_getDefault (PortableServer_Servant servant,
				  CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_default (pservant->pb, pservant->property_name);
}

static CORBA_char *
impl_Bonobo_Property_getDocString (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	return CORBA_string_dup (bonobo_property_bag_get_docstring (pservant->pb, pservant->property_name));
}


static CORBA_long
impl_Bonobo_Property_getFlags (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	BonoboPropertyServant *pservant = (BonoboPropertyServant *) servant;

	return bonobo_property_bag_get_flags (pservant->pb, pservant->property_name);
}

static POA_Bonobo_Property__epv *
bonobo_property_get_epv (void)
{
	static POA_Bonobo_Property__epv *epv = NULL;

	if (epv != NULL)
		return epv;

	epv = g_new0 (POA_Bonobo_Property__epv, 1);

	epv->getName      = impl_Bonobo_Property_getName;
	epv->getType      = impl_Bonobo_Property_getType;
	epv->getValue     = impl_Bonobo_Property_getValue;
	epv->setValue     = impl_Bonobo_Property_setValue;
	epv->getDefault   = impl_Bonobo_Property_getDefault;
	epv->getDocString = impl_Bonobo_Property_getDocString;
	epv->getFlags     = impl_Bonobo_Property_getFlags;

	return epv;
}

static POA_Bonobo_Property__vepv *
bonobo_property_get_vepv (void)
{
	static POA_Bonobo_Property__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_Bonobo_Property__vepv, 1);

	vepv->Bonobo_Property_epv = bonobo_property_get_epv ();

	return vepv;
}

PortableServer_Servant
bonobo_property_servant_new (PortableServer_POA     poa,
			     BonoboTransient        *bt,
			     char                   *property_name,
			     void                   *callback_data)
{
	BonoboPropertyServant	*servant;
	BonoboPropertyBag       *pb = (BonoboPropertyBag *)callback_data;
	CORBA_Environment        ev;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (property_name != NULL, NULL);

	/*
	 * Verify that the specified property exists.
	 */
	if (! bonobo_property_bag_has_property (pb, property_name))
		return CORBA_OBJECT_NIL;

	CORBA_exception_init (&ev);

	/*
	 * Create a transient servant for the property.
	 */
	servant = g_new0 (BonoboPropertyServant, 1);

	servant->property_name = g_strdup (property_name);
	servant->pb = pb;

	((POA_Bonobo_Property *) servant)->vepv = bonobo_property_get_vepv ();
	
	POA_Bonobo_Property__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboProperty: Could not initialize Property servant");
		g_free (servant->property_name);
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return servant;
}

void
bonobo_property_servant_destroy (PortableServer_Servant  servant,
				 void                   *callback_data)
{
	CORBA_Environment ev;

	g_return_if_fail (servant != NULL);

	CORBA_exception_init (&ev);

	POA_Bonobo_Property__fini ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("BonoboProperty: Could not deconstruct Property servant");
		CORBA_exception_free (&ev);
		return;
	}

	CORBA_exception_free (&ev);

	g_free (((BonoboPropertyServant *) servant)->property_name);
	g_free (servant);
}
