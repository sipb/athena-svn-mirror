/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Mock class for GPdfRecentFacade
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

#define PARENT_TYPE GPDF_TYPE_RECENT_FACADE
GNOME_CLASS_BOILERPLATE (MockRecentFacade, mock_recent_facade,
                         GPdfRecentFacade, PARENT_TYPE)

static void
impl_add_uri (GPdfRecentFacade *facade, const char *uri)
{
	g_object_set_data_full (G_OBJECT (facade), "added_uri",
				g_strdup (uri), g_free);
}

static void
mock_recent_facade_instance_init (MockRecentFacade *facade)
{
}

static void
mock_recent_facade_class_init (MockRecentFacadeClass *klass)
{
	GPDF_RECENT_FACADE_CLASS (klass)->add_uri = impl_add_uri;
}
