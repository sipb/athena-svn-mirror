/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "layout/htmlbox.h"
#include "debug.h"

void
debug_dump_boxes (HtmlBox *root, gint indent, gboolean has_node, xmlNode *n)
{
	HtmlBox *box;
	gint i;

	if (!root)
		return;
	
	if (has_node) {
		if (root->dom_node != NULL && root->dom_node->xmlnode != n)
			return;
	}
	
	box = root->children;
	
	
	for (i = 0; i < indent; i++)
		g_print (" ");

	g_print ("%s (%d %d %d %d)", G_OBJECT_TYPE_NAME (G_OBJECT (root)), root->x, root->y, root->width, root->height);

	if (root->dom_node)
		g_print ("%s ", root->dom_node->xmlnode->name);
	
	g_print ("\n");
	while (box) {
		debug_dump_boxes (box, indent + 1, has_node, n);
		box = box->next;
	}
}
