/*
 * bonobo-moniker-xmldb.c: xml database moniker implementation
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-exception.h>

#include "bonobo-config-xmldb.h"
#include "bonobo-config-dirdb.h"

#define EX_SET_NOT_FOUND(ev) bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound)

static Bonobo_Unknown
xmldb_resolve (BonoboMoniker               *moniker,
	       const Bonobo_ResolveOptions *options,
	       const CORBA_char            *requested_interface,
	       CORBA_Environment           *ev)
{
	Bonobo_Moniker         parent;
	Bonobo_ConfigDatabase  db, pdb = CORBA_OBJECT_NIL;
	const gchar           *name;

	if (strcmp (requested_interface, "IDL:Bonobo/ConfigDatabase:1.0")) {
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}

	parent = bonobo_moniker_get_parent (moniker, ev);
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;

	name = bonobo_moniker_get_name (moniker);


	if (parent != CORBA_OBJECT_NIL) {

		pdb = Bonobo_Moniker_resolve (parent, options, 
					      "IDL:Bonobo/ConfigDatabase:1.0", 
					      ev);
    
		bonobo_object_release_unref (parent, NULL);
		
		if (BONOBO_EX (ev) || pdb == CORBA_OBJECT_NIL)
			return CORBA_OBJECT_NIL;

	}

	if (!(db = bonobo_config_xmldb_new (name))) {
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}

	if (pdb != CORBA_OBJECT_NIL) {
		Bonobo_ConfigDatabase_addDatabase (db, pdb, "",
		        Bonobo_ConfigDatabase_DEFAULT, ev);
		
		bonobo_object_release_unref (pdb, NULL);
			
		if (BONOBO_EX (ev)) {
			bonobo_object_release_unref (db, NULL);
			return CORBA_OBJECT_NIL; 
		}
	}

	return db;
}			

static Bonobo_Unknown
dirdb_resolve (BonoboMoniker               *moniker,
	       const Bonobo_ResolveOptions *options,
	       const CORBA_char            *requested_interface,
	       CORBA_Environment           *ev)
{
	Bonobo_Moniker         parent;
	Bonobo_ConfigDatabase  db, pdb = CORBA_OBJECT_NIL;
	const gchar           *name;

	if (strcmp (requested_interface, "IDL:Bonobo/ConfigDatabase:1.0")) {
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}

	parent = bonobo_moniker_get_parent (moniker, ev);
	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;

	name = bonobo_moniker_get_name (moniker);


	if (parent != CORBA_OBJECT_NIL) {

		pdb = Bonobo_Moniker_resolve (parent, options, 
					      "IDL:Bonobo/ConfigDatabase:1.0", 
					      ev);
    
		bonobo_object_release_unref (parent, NULL);
		
		if (BONOBO_EX (ev) || pdb == CORBA_OBJECT_NIL)
			return CORBA_OBJECT_NIL;

	}

	if (!(db = bonobo_config_dirdb_new (name))) {
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}

	if (pdb != CORBA_OBJECT_NIL) {
		Bonobo_ConfigDatabase_addDatabase (db, pdb, "", 
		        Bonobo_ConfigDatabase_DEFAULT, ev);
		
		bonobo_object_release_unref (pdb, NULL);
		
		if (BONOBO_EX (ev)) {
			bonobo_object_release_unref (db, NULL);
			return CORBA_OBJECT_NIL; 
		}
	}

	return db;
}			

static BonoboObject *
bonobo_moniker_xmldb_factory (BonoboGenericFactory *this, 
			      const char           *object_id,
			      void                 *closure)
{

	if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_xmldb")) {

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
		        "xmldb:", xmldb_resolve));

	} else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_xmldirdb")) {

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
		        "xmldirdb:", dirdb_resolve));
		
	} else
		g_warning ("Failing to manufacture a '%s'", object_id);
	
	return NULL;
}

BONOBO_OAF_FACTORY_MULTI ("OAFIID:Bonobo_Moniker_xmldb_Factory",
			  "bonobo xml database moniker", "1.0",
			  bonobo_moniker_xmldb_factory,
			  NULL);
