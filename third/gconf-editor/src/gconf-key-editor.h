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

#ifndef __GCONF_KEY_EDITOR_H__
#define __GCONF_KEY_EDITOR_H__

#include <gtk/gtkdialog.h>
#include <gtk/gtkliststore.h>
#include <gconf/gconf.h>

#define GCONF_TYPE_KEY_EDITOR		  (gconf_key_editor_get_type ())
#define GCONF_KEY_EDITOR(obj)		  (GTK_CHECK_CAST ((obj), GCONF_TYPE_KEY_EDITOR, GConfKeyEditor))
#define GCONF_KEY_EDITOR_CLASS(klass)	  (GTK_CHECK_CLASS_CAST ((klass), GCONF_TYPE_KEY_EDITOR, GConfKeyEditorClass))
#define GCONF_IS_KEY_EDITOR(obj)	  (GTK_CHECK_TYPE ((obj), GCONF_TYPE_KEY_EDITOR))
#define GCONF_IS_KEY_EDITOR_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((obj), GCONF_TYPE_KEY_EDITOR))
#define GCONF_KEY_EDITOR_GET_CLASS(obj)   (GTK_CHECK_GET_CLASS ((obj), GCONF_TYPE_KEY_EDITOR, GConfKeyEditorClass))

typedef enum {
	GCONF_KEY_EDITOR_NEW_KEY,
	GCONF_KEY_EDITOR_EDIT_KEY,
} GConfKeyEditorAction;

typedef struct _GConfKeyEditor GConfKeyEditor;
typedef struct _GConfKeyEditorClass GConfKeyEditorClass;

struct _GConfKeyEditor {
	GtkDialog parent_instance;
	
	int active_type;
	GtkWidget *option_menu;
	GtkWidget *notebook;
	GtkWidget *bool_widget;
	GtkWidget *int_widget;
	GtkWidget *string_widget;
	GtkWidget *float_widget;
	
	GtkWidget *path_label;
	GtkWidget *path_box;
	GtkWidget *name_entry;

	/* List editing stuff */
	GtkWidget *list_widget;
	GtkListStore *list_model;
	GtkWidget *list_type_menu;
	GtkWidget *remove_button, *go_up_button, *go_down_button;

};

struct _GConfKeyEditorClass {
	GtkDialogClass parent_class;
};

GType       gconf_key_editor_get_type (void);
GtkWidget  *gconf_key_editor_new      (GConfKeyEditorAction action);

void        gconf_key_editor_set_value (GConfKeyEditor *editor,
					GConfValue     *value);
GConfValue *gconf_key_editor_get_value (GConfKeyEditor *editor);

void  gconf_key_editor_set_key_path      (GConfKeyEditor *editor,
					  const char     *path);
char *gconf_key_editor_get_full_key_path (GConfKeyEditor *editor);

void                 gconf_key_editor_set_key_name (GConfKeyEditor *editor,
						    const char     *path);
G_CONST_RETURN char *gconf_key_editor_get_key_name (GConfKeyEditor *editor);


#endif /* __GCONF_KEY_EDITOR_H__ */
