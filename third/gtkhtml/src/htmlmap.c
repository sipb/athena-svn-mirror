/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* This file is part of the GtkHTML library.
    
   Copyright (C) 2001, Ximian, Inc.

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


#include <config.h>
#include <string.h>
#include <glib.h>
#include "htmltypes.h"
#include "htmlmap.h"
#include "htmlshape.h"

HTMLMapClass html_map_class;
static HTMLObjectClass *parent_class = NULL;

static void
destroy (HTMLObject *self)
{
	HTMLMap *map = HTML_MAP (self);
	gint i;

	for (i = 0; i < map->shapes->len; i++) {
		html_shape_destroy (g_ptr_array_index (map->shapes, i));
	}
	
	g_ptr_array_free (map->shapes, FALSE);
	map->shapes = NULL;

	g_free (map->name);
	(* parent_class->destroy) (self);
}

void
html_map_class_init (HTMLMapClass *klass,
		     HTMLType type,
		     guint object_size)
{
	HTMLObjectClass *object_class;

	object_class = HTML_OBJECT_CLASS (klass);

	html_object_class_init (object_class, type, object_size);

	object_class->destroy = destroy;

	parent_class = &html_object_class;
}

void
html_map_add_shape (HTMLMap *map, HTMLShape *shape)
{
	g_return_if_fail (shape != NULL);

	g_ptr_array_add (map->shapes, shape);
}

char *
html_map_calc_point (HTMLMap *map, gint x, gint y)
{
	int i;

	for (i = 0; i < map->shapes->len; i++) {
		HTMLShape *shape;
		shape = g_ptr_array_index (map->shapes, i);

		if (html_shape_point (shape, x, y)) {
			return html_shape_get_url (shape);
		}		 
	}
	return NULL;
}

void
html_map_type_init (void)
{
	html_map_class_init (&html_map_class, HTML_TYPE_MAP, sizeof (HTMLMap));
}

void
html_map_init (HTMLMap *map,
	       HTMLMapClass *klass,
	       const gchar *name)
{
	html_object_init (HTML_OBJECT (map), HTML_OBJECT_CLASS (klass));

	map->shapes = g_ptr_array_new ();
	map->name = g_strdup (name);
}

HTMLObject *
html_map_new (const gchar *name)
{
	HTMLMap *map;

	map = g_new (HTMLMap, 1);
	html_map_init (map, &html_map_class, name);

	return HTML_OBJECT (map);
}









