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
 *  medusa-uri-list-queries.h  Builds and performs common queries on a list of uri's
 *
 */

#ifndef MEDUSA_URI_LIST_QUERIES_H
#define MEDUSA_URI_LIST_QUERIES_H

#include <glib.h>

#include "medusa-uri-list.h"
#include "medusa-rdb-record.h"

gboolean           medusa_uri_list_is_in_directory                     (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_recrd,
                                                                        char *directory_name);
gboolean           medusa_uri_list_is_named                            (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *file_name);
gboolean           medusa_uri_list_has_name_regexp_matching            (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);
gboolean           medusa_uri_list_has_name_not_regexp_matching        (MedusaURIList *uri_list,
                                                                    MedusaRDBRecord current_record,

                                                                    char *string);


gboolean           medusa_uri_list_is_in_directory_regexp_matching     (MedusaURIList *uri_list,

                                                                    MedusaRDBRecord current_record,

                                                                    char *string);


gboolean           medusa_uri_list_is_not_in_directory_regexp_matching (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);


gboolean           medusa_uri_list_has_name_containing                 (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);

gboolean           medusa_uri_list_has_name_not_containing             (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);


gboolean           medusa_uri_list_is_in_directory_containing          (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);


gboolean           medusa_uri_list_has_file_name_starting_with         (MedusaURIList *uri_list,
                                                                        MedusaRDBRecord current_record,
                                                                        char *string);


gboolean           medusa_uri_list_has_file_name_ending_with          (MedusaURIList *uri_list,
                                                                       MedusaRDBRecord current_record,
                                                                       char *string);


gboolean           medusa_uri_list_is_in_directory_tree               (MedusaURIList *uri_list,
                                                                   MedusaRDBRecord current_record,
                                                                   char *string);


gboolean           medusa_uri_list_has_name_glob_matching             (MedusaURIList *uri_list,
                                                                   MedusaRDBRecord current_record,
                                                                   char *string);


gboolean           medusa_uri_list_has_full_file_name                 (MedusaURIList *uri_list,
                                                                   MedusaRDBRecord current_record,
                                                                   char *full_file_name);


#endif /* MEDUSA_URI_LIST_QUERIES_H */


