/*
 * HighContrast GTK+ rendering engine for Gnome-Themes.
 *
 * Copyright 2003 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtkstyle.h>

typedef struct _HcStyle HcStyle;
typedef struct _HcStyleClass HcStyleClass;

extern GType hc_type_style;
extern GtkStyleClass *style_parent_class;

#define HC_TYPE_STYLE              hc_type_style
#define HC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), HC_TYPE_STYLE, HcStyle))
#define HC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), HC_TYPE_STYLE, HcStyleClass))
#define HC_IS_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), HC_TYPE_STYLE))
#define HC_IS_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), HC_TYPE_STYLE))
#define HC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), HC_TYPE_STYLE, HcStyleClass))

struct _HcStyle
{
  GtkStyle parent_instance;
};

struct _HcStyleClass
{
  GtkStyleClass parent_class;
};

void hc_style_register_type (GTypeModule *module);
