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
 *  medusa-unsearched-locations.c -- Way to specify files and directories to
 *  skip, like nfs mounts, removeable media, and other files we specifically aren't
 *  interested in.
 */

#include <config.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "medusa-utils.h"

#ifdef HAVE_GETMNTINFO
# undef MAX
# undef MIN
# include <sys/param.h>
# include <sys/ucred.h>
# include <sys/mount.h>
#elif (HAVE_SYS_MNTTAB_H)
#include <sys/mnttab.h>
#elif defined (HAVE_GETMNTENT)
# include <mntent.h>
#else
#warning "Can't find a valid mount function type"
#endif

#include <glib.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#include "medusa-unsearched-locations.h"

#define CDROM_DRIVE_STATUS	0x5326  /* Get tray position, etc. */
#define CDSL_CURRENT    	((int) (~0U>>1))
#define STOPLIST_FILE_NAME MEDUSA_PREFIX "/share/medusa/" "file-index-stoplist"

static void                 file_stoplist_initialize                          (void);
static void                 unsearched_mount_list_initialize                  (void);

#ifdef HAVE_GETMNTINFO
static gboolean             mount_point_is_cdrom                              (const struct statfs *fs_list);
#elif HAVE_SYS_MNTTAB_H
static  gboolean            mount_point_is_cdrom                              (const struct mnttab *mnttab_entry);
#elif defined(HAVE_GETMNTENT)
static gboolean             mount_point_is_cdrom                              (const struct mntent *mount_entry);
#endif

static void                 unsearched_location_free                          (gpointer key,
                                                                               gpointer value,
                                                                               gpointer user_data);

static GHashTable *unsearched_locations = NULL;

const char *mount_type_skip_list[] = {
        "nfs",
        "shm",
        NULL
};

static gboolean
mount_type_is_in_skip_list (const char *mount_type)
{
        int i;
        
        g_return_val_if_fail (mount_type != NULL, TRUE);

        for (i = 0; mount_type_skip_list[i] != NULL; i++) {
                if (strcmp (mount_type_skip_list[i], mount_type) == 0) {
                        return TRUE;
                }
        }

        return FALSE;
}

gboolean
medusa_is_unsearched_location (const char *file_name)
{
        g_return_val_if_fail (unsearched_locations != NULL, FALSE);

        if (g_hash_table_lookup (unsearched_locations, file_name) == NULL) {
                return FALSE;
        }
        else {
                return TRUE;
        }
}

void
medusa_unsearched_locations_initialize (void)
{
        unsearched_locations = g_hash_table_new (g_str_hash,
                                                 g_str_equal);
                                                 
        file_stoplist_initialize ();
        unsearched_mount_list_initialize ();
}

void
medusa_unsearched_locations_shutdown (void)
{
        g_hash_table_foreach (unsearched_locations,
                              unsearched_location_free,
                              NULL);

        g_hash_table_destroy (unsearched_locations);
        unsearched_locations = NULL;
}


static void
file_stoplist_initialize  (void)
{
  
	FILE *stop_list_handle;
	char next_file[NAME_MAX];

	stop_list_handle = fopen (STOPLIST_FILE_NAME, "r");
	g_return_if_fail (stop_list_handle != NULL);

	while (fscanf (stop_list_handle, "%s\n", next_file) != EOF) {
                /* Skip comment lines */
                if (next_file[0] == '#') {
                        continue;
                }
                g_hash_table_insert (unsearched_locations,
                                     g_strdup (next_file),
                                     GINT_TO_POINTER (1));
	}
	fclose (stop_list_handle);
        
}

/* Don't index or search nfs mount points and removeable media, for now */
static void              
unsearched_mount_list_initialize (void)
{
        
#ifdef HAVE_GETMNTINFO
	struct statfs *fs_list;
	int fs_count, i;
        /* FIXME: There is probably a leak here */
	fs_count = getmntinfo (&fs_list, MNT_LOCAL); /* returns the number of FSes. */
	for (i = 0; i < fs_count; ++i) {
		if (mount_type_is_in_skip_list (fs_list[i].f_fstypename)) {
			g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (fs_list[i].f_mntonname),
                                             GINT_TO_POINTER (1));
		}
                if (mount_point_is_cdrom (&fs_list[i])) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (fs_list[i].f_mntonname),
                                             GINT_TO_POINTER (1));
                }
        }
#elif HAVE_SYS_MNTTAB_H
        FILE *mount_file;
        struct mnttab *mnttab_entry;

        mount_file = setmntent ("/etc/mnttab", "r");
        while (getmntent (mount_file, mnttab_entry) != 0) {
                if (mount_type_is_in_skip_list (mnttab_entry->mnt_fstype)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mnttab_entry->mnt_mountp),
                                             GINT_TO_POINTER (1));
                }
                if (mount_point_is_cdrom (mnttab_entry)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mnttab_entry->mnt_mountp),
                                             GINT_TO_POINTER (1));
                }
        }
        endmntent (mount_file);

#elif defined(HAVE_GETMNTENT)
        FILE *mount_file;
        struct mntent *mount_entry;
        
        mount_file = setmntent (MOUNTED, "r");
        while ((mount_entry = getmntent (mount_file)) != NULL) {
                if (mount_type_is_in_skip_list (mount_entry->mnt_type)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mount_entry->mnt_dir),
                                             GINT_TO_POINTER (1));
                }
                if (mount_point_is_cdrom (mount_entry)) {
                        g_hash_table_insert (unsearched_locations,
                                             gnome_vfs_get_uri_from_local_path (mount_entry->mnt_dir),
                                             GINT_TO_POINTER (1));
                }
        }
	endmntent (mount_file);
        g_free (mount_entry);
#endif
}
        

#ifdef HAVE_GETMNTINFO        
static gboolean             
mount_point_is_cdrom (const struct statfs *fs_list)
{
        int fd;

        if (strcmp (fs_list->f_fstypename, "cdrom")) {
                return FALSE;
        }
        fd = open (fs_list->f_mntonname, O_RDONLY | O_NONBLOCK);

#elif HAVE_SYS_MNTTAB_H

static gboolean
mount_point_is_cdrom (const struct mnttab *mnttab_entry)
{
        int fd;

        if (strcmp (mnttab_entry->mnt_fstype, "iso9660")) {
                return FALSE;
        }

        fd = open (mnttab_entry->mnt_mountp, O_RDONLY | O_NONBLOCK);

#elif defined(HAVE_GETMNTENT)
static gboolean
mount_point_is_cdrom (const struct mntent *mount_entry)
{
        int fd;


        if (strcmp (mount_entry->mnt_type, "iso9660")) {
                return FALSE;
        }

        fd = open (mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
#endif

        /* These tests are shamelessly stolen from
           Gene Regan's nautilus-volume-monitor code
           in the nautilus module */
        if (fd < 0) {
                return FALSE;
        }

	if (ioctl (fd, CDROM_DRIVE_STATUS, CDSL_CURRENT) < 0) {
                close (fd);
                return FALSE;
        }

        close (fd);
        
        return TRUE;
        
}

static void                 
unsearched_location_free (gpointer key,
                          gpointer value,
                          gpointer user_data)
{
        char *unsearched_location;

        g_return_if_fail (key != NULL);

        unsearched_location = (char *) key;
        g_free (unsearched_location);
}
