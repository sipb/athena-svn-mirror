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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GCONF_MESSAGE_DIALOG_H__
#define __GCONF_MESSAGE_DIALOG_H__

#include <gtk/gtkmessagedialog.h>

#define GCONF_TYPE_MESSAGE_DIALOG		  (gconf_message_dialog_get_type ())
#define GCONF_MESSAGE_DIALOG(obj)		  (GTK_CHECK_CAST ((obj), GCONF_TYPE_MESSAGE_DIALOG, GConfMessageDialog))
#define GCONF_MESSAGE_DIALOG_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GCONF_TYPE_MESSAGE_DIALOG, GConfMessageDialogClass))
#define GCONF_IS_MESSAGE_DIALOG(obj)	  (GTK_CHECK_TYPE ((obj), GCONF_TYPE_MESSAGE_DIALOG))
#define GCONF_IS_MESSAGE_DIALOG_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_MESSAGE_DIALOG))
#define GCONF_MESSAGE_DIALOG_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GCONF_TYPE_MESSAGE_DIALOG, GConfMessageDialogClass))

typedef struct _GConfMessageDialog GConfMessageDialog;
typedef struct _GConfMessageDialogClass GConfMessageDialogClass;

struct _GConfMessageDialog {
	GtkMessageDialog parent_instance;
	gchar *gconf_key;
};

struct _GConfMessageDialogClass {
	GtkMessageDialogClass parent_class;
};

GType gconf_message_dialog_get_type (void);

gboolean gconf_message_dialog_should_show (const char *gconf_key);

GtkWidget *gconf_message_dialog_new (GtkWindow     *parent,
				     GtkDialogFlags flags,
				     GtkMessageType type,
				     GtkButtonsType buttons,
				     const gchar   *gconf_key,
				     const gchar   *message_format,
				     ...) G_GNUC_PRINTF (6, 7);

#endif /* __GCONF_MESSAGE_DIALOG_H__ */
