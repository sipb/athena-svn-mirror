/**
 * bonobo-config-dirdb.h: xml configuration database implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_DIRDB_H__
#define __BONOBO_CONFIG_DIRDB_H__

#include <stdio.h>
#include <bonobo-conf/bonobo-config-database.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <bonobo/bonobo-event-source.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_DIRDB_TYPE        (bonobo_config_dirdb_get_type ())
#define BONOBO_CONFIG_DIRDB(o)	        (GTK_CHECK_CAST ((o), BONOBO_CONFIG_DIRDB_TYPE, BonoboConfigDIRDB))
#define BONOBO_CONFIG_DIRDB_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_DIRDB_TYPE, BonoboConfigDIRDBClass))
#define BONOBO_IS_CONFIG_DIRDB(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_DIRDB_TYPE))
#define BONOBO_IS_CONFIG_DIRDB_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_DIRDB_TYPE))

typedef struct _BonoboConfigDIRDB     BonoboConfigDIRDB;
typedef struct _BonoboConfigDIRDBPriv BonoboConfigDIRDBPriv;

struct _BonoboConfigDIRDB {
	BonoboConfigDatabase   base;
	BonoboConfigDIRDBPriv *priv;
};

typedef struct {
	BonoboConfigDatabaseClass parent_class;
} BonoboConfigDIRDBClass;


GtkType		      
bonobo_config_dirdb_get_type  (void);

Bonobo_ConfigDatabase
bonobo_config_dirdb_new (const char *basedir);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_DIRDB_H__ */
