/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* vi: set ts=8 sts=0 sw=8 tw=80 noexpandtab: */
/* gnome-vfs-mime-info.c - GNOME xdg mime information implementation.

 Copyright (C) 2004 Red Hat, Inc
 All rights reserved.
 
 The Gnome Library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public License as
 published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.
  
 The Gnome Library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 
 You should have received a copy of the GNU Library General Public
 License along with the Gnome Library; see the file COPYING.LIB.  If not,
 write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <glib.h>
#include "gnome-vfs-mime.h"
#include "gnome-vfs-mime-info-cache.h"
#include "gnome-vfs-mime-info.h"
#include "gnome-vfs-mime-monitor.h"
#include "gnome-vfs-monitor.h"
#include "gnome-vfs-ops.h"
#include "gnome-vfs-utils.h"
#include "eggdesktopentries.h"
#include "eggdirfuncs.h"

typedef struct {
	char *path;
	GHashTable *mime_info_cache_map;
	GHashTable *defaults_list_map;
	GnomeVFSMonitorHandle  *cache_monitor_handle;
	GnomeVFSMonitorHandle  *defaults_monitor_handle;

	time_t mime_info_cache_timestamp;
	time_t defaults_list_timestamp;
} GnomeVFSMimeInfoCacheDir;

typedef struct {
	GList *dirs;               /* mimeinfo.cache and defaults.list */
	GHashTable *global_defaults_cache; /* global results of defaults.list lookup and validation */
        time_t last_stat_time;
	guint should_ping_mime_monitor : 1;
} GnomeVFSMimeInfoCache;

extern void _gnome_vfs_mime_monitor_emit_data_changed (GnomeVFSMIMEMonitor *monitor); 

static void gnome_vfs_mime_info_cache_dir_init (GnomeVFSMimeInfoCacheDir *dir);
static void gnome_vfs_mime_info_cache_dir_init_defaults_list (GnomeVFSMimeInfoCacheDir *dir);
static GnomeVFSMimeInfoCacheDir *gnome_vfs_mime_info_cache_dir_new (const char *path);
static void gnome_vfs_mime_info_cache_dir_free (GnomeVFSMimeInfoCacheDir *dir);
static char **gnome_vfs_mime_info_cache_get_search_path (void);

static gboolean gnome_vfs_mime_info_cache_dir_desktop_entry_is_valid (GnomeVFSMimeInfoCacheDir *dir,
								      const char *desktop_entry);
static void gnome_vfs_mime_info_cache_dir_add_desktop_entries (GnomeVFSMimeInfoCacheDir *dir,
							       const char *mime_type,
							       char **new_desktop_file_ids);
static void gnome_vfs_mime_info_cache_init (void);
static GnomeVFSMimeInfoCache *gnome_vfs_mime_info_cache_new (void);
static void gnome_vfs_mime_info_cache_free (GnomeVFSMimeInfoCache *cache);

static GnomeVFSMimeInfoCache *mime_info_cache = NULL;
G_LOCK_DEFINE_STATIC (mime_info_cache);


static void
destroy_info_cache_value (gpointer key, GList *value, gpointer data)
{
	gnome_vfs_list_deep_free (value);
}

static void
destroy_info_cache_map (GHashTable *info_cache_map)
{
	g_hash_table_foreach (info_cache_map, (GHFunc)destroy_info_cache_value,
			      NULL);
	g_hash_table_destroy (info_cache_map);
}

static gboolean
gnome_vfs_mime_info_cache_dir_out_of_date (GnomeVFSMimeInfoCacheDir *dir, const char *cache_file,
		                           time_t *timestamp)
{
	struct stat buf;
	char *filename;

	filename = g_build_filename (dir->path, cache_file, NULL);

	if (stat (filename, &buf) < 0) {
		g_free (filename);
		return TRUE;
	}
	g_free (filename);

	if (buf.st_mtime != *timestamp) 
		return TRUE;

	return FALSE;
}

/* Call with lock held */
static gboolean remove_all (gpointer  key,
			    gpointer  value,
			    gpointer  user_data)
{
	return TRUE;
}


static void
gnome_vfs_mime_info_cache_blow_global_cache (void)
{
	g_hash_table_foreach_remove (mime_info_cache->global_defaults_cache,
				     remove_all, NULL);
}

static void
gnome_vfs_mime_info_cache_dir_init (GnomeVFSMimeInfoCacheDir *dir)
{
	EggDesktopEntries *entries;
	GError *load_error;
	gchar *filename, **mime_types;
	int i;
	static gchar *allowed_start_groups[] = { "MIME Cache", NULL };
	struct stat buf;

	load_error = NULL;
	mime_types = NULL;
	entries = NULL;

	if (dir->mime_info_cache_map != NULL &&
	    dir->cache_monitor_handle == NULL &&
  	    !gnome_vfs_mime_info_cache_dir_out_of_date (dir, "mimeinfo.cache",
		                                        &dir->mime_info_cache_timestamp))
		return;

	if (dir->mime_info_cache_map != NULL) {
		destroy_info_cache_map (dir->mime_info_cache_map);
	}

	dir->mime_info_cache_map = g_hash_table_new_full (g_str_hash, g_str_equal,
							  (GDestroyNotify) g_free,
							  NULL);

	filename = g_build_filename (dir->path, "mimeinfo.cache", NULL);

	if (stat (filename, &buf) < 0)
		goto error;

	if (dir->mime_info_cache_timestamp > 0) 
		mime_info_cache->should_ping_mime_monitor = TRUE;

	dir->mime_info_cache_timestamp = buf.st_mtime;

	entries =
		egg_desktop_entries_new_from_file (allowed_start_groups,
						   EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP |
						   EGG_DESKTOP_ENTRIES_DISCARD_COMMENTS |
						   EGG_DESKTOP_ENTRIES_DISCARD_TRANSLATIONS,
						   filename,
						   &load_error);
	g_free (filename);
	filename = NULL;

	if (load_error != NULL)
		goto error;

	mime_types = egg_desktop_entries_get_keys (entries, "MIME Cache",
						   NULL, &load_error);

	if (load_error != NULL)
		goto error;

	for (i = 0; mime_types[i] != NULL; i++) {
		gchar **desktop_file_ids;
		desktop_file_ids = egg_desktop_entries_get_string_list (entries,
									"MIME Cache",
									mime_types[i],
									NULL,
									&load_error);

		if (load_error != NULL) {
			g_error_free (load_error);
			load_error = NULL;
			continue;
		}

		gnome_vfs_mime_info_cache_dir_add_desktop_entries (dir,
								   mime_types[i],
								   desktop_file_ids);

		g_strfreev (desktop_file_ids);
	}

	g_strfreev (mime_types);
	egg_desktop_entries_free (entries);

	return;
error:
	if (filename)
		g_free (filename);

	if (entries != NULL)
		egg_desktop_entries_free (entries);

	if (mime_types != NULL)
		g_strfreev (mime_types);

	if (load_error)
		g_error_free (load_error);
}

static void
gnome_vfs_mime_info_cache_dir_init_defaults_list (GnomeVFSMimeInfoCacheDir *dir)
{
	EggDesktopEntries *entries;
	GError *load_error;
	gchar *filename, **mime_types;
	char **desktop_file_ids;
	int i;
	struct stat buf;
	static gchar *allowed_start_groups[] = { "Default Applications", NULL };

	load_error = NULL;
	mime_types = NULL;
	entries = NULL;

	if (dir->defaults_list_map != NULL &&
	    dir->defaults_monitor_handle == NULL &&
  	    !gnome_vfs_mime_info_cache_dir_out_of_date (dir, "defaults.list",
		                                        &dir->defaults_list_timestamp))
		return;

	if (dir->defaults_list_map != NULL) {
		g_hash_table_destroy (dir->defaults_list_map);
	}

	dir->defaults_list_map = g_hash_table_new_full (g_str_hash, g_str_equal,
							g_free, (GDestroyNotify)g_strfreev);

	filename = g_build_filename (dir->path, "defaults.list", NULL);

	if (stat (filename, &buf) < 0)
		goto error;

	if (dir->defaults_list_timestamp > 0) 
		mime_info_cache->should_ping_mime_monitor = TRUE;

	dir->defaults_list_timestamp = buf.st_mtime;

	entries =
		egg_desktop_entries_new_from_file (allowed_start_groups,
						   EGG_DESKTOP_ENTRIES_GENERATE_LOOKUP_MAP |
						   EGG_DESKTOP_ENTRIES_DISCARD_COMMENTS |
						   EGG_DESKTOP_ENTRIES_DISCARD_TRANSLATIONS,
						   filename,
						   &load_error);
	g_free (filename);
	filename = NULL;

	if (load_error != NULL)
		goto error;

	mime_types = egg_desktop_entries_get_keys (entries, "Default Applications",
						   NULL, &load_error);

	if (load_error != NULL)
		goto error;

	for (i = 0; mime_types[i] != NULL; i++) {
		desktop_file_ids = egg_desktop_entries_get_string_list (entries,
								       "Default Applications",
								       mime_types[i],
								       NULL,
								       &load_error);
		if (load_error != NULL) {
			g_error_free (load_error);
			load_error = NULL;
			continue;
		}
		
		g_hash_table_replace (dir->defaults_list_map,
				      g_strdup (mime_types[i]),
				      desktop_file_ids);
	}

	g_strfreev (mime_types);
	egg_desktop_entries_free (entries);

	return;
error:
	if (filename)
		g_free (filename);

	if (entries != NULL)
		egg_desktop_entries_free (entries);

	if (mime_types != NULL)
		g_strfreev (mime_types);

	if (load_error)
		g_error_free (load_error);
}

static void
gnome_vfs_mime_info_cache_dir_changed (GnomeVFSMonitorHandle    *handle,
		                       const gchar              *monitor_uri,
		                       const gchar              *info_uri,
		                       GnomeVFSMonitorEventType  event_type,
	                               GnomeVFSMimeInfoCacheDir *dir)
{
	G_LOCK (mime_info_cache);
	gnome_vfs_mime_info_cache_blow_global_cache ();
	gnome_vfs_mime_info_cache_dir_init (dir);
	G_UNLOCK (mime_info_cache);
}

static void
gnome_vfs_mime_info_cache_dir_defaults_changed (GnomeVFSMonitorHandle    *handle,
					 const gchar              *monitor_uri,
					 const gchar              *info_uri,
					 GnomeVFSMonitorEventType  event_type,
					 GnomeVFSMimeInfoCacheDir *dir)
{
	G_LOCK (mime_info_cache);
	gnome_vfs_mime_info_cache_blow_global_cache ();
	gnome_vfs_mime_info_cache_dir_init_defaults_list (dir);
	G_UNLOCK (mime_info_cache);
}

static GnomeVFSMimeInfoCacheDir *
gnome_vfs_mime_info_cache_dir_new (const char *path)
{
	GnomeVFSMimeInfoCacheDir *dir;
	char *filename;

	dir = g_new0 (GnomeVFSMimeInfoCacheDir, 1);
	dir->path = g_strdup (path);

	filename = g_build_filename (dir->path, "mimeinfo.cache", NULL);

	gnome_vfs_monitor_add (&dir->cache_monitor_handle,
			       filename,
			       GNOME_VFS_MONITOR_FILE,
			       (GnomeVFSMonitorCallback) 
			       gnome_vfs_mime_info_cache_dir_changed,
			       dir);
	g_free (filename);

	filename = g_build_filename (dir->path, "defaults.list", NULL);

	gnome_vfs_monitor_add (&dir->defaults_monitor_handle,
			       filename,
			       GNOME_VFS_MONITOR_FILE,
			       (GnomeVFSMonitorCallback) 
			       gnome_vfs_mime_info_cache_dir_defaults_changed,
			       dir);
	g_free (filename);

	return dir;
}

static void
gnome_vfs_mime_info_cache_dir_free (GnomeVFSMimeInfoCacheDir *dir)
{
	if (dir == NULL)
		return;
	if (dir->mime_info_cache_map != NULL) {
		destroy_info_cache_map (dir->mime_info_cache_map);
		dir->mime_info_cache_map = NULL;

	}

	if (dir->defaults_list_map != NULL) {
		g_hash_table_destroy (dir->defaults_list_map);
		dir->defaults_list_map = NULL;
	}

	if (dir->defaults_monitor_handle) {
		gnome_vfs_monitor_cancel (dir->defaults_monitor_handle);
		dir->defaults_monitor_handle = NULL;
	}

	if (dir->cache_monitor_handle){
		gnome_vfs_monitor_cancel (dir->cache_monitor_handle);
		dir->cache_monitor_handle = NULL;
	}

	g_free (dir);
}

static char **
gnome_vfs_mime_info_cache_get_search_path (void)
{
	char **args = NULL;
	char **data_dirs;
	char *user_data_dir;
	int i, length, j;

	data_dirs = egg_get_secondary_data_dirs ();

	for (length = 0; data_dirs[length] != NULL; length++);

	args = g_new (char *, length + 2);

	j = 0;
	user_data_dir = egg_get_user_data_dir ();
	args[j++] = g_build_filename (user_data_dir, "applications", NULL);
	g_free (user_data_dir);

	for (i = 0; i < length; i++) {
		args[j++] = g_build_filename (data_dirs[i],
					      "applications", NULL);
	}
	args[j++] = NULL;

	g_strfreev (data_dirs);

	return args;
}

static gboolean
gnome_vfs_mime_info_cache_dir_desktop_entry_is_valid (GnomeVFSMimeInfoCacheDir *dir,
                                                      const char *desktop_entry)
{
	EggDesktopEntries *entries;
	GError *load_error;
	int i;
	gboolean can_show_in;
	char *filename;

	load_error = NULL;
	can_show_in = TRUE;

	filename = g_build_filename ("applications", desktop_entry, NULL);
	entries =
		egg_desktop_entries_new_from_file (NULL,
						   EGG_DESKTOP_ENTRIES_DISCARD_COMMENTS |
						   EGG_DESKTOP_ENTRIES_DISCARD_TRANSLATIONS,
						   filename,
						   &load_error);
	g_free (filename);

	if (load_error != NULL) {
		g_error_free (load_error);
		return FALSE;
	}

	if (egg_desktop_entries_has_key (entries,
					 egg_desktop_entries_get_start_group (entries),
					 "OnlyShowIn")) {

		char **only_show_in_list;
		only_show_in_list = egg_desktop_entries_get_string_list (entries,
									 egg_desktop_entries_get_start_group (entries),
									 "OnlyShowIn",
									 NULL,
									 &load_error);

		if (load_error != NULL) {
			g_error_free (load_error);
			g_strfreev (only_show_in_list);
			egg_desktop_entries_free (entries);
			return FALSE;
		}

		can_show_in = FALSE;
		for (i = 0; only_show_in_list[i] != NULL; i++) {
			if (strcmp (only_show_in_list[i], "GNOME") == 0) {
				can_show_in = TRUE;
				break;
			}
		}

		g_strfreev (only_show_in_list);
	}

	if (egg_desktop_entries_has_key (entries,
					 egg_desktop_entries_get_start_group (entries),
					 "NotShowIn")) {
		char **not_show_in_list;
		not_show_in_list = egg_desktop_entries_get_string_list (entries,
									egg_desktop_entries_get_start_group (entries),
									"NotShowIn",
									NULL,
									&load_error);

		if (load_error != NULL) {
			g_error_free (load_error);
			return FALSE;
		}

		for (i = 0; not_show_in_list[i] != NULL; i++) {
			if (strcmp (not_show_in_list[i], "GNOME") == 0) {
				can_show_in = FALSE;
				break;
			}
		}

		g_strfreev (not_show_in_list);
	}

	egg_desktop_entries_free (entries);
	return can_show_in;
}

static gboolean
gnome_vfs_mime_info_desktop_entry_is_valid (const char *desktop_entry)
{
	GList *l;
	
	for (l = mime_info_cache->dirs; l != NULL; l = l->next) {
		GnomeVFSMimeInfoCacheDir *app_dir;
		
		app_dir = (GnomeVFSMimeInfoCacheDir *) l->data;
		if (gnome_vfs_mime_info_cache_dir_desktop_entry_is_valid (app_dir,
									  desktop_entry)) {
			return TRUE;
		}
	}
	return FALSE;
}


static void
gnome_vfs_mime_info_cache_dir_add_desktop_entries (GnomeVFSMimeInfoCacheDir *dir,
                                                   const char *mime_type,
                                                   char **new_desktop_file_ids)
{
	GList *desktop_file_ids;
	int i;

	desktop_file_ids = g_hash_table_lookup (dir->mime_info_cache_map,
						mime_type);

	for (i = 0; new_desktop_file_ids[i] != NULL; i++) {
		if (!g_list_find (desktop_file_ids, new_desktop_file_ids[i]))
			desktop_file_ids = g_list_append (desktop_file_ids,
							  g_strdup (new_desktop_file_ids[i]));
	}

	g_hash_table_insert (dir->mime_info_cache_map, g_strdup (mime_type), desktop_file_ids);
}

static void
gnome_vfs_mime_info_cache_init_dir_lists (void)
{
	char **dirs;
	int i;

	mime_info_cache = gnome_vfs_mime_info_cache_new ();

	dirs = gnome_vfs_mime_info_cache_get_search_path ();

	for (i = 0; dirs[i] != NULL; i++) {
		GnomeVFSMimeInfoCacheDir *dir;

		dir = gnome_vfs_mime_info_cache_dir_new (dirs[i]);

		if (dir != NULL) {
			gnome_vfs_mime_info_cache_dir_init (dir);
			gnome_vfs_mime_info_cache_dir_init_defaults_list (dir);

			mime_info_cache->dirs = g_list_append (mime_info_cache->dirs,
			 		                       dir);
		}
	}
	g_strfreev (dirs);
}

static void
gnome_vfs_mime_info_cache_update_dir_lists (void)
{
	GList *tmp;
	tmp = mime_info_cache->dirs;
	while (tmp != NULL) {
		GnomeVFSMimeInfoCacheDir *dir;

		dir = (GnomeVFSMimeInfoCacheDir *) tmp->data;

		if (dir->cache_monitor_handle == NULL) {
			gnome_vfs_mime_info_cache_blow_global_cache ();
			gnome_vfs_mime_info_cache_dir_init (dir);
		}
		
		if (dir->defaults_monitor_handle == NULL) {
			gnome_vfs_mime_info_cache_blow_global_cache ();
			gnome_vfs_mime_info_cache_dir_init_defaults_list (dir);
		}

		tmp = tmp->next;
	}
}

static gboolean
emit_mime_changed (gpointer data)
{
	_gnome_vfs_mime_monitor_emit_data_changed (gnome_vfs_mime_monitor_get ());
	return FALSE;
}


static void
gnome_vfs_mime_info_cache_init (void)
{
	G_LOCK (mime_info_cache);
	if (mime_info_cache == NULL) {
		gnome_vfs_mime_info_cache_init_dir_lists ();
	} else {
		struct timeval tv;

		gettimeofday (&tv, NULL);

		if (tv.tv_sec >= mime_info_cache->last_stat_time + 5) {
			gnome_vfs_mime_info_cache_update_dir_lists ();
			mime_info_cache->last_stat_time = tv.tv_sec;
		}
	}

	if (mime_info_cache->should_ping_mime_monitor) {
		g_idle_add (emit_mime_changed, NULL);
		mime_info_cache->should_ping_mime_monitor = FALSE;
	}
	G_UNLOCK (mime_info_cache);
}


static GnomeVFSMimeInfoCache *
gnome_vfs_mime_info_cache_new (void)
{
	GnomeVFSMimeInfoCache *cache;

	cache = g_new0 (GnomeVFSMimeInfoCache, 1);

	cache->global_defaults_cache = g_hash_table_new_full (g_str_hash, g_str_equal,
							      (GDestroyNotify) g_free,
							      (GDestroyNotify) g_free);
	return cache;
}

static void
gnome_vfs_mime_info_cache_free (GnomeVFSMimeInfoCache *cache)
{
	if (cache == NULL)
		return;

	g_list_foreach (cache->dirs,
			(GFunc) gnome_vfs_mime_info_cache_dir_free,
			NULL);
	g_list_free (cache->dirs);
	g_hash_table_destroy (cache->global_defaults_cache);
	g_free (cache);
}

void
gnome_vfs_mime_info_cache_reload (const char *dir)
{
	/* FIXME: just reload the dir that needs reloading,
	 * don't blow the whole cache
	 */
	if (mime_info_cache != NULL) {
		G_LOCK (mime_info_cache);
		gnome_vfs_mime_info_cache_free (mime_info_cache);
		mime_info_cache = NULL;
		G_UNLOCK (mime_info_cache);
	}
}




static void
get_all_parent_types_helper (GList **mime_types, const char *mime_type)
{
	const gchar *parent_list;
	
	if (!g_list_find_custom (*mime_types, mime_type,
				 (GCompareFunc) strcmp)) {
		*mime_types = g_list_prepend (*mime_types, g_strdup (mime_type));
	}

	parent_list = gnome_vfs_mime_get_value (mime_type, "parent_classes");
	if (parent_list) {
		char **parents;
		int i;
		
		parents = g_strsplit (parent_list, ":", -1);
		for (i = 0; parents && parents[i] != NULL; i++) {
			get_all_parent_types_helper (mime_types, parents[i]);
		}
		g_strfreev (parents);
	}

	if (g_str_has_prefix (mime_type, "text/")) {
		if (!g_list_find_custom (*mime_types, "text/plain",
					 (GCompareFunc) strcmp)) {
			*mime_types = g_list_prepend (*mime_types, g_strdup ("text/plain"));
		}
	}
	if (!g_str_has_prefix (mime_type, "inode/")) {
		if (!g_list_find_custom (*mime_types, "application/octet-stream",
					 (GCompareFunc) strcmp)) {
			*mime_types = g_list_prepend (*mime_types, g_strdup ("application/octet-stream"));
		}
	}
}

static GList *
get_all_parent_types (const char *mime_type)
{
	GList *mime_types;

	/* TODO: Unalias mime_type first */
	
	mime_types = NULL;
	get_all_parent_types_helper (&mime_types, mime_type);
	return g_list_reverse (mime_types);
}

GList *
gnome_vfs_mime_get_all_desktop_entries (const char *mime_type)
{
	GList *desktop_entries, *list, *dir_list, *tmp;
	GList *mime_types, *m_list;
	GnomeVFSMimeInfoCacheDir *dir;
	char *type;

	gnome_vfs_mime_info_cache_init ();

	G_LOCK (mime_info_cache);
	mime_types = get_all_parent_types (mime_type);

	desktop_entries = NULL;
	for (m_list = mime_types; m_list != NULL; m_list = m_list->next) {
		type = m_list->data;
		
		for (dir_list = mime_info_cache->dirs;
		     dir_list != NULL;
		     dir_list = dir_list->next) {
			dir = (GnomeVFSMimeInfoCacheDir *) dir_list->data;
			
			list = g_hash_table_lookup (dir->mime_info_cache_map, type);
			
			for (tmp = list; tmp != NULL; tmp = tmp->next) {
				/* Add if not already in list, and valid */
				/* TODO: Should we cache here to avoid desktop file validation? */
				if (!g_list_find_custom (desktop_entries, tmp->data,
							 (GCompareFunc) strcmp) &&
				    gnome_vfs_mime_info_desktop_entry_is_valid (tmp->data)) {
					desktop_entries = g_list_prepend (desktop_entries,
									  g_strdup (tmp->data));
				}
			}
		}
	}

	G_UNLOCK (mime_info_cache);

	g_list_foreach (mime_types, (GFunc)g_free, NULL);
	g_list_free (mime_types);

	desktop_entries = g_list_reverse (desktop_entries);

	return desktop_entries;
}

gchar *
gnome_vfs_mime_get_default_desktop_entry (const char *mime_type)
{
	gchar *desktop_entry;
	char **desktop_entries;
	GList *dir_list;
	GnomeVFSMimeInfoCacheDir *dir;
	int i;

	gnome_vfs_mime_info_cache_init ();

	G_LOCK (mime_info_cache);

	desktop_entry = g_hash_table_lookup (mime_info_cache->global_defaults_cache,
					     mime_type);
	if (desktop_entry) {
		G_UNLOCK (mime_info_cache);
		return g_strdup (desktop_entry);
	}

	desktop_entry = NULL;
	for (dir_list = mime_info_cache->dirs; dir_list != NULL; dir_list = dir_list->next) {
		dir = dir_list->data;
		desktop_entries = g_hash_table_lookup (dir->defaults_list_map,
						       mime_type);
		for (i = 0; desktop_entries != NULL && desktop_entries[i] != NULL; i++) {
			desktop_entry = desktop_entries[i];
			if (desktop_entry != NULL &&
			    gnome_vfs_mime_info_desktop_entry_is_valid (desktop_entry)) {
				g_hash_table_insert (mime_info_cache->global_defaults_cache,
						     g_strdup (mime_type), g_strdup (desktop_entry));
				G_UNLOCK (mime_info_cache);
				return g_strdup (desktop_entry);
			}
		}
	}
	G_UNLOCK (mime_info_cache);

	return NULL;
}

