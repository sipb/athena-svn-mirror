/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * e-categories-master-list.c - The base class for the master list of categories.
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
#include "gal/util/e-util.h"
#include "e-categories-master-list.h"

#define PARENT_TYPE G_TYPE_OBJECT

#define d(x)

static GObjectClass *parent_class;

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

/**
 * e_categories_master_list_count:
 * @e_categories_master_list: The e-categories-master-list to operate on
 *
 * Returns: the number of categories in the list.
 */
int
e_categories_master_list_count (ECategoriesMasterList *ecml)
{
	g_return_val_if_fail (ecml != NULL, 0);
	g_return_val_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml), 0);

	return E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->count (ecml);
}

/**
 * e_categories_master_list_nth:
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the nth category in the list.
 */
const char *
e_categories_master_list_nth (ECategoriesMasterList *ecml, int n)
{
	g_return_val_if_fail (ecml != NULL, NULL);
	g_return_val_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml), NULL);

	return E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->nth (ecml, n);
}

/**
 * e_categories_master_list_nth_icon
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the pixmap file associated
 * with the nth category in the list.
 */
const char *
e_categories_master_list_nth_icon (ECategoriesMasterList *ecml, int n)
{
	g_return_val_if_fail (ecml != NULL, NULL);
	g_return_val_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml), NULL);

	return E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->nth_icon (ecml, n);
}

/**
 * e_categories_master_list_nth_color
 * @ecml: the e-categories-master-list to operate on
 * @n: The category to return.
 *
 * Return value: This function returns the X representation of the
 * color associated with the nth category in the list.
 */
const char *
e_categories_master_list_nth_color (ECategoriesMasterList *ecml, int n)
{
	g_return_val_if_fail (ecml != NULL, NULL);
	g_return_val_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml), NULL);

	return E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->nth_color (ecml, n);
}

/**
 * e_categories_master_list_append:
 * @ecml: the master list to append to
 * @category: The category to append
 * @color: The color associated with this category
 * @icon: The pixmap file associated with this category
 */
void
e_categories_master_list_append (ECategoriesMasterList *ecml,
				 const char *category,
				 const char *color,
				 const char *icon)
{
	g_return_if_fail (ecml != NULL);
	g_return_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml));
	g_return_if_fail (category != NULL);

	if (E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->append)
		E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->append (ecml, category, color, icon);
}

/**
 * e_categories_master_list_delete:
 * @ecml: the master list to remove from.
 * @n: the item to delete.
 */
void
e_categories_master_list_delete (ECategoriesMasterList *ecml, int n) 
{
	g_return_if_fail (ecml != NULL);
	g_return_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml));

	if (E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->delete)
		E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->delete (ecml, n);
}

/**
 * e_categories_master_list_commit:
 * @e_categories_master_list: The e-categories-master-list to operate on
 *
 * Returns: the number of categories in the list.
 */
void
e_categories_master_list_commit (ECategoriesMasterList *ecml)
{
	g_return_if_fail (ecml != NULL);
	g_return_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml));

	E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->commit (ecml);
}


/**
 * e_categories_master_list_reset:
 * @e_categories_master_list: The e-categories-master-list to operate on
 *
 * Returns: the number of categories in the list.
 */
void
e_categories_master_list_reset (ECategoriesMasterList *ecml)
{
	g_return_if_fail (ecml != NULL);
	g_return_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml));

	E_CATEGORIES_MASTER_LIST_GET_CLASS (ecml)->reset (ecml);
}


static void
e_categories_master_list_class_init (GObjectClass *object_class)
{
	ECategoriesMasterListClass *klass = E_CATEGORIES_MASTER_LIST_CLASS(object_class);
	parent_class = g_type_class_ref (PARENT_TYPE);
	
	signals [CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (ECategoriesMasterListClass, changed),
			      NULL, NULL,
			      e_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);

	klass->count   = NULL;     
	klass->nth     = NULL;         
	klass->append  = NULL;
	klass->delete  = NULL; 
	klass->commit  = NULL;     

	klass->reset   = NULL;     

	klass->changed = NULL;
}


E_MAKE_TYPE (e_categories_master_list,
	     "ECategoriesMasterList",
	     ECategoriesMasterList,
	     e_categories_master_list_class_init,
	     NULL,
	     PARENT_TYPE)

/**
 * e_categories_master_list_changed:
 * @ecml: the master list to notify of the change
 *
 * Use this function to notify any views of this master list that the
 * list of categories has changed.  This will emit the signal
 * "changed" on the @ecml object.
 */
void
e_categories_master_list_changed (ECategoriesMasterList *ecml)
{
	g_return_if_fail (ecml != NULL);
	g_return_if_fail (E_IS_CATEGORIES_MASTER_LIST (ecml));
	
	g_signal_emit (ecml,
		       signals [CHANGED], 0);
}
