/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
   eel-open-with-dialog.c: an open-with dialog

   Copyright (C) 2004 Novell, Inc.

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

   Authors: Dave Camp <dave@novell.com>
*/

#include <config.h>
#include "eel-open-with-dialog.h"

#include "eel-mime-extensions.h"
#include "eel-stock-dialogs.h"

#include <string.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkfilechooserdialog.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkicontheme.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkvbox.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnomevfs/gnome-vfs-uri.h>

struct _EelOpenWithDialogDetails {
	char *uri;

	char *mime_type;
	char *mime_description;

	gboolean new_mime_type;
	char *new_glob;

	GtkWidget *label;
	GtkWidget *entry;

	GtkWidget *open_label;
	GtkWidget *open_image;
};

enum {
	RESPONSE_OPEN
};

enum {
	APPLICATION_SELECTED,
	LAST_SIGNAL
};

static gpointer parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

static void
eel_open_with_dialog_finalize (GObject *object)
{
	EelOpenWithDialog *dialog;

	dialog = EEL_OPEN_WITH_DIALOG (object);

	g_free (dialog->details->uri);
	g_free (dialog->details->mime_type);
	g_free (dialog->details->mime_description);
	g_free (dialog->details->new_glob);

	g_free (dialog->details);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
eel_open_with_dialog_destroy (GtkObject *object)
{
	EelOpenWithDialog *dialog;

	dialog = EEL_OPEN_WITH_DIALOG (object);

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

/* An application is valid if:
 *
 * 1) The file exists
 * 2) The user has permissions to run the file
 *
 * At some point, we should probably allow desktop files, too.
 */
static gboolean
check_application (EelOpenWithDialog *dialog)
{
	char *path = NULL;
	char **argv = NULL;
	int argc;
	GError *error = NULL;
	gint retval = TRUE;

	g_shell_parse_argv (gtk_entry_get_text (GTK_ENTRY (dialog->details->entry)),
			    &argc, &argv, &error);

	if (error) {
		eel_show_error_dialog (_("Could not run application"),
				       error->message,
				       _("Could not run application"),
				       GTK_WINDOW (dialog));
		g_error_free (error);
		retval = FALSE;
		goto cleanup;
	}

	/* First, we test to see if the file is an absolute filepath, and if so,
	 * we check if it is an executable file.  Note that
	 * g_find_program_in_path() doesn't test absolute files, and doesn't
	 * check directories.
	 */
	if (argv[0][0] == '/' &&
	    (! (g_file_test (argv[0], G_FILE_TEST_IS_EXECUTABLE) &&
		g_file_test (argv[0], G_FILE_TEST_IS_REGULAR)))) {
		char *error_message;

		error_message = g_strdup_printf (_("The application '%s' is not a valid executable"),
						 argv[0]);

		eel_show_error_dialog (_("Could not open application"),
				       error_message,
				       _("Could not open application"),
				       GTK_WINDOW (dialog));
		g_free (error_message);
		retval = FALSE;
		goto cleanup;
	}

	/* Next, we check that argv[0] is in $PATH */
	path = g_find_program_in_path (argv[0]);
	if (!path) {
		char *error_message;

		error_message = g_strdup_printf (_("Could not find '%s'"),
						 argv[0]);

		eel_show_error_dialog (_("Could not find application"),
				       error_message,
				       _("Could not find application"),
				       GTK_WINDOW (dialog));
		g_free (error_message);
		retval = FALSE;
		goto cleanup;
	}

 cleanup:
	g_strfreev (argv);
	g_free (path);

	return retval;
}

static char *
get_app_name (EelOpenWithDialog *dialog)
{
	GError *error = NULL;
	char *basename;
	char *unquoted;
	char **argv;
	int argc;

	g_shell_parse_argv (gtk_entry_get_text (GTK_ENTRY (dialog->details->entry)),
			    &argc, &argv, &error);

	if (error) {
		eel_show_error_dialog (_("Could not run application"),
				       error->message,
				       _("Could not run application"),
				       GTK_WINDOW (dialog));
		g_error_free (error);

		return NULL;
	}

	unquoted = g_shell_unquote (argv[0], &error);

	if (!error) {
		basename = g_path_get_basename (unquoted);
	} else {
		basename = g_strdup (argv[0]);
		g_error_free (error);
	}

	g_free (unquoted);
	g_strfreev (argv);

	return basename;
}

/* This will check if the application the user wanted exists will return that
 * application.  If it doesn't exist, it will create one and return that.
 */
static GnomeVFSMimeApplication *
add_or_find_application (EelOpenWithDialog *dialog)
{
	GnomeVFSMimeApplication *app;
	char *app_name;

	if (dialog->details->new_mime_type) {
		eel_mime_add_glob_type (dialog->details->mime_type,
					dialog->details->mime_description,
					dialog->details->new_glob);

	}
	
	app = eel_mime_check_for_duplicates (dialog->details->mime_type,
					     gtk_entry_get_text (GTK_ENTRY (dialog->details->entry)));
	if (app != NULL)
		return app;


	app_name = get_app_name (dialog);

	if (app_name) {
		app = eel_mime_add_application (dialog->details->mime_type,
						gtk_entry_get_text (GTK_ENTRY (dialog->details->entry)),
						app_name,
						FALSE);
	} else {
		app = NULL;
	}

	if (!app) {
		eel_show_error_dialog (_("Could not add application"),
				       _("Could not add application to the application database"),
				       _("Could not add application"),
				       GTK_WINDOW (dialog));
	}

	return app;
}

static void
emit_application_selected (EelOpenWithDialog *dialog,
			   GnomeVFSMimeApplication *application)
{
	g_signal_emit (G_OBJECT (dialog), signals[APPLICATION_SELECTED], 0,
		       application);
}

static void
response_cb (EelOpenWithDialog *dialog,
	     int response_id,
	     gpointer data)
{
	GnomeVFSMimeApplication *application;

	switch (response_id) {
	case RESPONSE_OPEN:
		if (check_application (dialog)) {
			application = add_or_find_application (dialog);

			if (application) {
				emit_application_selected (dialog, application);
				gnome_vfs_mime_application_free (application);

				gtk_widget_destroy (GTK_WIDGET (dialog));
			}
		}

		break;
	case GTK_RESPONSE_NONE:
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL:
		gtk_widget_destroy (GTK_WIDGET (dialog));
		break;
	default :
		g_assert_not_reached ();
	}

}


static void
eel_open_with_dialog_class_init (EelOpenWithDialogClass *class)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);

	gobject_class = G_OBJECT_CLASS (class);
	gobject_class->finalize = eel_open_with_dialog_finalize;

	object_class = GTK_OBJECT_CLASS (class);
	object_class->destroy = eel_open_with_dialog_destroy;

	signals[APPLICATION_SELECTED] =
		g_signal_new ("application_selected",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EelOpenWithDialogClass,
					       application_selected),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER,
			      G_TYPE_NONE,
			      1, G_TYPE_POINTER);
}

static void
chooser_response_cb (GtkFileChooser *chooser,
		     int response,
		     gpointer user_data)
{
	EelOpenWithDialog *dialog;

	dialog = EEL_OPEN_WITH_DIALOG (user_data);

	if (response == GTK_RESPONSE_OK) {
		char *filename;

		filename = gtk_file_chooser_get_filename (chooser);

		if (filename) {
			char *quoted_text;

			quoted_text = g_shell_quote (filename);

			gtk_entry_set_text (GTK_ENTRY (dialog->details->entry),
					    quoted_text);
			gtk_entry_set_position (GTK_ENTRY (dialog->details->entry), -1);
			g_free (quoted_text);
			g_free (filename);
		}
	}

	gtk_widget_destroy (GTK_WIDGET (chooser));
}

static void
browse_clicked_cb (GtkWidget *button,
		   gpointer user_data)
{
	EelOpenWithDialog *dialog;
	GtkWidget *chooser;

	dialog = EEL_OPEN_WITH_DIALOG (user_data);

	chooser = gtk_file_chooser_dialog_new (_("Select an Application"),
					       GTK_WINDOW (dialog),
					       GTK_FILE_CHOOSER_ACTION_OPEN,
					       GTK_STOCK_CANCEL,
					       GTK_RESPONSE_CANCEL,
					       GTK_STOCK_OPEN,
					       GTK_RESPONSE_OK,
					       NULL);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (chooser), TRUE);
	g_signal_connect (chooser, "response",
			  G_CALLBACK (chooser_response_cb), dialog);
	gtk_dialog_set_default_response (GTK_DIALOG (chooser),
					 GTK_RESPONSE_OK);
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (chooser), TRUE);
	gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (chooser),
					      FALSE);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser),
					     "/usr/bin");

	gtk_widget_show (chooser);
}

static void
entry_changed_cb (GtkWidget *entry,
		  GtkWidget *button)
{
	if (gtk_entry_get_text (GTK_ENTRY (entry))[0] == '\000')
		gtk_widget_set_sensitive (button, FALSE);
	else
		gtk_widget_set_sensitive (button, TRUE);
}

static GtkWidget *
get_run_dialog_image (void)
{
	GtkWidget *image;
	static gboolean initted = FALSE;

	if (!initted) {
		GtkIconFactory *ifactory;
		GtkIconSet *iset;
		GtkIconSource *isource;

		ifactory = gtk_icon_factory_new ();
		iset = gtk_icon_set_new ();
		isource = gtk_icon_source_new ();
		gtk_icon_source_set_icon_name (isource, "gnome-run");
		gtk_icon_set_add_source (iset, isource);
		gtk_icon_factory_add (ifactory, "gnome-run", iset);
		gtk_icon_factory_add_default (ifactory);

		initted = TRUE;
	}

	image = gtk_image_new_from_stock ("gnome-run",
					  GTK_ICON_SIZE_DIALOG);

	return image;
}

static void
eel_open_with_dialog_instance_init (EelOpenWithDialog *dialog)
{
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *button;
	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *align;

	dialog->details = g_new0 (EelOpenWithDialogDetails, 1);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Open With"));
	gtk_window_set_default_size (GTK_WINDOW (dialog), 300, -1);
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);
	hbox = gtk_hbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox,
			    TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	image = get_run_dialog_image ();
	gtk_misc_set_alignment (GTK_MISC (image), 0.5, 0.0);
	gtk_widget_show (image);
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	/* Pack the text and the entry */
	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);

	dialog->details->label = gtk_label_new ("");
	gtk_misc_set_alignment (GTK_MISC (dialog->details->label), 0.0, 0.5);
	gtk_label_set_line_wrap (GTK_LABEL (dialog->details->label), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), dialog->details->label,
			    FALSE, FALSE, 0);
	gtk_widget_show (dialog->details->label);

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	dialog->details->entry = gtk_entry_new ();
	gtk_entry_set_activates_default (GTK_ENTRY (dialog->details->entry), TRUE);

	gtk_box_pack_start (GTK_BOX (hbox), dialog->details->entry,
			    TRUE, TRUE, 0);
	gtk_widget_show (dialog->details->entry);

	button = gtk_button_new_with_mnemonic (_("_Browse..."));
	g_signal_connect (button, "clicked",
			  G_CALLBACK (browse_clicked_cb), dialog);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
	gtk_widget_show (button);

	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       GTK_STOCK_CANCEL,
			       GTK_RESPONSE_CANCEL);


	/* Create a custom stock icon */
	button = gtk_button_new ();

	/* Hook up the entry to the button */
	gtk_widget_set_sensitive (button, FALSE);
	g_signal_connect (G_OBJECT (dialog->details->entry), "changed",
			  G_CALLBACK (entry_changed_cb), button);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox);

	image = gtk_image_new_from_stock (GTK_STOCK_EXECUTE,
					  GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (image);
	dialog->details->open_image = image;

	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic ("_Open");
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), GTK_WIDGET (button));
	gtk_widget_show (label);
	dialog->details->open_label = label;

	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_widget_show (align);

	gtk_widget_show (button);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);


	gtk_container_add (GTK_CONTAINER (align), hbox);
	gtk_container_add (GTK_CONTAINER (button), align);

	gtk_dialog_add_action_widget (GTK_DIALOG (dialog),
				      button, RESPONSE_OPEN);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 RESPONSE_OPEN);

	g_signal_connect (dialog, "response",
			  G_CALLBACK (response_cb),
			  dialog);
}

static char *
get_extension (const char *basename)
{
	char *p;

	p = strrchr (basename, '.');

	if (p && *(p + 1) != '\0') {
		return g_strdup (p + 1);
	} else {
		return NULL;
	}
}

static void
set_uri_and_mime_type (EelOpenWithDialog *dialog,
		       const char *uri,
		       const char *mime_type)
{
	char *label;
	char *name;
	GnomeVFSURI *vfs_uri;

	dialog->details->uri = g_strdup (uri);

	vfs_uri = gnome_vfs_uri_new (uri);

	name = gnome_vfs_uri_extract_short_name (vfs_uri);

	if (!strcmp (mime_type, "application/octet-stream")) {
		char *extension;

		extension = get_extension (uri);

		if (!extension) {
			g_warning ("No extension, not implemented yet");
			return;
		}

		dialog->details->mime_type =
			g_strdup_printf ("application/x-extension-%s",
					 extension);
		dialog->details->mime_description =
			g_strdup_printf (_("%s document"), extension);
		dialog->details->new_glob = g_strdup_printf ("*.%s",
							     extension);
		dialog->details->new_mime_type = TRUE;

		g_free (extension);
	} else {
		char *description;

		dialog->details->mime_type = g_strdup (mime_type);
		description = g_strdup (gnome_vfs_mime_get_description (mime_type));

		if (description == NULL) {
			description = g_strdup (_("Unknown"));
		}

		dialog->details->mime_description = description;
	}

	label = g_strdup_printf (_("Open <i>%s</i> and other files of type \"%s\" with:"), name, dialog->details->mime_description);

	gtk_label_set_markup (GTK_LABEL (dialog->details->label), label);

	g_free (label);
	g_free (name);
	gnome_vfs_uri_unref (vfs_uri);
}

GtkWidget *
eel_open_with_dialog_new (const char *uri,
			  const char *mime_type)
{
	GtkWidget *dialog;

	dialog = gtk_widget_new (EEL_TYPE_OPEN_WITH_DIALOG, NULL);

	set_uri_and_mime_type (EEL_OPEN_WITH_DIALOG (dialog), uri, mime_type);

	return dialog;
}

GtkWidget* 
eel_add_application_dialog_new (const char *uri,
				const char *mime_type)
{
	EelOpenWithDialog *dialog;
	
	dialog = EEL_OPEN_WITH_DIALOG (eel_open_with_dialog_new (uri, mime_type));
	
	gtk_label_set_text_with_mnemonic (GTK_LABEL (dialog->details->open_label),
					  _("_Add"));
	gtk_image_set_from_stock (GTK_IMAGE (dialog->details->open_image),
				  GTK_STOCK_ADD,
				  GTK_ICON_SIZE_BUTTON);

	gtk_window_set_title (GTK_WINDOW (dialog), _("Add Application"));

	return GTK_WIDGET (dialog);
}

GType
eel_open_with_dialog_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (EelOpenWithDialogClass),
			NULL,
			NULL,
			(GClassInitFunc)eel_open_with_dialog_class_init,
			NULL,
			NULL,
			sizeof (EelOpenWithDialog),
			0,
			(GInstanceInitFunc)eel_open_with_dialog_instance_init,
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
					       "EelOpenWithDialog",
					       &info, 0);
	}

	return type;
}
