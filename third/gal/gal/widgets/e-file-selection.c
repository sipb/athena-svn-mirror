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
#include <string.h>

static GtkFileSelectionClass *parent_class = NULL;

#if 0
enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };
#endif

#define PARENT_TYPE (gtk_file_selection_get_type ())

enum {
	ARG_0,
	ARG_MULTIPLE
};

struct _EFileSelectionPrivate {
	guint multiple : 1;
	guint in_selection_changed : 1;
	guint in_entry_changed : 1;
	GtkWidget *selection_entry;
};

static void
efs_get_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	EFileSelection *selection = E_FILE_SELECTION (o);

	switch (arg_id){
	case ARG_MULTIPLE:
		GTK_VALUE_BOOL (*arg) = selection->priv->multiple;
		break;
	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
efs_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	EFileSelection *selection = E_FILE_SELECTION (o);
	GtkWidget *file_list;

	switch (arg_id){
	case ARG_MULTIPLE:
		selection->priv->multiple = GTK_VALUE_BOOL (*arg);
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
	GtkObjectClass *object_class;

	object_class     = (GtkObjectClass*) klass;

	object_class->get_arg = efs_get_arg;
	object_class->set_arg = efs_set_arg;

	parent_class = gtk_type_class (PARENT_TYPE);

	gtk_object_add_arg_type ("EFileSelection::multiple", GTK_TYPE_BOOL,
				 GTK_ARG_READWRITE, ARG_MULTIPLE);

#if 0
	klass->changed        = NULL;

	signals [CHANGED] =
		gtk_signal_new ("changed",
				GTK_RUN_LAST,
				E_OBJECT_CLASS_TYPE (object_class),
				GTK_SIGNAL_OFFSET (EFileSelectionClass, changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1, GTK_TYPE_INT);

	E_OBJECT_CLASS_ADD_SIGNALS (object_class, signals, LAST_SIGNAL);
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
		text = gtk_entry_get_text (GTK_ENTRY (efs->priv->selection_entry));

		text = g_strdup (text);

		gtk_file_selection_complete (GTK_FILE_SELECTION (efs), text);

		g_free (text);

		text = gtk_entry_get_text (GTK_ENTRY (GTK_FILE_SELECTION (efs)->selection_entry));
		gtk_entry_set_text (GTK_ENTRY (efs->priv->selection_entry), text);

		selection_index = gtk_editable_get_position (GTK_EDITABLE (GTK_FILE_SELECTION (efs)->selection_entry));
		gtk_editable_set_position (GTK_EDITABLE (efs->priv->selection_entry), selection_index);

		gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");

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

	gtk_signal_connect (GTK_OBJECT (entry), "key_press_event",
			    GTK_SIGNAL_FUNC (e_file_selection_entry_key_press), file_selection);
	gtk_signal_connect (GTK_OBJECT (entry), "changed",
			    GTK_SIGNAL_FUNC (e_file_selection_entry_changed), file_selection);
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

	gtk_signal_connect (GTK_OBJECT (file_list), "select_row",
			    GTK_SIGNAL_FUNC (row_changed), file_selection);
	gtk_signal_connect (GTK_OBJECT (file_list), "unselect_row",
			    GTK_SIGNAL_FUNC (row_changed), file_selection);
}

GtkType
e_file_selection_get_type (void)
{
	static GtkType type = 0;

	if (!type)
	{
		static const GtkTypeInfo info =
		{
			"EFileSelection",
			sizeof (EFileSelection),
			sizeof (EFileSelectionClass),
			(GtkClassInitFunc) e_file_selection_class_init,
			(GtkObjectInitFunc) e_file_selection_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

GtkWidget *
e_file_selection_new (char *title)
{
	EFileSelection *selection = gtk_type_new (e_file_selection_get_type());
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

		text = gtk_entry_get_text (GTK_ENTRY (efs->priv->selection_entry));

		ret_val = g_new (char *, 2);
		ret_val[0] = (filesel_path && *text != '/') ? g_strconcat (filesel_path, text, NULL) : g_strdup (text);
		ret_val[1] = NULL;

		g_free (filesel_path);
	}

	return ret_val;
}
