/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* nautilus-bookmark.c - implementation of individual bookmarks.

   Copyright (C) 1999, 2000 Eazel, Inc.

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

   Authors: John Sullivan <sullivan@eazel.com>
*/

#include <config.h>
#include "nautilus-bookmark.h"

#include "nautilus-icon-factory.h"
#include <eel/eel-gdk-pixbuf-extensions.h>
#include <eel/eel-gtk-extensions.h>
#include <eel/eel-gtk-macros.h>
#include <eel/eel-string.h>
#include <eel/eel-vfs-extensions.h>
#include <gtk/gtkaccellabel.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkimagemenuitem.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-macros.h>
#include <libgnome/gnome-util.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>

enum {
	APPEARANCE_CHANGED,
	CONTENTS_CHANGED,
	LAST_SIGNAL
};

#define GENERIC_BOOKMARK_ICON_NAME	"i-bookmark"
#define MISSING_BOOKMARK_ICON_NAME	"i-bookmark-missing"

static guint signals[LAST_SIGNAL];

struct NautilusBookmarkDetails
{
	char *name;
	char *uri;
	NautilusScalableIcon *icon;
	NautilusFile *file;
};

static void	  nautilus_bookmark_connect_file	  (NautilusBookmark	 *file);
static void	  nautilus_bookmark_disconnect_file	  (NautilusBookmark	 *file);

GNOME_CLASS_BOILERPLATE (NautilusBookmark, nautilus_bookmark,
			 GtkObject, GTK_TYPE_OBJECT)

/* GtkObject methods.  */

static void
nautilus_bookmark_finalize (GObject *object)
{
	NautilusBookmark *bookmark;

	g_assert (NAUTILUS_IS_BOOKMARK (object));

	bookmark = NAUTILUS_BOOKMARK (object);

	nautilus_bookmark_disconnect_file (bookmark);	

	g_free (bookmark->details->name);
	g_free (bookmark->details->uri);
	g_free (bookmark->details);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* Initialization.  */

static void
nautilus_bookmark_class_init (NautilusBookmarkClass *class)
{
	G_OBJECT_CLASS (class)->finalize = nautilus_bookmark_finalize;

	signals[APPEARANCE_CHANGED] =
		g_signal_new ("appearance_changed",
		              G_TYPE_FROM_CLASS (class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (NautilusBookmarkClass, appearance_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);

	signals[CONTENTS_CHANGED] =
		g_signal_new ("contents_changed",
		              G_TYPE_FROM_CLASS (class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (NautilusBookmarkClass, contents_changed),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
				
}

static void
nautilus_bookmark_instance_init (NautilusBookmark *bookmark)
{
	bookmark->details = g_new0 (NautilusBookmarkDetails, 1);
}

/**
 * nautilus_bookmark_compare_with:
 *
 * Check whether two bookmarks are considered identical.
 * @a: first NautilusBookmark*.
 * @b: second NautilusBookmark*.
 * 
 * Return value: 0 if @a and @b have same name and uri, 1 otherwise 
 * (GCompareFunc style)
 **/
int		    
nautilus_bookmark_compare_with (gconstpointer a, gconstpointer b)
{
	NautilusBookmark *bookmark_a;
	NautilusBookmark *bookmark_b;

	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (a), 1);
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (b), 1);

	bookmark_a = NAUTILUS_BOOKMARK (a);
	bookmark_b = NAUTILUS_BOOKMARK (b);

	if (strcmp (bookmark_a->details->name,
		    bookmark_b->details->name) != 0) {
		return 1;
	}

	if (!eel_uris_match (bookmark_a->details->uri,
			     bookmark_b->details->uri)) {
		return 1;
	}
	
	return 0;
}

/**
 * nautilus_bookmark_compare_uris:
 *
 * Check whether the uris of two bookmarks are for the same location.
 * @a: first NautilusBookmark*.
 * @b: second NautilusBookmark*.
 * 
 * Return value: 0 if @a and @b have matching uri, 1 otherwise 
 * (GCompareFunc style)
 **/
int		    
nautilus_bookmark_compare_uris (gconstpointer a, gconstpointer b)
{
	NautilusBookmark *bookmark_a;
	NautilusBookmark *bookmark_b;

	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (a), 1);
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (b), 1);

	bookmark_a = NAUTILUS_BOOKMARK (a);
	bookmark_b = NAUTILUS_BOOKMARK (b);

	return !eel_uris_match (bookmark_a->details->uri,
				bookmark_b->details->uri);
}

NautilusBookmark *
nautilus_bookmark_copy (NautilusBookmark *bookmark)
{
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (bookmark), NULL);

	return nautilus_bookmark_new_with_icon (
			bookmark->details->uri,
			bookmark->details->name,
			bookmark->details->icon);
}

char *
nautilus_bookmark_get_name (NautilusBookmark *bookmark)
{
	g_return_val_if_fail(NAUTILUS_IS_BOOKMARK (bookmark), NULL);

	return g_strdup (bookmark->details->name);
}

GdkPixbuf *	    
nautilus_bookmark_get_pixbuf (NautilusBookmark *bookmark,
			      guint icon_size,
			      gboolean optimize_for_anti_aliasing)
{
	GdkPixbuf *result;
	NautilusScalableIcon *icon;
	
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (bookmark), NULL);

	icon = nautilus_bookmark_get_icon (bookmark);
	if (icon == NULL) {
		return NULL;
	}
	
	result = nautilus_icon_factory_get_pixbuf_for_icon
		(icon,
		 icon_size, icon_size, icon_size, icon_size,
		 NULL, TRUE);
	nautilus_scalable_icon_unref (icon);
	
	return result;
}

NautilusScalableIcon *
nautilus_bookmark_get_icon (NautilusBookmark *bookmark)
{
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (bookmark), NULL);

	/* Try to connect a file in case file exists now but didn't earlier. */
	nautilus_bookmark_connect_file (bookmark);

	if (bookmark->details->icon != NULL) {
		nautilus_scalable_icon_ref (bookmark->details->icon);
	}
	return bookmark->details->icon;
}

char *
nautilus_bookmark_get_uri (NautilusBookmark *bookmark)
{
	g_return_val_if_fail(NAUTILUS_IS_BOOKMARK (bookmark), NULL);

	/* Try to connect a file in case file exists now but didn't earlier.
	 * This allows a bookmark to update its image properly in the case
	 * where a new file appears with the same URI as a previously-deleted
	 * file. Calling connect_file here means that attempts to activate the 
	 * bookmark will update its image if possible. 
	 */
	nautilus_bookmark_connect_file (bookmark);

	return g_strdup (bookmark->details->uri);
}


/**
 * nautilus_bookmark_set_name:
 *
 * Change the user-displayed name of a bookmark.
 * @new_name: The new user-displayed name for this bookmark, mustn't be NULL.
 * 
 * Returns: TRUE if the name changed else FALSE.
 **/
gboolean
nautilus_bookmark_set_name (NautilusBookmark *bookmark, const char *new_name)
{
	g_return_val_if_fail (new_name != NULL, FALSE);
	g_return_val_if_fail (NAUTILUS_IS_BOOKMARK (bookmark), FALSE);

	if (strcmp (new_name, bookmark->details->name) == 0) {
		return FALSE;
	}

	g_free (bookmark->details->name);
	bookmark->details->name = g_strdup (new_name);

	g_signal_emit (bookmark, signals[APPEARANCE_CHANGED], 0);

	return TRUE;
}

static gboolean
nautilus_bookmark_icon_is_different (NautilusBookmark *bookmark,
		   		     NautilusScalableIcon *new_icon)
{
	char *new_uri, *new_mime_type, *new_name;
	char *old_uri, *old_mime_type, *old_name;
	gboolean result;

	g_assert (NAUTILUS_IS_BOOKMARK (bookmark));
	g_assert (new_icon != NULL);

	/* Bookmarks don't store the modifier or embedded text. */
	nautilus_scalable_icon_get_text_pieces 
		(new_icon, &new_uri, &new_mime_type, &new_name, NULL, NULL);

	if (bookmark->details->icon == NULL) {
		result = !eel_str_is_empty (new_uri)
			|| !eel_str_is_empty (new_mime_type)
			|| !eel_str_is_empty (new_name);
	} else {
		nautilus_scalable_icon_get_text_pieces 
			(bookmark->details->icon, &old_uri, &old_mime_type, &old_name, NULL, NULL);

		result = eel_strcmp (old_uri, new_uri) != 0
			|| eel_strcmp (old_mime_type, new_mime_type) != 0
			|| eel_strcmp (old_name, new_name) != 0;

		g_free (old_uri);
		g_free (old_mime_type);
		g_free (old_name);
	}

	g_free (new_uri);
	g_free (new_mime_type);
	g_free (new_name);

	return result;
}

/**
 * Update icon if there's a better one available.
 * Return TRUE if the icon changed.
 */
static gboolean
nautilus_bookmark_update_icon (NautilusBookmark *bookmark)
{
	NautilusScalableIcon *new_icon;

	g_assert (NAUTILUS_IS_BOOKMARK (bookmark));

	if (bookmark->details->file == NULL) {
		return FALSE;
	}

	if (nautilus_icon_factory_is_icon_ready_for_file (bookmark->details->file)) {
		new_icon = nautilus_icon_factory_get_icon_for_file (bookmark->details->file, NULL);
		if (nautilus_bookmark_icon_is_different (bookmark, new_icon)) {
			if (bookmark->details->icon != NULL) {
				nautilus_scalable_icon_unref (bookmark->details->icon);
			}
			bookmark->details->icon = new_icon;
			return TRUE;
		}
		nautilus_scalable_icon_unref (new_icon);
	}

	return FALSE;
}

static void
bookmark_file_changed_callback (NautilusFile *file, NautilusBookmark *bookmark)
{
	char *file_uri;
	gboolean should_emit_appearance_changed_signal;
	gboolean should_emit_contents_changed_signal;

	g_assert (NAUTILUS_IS_FILE (file));
	g_assert (NAUTILUS_IS_BOOKMARK (bookmark));
	g_assert (file == bookmark->details->file);

	should_emit_appearance_changed_signal = FALSE;
	should_emit_contents_changed_signal = FALSE;
	file_uri = nautilus_file_get_uri (file);

	if (!eel_uris_match (bookmark->details->uri, file_uri)) {
		g_free (bookmark->details->uri);
		bookmark->details->uri = file_uri;
		should_emit_contents_changed_signal = TRUE;
	} else {
		g_free (file_uri);
	}

	if (nautilus_file_is_gone (file)) {
		/* The file we were monitoring has been deleted,
		 * or moved in a way that we didn't notice. Make 
		 * a spanking new NautilusFile object for this 
		 * location so if a new file appears in this place 
		 * we will notice.
		 */
		nautilus_bookmark_disconnect_file (bookmark);
		nautilus_bookmark_connect_file (bookmark);
		should_emit_appearance_changed_signal = TRUE;		
	} else if (nautilus_bookmark_update_icon (bookmark)) {
		/* File hasn't gone away, but it has changed
		 * in a way that affected its icon.
		 */
		should_emit_appearance_changed_signal = TRUE;
	}

	if (should_emit_appearance_changed_signal) {
		g_signal_emit (bookmark, signals[APPEARANCE_CHANGED], 0);
	}

	if (should_emit_contents_changed_signal) {
		g_signal_emit (bookmark, signals[CONTENTS_CHANGED], 0);
	}
}

/**
 * nautilus_bookmark_set_icon_to_default:
 * 
 * Reset the icon to either the missing bookmark icon or the generic
 * bookmark icon, depending on whether the file still exists.
 */
static void
nautilus_bookmark_set_icon_to_default (NautilusBookmark *bookmark)
{
	const char *icon_name;

	if (bookmark->details->icon != NULL) {
		nautilus_scalable_icon_unref (bookmark->details->icon);
	}

	if (nautilus_bookmark_uri_known_not_to_exist (bookmark)) {
		icon_name = MISSING_BOOKMARK_ICON_NAME;
	} else {
		icon_name = GENERIC_BOOKMARK_ICON_NAME;
	}
	bookmark->details->icon = nautilus_scalable_icon_new_from_text_pieces 
		(NULL, NULL, icon_name, NULL, NULL);
}

/**
 * nautilus_bookmark_new:
 *
 * Create a new NautilusBookmark from a text uri and a display name.
 * The initial icon for the bookmark will be based on the information 
 * already available without any explicit action on NautilusBookmark's
 * part.
 * 
 * @uri: Any uri, even a malformed or non-existent one.
 * @name: A string to display to the user as the bookmark's name.
 * 
 * Return value: A newly allocated NautilusBookmark.
 * 
 **/
NautilusBookmark *
nautilus_bookmark_new (const char *uri, const char *name)
{
	return nautilus_bookmark_new_with_icon (uri, name, NULL);
}

static void
nautilus_bookmark_disconnect_file (NautilusBookmark *bookmark)
{
	g_assert (NAUTILUS_IS_BOOKMARK (bookmark));
	
	if (bookmark->details->file != NULL) {
		g_signal_handlers_disconnect_by_func (bookmark->details->file,
						      G_CALLBACK (bookmark_file_changed_callback),
						      bookmark);
		nautilus_file_unref (bookmark->details->file);
		bookmark->details->file = NULL;
	}

	if (bookmark->details->icon != NULL) {
		nautilus_scalable_icon_unref (bookmark->details->icon);
		bookmark->details->icon = NULL;
	}
}

static void
nautilus_bookmark_connect_file (NautilusBookmark *bookmark)
{
	g_assert (NAUTILUS_IS_BOOKMARK (bookmark));

	if (bookmark->details->file != NULL) {
		return;
	}

	if (!nautilus_bookmark_uri_known_not_to_exist (bookmark)) {
		bookmark->details->file = nautilus_file_get (bookmark->details->uri);
		g_assert (!nautilus_file_is_gone (bookmark->details->file));

		g_signal_connect_object (bookmark->details->file, "changed",
					 G_CALLBACK (bookmark_file_changed_callback), bookmark, 0);
	}	

	/* Set icon based on available information; don't force network i/o
	 * to get any currently unknown information. 
	 */
	if (!nautilus_bookmark_update_icon (bookmark)) {
		if (bookmark->details->icon == NULL || bookmark->details->file == NULL) {
			nautilus_bookmark_set_icon_to_default (bookmark);
		}
	}
}

NautilusBookmark *
nautilus_bookmark_new_with_icon (const char *uri, const char *name, 
				 NautilusScalableIcon *icon)
{
	NautilusBookmark *new_bookmark;

	new_bookmark = NAUTILUS_BOOKMARK (g_object_new (NAUTILUS_TYPE_BOOKMARK, NULL));
	g_object_ref (new_bookmark);
	gtk_object_sink (GTK_OBJECT (new_bookmark));

	new_bookmark->details->name = g_strdup (name);
	new_bookmark->details->uri = g_strdup (uri);

	if (icon != NULL) {
		nautilus_scalable_icon_ref (icon);
	}
	new_bookmark->details->icon = icon;

	nautilus_bookmark_connect_file (new_bookmark);

	return new_bookmark;
}				 

static GtkWidget *
create_image_widget_for_bookmark (NautilusBookmark *bookmark)
{
	GdkPixbuf *pixbuf;
	GtkWidget *widget;

	pixbuf = nautilus_bookmark_get_pixbuf (bookmark, NAUTILUS_ICON_SIZE_FOR_MENUS, FALSE);
	if (pixbuf == NULL) {
		return NULL;
	}

        widget = gtk_image_new_from_pixbuf (pixbuf);

	g_object_unref (pixbuf);
	return widget;
}

/**
 * nautilus_bookmark_menu_item_new:
 * 
 * Return a menu item representing a bookmark.
 * @bookmark: The bookmark the menu item represents.
 * Return value: A newly-created bookmark, not yet shown.
 **/ 
GtkWidget *
nautilus_bookmark_menu_item_new (NautilusBookmark *bookmark)
{
	GtkWidget *menu_item;
	GtkWidget *image_widget;
	GtkWidget *label;
	char *display_name;
	
	menu_item = gtk_image_menu_item_new ();

	image_widget = create_image_widget_for_bookmark (bookmark);
	if (image_widget != NULL) {
		gtk_widget_show (image_widget);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_item),
					       image_widget);
	}
	display_name = eel_truncate_text_for_menu_item (bookmark->details->name);
	label = gtk_label_new (display_name);
	g_free (display_name);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);

	gtk_container_add (GTK_CONTAINER (menu_item), label);
	gtk_widget_show (label);

	return menu_item;
}

gboolean
nautilus_bookmark_uri_known_not_to_exist (NautilusBookmark *bookmark)
{
	char *path_name;
	gboolean exists;

	/* Convert to a path, returning FALSE if not local. */
	path_name = gnome_vfs_get_local_path_from_uri (bookmark->details->uri);
	if (path_name == NULL) {
		return FALSE;
	}

	/* Now check if the file exists (sync. call OK because it is local). */
	exists = g_file_test (path_name, G_FILE_TEST_EXISTS);
	g_free (path_name);

	return !exists;
}
