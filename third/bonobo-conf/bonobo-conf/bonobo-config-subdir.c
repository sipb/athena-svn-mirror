/**
 * bonobo-config-subdir.c: config database subdirectory 
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-event-source.h>
#include <bonobo/bonobo-arg.h>

#include "bonobo-config-subdir.h"

#define MAX_RETRIES 1

static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_SUBDIR_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

#define SUBDIR_FROM_SERVANT(servant) (BONOBO_CONFIG_SUBDIR (bonobo_object_from_servant (servant)))


struct _BonoboConfigSubdirPrivate {
	Bonobo_ConfigDatabase  db;
	char                  *subdir;
	char                  *moniker;
	BonoboEventSource     *es;
};

static void
notify_cb (BonoboListener    *listener,
	   char              *event_name, 
	   CORBA_any         *any,
	   CORBA_Environment *ev,
	   gpointer           user_data)
{
	/* BonoboConfigSubdir *sd = BONOBO_CONFIG_SUBDIR (user_data); */
	char *tmp, *ename;

	tmp = bonobo_event_subtype (event_name);
	ename = g_strconcat ("Bonobo/Property:change:", tmp, NULL); 
	g_free (tmp);

	/* bonobo_event_source_notify_listeners (cb->es, ename, any, NULL); */

	g_free (ename);
}

static gboolean
server_broken (CORBA_Environment* ev)
{
	if (ev->_major == CORBA_SYSTEM_EXCEPTION)
		return TRUE;

	return FALSE;
}

static gboolean
try_reconnect (BonoboConfigSubdir *sd)
{
	Bonobo_ConfigDatabase db;
	CORBA_Environment ev;

	if (sd->priv->moniker) {

		CORBA_exception_init (&ev);
		
		db = bonobo_get_object (sd->priv->moniker, 
					"Bonobo/ConfigDatabase", &ev);

		if (BONOBO_EX (&ev) || db == CORBA_OBJECT_NIL) {
			CORBA_exception_free (&ev);
			return FALSE;
		}
		
		CORBA_exception_free (&ev);

		sd->priv->db = db;

		return TRUE;
	} else
		return FALSE;
}

static char *
join_keys (const char *key1, const char *key2)
{
	if (!key1) {

		if (!key2)
			return g_strdup ("");
		else
			return g_strdup (key2);
	}

	if (!key2)
		return g_strdup (key1);

	if (key2[0] == '/')
		return g_strconcat (key1, key2, NULL);
	else 
		return g_strconcat (key1, "/", key2, NULL);
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     const CORBA_char       *loc,
				     CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	CORBA_any *v = NULL;
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, key);


	do {
		CORBA_exception_free (ev);

		v = Bonobo_ConfigDatabase_getValue (sd->priv->db, full_key, 
						    loc, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
	
	return v;
}

static void 
impl_Bonobo_ConfigDatabase_setValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     const CORBA_any        *value,
				     CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, key);

	do {
		CORBA_exception_free (ev);

		Bonobo_ConfigDatabase_setValue (sd->priv->db, full_key, value,
						ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getDefault (PortableServer_Servant  servant,
				       const CORBA_char       *key, 
				       const CORBA_char       *locale,
				       CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	CORBA_any *v = NULL;
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, key);

	do {
		CORBA_exception_free (ev);

		v = Bonobo_ConfigDatabase_getDefault (sd->priv->db, full_key,
						      locale, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
	
	return v;
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listDirs (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	Bonobo_KeyList *l;
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, dir);

	do {
		CORBA_exception_free (ev);

		l = Bonobo_ConfigDatabase_listDirs (sd->priv->db, full_key, 
						    ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
	
	return l;
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listKeys (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	Bonobo_KeyList *l;
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, dir);

	do {
		CORBA_exception_free (ev);

		l = Bonobo_ConfigDatabase_listKeys (sd->priv->db, full_key,
						    ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
	
	return l;
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase_dirExists (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	CORBA_boolean res;
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, dir);

	do {
		CORBA_exception_free (ev);

		res = Bonobo_ConfigDatabase_dirExists (sd->priv->db, full_key,
						       ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
	
	return res;
}

static void 
impl_Bonobo_ConfigDatabase_removeValue (PortableServer_Servant  servant,
					const CORBA_char       *key,
					CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, key);

	do {
		CORBA_exception_free (ev);

		Bonobo_ConfigDatabase_removeValue (sd->priv->db, full_key, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
}

static void 
impl_Bonobo_ConfigDatabase_removeDir (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	char *full_key;
	int try = MAX_RETRIES;

	full_key = join_keys (sd->priv->subdir, dir);

	do {
		CORBA_exception_free (ev);

		Bonobo_ConfigDatabase_removeDir (sd->priv->db, full_key, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	g_free (full_key);
}

static void 
impl_Bonobo_ConfigDatabase_addDatabase (PortableServer_Servant servant,
					const Bonobo_ConfigDatabase ddb,
					const CORBA_char  *dir,
					const Bonobo_ConfigDatabase_DBFlags fl,
					CORBA_Environment *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	int try = MAX_RETRIES;

	if (sd->priv->subdir) {
		bonobo_exception_set (ev, 
				      ex_Bonobo_ConfigDatabase_BackendFailed);

		g_warning ("cant compose configuration subdirs");

	} else {

		do {
			CORBA_exception_free (ev);

			Bonobo_ConfigDatabase_addDatabase (sd->priv->db, ddb, 
							   dir, fl, ev); 

		} while (!try-- && server_broken (ev) && try_reconnect (sd));
	}
}

static void 
impl_Bonobo_ConfigDatabase_sync (PortableServer_Servant  servant, 
				 CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	int try = MAX_RETRIES;

	do {
		CORBA_exception_free (ev);

		Bonobo_ConfigDatabase_sync (sd->priv->db, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase__get_writeable (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
	BonoboConfigSubdir *sd = SUBDIR_FROM_SERVANT (servant);
	CORBA_boolean res;

	int try = MAX_RETRIES;

	do {
		CORBA_exception_free (ev);

		res = Bonobo_ConfigDatabase__get_writeable (sd->priv->db, ev);

	} while (!try-- && server_broken (ev) && try_reconnect (sd));

	return res;
}

static void
bonobo_config_subdir_destroy (GtkObject *object)
{
	BonoboConfigSubdir *sd = BONOBO_CONFIG_SUBDIR (object);

	if (sd->priv->db != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (sd->priv->db, NULL);
	sd->priv->db = CORBA_OBJECT_NIL;
	
	g_free (sd->priv->subdir);
	sd->priv->subdir = NULL;

	g_free (sd->priv->moniker);
	sd->priv->moniker = NULL;

	g_free (sd->priv);
	
	parent_class->destroy (object);
}

static void
bonobo_config_subdir_class_init (BonoboConfigSubdirClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_ConfigDatabase__epv *epv;
		
	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_subdir_destroy;

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
bonobo_config_subdir_init (BonoboConfigSubdir *cd)
{
	cd->priv = g_new0 (BonoboConfigSubdirPrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigSubdir, 
			 Bonobo_ConfigDatabase,
			 PARENT_TYPE,
			 bonobo_config_subdir);
 

Bonobo_ConfigDatabase 
bonobo_config_proxy_new       (Bonobo_ConfigDatabase  db, 
			       const char            *subdir,
			       const char            *moniker)
{
	BonoboConfigSubdir *sd;
	CORBA_Environment ev;
	char *m, *dir = NULL;
	int l;

	g_return_val_if_fail (db != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	if (subdir && subdir [0]) {

		if (subdir [0] == '/')
			dir = g_strdup (subdir);
		else
			dir = g_strconcat ("/", subdir, NULL);

		while ((l = strlen (dir)) > 1 && dir [l - 1] == '/') 
			dir [l] = '\0';

		if (!dir [0]) {
			g_free (dir);
			return CORBA_OBJECT_NIL;
		}
	}

	if (!(sd = gtk_type_new (BONOBO_CONFIG_SUBDIR_TYPE))) {
		g_free (dir);
		return CORBA_OBJECT_NIL;
	}

	sd->priv->subdir = dir;
	sd->priv->moniker = g_strdup (moniker);
	sd->priv->es = bonobo_event_source_new ();

	bonobo_object_add_interface (BONOBO_OBJECT (sd), 
				     BONOBO_OBJECT (sd->priv->es));

	CORBA_exception_init (&ev);

	bonobo_object_dup_ref (db, &ev);

	if (BONOBO_EX (&ev)) {
		bonobo_object_unref (BONOBO_OBJECT (sd));
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	sd->priv->db = db;
	
	m = g_strconcat ("Bonobo/ConfigDatabase:change:", NULL);
	
	bonobo_event_source_client_add_listener (db, notify_cb, m, NULL, sd);

	g_free (m);

	return CORBA_Object_duplicate (BONOBO_OBJREF (sd), NULL);
}

Bonobo_ConfigDatabase 
bonobo_config_subdir_new       (Bonobo_ConfigDatabase  db, 
				const char            *subdir)
{
	return bonobo_config_proxy_new (db, NULL, subdir);
}
