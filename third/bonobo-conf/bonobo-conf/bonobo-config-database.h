/**
 * bonobo-config-database.h: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_DATABASE_H__
#define __BONOBO_CONFIG_DATABASE_H__

#include <bonobo/bonobo-xobject.h>
#include <bonobo-conf/Bonobo_Config.h>

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_DATABASE_TYPE        (bonobo_config_database_get_type ())
#define BONOBO_CONFIG_DATABASE(o)	   (GTK_CHECK_CAST ((o), BONOBO_CONFIG_DATABASE_TYPE, BonoboConfigDatabase))
#define BONOBO_CONFIG_DATABASE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_DATABASE_TYPE, BonoboConfigDatabaseClass))
#define BONOBO_IS_CONFIG_DATABASE(o)	   (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_DATABASE_TYPE))
#define BONOBO_IS_CONFIG_DATABASE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_DATABASE_TYPE))

typedef struct _BonoboConfigDatabasePrivate BonoboConfigDatabasePrivate;
typedef struct _BonoboConfigDatabase        BonoboConfigDatabase;

struct _BonoboConfigDatabase {
	BonoboXObject base;

	gboolean writeable;

	BonoboConfigDatabasePrivate *priv;
};

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_ConfigDatabase__epv epv;

        /*
         * virtual methods
         */

	CORBA_any      *(*get_value)    (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 const CORBA_char     *locale,
					 CORBA_Environment    *ev);

	void            (*set_value)    (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 const CORBA_any      *value,
					 CORBA_Environment    *ev);

	Bonobo_KeyList *(*list_dirs)    (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	Bonobo_KeyList *(*list_keys)    (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	CORBA_boolean   (*dir_exists)   (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	void            (*remove_value) (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 CORBA_Environment    *ev);

	void            (*remove_dir)   (BonoboConfigDatabase *db,
					 const CORBA_char     *dir, 
					 CORBA_Environment    *ev);

	void            (*sync)         (BonoboConfigDatabase *db, 
					 CORBA_Environment    *ev);

} BonoboConfigDatabaseClass;


GtkType		      
bonobo_config_database_get_type  (void);

/* some handy utility functions */

gchar *
bonobo_config_get_string               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gchar *
bonobo_config_get_string_with_default  (Bonobo_ConfigDatabase  db,
				        const char            *key,
				        gchar                 *defval,
				        gboolean              *def);
gint16 
bonobo_config_get_short                (Bonobo_ConfigDatabase  db,
			                const char            *key,
			                CORBA_Environment     *opt_ev);
gint16 
bonobo_config_get_short_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint16                 defval,
					gboolean              *def);
guint16 
bonobo_config_get_ushort               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
guint16 
bonobo_config_get_ushort_with_default  (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint16                defval,
					gboolean              *def);
gint32 
bonobo_config_get_long                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gint32 
bonobo_config_get_long_with_default    (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint32                 defval,
					gboolean              *def);
guint32 
bonobo_config_get_ulong                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
guint32 
bonobo_config_get_ulong_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint32                defval,
					gboolean              *def);
gfloat 
bonobo_config_get_float                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gfloat 
bonobo_config_get_float_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					gfloat                 defval,
					gboolean              *def);
gdouble 
bonobo_config_get_double               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gdouble 
bonobo_config_get_double_with_default  (Bonobo_ConfigDatabase  db,
					const char            *key,
					gdouble                defval,
					gboolean              *def);
gboolean
bonobo_config_get_boolean              (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gboolean 
bonobo_config_get_boolean_with_default (Bonobo_ConfigDatabase  db,
					const char            *key,
					gboolean               defval,
					gboolean              *def);
gchar
bonobo_config_get_char                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gchar 
bonobo_config_get_char_with_default    (Bonobo_ConfigDatabase  db,
					const char            *key,
					gchar                  defval,
					gboolean              *def);
CORBA_any *
bonobo_config_get_value                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_TypeCode         opt_tc,
					CORBA_Environment     *opt_ev);

void
bonobo_config_set_string               (Bonobo_ConfigDatabase  db,
					const char            *key,
					const char            *value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_short                (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint16                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_ushort               (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint16                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_long                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint32                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_ulong                (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint32                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_float                (Bonobo_ConfigDatabase  db,
					const char            *key,
					gfloat                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_double               (Bonobo_ConfigDatabase  db,
					const char            *key,
					gdouble                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_boolean              (Bonobo_ConfigDatabase  db,
					const char            *key,
					gboolean               value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_char                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					gchar                  value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_value                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_any             *value,
					CORBA_Environment     *opt_ev);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_DATABASE_H__ */
