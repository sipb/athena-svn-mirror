/**
 * bonobo-config-database.c: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-arg.h>

#include "bonobo-config-database.h"


static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_DATABASE_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

#define DATABASE_FROM_SERVANT(servant) (BONOBO_CONFIG_DATABASE (bonobo_object_from_servant (servant)))

typedef struct {
	Bonobo_ConfigDatabase db;
	char *path;
} DataBaseInfo;

struct _BonoboConfigDatabasePrivate {
	GList *db_list;
};

static void
insert_key_name (gpointer	key,
		 gpointer	value,
		 gpointer	user_data)
{
	Bonobo_KeyList *key_list = (Bonobo_KeyList *)user_data;

	key_list->_buffer [key_list->_length++] = CORBA_string_dup (key);
}

Bonobo_KeyList *
merge_keylists (Bonobo_KeyList *cur_list, 
		Bonobo_KeyList *def_list)
{
	Bonobo_KeyList *key_list;
	GHashTable     *ht;
	int             i, len;

	ht =  g_hash_table_new (g_str_hash, g_str_equal);

	for (i = 0; i < cur_list->_length; i++) 
		g_hash_table_insert (ht, cur_list->_buffer [i], NULL);

	for (i = 0; i < def_list->_length; i++)
		g_hash_table_insert (ht, def_list->_buffer [i], NULL);

	len =  g_hash_table_size (ht);

	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;
	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 

	g_hash_table_foreach (ht, insert_key_name, key_list);

	g_hash_table_destroy (ht);

	return key_list;
}

static CORBA_any *
get_default (BonoboConfigDatabase   *cd,
	     const CORBA_char       *key, 
	     const CORBA_char       *locale,
	     CORBA_Environment      *ev)
{
	CORBA_any *value = NULL;
	DataBaseInfo *info;
	GList *l;

	bonobo_object_ref (BONOBO_OBJECT (cd));

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		value = Bonobo_ConfigDatabase_getValue (info->db, key, locale,
							ev);
		if (BONOBO_EX (ev)) {
			bonobo_object_unref (BONOBO_OBJECT (cd));
			return NULL;
		}

		if (value) {
			bonobo_object_unref (BONOBO_OBJECT (cd));
			return value;
		}
	}

	bonobo_object_unref (BONOBO_OBJECT (cd));

	bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);

	return NULL;
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     const CORBA_char       *locale,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_any *value = NULL;

	if (CLASS (cd)->get_value)
		value = CLASS (cd)->get_value (cd, key, locale, ev);

	if (!BONOBO_EX (ev) && value)
		return value;

	CORBA_exception_init (ev);

	return get_default (cd, key, locale, ev);
}

static void 
impl_Bonobo_ConfigDatabase_setValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     const CORBA_any        *value,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->set_value)
		CLASS (cd)->set_value (cd, key, value, ev);
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getDefault (PortableServer_Servant  servant,
				       const CORBA_char       *key, 
				       const CORBA_char       *locale,
				       CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	return get_default (cd, key, locale, ev);
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listDirs (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	GList *l;
	Bonobo_KeyList *cur_list, *def_list, *tmp_list;

	cur_list = NULL;

	if (CLASS (cd)->list_keys)
		cur_list = CLASS (cd)->list_dirs (cd, dir, ev);
	
	if (BONOBO_EX (ev))
		return NULL;
	
	CORBA_exception_init (&nev);

	bonobo_object_ref (BONOBO_OBJECT (cd));

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);

		def_list = Bonobo_ConfigDatabase_listDirs (info->db, dir,&nev);
		if (!BONOBO_EX (&nev) && def_list) {

			if (!def_list->_length) {
				CORBA_free (def_list);
				continue;
			}

			if (!cur_list || !cur_list->_length) {
				if (cur_list)
					CORBA_free (cur_list);
				cur_list = def_list;
				continue;
			}

			tmp_list = merge_keylists (cur_list, def_list);
			
			CORBA_free (cur_list);
			CORBA_free (def_list);

			cur_list = tmp_list;
		}
	}

	bonobo_object_unref (BONOBO_OBJECT (cd));

	CORBA_exception_free (&nev);

	return cur_list;
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listKeys (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	GList *l;
	Bonobo_KeyList *cur_list, *def_list, *tmp_list;

	cur_list = NULL;

	if (CLASS (cd)->list_keys)
		cur_list = CLASS (cd)->list_keys (cd, dir, ev);
	
	if (BONOBO_EX (ev))
		return NULL;

	CORBA_exception_init (&nev);

	bonobo_object_ref (BONOBO_OBJECT (cd));

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);

		def_list = Bonobo_ConfigDatabase_listKeys (info->db, dir,&nev);
		if (!BONOBO_EX (&nev) && def_list) {

			if (!def_list->_length) {
				CORBA_free (def_list);
				continue;
			}

			if (!cur_list || !cur_list->_length) {
				if (cur_list)
					CORBA_free (cur_list);
				cur_list = def_list;
				continue;
			}

			tmp_list = merge_keylists (cur_list, def_list);
			
			CORBA_free (cur_list);
			CORBA_free (def_list);

			cur_list = tmp_list;
		}
	}

	bonobo_object_unref (BONOBO_OBJECT (cd));

	CORBA_exception_free (&nev);

	return cur_list;
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase_dirExists (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	CORBA_boolean res;
	GList *l;

	if (CLASS (cd)->dir_exists &&
	    CLASS (cd)->dir_exists (cd, dir, ev))
		return TRUE;

	CORBA_exception_init (&nev);

	bonobo_object_ref (BONOBO_OBJECT (cd));

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);
		
		res = Bonobo_ConfigDatabase_dirExists (info->db, dir, &nev);
		
		CORBA_exception_free (&nev);
		
		if (!BONOBO_EX (&nev) && res)
			return TRUE;
	}

	bonobo_object_unref (BONOBO_OBJECT (cd));

	CORBA_exception_free (&nev);

	return CORBA_FALSE;
}

static void 
impl_Bonobo_ConfigDatabase_removeValue (PortableServer_Servant  servant,
					const CORBA_char       *key,
					CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->remove_value)
		CLASS (cd)->remove_value (cd, key, ev);
}

static void 
impl_Bonobo_ConfigDatabase_removeDir (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->remove_dir)
		CLASS (cd)->remove_dir (cd, dir, ev);
}

static void 
impl_Bonobo_ConfigDatabase_addDatabase (PortableServer_Servant servant,
					const Bonobo_ConfigDatabase ddb,
					const CORBA_char  *path,
					const Bonobo_ConfigDatabase_DBFlags fl,
					CORBA_Environment *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	DataBaseInfo *info;
	GList *l;

	g_return_if_fail (cd->priv != NULL);

	/* we cant add ourselves */
	if (CORBA_Object_is_equivalent (BONOBO_OBJREF (cd), ddb, NULL))
		return;

	bonobo_object_ref (BONOBO_OBJECT (cd));

	for (l = cd->priv->db_list; l; l = l->next) {
		info = l->data;
		if (CORBA_Object_is_equivalent (info->db, ddb, NULL)) {
			bonobo_object_unref (BONOBO_OBJECT (cd));
			return;
		}
	}

	info = g_new0 (DataBaseInfo , 1);

	info->db = bonobo_object_dup_ref (ddb, ev);

	if (BONOBO_EX (ev)) {
		g_free (info);
		return;
	}
	
	info->path = g_strdup (path);

	cd->priv->db_list = g_list_append (cd->priv->db_list, info);

	bonobo_object_unref (BONOBO_OBJECT (cd));
}

static void 
impl_Bonobo_ConfigDatabase_sync (PortableServer_Servant  servant, 
				 CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->sync)
		CLASS (cd)->sync (cd, ev);
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase__get_writeable (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	return cd->writeable;
}

static void
bonobo_config_database_destroy (GtkObject *object)
{
	BonoboConfigDatabase *cd = BONOBO_CONFIG_DATABASE (object);
	GList *db_list;
	GList *l;

	db_list = cd->priv->db_list;
	cd->priv->db_list = NULL;

	for (l = db_list; l; l = l->next) {
		DataBaseInfo *info = l->data;

		bonobo_object_release_unref (info->db, NULL);

		g_free (info->path);
		g_free (info);
	}
	
	g_list_free (db_list);

	parent_class->destroy (object);
}

static void
bonobo_config_database_finalize (GtkObject *object)
{
	BonoboConfigDatabase *cd = (BonoboConfigDatabase *) object;

	g_free (cd->priv);
	cd->priv = NULL;

	parent_class->finalize (object);
}

static void
bonobo_config_database_class_init (BonoboConfigDatabaseClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_ConfigDatabase__epv *epv;
		
	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_database_destroy;
	object_class->finalize = bonobo_config_database_finalize;

	epv = &class->epv;

	epv->getValue       = impl_Bonobo_ConfigDatabase_getValue;
	epv->setValue       = impl_Bonobo_ConfigDatabase_setValue;
	epv->getDefault     = impl_Bonobo_ConfigDatabase_getDefault;
	epv->listDirs       = impl_Bonobo_ConfigDatabase_listDirs;
	epv->listKeys       = impl_Bonobo_ConfigDatabase_listKeys;
	epv->dirExists      = impl_Bonobo_ConfigDatabase_dirExists;
	epv->removeValue    = impl_Bonobo_ConfigDatabase_removeValue;
	epv->removeDir      = impl_Bonobo_ConfigDatabase_removeDir;
	epv->addDatabase    = impl_Bonobo_ConfigDatabase_addDatabase;
	epv->sync           = impl_Bonobo_ConfigDatabase_sync;


	epv->_get_writeable = impl_Bonobo_ConfigDatabase__get_writeable;
}

static void
bonobo_config_database_init (BonoboConfigDatabase *cd)
{
	cd->priv = g_new0 (BonoboConfigDatabasePrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigDatabase, 
			 Bonobo_ConfigDatabase,
			 PARENT_TYPE,
			 bonobo_config_database);

#define MAKE_GET_SIMPLE(c_type, default, name, corba_tc, extract_fn)          \
c_type bonobo_config_get_##name  (Bonobo_ConfigDatabase  db,                  \
				 const char            *key,                  \
				 CORBA_Environment     *opt_ev)               \
{                                                                             \
	CORBA_any *value;                                                     \
	c_type retval;                                                        \
	if (!(value = bonobo_config_get_value (db, key, corba_tc, opt_ev)))   \
		return default;                                               \
	retval = extract_fn;                                                  \
	CORBA_free (value);                                                   \
	return retval;                                                        \
}

#define MAKE_GET_WITH_DEFAULT(c_type, name, assign_fn)                        \
c_type bonobo_config_get_##name##_with_default (Bonobo_ConfigDatabase  db,    \
						const char            *key,   \
						c_type                 defval,\
						gboolean              *def)   \
{                                                                             \
	c_type retval;                                                        \
	CORBA_Environment ev;                                                 \
	CORBA_exception_init (&ev);                                           \
        if (def) *def = FALSE;                                                \
	retval = bonobo_config_get_##name (db, key, &ev);                     \
	if (BONOBO_EX (&ev)) {                                                \
		retval = assign_fn (defval);                                  \
                if (def) *def = TRUE;                                         \
        }                                                                     \
	CORBA_exception_free (&ev);                                           \
	return retval;                                                        \
}

/**
 * bonobo_config_get_string:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a string from the configuration database
 *
 * Returns: the value contained in the database, or zero on error.
 */
MAKE_GET_SIMPLE (gchar *, NULL, string, TC_string, 
		 g_strdup (*(char **)value->_value));

/**
 * bonobo_config_get_string_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a string from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gchar *, string, g_strdup);

/**
 * bonobo_config_get_short:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint16, 0, short, TC_short, (*(gint16 *)value->_value));

/**
 * bonobo_config_get_short_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a 16 bit integer from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gint16, short, );

/**
 * bonobo_config_get_ushort:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint16, 0, ushort, TC_ushort, (*(guint16 *)value->_value));

/**
 * bonobo_config_get_ushort_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a 16 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (guint16, ushort, );

/**
 * bonobo_config_get_long:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint32, 0, long, TC_long, (*(gint32 *)value->_value));

/**
 * bonobo_config_get_long_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a 32 bit integer from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gint32, long, );

/**
 * bonobo_config_get_ulong:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint32, 0, ulong, TC_ulong, (*(guint32 *)value->_value));

/**
 * bonobo_config_get_ulong_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a 32 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (guint32, ulong, );

/**
 * bonobo_config_get_float:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a single precision floating point value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gfloat, 0.0, float, TC_float, (*(gfloat *)value->_value));

/**
 * bonobo_config_get_float_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a single precision floating point value from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gfloat, float, );

/**
 * bonobo_config_get_double:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a double precision floating point value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gdouble, 0.0, double, TC_double, (*(gdouble *)value->_value));

/**
 * bonobo_config_get_double_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a double precision floating point value from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gdouble, double, );

/**
 * bonobo_config_get_char:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 8 bit character value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gchar, '\0', char, TC_char, (*(gchar *)value->_value));

/**
 * bonobo_config_get_char_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a 8 bit character value from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gchar, char, );

/**
 * bonobo_config_get_boolean:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a boolean value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gboolean, FALSE, boolean, TC_boolean, 
		 (*(CORBA_boolean *)value->_value));

/**
 * bonobo_config_get_boolean_with_default:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @defval: the default value
 * @def: A pointer to a flag that will be set if the default value for the item
 * is returned
 *
 * Get a boolean value from the configuration database
 *
 * Returns: the value contained in the database, or @defval on error.
 */
MAKE_GET_WITH_DEFAULT (gboolean, boolean, );

/**
 * bonobo_config_get_value:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_tc: the type of the value, optional
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a value from the configuration database
 *
 * Returns: the value contained in the database, or zero on error.
 */
CORBA_any *
bonobo_config_get_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_TypeCode         opt_tc,
			  CORBA_Environment     *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any *retval;
	char *locale;

	bonobo_return_val_if_fail (db != CORBA_OBJECT_NIL, NULL, opt_ev);
	bonobo_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (!(locale = g_getenv ("LANG")))
		locale = "";

	retval = Bonobo_ConfigDatabase_getValue (db, key, locale, my_ev);

	if (BONOBO_EX (my_ev)) {
		if (!opt_ev) {
			char *err;

			err = bonobo_exception_get_text (my_ev);
			g_warning ("Cannot get value: %s\n", err);
			g_free (err);
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}


	if (retval && opt_tc != CORBA_OBJECT_NIL) {

		/* fixme: we can also try to do automatic type conversions */

		if (!CORBA_TypeCode_equal (opt_tc, retval->_type, my_ev)) {
			CORBA_free (retval);
			if (!opt_ev)
				CORBA_exception_free (&ev);
			bonobo_exception_set (opt_ev, 
			        ex_Bonobo_ConfigDatabase_InvalidType);
			return NULL;
		}

	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

#define MAKE_SET_SIMPLE(c_type, name, corba_type, corba_tc)                   \
void bonobo_config_set_##name (Bonobo_ConfigDatabase  db,                     \
			       const char            *key,                    \
			       const c_type           value,                  \
			       CORBA_Environment     *opt_ev)                 \
{                                                                             \
	CORBA_any *any;                                                       \
	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);               \
	bonobo_return_if_fail (key != NULL, opt_ev);                          \
	any = bonobo_arg_new (corba_tc);                                      \
	*((corba_type *)(any->_value)) = value;                               \
	bonobo_config_set_value (db, key, any, opt_ev);                       \
	bonobo_arg_release (any);                                             \
}

/**
 * bonobo_config_set_short:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit integer value in the configuration database.
 */
MAKE_SET_SIMPLE (gint16, short, CORBA_short, TC_short)
/**
 * bonobo_config_set_ushort:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit unsigned integer value in the configuration database.
 */
MAKE_SET_SIMPLE (guint16, ushort, CORBA_unsigned_short, TC_ushort)
/**
 * bonobo_config_set_long:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit integer value in the configuration database.
 */
MAKE_SET_SIMPLE (gint32, long, CORBA_long, TC_long)
/**
 * bonobo_config_set_ulong:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit unsigned integer value in the configuration database.
 */
MAKE_SET_SIMPLE (guint32, ulong, CORBA_unsigned_long, TC_ulong)
/**
 * bonobo_config_set_float:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a single precision floating point value in the configuration database.
 */
MAKE_SET_SIMPLE (gfloat, float, CORBA_float, TC_float)
/**
 * bonobo_config_set_double:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a double precision floating point value in the configuration database.
 */
MAKE_SET_SIMPLE (gdouble, double, CORBA_double, TC_double)
/**
 * bonobo_config_set_boolean:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a boolean value in the configuration database.
 */
MAKE_SET_SIMPLE (gboolean, boolean, CORBA_boolean, TC_boolean)

/**
 * bonobo_config_set_char:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 8 bit characte value in the configuration database.
 */
MAKE_SET_SIMPLE (gchar, char, CORBA_char, TC_char)
/**
 * bonobo_config_set_string:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a string value in the configuration database.
 */
void
bonobo_config_set_string (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  const char            *value,
			  CORBA_Environment     *opt_ev)
{
	CORBA_any *any;

	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);
	bonobo_return_if_fail (key != NULL, opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);

	any = bonobo_arg_new (TC_string);

	BONOBO_ARG_SET_STRING (any, value);

	bonobo_config_set_value (db, key, any, opt_ev);

	bonobo_arg_release (any);
}

/**
 * bonobo_config_set_value:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a value in the configuration database.
 */
void
bonobo_config_set_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_any             *value,
			  CORBA_Environment     *opt_ev)
{
	CORBA_Environment ev, *my_ev;

	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);
	bonobo_return_if_fail (key != NULL, opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	Bonobo_ConfigDatabase_setValue (db, key, value, my_ev);
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}
