/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Interface GgvDocument
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GGV_DOCUMENT_H
#define GGV_DOCUMENT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define GGV_DOCUMENT_TYPE             (ggv_document_get_type ())
#define GGV_DOCUMENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGV_DOCUMENT_TYPE, GgvDocument))
#define GGV_DOCUMENT_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), GGV_DOCUMENT_TYPE, GgvDocumentClass))
#define GGV_IS_DOCUMENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGV_DOCUMENT_TYPE))
#define GGV_IS_DOCUMENT_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), GGV_DOCUMENT_TYPE))
#define GGV_DOCUMENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), GGV_DOCUMENT_TYPE, GgvDocumentClass))

typedef struct _GgvDocument        GgvDocument;
typedef struct _GgvDocumentClass   GgvDocumentClass;

struct _GgvDocumentClass {
	GTypeInterface parent;

	gint    (*get_page_count) (GgvDocument *document);
	gchar **(*get_page_names) (GgvDocument *document);
};

GType   ggv_document_get_type       (void);
gint    ggv_document_get_page_count (GgvDocument *document);
gchar **ggv_document_get_page_names (GgvDocument *document);

G_END_DECLS

#endif /* GGV_DOCUMENT_H */
