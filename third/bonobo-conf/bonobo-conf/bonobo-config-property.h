/**
 * bonobo-config-property.h: config property object implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_PROPERTY_H__
#define __BONOBO_CONFIG_PROPERTY_H__

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-event-source.h>

#include "bonobo-config-database.h"

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_PROPERTY_TYPE        (bonobo_config_property_get_type ())
#define BONOBO_CONFIG_PROPERTY(o)	   (GTK_CHECK_CAST ((o), BONOBO_CONFIG_PROPERTY_TYPE, BonoboConfigProperty))
#define BONOBO_CONFIG_PROPERTY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_PROPERTY_TYPE, BonoboConfigPropertyClass))
#define BONOBO_IS_CONFIG_PROPERTY(o)	   (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_PROPERTY_TYPE))
#define BONOBO_IS_CONFIG_PROPERTY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_PROPERTY_TYPE))

typedef struct _BonoboConfigPropertyPrivate BonoboConfigPropertyPrivate;
typedef struct _BonoboConfigProperty        BonoboConfigProperty;

struct _BonoboConfigProperty {
	BonoboXObject                base;
	
	BonoboConfigPropertyPrivate *priv;
};

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_Property__epv epv;

} BonoboConfigPropertyClass;


GtkType		  
bonobo_config_property_get_type  (void);

BonoboConfigProperty *
bonobo_config_property_new	 (Bonobo_ConfigDatabase db,
				  const gchar *path);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_PROPERTY_H__ */
