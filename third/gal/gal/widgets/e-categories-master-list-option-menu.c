/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-categories-master-list-option-menu.c
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

#include <config.h>

#include "e-categories-master-list-option-menu.h"

#include <gal/util/e-i18n.h>
#include <gal/util/e-util.h>
#include <gal/widgets/e-unicode.h>
#include <gtk/gtksignal.h>

struct ECategoriesMasterListOptionMenuPriv {
	ECategoriesMasterList *ecml;

	char **categories;
	int ecml_changed_signal_id;
};

#define PARENT_TYPE (e_option_menu_get_type())

static GtkObjectClass *parent_class;

/* The arguments we take */
enum {
	ARG_0,
	ARG_ECML,
};

static void
ecmlom_ecml_changed (ECategoriesMasterList *ecml, ECategoriesMasterListOptionMenu *ecmlom)
{
	int count;
	int i;
	const char **strings1;
	char **strings;

	count = e_categories_master_list_count (ecml);
	strings1 = g_new (const char *, count + 2);
	strings = g_new (char *, count + 2);

	strings1[0] = "";

	for (i = 0; i < count; i++) {
		strings1[i + 1] = e_categories_master_list_nth (ecml, i);
	}
	strings1[count + 1] = NULL;

	g_strfreev (ecmlom->priv->categories);
	ecmlom->priv->categories = e_strdupv (strings1);

	strings[0] = g_strdup (_("All Categories"));
	/* Walk through the list and convert utf8 strings to locale */
	for (i = 0; i < count; i++) {
		/* We don't own the strings returned by e_categories_master_list_nth, so we
		   don't need to free them. */
		strings[i + 1] = e_utf8_to_gtk_string (GTK_WIDGET (ecmlom), strings1[i + 1]);
	}
	strings[count + 1] = NULL;

	e_option_menu_set_strings_from_array (E_OPTION_MENU (ecmlom), strings);
	g_strfreev (strings);
	g_free (strings1);
}

static void
ecmlom_add_ecml (ECategoriesMasterListOptionMenu *ecmlom,
		ECategoriesMasterList *ecml)
{
	if (ecmlom->priv->ecml)
		return;

	ecmlom->priv->ecml = ecml;
	if (ecml) {
		gtk_object_ref (GTK_OBJECT (ecml));
		ecmlom->priv->ecml_changed_signal_id =
			gtk_signal_connect (GTK_OBJECT (ecml), "changed",
					    GTK_SIGNAL_FUNC (ecmlom_ecml_changed), ecmlom);
		ecmlom_ecml_changed (ecml, ecmlom);
	}
}

static void
ecmlom_remove_ecml (ECategoriesMasterListOptionMenu *ecmlom)
{
	if (ecmlom->priv->ecml) {
		if (ecmlom->priv->ecml_changed_signal_id)
			gtk_signal_disconnect (GTK_OBJECT (ecmlom->priv->ecml),
					       ecmlom->priv->ecml_changed_signal_id);
		gtk_object_unref (GTK_OBJECT (ecmlom->priv->ecml));
	}
	ecmlom->priv->ecml = NULL;
	ecmlom->priv->ecml_changed_signal_id = 0;
}

static void
ecmlom_destroy (GtkObject *object)
{
	ECategoriesMasterListOptionMenu *ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (object);

	ecmlom_remove_ecml (ecmlom);
	g_strfreev (ecmlom->priv->categories);
	g_free (ecmlom->priv);
	ecmlom->priv = NULL;
	
	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
ecmlom_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListOptionMenu *ecmlom;

	ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (o);
	
	switch (arg_id){
	case ARG_ECML:
		ecmlom_remove_ecml (ecmlom);
		ecmlom_add_ecml (ecmlom, (ECategoriesMasterList *) GTK_VALUE_OBJECT (*arg));
		break;
	}
}

static void
ecmlom_get_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	ECategoriesMasterListOptionMenu *ecmlom;

	ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (o);

	switch (arg_id) {
	case ARG_ECML:
		GTK_VALUE_OBJECT (*arg) = (GtkObject *) ecmlom->priv->ecml;
		break;

	default:
		arg->type = GTK_TYPE_INVALID;
		break;
	}
}

static void
ecmlom_class_init (GtkObjectClass *object_class)
{
	parent_class   = gtk_type_class (PARENT_TYPE);

	object_class->destroy = ecmlom_destroy;
	object_class->set_arg = ecmlom_set_arg;
	object_class->get_arg = ecmlom_get_arg;

	gtk_object_add_arg_type ("ECategoriesMasterListOptionMenu::ecml", E_CATEGORIES_MASTER_LIST_TYPE,
				 GTK_ARG_READWRITE, ARG_ECML);
}

static void
ecmlom_init (ECategoriesMasterListOptionMenu *ecmlom)
{
	ecmlom->priv                         = g_new (ECategoriesMasterListOptionMenuPriv, 1);

	ecmlom->priv->ecml                   = NULL;
	ecmlom->priv->categories             = NULL;
	ecmlom->priv->ecml_changed_signal_id = 0;
}

E_MAKE_TYPE(e_categories_master_list_option_menu, "ECategoriesMasterListOptionMenu", ECategoriesMasterListOptionMenu, ecmlom_class_init, ecmlom_init, PARENT_TYPE);

/**
 * e_categories_master_list_option_menu_new:
 *
 * Creates a new ECategoriesMasterListOptionMenu object.
 *
 * Returns: The ECategoriesMasterListOptionMenu object.
 */
GtkWidget *
e_categories_master_list_option_menu_new (ECategoriesMasterList *ecml)
{
	ECategoriesMasterListOptionMenu *ecmlom = gtk_type_new (E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE);

	if (e_categories_master_list_option_menu_construct (ecmlom, ecml) == NULL){
		gtk_object_destroy (GTK_OBJECT (ecmlom));
		return NULL;
	}

	return GTK_WIDGET (ecmlom);
}

/**
 * e_categories_master_list_option_menu_construct: Constructs a given combo object.
 * @ecmlom: The combo to construct.
 * @ecml: The master list to use.
 * 
 * Construct the given combo.  Sets the ecml.
 * 
 * Return value: the given combo as a GtkWidget.
 **/
GtkWidget *
e_categories_master_list_option_menu_construct (ECategoriesMasterListOptionMenu *ecmlom,
						ECategoriesMasterList       *ecml)
{
	g_return_val_if_fail (ecmlom != NULL, NULL);
	g_return_val_if_fail (ecml != NULL, NULL);

	gtk_object_set (GTK_OBJECT (ecmlom),
			"ecml", ecml,
			NULL);

	return GTK_WIDGET (ecmlom);
}

char *
e_categories_master_list_option_menu_get_category (ECategoriesMasterListOptionMenu *ecmlom)
{
	int value;

	value = e_option_menu_get_value (E_OPTION_MENU (ecmlom));

	if (ecmlom->priv->categories && ecmlom->priv->categories[value])
		return ecmlom->priv->categories[value];
	else
		return "";
}
