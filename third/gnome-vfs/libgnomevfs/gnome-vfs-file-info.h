/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-file-info.h - Handling of file information for the GNOME
   Virtual File System.

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

#ifndef GNOME_VFS_FILE_INFO_H
#define GNOME_VFS_FILE_INFO_H

#include "gnome-vfs.h"

#define GNOME_VFS_FILE_INFO_SYMLINK(info)		\
	((info)->flags & GNOME_VFS_FILE_FLAGS_SYMLINK)

#define GNOME_VFS_FILE_INFO_SET_SYMLINK(info, value)			\
	(value ? ((info)->flags |= GNOME_VFS_FILE_FLAGS_SYMLINK)	\
	       : ((info)->flags &= ~GNOME_VFS_FILE_FLAGS_SYMLINK))

#define GNOME_VFS_FILE_INFO_LOCAL(info)			\
	((info)->flags & GNOME_VFS_FILE_FLAGS_LOCAL)

#define GNOME_VFS_FILE_INFO_SET_LOCAL(info, value)			\
	(value ? ((info)->flags |= GNOME_VFS_FILE_FLAGS_LOCAL)		\
	       : ((info)->flags &= ~GNOME_VFS_FILE_FLAGS_LOCAL))



#define GNOME_VFS_FILE_INFO_SUID(info)			\
	((info)->permissions & GNOME_VFS_PERM_SUID)

#define GNOME_VFS_FILE_INFO_SGID(info)			\
	((info)->permissions & GNOME_VFS_PERM_SGID)

#define GNOME_VFS_FILE_INFO_STICKY(info)		\
	((info)->permissions & GNOME_VFS_PERM_STICKY)


#define GNOME_VFS_FILE_INFO_SET_SUID(info, value)		\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_SUID)	\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_SUID))

#define GNOME_VFS_FILE_INFO_SET_SGID(info, value)		\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_SGID)	\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_SGID))

#define GNOME_VFS_FILE_INFO_SET_STICKY(info, value)			\
	(value ? ((info)->permissions |= GNOME_VFS_PERM_STICKY)		\
	       : ((info)->permissions &= ~GNOME_VFS_PERM_STICKY))



GnomeVFSFileInfo *
		 gnome_vfs_file_info_new 	(void);
void		 gnome_vfs_file_info_init	(GnomeVFSFileInfo *info);
void		 gnome_vfs_file_info_clear	(GnomeVFSFileInfo *info);
void 		 gnome_vfs_file_info_unref   	(GnomeVFSFileInfo *info);
void 		 gnome_vfs_file_info_ref     	(GnomeVFSFileInfo *info);
const gchar	*gnome_vfs_file_info_get_mime_type
						(GnomeVFSFileInfo *info);

void		 gnome_vfs_file_info_copy 	(GnomeVFSFileInfo *dest,
						 const GnomeVFSFileInfo *src);

GnomeVFSFileInfo *
		 gnome_vfs_file_info_dup 	(const GnomeVFSFileInfo *orig);


gboolean	 gnome_vfs_file_info_matches	(const GnomeVFSFileInfo *a,
						 const GnomeVFSFileInfo *b);

gint		 gnome_vfs_file_info_compare_for_sort
						(const GnomeVFSFileInfo *a,
						 const GnomeVFSFileInfo *b,
						 const GnomeVFSDirectoryFilterType *sort_rules);

gint		 gnome_vfs_file_info_compare_for_sort_reversed
						(const GnomeVFSFileInfo *a,
						 const GnomeVFSFileInfo *b,
						 const GnomeVFSDirectoryFilterType *sort_rules);

GList           *gnome_vfs_file_info_list_ref   (GList *list);
GList           *gnome_vfs_file_info_list_unref (GList *list);
GList           *gnome_vfs_file_info_list_copy  (GList *list);
void             gnome_vfs_file_info_list_free  (GList *list);

#endif /* GNOME_VFS_FILE_INFO_H */
