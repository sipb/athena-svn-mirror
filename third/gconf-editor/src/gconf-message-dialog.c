/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gconf-message-dialog.h"

#include <gconf/gconf-client.h>

#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktogglebutton.h>
#include <libintl.h>

#define _(x) gettext (x)

static GtkMessageDialogClass *parent_class = NULL;

gboolean
gconf_message_dialog_should_show (const char *gconf_key)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean value;

	full_key = g_strdup_printf ("/apps/gconf-editor/dialog-settings/%s",
				    gconf_key);
	
	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	value = gconf_client_get_bool (client, full_key, NULL);

	g_free (full_key);

	return !value;
}


static void
gconf_message_dialog_button_toggled (GtkWidget *widget, GConfMessageDialog *dialog)
{
	static GConfClient *client = NULL;
	gchar *full_key;
	gboolean value;
	
	g_assert (dialog->gconf_key != NULL);
	
	if (client == NULL) {
		client = gconf_client_get_default ();
	}
	
	value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

	full_key = g_strdup_printf ("/apps/gconf-editor/dialog-settings/%s",
				   dialog->gconf_key);

	gconf_client_set_bool (client, full_key, !value, NULL);
	g_free (full_key);
}

static void
gconf_message_dialog_finalize (GObject *object)
{
	GConfMessageDialog *dialog;

	dialog = GCONF_MESSAGE_DIALOG (object);

	g_free (dialog->gconf_key);
	
	(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
gconf_message_dialog_class_init (GConfMessageDialogClass *klass)
{
	GObjectClass *gobject_class;
	GtkObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	
	gobject_class = (GObjectClass *)klass;
	object_class = (GtkObjectClass *)klass;

	gobject_class->finalize = gconf_message_dialog_finalize;
}

static void
gconf_message_dialog_init (GConfMessageDialog *dialog)
{
}
	
GType
gconf_message_dialog_get_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (GConfMessageDialogClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) gconf_message_dialog_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (GConfMessageDialog),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gconf_message_dialog_init
		};

		object_type = g_type_register_static (GTK_TYPE_MESSAGE_DIALOG, "GConfMessageDialog", &object_info, 0);
	}

	return object_type;
}

GtkWidget *
gconf_message_dialog_new (GtkWindow     *parent,
			  GtkDialogFlags flags,
			  GtkMessageType type,
			  GtkButtonsType buttons,
			  const gchar   *gconf_key,
			  const gchar   *message_format,
			  ...)
{
	GtkWidget *widget;
	GtkDialog *dialog;
	GtkWidget *check_button;
	gchar* msg = NULL;
	va_list args;
	
	widget = GTK_WIDGET (g_object_new (GCONF_TYPE_MESSAGE_DIALOG,
					   "message_type", type,
					   "buttons", buttons, 0));

	dialog = GTK_DIALOG (widget);
	
	if (flags & GTK_DIALOG_NO_SEPARATOR) {
		flags &= ~GTK_DIALOG_NO_SEPARATOR;
	}

	if (message_format) {
		va_start (args, message_format);
		msg = g_strdup_vprintf(message_format, args);
		va_end (args);
		
		gtk_label_set_text (GTK_LABEL (GTK_MESSAGE_DIALOG (widget)->label),
				    msg);
		
		g_free (msg);
	}
	
	if (parent != NULL)
		gtk_window_set_transient_for (GTK_WINDOW (widget),
					      GTK_WINDOW (parent));
	
	if (flags & GTK_DIALOG_MODAL)
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	
	if (flags & GTK_DIALOG_DESTROY_WITH_PARENT)
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	
	if (flags & GTK_DIALOG_NO_SEPARATOR)
		gtk_dialog_set_has_separator (dialog, FALSE);

	if (gconf_key != NULL) {
		GCONF_MESSAGE_DIALOG (dialog)->gconf_key = g_strdup (gconf_key);
		check_button = gtk_check_button_new_with_mnemonic (_("_Show this dialog again"));
		g_signal_connect (check_button, "toggled",
				  G_CALLBACK (gconf_message_dialog_button_toggled), dialog);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), TRUE);
		gtk_widget_show (check_button);

		gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->vbox),
				    check_button,
				    FALSE, FALSE, 0);
	}
	
	return widget;
}
