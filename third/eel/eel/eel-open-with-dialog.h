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

#ifndef EEL_OPEN_WITH_DIALOG_H
#define EEL_OPEN_WITH_DIALOG_H

#include <gtk/gtkdialog.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#define EEL_TYPE_OPEN_WITH_DIALOG         (eel_open_with_dialog_get_type ())
#define EEL_OPEN_WITH_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEL_TYPE_OPEN_WITH_DIALOG, EelOpenWithDialog))
#define EEL_OPEN_WITH_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), EEL_TYPE_OPEN_WITH_DIALOG, EelOpenWithDialogClass))
#define EEL_IS_OPEN_WITH_DIALOG(obj)      (G_TYPE_INSTANCE_CHECK_TYPE ((obj), EEL_TYPE_OPEN_WITH_DIALOG)

typedef struct _EelOpenWithDialog        EelOpenWithDialog;
typedef struct _EelOpenWithDialogClass   EelOpenWithDialogClass;
typedef struct _EelOpenWithDialogDetails EelOpenWithDialogDetails;

struct _EelOpenWithDialog {
	GtkDialog parent;
	EelOpenWithDialogDetails *details;
};

struct _EelOpenWithDialogClass {
	GtkDialogClass parent_class;

	void (*application_selected) (EelOpenWithDialog *dialog,
				      GnomeVFSMimeApplication *application);
};

GType      eel_open_with_dialog_get_type (void);
GtkWidget* eel_open_with_dialog_new      (const char *uri,
					  const char *mime_type);
GtkWidget* eel_add_application_dialog_new (const char *uri,
					   const char *mime_type);



#endif /* EEL_OPEN_WITH_DIALOG_H */
