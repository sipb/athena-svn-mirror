/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF links canvas layer
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

#include <aconf.h>
#include "gpdf-util.h"
#include "gpdf-links-canvas-layer.h"
#include "gpdf-g-switch.h"
#  include <glib/gi18n.h>
#  include <libgnome/gnome-macros.h>
#  include "gpdf-marshal.h"
#include "gpdf-g-switch.h"
#include "Object.h"
#include "Link.h"
#include "gpdf-link-canvas-item.h"

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

BEGIN_EXTERN_C

struct _GPdfLinksCanvasLayerPrivate {
	Links *links;
};

enum {
	LINK_CLICKED,
	LINK_ENTERED,
	LINK_LEAVED, 
	LAST_SIGNAL
};

static guint gpdf_links_canvas_layer_signals [LAST_SIGNAL];

enum {
	PROP_0,
	PROP_LINKS
};

#define PARENT_TYPE GNOME_TYPE_CANVAS_GROUP
GPDF_CLASS_BOILERPLATE (GPdfLinksCanvasLayer, gpdf_links_canvas_layer,
			GnomeCanvasGroup, PARENT_TYPE);

static void
link_clicked_cb (GPdfLinkCanvasItem *link_item, Link *link, gpointer user_data)
{
	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (user_data));

	g_signal_emit (G_OBJECT (user_data),
		       gpdf_links_canvas_layer_signals [LINK_CLICKED], 0,
		       link);	
}

static void
link_entered_cb (GPdfLinkCanvasItem *link_item, Link *link, gpointer user_data)
{
	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (user_data));

	g_signal_emit (G_OBJECT (user_data),
		       gpdf_links_canvas_layer_signals [LINK_ENTERED], 0,
		       link);	
}

static void
link_leaved_cb (GPdfLinkCanvasItem *link_item, Link *link, gpointer user_data)
{
	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (user_data));

	g_signal_emit (G_OBJECT (user_data),
		       gpdf_links_canvas_layer_signals [LINK_LEAVED], 0,
		       link);	
}

static void
gpdf_links_canvas_layer_add_link (GPdfLinksCanvasLayer *links_layer,
				  Link *link)
{
	GnomeCanvasItem *link_item;

	link_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (links_layer),
					   GPDF_TYPE_LINK_CANVAS_ITEM,
					   "link", link,
					   NULL);

	DBG (g_object_set (G_OBJECT (link_item),
			   "fill_color_rgba", 0x0000FFAAU,
			   NULL));

	g_signal_connect (link_item, "clicked",
			  G_CALLBACK (link_clicked_cb), links_layer);
	g_signal_connect (link_item, "enter",
			  G_CALLBACK (link_entered_cb), links_layer);
	g_signal_connect (link_item, "leave",
			  G_CALLBACK (link_leaved_cb), links_layer);
}

static void
gpdf_links_canvas_layer_set_links (GPdfLinksCanvasLayer *links_layer,
				   Links *links)
{
	int i;

	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (links_layer));
	g_return_if_fail (links == NULL
			  || dynamic_cast <Links *> (links) != NULL);

	links_layer->priv->links = links;
	if (links == NULL)
		return;

	for (i = 0; i < links->getNumLinks (); ++i)
		gpdf_links_canvas_layer_add_link (links_layer,
						  links->getLink (i));
}

int
gpdf_links_canvas_layer_get_num_links (GPdfLinksCanvasLayer *links_layer)
{
	g_return_val_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (links_layer), 0);

	return g_list_length (GNOME_CANVAS_GROUP (links_layer)->item_list);
}

static void
gpdf_links_canvas_layer_set_property (GObject *object, guint param_id,
				      const GValue *value, GParamSpec *pspec)
{
	GPdfLinksCanvasLayer *links_layer;

	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (object));

	links_layer = GPDF_LINKS_CANVAS_LAYER (object);

	switch (param_id) {
	case PROP_LINKS:
		gpdf_links_canvas_layer_set_links (
			links_layer,
			reinterpret_cast <Links *> (
				g_value_get_pointer (value)));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_links_canvas_layer_dispose (GObject *object)
{
	GPdfLinksCanvasLayer *links_layer;

	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (object));

	links_layer = GPDF_LINKS_CANVAS_LAYER (object);

	if (links_layer->priv->links) {
		delete links_layer->priv->links;
		links_layer->priv->links = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_links_canvas_layer_finalize (GObject *object)
{
	GPdfLinksCanvasLayer *links_layer;

	g_return_if_fail (GPDF_IS_LINKS_CANVAS_LAYER (object));

	links_layer = GPDF_LINKS_CANVAS_LAYER (object);

	if (links_layer->priv) {
		g_free (links_layer->priv);
		links_layer->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_links_canvas_layer_class_init (GPdfLinksCanvasLayerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gpdf_links_canvas_layer_dispose;
	object_class->finalize = gpdf_links_canvas_layer_finalize;
	object_class->set_property = gpdf_links_canvas_layer_set_property;

	g_object_class_install_property (
		object_class, PROP_LINKS,
		g_param_spec_pointer (
			"links",
			_("Links"),
			_("Links"),
			(GParamFlags)(G_PARAM_WRITABLE)));

	gpdf_links_canvas_layer_signals [LINK_CLICKED] =
		g_signal_new ("link_clicked",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinksCanvasLayerClass,
					       link_clicked),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	gpdf_links_canvas_layer_signals [LINK_ENTERED] =
		g_signal_new ("link_entered",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinksCanvasLayerClass,
					       link_entered),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);

	gpdf_links_canvas_layer_signals [LINK_LEAVED] =
		g_signal_new ("link_leaved",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GPdfLinksCanvasLayerClass,
					       link_leaved),
			      NULL, NULL,
			      gpdf_marshal_VOID__POINTER,
			      G_TYPE_NONE, 1,
			      G_TYPE_POINTER);
}

static void
gpdf_links_canvas_layer_instance_init (GPdfLinksCanvasLayer *links_layer)
{
	links_layer->priv = g_new0 (GPdfLinksCanvasLayerPrivate, 1);
}

END_EXTERN_C
