/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gpa-state.c: 
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
 *  Copyright (C) 2004 Red Hat, Inc.
 *
 */

#include <config.h>

#include <string.h>

#include "gpa-utils.h"
#include "gpa-node-private.h"
#include "gpa-state.h"

static void gpa_state_class_init (GPAStateClass *klass);
static void gpa_state_init (GPAState *key);
static void gpa_state_finalize (GObject *key);
static gboolean gpa_state_set_value (GPANode *node, const guchar *value);
static guchar *gpa_state_get_value (GPANode *node);

static GPANodeClass *parent_class = NULL;

GType
gpa_state_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GPAStateClass),
			NULL, NULL,
			(GClassInitFunc) gpa_state_class_init,
			NULL, NULL,
			sizeof (GPAState),
			0,
			(GInstanceInitFunc) gpa_state_init
		};
		type = g_type_register_static (GPA_TYPE_NODE, "GPAState", &info, 0);
	}
	return type;
}

static void
gpa_state_class_init (GPAStateClass *klass)
{
	GObjectClass *object_class;
	GPANodeClass *node_class;

	object_class = (GObjectClass *) klass;
	node_class = (GPANodeClass *) klass;

	parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = gpa_state_finalize;
	node_class->set_value = gpa_state_set_value;
	node_class->get_value = gpa_state_get_value;
}

static void
gpa_state_init (GPAState *key)
{
}

static void
gpa_state_finalize (GObject *obj)
{
	g_free (((GPAState *)obj)->value);
}

GPAState *
gpa_state_new (const char *id)
{
	return GPA_STATE (gpa_node_new (GPA_TYPE_STATE, id));
}

static gboolean
gpa_state_set_value (GPANode *node, const guchar *value)
{
	GPAState *state = GPA_STATE (node);
	g_free (state->value);
	state->value = g_strdup (value);
	return TRUE;
}

static guchar *
gpa_state_get_value (GPANode *node)
{
	GPAState *state = GPA_STATE (node);
	return g_strdup (state->value);
}
