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
#include "htmlmap.h"
#include "htmlshape.h"

void
html_map_destroy (HTMLMap *map)
{
	gint i;

	for (i = 0; i < map->shapes->len; i++)
		html_shape_destroy (g_ptr_array_index (map->shapes, i));
	
	g_ptr_array_free (map->shapes, FALSE);
	map->shapes = NULL;

	g_free (map->name);
	g_free (map);
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

HTMLMap *
html_map_new (const gchar *name)
{
	HTMLMap *map;

	map = g_new (HTMLMap, 1);
	map->shapes = g_ptr_array_new ();
	map->name = g_strdup (name);

	return map;
}
