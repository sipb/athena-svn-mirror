/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-file-selection.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <gtk/gtksignal.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkclist.h>
#include <gdk/gdkkeysyms.h>
#include "e-file-selection.h"
#include "gal/util/e-util.h"
#include "gal/util/e-i18n.h"
#include <string.h>

#define PARENT_TYPE (gtk_file_selection_get_type ())
static GtkFileSelectionClass *parent_class = NULL;

#if 0
enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };
#endif

enum {
	PROP_0,
	PROP_MULTIPLE
};

struct _EFileSelectionPrivate {
	guint multiple : 1;
	guint in_selection_changed : 1;
	guint in_entry_changed : 1;
	GtkWidget *selection_entry;
};

static void
efs_get_property (GObject *object,
		  guint prop_id,
		  GValue *value,
		  GParamSpec *pspec)
{
	EFileSelection *selection = E_FILE_SELECTION (object);

	switch (prop_id){
	case PROP_MULTIPLE:
		g_value_set_boolean (value, selection->priv->multiple);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
efs_set_property (GObject *object,
		  guint prop_id,
		  const GValue *value,
		  GParamSpec *pspec)
{
	EFileSelection *selection = E_FILE_SELECTION (object);
	GtkWidget *file_list;

	switch (prop_id){
	case PROP_MULTIPLE:
		selection->priv->multiple = g_value_get_boolean (value);
		file_list = GTK_FILE_SELECTION (selection)->file_list;
		gtk_clist_set_selection_mode (GTK_CLIST (file_list),
					      selection->priv->multiple ?
					      GTK_SELECTION_EXTENDED :
					      GTK_SELECTION_SINGLE);
		break;
	default:
		break;
	}
}

static void
e_file_selection_class_init (EFileSelectionClass *klass)
{
	GObjectClass *object_class;

	object_class     = (GObjectClass*) klass;

	object_class->get_property = efs_get_property;
	object_class->set_property = efs_set_property;

	parent_class = g_type_class_ref (PARENT_TYPE);

	g_object_class_install_property (object_class, PROP_MULTIPLE,
					 g_param_spec_boolean ("multiple",
							       _( "Multiple" ),
							       _( "Multiple" ),
							       FALSE,
							       G_PARAM_READWRITE));
#if 0
	klass->changed        = NULL;

	signals [CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (EFileSelectionClass, changed),
			      NULL, NULL,
			      e_marshal_NONE__INT,
			      G_TYPE_NONE, 1, G_TYPE_INT);
#endif
}

static gint
e_file_selection_entry_key_press (GtkWidget   *widget,
				  GdkEventKey *event,
				  gpointer     user_data)
{
	EFileSelection *efs;
	char *text;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (event->keyval == GDK_Tab) {
		int selection_index;
		efs = E_FILE_SELECTION (user_data);
		text = (char *) gtk_entry_get_text (GTK_ENTRY (efs->priv->selection_entry));

		text = g_strdup (text);

		gtk_file_selection_complete (GTK_FILE_SELECTION (efs), text);

		g_free (text);

		text = (char *) gtk_entry_get_text (GTK_ENTRY (GTK_FILE_SELECTION (efs)->selection_entry));
		gtk_entry_set_text (GTK_ENTRY (efs->priv->selection_entry), text);

		selection_index = gtk_editable_get_position (GTK_EDITABLE (GTK_FILE_SELECTION (efs)->selection_entry));
		gtk_editable_set_position (GTK_EDITABLE (efs->priv->selection_entry), selection_index);

		g_signal_stop_emission_by_name (widget, "key_press_event");

		return TRUE;
	}

	return FALSE;
}

static void
e_file_selection_entry_changed (GtkWidget   *widget,
				EFileSelection *efs)
{
	if (efs->priv->in_entry_changed || efs->priv->in_selection_changed)
		return;

	efs->priv->in_entry_changed = TRUE;

	gtk_clist_unselect_all (GTK_CLIST (GTK_FILE_SELECTION(efs)->file_list));

	efs->priv->in_entry_changed = FALSE;
}

static void
selection_changed (EFileSelection *efs)
{
	char **strings;
	GtkCList *file_list;

	if (efs->priv->in_entry_changed || efs->priv->in_selection_changed)
		return;

	efs->priv->in_selection_changed = TRUE;

	file_list = GTK_CLIST (GTK_FILE_SELECTION (efs)->file_list);

	if (file_list->selection) {
		char *text;
		int i;
		int count = 0;
		GList *node;

		for (node = file_list->selection; node; node = node->next) {
			count ++;
		}

		strings = g_new (char *, count + 1);

		for (node = file_list->selection, i = 0; node; node = node->next, i++) {
			gtk_clist_get_text (file_list, GPOINTER_TO_INT (node->data), 0, &text);
			strings[i] = text;
		}

		strings[count] = NULL;

		text = g_strjoinv (", ", strings);
		gtk_entry_set_text (GTK_ENTRY (efs->priv->selection_entry), text);
		g_free (text);
		g_free (strings);

	} else {
		const char *text = gtk_entry_get_text (GTK_ENTRY (GTK_FILE_SELECTION (efs)->selection_entry));
		gtk_entry_set_text (GTK_ENTRY (efs->priv->selection_entry), text);
	}

	efs->priv->in_selection_changed = FALSE;
}

static void
row_changed (GtkCList *clist, gint row, gint column, GdkEvent *event, EFileSelection *efs)
{
	selection_changed (efs);
}

static void
e_file_selection_init (EFileSelection *file_selection)
{
	GtkWidget *widget;
	GtkWidget *parent;
	GtkWidget *entry;
	GtkWidget *file_list;

	file_selection->priv = g_new (EFileSelectionPrivate, 1);

	file_selection->priv->multiple = FALSE;
	file_selection->priv->in_selection_changed = FALSE;
	file_selection->priv->selection_entry = entry = gtk_entry_new();

	g_signal_connect (entry, "key_press_event",
			  G_CALLBACK (e_file_selection_entry_key_press), file_selection);
	g_signal_connect (entry, "changed",
			  G_CALLBACK (e_file_selection_entry_changed), file_selection);
	gtk_signal_connect_object (GTK_OBJECT (entry), "focus_in_event",
				   (GtkSignalFunc) gtk_widget_grab_default,
				   GTK_OBJECT (GTK_FILE_SELECTION(file_selection)->ok_button));
	gtk_signal_connect_object (GTK_OBJECT (entry), "activate",
				   (GtkSignalFunc) gtk_button_clicked,
				   GTK_OBJECT (GTK_FILE_SELECTION(file_selection)->ok_button));

	widget = GTK_FILE_SELECTION (file_selection)->selection_entry;
	parent = widget->parent;

	if (!parent)
		return;

	gtk_widget_hide (widget);
	gtk_box_pack_start (GTK_BOX (parent), file_selection->priv->selection_entry, TRUE, TRUE, 0);
	gtk_widget_show (file_selection->priv->selection_entry);

	file_list = GTK_FILE_SELECTION (file_selection)->file_list;

	g_signal_connect (file_list, "select_row",
			  G_CALLBACK (row_changed), file_selection);
	g_signal_connect (file_list, "unselect_row",
			  G_CALLBACK (row_changed), file_selection);
}

E_MAKE_TYPE (e_file_selection,
	     "EFileSelection",
	     EFileSelection,
	     e_file_selection_class_init,
	     e_file_selection_init,
	     PARENT_TYPE)

GtkWidget *
e_file_selection_new (char *title)
{
	EFileSelection *selection = g_object_new (E_FILE_SELECTION_TYPE, NULL);
	gtk_window_set_title (GTK_WINDOW (selection), title);
	return GTK_WIDGET (selection);
}

char **
e_file_selection_get_filenames (EFileSelection *efs)
{
	char **ret_val;
	GtkCList *file_list = GTK_CLIST (GTK_FILE_SELECTION (efs)->file_list);

	if (file_list->selection) {
		char *text;
		char *filesel_path;
		int i;
		GList *sel_list = NULL, *node;
		int count = 0;

		node = file_list->selection;
		while (node) {
			sel_list = g_list_prepend (sel_list, node->data);
			count ++;
			node = node->next;
		}

		filesel_path = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (efs)));
		text = strrchr (filesel_path, '/');
		if (text) text[1] = '\0';   /* one past the / */

		ret_val = g_new (char *, count + 1);

		for (node = sel_list, i = count - 1; node; node = node->next, i--) {
			gtk_clist_get_text (file_list, GPOINTER_TO_INT (node->data), 0, &text);
			ret_val[i] = (filesel_path && *text != '/') ? g_strconcat (filesel_path, text, NULL) : g_strdup (text);
		}
		ret_val[count] = NULL;
		g_free (filesel_path);
	} else {
		char *text, *filesel_path;

		filesel_path = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (efs)));
		text = strrchr (filesel_path, '/');
		if (text) text[1] = '\0';   /* one past the / */

		text = (char *) gtk_entry_get_text (GTK_ENTRY (efs->priv->selection_entry));

		ret_val = g_new (char *, 2);
		ret_val[0] = (filesel_path && *text != '/') ? g_strconcat (filesel_path, text, NULL) : g_strdup (text);
		ret_val[1] = NULL;

		g_free (filesel_path);
	}

	return ret_val;
}
