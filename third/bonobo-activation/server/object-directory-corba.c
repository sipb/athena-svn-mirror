/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oafd: OAF CORBA dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyrigfht (C) 1999, 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>,
 *
 */

#include "config.h"

#include <stdio.h>
#include <time.h>
#include <glib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "server.h"

#include "bonobo-activation/bonobo-activation-i18n.h"
#include "activation-server-corba-extensions.h"

/*
 *    We always have the local NamingContext in the
 * registry at exit, so quit if only 1 left.
 */
#define RESIDUAL_SERVERS 1

/*** App-specific servant structures ***/

typedef struct {
        /* CORBA servant structure */
	POA_Bonobo_ObjectDirectory servant;

        /* Information on all servers */
	GHashTable           *by_iid;
	Bonobo_ServerInfoList attr_servers;
	Bonobo_CacheTime      time_list_changed;

	CORBA_char           *attr_domain;

        /* CORBA Object tracking */
	GHashTable      *active_servers;
        guint            no_servers_timeout;
	Bonobo_CacheTime time_active_changed;

        /* Source polling bits */
        char           **registry_source_directories;
        time_t           time_did_stat;
        GHashTable      *registry_directory_mtimes;

	CORBA_Object self;
} impl_POA_Bonobo_ObjectDirectory;

/*
 * DON'T FIXME: this smells, here we vandalise the oh so
 * complicated design and enforce the invariant that there
 * can only ever be one ObjectDirectory in-proc.
 */
static impl_POA_Bonobo_ObjectDirectory *main_dir = NULL;

#ifdef BONOBO_ACTIVATION_DEBUG
static void
od_dump_list (impl_POA_Bonobo_ObjectDirectory * od)
{
#if 0
	int i, j, k;

	for (i = 0; i < od->attr_servers._length; i++) {
		g_print ("IID %s, type %s, location %s\n",
			 od->attr_servers._buffer[i].iid,
			 od->attr_servers._buffer[i].server_type,
			 od->attr_servers._buffer[i].location_info);
		for (j = 0; j < od->attr_servers._buffer[i].props._length;
		     j++) {
			Bonobo_ActivationProperty *prop =
				&(od->attr_servers._buffer[i].
				  props._buffer[j]);
                        if (strchr (prop->name, '-') != NULL) /* Translated, likely to
                                                         be annoying garbage value */
                                continue;

			g_print ("    %s = ", prop->name);
			switch (prop->v._d) {
			case Bonobo_ACTIVATION_P_STRING:
				g_print ("\"%s\"\n", prop->v._u.value_string);
				break;
			case Bonobo_ACTIVATION_P_NUMBER:
				g_print ("%f\n", prop->v._u.value_number);
				break;
			case Bonobo_ACTIVATION_P_BOOLEAN:
				g_print ("%s\n",
					 prop->v.
					 _u.value_boolean ? "TRUE" : "FALSE");
				break;
			case Bonobo_ACTIVATION_P_STRINGV:
				g_print ("[");
				for (k = 0;
				     k < prop->v._u.value_stringv._length;
				     k++) {
					g_print ("\"%s\"",
						 prop->v._u.
						 value_stringv._buffer[k]);
					if (k <
					    (prop->v._u.
					     value_stringv._length - 1))
						g_print (", ");
				}
				g_print ("]\n");
				break;
			}
		}
	}
#endif
}
#endif

static gboolean
registry_directory_needs_update (impl_POA_Bonobo_ObjectDirectory *servant,
                                 const char                      *directory)
{
        gboolean needs_update;
        struct stat statbuf;
        time_t old_mtime;

        if (stat (directory, &statbuf) != 0) {
                return FALSE;
        }
 
        old_mtime = (time_t) g_hash_table_lookup (
                servant->registry_directory_mtimes, directory);

        g_hash_table_insert (servant->registry_directory_mtimes,
                             (gpointer) directory,
                             (gpointer) statbuf.st_mtime);

        needs_update = (old_mtime != statbuf.st_mtime);

#ifdef BONOBO_ACTIVATION_DEBUG
        if (needs_update)
                g_warning ("Compare old_mtime on '%s' with %ld ==? %ld",
                           directory,
                           (long) old_mtime, (long) statbuf.st_mtime);
#endif

        return needs_update;
}

static void
update_registry (impl_POA_Bonobo_ObjectDirectory *servant, gboolean force_reload)
{
        int i;
        time_t cur_time;
        gboolean must_load;
        static gboolean doing_reload = FALSE;

        if (doing_reload)
                return;
        doing_reload = TRUE;

#ifdef BONOBO_ACTIVATION_DEBUG
        g_warning ("Update registry %p", servant->by_iid);
#endif

        /* get first time init right */
        must_load = (servant->by_iid == NULL);
        
        cur_time = time (NULL);

        if (cur_time - 5 > servant->time_did_stat) {
                servant->time_did_stat = cur_time;
                
                for (i = 0; servant->registry_source_directories[i] != NULL; i++) {
                        if (registry_directory_needs_update 
                            (servant, servant->registry_source_directories[i]))
                                must_load = TRUE;
                }
        }
        
        if (must_load || force_reload) {
                /*
                 * FIXME bugzilla.eazel.com 2727: we should only reload those
                 * directories that have actually changed instead of reloading
                 * all when any has changed. 
                 */
#ifdef BONOBO_ACTIVATION_DEBUG
                g_warning ("Re-load %d %d", must_load, force_reload);
#endif
                bonobo_server_info_load (servant->registry_source_directories,
                                         &servant->attr_servers,
                                         &servant->by_iid,
                                         bonobo_activation_hostname_get (),
                                         servant->attr_domain);
                servant->time_did_stat = servant->time_list_changed = time (NULL);

#ifdef BONOBO_ACTIVATION_DEBUG
                od_dump_list (servant);
#endif
                if (must_load)
                        activation_clients_cache_notify ();
        }

        doing_reload = FALSE;
}

static gchar **
split_path_unique (const char *colon_delimited_path)
{
        int i, max;
        gboolean different;
        gchar **ret, **wrk;
        GSList *l, *tmp = NULL;

        g_return_val_if_fail (colon_delimited_path != NULL, NULL);

        wrk = g_strsplit (colon_delimited_path, ":", -1);

        g_return_val_if_fail (wrk != NULL, NULL);

        for (max = i = 0; wrk [i]; i++) {
                different = TRUE;
                for (l = tmp; l; l = l->next) {
                        if (!strcmp (l->data, wrk [i])) {
                                different = FALSE;
                        } else if (wrk [i] == '\0') {
                                different = FALSE;
                        }
                }
                if (different) {
                        tmp = g_slist_prepend (tmp, g_strdup (wrk [i]));
                        max++;
                }
        }

        tmp = g_slist_reverse (tmp);

        ret = g_new (char *, max + 1);

        for (l = tmp, i = 0; l; l = l->next)
                ret [i++] = l->data;

        ret [i] = NULL;

        g_slist_free (tmp);
        g_strfreev (wrk);

        return ret;
}

static Bonobo_ServerInfoListCache *
impl_Bonobo_ObjectDirectory__get_servers (
        impl_POA_Bonobo_ObjectDirectory *servant,
        Bonobo_CacheTime                 only_if_newer,
        CORBA_Environment               *ev)
{
	Bonobo_ServerInfoListCache *retval;

        update_registry (servant, FALSE);

	retval = Bonobo_ServerInfoListCache__alloc ();

	retval->_d = (only_if_newer < servant->time_list_changed);
	if (retval->_d) {
		retval->_u.server_list = servant->attr_servers;
		CORBA_sequence_set_release (&retval->_u.server_list,
					    CORBA_FALSE);
	}

	return retval;
}

typedef struct {
	Bonobo_ImplementationID *seq;
	int                      last_used;
} StateCollectionInfo;

static void
collate_active_server (char *key, gpointer value, StateCollectionInfo *sci)
{
	sci->seq [(sci->last_used)++] = CORBA_string_dup (key);
}

static Bonobo_ServerStateCache *
impl_Bonobo_ObjectDirectory_get_active_servers (
        impl_POA_Bonobo_ObjectDirectory *servant,
        Bonobo_CacheTime                 only_if_newer,
        CORBA_Environment               *ev)
{
	Bonobo_ServerStateCache *retval;

	retval = Bonobo_ServerStateCache__alloc ();

	retval->_d = (only_if_newer < servant->time_active_changed);
	if (retval->_d) {
		StateCollectionInfo sci;

		retval->_u.active_servers._length =
			g_hash_table_size (servant->active_servers);
		retval->_u.active_servers._buffer = sci.seq =
			CORBA_sequence_Bonobo_ImplementationID_allocbuf
			(retval->_u.active_servers._length);
		sci.last_used = 0;

		g_hash_table_foreach (servant->active_servers,
				      (GHFunc) collate_active_server, &sci);
		CORBA_sequence_set_release (&(retval->_u.active_servers),
					    CORBA_TRUE);
	}

	return retval;
}

static CORBA_Object 
od_get_active_server (impl_POA_Bonobo_ObjectDirectory *servant,
                      Bonobo_ImplementationID          iid,
                      CORBA_Context                    ctx,
                      CORBA_Environment               *ev)
{
	CORBA_Object retval;
        char *display;
        char *display_iid;

        display = activation_server_CORBA_Context_get_value (
                ctx, "display", NULL, ev);
        
        if (display != NULL) {
                display_iid = g_strconcat (display, ",", iid, NULL);
                
                retval = g_hash_table_lookup (servant->active_servers, display_iid);

		g_free (display);
                g_free (display_iid);
                
                if (retval != CORBA_OBJECT_NIL &&
                    !CORBA_Object_non_existent (retval, ev))
                        return CORBA_Object_duplicate (retval, ev);
        }

        retval = g_hash_table_lookup (servant->active_servers, iid);
        
        if (retval != CORBA_OBJECT_NIL &&
            !CORBA_Object_non_existent (retval, ev))
                return CORBA_Object_duplicate (retval, ev);

        return CORBA_OBJECT_NIL;
}

static CORBA_Object
impl_Bonobo_ObjectDirectory_activate (
        impl_POA_Bonobo_ObjectDirectory *servant,
        Bonobo_ImplementationID          iid,
        Bonobo_ActivationContext         ac,
        Bonobo_ActivationFlags           flags,
        CORBA_Context                    ctx,
        CORBA_Environment               *ev)
{
	CORBA_Object retval;
	Bonobo_ServerInfo *si;
	ODActivationInfo ai;
        CORBA_Environment retry_ev;
#ifdef BONOBO_ACTIVATION_DEBUG
        static int depth = 0;
#endif

	retval = CORBA_OBJECT_NIL;

        update_registry (servant, FALSE);

        if (!(flags & Bonobo_ACTIVATION_FLAG_PRIVATE)) {
                retval = od_get_active_server (servant, iid, ctx, ev);

                if (retval != CORBA_OBJECT_NIL)
                        return retval;
        }

	if (flags & Bonobo_ACTIVATION_FLAG_EXISTING_ONLY) {
		return CORBA_OBJECT_NIL;
        }

#ifdef BONOBO_ACTIVATION_DEBUG
        {
                int i;
                depth++;
                for (i = 0; i < depth; i++)
                        fputc (' ', stderr);
                fprintf (stderr, "Activate '%s'\n", iid);
        }
#endif

	ai.ac = ac;
	ai.flags = flags;
	ai.ctx = ctx;

	si = g_hash_table_lookup (servant->by_iid, iid);

	if (si) {
		retval = od_server_activate (
                        si, &ai, servant->self, ev);

                /* If we failed to activate - it may be because our
                 * request re-entered _during_ the activation
                 * process resulting in a second process being started
                 * but failing to register - so we'll look up again here
                 * to see if we can get it.
                 * FIXME: we should not be forking redundant processes
                 * while an activation of that same process is on the
                 * stack.
                 * FIXME: we only get away with this hack because we
                 * try and fork another process & thus allow the reply
                 * from the initial process to be handled in the event
                 * loop.
                 */
                /* FIXME: this path is theoretically redundant now */
                if (ev->_major != CORBA_NO_EXCEPTION ||
                    retval == CORBA_OBJECT_NIL) {
                        CORBA_exception_init (&retry_ev);

                        retval = od_get_active_server (servant, iid, ctx, &retry_ev);

                        CORBA_exception_free (&retry_ev);
                        
                        if (retval != CORBA_OBJECT_NIL)
                                CORBA_exception_free (ev);
                }
        }

#ifdef BONOBO_ACTIVATION_DEBUG
        {
                int i;
                for (i = 0; i < depth; i++)
                        fputc (' ', stderr);
                fprintf (stderr, "Activated '%s' = %p\n", iid, retval);
                depth--;
        }
#endif

	return retval;
}

extern GMainLoop *main_loop;

static gboolean
quit_server_timeout (gpointer user_data)
{
#ifdef BONOBO_ACTIVATION_DEBUG
        g_warning ("Quit server !");
#endif

        if (!main_dir ||
            g_hash_table_size (main_dir->active_servers) > RESIDUAL_SERVERS ||
            !activation_clients_is_empty_scan ())
                g_warning ("Serious error handling server count, not quitting");
        else
                g_main_loop_quit (main_loop);

        main_dir->no_servers_timeout = 0;

        return FALSE;
}

void
check_quit (void)
{
        impl_POA_Bonobo_ObjectDirectory *servant = main_dir;

        /* We had some activity - so push out the shutdown timeout */
        if (servant->no_servers_timeout != 0)
                g_source_remove (servant->no_servers_timeout);
        servant->no_servers_timeout = 0;

        if (g_hash_table_size (servant->active_servers) <= RESIDUAL_SERVERS &&
            activation_clients_is_empty_scan ())
                servant->no_servers_timeout = g_timeout_add (
                        SERVER_IDLE_QUIT_TIMEOUT, quit_server_timeout, NULL);

	servant->time_active_changed = time (NULL);
}

static gboolean
prune_dead_servers (gpointer key,
                    gpointer value,
                    gpointer user_data)
{
        gboolean dead;
        ORBitConnectionStatus status;

        status = ORBit_small_get_connection_status (value);

        dead = (status == ORBIT_CONNECTION_DISCONNECTED);

#ifdef BONOBO_ACTIVATION_DEBUG
        fprintf (stderr, "IID '%20s', %s\n", (char *) key,
                 dead ? "dead" : "alive");
#endif
        
        return dead;
}

static void
active_server_cnx_broken (ORBitConnection *cnx,
                          gpointer         dummy)

{
        impl_POA_Bonobo_ObjectDirectory *servant = main_dir;

        if (!servant) /* shutting down */
                return;

        g_hash_table_foreach_remove (servant->active_servers,
                                     prune_dead_servers, servant);
#ifdef BONOBO_ACTIVATION_DEBUG
        g_warning ("After prune: %d live servers",
                   g_hash_table_size (
                           servant->active_servers) - RESIDUAL_SERVERS);
#endif

        check_quit ();
}

static void
add_active_server (impl_POA_Bonobo_ObjectDirectory *servant,
                   const char                      *iid,
                   CORBA_Object                     object)
{
        ORBitConnection *cnx;

        cnx = ORBit_small_get_connection (object);
        if (cnx) {
                if (!g_object_get_data (G_OBJECT (cnx), "object_count")) {
                        g_object_set_data (
                                G_OBJECT (cnx), "object_count", GUINT_TO_POINTER (1));
                        
                        g_signal_connect (
                                cnx, "broken",
                                G_CALLBACK (active_server_cnx_broken),
                                NULL);
                }
        } else
                g_assert (!strcmp (iid, NAMING_CONTEXT_IID));

	g_hash_table_replace (servant->active_servers,
                              g_strdup (iid),
                              CORBA_Object_duplicate (object, NULL));

        if (cnx)
                check_quit ();
}

static void
active_server_free (gpointer active_server)
{
        CORBA_Object_release (active_server, NULL);
}

static void
remove_active_server (impl_POA_Bonobo_ObjectDirectory *servant,
                      const char                      *iid,
                      CORBA_Object                     object)
{
	g_hash_table_remove (servant->active_servers, iid);

        check_quit ();
}

static Bonobo_RegistrationResult
impl_Bonobo_ObjectDirectory_register_new (
        impl_POA_Bonobo_ObjectDirectory *servant,
        Bonobo_ImplementationID          iid,
        CORBA_Object                     obj,
        CORBA_Environment               *ev)
{
	CORBA_Object oldobj;
        Bonobo_ImplementationID actual_iid;

	oldobj = g_hash_table_lookup (servant->active_servers, iid);

	if (oldobj != CORBA_OBJECT_NIL) {
		if (!CORBA_Object_non_existent (oldobj, ev))
			return Bonobo_ACTIVATION_REG_ALREADY_ACTIVE;
	}

        actual_iid = strrchr (iid, ',');
        if (actual_iid == NULL) {
                actual_iid = iid;
        } else {
                actual_iid++;
        }

	if (!g_hash_table_lookup (servant->by_iid, actual_iid))
		return Bonobo_ACTIVATION_REG_NOT_LISTED;

#ifdef BONOBO_ACTIVATION_DEBUG
        g_warning ("Server register. '%s' : %p", iid, obj);
#endif

        add_active_server (servant, iid, obj);

	return Bonobo_ACTIVATION_REG_SUCCESS;
}

static void
impl_Bonobo_ObjectDirectory_unregister (
        impl_POA_Bonobo_ObjectDirectory      *servant,
        Bonobo_ImplementationID               iid,
        CORBA_Object                          obj,
        CORBA_Environment                    *ev)
{
	CORBA_Object orig_object;

        orig_object = g_hash_table_lookup (servant->active_servers, iid);
        
        if (orig_object == CORBA_OBJECT_NIL ||
	    !CORBA_Object_is_equivalent (orig_object, obj, ev)) {
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
                                     ex_Bonobo_ObjectDirectory_NotRegistered,
                                     NULL);
                return;
        }

        remove_active_server (servant, iid, obj);
}

static CORBA_char *
impl_Bonobo_ObjectDirectory__get_domain (
        impl_POA_Bonobo_ObjectDirectory *servant,
        CORBA_Environment               *ev)
{
	return CORBA_string_dup (servant->attr_domain);
}

static CORBA_char *
impl_Bonobo_ObjectDirectory__get_hostID (
        impl_POA_Bonobo_ObjectDirectory *servant,
        CORBA_Environment               *ev)
{
        return CORBA_string_dup (bonobo_activation_hostname_get ());
}

static CORBA_char *
impl_Bonobo_ObjectDirectory__get_username (
        impl_POA_Bonobo_ObjectDirectory *servant,
        CORBA_Environment               *ev)
{
	return CORBA_string_dup (g_get_user_name ());
}

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_Bonobo_ObjectDirectory_base_epv = {
	NULL,			/* _private data */
	NULL,			/* finalize routine */
	NULL			/* default_POA routine */
};

/* FIXME: fill me in / deal with me globaly */
static POA_Bonobo_Unknown__epv impl_Bonobo_Unknown_epv = {
	NULL,			/* _private data */
	NULL,
	NULL,
        NULL
};

static POA_Bonobo_ObjectDirectory__epv impl_Bonobo_ObjectDirectory_epv = {
	NULL,			/* _private */
	(gpointer) &impl_Bonobo_ObjectDirectory__get_servers,
	(gpointer) &impl_Bonobo_ObjectDirectory_get_active_servers,
	(gpointer) &impl_Bonobo_ObjectDirectory__get_username,
	(gpointer) &impl_Bonobo_ObjectDirectory__get_hostID,
	(gpointer) &impl_Bonobo_ObjectDirectory__get_domain,
	(gpointer) &impl_Bonobo_ObjectDirectory_activate,
	(gpointer) &impl_Bonobo_ObjectDirectory_register_new,
	(gpointer) &impl_Bonobo_ObjectDirectory_unregister
};

/*** vepv structures ***/

static POA_Bonobo_ObjectDirectory__vepv impl_Bonobo_ObjectDirectory_vepv = {
	&impl_Bonobo_ObjectDirectory_base_epv,
	&impl_Bonobo_Unknown_epv,
	&impl_Bonobo_ObjectDirectory_epv
};

Bonobo_ObjectDirectory
bonobo_object_directory_get (void)
{
        if (!main_dir)
                return CORBA_OBJECT_NIL;
        else
                return main_dir->self;
}

void
bonobo_object_directory_init (PortableServer_POA poa,
                              const char        *domain,
                              const char        *registry_path,
                              CORBA_Environment *ev)
{
	PortableServer_ObjectId         *objid;
	impl_POA_Bonobo_ObjectDirectory *servant;

        g_assert (main_dir == NULL);

	servant = g_new0 (impl_POA_Bonobo_ObjectDirectory, 1);

        main_dir = servant; /* DON'T FIXME: this is good. */

	servant->servant.vepv = &impl_Bonobo_ObjectDirectory_vepv;
	POA_Bonobo_ObjectDirectory__init ((PortableServer_Servant) servant,
				       ev);
	objid = PortableServer_POA_activate_object (poa, servant, ev);
	CORBA_free (objid);
	servant->self =
		PortableServer_POA_servant_to_reference (poa, servant, ev);

	servant->attr_domain = g_strdup (domain);
	servant->by_iid = NULL;

        servant->registry_source_directories = split_path_unique (registry_path);
        servant->registry_directory_mtimes = g_hash_table_new (g_str_hash, g_str_equal);

        update_registry (servant, FALSE);

        servant->active_servers =
                g_hash_table_new_full (g_str_hash, g_str_equal,
                                       g_free, active_server_free);
        servant->no_servers_timeout = 0;
}

void
bonobo_object_directory_shutdown (PortableServer_POA poa,
                                  CORBA_Environment *ev)
{
	PortableServer_ObjectId *oid;
	impl_POA_Bonobo_ObjectDirectory *servant = main_dir;

	oid = PortableServer_POA_servant_to_id (poa, servant, ev);
	PortableServer_POA_deactivate_object (poa, oid, ev);
	CORBA_free (oid);

        main_dir = NULL;

        CORBA_Object_release (servant->self, ev);
        g_hash_table_destroy (servant->active_servers);
        servant->active_servers = NULL;
        g_hash_table_destroy (servant->registry_directory_mtimes);
        servant->registry_directory_mtimes = NULL;
        g_strfreev (servant->registry_source_directories);

        g_free (servant);
}

CORBA_Object
bonobo_object_directory_re_check_fn (const char        *display,
                                     const char        *act_iid,
                                     gpointer           user_data,
                                     CORBA_Environment *ev)
{
        CORBA_Object retval;
        ODActivationInfo *info = user_data;

        retval = od_get_active_server (
                main_dir, (Bonobo_ImplementationID) act_iid, info->ctx, ev);

        if (ev->_major != CORBA_NO_EXCEPTION ||
            retval == CORBA_OBJECT_NIL) {
                char *msg;
		Bonobo_GeneralError *errval = Bonobo_GeneralError__alloc ();

                CORBA_exception_free (ev);

                /*
                 * If this exception blows ( which it will only do with a multi-object )
                 * factory, you need to ensure you register the object you were activated
                 * for [use const char *bonobo_activation_iid_get (void); ] is registered
                 * with bonobo_activation_active_server_register - _after_ any other
                 * servers are registered.
                 */
                msg = g_strdup_printf (_("Race condition activating server '%s'"), act_iid);
                errval->description = CORBA_string_dup (msg);
                g_free (msg);

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_GeneralError, errval);
                retval = CORBA_OBJECT_NIL;
        }

        return retval;
}

void
bonobo_object_directory_reload (void)
{
#ifdef BONOBO_ACTIVATION_DEBUG
        g_print ("reloading our object directory!\n");
#endif
        update_registry (main_dir, TRUE);
}
