#define _E_CACHE_C_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include "e-cache.h"

#define E_CACHE_VERBOSE

typedef struct _ECacheEntry ECacheEntry;

struct _ECache {
	gint refcount;

	ECacheDupFunc key_dup_func;
	ECacheFreeFunc key_free_func;
	ECacheFreeFunc object_free_func;

	GHashTable * dict;
	ECacheEntry * first, * last;

	size_t softlimit;
	size_t hardlimit;
	size_t size;
};

struct _ECacheEntry {
	ECacheEntry * prev, * next;
	gpointer key;
	gpointer data;
	size_t size;
};

static void e_cache_forget_last (ECache * cache);
static void e_cache_forget_entry (ECache * cache, ECacheEntry * e);

ECache * e_cache_new (GHashFunc key_hash_func,
		      GCompareFunc key_compare_func,
		      ECacheDupFunc key_dup_func,
		      ECacheFreeFunc key_free_func,
		      ECacheFreeFunc object_free_func,
		      size_t softlimit,
		      size_t hardlimit)
{
	ECache * cache;

	cache = g_new (ECache, 1);

	cache->refcount = 1;
	cache->key_dup_func = key_dup_func;
	cache->key_free_func = key_free_func;
	cache->object_free_func = object_free_func;

	cache->dict = g_hash_table_new (key_hash_func, key_compare_func);

	cache->first = cache->last = NULL;

	cache->softlimit = softlimit;
	cache->hardlimit = hardlimit;
	cache->size = 0;

	return cache;
}

void
e_cache_ref (ECache * cache)
{
	g_return_if_fail (cache != NULL);

	cache->refcount++;
}

void
e_cache_unref (ECache * cache)
{
	g_return_if_fail (cache != NULL);

	cache->refcount--;

	if (cache->refcount < 1) {
#ifdef E_CACHE_VERBOSE
		g_print ("Destroying cache\n");
#endif
		while (cache->last) {
			e_cache_forget_last (cache);
		}
		g_hash_table_destroy (cache->dict);
		g_free (cache);
	}
}

gpointer
e_cache_lookup (ECache * cache, gconstpointer key)
{
	ECacheEntry * e;

	g_return_val_if_fail (cache != NULL, NULL);

	e = g_hash_table_lookup (cache->dict, key);

	if (e == NULL) return NULL;

	if (e != cache->first) {
		if (e->prev) {
			e->prev->next = e->next;
		} else {
			g_assert_not_reached ();
		}
		if (e->next) {
			e->next->prev = e->prev;
		} else {
			g_assert (e == cache->last);
			cache->last = e->prev;
		}
		e->next = cache->first;
		e->next->prev = e;
		e->prev = NULL;
		cache->first = e;
	}

#ifdef E_CACHE_VERBOSE
		g_print ("Found cched object: %ld bytes\n", (long)e->size);
#endif

	return e->data;
}

gpointer
e_cache_lookup_notouch (ECache * cache, gconstpointer key)
{
	ECacheEntry * e;

	g_return_val_if_fail (cache != NULL, NULL);

	e = g_hash_table_lookup (cache->dict, key);

	if (e == NULL) return NULL;

	return e->data;
}

gboolean
e_cache_insert (ECache * cache, gpointer key, gpointer data, size_t size)
{
	ECacheEntry * e;

	g_return_val_if_fail (cache != NULL, FALSE);

	if (size > cache->hardlimit) return FALSE;

	e = g_hash_table_lookup (cache->dict, key);

	if (e) e_cache_forget_entry (cache, e);

	while (cache->size + size > cache->hardlimit) {
		g_assert (cache->last);
		e_cache_forget_last (cache);
	}

	e = g_new (ECacheEntry, 1);

	if (cache->key_dup_func) {
		e->key = (* cache->key_dup_func) (key);
	} else {
		e->key = key;
	}

	e->data = data;
	e->size = size;

	e->next = cache->first;
	if (e->next) e->next->prev = e;
	e->prev = NULL;
	cache->first = e;
	if (!cache->last) cache->last = e;

	cache->size += size;

	g_hash_table_insert (cache->dict, e->key, e);

#ifdef E_CACHE_VERBOSE
		g_print ("Inserted object, cache size now %ld\n", (long) cache->size);
#endif

	return TRUE;
}

void
e_cache_invalidate (ECache * cache, gpointer key)
{
	ECacheEntry * e;

	g_return_if_fail (cache != NULL);

	e = g_hash_table_lookup (cache->dict, key);

	g_return_if_fail (e != NULL);

	e_cache_forget_entry (cache, e);
}

void
e_cache_invalidate_all (ECache * cache)
{
	g_return_if_fail (cache != NULL);

	while (cache->last) {
		e_cache_forget_last (cache);
	}
}

size_t
e_cache_query_object_size (ECache * cache, gconstpointer key)
{
	ECacheEntry * e;

	g_return_val_if_fail (cache != NULL, 0);

	e = g_hash_table_lookup (cache->dict, key);

	g_return_val_if_fail (e != NULL, 0);

	return e->size;
}

static void
e_cache_forget_last (ECache * cache)
{
	g_assert (cache->last);

	e_cache_forget_entry (cache, cache->last);
}

static void
e_cache_forget_entry (ECache * cache, ECacheEntry * e)
{
	g_assert (cache != NULL);
	g_assert (e != NULL);

	if (e->prev) {
		e->prev->next = e->next;
	} else {
		g_assert (e == cache->first);
		cache->first = e->next;
	}
	if (e->next) {
		e->next->prev = e->prev;
	} else {
		g_assert (e == cache->last);
		cache->last = e->prev;
	}

	g_hash_table_remove (cache->dict, e->key);

	if (cache->object_free_func) (* cache->object_free_func) (e->data);
	if (cache->key_free_func) (* cache->key_free_func) (e->key);

	cache->size -= e->size;

	g_free (e);

#ifdef E_CACHE_VERBOSE
		g_print ("Removed object, cache size now %ld\n", (long) cache->size);
#endif
}




