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

#include <glib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include "htmlstyle.h"
#include "htmlshape.h"

struct _HTMLShape {
	HTMLShapeType type;
	gchar *url;
	gchar *target;
	GPtrArray *coords;
};

static HTMLLength *
parse_length (char **str) {
	char *cur = *str;
	HTMLLength *len = g_new0 (HTMLLength, 1);
	
	/* g_warning ("begin \"%s\"", *str); */

	while (isspace (*cur)) cur++;

	len->val = atoi (cur);
	len->type = HTML_LENGTH_TYPE_PIXELS;

	while (isdigit (*cur) || *cur == '-') cur++;

	switch (*cur) {
	case '*':
		if (len->val == 0)
			len->val = 1;
		len->type = HTML_LENGTH_TYPE_FRACTION;
		cur++;
		break;
	case '%':
		len->type = HTML_LENGTH_TYPE_PERCENT;
                cur++;
		break;
	}
	
	if (cur <= *str) {
		g_free (len);
		return NULL;
	} 

	/* g_warning ("length len->val=%d, len->type=%d", len->val, len->type); */
	*str = cur;
	cur = strstr (cur, ",");	
	if (cur) {
		cur++;
		*str = cur;
	}

	return len;
}

void
html_length_array_parse (GPtrArray *array, char *str)
{
	HTMLLength *length;

	if (str == NULL)
		return;

	while ((length = parse_length (&str)))
	       g_ptr_array_add (array, length);

}

void
html_length_array_destroy (GPtrArray *array)
{
	int i;
	
	for (i = 0; i < array->len; i++)
		g_free (g_ptr_array_index (array, i));
}

#define HTML_DIST(x,y) (gint)sqrt((x)*(x) + (y)*(y))

gboolean
html_shape_point (HTMLShape *shape, gint x, gint y)
{
	int i;
	int j = 0;
	int odd = 0;

	HTMLLength **poly = (HTMLLength **)shape->coords->pdata;

	/*
	 * TODO: Add support for percentage lengths, The information is stored
	 * so it is simply a matter of modifying the point routine to know the
	 * the overall size, then scaling the values.
	 */

	switch (shape->type) {
	case HTML_SHAPE_RECT:
		if ((x >= poly[0]->val) 
		    && (x <= poly[2]->val) 
		    && (y >= poly[1]->val) 
		    && (y <= poly[3]->val))
			return TRUE;
		break;
	case HTML_SHAPE_CIRCLE:
		if (HTML_DIST (x - poly[0]->val, y - poly[1]->val) <= poly[2]->val)
			return TRUE;
		
		break;
	case HTML_SHAPE_POLY:
		for (i=0; i < shape->coords->len; i+=2) {
			j+=2; 
			if (j == shape->coords->len) 
				j=0;
			
			if ((poly[i+1]->val < y && poly[j+1]->val >= y)
			    || (poly[j+1]->val < y && poly[i+1]->val >= y)) {
				
				if (poly[i]->val + (y - poly[i+1]->val) 
				    / (poly[j+1]->val - poly[i+1]->val) 
				    * (poly[j]->val - poly[i]->val) < x) {
					odd = !odd;
				}
			}
		}
		return odd;
		break;
	case HTML_SHAPE_DEFAULT:
		return TRUE;
		break;
	}
	return FALSE;
}

static HTMLShapeType
parse_shape_type (char *token) {
	HTMLShapeType type = HTML_SHAPE_RECT;

	if (!token || strncasecmp (token, "rect", 4) == 0)
		type = HTML_SHAPE_RECT;
	else if (strncasecmp (token, "poly", 4) == 0)
		type = HTML_SHAPE_POLY;
	else if (strncasecmp (token, "circle", 6) == 0)
		type = HTML_SHAPE_CIRCLE;
	else if (strncasecmp (token, "default", 7) == 0)
		type = HTML_SHAPE_DEFAULT;

	return type;
}

char *
html_shape_get_url (HTMLShape *shape)
{
	return shape->url;
}

HTMLShape *
html_shape_new (char *type_str, char *coords, char *url, char *target)
{
	HTMLShape *shape;
	HTMLShapeType type = parse_shape_type (type_str);

	if (coords == NULL && type != HTML_SHAPE_DEFAULT)
		return NULL;

	shape = g_new (HTMLShape, 1);

	shape->type = type;
	shape->url = g_strdup (url);
	shape->target = g_strdup (target);
	shape->coords = g_ptr_array_new ();

	html_length_array_parse (shape->coords, coords);
	
	switch (shape->type) {
	case HTML_SHAPE_RECT:
		while (shape->coords->len < 4)
			g_ptr_array_add (shape->coords, 
					 g_new0 (HTMLLength, 1));
	case HTML_SHAPE_CIRCLE:
		while (shape->coords->len < 3)
			g_ptr_array_add (shape->coords, 
					 g_new0 (HTMLLength, 1));
	case HTML_SHAPE_POLY:
		if (shape->coords->len % 2)
			g_ptr_array_add (shape->coords, 
					 g_new0 (HTMLLength, 1));

		break;
	default:
		break;
	}
	return shape;
}

void
html_shape_destroy (HTMLShape *shape)
{
	g_free (shape->url);
	g_free (shape->target);
	html_length_array_destroy (shape->coords);
	
	g_free (shape);
}
		


