/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/*
 *  Medusa
 *
 *  medusa-text-index.c : Do the text indexing of the file system
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

#include <glib.h>
#include <string.h>
#include <unistd.h>

#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-directory.h>

#include "medusa-conf.h"
#include "medusa-enums.h"
#include "medusa-file-index.h"
#include "medusa-io-handler.h"
#include "medusa-hash.h"
#include "medusa-text-index.h"
#include "medusa-text-index-private.h"
#include "medusa-text-index-queries.h"
#include "medusa-text-index-plaintext-module.h"



#define MEDUSA_TEXT_INDEX_TEMP_FILE_MAGIC_NUMBER         "9124"
#define MEDUSA_TEXT_INDEX_TEMP_FILE_VERSION_NUMBER       "0.1"
#define MEDUSA_TEXT_INDEX_START_INDEX_MAGIC_NUMBER       "9125"
#define MEDUSA_TEXT_INDEX_START_INDEX_VERSION_NUMBER     "0.1"
#define MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_MAGIC_NUMBER   "9126"
#define MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_VERSION_NUMBER "0.1"

#define LOG_OUTPUT(args) \
if (text_index->log_level == MEDUSA_DB_LOG_EVERYTHING) { \
        printf args; \
}

#define ABBREV_LOG_OUTPUT(args) \
if (text_index->log_level == MEDUSA_DB_LOG_EVERYTHING || \
    text_index->log_level == MEDUSA_DB_LOG_ABBREVIATED) { \
        printf args; \
}

#define TEXT_INDEX_LOG_OUTPUT(args) \
if (text_index->log_level == MEDUSA_DB_LOG_TEXT_INDEX_DATA) { \
        printf args; \
}


/* Before creating a text index we want to be sure that we aren't overwriting 
   anything when we make new files */
static gboolean             permenant_text_index_files_exist          (const char *start_index_file,
                                                                       const char *locations_index_file,
                                                                       const char *semantic_units_index_file);
static gboolean             text_index_files_dont_exist               (const char *start_index_file,
                                                                       const char *locations_index_file,
                                                                       const char *semantic_units_index_file,
                                                                       const char *temp_index_file);
/* ie text index structure is set up, and all files were
   found or made successfully */
static gboolean             text_index_files_are_ready                (MedusaTextIndex *text_index);
static char *               make_temp_index_file_name                 (const char *temp_index_file, 
                                                                       int file_number);
static void                 array_of_words_free                       (char **array, 
                                                                       int number_of_words);
static void                 text_index_add_mime_modules               (MedusaTextIndex *text_index);

static void                 setup_temp_index_io_handlers              (MedusaTextIndex *text_index);
static void                 sort_temp_index_data_into_permenant_index (MedusaTextIndex *text_index,
                                                                       int temp_index_number);
static void                 add_word_to_real_index                    (gpointer key,
                                                                       gpointer value,
                                                                       gpointer user_data);

static gint32               get_uri_number_from_temp_index_cell   (MedusaTextIndex *text_index,
                                                                   int cell_number,
                                                                   int temp_index_number);
static gint32               get_last_cell_from_temp_index_cell    (MedusaTextIndex *text_index,
                                                                   int cell_number,
                                                                   int temp_index_number);

static void                 write_start_location_to_start_file    (MedusaTextIndex *text_index,
                                                                   MedusaToken word_token,
                                                                   gint32 offset);
static void                 write_end_location_to_start_file      (MedusaTextIndex *text_index,
                                                                   MedusaToken word_token,
                                                                   gint32 offset);
static void                 write_uri_number_to_location_file     (MedusaTextIndex *text_index,
                                                                   gint32 uri_number);
                                                                   
                                                                   
static void                 medusa_text_index_destroy             (MedusaTextIndex *text_index);

MedusaTextIndex *
medusa_text_index_open (const char *start_index_file,
                        MedusaLogLevel log_level,
                        const char *locations_index_file,
                        const char *semantic_units_index_file)
{
        MedusaTextIndex *text_index;
        MedusaVersionedFileResult start_index_open_result, locations_index_open_result;

        g_return_val_if_fail (permenant_text_index_files_exist (start_index_file,
                                                                locations_index_file,
                                                                semantic_units_index_file), NULL);

        text_index = g_new0 (MedusaTextIndex, 1);

        text_index->creating_index = FALSE;
        text_index->semantic_units = medusa_hash_open (semantic_units_index_file, 
                                                       SEMANTIC_UNITS_BITS);
        text_index->start_index_name = g_strdup (start_index_file);
        text_index->start_index = medusa_versioned_file_open (start_index_file,
                                                              MEDUSA_TEXT_INDEX_START_INDEX_MAGIC_NUMBER,
                                                              MEDUSA_TEXT_INDEX_START_INDEX_VERSION_NUMBER,
                                                              &start_index_open_result);
        text_index->locations_index_name = g_strdup (locations_index_file);
        text_index->locations_index = medusa_versioned_file_open (locations_index_file,
                                                                  MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_MAGIC_NUMBER,
                                                                  MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_VERSION_NUMBER,
                                                                  &locations_index_open_result);
        text_index->log_level = log_level;
        text_index->ref_count = 1;

        if (start_index_open_result != MEDUSA_VERSIONED_FILE_OK) {
                ABBREV_LOG_OUTPUT (("Problems opening medusa start index with result %d\n", start_index_open_result));
                medusa_text_index_unref (text_index);
                return NULL;
        }
        if (locations_index_open_result != MEDUSA_VERSIONED_FILE_OK) {
                ABBREV_LOG_OUTPUT (("Problems opening medusa locations index with result %d\n", locations_index_open_result));
                medusa_text_index_unref (text_index);
                return NULL;
        }
        return text_index;

}

MedusaTextIndex *
medusa_text_index_new (const char *start_index_file,
                       MedusaLogLevel log_level,
                       const char *locations_index_file,
                       const char *semantic_units_index_file,
                       const char *temp_index_file)
{
        MedusaTextIndex *text_index;
        int zero, i;
        MedusaVersionedFileResult start_index_create_result, locations_index_create_result;
        
        g_return_val_if_fail (text_index_files_dont_exist (start_index_file,
                                                           locations_index_file,
                                                           semantic_units_index_file,
                                                           temp_index_file), NULL);
        
        text_index = g_new0 (MedusaTextIndex, 1);
        text_index->creating_index = TRUE;

        text_index->semantic_units = medusa_hash_new (semantic_units_index_file, 
                                                      SEMANTIC_UNITS_BITS);
        text_index->last_occurrence = g_hash_table_new (g_direct_hash,
                                                        g_direct_equal);


        zero = 0;
        /* Assume for now that if a location index file already exists,
           we are just here to read.  If it doesn't we are here to
           make an index, so set up the temporary index hardware */
        text_index->temp_indices_are_memory_mapped = FALSE;

        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                text_index->temp_index_name[i] = make_temp_index_file_name (temp_index_file, i);
                text_index->temp_index_stream[i] = 
                        fopen_new_with_medusa_io_handler_header (text_index->temp_index_name[i],
                                                                 MEDUSA_TEXT_INDEX_TEMP_FILE_MAGIC_NUMBER,
                                                                 MEDUSA_TEXT_INDEX_TEMP_FILE_VERSION_NUMBER);
                /* Write initial cell, so that no one uses cell 0 */
                fwrite (&zero,
                        sizeof (int),
                        1,
                        text_index->temp_index_stream[i]);
                        fwrite (&zero,
                                sizeof (int), 
                                1,
                                text_index->temp_index_stream[i]);
                        /* Start with cell number 1 */
                        text_index->current_cell_number[i] = 1;
        }

        text_index->start_index_name = g_strdup (start_index_file);
        text_index->start_index = medusa_versioned_file_create (start_index_file,
                                                                MEDUSA_TEXT_INDEX_START_INDEX_MAGIC_NUMBER,
                                                                MEDUSA_TEXT_INDEX_START_INDEX_VERSION_NUMBER,
                                                                &start_index_create_result);
        if (start_index_create_result != MEDUSA_VERSIONED_FILE_OK) {
                ABBREV_LOG_OUTPUT (("Could not create start index file with error code %d\n", start_index_create_result));
        }
        text_index->reverse_index_position = 1;
        

        text_index->locations_index_name = g_strdup (locations_index_file);
        text_index->locations_index = medusa_versioned_file_create (locations_index_file,
                                                                    MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_MAGIC_NUMBER,
                                                                    MEDUSA_TEXT_INDEX_LOCATIONS_INDEX_VERSION_NUMBER,
                                                                    &locations_index_create_result);
        if (locations_index_create_result != MEDUSA_VERSIONED_FILE_OK) {
                ABBREV_LOG_OUTPUT (("Could not create locations index file with error code %d\n", locations_index_create_result));
        }

        medusa_versioned_file_write (text_index->locations_index, &zero, sizeof (gint32), 1);
        text_index_add_mime_modules (text_index);

        text_index->log_level = log_level;
        text_index->ref_count = 1;

        if (start_index_create_result != MEDUSA_VERSIONED_FILE_OK ||
            locations_index_create_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_text_index_unref (text_index);
                return NULL;
        }
                
        if (text_index_files_are_ready (text_index) == FALSE) {
                g_warning ("Cannot open text index for reading\n");
                medusa_text_index_unref (text_index);
                return NULL;
        }


        return text_index;
}

static gint32                  
get_uri_number_from_temp_index_cell (MedusaTextIndex *text_index,
                                     int cell_number,
                                     int temp_index_number)
{
        gint32 uri_number;
        char * data_region;
        gpointer cell_location;

        data_region = medusa_io_handler_get_data_region (text_index->temp_index_io_handler[temp_index_number]);
        cell_location = data_region + 2 * cell_number * sizeof (gint32);
        memcpy (&uri_number, cell_location, sizeof (gint32));

        return uri_number;

}
static gint32                  
get_last_cell_from_temp_index_cell (MedusaTextIndex *text_index,
                                    int cell_number,
                                    int temp_index_number)
{
        gint32 last_cell_number;
        char *data_region; 
        gpointer cell_location;

        data_region = medusa_io_handler_get_data_region (text_index->temp_index_io_handler[temp_index_number]);
        cell_location = data_region + (2 * cell_number + 1) * sizeof (gint32);
        memcpy (&last_cell_number, cell_location, sizeof (gint32));
        return last_cell_number;

}

static char *
make_temp_index_file_name (const char *temp_index_file, 
                           int file_number)
{
        return g_strdup_printf ("%s.%d", temp_index_file, file_number);
}

static gboolean
permenant_text_index_files_exist (const char *start_index_file,
                                  const char *locations_index_file,
                                  const char *semantic_units_index_file)
{
       if (access (start_index_file, X_OK) == -1) {
                return FALSE;
        }
        if (access (locations_index_file, X_OK) == -1) {
                return FALSE;
        }
        if (access (semantic_units_index_file, X_OK) == -1) {
                return FALSE;
        }
 
        return TRUE;
}

static gboolean             
text_index_files_dont_exist (const char *start_index_file,
                             const char *locations_index_file,
                             const char *semantic_units_index_file,
                             const char *temp_index_file)
{
        int i;
        char *one_temp_index_name;

        if (permenant_text_index_files_exist (start_index_file,
                                              locations_index_file,
                                              semantic_units_index_file)) {
                return FALSE;
        }
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                one_temp_index_name = make_temp_index_file_name (temp_index_file, i); 
                if (access (one_temp_index_name, X_OK) != -1) {
                        return FALSE;
                }
                g_free (one_temp_index_name);
        }

        return TRUE;
}

static gboolean
text_index_files_are_ready (MedusaTextIndex *text_index)
{
        /* Make sure the indexes are valid */
        /* FIXME bugzilla.eazel.com 2994: 
           Should there be a check for temp index stuff here? */
        return  text_index->semantic_units != NULL &&
                text_index->last_occurrence != NULL &&
                text_index->start_index != NULL &&
                text_index->locations_index != NULL;

}



void
medusa_text_index_read_file (MedusaTextIndex *text_index,
                             char *uri,
                             int uri_number,
                             GnomeVFSFileInfo *file_info)
{
        MedusaTextIndexMimeModule *module;
        MedusaTextParsingFunc read_words;
        MedusaToken word_token;
        char **results;
        int i, number_of_words;
        gint32 last_cell_number;

        g_return_if_fail (text_index != NULL);
        LOG_OUTPUT (("Trying to index file %s with mime_type %s\n", uri, file_info->mime_type));
        module = medusa_text_index_mime_module_first_valid_module (text_index->mime_modules,
                                                                   file_info->mime_type);
        if (module == NULL) {
                return;
        }
        
        read_words = medusa_text_index_mime_module_get_parser (module);
        number_of_words = read_words (uri,
                                      &results,
                                      (gpointer) NULL);

        for (i = 0; i < number_of_words; i++) {
                /* Find the last location where we recorded
                   information about this word in the temp
                   index */
                g_assert (results[i] != NULL);

                word_token = medusa_string_to_token (text_index->semantic_units,
                                                     results[i]);
                last_cell_number = GPOINTER_TO_INT(g_hash_table_lookup (text_index->last_occurrence,
                                                                        GINT_TO_POINTER (word_token)));

                fwrite (&uri_number,
                        sizeof (gint32),
                        1,
                        text_index->temp_index_stream[word_token % NUMBER_OF_TEMP_INDEXES]);
                fwrite (&last_cell_number,
                        sizeof (gint32),
                        1,
                        text_index->temp_index_stream[word_token % NUMBER_OF_TEMP_INDEXES]);
                /* FIXME bugzilla.eazel.com 2995: 
                   Should lower case all additions to the text index */
                /* g_strdown (results[i]);*/
                g_hash_table_insert (text_index->last_occurrence,
                                     GINT_TO_POINTER (word_token),
                                     GINT_TO_POINTER (text_index->current_cell_number[word_token % NUMBER_OF_TEMP_INDEXES]));
                text_index->current_cell_number[word_token % NUMBER_OF_TEMP_INDEXES]++;
                
        }
        TEXT_INDEX_LOG_OUTPUT (("%s\t%s\t%d\t%d\n",uri,file_info->mime_type, number_of_words, (int) file_info->size));
        /* Free the array itself and its word contents */
        array_of_words_free (results, number_of_words);

}

void
medusa_text_index_finish_indexing (MedusaTextIndex *text_index)
{
        unsigned int i;
        
        /* two guint 32 integers per cell */
        medusa_versioned_file_append_zeros (text_index->start_index, 
                                           sizeof (gint32) *
                                           (1 << (text_index->semantic_units->key_bits)) * 2);
        ABBREV_LOG_OUTPUT (("Appending %d bytes to the start_index\n",
                            (int)((1 << (text_index->semantic_units->key_bits)) * 2 * sizeof (gint32))));
        /* We memory map the temporary indices for this section
           of the processing, since there is high locality of reference */
        setup_temp_index_io_handlers (text_index);
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                /* We can filter each small index, serially,
                   which cuts down greatly on memory usage */
                sort_temp_index_data_into_permenant_index (text_index, i);
        }
}


static void
setup_temp_index_io_handlers (MedusaTextIndex *index)
{
        int i;
        /* First close all of the temporary index file pointers */
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                if (fclose (index->temp_index_stream[i]) == -1) {
                        g_warning ("Could not close temp index file %s\n", index->temp_index_name[i]);
                }
                index->temp_index_stream[i] = NULL;
        }

        /* Now create io handlers for everything */
        index->temp_indices_are_memory_mapped = TRUE;
        for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                index->temp_index_io_handler[i] = medusa_io_handler_open (index->temp_index_name[i],
                                                                          MEDUSA_TEXT_INDEX_TEMP_FILE_MAGIC_NUMBER,
                                                                          MEDUSA_TEXT_INDEX_TEMP_FILE_VERSION_NUMBER);
        }
}

static void                 
sort_temp_index_data_into_permenant_index (MedusaTextIndex *index,
                                           int temp_index_number)
{
        int j;
        char *next_word;
        
        for (j = temp_index_number; j < (1 << index->semantic_units->key_bits); j += NUMBER_OF_TEMP_INDEXES) {
                next_word = medusa_token_to_string (index->semantic_units, j);
                if (strlen (next_word)) {
                                /* FIXME bugzilla.eazel.com 2996: 
                                   This function needs a better signature, but for now
                                   i think using the function in this way will be ok */
                        add_word_to_real_index (GINT_TO_POINTER (j),
                                                g_hash_table_lookup (index->last_occurrence,
                                                                     GINT_TO_POINTER (j)),
                                                index);
                }
                
        }

}


static void
add_word_to_real_index (gpointer key,
                        gpointer value,
                        gpointer user_data)
{
        MedusaTextIndex *text_index;
        char *word;
        MedusaToken word_token;
        int last_cell_number;
        gint32 uri_number;

        word_token = GPOINTER_TO_UINT (key);
        last_cell_number = GPOINTER_TO_INT (value);
        text_index = (MedusaTextIndex *) user_data;
        word = medusa_token_to_string (text_index->semantic_units, word_token);
        LOG_OUTPUT (("going on to word %s at cell number %d\n", word, last_cell_number));
        write_start_location_to_start_file (text_index, word_token, text_index->reverse_index_position);
        LOG_OUTPUT (("inserting starting point for word %s (token %d) at position %d\n",
                     word, word_token, text_index->reverse_index_position));
        uri_number = get_uri_number_from_temp_index_cell (text_index, last_cell_number, word_token % NUMBER_OF_TEMP_INDEXES);
        while (uri_number > 0) {

                last_cell_number = get_last_cell_from_temp_index_cell (text_index, 
                                                                       last_cell_number,
                                                                       word_token % NUMBER_OF_TEMP_INDEXES);

                LOG_OUTPUT (("Next occurrence of word %s is cell number %d, uri number %d\n", 
                             word, last_cell_number, uri_number));
                write_uri_number_to_location_file (text_index, uri_number);
                text_index->reverse_index_position++;
                uri_number = get_uri_number_from_temp_index_cell (text_index, 
                                                                  last_cell_number, 
                                                                  word_token % NUMBER_OF_TEMP_INDEXES);
                
        }
        LOG_OUTPUT (("inserting ending point for word %s (token %d) at position %d\n",
                    word, word_token, text_index->reverse_index_position));
        write_end_location_to_start_file (text_index, word_token, text_index->reverse_index_position);
        
}

/* FIXME: These three functions should return codes in case of error */        
static void                 
write_start_location_to_start_file (MedusaTextIndex *text_index,
                                       MedusaToken word_token,
                                       gint32 offset)
{
        MedusaVersionedFileResult seek_result, write_result;
        seek_result = medusa_versioned_file_seek (text_index->start_index,
                                                  2 * word_token * sizeof (gint32));
        if (seek_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Error occurred while writing start location to start index file",
                                                    seek_result);
        }
        write_result = medusa_versioned_file_write (text_index->start_index,
                                                    &offset,
                                                    sizeof (gint32),
                                                    1);
        if (write_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Error occurred while writing start location to start index file",
                                                    write_result);
        }
        
}


static void                 
write_end_location_to_start_file   (MedusaTextIndex *text_index,
                                    MedusaToken word_token,
                                    gint32 offset)
{
        MedusaVersionedFileResult seek_result, write_result;

        seek_result = medusa_versioned_file_seek (text_index->start_index,
                                                  (2 * word_token + 1) * sizeof (gint32));
        if (seek_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Error occurred while writing end location to start index file", 
                                                    seek_result);
                return;
        }
        write_result = medusa_versioned_file_write (text_index->start_index,
                                                    &offset,
                                                    sizeof (gint32),
                                                    1);
        if (write_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Error occurred while writing end location to start index file", 
                                                    write_result);
                return;
        }
}

static void                 
write_uri_number_to_location_file (MedusaTextIndex *text_index,
                                   gint32 uri_number)
{
        MedusaVersionedFileResult result;
        result = medusa_versioned_file_write (text_index->locations_index, &uri_number, sizeof (gint32), 1);
        if (result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Error occurred while writing uri number to location file",
                                                    result);
        }
        
}


gint32
medusa_text_index_read_start_location_from_start_file (MedusaTextIndex *text_index,
                                                       MedusaToken word_token)
                  
{
        gint32 offset;
        MedusaVersionedFileResult seek_result, read_result;

        seek_result = medusa_versioned_file_seek (text_index->start_index,
                                                  2 * word_token * sizeof (gint32));
        if (seek_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Could not go to start location from file", seek_result);
                return -1;
        }
        read_result = medusa_versioned_file_read (text_index->start_index,
                                                  &offset,
                                                  sizeof (gint32),
                                                  1);
        if (read_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Could not read start location from file", read_result);
                return -1;
        }
        return offset;
}


gint32
medusa_text_index_read_end_location_from_start_file (MedusaTextIndex *text_index,
                                                     MedusaToken word_token)
                  
{
        gint32 offset;
        MedusaVersionedFileResult seek_result, read_result;

        seek_result = medusa_versioned_file_seek (text_index->start_index,
                                                  (2 * word_token + 1)  * sizeof (gint32));
        if (seek_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Could not go to end location from text start index file", seek_result);
                return -1;
        }
        read_result = medusa_versioned_file_read (text_index->start_index,
                                                  &offset,
                                                  sizeof (gint32),
                                                  1);
        if (read_result != MEDUSA_VERSIONED_FILE_OK) {
                medusa_versioned_file_error_notify ("Could not read end location from text start index file.", read_result);
                return -1;
        }
        return offset;

}

static void
array_of_words_free (char **array, int number_of_words)
{
        int i;
        for (i = 0; i < number_of_words; i++) {
                g_free (array[i]);
        }
        if (number_of_words > 0) {
                g_free (array);
        }
}

void
medusa_text_index_ref (MedusaTextIndex *text_index)
{
        text_index->ref_count++;
}

void
medusa_text_index_unref (MedusaTextIndex *text_index)
{
        g_assert (text_index->ref_count > 0);
        if (text_index->ref_count == 1) {
                medusa_text_index_destroy (text_index);
        }
        else {
                text_index->ref_count--;
        }
}

static void
text_index_add_mime_modules (MedusaTextIndex *text_index)
{
        MedusaTextIndexMimeModule *plaintext_indexer;
       
        /* Enable indexing of plain text files */
        plaintext_indexer = medusa_text_index_mime_module_new (medusa_text_index_parse_plaintext);
        medusa_text_index_mime_module_add_mime_pattern (plaintext_indexer,
                                                        "text/");
        /* bugzilla.eazel.com bug 1690:
           add option to index source code here */
        text_index->mime_modules = g_list_prepend (text_index->mime_modules,
                                                   plaintext_indexer);
        

}



static void
medusa_text_index_destroy (MedusaTextIndex *text_index)
{
        int i;

        medusa_hash_unref (text_index->semantic_units);

        if (text_index->creating_index) {
                /* Don't need to free the data in the hash table
                   because it is int to int */
                g_hash_table_destroy (text_index->last_occurrence);
                for (i = 0; i < NUMBER_OF_TEMP_INDEXES; i++) {
                        g_free (text_index->temp_index_name[i]);
                        if (text_index->temp_indices_are_memory_mapped) {
                                medusa_io_handler_free (text_index->temp_index_io_handler[i]);
                        }
                        else {
                                /* We exited early, close the file pointers */
                                fclose (text_index->temp_index_stream[i]);
                        }

                }
                g_list_foreach (text_index->mime_modules, 
                               medusa_text_index_mime_module_unref_cover,
                                NULL);
                g_list_free (text_index->mime_modules);
        }

        g_free (text_index->start_index_name);
        medusa_versioned_file_destroy (text_index->start_index);
        
        g_free (text_index->locations_index_name);
        medusa_versioned_file_destroy (text_index->locations_index);

        g_free (text_index);
}



void
medusa_text_index_test (void)
{


}
