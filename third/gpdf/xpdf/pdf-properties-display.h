/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF Properties Display Widget
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PDF_PROPERTIES_DISPLAY_H
#define PDF_PROPERTIES_DISPLAY_H

G_BEGIN_DECLS

#include <glib-object.h>
#include <gtk/gtkhbox.h>

#define GPDF_TYPE_PROPERTIES_DISPLAY            (gpdf_properties_display_get_type ())
#define GPDF_PROPERTIES_DISPLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_PROPERTIES_DISPLAY, GPdfPropertiesDisplay))
#define GPDF_PROPERTIES_DISPLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_PROPERTIES_DISPLAY, GPdfPropertiesDisplayClass))
#define GPDF_IS_PROPERTIES_DISPLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_PROPERTIES_DISPLAY))
#define GPDF_IS_PROPERTIES_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_PROPERTIES_DISPLAY))

typedef struct _GPdfPropertiesDisplay        GPdfPropertiesDisplay;
typedef struct _GPdfPropertiesDisplayClass   GPdfPropertiesDisplayClass;
typedef struct _GPdfPropertiesDisplayPrivate GPdfPropertiesDisplayPrivate;

struct _GPdfPropertiesDisplay {
	        GtkHBox parent;
	
	        GPdfPropertiesDisplayPrivate *priv;
	};

struct _GPdfPropertiesDisplayClass {
	        GtkHBoxClass parent_class;
	};

GType gpdf_properties_display_get_type (void);

G_END_DECLS

#endif /* PDF_PROPERTIES_DISPLAY_H */
