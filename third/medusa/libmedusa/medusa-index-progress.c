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
 *  medusa-index-progress.c -- Structures that keeps track of the
 *  total amount of disk space there is to index (or search)
 *  and reports back through a callback how much has been done.
 */

#include <config.h>
#include <fcntl.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_GETMNTINFO
# undef MAX
# undef MIN
# include <sys/param.h>
# include <sys/ucred.h>
# include <sys/mount.h>
#elif defined(HAVE_GETMNTENT)
# include <mntent.h>
#else
#warning "Can't find a valid mount function type"
#endif

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif

#include "medusa-index-progress.h"

#define INDEX_PROGRESS_FILE_NAME "/tmp/index-progress"

struct MedusaIndexProgress {
        gboolean index_progress_is_valid;
        size_t total_blocks_to_index;
        size_t block_size;
        size_t blocks_indexed_so_far;
        int percent_complete;
        int output_fd;
};

static gboolean             get_disk_used_information                  (char *root_directory,
                                                                        size_t *total_blocks_to_index,
                                                                        size_t *block_size);
static gboolean             update_new_public_percent_complete         (MedusaIndexProgress *progress,
                                                                        int new_percent_complete);

void
medusa_index_progress_file_clear (void)
{
        unlink (INDEX_PROGRESS_FILE_NAME);
}


MedusaIndexProgress *         
medusa_index_progress_new (char *root_directory)
{
        size_t total_blocks_to_index, block_size;
        int output_fd;
        MedusaIndexProgress *index_progress;
        gboolean disk_information_was_found;

        disk_information_was_found = get_disk_used_information (root_directory, 
                                                                &total_blocks_to_index, 
                                                                &block_size);
        g_return_val_if_fail (disk_information_was_found, NULL);
        output_fd = open (INDEX_PROGRESS_FILE_NAME, O_RDWR | O_CREAT | O_EXCL | O_SYNC);
        g_return_val_if_fail (output_fd != -1, NULL);

        chmod (INDEX_PROGRESS_FILE_NAME, S_IRWXU | S_IRWXG | S_IRWXO);

        index_progress = g_new0 (MedusaIndexProgress, 1);
        index_progress->total_blocks_to_index = total_blocks_to_index;
        index_progress->block_size = block_size;
        if (index_progress->total_blocks_to_index > 0) {
                g_print ("Total blocks to index is %d\n", index_progress->total_blocks_to_index);
                index_progress->index_progress_is_valid = TRUE;
        }
        else {
                g_warning ("Can't find the amount of disk space on your drive.  We won't be recording index progress variables\n");
                index_progress->index_progress_is_valid = FALSE;
        }
        index_progress->blocks_indexed_so_far = 0;
        index_progress->output_fd = output_fd;
        index_progress->percent_complete = 0;

        if (update_new_public_percent_complete (index_progress, index_progress->percent_complete) == FALSE) {
                medusa_index_progress_destroy (index_progress);
                return NULL;
        }
        return index_progress;
}


void
medusa_index_progress_update (MedusaIndexProgress *progress,
                              size_t new_bytes_indexed)
{
        int new_percent_complete;

        g_return_if_fail (progress != NULL);
        
        /* Don't calculate progress if we couldn't establish a total amount of space to index */
        if (!progress->index_progress_is_valid) {
                return;
        }

        if (new_bytes_indexed % progress->block_size) {
                progress->blocks_indexed_so_far += new_bytes_indexed / progress->block_size + 1;
        }
        else {
                progress->blocks_indexed_so_far += new_bytes_indexed / progress->block_size;
        }
                
        new_percent_complete = progress->blocks_indexed_so_far * 100 / progress->total_blocks_to_index;
        /* Don't update progress above 100% */
        if (new_percent_complete > 100) {
                return;
        }
        if (new_percent_complete > progress->percent_complete) {
                if (update_new_public_percent_complete (progress, 
                                                        new_percent_complete) == FALSE) {
                        g_warning ("Could not update progress successfully\n");
                }
                progress->percent_complete = new_percent_complete;
        }
}

void
medusa_index_progress_destroy (MedusaIndexProgress *progress)
{
        close (progress->output_fd);
        unlink (INDEX_PROGRESS_FILE_NAME);
        g_free (progress);
}
                                     

gboolean
medusa_index_is_currently_running (void)
{
        return access (INDEX_PROGRESS_FILE_NAME, F_OK) == 0;
}
          
int
medusa_index_progress_get_percentage_complete (void)
{
        int progress_fd;
        int percentage_complete;
        int read_result, close_result;
        
        
        g_return_val_if_fail (access (INDEX_PROGRESS_FILE_NAME, F_OK) != -1, -1);

        progress_fd = open (INDEX_PROGRESS_FILE_NAME, O_RDONLY);
        g_return_val_if_fail (progress_fd != -1, -1);

        read_result = read (progress_fd, &percentage_complete, sizeof (int));
        if (read_result < sizeof (int)) {
                return -1;
        }
        close_result = close (progress_fd);
        if (close_result == -1) {
                return -1;
        }

        return percentage_complete;
}


static gboolean
get_disk_used_information (char *root_directory,
                           size_t *total_blocks_to_index,
                           size_t *block_size)
{
        struct statfs statfs_buffer;
        int statfs_result;
        
        statfs_result = statfs (root_directory, &statfs_buffer);
        g_return_val_if_fail (statfs_result == 0, FALSE);
        *block_size = statfs_buffer.f_bsize; 
        *total_blocks_to_index = statfs_buffer.f_blocks;
        return TRUE;
}

static gboolean           
update_new_public_percent_complete (MedusaIndexProgress *progress,
                                    int new_percent_complete)
{
        int lseek_result, write_result;
        /*        const char *new_line = "\n"; */

        lseek_result = lseek (progress->output_fd, 0, SEEK_SET);
        if (lseek_result != 0) {
                return FALSE;
        }
        write_result = write (progress->output_fd, &new_percent_complete, sizeof (int));
        if (write_result != sizeof (int)) {
                return FALSE;
        }
        /* write_result = write (progress->output_fd, new_line, sizeof (char) * strlen (new_line));
        if (write_result != strlen (new_line) * sizeof (char)) {
                return FALSE;
                }*/
        return TRUE;
}
