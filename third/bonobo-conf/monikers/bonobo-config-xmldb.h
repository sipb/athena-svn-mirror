/**
 * bonobo-config-xmldb.h: xml configuration database implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_XMLDB_H__
#define __BONOBO_CONFIG_XMLDB_H__

#include <stdio.h>
#include <bonobo-conf/bonobo-config-database.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <bonobo/bonobo-event-source.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_XMLDB_TYPE        (bonobo_config_xmldb_get_type ())
#define BONOBO_CONFIG_XMLDB(o)	        (GTK_CHECK_CAST ((o), BONOBO_CONFIG_XMLDB_TYPE, BonoboConfigXMLDB))
#define BONOBO_CONFIG_XMLDB_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_XMLDB_TYPE, BonoboConfigXMLDBClass))
#define BONOBO_IS_CONFIG_XMLDB(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_XMLDB_TYPE))
#define BONOBO_IS_CONFIG_XMLDB_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_XMLDB_TYPE))

typedef struct _DirData DirData;

struct _DirData {
	char       *name;
	GSList     *entries;
	GSList     *subdirs;
	xmlNodePtr  node;
	DirData    *dir;
};

typedef struct {
	char       *name;
	CORBA_any  *value;
	xmlNodePtr  node;
	DirData    *dir;
} DirEntry;

typedef struct _BonoboConfigXMLDB        BonoboConfigXMLDB;

struct _BonoboConfigXMLDB {
	BonoboConfigDatabase  base;
	
	char                 *filename;
	xmlDocPtr             doc;
	DirData              *dir;
	guint                 time_id;

	BonoboEventSource    *es;
};

typedef struct {
	BonoboConfigDatabaseClass parent_class;
} BonoboConfigXMLDBClass;


GtkType		      
bonobo_config_xmldb_get_type  (void);

Bonobo_ConfigDatabase
bonobo_config_xmldb_new (const char *filename);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_XMLDB_H__ */
