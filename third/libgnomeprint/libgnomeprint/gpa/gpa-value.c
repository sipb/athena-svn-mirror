/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-value.c:
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors :
 *    Jose M. Celorio <chema@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_VALUE_C__

#include <config.h>
#include "gpa-value.h"
#include <libxml/globals.h>

/* GPAValue */

static void gpa_value_class_init (GPAValueClass *klass);
static void gpa_value_init (GPAValue *value);

static void gpa_value_finalize (GObject *object);

GPANode *gpa_value_duplicate (GPANode *node);
static guchar *gpa_value_get_value (GPANode *node);

static GPANodeClass *parent_class = NULL;

GType
gpa_value_get_type (void) {
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAValueClass),
			NULL, NULL,
			(GClassInitFunc) gpa_value_class_init,
			NULL, NULL,
			sizeof (GPAValue),
			0,
			(GInstanceInitFunc) gpa_value_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAValue", &info, 0);
	}
	return type;
}

static void
gpa_value_class_init (GPAValueClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_value_finalize;

	node_class->duplicate = gpa_value_duplicate;
	node_class->get_value = gpa_value_get_value;
}

static void
gpa_value_init (GPAValue *value)
{
	value->value = NULL;
}

static void
gpa_value_finalize (GObject *object)
{
	GPAValue *value;

	value = GPA_VALUE (object);

	if (value->value) {
		g_free (value->value);
		value->value = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

GPANode *
gpa_value_duplicate (GPANode *node)
{
	GPAValue *value, *new;

	value = GPA_VALUE (node);

	new = (GPAValue *) gpa_node_new (GPA_TYPE_VALUE, NULL);

	if (value->value) new->value = g_strdup (value->value);

	return GPA_NODE (new);
}

static guchar *
gpa_value_get_value (GPANode *node)
{
	GPAValue *value;

	value = GPA_VALUE (node);

	if (value->value) return g_strdup (value->value);

	return NULL;
}

GPANode *
gpa_value_new (const guchar *id, const guchar *content)
{
	GPAValue *value;

	g_return_val_if_fail (content != NULL, NULL);
	g_return_val_if_fail (*content != '\0', NULL);
	g_return_val_if_fail (!id || *id, NULL);

	value = GPA_VALUE (gpa_node_new (GPA_TYPE_VALUE, id));

	value->value = g_strdup (content);

	return GPA_NODE (value);
}

GPANode *
gpa_value_new_from_tree (const guchar *id, xmlNodePtr tree)
{
	GPANode *value;
	xmlChar *xmlval;

	g_return_val_if_fail (tree != NULL, NULL);
	g_return_val_if_fail (!id || *id, NULL);

	xmlval = xmlNodeGetContent (tree);
	g_return_val_if_fail (xmlval != NULL, NULL);

	value = gpa_value_new (id, xmlval);

	xmlFree (xmlval);

	return value;
}

gboolean
gpa_value_set_value_forced (GPAValue *value, const guchar *val)
{
	g_return_val_if_fail (value != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_VALUE (value), FALSE);
	g_return_val_if_fail (val != NULL, FALSE);
	g_return_val_if_fail (*val != '\0', FALSE);

	if (value->value) g_free (value->value);
	value->value = g_strdup (val);

	return TRUE;
}


