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

#include <gconf/gconf.h>
#include <libintl.h>

#include "gconf-editor-application.h"
#include "gconf-stock-icons.h"
#include "gconf-editor-window.h"


static void
invalid_arg_error_dialog (GtkWindow  *parent,
			  const char *key,
			  const char *error_message)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (parent,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 _("Invalid key \"%s\": %s"),
					 key, error_message);
	g_signal_connect (dialog, "response",
			  G_CALLBACK (gtk_widget_destroy), NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	gtk_widget_show (dialog);
}

gint
main (gint argc, gchar **argv)
{
	GnomeProgram *program;
	GValue value = { 0 };
	poptContext pctx;
	GtkWidget *window;
	const char *initial_key;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);


	program = gnome_program_init ("gconf-editor", VERSION, 
	                              LIBGNOMEUI_MODULE, argc, argv,
	                              GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (program),
	                       GNOME_PARAM_POPT_CONTEXT,
	                       &value);
	pctx = g_value_get_pointer (&value);
	g_value_unset (&value);

	/* Register our stock icons */
	gconf_stock_icons_register ();

	window = gconf_editor_application_create_editor_window (GCONF_EDITOR_WINDOW_NORMAL);
	gtk_widget_show_now (window);

	initial_key = poptGetArg (pctx);

	if (initial_key != NULL) {
		char *reason;

		if (gconf_valid_key (initial_key, &reason))
			gconf_editor_window_go_to (GCONF_EDITOR_WINDOW (window),
			                           initial_key);
		else {
			invalid_arg_error_dialog (GTK_WINDOW (window),
			                          initial_key, reason);
			g_free (reason);
		}
	}
	poptFreeContext (pctx);

	gtk_main ();
	
	return 0;
}
