/**
 * bonobo-config-gconfdb.c: GConf based configuration backend
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-xobject.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag-xml.h>
#include <bonobo-conf/bonobo-config-utils.h>

#include "bonobo-config-gconfdb.h"

static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_GCONFDB_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE BONOBO_CONFIG_DATABASE_TYPE

#define ANY_PREFIX "%CORBA:ANY%"

#define SET_BACKEND_FAILED(ev) CORBA_exception_set (ev, CORBA_USER_EXCEPTION, ex_Bonobo_ConfigDatabase_BackendFailed, NULL)

static CORBA_any *
gconf_to_corba_any (GConfValue *gv)
{ 
	BonoboArg         *value = NULL;
	const gchar       *str;
	BonoboUINode      *node;
        CORBA_Environment  ev;

	if (!gv)
		return bonobo_arg_new (BONOBO_ARG_NULL);

        CORBA_exception_init (&ev);

	switch (gv->type) {
	case GCONF_VALUE_INVALID:
		return NULL;
	case GCONF_VALUE_INT:
		value = bonobo_arg_new (BONOBO_ARG_INT);
		BONOBO_ARG_SET_LONG (value, gconf_value_get_int (gv));
		return value;
	case GCONF_VALUE_FLOAT:
		value = bonobo_arg_new (BONOBO_ARG_DOUBLE);
		BONOBO_ARG_SET_DOUBLE (value, gconf_value_get_float (gv));
		return value;
	case GCONF_VALUE_BOOL:
		value = bonobo_arg_new (BONOBO_ARG_BOOLEAN);
		BONOBO_ARG_SET_BOOLEAN (value, gconf_value_get_bool (gv));
		return value;
	case GCONF_VALUE_STRING:
		str = gconf_value_get_string (gv);
		
		if (strncmp (str, ANY_PREFIX, strlen (ANY_PREFIX))) {
			value = bonobo_arg_new (TC_string);
			BONOBO_ARG_SET_STRING (value, str);

			return value;
		}

		node = bonobo_ui_node_from_string (&str[strlen (ANY_PREFIX)]);
		if (!node)
			return NULL;

		value = bonobo_property_bag_xml_decode_any (node, &ev);
		bonobo_ui_node_free (node);

		return value;
	default: 
		return NULL;
	}

	return NULL;
} 

static GConfValue *
corba_any_to_gconf (const CORBA_any *any)
{
	BonoboUINode      *node;
	gchar             *enc, *str;
        CORBA_Environment  ev;
	GConfValue        *gv;

        CORBA_exception_init (&ev);

	g_return_val_if_fail (any != NULL, NULL);

	if (bonobo_arg_type_is_equal (any->_type, BONOBO_ARG_STRING, NULL)) {
		gv = gconf_value_new (GCONF_VALUE_STRING);
		gconf_value_set_string (gv, BONOBO_ARG_GET_STRING (any));
		return gv;
	}

	if (bonobo_arg_type_is_equal (any->_type, BONOBO_ARG_INT, NULL)) {
		gv = gconf_value_new (GCONF_VALUE_INT);
		gconf_value_set_int (gv, BONOBO_ARG_GET_INT (any));
		return gv;
	}

	if (bonobo_arg_type_is_equal (any->_type, BONOBO_ARG_DOUBLE, NULL)) {
		gv = gconf_value_new (GCONF_VALUE_FLOAT);
		gconf_value_set_float (gv, BONOBO_ARG_GET_DOUBLE (any));
		return gv;
	}

	if (bonobo_arg_type_is_equal (any->_type, BONOBO_ARG_BOOLEAN, NULL)) {
		gv = gconf_value_new (GCONF_VALUE_BOOL);
		gconf_value_set_bool (gv, BONOBO_ARG_GET_BOOLEAN (any));
		return gv;
	}

	if (!(node = bonobo_property_bag_xml_encode_any (NULL, any, &ev)))
		return NULL;
	if (!(enc = bonobo_ui_node_to_string (node, TRUE))) {
		bonobo_ui_node_free (node);
		return NULL;
	}

	str = g_strconcat (ANY_PREFIX, enc, NULL);
	bonobo_ui_node_free_string (enc);
	bonobo_ui_node_free (node);

	gv = gconf_value_new (GCONF_VALUE_STRING);
	gconf_value_set_string (gv, str);
	g_free (str);
      
	return gv;
}

static CORBA_any *
real_get_value (BonoboConfigDatabase *db,
		const CORBA_char     *key, 
		const CORBA_char     *locale, 
		CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GConfEntry          *ge;
	GConfSchema         *s;
	CORBA_any           *value = NULL;
	char                *rk;
	int                  t = 0;

	if (!strncmp (key, "/doc/short/", 11) && key [11]) {
		t = 1;
		rk = g_strdup (&key [10]);
	} else if (!strncmp (key, "/doc/long/", 10) && key [10]) {
		t = 2;
		rk = g_strdup (&key [9]);
	} else 
		rk = g_strdup (key);
	
	/* disabled locale because it is not supported by GConf */
	ge = gconf_client_get_entry (gconfdb->client, rk, /* locale */ NULL, 
				     TRUE, NULL);
	
	g_free (rk);

	if (!t) {
		if (ge) { 
			value = gconf_to_corba_any (ge->value);
			gconf_entry_free (ge);
		}

		if (!value)
			bonobo_exception_set (ev, 
			        ex_Bonobo_ConfigDatabase_NotFound);
		
		return value;
	}

	/* this does not work at all, because GConf does not return 
	 * schema_name (always get NULL) */  

	if (!ge || !ge->schema_name) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return NULL;
	}

	rk = g_strdup (ge->schema_name);
	gconf_entry_free (ge);

	if ((s = gconf_client_get_schema (gconfdb->client, rk, NULL))) {

		if (t == 1 && s->short_desc) {
			value = bonobo_arg_new (TC_string);
			BONOBO_ARG_SET_STRING (value, s->short_desc);
		} else if (t == 2 && s->long_desc) {
			value = bonobo_arg_new (TC_string);
			BONOBO_ARG_SET_STRING (value, s->long_desc);
		}
		
		gconf_schema_free (s);
	}

	g_free (rk);

	if (!value)
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);

	return value;
}

static void
real_sync (BonoboConfigDatabase *db, 
	   CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);

	gconf_client_suggest_sync (gconfdb->client, NULL);
}


static void
real_set_value (BonoboConfigDatabase *db,
		const CORBA_char     *key, 
		const CORBA_any      *value,
		CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GConfValue          *gv;
	GError              *err = NULL;

	if (!strncmp (key, "/doc/short/", 11) ||
	    !strncmp (key, "/doc/long/", 10)) {
		bonobo_exception_set (ev, ex_Bonobo_ConfigDatabase_NotFound);
		return;
	}

	if (!(gv = corba_any_to_gconf (value))) 
		return;

	gconf_client_set (gconfdb->client, key, gv, &err);
	
	gconf_value_free (gv);

	if (err) {
		SET_BACKEND_FAILED (ev);
		g_error_free (err);
	}
}

static Bonobo_KeyList *
real_list_dirs (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GSList              *l, *gcl;
	GError              *err = NULL;
	Bonobo_KeyList      *key_list;
	int                  len;

	gcl = gconf_client_all_dirs (gconfdb->client, dir, &err); 

	if (err) {
		SET_BACKEND_FAILED (ev);
		g_error_free (err);
		return NULL;
	}

	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;
	len = g_slist_length (gcl);

	if (!len)
		return key_list;
	
	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 

	for (l = gcl; l != NULL; l = l->next) {
		key_list->_buffer [key_list->_length] = 
			CORBA_string_dup ((char *)l->data);
		g_free (l->data);
		key_list->_length++;
	}
	
	g_slist_free (gcl);

	return key_list;
}

static Bonobo_KeyList *
real_list_keys (BonoboConfigDatabase *db,
		const CORBA_char     *dir,
		CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GSList              *l, *gcl;
	GError              *err = NULL;
	Bonobo_KeyList      *key_list;
	GConfEntry          *ge;
	int                  len;

	gcl = gconf_client_all_entries (gconfdb->client, dir, &err);

	if (err) {
		SET_BACKEND_FAILED (ev);
		g_error_free (err);
		return NULL;
	}

	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;
	len = g_slist_length (gcl);

	if (!len)
		return key_list;
	
	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 

	for (l = gcl; l != NULL; l = l->next) {
		ge = (GConfEntry *)l->data;
		key_list->_buffer [key_list->_length] = 
			CORBA_string_dup (g_basename (ge->key));
		gconf_entry_free (ge);
		key_list->_length++;
	}
	
	g_slist_free (gcl);

	return key_list;
}

static CORBA_boolean
real_dir_exists (BonoboConfigDatabase *db,
		 const CORBA_char     *dir,
		 CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GError              *err = NULL;
	gboolean             retval;

	retval = gconf_client_dir_exists (gconfdb->client, dir, &err);

	if (err) {
		SET_BACKEND_FAILED (ev);
		g_error_free (err);
		return FALSE;
	}

	return retval;
}


static void
real_remove_value (BonoboConfigDatabase *db,
		   const CORBA_char     *key, 
		   CORBA_Environment    *ev)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (db);
	GError              *err = NULL;
	
	gconf_client_unset (gconfdb->client, key, &err);

	if (err) {
		SET_BACKEND_FAILED (ev);
		g_error_free (err);
	}
}


static void
real_remove_dir (BonoboConfigDatabase *db,
		 const CORBA_char     *dir, 
		 CORBA_Environment    *ev)
{
	/* fixme: can we simply ignore this? */
}

static void
bonobo_config_gconfdb_destroy (GtkObject *object)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (object);

	if (gconfdb->es)
		bonobo_object_unref (BONOBO_OBJECT (gconfdb->es));

	if (gconfdb->client) {

		gconf_client_notify_remove (gconfdb->client, gconfdb->nid);

		gtk_object_unref (GTK_OBJECT (gconfdb->client));
	}

	parent_class->destroy (object);
}

static void
bonobo_config_gconfdb_class_init (BonoboConfigDatabaseClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	BonoboConfigDatabaseClass *cd_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_gconfdb_destroy;

	cd_class = BONOBO_CONFIG_DATABASE_CLASS (class);
		
	cd_class->get_value    = real_get_value;
	cd_class->set_value    = real_set_value;
	cd_class->list_dirs    = real_list_dirs;
	cd_class->list_keys    = real_list_keys;
	cd_class->dir_exists   = real_dir_exists;
	cd_class->remove_value = real_remove_value;
	cd_class->remove_dir   = real_remove_dir;
	cd_class->sync         = real_sync;
}

static void
bonobo_config_gconfdb_init (BonoboConfigGConfDB *xmldb)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC (BonoboConfigGConfDB, PARENT_TYPE, bonobo_config_gconfdb);

static void
bonobo_config_gconfdb_notify_listeners (GConfClient* client,
					guint cnxn_id,
					GConfEntry *entry,
					gpointer user_data)
{
	BonoboConfigGConfDB *gconfdb = BONOBO_CONFIG_GCONFDB (user_data);
	char                *ename, *dir_name, *leaf_name;
	CORBA_any           *value;
	CORBA_Environment    ev;

	CORBA_exception_init(&ev);

	value = gconf_to_corba_any (entry->value);

	ename = g_strconcat ("Bonobo/Property:change:", entry->key, NULL);

	bonobo_event_source_notify_listeners(gconfdb->es, ename, value, &ev);

	g_free (ename);

	if (!(dir_name = bonobo_config_dir_name (entry->key)))
		dir_name = g_strdup ("");

	if (!(leaf_name = bonobo_config_leaf_name (entry->key)))
		leaf_name = g_strdup ("");
	
	ename = g_strconcat ("Bonobo/ConfigDatabase:change", dir_name, ":", 
			     leaf_name, NULL);

	bonobo_event_source_notify_listeners (gconfdb->es, ename, value, &ev);
	
	g_free (ename);
	g_free (dir_name);
	g_free (leaf_name);

	CORBA_exception_free (&ev);

	bonobo_arg_release (value);

}

void
bonobo_config_init_gconf_listener (GConfClient *client)
{
	static gboolean bonobo_config_gconf_init = TRUE;

	if (bonobo_config_gconf_init) {
		gconf_client_add_dir (client, "/", GCONF_CLIENT_PRELOAD_NONE,
				      NULL);
		bonobo_config_gconf_init = FALSE;
	}
}

Bonobo_ConfigDatabase
bonobo_config_gconfdb_new ()
{
	BonoboConfigGConfDB   *gconfdb;
	GConfClient           *client;

	if (!gconf_is_initialized())
		gconf_init (0, NULL, NULL);

	if (!(client = gconf_client_get_default ()))
		return NULL;

	if (!(gconfdb = gtk_type_new (BONOBO_CONFIG_GCONFDB_TYPE)))
		return CORBA_OBJECT_NIL;

	gconfdb->client = client;

	gconfdb->es = bonobo_event_source_new ();

	bonobo_object_add_interface (BONOBO_OBJECT (gconfdb), 
				     BONOBO_OBJECT (gconfdb->es));

	bonobo_config_init_gconf_listener (client);

	gconfdb->nid = gconf_client_notify_add 
		(client, "/", bonobo_config_gconfdb_notify_listeners, gconfdb, 
		 NULL, NULL);

	return CORBA_Object_duplicate (BONOBO_OBJREF (gconfdb), NULL);
}
