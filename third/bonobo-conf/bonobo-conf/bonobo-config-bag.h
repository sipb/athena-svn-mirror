/**
 * bonobo-config-bag.h: config bag object implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000, 2001 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_BAG_H__
#define __BONOBO_CONFIG_BAG_H__

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-event-source.h>

#include "bonobo-config-database.h"

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_BAG_TYPE        (bonobo_config_bag_get_type ())
#define BONOBO_CONFIG_BAG(o)	      (GTK_CHECK_CAST ((o), BONOBO_CONFIG_BAG_TYPE, BonoboConfigBag))
#define BONOBO_CONFIG_BAG_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_BAG_TYPE, BonoboConfigBagClass))
#define BONOBO_IS_CONFIG_BAG(o)	      (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_BAG_TYPE))
#define BONOBO_IS_CONFIG_BAG_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_BAG_TYPE))

typedef struct _BonoboConfigBag        BonoboConfigBag;

struct _BonoboConfigBag {
	BonoboXObject                  base;

	gchar                         *path;
	gchar                         *locale;
	Bonobo_ConfigDatabase          db;
	BonoboEventSource             *es;
	BonoboTransient               *transient;

	Bonobo_EventSource_ListenerId  listener_id;
};

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_PropertyBag__epv epv;

        /*
         * virtual methods
         */

	Bonobo_PropertyList  *(*get_properties)         (BonoboConfigBag *cb,
							 CORBA_Environment * ev);
	Bonobo_Property       (*get_property_by_name)   (BonoboConfigBag *cb,
							 const CORBA_char * name,
							 CORBA_Environment * ev);
	Bonobo_PropertyNames *(*get_property_names)     (BonoboConfigBag *cb,
							 CORBA_Environment * ev);
	void                  (*set_values)             (BonoboConfigBag *cb,
							 const Bonobo_PropertySet * set,
							 CORBA_Environment * ev);
	Bonobo_PropertySet   *(*get_values)             (BonoboConfigBag *cb,
							 CORBA_Environment * ev);
} BonoboConfigBagClass;


GtkType		  bonobo_config_bag_get_type  (void);
BonoboConfigBag	 *bonobo_config_bag_new	      (Bonobo_ConfigDatabase db,
					       const gchar *path);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_BAG_H__ */
