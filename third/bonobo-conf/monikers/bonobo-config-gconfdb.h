/**
 * bonobo-config-gconfdb.h: GConf based configuration backend
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_GCONFDB_H__
#define __BONOBO_CONFIG_GCONFDB_H__

#include <bonobo-conf/bonobo-config-database.h>
#include <bonobo/bonobo-event-source.h>
#include <gconf/gconf-client.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_GCONFDB_TYPE        (bonobo_config_gconfdb_get_type ())
#define BONOBO_CONFIG_GCONFDB(o)	  (GTK_CHECK_CAST ((o), BONOBO_CONFIG_GCONFDB_TYPE, BonoboConfigGConfDB))
#define BONOBO_CONFIG_GCONFDB_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_GCONFDB_TYPE, BonoboConfigGConfDBClass))
#define BONOBO_IS_CONFIG_GCONFDB(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_GCONFDB_TYPE))
#define BONOBO_IS_CONFIG_GCONFDB_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_GCONFDB_TYPE))

typedef struct {
	BonoboConfigDatabase  base;
	
	GConfClient          *client;
	BonoboEventSource    *es;
	guint                 nid;

} BonoboConfigGConfDB;

typedef struct {
	BonoboConfigDatabaseClass parent_class;
} BonoboConfigGConfDBClass;


GtkType		      
bonobo_config_gconfdb_get_type  (void);

Bonobo_ConfigDatabase
bonobo_config_gconfdb_new ();

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_GCONFDB_H__ */
