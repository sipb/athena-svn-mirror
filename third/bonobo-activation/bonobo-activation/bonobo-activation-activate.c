/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 */
#include <config.h>
#include <bonobo-activation/bonobo-activation-activate.h>

#include <bonobo-activation/bonobo-activation-activate-private.h>
#include <bonobo-activation/bonobo-activation-id.h>
#include <bonobo-activation/bonobo-activation-init.h>
#include <bonobo-activation/bonobo-activation-server-info.h>
#include <bonobo-activation/bonobo-activation-private.h>
#include <bonobo-activation/bonobo-activation-shlib.h>
#include <bonobo-activation/bonobo-activation-client.h>
#include <bonobo-activation/Bonobo_ActivationContext.h>

#include <string.h>

static gboolean test_components_enabled = FALSE;


/**
 * bonobo_activation_set_test_components_enabled:
 * @val: if TRUE, enable test components. If FALSE, disable them.
 * 
 * Enable test components.
 */
void
bonobo_activation_set_test_components_enabled (gboolean val)
{
        test_components_enabled = val;
}

/**
 * bonobo_activation_get_test_components_enabled:
 * 
 * Return value: returns whether or not the 
 *               test components are enabled.
 */
gboolean
bonobo_activation_get_test_components_enabled (void)
{
        return test_components_enabled;
}

/* internal function.*/
char *
bonobo_activation_maybe_add_test_requirements (const char *requirements) 
{
        char *ext_requirements;

        if (!bonobo_activation_get_test_components_enabled ()) {
                ext_requirements = g_strconcat ("( ", requirements,
                                                " ) AND (NOT test_only.defined() OR NOT test_only)",
                                                NULL);
        } else {
                ext_requirements = NULL;
        }

        return ext_requirements;
}


/* internal funtion */
void 
bonobo_activation_copy_string_array_to_Bonobo_StringList (char *const *selection_order, Bonobo_StringList *ret_val)
{
        int i;

	if (selection_order) {
		for (i = 0; selection_order[i]; i++)
			/**/;

		ret_val->_length = i;
		ret_val->_buffer = (char **) selection_order;
		CORBA_sequence_set_release (ret_val, CORBA_FALSE);
	} else {
		memset (ret_val, 0, sizeof (*ret_val));
        }
}

/* Limit of the number of cached queries */
#define QUERY_CACHE_MAX 32
#undef QUERY_CACHE_DEBUG

static GHashTable *query_cache = NULL;

typedef struct {
	char  *query;
	char **sort_criteria;

	Bonobo_ServerInfoList *list;
} QueryCacheEntry;

static void
query_cache_entry_free (gpointer data)
{
        QueryCacheEntry *entry = data;

#ifdef QUERY_CACHE_DEBUG
        g_warning ("Blowing item %p", entry);
#endif /* QUERY_CACHE_DEBUG */

        g_free (entry->query);
        g_strfreev (entry->sort_criteria);
        CORBA_free (entry->list);
        g_free (entry);
}

static gboolean
cache_clean_half (gpointer  key,
                  gpointer  value,
                  gpointer  user_data)
{
        int *a = user_data;
        /* Blow half the elements */
        return (*a)++ % 2;
}

static gboolean
query_cache_equal (gconstpointer a, gconstpointer b)
{
	int i;
	char **strsa, **strsb;
	const QueryCacheEntry *entrya = a;
	const QueryCacheEntry *entryb = b;

	if (strcmp (entrya->query, entryb->query))
		return FALSE;

	strsa = entrya->sort_criteria;
	strsb = entryb->sort_criteria;

	if (!strsa && !strsb)
		return TRUE;

	if (!strsa || !strsb)
		return FALSE;

	for (i = 0; strsa [i] && strsb [i]; i++)
		if (strcmp (strsa [i], strsb [i]))
			return FALSE;

	if (strsa [i] || strsb [i])
		return FALSE;

	return TRUE;
}

static guint
query_cache_hash (gconstpointer a)
{
	guint hash, i;
	char **strs;
	const QueryCacheEntry *entry = a;
	
	hash = g_str_hash (entry->query);
	strs = entry->sort_criteria;

	for (i = 0; strs && strs [i]; i++)
		hash ^= g_str_hash (strs [i]);

	return hash;
}

static void
query_cache_reset (void)
{
        if (query_cache) {
                g_hash_table_destroy (query_cache);
                query_cache = NULL;
        }
}

static void
create_query_cache (void)
{
        query_cache = g_hash_table_new_full (
                query_cache_hash,
                query_cache_equal,
                query_cache_entry_free,
                NULL);
        bonobo_activation_add_reset_notify (query_cache_reset);
}

static Bonobo_ServerInfoList *
query_cache_lookup (char         *query,
		    char * const *sort_criteria)
{
	QueryCacheEntry  fake;
	QueryCacheEntry *entry;

	if (!query_cache) {
                create_query_cache ();
		return NULL;
	}

	fake.query = query;
	fake.sort_criteria = (char **) sort_criteria;
	if ((entry = g_hash_table_lookup (query_cache, &fake))) {
#ifdef QUERY_CACHE_DEBUG
		g_warning ("\n\n ---  Hit (%p)  ---\n\n\n", entry->list);
#endif /* QUERY_CACHE_DEBUG */
		return Bonobo_ServerInfoList_duplicate (entry->list);
	} else {
#ifdef QUERY_CACHE_DEBUG
		g_warning ("Miss");
#endif /* QUERY_CACHE_DEBUG */
		return NULL;
	}
}

static void
query_cache_insert (const char   *query,
		    char * const *sort_criteria,
		    Bonobo_ServerInfoList *list)
{
        int idx = 0;
	QueryCacheEntry *entry = g_new (QueryCacheEntry, 1);

        if (!query_cache) {
                create_query_cache ();
        
        } else if (g_hash_table_size (query_cache) > QUERY_CACHE_MAX) {
                g_hash_table_foreach_remove (
                        query_cache, cache_clean_half, &idx);
        }

	entry->query = g_strdup (query);
	entry->sort_criteria = g_strdupv ((char **) sort_criteria);
	entry->list = Bonobo_ServerInfoList_duplicate (list);

	g_hash_table_replace (query_cache, entry, entry);

#ifdef QUERY_CACHE_DEBUG
	g_warning ("Query cache size now %d",
                g_hash_table_size (query_cache));
#endif /* QUERY_CACHE_DEBUG */
}

/**
 * bonobo_activation_query: 
 * @requirements: query string.
 * @selection_order: sort criterion for returned list.
 * @ev: a %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation, or NULL
 *
 * Executes the @requirements query on the bonobo-activation-server.
 * The result is sorted according to @selection_order. 
 * @selection_order can safely be NULL as well as @ev.
 * The returned list has to be freed with CORBA_free.
 *
 * Return value: the list of servers matching the requirements.
 */
Bonobo_ServerInfoList *
bonobo_activation_query (const char   *requirements,
                         char * const *selection_order,
                         CORBA_Environment *ev)
{
	Bonobo_StringList selorder;
	Bonobo_ServerInfoList *res;
	CORBA_Environment myev;
	Bonobo_ActivationContext ac;
        char *ext_requirements;
        char *query_requirements;

	g_return_val_if_fail (requirements, CORBA_OBJECT_NIL);
	ac = bonobo_activation_activation_context_get ();
	g_return_val_if_fail (ac, CORBA_OBJECT_NIL);

        ext_requirements = bonobo_activation_maybe_add_test_requirements (requirements);

        if (ext_requirements == NULL) {
                query_requirements = (char *) requirements;
        } else {
                query_requirements = (char *) ext_requirements;
        } 

	if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

	res = query_cache_lookup (query_requirements, selection_order);

        if (!res) {
                bonobo_activation_copy_string_array_to_Bonobo_StringList (selection_order, &selorder);

                res = Bonobo_ActivationContext_query (
                        ac, query_requirements,
                        &selorder, bonobo_activation_context_get (), ev);

                if (ev->_major != CORBA_NO_EXCEPTION) {
                        res = NULL;
                }

                query_cache_insert (query_requirements, selection_order, res);
        }

        if (ext_requirements != NULL) {
                g_free (ext_requirements);
        }

	if (ev == &myev) {
		CORBA_exception_free (&myev);
        }

	return res;
}


/**
 * bonobo_activation_activate:
 * @requirements: query string.
 * @selection_order: sort criterion for returned list.
 * @flags: how to activate the object.
 * @ret_aid: AID of the activated object.
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Activates a given object. @ret_aid can be safely NULLed as well
 * as @ev and @selection_order. @flags can be set to zero if you do 
 * not what to use.
 *
 * Return value: the CORBA object reference of the activated object.
 *               This value can be CORBA_OBJECT_NIL: you are supposed 
 *               to check @ev for success.
 */
CORBA_Object
bonobo_activation_activate (const char *requirements, char *const *selection_order,
                            Bonobo_ActivationFlags flags, Bonobo_ActivationID * ret_aid,
                            CORBA_Environment * ev)
{
	Bonobo_StringList selorder;
	CORBA_Object retval;
	Bonobo_ActivationResult *res;
	CORBA_Environment myev;
	Bonobo_ActivationContext ac;
        char *ext_requirements;

        retval = CORBA_OBJECT_NIL;

	g_return_val_if_fail (requirements, CORBA_OBJECT_NIL);
	ac = bonobo_activation_activation_context_get ();
	g_return_val_if_fail (ac, CORBA_OBJECT_NIL);

        ext_requirements = bonobo_activation_maybe_add_test_requirements (requirements);

	if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

        bonobo_activation_copy_string_array_to_Bonobo_StringList (selection_order, &selorder);

        if (ext_requirements == NULL) {
                res = Bonobo_ActivationContext_activate (ac, (char *) requirements,
                                                      &selorder, flags,
                                                      bonobo_activation_context_get (), ev);
        } else {
                res = Bonobo_ActivationContext_activate (ac, (char *) ext_requirements,
                                                      &selorder, flags,
                                                      bonobo_activation_context_get (), ev);
        }

        if (ext_requirements != NULL) {
                g_free (ext_requirements);
        }

	if (ev->_major != CORBA_NO_EXCEPTION) {
                if (ev == &myev) {
                        CORBA_exception_free (&myev);
                }
                
                return retval;
        }


	switch (res->res._d) {
	case Bonobo_ACTIVATION_RESULT_SHLIB:
		retval = bonobo_activation_activate_shlib_server (res, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (res->res._u.res_object, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_NONE:
	default:
		break;
	}

	if (ret_aid) {
		*ret_aid = NULL;
		if (*res->aid)
			*ret_aid = g_strdup (res->aid);
	}

	CORBA_free (res);

	if (ev == &myev)
		CORBA_exception_free (&myev);


	return retval;
}

/**
 * bonobo_activation_activate_from_id
 * @aid: AID or IID of the object to activate.
 * @flags: activation flag.
 * @ret_aid: AID of the activated server.
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Activates the server corresponding to @aid. @ret_aid can be safely 
 * NULLed as well as @ev. @flags can be zero if you do not know what 
 * to do.
 *
 * Return value: a CORBA object reference to the newly activated 
 *               server. Do not forget to check @ev for failure!!
 */

CORBA_Object
bonobo_activation_activate_from_id (const Bonobo_ActivationID aid, 
                                    Bonobo_ActivationFlags    flags,
                                    Bonobo_ActivationID      *ret_aid,
                                    CORBA_Environment        *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
        Bonobo_ActivationResult *res;
	CORBA_Environment myev;
	Bonobo_ActivationContext ac;
	BonoboActivationInfo *ai;

	g_return_val_if_fail (aid, CORBA_OBJECT_NIL);

        if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

        ac = bonobo_activation_internal_activation_context_get_extended (
                (flags & Bonobo_ACTIVATION_FLAG_EXISTING_ONLY) != 0, ev);

        if (ac == CORBA_OBJECT_NIL)
                goto out;

	ai = bonobo_activation_id_parse (aid);

 	if (ai != NULL) {		
                /* This is so that using an AID in an unactivated OD will work nicely */
                bonobo_activation_object_directory_get (ai->user, ai->host, ai->domain);

		bonobo_activation_info_free (ai);
	}

        res = Bonobo_ActivationContext_activate_from_id (
                ac, aid, flags, bonobo_activation_context_get (), ev);
        
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto out;

	switch (res->res._d) {
	case Bonobo_ACTIVATION_RESULT_SHLIB:
                retval = bonobo_activation_activate_shlib_server (
                        (Bonobo_ActivationResult *) res, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (res->res._u.res_object, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_NONE:
	default:
		break;
	}

	if (ret_aid) {
		*ret_aid = NULL;
		if (*res->aid)
			*ret_aid = g_strdup (res->aid);
	}

        CORBA_free (res);

      out:
	if (ev == &myev)
		CORBA_exception_free (&myev);

	return retval;
}

/**
 * bonobo_activation_name_service_get:
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Returns the name server of bonobo-activation. @ev can be NULL.
 *
 * Return value: the name server of bonobo-activation.
 */
CORBA_Object
bonobo_activation_name_service_get (CORBA_Environment * ev)
{
	return bonobo_activation_activate_from_id (
                "OAFIID:Bonobo_CosNaming_NamingContext", 0, NULL, ev);
}
