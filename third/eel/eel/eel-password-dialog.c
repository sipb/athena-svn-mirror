/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-password-dialog.c - A use password prompting dialog widget.

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

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#include <config.h>
#include "eel-password-dialog.h"

#include "eel-caption-table.h"
#include "eel-gnome-extensions.h"
#include "eel-gtk-macros.h"
#include "eel-i18n.h"
#include <gtk/gtkbox.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>

struct EelPasswordDialogDetails
{
	/* Attributes */
	char *username;
	char *password;
	gboolean readonly_username;
	gboolean remember;
	char *remember_label_text;

	/* Internal widgetry and flags */
	GtkWidget *table;
	GtkWidget *remember_button;
};

/* Caption table rows indices */
static const guint CAPTION_TABLE_USERNAME_ROW = 0;
static const guint CAPTION_TABLE_PASSWORD_ROW = 1;

/* Layout constants */
static const guint DIALOG_BORDER_WIDTH = 0;
static const guint CAPTION_TABLE_BORDER_WIDTH = 4;

/* EelPasswordDialogClass methods */
static void eel_password_dialog_class_init (EelPasswordDialogClass *password_dialog_class);
static void eel_password_dialog_init       (EelPasswordDialog      *password_dialog);

/* GObjectClass methods */
static void eel_password_dialog_finalize         (GObject                *object);


/* GtkDialog callbacks */
static void dialog_show_callback                 (GtkWidget              *widget,
						  gpointer                callback_data);
static void dialog_close_callback                (GtkWidget              *widget,
						  gpointer                callback_data);
/* Caption table callbacks */
static void caption_table_activate_callback      (GtkWidget              *widget,
						  guint                   entry,
						  gpointer                callback_data);


EEL_CLASS_BOILERPLATE (EelPasswordDialog,
			      eel_password_dialog,
			      gtk_dialog_get_type ());


static void
eel_password_dialog_class_init (EelPasswordDialogClass * klass)
{
	G_OBJECT_CLASS (klass)->finalize = eel_password_dialog_finalize;
}

static void
eel_password_dialog_init (EelPasswordDialog *password_dialog)
{
	password_dialog->details = g_new0 (EelPasswordDialogDetails, 1);
}

/* GObjectClass methods */
static void
eel_password_dialog_finalize (GObject *object)
{
	EelPasswordDialog *password_dialog;
	
	password_dialog = EEL_PASSWORD_DIALOG (object);
	
	g_free (password_dialog->details->username);
	g_free (password_dialog->details->password);
	g_free (password_dialog->details->remember_label_text);
	g_free (password_dialog->details);

	EEL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/* GtkDialog callbacks */
static void
dialog_show_callback (GtkWidget *widget, gpointer callback_data)
{
	EelPasswordDialog *password_dialog;

	password_dialog = EEL_PASSWORD_DIALOG (callback_data);

	if (password_dialog->details->readonly_username == FALSE) {
		eel_caption_table_entry_grab_focus (EEL_CAPTION_TABLE (password_dialog->details->table), 
						    CAPTION_TABLE_USERNAME_ROW);
	}
	else {
		eel_caption_table_entry_grab_focus (EEL_CAPTION_TABLE (password_dialog->details->table), 
						    CAPTION_TABLE_PASSWORD_ROW);
	}
}

static void
dialog_close_callback (GtkWidget *widget, gpointer callback_data)
{
	gtk_widget_hide (widget);
}

/* Caption table callbacks */
static void
caption_table_activate_callback (GtkWidget *widget, guint entry, gpointer callback_data)
{
	EelPasswordDialog *password_dialog;

	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (callback_data));

	password_dialog = EEL_PASSWORD_DIALOG (callback_data);

	/* If the username entry was activated, simply advance the focus to the password entry */
	if (entry == CAPTION_TABLE_USERNAME_ROW) {
		eel_caption_table_entry_grab_focus (EEL_CAPTION_TABLE (password_dialog->details->table), 
							 CAPTION_TABLE_PASSWORD_ROW);
	}
	/* If the password entry was activated, then simulate and OK button press to continue to hide process */
	else if (entry == CAPTION_TABLE_PASSWORD_ROW) {
		gtk_window_activate_default (GTK_WINDOW (password_dialog));
	}
}

/* Public EelPasswordDialog methods */
GtkWidget *
eel_password_dialog_new (const char	*dialog_title,
			      const char	*message,
			      const char	*username,
			      const char	*password,
			      gboolean		readonly_username)
{
	EelPasswordDialog *password_dialog;
	GtkLabel *message_label;

	password_dialog = EEL_PASSWORD_DIALOG (gtk_widget_new (eel_password_dialog_get_type (), NULL));

	gtk_window_set_title (GTK_WINDOW (password_dialog), dialog_title);
	gtk_dialog_add_buttons (GTK_DIALOG (password_dialog),
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OK, GTK_RESPONSE_OK,
				NULL);

	/* Setup the dialog */
	gtk_window_set_resizable (GTK_WINDOW (password_dialog), TRUE);

 	gtk_window_set_position (GTK_WINDOW (password_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (password_dialog), TRUE);

 	gtk_container_set_border_width (GTK_CONTAINER (password_dialog), DIALOG_BORDER_WIDTH);

	gtk_dialog_set_default_response (GTK_DIALOG (password_dialog), GTK_RESPONSE_OK);

	g_signal_connect (password_dialog, "show",
			  G_CALLBACK (dialog_show_callback), password_dialog);
	g_signal_connect (password_dialog, "close",
			  G_CALLBACK (dialog_close_callback), password_dialog);

	/* The table that holds the captions */
	password_dialog->details->table = eel_caption_table_new (2);
	
	g_signal_connect (password_dialog->details->table,
			    "activate",
			    G_CALLBACK (caption_table_activate_callback),
			    password_dialog);

	eel_caption_table_set_row_info (EEL_CAPTION_TABLE (password_dialog->details->table),
					     CAPTION_TABLE_USERNAME_ROW,
					     _("_Username:"),
					     "",
					     TRUE,
					     TRUE);

	eel_caption_table_set_row_info (EEL_CAPTION_TABLE (password_dialog->details->table),
					     CAPTION_TABLE_PASSWORD_ROW,
					     _("_Password:"),
					     "",
					     FALSE,
					     FALSE);
	
	/* Configure the GTK_DIALOG's vbox */
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (password_dialog)->vbox), 10);
	
	if (message) {
		message_label = GTK_LABEL (gtk_label_new (message));
		gtk_label_set_justify (message_label, GTK_JUSTIFY_LEFT);
		gtk_label_set_line_wrap (message_label, TRUE);

		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (password_dialog)->vbox),
				    GTK_WIDGET (message_label),
				    TRUE,	/* expand */
				    TRUE,	/* fill */
				    0);		/* padding */
	}

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (password_dialog)->vbox),
			    password_dialog->details->table,
			    TRUE,	/* expand */
			    TRUE,	/* fill */
			    0);		/* padding */
#if 0 /* FIXME: disabled for PR2 originally, when will we re-enable? */
	password_dialog->details->remember_button = 
		gtk_check_button_new_with_label (_("Remember this password"));

	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (password_dialog)->vbox),
			  password_dialog->details->remember_button,
			  TRUE,	/* expand */
			  TRUE,	/* fill */
			  4);		/* padding */
#else
	password_dialog->details->remember_button = NULL;
#endif	
	/* Configure the table */
 	gtk_container_set_border_width (GTK_CONTAINER(password_dialog->details->table), CAPTION_TABLE_BORDER_WIDTH);
	
	gtk_widget_show_all (GTK_DIALOG (password_dialog)->vbox);
	
	eel_password_dialog_set_username (password_dialog, username);
	eel_password_dialog_set_password (password_dialog, password);
	eel_password_dialog_set_readonly_username (password_dialog, readonly_username);
	
	return GTK_WIDGET (password_dialog);
}

gboolean
eel_password_dialog_run_and_block (EelPasswordDialog *password_dialog)
{
	gint button_clicked;

	g_return_val_if_fail (password_dialog != NULL, FALSE);
	g_return_val_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog), FALSE);
	
	button_clicked = gtk_dialog_run (GTK_DIALOG (password_dialog));
	gtk_widget_hide (GTK_WIDGET (password_dialog));

	return button_clicked == GTK_RESPONSE_OK;
}

void
eel_password_dialog_set_username (EelPasswordDialog	*password_dialog,
				       const char		*username)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog));

	eel_caption_table_set_entry_text (EEL_CAPTION_TABLE (password_dialog->details->table),
					       CAPTION_TABLE_USERNAME_ROW,
					       username);
}

void
eel_password_dialog_set_password (EelPasswordDialog	*password_dialog,
				       const char		*password)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog));
	
	eel_caption_table_set_entry_text (EEL_CAPTION_TABLE (password_dialog->details->table),
					       CAPTION_TABLE_PASSWORD_ROW,
					       password);
}

void
eel_password_dialog_set_readonly_username (EelPasswordDialog	*password_dialog,
						gboolean		readonly)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog));

	password_dialog->details->readonly_username = readonly;
	
	eel_caption_table_set_entry_readonly (EEL_CAPTION_TABLE (password_dialog->details->table),
						   CAPTION_TABLE_USERNAME_ROW,
						   readonly);
}

char *
eel_password_dialog_get_username (EelPasswordDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, NULL);
	g_return_val_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog), NULL);

	return eel_caption_table_get_entry_text (EEL_CAPTION_TABLE (password_dialog->details->table),
						      CAPTION_TABLE_USERNAME_ROW);
}

char *
eel_password_dialog_get_password (EelPasswordDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, NULL);
	g_return_val_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog), NULL);

	return eel_caption_table_get_entry_text (EEL_CAPTION_TABLE (password_dialog->details->table),
						      CAPTION_TABLE_PASSWORD_ROW);
}

gboolean
eel_password_dialog_get_remember (EelPasswordDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, FALSE);
	g_return_val_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog), FALSE);

#if 0	/* remove for PR2 */
	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (password_dialog->details->remember_button));
#else
	return FALSE;
#endif
}

void
eel_password_dialog_set_remember (EelPasswordDialog *password_dialog,
				       gboolean                remember)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog));

#if 0	/* remove for PR2 */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (password_dialog->details->remember_button),
				      remember);
#endif
}

void
eel_password_dialog_set_remember_label_text (EelPasswordDialog *password_dialog,
						  const char             *remember_label_text)
{
#if 0	/* remove for PR2 */
	GtkWidget *label;

	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (EEL_IS_PASSWORD_DIALOG (password_dialog));

	label = GTK_BIN (password_dialog->details->remember_button)->child;

	g_assert (label != NULL);
	g_assert (GTK_IS_LABEL (label));

	gtk_label_set_text (GTK_LABEL (label), remember_label_text);
#endif
}
