/* Metal theme engine
 * Copyright (C) 2001 Red Hat, Inc.
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
 *
 * Written by Owen Taylor <otaylor@redhat.com>
 */

#include "metal_style.h"
#include "metal_rc_style.h"

static void      metal_rc_style_init         (MetalRcStyle      *style);
static void      metal_rc_style_class_init   (MetalRcStyleClass *klass);
static GtkStyle *metal_rc_style_create_style (GtkRcStyle         *rc_style);

static GtkRcStyleClass *parent_class;

GType metal_type_rc_style = 0;

void
metal_rc_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (MetalRcStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) metal_rc_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (MetalRcStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) metal_rc_style_init,
  };
  
  metal_type_rc_style = g_type_module_register_type (module,
						     GTK_TYPE_RC_STYLE,
						     "MetalRcStyle",
						     &object_info, 0);
}

static void
metal_rc_style_init (MetalRcStyle *style)
{
}

static void
metal_rc_style_class_init (MetalRcStyleClass *klass)
{
  GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  rc_style_class->create_style = metal_rc_style_create_style;
}

/* Create an empty style suitable to this RC style
 */
static GtkStyle *
metal_rc_style_create_style (GtkRcStyle *rc_style)
{
  return GTK_STYLE (g_object_new (METAL_TYPE_STYLE, NULL));
}

#define SCROLLBAR_WIDTH 17

/*   rangeclass->min_slider_size = SCROLLBAR_WIDTH; */

