/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef __GPA_UTILS_H__
#define __GPA_UTILS_H__

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
 *   Jose M. Celorio <chema@ximian.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2000-2001 Ximian, Inc. and Jose M. Celorio
 *
 * Contains pieces of code from glib, Copyright GLib Team and
 * others 1997-2000.
 *
 */

#include <glib/gmacros.h>

G_BEGIN_DECLS

#include <libxml/tree.h>
#include "gpa-node.h"

guchar *gpa_id_new (const guchar *key);

/* Attach and detach */

GPANode *gpa_node_attach (GPANode *parent, GPANode *child);
GPANode *gpa_node_attach_ref (GPANode *parent, GPANode *child);

GPANode *gpa_node_detach (GPANode *parent, GPANode *child);
GPANode *gpa_node_detach_unref (GPANode *parent, GPANode *child);

GPANode *gpa_node_detach_next (GPANode *parent, GPANode *child);
GPANode *gpa_node_detach_unref_next (GPANode *parent, GPANode *child);

/* Lookup */

const guchar *gpa_node_lookup_check (const guchar *path, const guchar *key);
gboolean gpa_node_lookup_ref (GPANode **child, GPANode *node, const guchar *path, const guchar *key);

/* XML helpers */

xmlChar *gpa_xml_node_get_name (xmlNodePtr node);
xmlNodePtr gpa_xml_node_get_child (xmlNodePtr node, const guchar *name);

/* Cache */

GPANode *gpa_node_cache (GPANode *node);

/* Dumps the node and all nodes below it to the console */
void gpa_utils_dump_tree (GPANode *node);

/* Private quarks */

typedef guint GPAQuark;

GPAQuark gpa_quark_try_string (const guchar *string);
GPAQuark gpa_quark_from_string (const guchar *string);
GPAQuark gpa_quark_from_static_string (const guchar *string);
const guchar *gpa_quark_to_string (GPAQuark quark);

G_END_DECLS

#endif /* __GPA_UTILS_H__ */
