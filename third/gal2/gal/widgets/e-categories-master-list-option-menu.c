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
	PROP_0,
	PROP_ECML,
};

static void
ecmlom_ecml_changed (ECategoriesMasterList *ecml, ECategoriesMasterListOptionMenu *ecmlom)
{
	int count;
	int i;
	const char **strings;

	count = e_categories_master_list_count (ecml);
	strings = g_new (const char *, count + 2);

	strings[0] = "";
	for (i = 0; i < count; i++) {
		strings[i + 1] = e_categories_master_list_nth (ecml, i);
	}
	strings[count + 1] = NULL;

	g_strfreev (ecmlom->priv->categories);
	ecmlom->priv->categories = e_strdupv (strings);

	strings[0] = _("All Categories");
	e_option_menu_set_strings_from_array (E_OPTION_MENU (ecmlom), strings);

	g_free (strings);
}

static void
ecmlom_add_ecml (ECategoriesMasterListOptionMenu *ecmlom,
		ECategoriesMasterList *ecml)
{
	if (ecmlom->priv->ecml)
		return;

	ecmlom->priv->ecml = ecml;
	if (ecml) {
		g_object_ref (ecml);
		ecmlom->priv->ecml_changed_signal_id =
			g_signal_connect (ecml, "changed",
					  G_CALLBACK (ecmlom_ecml_changed), ecmlom);
		ecmlom_ecml_changed (ecml, ecmlom);
	}
}

static void
ecmlom_remove_ecml (ECategoriesMasterListOptionMenu *ecmlom)
{
	if (ecmlom->priv->ecml) {
		if (ecmlom->priv->ecml_changed_signal_id) {
			g_signal_handler_disconnect (ecmlom->priv->ecml,
						     ecmlom->priv->ecml_changed_signal_id);
			ecmlom->priv->ecml_changed_signal_id = 0;
		}

		g_object_unref (ecmlom->priv->ecml);
	}
	ecmlom->priv->ecml = NULL;
}

static void
ecmlom_dispose (GObject *object)
{
	ECategoriesMasterListOptionMenu *ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (object);

	if (ecmlom->priv) {
		ecmlom_remove_ecml (ecmlom);
		if (ecmlom->priv->categories) {
			g_strfreev (ecmlom->priv->categories);
			ecmlom->priv->categories = NULL;
		}
		g_free (ecmlom->priv);
		ecmlom->priv = NULL;
	}
	
	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
ecmlom_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	ECategoriesMasterListOptionMenu *ecmlom;

	ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (object);
	
	switch (prop_id){
	case PROP_ECML:
		ecmlom_remove_ecml (ecmlom);
		ecmlom_add_ecml (ecmlom, (ECategoriesMasterList *) g_value_get_object (value));
		break;
	}
}

static void
ecmlom_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	ECategoriesMasterListOptionMenu *ecmlom;

	ecmlom = E_CATEGORIES_MASTER_LIST_OPTION_MENU (object);

	switch (prop_id) {
	case PROP_ECML:
		g_value_set_object (value, ecmlom->priv->ecml);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
ecmlom_class_init (GObjectClass *object_class)
{
	parent_class   = g_type_class_ref (PARENT_TYPE);

	object_class->dispose = ecmlom_dispose;
	object_class->set_property = ecmlom_set_property;
	object_class->get_property = ecmlom_get_property;

	g_object_class_install_property (object_class, PROP_ECML,
					 g_param_spec_object ("ecml",
							      _( "ECML" ),
							      _( "ECategoriesMasterListCombo" ),
							      E_CATEGORIES_MASTER_LIST_TYPE,
							      G_PARAM_READWRITE));
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
	ECategoriesMasterListOptionMenu *ecmlom = g_object_new (E_CATEGORIES_MASTER_LIST_OPTION_MENU_TYPE, NULL);

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

	g_object_set (ecmlom,
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
