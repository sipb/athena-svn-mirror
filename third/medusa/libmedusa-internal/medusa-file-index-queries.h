/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
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
 *
 *  medusa-file-index-queries.h  Builds and performs common queries on a file index
 *
 */

#ifndef MEDUSA_FILE_INDEX_QUERIES_H
#define MEDUSA_FILE_INDEX_QUERIES_H

#include "medusa-file-index.h"

gboolean medusa_file_index_is_of_type                         (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               const char         *type);
gboolean medusa_file_index_is_not_of_type                     (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               const char         *type);
gboolean medusa_file_index_is_file                            (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               gpointer            arg);
gboolean medusa_file_index_is_text_file                       (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               gpointer            arg);
gboolean medusa_file_index_is_application                     (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               gpointer            arg);
gboolean medusa_file_index_is_directory                       (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               gpointer            arg);
gboolean medusa_file_index_is_music                           (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               gpointer            arg);

gboolean medusa_file_index_has_uid                            (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 uid);
gboolean medusa_file_index_does_not_have_uid                  (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 uid);
gboolean medusa_file_index_has_gid                            (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 gid);
gboolean medusa_file_index_does_not_have_gid                  (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 gid);

/* FIXME bugzilla.eazel.com 2649: 
   These should take time_t, not ints */

gboolean medusa_file_index_is_modified_before_time            (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 unix_time);
gboolean medusa_file_index_is_modified_after_time             (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 unix_time);
gboolean medusa_file_index_uid_can_read_file                  (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 uid);
gboolean medusa_file_index_is_larger_than                     (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 size);
gboolean medusa_file_index_is_smaller_than                    (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 size);
gboolean medusa_file_index_is_of_size                         (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               int                 size);
gboolean medusa_file_index_marked_with_keyword                (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               const char         *keyword);
gboolean medusa_file_index_not_marked_with_keyword            (MedusaFileSystemDB *file_system_db,
                                                               MedusaRDBRecord     record,
                                                               const char         *keyword);

#endif /* MEDUSA_FILE_INDEX_QUERIES_H */
