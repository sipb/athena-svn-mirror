/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-optionmenu.c
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
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include "e-option-menu.h"
#include "gal/util/e-util.h"

static void e_option_menu_init       (EOptionMenu         *card);
static void e_option_menu_class_init (EOptionMenuClass    *klass);

static GtkOptionMenuClass *parent_class = NULL;

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

#define PARENT_TYPE (gtk_option_menu_get_type ())

GtkType
e_option_menu_get_type (void)
{
	static GtkType optionmenu_type = 0;

	if (!optionmenu_type)
	{
		static const GtkTypeInfo optionmenu_info =
		{
			"EOptionMenu",
			sizeof (EOptionMenu),
			sizeof (EOptionMenuClass),
			(GtkClassInitFunc) e_option_menu_class_init,
			(GtkObjectInitFunc) e_option_menu_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		optionmenu_type = gtk_type_unique (PARENT_TYPE, &optionmenu_info);
	}

	return optionmenu_type;
}

static void
e_option_menu_class_init (EOptionMenuClass *klass)
{
	GtkObjectClass *object_class;
	GtkOptionMenuClass *optionmenu_class;

	object_class     = (GtkObjectClass*) klass;
	optionmenu_class = (GtkOptionMenuClass *) klass;

	parent_class = gtk_type_class (PARENT_TYPE);

	klass->changed        = NULL;

	signals [CHANGED] =
		gtk_signal_new ("changed",
				GTK_RUN_LAST,
				E_OBJECT_CLASS_TYPE (object_class),
				GTK_SIGNAL_OFFSET (EOptionMenuClass, changed),
				gtk_marshal_NONE__INT,
				GTK_TYPE_NONE, 1, GTK_TYPE_INT);

	E_OBJECT_CLASS_ADD_SIGNALS (object_class, signals, LAST_SIGNAL);
}

static void
e_option_menu_init (EOptionMenu *optionmenu)
{
	optionmenu->value = 0;
}

GtkWidget *
e_option_menu_new (char *first_label, ...)
{
	va_list args;
	int i;
	char *s;
	char **labels;
	GtkWidget *widget;

	va_start (args, first_label);
	i = 0;
	for (s = first_label; s; s = va_arg (args, char *))
		i++;
	va_end (args);

	labels = g_new (char *, i + 1);

	va_start (args, first_label);
	i = 0;
	for (s = first_label; s; s = va_arg (args, char *))
		labels[i++] = s;
	va_end (args);
	labels[i] = NULL;

	widget = e_option_menu_new_from_array (labels);

	g_free (labels);

	return widget;
}

GtkWidget *
e_option_menu_new_from_array (char **labels)
{
	EOptionMenu *menu = gtk_type_new (e_option_menu_get_type());
	e_option_menu_construct_from_array (menu, labels);
	return (GtkWidget *) menu;
}

GtkWidget *
e_option_menu_construct_from_array (EOptionMenu *option_menu, char **labels)
{
	e_option_menu_set_strings_from_array (option_menu, labels);
	return (GtkWidget *) option_menu;
}

void
e_option_menu_set_strings (EOptionMenu *menu, char *first_label, ...)
{
	va_list args;
	int i;
	char *s;
	char **labels;

	va_start (args, first_label);
	i = 0;
	for (s = first_label; s; s = va_arg (args, char *))
		i++;
	va_end (args);

	labels = g_new (char *, i + 1);

	va_start (args, first_label);
	i = 0;
	for (s = first_label; s; s = va_arg (args, char *))
		labels[i++] = s;
	va_end (args);
	labels[i] = NULL;

	e_option_menu_set_strings_from_array (menu, labels);

	g_free (labels);
}

typedef struct {
	EOptionMenu *option_menu;
	int value;
} CallbackStruct;

static void
item_activated_cb (GtkWidget *widget, CallbackStruct *cb_struct)
{
	cb_struct->option_menu->value = cb_struct->value;
	gtk_signal_emit (GTK_OBJECT (cb_struct->option_menu), signals[CHANGED],
			 cb_struct->value);
}

static void
item_destroyed_cb (GtkWidget *widget, CallbackStruct *cb_struct)
{
	g_free (cb_struct);
}

void
e_option_menu_set_strings_from_array (EOptionMenu *option_menu, char **labels)
{
	GtkWidget *menu;
	int i;

	menu = gtk_menu_new ();
	if (labels) {
		for (i = 0; labels[i]; i++) {
			GtkWidget *menu_item;
			CallbackStruct *cb_struct;

			if (labels[i]) {
				menu_item = gtk_menu_item_new_with_label (labels[i]);

				cb_struct = g_new (CallbackStruct, 1);
				cb_struct->option_menu = option_menu;
				cb_struct->value = i;

				gtk_signal_connect (GTK_OBJECT (menu_item),
						    "activate",
						    GTK_SIGNAL_FUNC (item_activated_cb),
						    cb_struct);
				gtk_signal_connect (GTK_OBJECT (menu_item),
						    "destroy",
						    GTK_SIGNAL_FUNC (item_destroyed_cb),
						    cb_struct);
			} else {
				menu_item = gtk_menu_item_new ();
				gtk_widget_set_sensitive (menu_item, FALSE);
			}


			gtk_widget_show (menu_item);
			gtk_menu_append (GTK_MENU (menu), menu_item);
		}
	}
	gtk_option_menu_remove_menu (GTK_OPTION_MENU (option_menu));
	gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
}

gint
e_option_menu_get_value (EOptionMenu *menu)
{
	return menu->value;
}
