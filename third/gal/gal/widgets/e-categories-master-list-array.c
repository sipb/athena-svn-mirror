/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-categories-master-list-array.c
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

#include "e-categories-master-list-array.h"

#include <gal/util/e-i18n.h>
#include <gal/util/e-xml-utils.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <gnome-xml/xmlmemory.h>
#include "e-unicode.h"
#include <string.h>

#define PARENT_TYPE e_categories_master_list_get_type ()

#define d(x)

typedef enum {
	E_CATEGORIES_MASTER_LIST_ARRAY_NORMAL,
	E_CATEGORIES_MASTER_LIST_ARRAY_DELETED,
} ECategoriesMasterListArrayState;

typedef struct {
	char *category;
	char *icon;
	char *color;
} ECategoriesMasterListArrayItem;

struct _ECategoriesMasterListArrayPriv {
	ECategoriesMasterListArrayItem  **categories;
	int                               count;

	ECategoriesMasterListArrayState  *states;
	ECategoriesMasterListArrayItem  **appends;
	int                               appends_count;
};

static ECategoriesMasterListClass *parent_class;

struct {
	gchar *category;
	gchar *icon;
} builtin_categories[] = {
	{ N_("Birthday"),         GAL_IMAGESDIR "/category_birthday_16.png" },
	{ N_("Business"),         GAL_IMAGESDIR "/category_business_16.png" },
	{ N_("Competition"),      NULL },
	{ N_("Favorites"),        GAL_IMAGESDIR "/category_favorites_16.png" },
	{ N_("Gifts"),            GAL_IMAGESDIR "/category_gifts_16.png" },
	{ N_("Goals/Objectives"), GAL_IMAGESDIR "/category_goals_16.png" },
	{ N_("Holiday"),          GAL_IMAGESDIR "/category_holiday_16.png" },
	{ N_("Holiday Cards"),    GAL_IMAGESDIR "/category_holiday-cards_16.png" },
	{ N_("Hot Contacts"),     GAL_IMAGESDIR "/category_hot-contacts_16.png" },
	{ N_("Ideas"),            GAL_IMAGESDIR "/category_ideas_16.png" },
	{ N_("International"),    GAL_IMAGESDIR "/category_international_16.png" },
	{ N_("Key Customer"),     GAL_IMAGESDIR "/category_key-customer_16.png" },
	{ N_("Miscellaneous"),    GAL_IMAGESDIR "/category_miscellaneous_16.png" },
	{ N_("Personal"),         GAL_IMAGESDIR "/category_personal_16.png" },
	{ N_("Phone Calls"),      GAL_IMAGESDIR "/category_phonecalls_16.png" },
	{ N_("Status"),           GAL_IMAGESDIR "/category_status_16.png" },
	{ N_("Strategies"),       GAL_IMAGESDIR "/category_strategies_16.png" },
	{ N_("Suppliers"),        GAL_IMAGESDIR "/category_suppliers_16.png" },
	{ N_("Time & Expenses"),  GAL_IMAGESDIR "/category_time-and-expenses_16.png" },
	{ N_("VIP"),              NULL },
	{ N_("Waiting"),          NULL },
};

#define BUILTIN_CATEGORY_COUNT (sizeof(builtin_categories) / sizeof(builtin_categories[0]))

static void
ecmlai_free (ECategoriesMasterListArrayItem *ecmlai)
{
	g_free (ecmlai->category);
	g_free (ecmlai->icon);
	g_free (ecmlai->color);
	g_free (ecmlai);
}

static ECategoriesMasterListArrayItem *
ecmlai_new (const char *category,
	    const char *icon,
	    const char *color)
{
	ECategoriesMasterListArrayItem *ret_val;
	ret_val = g_new (ECategoriesMasterListArrayItem, 1);
	ret_val->category = g_strdup (category);
	ret_val->icon = g_strdup (icon);
	ret_val->color = g_strdup (color);
	return ret_val;
}

static void
ecmla_free (ECategoriesMasterListArray *ecmla)
{
	int i;

	g_free (ecmla->priv->states);
	ecmla->priv->states = NULL;

	for (i = 0; i < ecmla->priv->appends_count; i++)
		ecmlai_free (ecmla->priv->appends[i]);
	g_free (ecmla->priv->appends);
	ecmla->priv->appends = NULL;
	ecmla->priv->appends_count = 0;

	for (i = 0; i < ecmla->priv->count; i++)
		ecmlai_free (ecmla->priv->categories[i]);
	g_free (ecmla->priv->categories);
	ecmla->priv->categories = NULL;
	ecmla->priv->count = 0;
}

/**
 * ecmla_count:
 * @ecmla: The e-categories-master-list to operate on
 *
 * Returns: the number of categories in the list.
 */
static int
ecmla_count (ECategoriesMasterList *ecml)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	return ecmla->priv->count;
}

/**
 * ecmla_nth:
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the nth category in the list.
 */
static const char *
ecmla_nth (ECategoriesMasterList *ecml, int n)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	g_return_val_if_fail (n < ecmla->priv->count, NULL);
	g_return_val_if_fail (n >= 0, NULL);

	return ecmla->priv->categories[n]->category;
}

/**
 * ecmla_nth_icon
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the pixmap file associated
 * with the nth category in the list.
 */
static const char *
ecmla_nth_icon (ECategoriesMasterList *ecml, int n)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	g_return_val_if_fail (n < ecmla->priv->count, NULL);
	g_return_val_if_fail (n >= 0, NULL);

	return ecmla->priv->categories[n]->icon;
}

/**
 * ecmla_nth_color
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the X color representation
 * associated with the nth category in the list.
 */
static const char *
ecmla_nth_color (ECategoriesMasterList *ecml, int n)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	g_return_val_if_fail (n < ecmla->priv->count, NULL);
	g_return_val_if_fail (n >= 0, NULL);

	return ecmla->priv->categories[n]->color;
}

/**
 * ecmla_append:
 * @ecml: the master list to append to
 * @category: The category to append
 * @color: The color associated with this category
 * @icon: The pixmap file associated with this category
 */
static void
ecmla_append (ECategoriesMasterList *ecml,
	      const char *category,
	      const char *color,
	      const char *icon)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	ecmla->priv->appends = g_renew(ECategoriesMasterListArrayItem *, ecmla->priv->appends, ecmla->priv->appends_count + 1);
	ecmla->priv->appends[ecmla->priv->appends_count] = ecmlai_new (category, icon, color);
	ecmla->priv->appends_count ++;
}

/**
 * ecmla_delete:
 * @ecml: the master list to remove from.
 * @n: the item to delete.
 */
static void
ecmla_delete (ECategoriesMasterList *ecml, int n) 
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);
	int i;

	g_return_if_fail (n < ecmla->priv->count);
	g_return_if_fail (n >= 0);

	if (ecmla->priv->states == NULL) {
		ecmla->priv->states = g_new(ECategoriesMasterListArrayState, ecmla->priv->count);
		for (i = 0; i < ecmla->priv->count; i++)
			ecmla->priv->states[i] = E_CATEGORIES_MASTER_LIST_ARRAY_NORMAL;
	}
	ecmla->priv->states[n] = E_CATEGORIES_MASTER_LIST_ARRAY_DELETED;
}

/**
 * ecmla_commit:
 * @ecml: the master list to remove from.
 */
static void
ecmla_commit (ECategoriesMasterList *ecml)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);
	int count;
	int i;
	int j;
	ECategoriesMasterListArrayItem **new_list;

	if (ecmla->priv->states != NULL) {
		count = 0;
		for (i = 0; i < ecmla->priv->count; i++) {
			if (ecmla->priv->states[i] == E_CATEGORIES_MASTER_LIST_ARRAY_NORMAL)
				count ++;
		}
	} else {
		count = ecmla->priv->count;
	}

	count += ecmla->priv->appends_count;

	new_list = g_new (ECategoriesMasterListArrayItem *, count);

	j = 0;
	for (i = 0; i < ecmla->priv->count; i++) {
		if (ecmla->priv->states == NULL || ecmla->priv->states[i] == E_CATEGORIES_MASTER_LIST_ARRAY_NORMAL)
			new_list[j++] = ecmla->priv->categories[i];
		else
			ecmlai_free (ecmla->priv->categories[i]);
	}
	for (i = 0; i < ecmla->priv->appends_count; i++ )
		new_list[j++] = ecmla->priv->appends[i];

	g_free (ecmla->priv->categories);
	g_free (ecmla->priv->states);
	g_free (ecmla->priv->appends);
	ecmla->priv->states = NULL;
	ecmla->priv->appends = NULL;
	ecmla->priv->appends_count = 0;

	ecmla->priv->categories = new_list;
	ecmla->priv->count = j;

	e_categories_master_list_changed (ecml);
}

static void
ecmla_default (ECategoriesMasterListArray *ecmla)
{
	int i;

	ecmla->priv->count = BUILTIN_CATEGORY_COUNT;
	ecmla->priv->categories = g_new (ECategoriesMasterListArrayItem *, BUILTIN_CATEGORY_COUNT);

	for (i = 0; i < BUILTIN_CATEGORY_COUNT; i++) {
		char *category = e_utf8_from_locale_string(_(builtin_categories[i].category));
		ecmla->priv->categories[i] = ecmlai_new (category, builtin_categories[i].icon, NULL);
		g_free (category);
	}
}

/**
 * ecmla_reset:
 * @ecml: the master list to reset.
 */
static void
ecmla_reset (ECategoriesMasterList *ecml)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (ecml);

	ecmla_free (ecmla);
	ecmla_default (ecmla);
}

static void
ecmla_destroy (GtkObject *object)
{
	ECategoriesMasterListArray *ecmla = E_CATEGORIES_MASTER_LIST_ARRAY (object);

	ecmla_free (ecmla);
	g_free (ecmla->priv);
	ecmla->priv = NULL;

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


static void
ecmla_class_init (GtkObjectClass *object_class)
{
	ECategoriesMasterListClass *ecml_class = E_CATEGORIES_MASTER_LIST_CLASS(object_class);

	parent_class          = gtk_type_class (PARENT_TYPE);

	ecml_class->count     = ecmla_count    ;
	ecml_class->nth       = ecmla_nth      ;
	ecml_class->nth_icon  = ecmla_nth_icon ;
	ecml_class->nth_color = ecmla_nth_color;
	ecml_class->append    = ecmla_append   ;
	ecml_class->delete    = ecmla_delete   ;
	ecml_class->commit    = ecmla_commit   ;

	ecml_class->reset     = ecmla_reset    ;

	object_class->destroy = ecmla_destroy  ;
}

static void
ecmla_init (ECategoriesMasterListArray *ecmla)
{
	ecmla->priv                = g_new (ECategoriesMasterListArrayPriv, 1);
	ecmla->priv->count         = 0;
	ecmla->priv->categories    = NULL;
	ecmla->priv->states        = NULL;
	ecmla->priv->appends       = NULL;
	ecmla->priv->appends_count = 0;

	ecmla_default (ecmla);
}

guint
e_categories_master_list_array_get_type (void)
{
	static guint type = 0;
	
	if (!type) {
		GtkTypeInfo info = {
			"ECategoriesMasterListArray",
			sizeof (ECategoriesMasterListArray),
			sizeof (ECategoriesMasterListArrayClass),
			(GtkClassInitFunc) ecmla_class_init,
			(GtkObjectInitFunc) ecmla_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		type = gtk_type_unique (PARENT_TYPE, &info);
	}

	return type;
}

ECategoriesMasterList *
e_categories_master_list_array_new       (void)
{
	return E_CATEGORIES_MASTER_LIST (gtk_type_new (e_categories_master_list_array_get_type ()));
}

void
e_categories_master_list_array_from_string (ECategoriesMasterListArray *ecmla,
					    const char *string)
{
	xmlDoc *doc;
	xmlNode *node;
	xmlNode *children;
	int count;
	int i;
	ECategoriesMasterListArrayItem **categories;

	char *string_copy = g_strdup (string);
	doc = xmlParseMemory(string_copy, strlen (string_copy));
	node = xmlDocGetRootElement (doc);
	g_free (string_copy);

	count = 0;

	for (children = node->xmlChildrenNode;
	     children;
	     children = children->next) {
		count ++;
	}

	categories = g_new (ECategoriesMasterListArrayItem *, count);
	i = 0;

	for (children = node->xmlChildrenNode;
	     children;
	     children = children->next) {
		categories[i++] = ecmlai_new (e_xml_get_string_prop_by_name (children, "a"),
					      e_xml_get_string_prop_by_name (children, "icon"),
					      e_xml_get_string_prop_by_name (children, "color"));
	}

	ecmla_free (ecmla);
	ecmla->priv->count = count;
	ecmla->priv->categories = categories;

	e_categories_master_list_changed (E_CATEGORIES_MASTER_LIST(ecmla));

	xmlFreeDoc (doc);
}

char *
e_categories_master_list_array_to_string (ECategoriesMasterListArray *ecmla)
{
	xmlDoc *doc;
	xmlNode *node;
	xmlNode *child;
	int i;
	char *string;
	xmlChar *temp;
	int length;

	doc = xmlNewDoc (XML_DEFAULT_VERSION);
	node = xmlNewNode (NULL, "catlist");
	xmlDocSetRootElement (doc, node);

	for (i = 0; i < ecmla->priv->count; i++) {
		child = xmlNewChild (node, NULL, "cat", NULL);
		e_xml_set_string_prop_by_name (child, "a", ecmla->priv->categories[i]->category);
		if (ecmla->priv->categories[i]->color)
			e_xml_set_string_prop_by_name (child, "color", ecmla->priv->categories[i]->color);
		if (ecmla->priv->categories[i]->icon)
			e_xml_set_string_prop_by_name (child, "icon", ecmla->priv->categories[i]->icon);
	}
	xmlDocDumpMemory (doc, &temp, &length);
	string = g_strdup (temp);
	xmlFree (temp);
	return string;
}
