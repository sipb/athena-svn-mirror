/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-directory-list.c - Support for directory lists in the
   GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

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

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <fnmatch.h>
#include <regex.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

struct GnomeVFSDirectoryList {
	GList *entries;		/* GnomeVFSFileInfo */
	GList *current_entry;
	GList *last_entry;

	guint num_entries;
};

static void
remove_entry (GnomeVFSDirectoryList *list,
	      GList *p)
{
	GnomeVFSFileInfo *info;

	info = p->data;
	gnome_vfs_file_info_unref (info);

	if (list->current_entry == p)
		list->current_entry = NULL;
	if (list->last_entry == p)
		list->last_entry = p->prev;
	list->entries = g_list_remove_link (list->entries, p);

	list->num_entries--;

	g_list_free (p);
}

/**
 * gnome_vfs_directory_list_new:
 * 
 * Create a new directory list object.
 * 
 * Return value: A pointer to the newly created object.
 **/
GnomeVFSDirectoryList *
gnome_vfs_directory_list_new (void)
{
	GnomeVFSDirectoryList *new_list;

	new_list = g_new0 (GnomeVFSDirectoryList, 1);
	
	return new_list;
}

/**
 * gnome_vfs_directory_list_destroy:
 * @list: A directory list
 * 
 * Destroy @list
 **/
void
gnome_vfs_directory_list_destroy (GnomeVFSDirectoryList *list)
{
	g_return_if_fail (list != NULL);

	gnome_vfs_file_info_list_free (list->entries);
	g_free (list);
}

/**
 * gnome_vfs_directory_list_prepend:
 * @list: A directory list
 * @info: Information to be added to the list
 * 
 * Add @info at the beginning of @list.
 **/
void
gnome_vfs_directory_list_prepend (GnomeVFSDirectoryList *list,
				  GnomeVFSFileInfo *info)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (info != NULL);

	list->entries = g_list_prepend (list->entries, info);
	if (list->last_entry == NULL)
		list->last_entry = list->entries;

	list->num_entries++;
}

/**
 * gnome_vfs_directory_list_append:
 * @list: A directory list
 * @info: Information to be added to the list
 * 
 * Add @info at the end of @list.
 * 
 **/
void
gnome_vfs_directory_list_append (GnomeVFSDirectoryList *list,
				 GnomeVFSFileInfo *info)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (info != NULL);

	if (list->entries == NULL) {
		list->entries = g_list_alloc ();
		list->entries->data = info;
		list->last_entry = list->entries;
	} else {
		g_list_append (list->last_entry, info);
		list->last_entry = list->last_entry->next;
	}

	list->num_entries++;
}

/**
 * gnome_vfs_directory_list_first:
 * @list: A directory list
 * 
 * Retrieve the first item in @list, and set it as the current entry.
 * 
 * Return value: A pointer to the information retrieved.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_first (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	list->current_entry = list->entries;
	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_last:
 * @list: A directory list
 * 
 * Retrieve the last item in @list, and set it as the current entry.
 * 
 * Return value: A pointer to the information retrieved.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_last (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	list->current_entry = list->last_entry;
	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_next:
 * @list: A directory list
 * 
 * Retrieve the next item in @list, and set it as the current entry.
 * 
 * Return value: A pointer to the information retrieved, or NULL if the current
 * entry is the last one.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_next (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	if (list->current_entry == NULL)
		return NULL;

	list->current_entry = list->current_entry->next;
	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_prev:
 * @list: A directory list
 * 
 * Retrieve the previous item in @list, and set it as the current entry.
 * 
 * Return value: A pointer to the information retrieved, or %NULL if the
 * current entry is the last one.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_prev (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	if (list->current_entry == NULL)
		return NULL;

	list->current_entry = list->current_entry->prev;
	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_current:
 * @list: A directory list
 * 
 * Retrieve the current entry in @list.
 * 
 * Return value: A pointer to the current entry, or %NULL if no current entry
 * is set.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_current (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_nth:
 * @list: A directory list
 * @n: Ordinal number of the element to retrieve
 * 
 * Retrieve the @n'th element in @list.
 * 
 * Return value: A pointer to the @n'th element in @list, or %NULL if no such
 * element exists.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_nth (GnomeVFSDirectoryList *list, guint n)
{
	g_return_val_if_fail (list != NULL, NULL);

	list->current_entry = g_list_nth (list->entries, n);
	if (list->current_entry == NULL)
		return NULL;

	return list->current_entry->data;
}

/**
 * gnome_vfs_directory_list_filter:
 * @list: A directory list
 * @filter: A directory filter
 * 
 * Filter @list through @filter
 **/
void
gnome_vfs_directory_list_filter	(GnomeVFSDirectoryList *list,
				 GnomeVFSDirectoryFilter *filter)
{
	GList *p;

	g_return_if_fail (list != NULL);

	if (filter == NULL)
		return;

	p = list->entries;
	while (p != NULL) {
		GnomeVFSFileInfo *info;
		GList *pnext;

		info = p->data;
		pnext = p->next;

		if (! gnome_vfs_directory_filter_apply (filter, info))
			remove_entry (list, p);

		p = pnext;
	}
}

/**
 * gnome_vfs_directory_list_sort:
 * @list: A directory list
 * @reversed: Boolean specifying whether the sort order should be reversed
 * @rules: %NULL-terminated array of sorting rules
 * 
 * Sort @list according to @rules.
 **/
void
gnome_vfs_directory_list_sort (GnomeVFSDirectoryList *list,
			       gboolean reversed,
			       const GnomeVFSDirectorySortRule *rules)
{
	GnomeVFSListCompareFunc func;

	g_return_if_fail (list != NULL);
	g_return_if_fail (rules[0] != GNOME_VFS_DIRECTORY_SORT_NONE);

	if (reversed)
		func = (GnomeVFSListCompareFunc) gnome_vfs_file_info_compare_for_sort_reversed;
	else
		func = (GnomeVFSListCompareFunc) gnome_vfs_file_info_compare_for_sort;

	list->entries = gnome_vfs_list_sort (list->entries,
					     func, (gpointer) rules);

	gnome_vfs_directory_list_set_position
		(list, GNOME_VFS_DIRECTORY_LIST_POSITION_NONE);
}

/**
 * gnome_vfs_directory_list_sort_custom:
 * @list: A directory list
 * @func: A directory sorting function
 * @data: Data to be passed to @func at each iteration
 * 
 * Sort @list using @func.  @func should return -1 if the element in the first
 * argument should come before the element in the second; +1 if the element in
 * the first argument should come after the element in the second; 0 if it
 * elements are equal from the point of view of sorting.
 **/
void
gnome_vfs_directory_list_sort_custom (GnomeVFSDirectoryList *list,
				      GnomeVFSDirectorySortFunc func,
				      gpointer data)
{
	g_return_if_fail (list != NULL);
	g_return_if_fail (func != NULL);

	gnome_vfs_list_sort (list->entries, (GnomeVFSListCompareFunc) func,
			     data);
}

/**
 * gnome_vfs_directory_list_get:
 * @list: A directory list
 * @position: A directory list position
 * 
 * Retrieve element at @position.
 * 
 * Return value: A pointer to the element at @position.
 **/
GnomeVFSFileInfo *
gnome_vfs_directory_list_get (GnomeVFSDirectoryList *list,
			      GnomeVFSDirectoryListPosition position)
{
	GList *p;
	GnomeVFSFileInfo *info;

	g_return_val_if_fail (list != NULL, NULL);
	g_return_val_if_fail (position != GNOME_VFS_DIRECTORY_LIST_POSITION_NONE,
			      NULL);

	p = (GList *) position;
	info = p->data;

	return info;
}

/**
 * gnome_vfs_directory_list_get_num_entries:
 * @list: A directory list
 * 
 * Retrieve the number of elements in @list.  This is an O(0) operation.
 * 
 * Return value: The number of elements in @list.
 **/
guint
gnome_vfs_directory_list_get_num_entries (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, 0);

	return list->num_entries;
}

/**
 * gnome_vfs_directory_list_get_position:
 * @list: A directory list
 * 
 * Retrieve the current position in @list.
 * 
 * Return value: An opaque value representing the current position in @list.
 **/
GnomeVFSDirectoryListPosition
gnome_vfs_directory_list_get_position (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	return list->current_entry;
}

/**
 * gnome_vfs_directory_list_set_position:
 * @list: A directory list
 * @position: A position in @list
 * 
 * Set @list's current position.
 **/
void
gnome_vfs_directory_list_set_position (GnomeVFSDirectoryList *list,
				       GnomeVFSDirectoryListPosition position)
{
	g_return_if_fail (list != NULL);

	list->current_entry = position;
}

/**
 * gnome_vfs_directory_list_get_last_position:
 * @list: A directory list
 * 
 * Get the position of the last element in @list.
 * 
 * Return value: An opaque type representing the position of the last element
 * in @list.
 **/
GnomeVFSDirectoryListPosition
gnome_vfs_directory_list_get_last_position (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	return list->last_entry;
}

/**
 * gnome_vfs_directory_list_get_first_position:
 * @list: A directory list
 * 
 * Get the position of the first element in @list.
 * 
 * Return value: An opaque type representing the position of the first element
 * in @list.
 **/
GnomeVFSDirectoryListPosition
gnome_vfs_directory_list_get_first_position (GnomeVFSDirectoryList *list)
{
	g_return_val_if_fail (list != NULL, NULL);

	return list->entries;
}

/**
 * gnome_vfs_directory_list_position_next:
 * @position: A directory list position
 * 
 * Get the position next to @position.
 * 
 * Return value: An opaque type representing the position of the element that
 * comes after @position.
 **/
GnomeVFSDirectoryListPosition
gnome_vfs_directory_list_position_next (GnomeVFSDirectoryListPosition position)
{
	GList *list;

	g_return_val_if_fail (position != GNOME_VFS_DIRECTORY_LIST_POSITION_NONE, GNOME_VFS_DIRECTORY_LIST_POSITION_NONE);

	list = position;
	return list->next;
}

/**
 * gnome_vfs_directory_list_position_prev:
 * @position: A directory list position
 * 
 * Get the position previous to @position.
 * 
 * Return value: An opaque type representing the position of the element that
 * comes before @position.
 **/
GnomeVFSDirectoryListPosition
gnome_vfs_directory_list_position_prev (GnomeVFSDirectoryListPosition position)
{
	GList *list;

	g_return_val_if_fail (position != NULL, NULL);

	list = position;
	return list->prev;
}

static GnomeVFSResult
load_from_handle (GnomeVFSDirectoryList **list,
		  GnomeVFSDirectoryHandle *handle)
{
	GnomeVFSResult result;
	GnomeVFSFileInfo *info;

	*list = gnome_vfs_directory_list_new ();

	while (1) {
		info = gnome_vfs_file_info_new ();
		result = gnome_vfs_directory_read_next (handle, info);
		if (result != GNOME_VFS_OK)
			break;
		gnome_vfs_directory_list_append (*list, info);
	}

	gnome_vfs_file_info_unref (info);

	if (result != GNOME_VFS_ERROR_EOF) {
		gnome_vfs_directory_list_destroy (*list);
		return result;
	}

	return GNOME_VFS_OK;
}

/**
 * gnome_vfs_directory_list_load:
 * @list: A pointer to a pointer to a directory list
 * @text_uri: A text URI
 * @options: Options for loading the directory 
 * @filter: Filter to be applied to the files being read
 * 
 * Load a directory from @text_uri with the specified @options
 * into a newly created directory list.  Directory entries are filtered through
 * @filter.  On return @*list will point to such a list.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_directory_list_load (GnomeVFSDirectoryList **list,
			       const gchar *text_uri,
			       GnomeVFSFileInfoOptions options,
			       const GnomeVFSDirectoryFilter *filter)
{
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;

	result = gnome_vfs_directory_open (&handle, text_uri, options,
					   filter);
	if (result != GNOME_VFS_OK)
		return result;

	result = load_from_handle (list, handle);

	gnome_vfs_directory_close (handle);
	return result;
}

/**
 * gnome_vfs_directory_list_load_from_uri:
 * @list: A pointer to a pointer to a directory list
 * @uri: A GnomeVFSURI
 * @options: Options for loading the directory 
 * @filter: Filter to be applied to the files being read
 * 
 * Load a directory from @uri with the specified @options
 * into a newly created directory list.  Directory entries are filtered through
 * @filter.  On return @*list will point to such a list.
 * 
 * Return value: An integer representing the result of the operation.
 **/
GnomeVFSResult
gnome_vfs_directory_list_load_from_uri (GnomeVFSDirectoryList **list,
					GnomeVFSURI *uri,
					GnomeVFSFileInfoOptions options,
					const GnomeVFSDirectoryFilter *filter)
{
	GnomeVFSDirectoryHandle *handle;
	GnomeVFSResult result;

	result = gnome_vfs_directory_open_from_uri (&handle, uri, options,
						    filter);
	if (result != GNOME_VFS_OK)
		return result;

	result = load_from_handle (list, handle);

	gnome_vfs_directory_close (handle);
	return result;
}

