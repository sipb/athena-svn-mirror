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

#include <config.h>

#include "ggv-document.h"


GType
ggv_document_get_type (void)
{
        static GType type = 0;
        if (type == 0) {
                static const GTypeInfo info = {
                        sizeof (GgvDocumentClass),
                        NULL,   /* base_init */
                        NULL,   /* base_finalize */
                        NULL,   /* class_init */
                        NULL,   /* class_finalize */
                        NULL,   /* class_data */
                        0,
                        0,      /* n_preallocs */
                        NULL    /* instance_init */
                };
                type = g_type_register_static (G_TYPE_INTERFACE, "GgvDocument", &info, 0);
        }
        return type;
}

gint
ggv_document_get_page_count (GgvDocument *document)
{
        return GGV_DOCUMENT_GET_CLASS (document)->get_page_count (document);
}

gchar **
ggv_document_get_page_names (GgvDocument *document)
{
	return GGV_DOCUMENT_GET_CLASS (document)->get_page_names (document);
}
