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

#include "gconf-editor-application.h"

#include "gconf-editor-window.h"
#include <gtk/gtkmain.h>

static GSList *editor_windows = NULL;

static void
gconf_editor_application_window_destroyed (GtkObject *window)
{
	editor_windows = g_slist_remove (editor_windows, window);

	if (editor_windows == NULL)
		gtk_main_quit ();
}

GtkWidget *
gconf_editor_application_create_editor_window (void)
{
	GtkWidget *window;

	window = g_object_new (GCONF_TYPE_EDITOR_WINDOW, NULL);
	g_signal_connect (window, "destroy",
			  G_CALLBACK (gconf_editor_application_window_destroyed), NULL);
	
	editor_windows = g_slist_prepend (editor_windows, window);

	return window;
}

