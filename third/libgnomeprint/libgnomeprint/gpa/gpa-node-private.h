/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_NODE_PRIVATE_H__
#define __GPA_NODE_PRIVATE_H__

/*
 * This file is part of libgnomeprint 2
 *
 * Libgnomeprint is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * Libgnomeprint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the libgnomeprint; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors :
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

G_BEGIN_DECLS

#include <glib-object.h>
#include "gpa-utils.h"

#define GPA_NODE_MODIFIED_SHIFT 0
#define GPA_NODE_MODIFIED_FLAG (1 << GPA_NODE_MODIFIED_SHIFT)
#define GPA_MODIFIED_FLAG GPA_NODE_MODIFIED_FLAG
#define GPA_NODE_LAST_SHIFT GPA_NODE_MODIFIED_SHIFT

#define GPA_NODE_FLAGS(n) (GPA_NODE(n)->flags)
#define GPA_NODE_SET_FLAGS(n,f) (GPA_NODE(n)->flags |= f)
#define GPA_NODE_UNSET_FLAGS(n,f) (GPA_NODE(n)->flags &= (~f))

struct _GPANode {
	GObject object;
	guint32 flags;
	guint32 qid;
	GPANode *parent;
	GPANode *next;
};

struct _GPANodeClass {
	GObjectClass object_class;

	GPANode * (* duplicate) (GPANode *node);
	gboolean (* verify) (GPANode *node);

	guchar * (* get_value) (GPANode *node);
	gboolean (* set_value) (GPANode *node, const guchar *value);

	GPANode * (* get_child) (GPANode *node, GPANode *ref);

	GPANode * (* lookup) (GPANode *node, const guchar *path);

	/* Signal that node or its children have been modified */
	void (* modified) (GPANode *node, guint flags);
};

#define GPA_NODE_ID(n) (gpa_quark_to_string (GPA_NODE (n)->qid))
#define GPA_NODE_ID_COMPARE(n,s) ((s) && (gpa_quark_try_string (s) == GPA_NODE (n)->qid))
#define GPA_NODE_ID_EXISTS(n) (GPA_NODE (n)->qid != 0)
#define GPA_NODE_PARENT(n) (GPA_NODE (n)->parent)

GType gpa_node_get_type (void);

GPANode *gpa_node_new (GType type, const guchar *id);
GPANode *gpa_node_construct (GPANode *node, const guchar *id);

GPANode *gpa_node_duplicate (GPANode *node);
gboolean gpa_node_verify (GPANode *node);

GPANode *gpa_node_lookup (GPANode *node, const guchar *path);

/* Request that "modified" signal will be run from idle loop */
void gpa_node_request_modified (GPANode *node, guint flags);
void gpa_node_emit_modified (GPANode *node, guint flags);


/* fixme: */
GSList *gpa_list_nodes (void);

G_END_DECLS

#endif

