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

#ifndef GPDF_RECENT_FACADE_H
#define GPDF_RECENT_FACADE_H

#include <glib-object.h>
#include <recent-files/egg-recent-model.h>

G_BEGIN_DECLS

#define GPDF_TYPE_RECENT_FACADE            (gpdf_recent_facade_get_type ())
#define GPDF_RECENT_FACADE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_RECENT_FACADE, GPdfRecentFacade))
#define GPDF_RECENT_FACADE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_RECENT_FACADE, GPdfRecentFacadeClass))
#define GPDF_IS_RECENT_FACADE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_RECENT_FACADE))
#define GPDF_IS_RECENT_FACADE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_RECENT_FACADE))
#define GPDF_RECENT_FACADE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GPDF_TYPE_RECENT_FACADE, GPdfRecentFacadeClass))

typedef struct _GPdfRecentFacade        GPdfRecentFacade;
typedef struct _GPdfRecentFacadeClass   GPdfRecentFacadeClass;
typedef struct _GPdfRecentFacadePrivate GPdfRecentFacadePrivate;

struct _GPdfRecentFacade {
        GObject parent;
        
        GPdfRecentFacadePrivate *priv;
};

struct _GPdfRecentFacadeClass {
        GObjectClass parent_class;

	void                  (*add_uri)   (GPdfRecentFacade *facade, const char *uri);
	const EggRecentModel *(*get_model) (GPdfRecentFacade *facade);
};

GType                 gpdf_recent_facade_get_type  (void);
void                  gpdf_recent_facade_add_uri   (GPdfRecentFacade *facade, const char *uri);
const EggRecentModel *gpdf_recent_facade_get_model (GPdfRecentFacade *facade);

#define TYPE_MOCK_RECENT_FACADE            (mock_recent_facade_get_type ())
#define MOCK_RECENT_FACADE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_MOCK_RECENT_FACADE, MockRecentFacade))
#define MOCK_RECENT_FACADE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_MOCK_RECENT_FACADE, MockRecentFacadeClass))
#define IS_MOCK_RECENT_FACADE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_MOCK_RECENT_FACADE))
#define IS_MOCK_RECENT_FACADE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_MOCK_RECENT_FACADE))

typedef struct _MockRecentFacade        MockRecentFacade;
typedef struct _MockRecentFacadeClass   MockRecentFacadeClass;
typedef struct _MockRecentFacadePrivate MockRecentFacadePrivate;

struct _MockRecentFacade {
        GPdfRecentFacade parent;
        
        MockRecentFacadePrivate *priv;
};

struct _MockRecentFacadeClass {
        GPdfRecentFacadeClass parent_class;
};

GType mock_recent_facade_get_type (void);

G_END_DECLS

#endif /* GPDF_RECENT_FACADE_H */
