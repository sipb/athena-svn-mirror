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

#include <gtk/gtk.h>
#include <libintl.h>

#include "gconf-editor-application.h"
#include "gconf-stock-icons.h"
#include "gconf-message-dialog.h"

#define _(x) gettext(x)

gint
main (gint argc, gchar **argv)
{
	GtkWidget *window, *dialog;

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	/* Register our stock icons */
	gconf_stock_icons_register ();

	window = gconf_editor_application_create_editor_window ();
	gtk_widget_show (window);
	
	/* Put up a caveat dialog */
	if (gconf_message_dialog_should_show ("caveat-dialog")) {
		dialog = gconf_message_dialog_new (GTK_WINDOW (window), 0,
						   GTK_MESSAGE_WARNING,
						   GTK_BUTTONS_OK,
						   "caveat-dialog",
						   _("This tool allows you to directly edit your configuration database. "
						     "This is not the recommended way of setting desktop preferences. "
						     "Use this tool at your own risk."));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	
	gtk_main ();
	
	return 0;
}
