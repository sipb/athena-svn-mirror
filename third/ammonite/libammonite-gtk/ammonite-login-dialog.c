/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* ammonite-login-dialog.c: Dialog for prompting users to login to eazel services.
   Shamelessly stolen from nautilus-password-dialog.c 

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
   	    Mike Fleming <mfleming@eazel.com>
*/

#include <config.h>
#include "ammonite-login-dialog.h"
#include "nautilus-caption-table.h"

#include <libgnomeui/gnome-stock.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtksignal.h>
#include "nautilus-gtk-macros.h"
#include "copy-paste.h"

struct _AmmoniteLoginDialogDetails
{
	gboolean	username_should_focus;

	/* Internal widgetry and flags */
	GtkWidget	*table;
	GtkWidget	*remember_button;
	GtkLabel	*message;

	enum {
		BUTTON_MODE_FORGOTPW	 	= 0,
		BUTTON_MODE_REGISTER		= 1,
		BUTTON_MODE_FAIL	 	= 2,
	} button_mode;
};

/* [LoginDialogReturn][button_mode] == button index */
static const int button_mappings [][3] = {
			/*    FORGOT  REGISTER  FAIL */
	/* BUTTON_OK	   */	{2,	2,	-1},
	/* BUTTON_CANCEL   */	{1,	1,	2},
	/* BUTTON_REGISTER */	{-1,	0,	1},
	/* BUTTON_FORGOTPW */	{0,	-1,	0}
};


static const char * forgot_pw_buttons[] =
{
	_("I forgot my password"),
	GNOME_STOCK_BUTTON_CANCEL,
	GNOME_STOCK_BUTTON_OK,
	NULL
};

static const char * register_buttons[] =
{
	_("Register"),
	GNOME_STOCK_BUTTON_CANCEL,
	GNOME_STOCK_BUTTON_OK,
	NULL
};

static const char * fail_buttons[] =
{
	_("Register"),
	_("I forgot my password"),
	GNOME_STOCK_BUTTON_OK,
	NULL
};



/* Caption table rows indeces */
static const guint CAPTION_TABLE_USERNAME_ROW = 0;
static const guint CAPTION_TABLE_PASSWORD_ROW = 1;

/* Layout constants */
static const guint DIALOG_BORDER_WIDTH = 0;
static const guint CAPTION_TABLE_BORDER_WIDTH = 4;

/* AmmoniteLoginDialogClass methods */
static void ammonite_login_dialog_initialize_class (AmmoniteLoginDialogClass *password_dialog_class);
static void ammonite_login_dialog_initialize       (AmmoniteLoginDialog      *password_dialog);



/* GtkObjectClass methods */
static void ammonite_login_dialog_destroy          (GtkObject                   *object);


/* GtkDialog callbacks */
static void dialog_show_callback                      (GtkWidget                   *widget,
						       gpointer                     callback_data);
static void dialog_close_callback                     (GtkWidget                   *widget,
						       gpointer                     callback_data);
/* Caption table callbacks */
static void caption_table_activate_callback           (GtkWidget                   *widget,
						       gint                         entry,
						       gpointer                     callback_data);

static const char * pixmap_path_for_AuthnPromptKind (EazelProxy_AuthnPromptKind kind);

static const char * text_for_AuthnPromptKind (EazelProxy_AuthnPromptKind kind);


NAUTILUS_DEFINE_CLASS_BOILERPLATE (AmmoniteLoginDialog,
				   ammonite_login_dialog,
				   gnome_dialog_get_type ());


static void
ammonite_login_dialog_initialize_class (AmmoniteLoginDialogClass * klass)
{
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;
	
	object_class = GTK_OBJECT_CLASS(klass);
	widget_class = GTK_WIDGET_CLASS(klass);

	/* GtkObjectClass */
	object_class->destroy = ammonite_login_dialog_destroy;
}

static void
ammonite_login_dialog_initialize (AmmoniteLoginDialog *password_dialog)
{
	password_dialog->details = g_new (AmmoniteLoginDialogDetails, 1);

	password_dialog->details->table = NULL;
	password_dialog->details->remember_button = NULL;
	password_dialog->details->message = NULL;

	password_dialog->details->username_should_focus = FALSE;
}

/* GtkObjectClass methods */
static void
ammonite_login_dialog_destroy (GtkObject* object)
{
	AmmoniteLoginDialog *password_dialog;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (object));
	
	password_dialog = AMMONITE_LOGIN_DIALOG (object);

	if (password_dialog->details->message) {
		gtk_widget_destroy (GTK_WIDGET (password_dialog->details->message));
	}

	g_free (password_dialog->details);
}

/* GtkDialog callbacks */
static void
dialog_show_callback (GtkWidget *widget, gpointer callback_data)
{
	AmmoniteLoginDialog *password_dialog;

	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (callback_data));

	password_dialog = AMMONITE_LOGIN_DIALOG (callback_data);

	if (password_dialog->details->username_should_focus) {
		/* Move the focus to the username entry */
		nautilus_caption_table_entry_grab_focus (NAUTILUS_CAPTION_TABLE (password_dialog->details->table), 
							 CAPTION_TABLE_USERNAME_ROW);
	} else {
		/* Move the focus to the password entry */
		nautilus_caption_table_entry_grab_focus (NAUTILUS_CAPTION_TABLE (password_dialog->details->table), 
							 CAPTION_TABLE_PASSWORD_ROW);
	}
}

static void
dialog_close_callback (GtkWidget *widget, gpointer callback_data)
{
	AmmoniteLoginDialog *password_dialog;

	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (callback_data));

	password_dialog = AMMONITE_LOGIN_DIALOG (callback_data);

	gtk_widget_hide (widget);
}

/* Caption table callbacks */
static void
caption_table_activate_callback (GtkWidget *widget, gint entry, gpointer callback_data)
{
	AmmoniteLoginDialog *password_dialog;

	g_return_if_fail (callback_data != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (callback_data));

	password_dialog = AMMONITE_LOGIN_DIALOG (callback_data);

	/* If the username entry was activated, simply advance the focus to the password entry */
	if (entry == CAPTION_TABLE_USERNAME_ROW) {
		nautilus_caption_table_entry_grab_focus (NAUTILUS_CAPTION_TABLE (password_dialog->details->table), 
							 CAPTION_TABLE_PASSWORD_ROW);
	}
	/* If the password entry was activated, then simulate and OK button press to continue to hide process */
	else if (entry == CAPTION_TABLE_PASSWORD_ROW) {
		GtkWidget *button;
		
		button = g_list_nth_data (GNOME_DIALOG (password_dialog)->buttons, 
					button_mappings[BUTTON_OK][password_dialog->details->button_mode]);
		
		g_assert (button != NULL);
		g_assert (GTK_IS_BUTTON (button));

		gtk_button_clicked (GTK_BUTTON (button));
	}
}

/* Public AmmoniteLoginDialog methods */

/* I truely appologize for the disaster that follows */
/* This is so hacked up its embarassing */

GtkWidget*
ammonite_login_dialog_new	(EazelProxy_AuthnPromptKind prompt_kind,
				     const char             *username,
				     const char             *password,
				     gboolean                readonly_username)
{

	AmmoniteLoginDialog *password_dialog;
	GtkWidget *hbox_top;
	GtkWidget *hbox_middle;
	GtkLabel *secondary_message = NULL;
	GtkWidget *pixmap;

	password_dialog = gtk_type_new (ammonite_login_dialog_get_type ());


	if (prompt_kind == EazelProxy_InitialFail) {
		gnome_dialog_constructv (GNOME_DIALOG (password_dialog), "", fail_buttons);	
		password_dialog->details->button_mode = BUTTON_MODE_FAIL;
	} else if (prompt_kind == EazelProxy_Initial) {
		gnome_dialog_constructv (GNOME_DIALOG (password_dialog), "", register_buttons);
		password_dialog->details->button_mode = BUTTON_MODE_REGISTER;
	} else /* prompt_kind == EazelProxy_Retry (none of the other args are valid)*/ {
		gnome_dialog_constructv (GNOME_DIALOG (password_dialog), "", forgot_pw_buttons);
		password_dialog->details->button_mode = BUTTON_MODE_FORGOTPW;
	}

	gnome_dialog_set_default (GNOME_DIALOG (password_dialog), 
		button_mappings [BUTTON_OK][password_dialog->details->button_mode] );
	
	/* Setup the dialog */
	gtk_window_set_policy (GTK_WINDOW (password_dialog), 
			      FALSE,	/* allow_shrink */
			      TRUE,	/* allow_grow */
			      FALSE);	/* auto_shrink */

 	gtk_window_set_position (GTK_WINDOW (password_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal (GTK_WINDOW (password_dialog), TRUE);

 	gtk_container_set_border_width (GTK_CONTAINER (password_dialog), DIALOG_BORDER_WIDTH);


	/* Dont close the dialog on click.  We'll mange the destruction our selves */
	gnome_dialog_set_close (GNOME_DIALOG (password_dialog), FALSE);

	/* Make the close operation 'just_hide' the dialog - not nuke it */
	gnome_dialog_close_hides (GNOME_DIALOG (password_dialog), TRUE);
	
	gtk_signal_connect_while_alive (GTK_OBJECT (password_dialog),
					"show",
					GTK_SIGNAL_FUNC (dialog_show_callback),
					(gpointer) password_dialog,
					GTK_OBJECT (password_dialog));
	
	gtk_signal_connect_while_alive (GTK_OBJECT (password_dialog),
					"close",
					GTK_SIGNAL_FUNC (dialog_close_callback),
					(gpointer) password_dialog,
					GTK_OBJECT (password_dialog));

	if (prompt_kind != EazelProxy_InitialFail) {
		/* yes, this type of thing should be subclassed */
		/* 
		 * The table that holds the captions
		 */
		password_dialog->details->table = nautilus_caption_table_new (2);
		
		gtk_signal_connect (GTK_OBJECT (password_dialog->details->table),
				   "activate",
				   GTK_SIGNAL_FUNC (caption_table_activate_callback),
				   (gpointer) password_dialog);

		nautilus_caption_table_set_row_info (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
						     CAPTION_TABLE_USERNAME_ROW,
						     "User Name:",
						     "",
						     TRUE,
						     TRUE);

		nautilus_caption_table_set_row_info (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
						     CAPTION_TABLE_PASSWORD_ROW,
						     "Password:",
						     "",
						     FALSE,
						     FALSE);



		/* Configure the table */
	 	gtk_container_set_border_width (GTK_CONTAINER(password_dialog->details->table), CAPTION_TABLE_BORDER_WIDTH);
		
		/* if a username was specified, the focus should start on the password field */
		if (username && 0 < strlen (username) ) {
			password_dialog->details->username_should_focus = FALSE;
		} else {
			password_dialog->details->username_should_focus = TRUE;
		}	
		
		ammonite_login_dialog_set_username (password_dialog, username);
		ammonite_login_dialog_set_password (password_dialog, password);
		ammonite_login_dialog_set_readonly_username (password_dialog, readonly_username);
	} else {
		static const char *fail_message = 
			_(
			  "If you do not already have an Eazel Service account and "
			  "would like one, please click on the "
			  "\"Register for Eazel Services\" button below.\n\n"
			  "If you have an account and need help remembering your "
			  "password, please click the \"I forgot my password\" "
			  "button below. \n\n"
			  "Otherwise, click the \"OK\" button to cancel the current operation."); 

		secondary_message = GTK_LABEL (gtk_label_new (fail_message));

		gtk_label_set_justify (secondary_message, GTK_JUSTIFY_LEFT);
		gtk_label_set_line_wrap (secondary_message, TRUE);

	}




	/*
	 * The pixmap icon
	 */

	pixmap = gnome_pixmap_new_from_file ( pixmap_path_for_AuthnPromptKind (prompt_kind) );

	/*
	 * The main (bold face) message
	 */

	password_dialog->details->message =
		GTK_LABEL (gtk_label_new (text_for_AuthnPromptKind (prompt_kind)));
	gtk_label_set_justify (password_dialog->details->message, GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (password_dialog->details->message, TRUE);
	ammonite_gtk_label_make_bold (password_dialog->details->message);


	/*
	 * Configure layout
	 */

	/*
	 * Top hbox contains icon and message
	 */

	hbox_top = gtk_hbox_new (FALSE, 0);

	if (pixmap) {
		gtk_box_pack_start (GTK_BOX(hbox_top),  pixmap, FALSE, FALSE, 10);
	}

	gtk_box_pack_start (GTK_BOX(hbox_top), GTK_WIDGET (password_dialog->details->message), TRUE, TRUE, 10);

	/* Bogus label to get right-side padding */
	gtk_box_pack_start (GTK_BOX(hbox_top), GTK_WIDGET (gtk_label_new ("")), FALSE, FALSE, 20);


	/*
	 * Middle hbox containing table and spacer
	 */

	hbox_middle = gtk_hbox_new (FALSE, 5);

	if (password_dialog->details->table) {
		gtk_box_pack_start (GTK_BOX(hbox_middle), GTK_WIDGET (gtk_label_new ("")), FALSE, FALSE, 45);

		gtk_box_pack_start (GTK_BOX(hbox_middle), 
			password_dialog->details->table, 
			TRUE, TRUE, 10);
	} else {
		gtk_box_pack_start (GTK_BOX(hbox_middle), GTK_WIDGET (gtk_label_new ("")), FALSE, FALSE, 21);

		gtk_box_pack_start (GTK_BOX(hbox_middle), 
			GTK_WIDGET (secondary_message), 
			TRUE, TRUE, 10);
	}

	/*
	 * vbox contains: (from top to bottom) top hbox, table (right aligned) horiz rule, buttons
	 */

 	g_assert (GNOME_DIALOG (password_dialog)->vbox != NULL);

	gtk_box_set_spacing (GTK_BOX (GNOME_DIALOG (password_dialog)->vbox), 10);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (password_dialog)->vbox),
			    GTK_WIDGET (hbox_top),
			    TRUE,	/* expand */
			    TRUE,	/* fill */
			    5);		/* padding */


	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (password_dialog)->vbox),
			    hbox_middle,
			    TRUE,	/* expand */
			    TRUE,	/* fill */
			    0);		/* padding */

	gtk_widget_show_all (GNOME_DIALOG (password_dialog)->vbox);
	
	
	return GTK_WIDGET (password_dialog);

}


LoginDialogReturn
ammonite_login_dialog_run_and_block (AmmoniteLoginDialog *password_dialog)
{
	gint button_clicked;
	gint i;

	g_return_val_if_fail (password_dialog != NULL, FALSE);
	g_return_val_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog), FALSE);
	
	button_clicked = gnome_dialog_run_and_close (GNOME_DIALOG (password_dialog));

	for (i = 0 ; i < (sizeof (button_mappings) / sizeof (button_mappings[0])) ; i++ ) {
		if ( button_mappings[i][password_dialog->details->button_mode] == button_clicked ) {
			break;
		}
	}

	if (i == sizeof (button_mappings)) {
		g_warning ("Error: misconfigured ammonite dialog\n");
		i = -1;
	}
	
	return (LoginDialogReturn) i;
}

void
ammonite_login_dialog_set_username (AmmoniteLoginDialog	*password_dialog,
				       const char		*username)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog));

	if (password_dialog->details->table) {
		nautilus_caption_table_set_entry_text (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
						       CAPTION_TABLE_USERNAME_ROW,
						       username);
	}
}

void
ammonite_login_dialog_set_password (AmmoniteLoginDialog	*password_dialog,
				       const char		*password)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog));
	
	if (password_dialog->details->table) {
		nautilus_caption_table_set_entry_text (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
						       CAPTION_TABLE_PASSWORD_ROW,
						       password);
	}
}

void
ammonite_login_dialog_set_readonly_username (AmmoniteLoginDialog	*password_dialog,
						gboolean		readonly)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog));
	
	if (password_dialog->details->table) {
		nautilus_caption_table_set_entry_readonly (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
							   CAPTION_TABLE_USERNAME_ROW,
							   readonly);
	}
}

char *
ammonite_login_dialog_get_username (AmmoniteLoginDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, NULL);
	g_return_val_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog), NULL);

	if (password_dialog->details->table) {
		return nautilus_caption_table_get_entry_text (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
							      CAPTION_TABLE_USERNAME_ROW);
	} else {
		return NULL;
	}
}

char *
ammonite_login_dialog_get_password (AmmoniteLoginDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, NULL);
	g_return_val_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog), NULL);

	if (password_dialog->details->table) {
		return nautilus_caption_table_get_entry_text (NAUTILUS_CAPTION_TABLE (password_dialog->details->table),
							      CAPTION_TABLE_PASSWORD_ROW);
	} else {
		return NULL;
	}
}

gboolean
ammonite_login_dialog_get_remember (AmmoniteLoginDialog *password_dialog)
{
	g_return_val_if_fail (password_dialog != NULL, FALSE);
	g_return_val_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog), FALSE);

	return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (password_dialog->details->remember_button));
}

void
ammonite_login_dialog_set_remember (AmmoniteLoginDialog *password_dialog,
				       gboolean                remember)
{
	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (password_dialog->details->remember_button),
				      remember);
}

void
ammonite_login_dialog_set_remember_label_text (AmmoniteLoginDialog *password_dialog,
						  const char             *remember_label_text)
{
	GtkWidget *label;

	g_return_if_fail (password_dialog != NULL);
	g_return_if_fail (NAUTILUS_IS_PASSWORD_DIALOG (password_dialog));

	label = GTK_BIN (password_dialog->details->remember_button)->child;

	g_assert (label != NULL);
	g_assert (GTK_IS_LABEL (label));

	gtk_label_set_text (GTK_LABEL (label), remember_label_text);
}

static const char *
pixmap_path_for_AuthnPromptKind (EazelProxy_AuthnPromptKind kind)
{
	const char * ret;

	switch (kind) {
		case EazelProxy_Initial:
			ret = DATADIR "/pixmaps/nautilus/big_services_icon.png";
			break;
		case EazelProxy_InitialRetry:
			ret = DATADIR "/pixmaps/nautilus/serv_dialog_alert.png";
			break;
		case EazelProxy_InitialFail:
			ret = DATADIR "/pixmaps/nautilus/serv_dialog_alert.png";
			break;
		default:
			ret = DATADIR "/pixmaps/nautilus/serv_dialog_alert.png";
	}

	return ret;
}

static const char *
text_for_AuthnPromptKind (EazelProxy_AuthnPromptKind kind)
{
	const char * ret;
	
	switch (kind) {
		case EazelProxy_Initial:
			ret = _("Before you can continue, you need to log in to your Eazel Service account:");
			break;
		case EazelProxy_InitialRetry:
			ret = _("Oops!  Your user name or password were not correct.  Please try again:");
			break;
		case EazelProxy_InitialFail:
			ret = _("We're sorry, but your name or password are still not recognized.");
			break;
		default:
			ret = "(unsupported prompt type)";
			break;
	}

	return ret;
}




