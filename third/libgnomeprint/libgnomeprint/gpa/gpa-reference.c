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
 *    Jose M. Celorio <chema@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 */

#define __GPA_REFERENCE_C__

#include <string.h>

#include "gpa-reference.h"

/* GPAReference */

enum {SET_VALUE, LAST_SIGNAL};

static void gpa_reference_class_init (GPAReferenceClass *klass);
static void gpa_reference_init (GPAReference *reference);

static void gpa_reference_finalize (GObject *object);

static GPANode *gpa_reference_duplicate (GPANode *node);
static gboolean gpa_reference_verify (GPANode *node);
static guchar *gpa_reference_get_value (GPANode *node);
static gboolean gpa_reference_set_value (GPANode *node, const guchar *value);
static GPANode *gpa_reference_get_child (GPANode *node, GPANode *ref);
static GPANode *gpa_reference_lookup (GPANode *node, const guchar *path);
static void gpa_reference_modified (GPANode *node, guint flags);

static void gpa_reference_reference_modified (GPANode *node, guint flags, gpointer data);

static void gpa_cclosure_marshal_BOOLEAN__STRING (GClosure *closure, GValue *ret,
						  guint n_param, const GValue *param, gpointer hint, gpointer data);

static GPANodeClass *parent_class = NULL;
static guint reference_signals[LAST_SIGNAL] = {0};

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

	reference_signals[SET_VALUE] = g_signal_new ("set_value",
					       G_OBJECT_CLASS_TYPE (object_class),
					       G_SIGNAL_RUN_LAST,
					       G_STRUCT_OFFSET (GPAReferenceClass, set_value),
					       NULL, NULL,
					       gpa_cclosure_marshal_BOOLEAN__STRING,
					       G_TYPE_BOOLEAN, 1, G_TYPE_STRING);

	object_class->finalize = gpa_reference_finalize;

	node_class->duplicate = gpa_reference_duplicate;
	node_class->verify = gpa_reference_verify;
	node_class->get_value = gpa_reference_get_value;
	node_class->set_value = gpa_reference_set_value;
	node_class->get_child = gpa_reference_get_child;
	node_class->lookup = gpa_reference_lookup;
	node_class->modified = gpa_reference_modified;
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

	if (reference->ref) {
		g_signal_handlers_disconnect_matched (G_OBJECT (reference->ref),
						      G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, reference);
		reference->ref = gpa_node_unref (reference->ref);
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static GPANode *
gpa_reference_duplicate (GPANode *node)
{
	GPAReference *reference, *new;

	reference = GPA_REFERENCE (node);

	new = (GPAReference *) gpa_node_new (GPA_TYPE_REFERENCE, GPA_NODE_ID (node));

	new->ref = reference->ref;
	if (new->ref) {
		gpa_node_ref (new->ref);
		g_signal_connect (G_OBJECT (new->ref), "modified",
				  (GCallback) gpa_reference_reference_modified, new);
	}

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

static guchar *
gpa_reference_get_value (GPANode *node)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (reference->ref) {
		return gpa_node_get_value (reference->ref);
	}

	return NULL;
}

static gboolean
gpa_reference_set_value (GPANode *node, const guchar *value)
{
	GPAReference *reference;
	gboolean ret;

	reference = GPA_REFERENCE (node);

	ret = FALSE;
	g_signal_emit (G_OBJECT (node), reference_signals[SET_VALUE], 0, value, &ret);

	return ret;
}

static GPANode *
gpa_reference_get_child (GPANode *node, GPANode *ref)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (reference->ref) {
		return gpa_node_get_child (reference->ref, ref);
	}

	return NULL;
}

static GPANode *
gpa_reference_lookup (GPANode *node, const guchar *path)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (reference->ref) {
		return gpa_node_lookup (reference->ref, path);
	}

	return NULL;
}

static void
gpa_reference_modified (GPANode *node, guint flags)
{
	GPAReference *reference;

	reference = GPA_REFERENCE (node);

	if (reference->ref && (GPA_NODE_FLAGS (reference->ref) & GPA_MODIFIED_FLAG)) {
		gpa_node_emit_modified (reference->ref, 0);
	}
}

static void
gpa_reference_reference_modified (GPANode *node, guint flags, gpointer data)
{
	gpa_node_request_modified (GPA_NODE (data), 0);
}

GPANode *
gpa_reference_new (GPANode *ref)
{
	GPAReference *reference;

	g_return_val_if_fail (ref != NULL, NULL);
	g_return_val_if_fail (GPA_IS_NODE (ref), NULL);

	reference = (GPAReference *) gpa_node_new (GPA_TYPE_REFERENCE, GPA_NODE_ID (ref));

	reference->ref = gpa_node_ref (ref);
	g_signal_connect (G_OBJECT (reference->ref), "modified",
			  (GCallback) gpa_reference_reference_modified, reference);


	return GPA_NODE (reference);
}

GPANode *
gpa_reference_new_empty (void)
{
	GPAReference *reference;

	reference = (GPAReference *) gpa_node_new (GPA_TYPE_REFERENCE, NULL);

	return GPA_NODE (reference);
}

gboolean
gpa_reference_set_reference (GPAReference *reference, GPANode *ref)
{
	g_return_val_if_fail (reference != NULL, FALSE);
	g_return_val_if_fail (GPA_IS_REFERENCE (reference), FALSE);
	g_return_val_if_fail (!ref || GPA_IS_NODE (ref), FALSE);

	if (reference->ref) {
		g_signal_handlers_disconnect_matched (G_OBJECT (reference->ref),
						      G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, reference);
		reference->ref = gpa_node_unref (reference->ref);
	}

	if (ref) {
		reference->ref = gpa_node_ref (ref);
		g_signal_connect (G_OBJECT (reference->ref), "modified",
				  (GCallback) gpa_reference_reference_modified, reference);
	}

	gpa_node_request_modified (GPA_NODE (reference), 0);

	return TRUE;
}

static void
gpa_cclosure_marshal_BOOLEAN__STRING (GClosure *closure, GValue *ret, guint n_param, const GValue *param, gpointer hint, gpointer data)
{
	typedef gboolean (*GMarshalFunc_BOOLEAN__STRING) (gpointer data1, gpointer arg1, gpointer data2);

	GMarshalFunc_BOOLEAN__STRING callback;
	GCClosure *cc = (GCClosure *) closure;
	gpointer data1, data2;
	gboolean retval;
	
	g_return_if_fail (ret != NULL);
	g_return_if_fail (n_param == 2);

	if (G_CCLOSURE_SWAP_DATA (closure)) {
		data1 = closure->data;
		data2 = g_value_peek_pointer (param + 0);
	} else {
		data2 = closure->data;
		data1 = g_value_peek_pointer (param + 0);
	}

	callback = (GMarshalFunc_BOOLEAN__STRING) (data ? data : cc->callback);

	retval = callback (data1, (gchar *) g_value_get_string (param + 1), data2);

	g_value_set_boolean (ret, retval);
}

