/*
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "htmlboxtableaccessible.h"
#include "htmlboxaccessible.h"
#include "a11y/htmlviewaccessible.h"
#include <libgtkhtml/layout/htmlboxtable.h>

static void         html_box_table_accessible_class_init               (HtmlBoxTableAccessibleClass  *klass);

static gint         html_box_table_accessible_get_n_children           (AtkObject        *obj);
static AtkObject*   html_box_table_accessible_ref_child                (AtkObject        *obj,
                                                                        gint             i);

static gpointer parent_class = NULL;

GType
html_box_table_accessible_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HtmlBoxTableAccessibleClass),
			(GBaseInitFunc) NULL, /* base init */
			(GBaseFinalizeFunc) NULL, /* base finalize */
			(GClassInitFunc) html_box_table_accessible_class_init,
			(GClassFinalizeFunc) NULL, /* class finalize */
			NULL, /* class data */
			sizeof (HtmlBoxTableAccessible),
			0, /* nb preallocs */
			(GInstanceInitFunc) NULL, /* instance init */
			NULL /* value table */
		};

		type = g_type_register_static (HTML_TYPE_BOX_ACCESSIBLE, "HtmlBoxTableAccessible", &tinfo, 0);
	}

	return type;
}

static void
html_box_table_accessible_class_init (HtmlBoxTableAccessibleClass *klass)
{
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	class->get_n_children = html_box_table_accessible_get_n_children;
	class->ref_child = html_box_table_accessible_ref_child;
}

AtkObject*
html_box_table_accessible_new (GObject *obj)
{
	GObject *object;
	AtkObject *atk_object;

	g_return_val_if_fail (HTML_IS_BOX_TABLE (obj), NULL);
	object = g_object_new (HTML_TYPE_BOX_TABLE_ACCESSIBLE, NULL);
	atk_object = ATK_OBJECT (object);
	atk_object_initialize (atk_object, obj);
	atk_object->role = ATK_ROLE_TABLE;
	return atk_object;
}

static gint
html_box_table_accessible_get_n_children (AtkObject *obj)
{
	AtkGObjectAccessible *atk_gobject;
	HtmlBoxTable *box_table;
	HtmlBox *box;
	GObject *g_obj;
	GSList *last;
        gint n_rows, n_cells;

	g_return_val_if_fail (HTML_IS_BOX_TABLE_ACCESSIBLE (obj), 0);
	atk_gobject = ATK_GOBJECT_ACCESSIBLE (obj);
	g_obj = atk_gobject_accessible_get_object (atk_gobject);
	if (g_obj == NULL)
		return 0;

	g_return_val_if_fail (HTML_IS_BOX_TABLE (g_obj), 0);

	box_table = HTML_BOX_TABLE (g_obj);

	n_rows = g_slist_length (box_table->body_list);
	n_cells =  (n_rows -1) * box_table->cols;
	last = g_slist_last (box_table->body_list);
	box = HTML_BOX (last->data);
	if (box->children) {
		box = box->children;

		while (box) {
			n_cells++;
			box = box->next;
		}
	}
	return n_cells;
}

static AtkObject*
html_box_table_accessible_ref_child (AtkObject *obj,
                                     gint      i)
{
	AtkGObjectAccessible *atk_gobject;
	HtmlBoxTable *box_table;
	HtmlBoxTableRow *table_row;
	HtmlBox *cell;
	GObject *g_obj;
	AtkObject *atk_child = NULL;
 	gint index;
        gint n_rows;

	g_return_val_if_fail (HTML_IS_BOX_TABLE_ACCESSIBLE (obj), NULL);
	atk_gobject = ATK_GOBJECT_ACCESSIBLE (obj);
	g_obj = atk_gobject_accessible_get_object (atk_gobject);
	if (g_obj == NULL)
		return NULL;

	g_return_val_if_fail (HTML_IS_BOX_TABLE (g_obj), 0);

	box_table = HTML_BOX_TABLE (g_obj);

	n_rows = g_slist_length (box_table->body_list);
	if (i < 0 || i >= n_rows * box_table->cols)
		return NULL;

	index = g_slist_length (box_table->header_list) * box_table->cols + i;
	cell = box_table->cells[index];
	if (cell) {
		atk_child = atk_gobject_accessible_for_object (G_OBJECT (cell));
		g_object_ref (atk_child);
	}
	return atk_child;
}
