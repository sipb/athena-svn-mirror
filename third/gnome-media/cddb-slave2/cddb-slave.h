/*
 * cddb-slave.h: Header for CDDBSlave object
 * internal to cddb-slave
 *
 * Copyright (C) 2001-2002 Iain Holmes
 *
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __CDDB_SLAVE_H__
#define __CDDB_SLAVE_H__

#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-event-source.h>
#include "GNOME_Media_CDDBSlave2.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define CDDB_SLAVE_TYPE (cddb_slave_get_type ())
#define CDDB_SLAVE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CDDB_SLAVE_TYPE, CDDBSlave))
#define CDDB_SLAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CDDB_SLAVE_TYPE, CDDBSlaveClass))
#define IS_CDDB_SLAVE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CDDB_SLAVE_TYPE))
#define IS_CDDB_SLAVE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CDDB_SLAVE_TYPE))

typedef struct _CDDBSlave CDDBSlave;
typedef struct _CDDBSlavePrivate CDDBSlavePrivate;
typedef struct _CDDBSlaveClass CDDBSlaveClass;
typedef struct _CDDBEntry {
	char *realdiscid; /* The discid of the file and discid of the contents may not be the same */
	char *discid;
	int ntrks;
	int revision;
	int disc_length;

	int *offsets;
	int *lengths;

	GList *comments;
	GHashTable *fields;
	gboolean is_valid; /* TRUE when result of a good lookup or editor save */
	GNOME_Media_CDDBSlave2_Result result; /* result of query */
} CDDBEntry;


struct _CDDBSlave {
	BonoboObject parent;

	CDDBSlavePrivate *priv;
};

struct _CDDBSlaveClass {
	BonoboObjectClass parent_class;

	POA_GNOME_Media_CDDBSlave2__epv epv;
};

GType cddb_slave_get_type (void);
CDDBSlave *cddb_slave_new (const char *server,
			   int port,
			   const char *name,
			   const char *hostname);
CDDBSlave *cddb_slave_new_full (const char *server,
				int port,
				const char *name,
				const char *hostname,
				BonoboEventSource *event_source);
BonoboEventSource *cddb_slave_get_event_source (CDDBSlave *cddb);

#ifdef __cplusplus
}
#endif

#endif
