/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Facade for egg Recent Files
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
#include "gpdf-recent-facade.h"

#include <libgnome/gnome-macros.h>
#include <recent-files/egg-recent-model.h>

#define GPDF_MAX_RECENTS 5

#define PARENT_TYPE G_TYPE_OBJECT
GNOME_CLASS_BOILERPLATE (GPdfRecentFacade, gpdf_recent_facade,
                         GObject, PARENT_TYPE)

struct _GPdfRecentFacadePrivate {
	EggRecentModel *recent_model;
};

#define RECENT_GROUP "gpdf"

static void
impl_add_uri (GPdfRecentFacade *facade, const char *uri)
{
	EggRecentItem *item;

	item = egg_recent_item_new_from_uri (uri);
	egg_recent_item_add_group (item, RECENT_GROUP);
	egg_recent_model_add_full (facade->priv->recent_model, item);
	egg_recent_item_unref (item);
}

void
gpdf_recent_facade_add_uri (GPdfRecentFacade *facade, const char *uri)
{
        GPDF_RECENT_FACADE_GET_CLASS (facade)->add_uri (facade, uri);
}

static const EggRecentModel *
impl_get_model (GPdfRecentFacade *facade)
{
	return facade->priv->recent_model;
}

const EggRecentModel *
gpdf_recent_facade_get_model (GPdfRecentFacade *facade)
{
	return GPDF_RECENT_FACADE_GET_CLASS (facade)->get_model (facade);
}

static void
gpdf_recent_facade_finalize (GObject *object)
{
	GPdfRecentFacade *facade = GPDF_RECENT_FACADE (object);

	if (facade->priv) {
		g_free (facade->priv);
		facade->priv = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_recent_facade_dispose (GObject *object)
{
	GPdfRecentFacade *facade = GPDF_RECENT_FACADE (object);

	if (facade->priv->recent_model) {
		g_object_unref (G_OBJECT (facade->priv->recent_model));
		facade->priv->recent_model = NULL;
	}

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_recent_facade_instance_init (GPdfRecentFacade *facade)
{
	GPdfRecentFacadePrivate *priv;

	facade->priv = priv = g_new0 (GPdfRecentFacadePrivate, 1);
	
	priv->recent_model = egg_recent_model_new (EGG_RECENT_MODEL_SORT_MRU);
	egg_recent_model_set_limit (priv->recent_model, GPDF_MAX_RECENTS);
	egg_recent_model_set_filter_groups (priv->recent_model,
					    RECENT_GROUP, NULL);
}

static void
gpdf_recent_facade_class_init (GPdfRecentFacadeClass *klass)
{
	G_OBJECT_CLASS (klass)->finalize = gpdf_recent_facade_finalize;
	G_OBJECT_CLASS (klass)->dispose = gpdf_recent_facade_dispose;
	klass->add_uri = impl_add_uri;
	klass->get_model = impl_get_model;
}
