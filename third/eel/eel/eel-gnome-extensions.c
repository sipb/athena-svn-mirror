/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-gnome-extensions.c - implementation of new functions that operate on
                            gnome classes. Perhaps some of these should be
  			    rolled into gnome someday.

   Copyright (C) 1999, 2000, 2001 Eazel, Inc.

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

   Authors: Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include "eel-gnome-extensions.h"

#include "eel-art-extensions.h"
#include "eel-gdk-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-gtk-extensions.h"
#include "eel-stock-dialogs.h"
#include "eel-i18n.h"
#include "egg-screen-exec.h"
#include <X11/Xatom.h>
#include <errno.h>
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <gtk/gtkwidget.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_rgb.h>
#include <libgnome/gnome-exec.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-file-entry.h>
#include <libgnomeui/gnome-icon-list.h>
#include <libgnomeui/gnome-icon-sel.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo-activation/bonobo-activation.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* structure for the icon selection dialog */
struct IconSelectionData {
	GtkWidget *dialog;
        GtkWidget *icon_selection;
	GtkWidget *file_entry;
	GtkWindow *owning_window;
	gboolean dismissed;
	EelIconSelectionFunction selection_function;
	gpointer callback_data;
};

typedef struct IconSelectionData IconSelectionData;

void 
eel_gnome_shell_execute_on_screen (const char *command,
				   GdkScreen  *screen)
{
	GError *error = NULL;

	if (screen == NULL) {
		screen = gdk_screen_get_default ();
	}

	if (!egg_screen_execute_command_line_async (screen, command, &error)) {
		g_warning ("Error starting command '%s': %s\n", command, error->message);
		g_error_free (error);
	}
}

void 
eel_gnome_shell_execute (const char *command)
{
	eel_gnome_shell_execute_on_screen (command, NULL);
}

/* Return a command string containing the path to a terminal on this system. */

static char *
try_terminal_command (const char *program,
		      const char *args)
{
	char *program_in_path, *quoted, *result;

	if (program == NULL) {
		return NULL;
	}

	program_in_path = g_find_program_in_path (program);
	if (program_in_path == NULL) {
		return NULL;
	}

	quoted = g_shell_quote (program_in_path);
	if (args == NULL || args[0] == '\0') {
		return quoted;
	}
	result = g_strconcat (quoted, " ", args, NULL);
	g_free (quoted);
	return result;
}

static char *
try_terminal_command_argv (int argc,
			   char **argv)
{
	GString *string;
	int i;
	char *quoted, *result;

	if (argc == 0) {
		return NULL;
	}

	if (argc == 1) {
		return try_terminal_command (argv[0], NULL);
	}
	
	string = g_string_new (argv[1]);
	for (i = 2; i < argc; i++) {
		quoted = g_shell_quote (argv[i]);
		g_string_append_c (string, ' ');
		g_string_append (string, quoted);
		g_free (quoted);
	}
	result = try_terminal_command (argv[0], string->str);
	g_string_free (string, TRUE);

	return result;
}

static char *
get_terminal_command_prefix (gboolean for_command)
{
	int argc;
	char **argv;
	char *command;
	guint i;
	static const char *const commands[][3] = {
		{ "gnome-terminal", "-x",                                      "" },
		{ "dtterm",         "-e",                                      "-ls" },
		{ "nxterm",         "-e",                                      "-ls" },
		{ "color-xterm",    "-e",                                      "-ls" },
		{ "rxvt",           "-e",                                      "-ls" },
		{ "xterm",          "-e",                                      "-ls" },
	};

	/* Try the terminal from preferences. Use without any
	 * arguments if we are just doing a standalone terminal.
	 */
	argc = 0;
	argv = g_new0 (char *, 1);
	gnome_prepend_terminal_to_vector (&argc, &argv);

	command = NULL;
	if (argc != 0) {
		if (for_command) {
			command = try_terminal_command_argv (argc, argv);
		} else {
			/* Strip off the arguments in a lame attempt
			 * to make it be an interactive shell.
			 */
			command = try_terminal_command (argv[0], NULL);
		}
	}

	while (argc != 0) {
		g_free (argv[--argc]);
	}
	g_free (argv);

	if (command != NULL) {
		return command;
	}

	/* Try well-known terminal applications in same order that gmc did. */
	for (i = 0; i < G_N_ELEMENTS (commands); i++) {
		command = try_terminal_command (commands[i][0],
						commands[i][for_command ? 1 : 2]);
		if (command != NULL) {
			break;
		}
	}
	
	return command;
}

char *
eel_gnome_make_terminal_command (const char *command)
{
	char *prefix, *quoted, *terminal_command;

	if (command == NULL) {
		return get_terminal_command_prefix (FALSE);
	}
	prefix = get_terminal_command_prefix (TRUE);
	quoted = g_shell_quote (command);
	terminal_command = g_strconcat (prefix, " /bin/sh -c ", quoted, NULL);
	g_free (prefix);
	g_free (quoted);
	return terminal_command;
}

void
eel_gnome_open_terminal_on_screen (const char *command,
				   GdkScreen  *screen)
{
	char *command_line;
	
	command_line = eel_gnome_make_terminal_command (command);
	if (command_line == NULL) {
		g_message ("Could not start a terminal");
		return;
	}
	eel_gnome_shell_execute_on_screen (command_line, screen);
	g_free (command_line);
}

void
eel_gnome_open_terminal (const char *command)
{
	eel_gnome_open_terminal_on_screen (command, NULL);
}

/* create a new icon selection dialog */
static gboolean
widget_destroy_callback (gpointer callback_data)
{
	IconSelectionData *selection_data;

	selection_data = (IconSelectionData *) callback_data;
	gtk_widget_destroy (selection_data->dialog);
	g_free (selection_data);	
	return FALSE;
}

/* set the image of the file object to the selected file */
static void
icon_selected (IconSelectionData *selection_data)
{
	const char *icon_path;
	struct stat buf;
	GtkWidget *entry;
	
	gnome_icon_selection_stop_loading (GNOME_ICON_SELECTION (selection_data->icon_selection));

	/* Hide the dialog now, so the user can't press the buttons multiple
	   times. */
	gtk_widget_hide (selection_data->dialog);

	/* If we've already acted on the dialog, just return. */
	if (selection_data->dismissed)
		return;

	/* Set the flag to indicate we've acted on the dialog and are about
	   to destroy it. */
	selection_data->dismissed = TRUE;

	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (selection_data->file_entry));
	icon_path = gtk_entry_get_text (GTK_ENTRY (entry));
	
	/* if a specific file wasn't selected, put up a dialog to tell the
	 * user to pick something, and leave the picker up
	 */
	stat (icon_path, &buf);
	if (S_ISDIR (buf.st_mode)) {
		eel_show_error_dialog (_("No image was selected.  You must click on an image to select it."),
				       _("No selection made"),
				       selection_data->owning_window);
	} else {	 
		/* invoke the callback to inform it of the file path */
		selection_data->selection_function (icon_path, selection_data->callback_data);
	}
		
	/* We have to get rid of the icon selection dialog, but we can't do it now, since the
	 * file entry might still need it. Do it when the next idle rolls around
	 */ 
	gtk_idle_add (widget_destroy_callback, selection_data);
}

/* handle the cancel button being pressed */
static void
icon_cancel_pressed (IconSelectionData *selection_data)
{
	/* nothing to do if it's already been dismissed */
	if (selection_data->dismissed) {
		return;
	}
	
	gtk_widget_destroy (selection_data->dialog);
	g_free (selection_data);
}

/* handle an icon being selected by updating the file entry */
static void
list_icon_selected_callback (GnomeIconList *gil, int num, GdkEvent *event, IconSelectionData *selection_data)
{
	char *icon;
	GtkWidget *entry;
	
	icon = gnome_icon_selection_get_icon (GNOME_ICON_SELECTION (selection_data->icon_selection), TRUE);
	if (icon != NULL) {
		entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (selection_data->file_entry));
		gtk_entry_set_text (GTK_ENTRY (entry), icon);
		g_free (icon);
	}

	/* handle double-clicks as if the user pressed OK */
	if (event != NULL && event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1) {
		icon_selected (selection_data);
	}
}

static void
dialog_response_callback (GtkWidget *dialog, int response_id, IconSelectionData *selection_data)
{
	switch (response_id) {
	case GTK_RESPONSE_OK:
		icon_selected (selection_data);
		break;
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_DELETE_EVENT:
		icon_cancel_pressed (selection_data);
		break;
	default:
		break;
	}
}

/* handle the file entry changing */
static void
entry_activated_callback (GtkWidget *widget, IconSelectionData *selection_data)
{
	struct stat buf;
	GnomeIconSelection *icon_selection;
	const char *filename;

	filename = gtk_entry_get_text (GTK_ENTRY (widget));
	if (filename == NULL) {
		return;
	}
	
	if (stat (filename, &buf) == 0 && S_ISDIR (buf.st_mode)) {
		icon_selection = GNOME_ICON_SELECTION (selection_data->icon_selection);
		gnome_icon_selection_clear (icon_selection, TRUE);
		gnome_icon_selection_add_directory (icon_selection, filename);
		gnome_icon_selection_show_icons(icon_selection);
	} else {
		/* We pretend like ok has been called */
		icon_selected (selection_data);
	}
}

/* here's the actual routine that creates the icon selector.
   Note that this may return NULL if the dialog was destroyed before the
   icons were all loaded. GnomeIconSelection reenters the main loop while
   it loads the icons, so beware! */

GtkWidget *
eel_gnome_icon_selector_new (const char *title,
			     const char *icon_directory,
			     GtkWindow *owner,
			     EelIconSelectionFunction selected,
			     gpointer callback_data)
{
	GtkWidget *dialog, *icon_selection, *retval;
	GtkWidget *entry, *file_entry;
	IconSelectionData *selection_data;
	
	dialog = gtk_dialog_new_with_buttons (title, owner, 0, 
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	icon_selection = gnome_icon_selection_new ();

	file_entry = gnome_file_entry_new (NULL,NULL);
	
	selection_data = g_new0 (IconSelectionData, 1);
	selection_data->dialog = dialog;
 	selection_data->icon_selection = icon_selection;
 	selection_data->file_entry = file_entry;
 	selection_data->owning_window = owner;
	selection_data->selection_function = selected;
 	selection_data->callback_data = callback_data;
 	
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    file_entry, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    icon_selection, TRUE, TRUE, 0);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
	if (owner != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), owner);
 	}
 	gtk_window_set_wmclass (GTK_WINDOW (dialog), "file_selector", "Eel");
	gtk_widget_show_all (dialog);
	
	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (file_entry));
	
	if (icon_directory == NULL) {
		gtk_entry_set_text (GTK_ENTRY (entry), DATADIR "/pixmaps");
		gnome_icon_selection_add_directory (GNOME_ICON_SELECTION (icon_selection), DATADIR "/pixmaps");	
	} else {
		gtk_entry_set_text (GTK_ENTRY (entry), icon_directory);
		gnome_icon_selection_add_directory (GNOME_ICON_SELECTION (icon_selection), icon_directory);
	}
	
	g_signal_connect (dialog, "response",
			  G_CALLBACK (dialog_response_callback), selection_data);

	g_signal_connect_after (gnome_icon_selection_get_gil (GNOME_ICON_SELECTION (selection_data->icon_selection)),
				"select_icon",
				G_CALLBACK (list_icon_selected_callback), selection_data);

	g_signal_connect (entry, "activate",
			  G_CALLBACK (entry_activated_callback), selection_data);

	/* We add a weak pointer to the dialog, so we know if it has been
	   destroyed while the icons are being loaded. */
	eel_add_weak_pointer (&dialog);

	gnome_icon_selection_show_icons (GNOME_ICON_SELECTION (icon_selection));

	/* eel_remove_weak_pointer() will set dialog to NULL, so we need to
	   remember the dialog pointer here, if there is one. */
	retval = dialog;

	/* Now remove the weak pointer, if the dialog still exists. */
	eel_remove_weak_pointer (&dialog);

	return retval;
}


char *
eel_bonobo_make_registration_id (const char *iid)
{
	return bonobo_activation_make_registration_id (
		iid, DisplayString (gdk_display));
}

/**
 * eel_glade_get_file:
 * @filename: the XML file name.
 * @root: the widget node in @fname to start building from (or %NULL)
 * @domain: the translation domain for the XML file (or %NULL for default)
 * @first_required_widget: the name of the first widget we require
 * @: NULL terminated list of name, GtkWidget ** pairs.
 * 
 * Loads and parses the glade file, returns widget pointers for the names,
 * ensures that all the names are found.
 * 
 * Return value: the XML file, or NULL.
 **/
GladeXML *
eel_glade_get_file (const char *filename,
		    const char *root,
		    const char *domain,
		    const char *first_required_widget, ...)
{
	va_list     args;
	GladeXML   *gui;
	const char *name;
	GtkWidget **widget_ptr;
	GList      *ptrs, *l;

	va_start (args, first_required_widget);

	if (!(gui = glade_xml_new (filename, root, domain))) {
		g_warning ("Couldn't find necessary glade file '%s'", filename);
		va_end (args);
		return NULL;
	}

	ptrs = NULL;
	for (name = first_required_widget; name; name = va_arg (args, char *)) {
		widget_ptr = va_arg (args, void *);
		
		*widget_ptr = glade_xml_get_widget (gui, name);

		if (!*widget_ptr) {
			g_warning ("Glade file '%s' is missing widget '%s', aborting",
				   filename, name);
			
			for (l = ptrs; l; l = l->next) {
				*((gpointer *)l->data) = NULL;
			}
			g_list_free (ptrs);
			g_object_unref (gui);
			return NULL;
		} else {
			ptrs = g_list_prepend (ptrs, widget_ptr);
		}
	}

	va_end (args);

	return gui;
}
