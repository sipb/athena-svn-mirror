/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index.h : Top level spec for text indexing
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Rebecca Schulman <rebecka@eazel.com>
 */

#ifndef MEDUSA_TEXT_INDEX_H
#define MEDUSA_TEXT_INDEX_H

#include "medusa-enums.h"
#include "medusa-file-index.h"
#include "medusa-text-index-mime-module.h"

typedef struct MedusaTextIndex            MedusaTextIndex;
typedef struct MedusaTextIndexedLocations MedusaTextIndexedLocations;




/* Create a new index structure */
MedusaTextIndex *        medusa_text_index_new                  (const char *start_index_file,
                                                                 MedusaLogLevel log_level,
                                                                 const char *locations_index_file,
                                                                 const char *semantic_unit_index_file,
                                                                 const char *temp_index_file);
MedusaTextIndex *        medusa_text_index_open                 (const char *start_index_file,
                                                                 MedusaLogLevel log_level,
                                                                 const char *locations_index_file,
                                                                 const char *semantic_unit_index_file);

/* Start the indexing process */
void                     medusa_text_index_read_file            (MedusaTextIndex *index,
                                                                 char *uri,
                                                                 int uri_number,
                                                                 GnomeVFSFileInfo *file_info);

/* Make the temporary file into the full reverse index */
void                     medusa_text_index_finish_indexing      (MedusaTextIndex *index);

/* Add a module that can understand a certain file type */
void                     medusa_text_index_add_mime_module      (MedusaTextIndex *index,
                                                                 MedusaTextIndexMimeModule *module);


void                     medusa_text_index_ref                  (MedusaTextIndex *index);
void                     medusa_text_index_unref                (MedusaTextIndex *index);

void                     medusa_text_index_test                 (void);

#endif /* MEDUSA_TEXT_INDEX_H */



