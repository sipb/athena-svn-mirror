/*
 * Copyright (C) 2001, 2002 Anders Carlsson <andersca@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "gconf-stock-icons.h"

#include <gtk/gtkiconfactory.h>

static GtkIconSet *
create_icon_set_from_filename (const gchar *filename)
{
	GtkIconSet *set;
	GtkIconSource *source;
	GdkPixbuf *pixbuf;
	gchar *path;
	
	path = g_strdup_printf (GCONF_EDITOR_IMAGEDIR"/%s", filename);

	pixbuf = gdk_pixbuf_new_from_file (path, NULL);
	g_free (path);
	
	if (pixbuf == NULL) {
		return NULL;
	}
	
	set = gtk_icon_set_new ();
	source = gtk_icon_source_new ();
	gtk_icon_source_set_pixbuf (source, pixbuf);
	g_object_unref (pixbuf);
	gtk_icon_source_set_size (source, GTK_ICON_SIZE_MENU);
	gtk_icon_source_set_size_wildcarded (source, TRUE);
	gtk_icon_set_add_source (set, source);
	gtk_icon_source_free (source);
	
	return set;
}

void
gconf_stock_icons_register (void)
{
	GtkIconFactory *icon_factory;
	GtkIconSet *set;
	static gboolean initialized = FALSE;

	if (initialized == TRUE) {
		return;
	}

	icon_factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (icon_factory);

	set = create_icon_set_from_filename ("stock-about-16.png");

	if (set != NULL) {
		gtk_icon_factory_add (icon_factory, GCONF_STOCK_ABOUT, set);
		gtk_icon_set_unref (set);
	}

	set = create_icon_set_from_filename ("folder-closed.png");

	if (set != NULL) {
		gtk_icon_factory_add (icon_factory, GCONF_STOCK_BOOKMARK, set);
		gtk_icon_set_unref (set);
	}

	initialized = TRUE;
}
