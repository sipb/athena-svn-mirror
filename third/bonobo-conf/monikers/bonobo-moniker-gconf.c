/*
 * bonobo-moniker-config.c: Configuration moniker implementation
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-exception.h>

#include "bonobo-config-gconfdb.h"

#define EX_SET_NOT_FOUND(ev) bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound)

static Bonobo_ConfigDatabase db;

static Bonobo_Unknown
gconf_resolve (BonoboMoniker                *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char            *requested_interface,
		CORBA_Environment           *ev)
{
	if (strcmp (requested_interface, "IDL:Bonobo/ConfigDatabase:1.0")) {
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}

	if (db == CORBA_OBJECT_NIL) {
		g_warning ("no GConf default client");
		EX_SET_NOT_FOUND (ev);
		return CORBA_OBJECT_NIL; 
	}
	
	bonobo_object_dup_ref (db, ev);
	
	return db; 
}


static BonoboObject *
bonobo_moniker_gconf_factory (BonoboGenericFactory *this, 
			      const char           *object_id,
			      void                 *closure)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;		
		db = bonobo_config_gconfdb_new ();
	}

	if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_gconf")) {

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
		        "gconf:", gconf_resolve));
	
	} else
		g_warning ("Failing to manufacture a '%s'", object_id);
	
	return NULL;
}

BONOBO_OAF_SHLIB_FACTORY_MULTI ("OAFIID:Bonobo_Moniker_gconf_Factory",
				"bonobo GConf moniker",
				bonobo_moniker_gconf_factory,
				NULL);
