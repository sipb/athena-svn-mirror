/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-reference.c:
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
 *    Chema Celorio <chema@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2003 Ximian, Inc.
 *
 */

#include <config.h>

#include "gpa-reference.h"
#include "gpa-list.h"

static void gpa_reference_class_init (GPAReferenceClass *klass);
static void gpa_reference_init (GPAReference *reference);
static void gpa_reference_finalize (GObject *object);

static GPANode * gpa_reference_duplicate (GPANode *node);
static gboolean  gpa_reference_verify    (GPANode *node);
static guchar *  gpa_reference_get_value (GPANode *node);
static gboolean  gpa_reference_set_value (GPANode *node, const guchar *value);

static GPANodeClass *parent_class = NULL;

GType
gpa_reference_get_type (void) {
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAReferenceClass),
			NULL, NULL,
			(GClassInitFunc) gpa_reference_class_init,
			NULL, NULL,
			sizeof (GPAReference),
			0,
			(GInstanceInitFunc) gpa_reference_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAReference", &info, 0);
	}
	return type;
}

static void
gpa_reference_class_init (GPAReferenceClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = gpa_reference_finalize;

	node_class->duplicate = gpa_reference_duplicate;
	node_class->verify    = gpa_reference_verify;
	node_class->get_value = gpa_reference_get_value;
	node_class->set_value = gpa_reference_set_value;
}

static void
gpa_reference_init (GPAReference *reference)
{
	reference->ref = NULL;
}

static void
gpa_reference_finalize (GObject *object)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (object);

	gpa_node_unref (reference->ref);
	reference->ref = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_reference_duplicate (GPANode *node)
{
	GPAReference *src, *new;

	src = GPA_REFERENCE (node);

	new = (GPAReference *) gpa_node_new (GPA_TYPE_REFERENCE, gpa_node_id (node));

	if (src->ref)
		gpa_node_ref (src->ref);
	new->ref = src->ref;

	return GPA_NODE (new);
}

static gboolean
gpa_reference_verify (GPANode *node)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (!reference->ref)
		return FALSE;
	if (!gpa_node_verify (reference->ref))
		return FALSE;

	return TRUE;
}

static gboolean
gpa_reference_set_value (GPANode *node, const guchar *value)
{
	GPAReference *reference;
	GPANode *old;
	GPANode *new;

	reference = GPA_REFERENCE (node);
	old = reference->ref;

	g_return_val_if_fail (old->parent != NULL, FALSE);
	g_return_val_if_fail (G_OBJECT_TYPE (old->parent) == GPA_TYPE_LIST, FALSE);

	new = gpa_node_lookup (old->parent, value);
	if (!new) {
		g_warning ("Could not GPAReference %s to %s\n", gpa_node_id (node), value);
		return FALSE;
	}

	gpa_reference_set_reference (reference, new);

	return TRUE;
}

static guchar *
gpa_reference_get_value (GPANode *node)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (!reference->ref)
		return g_strdup ("");

	return g_strdup (gpa_node_id (reference->ref));
}

GPAReference *
gpa_reference_new_emtpy (const guchar *id)
{
	GPAReference *reference;

	g_return_val_if_fail (id != NULL, NULL);
	
	reference = (GPAReference *) gpa_node_new (GPA_TYPE_REFERENCE, id);

	return reference;
}
		
GPAReference *
gpa_reference_new (GPANode *node, const guchar *id)
{
	GPAReference *reference;

	g_return_val_if_fail (node != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (node), NULL);
	g_return_val_if_fail (id != NULL, NULL);
	g_return_val_if_fail (id[0] != '0', NULL);

	reference = gpa_reference_new_emtpy (id);
	reference->ref = gpa_node_ref (node);

	return reference;
}

gboolean
gpa_reference_set_reference (GPAReference *reference, GPANode *node)
{
	g_return_val_if_fail (reference != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_REFERENCE (reference), FALSE);

	if (reference->ref)
		gpa_node_unref (reference->ref);

	if (node) {
		g_return_val_if_fail (GPA_IS_NODE (node), FALSE);
		reference->ref = gpa_node_ref (node);
	} else
		reference->ref = NULL;

	return TRUE;
}
