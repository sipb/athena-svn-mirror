/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef __HTML_A11Y_H__
#define __HTML_A11Y_H__

#include <atk/atkobject.h>
#include <atk/atkcomponent.h>
#include "htmltypes.h"
#include "object.h"

#define G_TYPE_HTML_A11Y            (html_a11y_get_type ())
#define HTML_A11Y(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_HTML_A11Y, HTMLA11Y))
#define HTML_A11Y_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass),  G_TYPE_HTML_A11Y, HTMLA11YClass))
#define G_IS_HTML_A11Y(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), G_TYPE_HTML_A11Y))
#define G_IS_HTML_A11Y_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  G_TYPE_HTML_A11Y))
#define HTML_A11Y_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj),  G_TYPE_HTML_A11Y, HTMLA11YClass))

#define HTML_ID "html-object"
#define HTML_A11Y_HTML(o) HTML_OBJECT (g_object_get_data (G_OBJECT (o), HTML_ID))

typedef struct _HTMLA11Y      HTMLA11Y;
typedef struct _HTMLA11YClass HTMLA11YClass;

struct _HTMLA11Y {
	AtkObject atk_object;
};

GType html_a11y_get_type (void);

struct _HTMLA11YClass {
	AtkObjectClass parent_class;
};

AtkObject * html_a11y_new (HTMLObject *html_obj, AtkRole role);

GtkHTMLA11Y * html_a11y_get_gtkhtml_parent (HTMLA11Y *obj);

/* private, used in text.c */
void  html_a11y_get_extents (AtkComponent *component, gint *x, gint *y, gint *width, gint *height, AtkCoordType coord_type);
void  html_a11y_get_size (AtkComponent *component, gint *width, gint *height);

#endif
