/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Toolitem (button with dropdown menu) for recent files
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

#ifndef GPDF_RECENT_VIEW_TOOLITEM_H
#define GPDF_RECENT_VIEW_TOOLITEM_H

#include <glib/gmacros.h>

G_BEGIN_DECLS

#include <gtk/gtktogglebutton.h>
#include <recent-files/egg-recent-model.h>

#define GPDF_TYPE_RECENT_VIEW_TOOLITEM            (gpdf_recent_view_toolitem_get_type ())
#define GPDF_RECENT_VIEW_TOOLITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_RECENT_VIEW_TOOLITEM, GPdfRecentViewToolitem))
#define GPDF_RECENT_VIEW_TOOLITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_RECENT_VIEW_TOOLITEM, GPdfRecentViewToolitemClass))
#define GPDF_IS_RECENT_VIEW_TOOLITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_RECENT_VIEW_TOOLITEM))
#define GPDF_IS_RECENT_VIEW_TOOLITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_RECENT_VIEW_TOOLITEM))

typedef struct _GPdfRecentViewToolitem        GPdfRecentViewToolitem;
typedef struct _GPdfRecentViewToolitemClass   GPdfRecentViewToolitemClass;
typedef struct _GPdfRecentViewToolitemPrivate GPdfRecentViewToolitemPrivate;

struct _GPdfRecentViewToolitem {
	GtkToggleButton parent;
	
	GPdfRecentViewToolitemPrivate *priv;
};

struct _GPdfRecentViewToolitemClass {
	GtkToggleButtonClass parent_class;

	void (*item_activate) (GPdfRecentViewToolitem *toolitem, EggRecentItem *item);
};

GType gpdf_recent_view_toolitem_get_type  (void);
void  gpdf_recent_view_toolitem_set_model (GPdfRecentViewToolitem *toolitem, EggRecentModel *model);

G_END_DECLS

#endif /* GPDF_RECENT_VIEW_TOOLITEM_H */
