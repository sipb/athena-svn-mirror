/* GAIL - The GNOME Accessibility Implementation Library
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
#include "gailimage.h"

static void      gail_image_class_init         (GailImageClass *klass);
static void      gail_image_object_init        (GailImage      *image);

static void      atk_image_interface_init      (AtkImageIface  *iface);

static G_CONST_RETURN gchar *
                 gail_image_get_image_description (AtkImage     *image);
static void	 gail_image_get_image_position    (AtkImage     *image,
                                                   gint         *x,
                                                   gint         *y,
                                                   AtkCoordType coord_type);
static void      gail_image_get_image_size     (AtkImage        *image,
                                                gint            *width,
                                                gint            *height);
static gboolean  gail_image_set_image_description (AtkImage     *image,
                                                const gchar     *description);
static void      gail_image_finalize           (GObject         *object);
                                                         
static gpointer parent_class = NULL;

GType
gail_image_get_type (void)
{
  static GType type = 0;

  if (!type)
  {
    static const GTypeInfo tinfo =
    {
      sizeof (GailImageClass),
      (GBaseInitFunc) NULL, /* base init */
      (GBaseFinalizeFunc) NULL, /* base finalize */
      (GClassInitFunc) gail_image_class_init, /* class init */
      (GClassFinalizeFunc) NULL, /* class finalize */
      NULL, /* class data */
      sizeof (GailImage), /* instance size */
      0, /* nb preallocs */
      (GInstanceInitFunc) gail_image_object_init, /* instance init */
      NULL /* value table */
    };

    static const GInterfaceInfo atk_image_info =
    {
        (GInterfaceInitFunc) atk_image_interface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL
    };

    type = g_type_register_static (GAIL_TYPE_WIDGET,
                                   "GailImage", &tinfo, 0);

    g_type_add_interface_static (type, ATK_TYPE_IMAGE,
                                 &atk_image_info);
  }

  return type;
}

static void
gail_image_class_init (GailImageClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gail_image_finalize;
}

static void
gail_image_object_init (GailImage *image)
{
  image->image_description = NULL;
}

AtkObject* 
gail_image_new (GtkWidget *widget)
{
  GObject *object;
  AtkObject *accessible;

  g_return_val_if_fail (GTK_IS_IMAGE (widget), NULL);

  object = g_object_new (GAIL_TYPE_IMAGE, NULL);

  accessible = ATK_OBJECT (object);
  atk_object_initialize (accessible, widget);

  accessible->role = ATK_ROLE_ICON;

  return accessible;
}

static void
atk_image_interface_init (AtkImageIface *iface)
{
  g_return_if_fail (iface != NULL);

  iface->get_image_description = gail_image_get_image_description;
  iface->get_image_position = gail_image_get_image_position;
  iface->get_image_size = gail_image_get_image_size;
  iface->set_image_description = gail_image_set_image_description;
}

static G_CONST_RETURN gchar * 
gail_image_get_image_description (AtkImage     *image)
{
  GailImage* aimage = GAIL_IMAGE (image);

  return aimage->image_description;
}

static void
gail_image_get_image_position (AtkImage     *image,
                               gint         *x,
                               gint         *y,
                               AtkCoordType coord_type)
{
  atk_component_get_position (ATK_COMPONENT (image), x, y, coord_type);
}

static void
gail_image_get_image_size (AtkImage *image, 
                           gint     *width,
                           gint     *height)
{
  GtkWidget* widget;
  GtkImage *gtk_image;
  GtkImageType image_type;

  widget = GTK_ACCESSIBLE (image)->widget;
  if (widget == 0)
  {
    /* State is defunct */
    *height = -1;
    *width = -1;
    return;
  }

  gtk_image = GTK_IMAGE(widget);

  image_type = gtk_image_get_storage_type(gtk_image);
 
  switch(image_type) {
    case GTK_IMAGE_PIXMAP:
    {	
      GdkPixmap *pixmap;
      gtk_image_get_pixmap(gtk_image, &pixmap, NULL);
      gdk_window_get_size (pixmap, width, height);
      break;
    }
    case GTK_IMAGE_PIXBUF:
    {
      GdkPixbuf *pixbuf;
      pixbuf = gtk_image_get_pixbuf(gtk_image);
      *height = gdk_pixbuf_get_height(pixbuf);
      *width = gdk_pixbuf_get_width(pixbuf);
      break;
    }
    case GTK_IMAGE_IMAGE:
    {
      GdkImage *gdk_image;
      gtk_image_get_image(gtk_image, &gdk_image, NULL);
      *height = gdk_image->height;
      *width = gdk_image->width;
      break;
    }
    case GTK_IMAGE_STOCK:
    {
      GtkIconSize size;
      gtk_image_get_stock(gtk_image, NULL, &size);
      gtk_icon_size_lookup(size, width, height);
      break;
    }
    case GTK_IMAGE_ICON_SET:
    {
      GtkIconSize size;
      gtk_image_get_icon_set(gtk_image, NULL, &size);
      gtk_icon_size_lookup(size, width, height);
      break;
    }
    case GTK_IMAGE_ANIMATION:
    {
      GdkPixbufAnimation *animation;
      animation = gtk_image_get_animation(gtk_image);
      *height = gdk_pixbuf_animation_get_height(animation);
      *width = gdk_pixbuf_animation_get_width(animation);
      break;
    }
    default:
    {
      *height = -1;
      *width = -1;
      break;
    }
  }
}

static gboolean
gail_image_set_image_description (AtkImage     *image,
                                  const gchar  *description)
{
  GailImage* aimage = GAIL_IMAGE (image);

  g_free (aimage->image_description);
  aimage->image_description = g_strdup (description);
  return TRUE;
}

static void
gail_image_finalize (GObject      *object)
{
  GailImage *aimage = GAIL_IMAGE (object);

  g_free (aimage->image_description);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}
