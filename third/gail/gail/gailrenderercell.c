/* GAIL - The GNOME Accessibility Enabling Library
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include "gailrenderercell.h"

static void      gail_renderer_cell_class_init          (GailRendererCellClass *klass);
static void      gail_renderer_cell_object_init         (GailRendererCell      *renderer_cell);

static void      gail_renderer_cell_finalize            (GObject               *object)
;
static gpointer parent_class = NULL;

GType
gail_renderer_cell_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GTypeInfo tinfo =
    {
      sizeof (GailRendererCellClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) gail_renderer_cell_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (GailRendererCell), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) gail_renderer_cell_object_init, /* instance init */
      NULL /* value table */
    };

    type = g_type_register_static (GAIL_TYPE_CELL,
                                   "GailRendererCell", &tinfo, 0);
  }
  return type;
}

static void 
gail_renderer_cell_class_init (GailRendererCellClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gail_renderer_cell_finalize;
}

static void
gail_renderer_cell_object_init (GailRendererCell *renderer_cell)
{
  renderer_cell->renderer = NULL;
}

static void
gail_renderer_cell_finalize (GObject  *object)
{
  GailRendererCell *renderer_cell = GAIL_RENDERER_CELL (object);

  if (renderer_cell->renderer)
    g_object_unref (renderer_cell->renderer);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

gboolean
gail_renderer_cell_update_cache (GailRendererCell *cell, 
                                 gboolean         emit_change_signal)
{
  GailRendererCellClass *class = GAIL_RENDERER_CELL_GET_CLASS(cell);
  if (class->update_cache)
    return (class->update_cache)(cell, emit_change_signal);
  return FALSE;
}
