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
#include <bonobo/bonobo-ui-container.h>
#include <bonobo/bonobo-widget.h>

#include <bonobo-conf/bonobo-config-bag.h>
#include <bonobo-conf/bonobo-config-property.h>
#include <bonobo-conf/bonobo-config-subdir.h>

#include <bonobo-conf/bonobo-property-editor.h>
#include <bonobo-conf/bonobo-property-bag-editor.h>
/*
#include <bonobo-conf/bonobo-property-editor-struct.h>
#include <bonobo-conf/bonobo-property-editor-list.h>
*/
#include <bonobo-conf/gtkwtree.h>
#include <bonobo-conf/gtkwtreeitem.h>

/*
#define SYSTEMDB  "xmldirdb:"DATADIR"/bonobo-config"
#define USERDB    "xmldirdb:~/.bonobo-config"
#define DEFAULTDB (SYSTEMDB "#" USERDB)
*/

#define DEFAULTDB "gconf:"

static Bonobo_Unknown
create_bag_editor (Bonobo_ConfigDatabase  db,
		   const char            *name, 
		   CORBA_Environment     *ev)
{
	BonoboConfigBag     *config_bag;
	CORBA_Object         bag;
	BonoboControl       *control;
	BonoboUIContainer   *uic;

	if (!(config_bag = bonobo_config_bag_new (db, name))) {
		bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound);
		return CORBA_OBJECT_NIL;
	}

	bag = BONOBO_OBJREF (config_bag);

	uic = bonobo_ui_container_new ();

	control = bonobo_property_bag_editor_new (bag, BONOBO_OBJREF(uic), ev);

	bonobo_object_unref (BONOBO_OBJECT (uic));

	/* fixme: */ 
	/* bonobo_object_unref (BONOBO_OBJECT (config_bag)); */

	return CORBA_Object_duplicate (BONOBO_OBJREF (control), ev);
}

static Bonobo_Unknown
config_resolve (BonoboMoniker               *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char            *requested_interface,
		CORBA_Environment           *ev)
{
	Bonobo_Moniker         parent;
	Bonobo_ConfigDatabase  db, dbproxy;
	const gchar           *name, *pdn;

	parent = bonobo_moniker_get_parent (moniker, ev);
	if (BONOBO_EX (ev)) 
		return CORBA_OBJECT_NIL;

	if (parent != CORBA_OBJECT_NIL) {

		pdn = Bonobo_Moniker_getDisplayName (parent, ev);
		if (BONOBO_EX (ev) || pdn == NULL) {
			bonobo_object_release_unref (parent, NULL);
			return CORBA_OBJECT_NIL;
		}
		
		db = Bonobo_Moniker_resolve (parent, options, 
		        "IDL:Bonobo/ConfigDatabase:1.0", ev);

		bonobo_object_release_unref (parent, NULL);
	
		if (BONOBO_EX (ev) || db == CORBA_OBJECT_NIL)
			return CORBA_OBJECT_NIL;
	} else {

		pdn = DEFAULTDB;

		db = bonobo_get_object (pdn, "Bonobo/ConfigDatabase", ev);

		if (BONOBO_EX (ev) || db == CORBA_OBJECT_NIL)
			return CORBA_OBJECT_NIL;
	}

	name = bonobo_moniker_get_name (moniker);

	if (!strcmp (requested_interface, "IDL:Bonobo/ConfigDatabase:1.0")) {
	
		dbproxy = db;
		/*
		  dbproxy =  bonobo_config_proxy_new (db, name, pdn);
		  bonobo_object_release_unref (db, NULL);
		*/
		if (dbproxy != CORBA_OBJECT_NIL)
			return dbproxy;

		bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound);

		return CORBA_OBJECT_NIL;		
	}

	if (!strcmp (requested_interface, "IDL:Bonobo/PropertyBag:1.0")) {
		BonoboConfigBag *bag;
	
		dbproxy = db;
		/*
		  dbproxy =  bonobo_config_proxy_new (db, NULL, pdn);
		  bonobo_object_release_unref (db, NULL);
		*/

		if (dbproxy == CORBA_OBJECT_NIL) {
			bonobo_exception_set (ev, 
			        ex_Bonobo_Moniker_InterfaceNotFound);
			return CORBA_OBJECT_NIL;
		}

		bag = bonobo_config_bag_new (dbproxy, name);
		bonobo_object_release_unref (dbproxy, NULL);
	
		if (bag)
			return (Bonobo_Unknown) CORBA_Object_duplicate (
			      BONOBO_OBJREF (bag), ev);
		
	       
		bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound);

		return CORBA_OBJECT_NIL;
	}

	
 	if (!strcmp (requested_interface, "IDL:Bonobo/Property:1.0")) {
		BonoboConfigProperty *prop;

		dbproxy = db;
		/* 
		   dbproxy =  bonobo_config_proxy_new (db, NULL, pdn);
		   bonobo_object_release_unref (db, NULL);
		*/
		if (dbproxy == CORBA_OBJECT_NIL) {
			bonobo_exception_set (ev, 
			        ex_Bonobo_Moniker_InterfaceNotFound);
			return CORBA_OBJECT_NIL;
		}

		prop = bonobo_config_property_new (dbproxy, name);
		bonobo_object_release_unref (dbproxy, NULL);

		if (prop)
			return (Bonobo_Unknown) CORBA_Object_duplicate (
			      BONOBO_OBJREF (prop), ev);

		bonobo_exception_set (ev, ex_Bonobo_Moniker_InterfaceNotFound);

		return CORBA_OBJECT_NIL;
	} 

 	if (!strcmp (requested_interface, "IDL:Bonobo/Control:1.0")) {
		if (Bonobo_ConfigDatabase_dirExists (db, name, ev) &&
		    !BONOBO_EX (ev)) {
			Bonobo_Unknown o;

			dbproxy = db;
			/*
			  dbproxy =  bonobo_config_proxy_new (db, NULL, pdn);
			  bonobo_object_release_unref (db, NULL);
			*/

			if (dbproxy == CORBA_OBJECT_NIL) {
				bonobo_exception_set (ev, 
				        ex_Bonobo_Moniker_InterfaceNotFound);
				return CORBA_OBJECT_NIL;
			}

			o = create_bag_editor (dbproxy, name, ev);

			bonobo_object_release_unref (dbproxy, NULL);

			return o;
		}
	} 
		
	return CORBA_OBJECT_NIL; /* try moniker extenders */
}


static BonoboObject *
bonobo_moniker_config_factory (BonoboGenericFactory *this, 
			       const char           *object_id,
			       void                 *closure)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		initialized = TRUE;		
	}

	if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_config")) {

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
		        "config:", config_resolve));	
	} else
		g_warning ("Failing to manufacture a '%s'", object_id);
	
	return NULL;
}

BONOBO_OAF_SHLIB_FACTORY_MULTI ("OAFIID:Bonobo_Moniker_config_Factory",
				"bonobo configuration moniker",
				bonobo_moniker_config_factory,
				NULL);
