/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Process all sorts of incoming URIs
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

#ifndef GPDF_URI_INPUT_H
#define GPDF_URI_INPUT_H

#include <glib-object.h>
#include "gpdf-recent-facade.h"

G_BEGIN_DECLS

#define GPDF_TYPE_URI_INPUT            (gpdf_uri_input_get_type ())
#define GPDF_URI_INPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPDF_TYPE_URI_INPUT, GPdfURIInput))
#define GPDF_URI_INPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GPDF_TYPE_URI_INPUT, GPdfURIInputClass))
#define GPDF_IS_URI_INPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPDF_TYPE_URI_INPUT))
#define GPDF_IS_URI_INPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GPDF_TYPE_URI_INPUT))

typedef struct _GPdfURIInput        GPdfURIInput;
typedef struct _GPdfURIInputClass   GPdfURIInputClass;
typedef struct _GPdfURIInputPrivate GPdfURIInputPrivate;

struct _GPdfURIInput {
        GObject parent;
        
        GPdfURIInputPrivate *priv;
};

struct _GPdfURIInputClass {
        GObjectClass parent_class;

	void (*open_request) (GPdfURIInput *uri_in, const char *uri);
};

GType gpdf_uri_input_get_type          (void);
void  gpdf_uri_input_open_uri          (GPdfURIInput *uri_in, const char *uri);
void  gpdf_uri_input_open_uri_glist    (GPdfURIInput *uri_in, GList *uri_glist);
void  gpdf_uri_input_open_uri_list     (GPdfURIInput *uri_in, const char *uri_list);
void  gpdf_uri_input_open_shell_arg    (GPdfURIInput *uri_in, const char *location);

void  gpdf_uri_input_set_recent_facade (GPdfURIInput *uri_in, GPdfRecentFacade *recent);

G_END_DECLS

#endif /* GPDF_URI_INPUT_H */
