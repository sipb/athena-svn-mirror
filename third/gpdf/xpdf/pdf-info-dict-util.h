/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* PDF Info Dictionary helper functions
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

#ifndef PDF_INFO_DICT_UTIL_H
#define PDF_INFO_DICT_UTIL_H

#include "gpdf-g-switch.h"
#  include <glib.h>
#  include <glib-object.h>
#include "gpdf-g-switch.h"
#include <time.h>
#include "Object.h"
#include "PDFDoc.h"

G_BEGIN_DECLS

void       pdf_doc_process_properties (PDFDoc *pdf_doc, GObject *target);
gchar     *pdf_info_dict_get_string   (Dict *info_dict, const gchar *key);
struct tm *pdf_info_dict_get_date     (Dict *info_dict, const gchar *key);

G_END_DECLS

#endif /* PDF_INFO_DICT_UTIL_H */
