/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-directory-list.h - Support for directory lists in the
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

#ifndef _GNOME_VFS_DIRECTORY_LIST_H
#define _GNOME_VFS_DIRECTORY_LIST_H

GnomeVFSDirectoryList *
	 gnome_vfs_directory_list_new		(void);

void 	 gnome_vfs_directory_list_destroy	(GnomeVFSDirectoryList *list);
void	 gnome_vfs_directory_list_prepend	(GnomeVFSDirectoryList *list,
						 GnomeVFSFileInfo *info);
void	 gnome_vfs_directory_list_append 	(GnomeVFSDirectoryList *list,
						 GnomeVFSFileInfo *info);

void	 gnome_vfs_directory_list_filter	(GnomeVFSDirectoryList *list,
						 GnomeVFSDirectoryFilter
						 	*filter);

void	 gnome_vfs_directory_list_sort		(GnomeVFSDirectoryList *list,
						 gboolean reversed,
						 const GnomeVFSDirectorySortRule
						 	*rules);
void     gnome_vfs_directory_list_sort_custom	(GnomeVFSDirectoryList *list,
						 GnomeVFSDirectorySortFunc
						 	compare_func,
						 gpointer data);

GnomeVFSFileInfo *
	gnome_vfs_directory_list_first		(GnomeVFSDirectoryList *list);
GnomeVFSFileInfo *
	gnome_vfs_directory_list_next		(GnomeVFSDirectoryList *list);
GnomeVFSFileInfo *
	gnome_vfs_directory_list_prev		(GnomeVFSDirectoryList *list);
GnomeVFSFileInfo *
	gnome_vfs_directory_list_last		(GnomeVFSDirectoryList *list);
GnomeVFSFileInfo *
	gnome_vfs_directory_list_current	(GnomeVFSDirectoryList *list);
GnomeVFSFileInfo *
	gnome_vfs_directory_list_nth		(GnomeVFSDirectoryList *list,
						 guint n);

GnomeVFSFileInfo *
	gnome_vfs_directory_list_get		(GnomeVFSDirectoryList *list,
						 GnomeVFSDirectoryListPosition
						         position);

guint	gnome_vfs_directory_list_get_num_entries
						(GnomeVFSDirectoryList *list);

GnomeVFSDirectoryListPosition
	gnome_vfs_directory_list_get_position	(GnomeVFSDirectoryList *list);
void	gnome_vfs_directory_list_set_position   (GnomeVFSDirectoryList *list,
						 GnomeVFSDirectoryListPosition
						 	position);
GnomeVFSDirectoryListPosition
	gnome_vfs_directory_list_get_last_position
						(GnomeVFSDirectoryList *list);
GnomeVFSDirectoryListPosition
	gnome_vfs_directory_list_get_first_position
						(GnomeVFSDirectoryList *list);

GnomeVFSDirectoryListPosition
	gnome_vfs_directory_list_position_next  (GnomeVFSDirectoryListPosition
						 	position);
GnomeVFSDirectoryListPosition
	gnome_vfs_directory_list_position_prev  (GnomeVFSDirectoryListPosition
						 	position);

GnomeVFSResult	gnome_vfs_directory_list_load
					(GnomeVFSDirectoryList **list,
					 const gchar *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter);
GnomeVFSResult	gnome_vfs_directory_list_load_from_uri
					(GnomeVFSDirectoryList **list,
					 GnomeVFSURI *uri,
					 GnomeVFSFileInfoOptions options,
					 const GnomeVFSDirectoryFilter *filter);

#endif /* _GNOME_VFS_DIRECTORY_LIST_H */
