/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-uri.h - URI handling for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it>
*/

#ifndef GNOME_VFS_URI_H
#define GNOME_VFS_URI_H

GnomeVFSURI 	     *gnome_vfs_uri_new                   (const gchar *text_uri);
GnomeVFSURI 	     *gnome_vfs_uri_ref                   (GnomeVFSURI *uri);
void        	      gnome_vfs_uri_unref                 (GnomeVFSURI *uri);

GnomeVFSURI          *gnome_vfs_uri_append_string         (const GnomeVFSURI *uri,
						           const char *path);
GnomeVFSURI          *gnome_vfs_uri_append_path           (const GnomeVFSURI *uri,
						           const char *path);
GnomeVFSURI          *gnome_vfs_uri_append_file_name      (const GnomeVFSURI *uri,
						           const gchar *filename);
gchar       	     *gnome_vfs_uri_to_string             (const GnomeVFSURI *uri,
						           GnomeVFSURIHideOptions hide_options);
GnomeVFSURI 	     *gnome_vfs_uri_dup                   (const GnomeVFSURI *uri);
gboolean    	      gnome_vfs_uri_is_local              (const GnomeVFSURI *uri);
gboolean	      gnome_vfs_uri_has_parent	          (const GnomeVFSURI *uri);
GnomeVFSURI	     *gnome_vfs_uri_get_parent            (const GnomeVFSURI *uri);

GnomeVFSToplevelURI *gnome_vfs_uri_get_toplevel           (const GnomeVFSURI *uri);

const gchar 	    *gnome_vfs_uri_get_host_name          (const GnomeVFSURI *uri);
const gchar         *gnome_vfs_uri_get_scheme             (const GnomeVFSURI *uri);
guint 	    	     gnome_vfs_uri_get_host_port          (const GnomeVFSURI *uri);
const gchar 	    *gnome_vfs_uri_get_user_name          (const GnomeVFSURI *uri);
const gchar	    *gnome_vfs_uri_get_password           (const GnomeVFSURI *uri);

void		     gnome_vfs_uri_set_host_name          (GnomeVFSURI *uri,
						           const gchar *host_name);
void 	    	     gnome_vfs_uri_set_host_port          (GnomeVFSURI *uri,
						           guint host_port);
void		     gnome_vfs_uri_set_user_name          (GnomeVFSURI *uri,
						           const gchar *user_name);
void		     gnome_vfs_uri_set_password           (GnomeVFSURI *uri,
						           const gchar *password);

gboolean	     gnome_vfs_uri_equal	          (const GnomeVFSURI *a,
						           const GnomeVFSURI *b);

gboolean	     gnome_vfs_uri_is_parent	          (const GnomeVFSURI *parent,
						           const GnomeVFSURI *item,
						           gboolean recursive);
				  
const gchar 	    *gnome_vfs_uri_get_path                (const GnomeVFSURI *uri);
const gchar 	    *gnome_vfs_uri_get_basename            (const GnomeVFSURI *uri);
const gchar 	    *gnome_vfs_uri_get_fragment_identifier (const GnomeVFSURI *uri);
gchar 		    *gnome_vfs_uri_extract_dirname         (const GnomeVFSURI *uri);
gchar		    *gnome_vfs_uri_extract_short_name      (const GnomeVFSURI *uri);
gchar		    *gnome_vfs_uri_extract_short_path_name (const GnomeVFSURI *uri);

gint		     gnome_vfs_uri_hequal 	           (gconstpointer a,
						            gconstpointer b);
guint		     gnome_vfs_uri_hash		           (gconstpointer p);

GList               *gnome_vfs_uri_list_ref                (GList *list);
GList               *gnome_vfs_uri_list_unref              (GList *list);
GList               *gnome_vfs_uri_list_copy               (GList *list);
void                 gnome_vfs_uri_list_free               (GList *list);

#endif /* GNOME_VFS_URI_H */
