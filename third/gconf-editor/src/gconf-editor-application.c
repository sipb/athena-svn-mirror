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

#include <config.h>
#include "gconf-editor-application.h"

#include "gconf-editor-window.h"
#include "gconf-tree-model.h"
#include "gconf-list-model.h"
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
gconf_editor_application_create_editor_window (int type)
{
	GConfEngine *engine;
	GtkWidget *window;
	GConfEditorWindow *gconfwindow;
	GError *error = NULL;

	window = g_object_new (GCONF_TYPE_EDITOR_WINDOW, NULL);

	gconfwindow = GCONF_EDITOR_WINDOW (window);

	switch (type) {
	case GCONF_EDITOR_WINDOW_NORMAL:
		gconfwindow->client = gconf_client_get_default ();
		break;
	case GCONF_EDITOR_WINDOW_DEFAULTS:
		engine = gconf_engine_get_for_address (GCONF_DEFAULTS_SOURCE, &error);
		if (error) {
			gconf_editor_window_popup_error_dialog (&gconfwindow->parent_instance,
								_("Cannot create GConf engine. Error was:\n%s"), error);
			gconf_editor_application_window_destroyed (GTK_OBJECT (window));
			return NULL;
		}
		gconfwindow->client = gconf_client_get_for_engine (engine);
		gconf_engine_unref (engine);
		break;
	case GCONF_EDITOR_WINDOW_MANDATORY:
		engine = gconf_engine_get_for_address (GCONF_MANDATORY_SOURCE, &error);
		if (error) {
			gconf_editor_window_popup_error_dialog (&gconfwindow->parent_instance,
								_("Cannot create GConf engine. Error was:\n%s"), error);
			gconf_editor_application_window_destroyed (GTK_OBJECT (window));
			return NULL;
		}
		gconfwindow->client = gconf_client_get_for_engine (engine);
		gconf_engine_unref (engine);
		break;
	default:
		g_assert_not_reached ();
	}

	gconf_tree_model_set_client (GCONF_TREE_MODEL (gconfwindow->tree_model), gconfwindow->client);
	gconf_list_model_set_client (GCONF_LIST_MODEL (gconfwindow->list_model), gconfwindow->client);
	
	g_signal_connect (window, "destroy",
			  G_CALLBACK (gconf_editor_application_window_destroyed), NULL);
	
	editor_windows = g_slist_prepend (editor_windows, window);

	gconf_editor_window_expand_first (gconfwindow);

	return window;
}

