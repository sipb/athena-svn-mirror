/*
* gok-sound.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnome.h>
#include <libgnome/gnome-sound.h>
#include <esd.h>
#include "gok-sound.h"
#include "gok-log.h"

static void free_entry (gpointer soundfile, gpointer sample_id,
                        gpointer user_data);

static gint gnome_sound_connection;
static GHashTable *stored_sounds;

/**
 * gok_sound_initialize:
 *
 * Initialises gok-sound.
 */
void gok_sound_initialize ()
{
    gok_log_enter ();
    gnome_sound_init ("localhost");
    gnome_sound_connection = gnome_sound_connection_get ();
    stored_sounds = g_hash_table_new (g_str_hash, g_str_equal);
    gok_log_leave ();
}

/**
 * gok_sound_shutdown:
 *
 * Shuts down gok-sound and frees any loaded sounds.
 */
void gok_sound_shutdown ()
{
    gok_log_enter ();
    g_hash_table_foreach (stored_sounds, free_entry, NULL);
    g_hash_table_destroy (stored_sounds);
    fsync (gnome_sound_connection);
    gnome_sound_shutdown ();
    gok_log_leave ();
}

void free_entry (gpointer soundfile, gpointer sample_id, gpointer user_data)
{
    esd_sample_free (gnome_sound_connection, *((gint *)sample_id) );
    gok_log ("Freed sample_id %d", *((gint *)sample_id) );
    g_free (soundfile);
    g_free (sample_id);
}

/**
 * gok_sound_play:
 * @soundfile: The sound file to play.
 *
 * Plays the sound file @soundfile. The @soundfile is stored in a cache
 * the first time that it is played and from then on the sound is played
 * from the cache.
 */
void gok_sound_play (gchar *soundfile)
{
    gchar *copy_of_soundfile;
    gint *sample_id;

    gok_log_enter ();

    /* Have we already loaded soundfile? */
    if ( (sample_id = g_hash_table_lookup (stored_sounds, soundfile))
         == NULL)
    {
        copy_of_soundfile = g_strdup (soundfile);
        sample_id = g_malloc (sizeof (gint));

        gok_log ("Loading soundfile %s", copy_of_soundfile);
        *sample_id = gnome_sound_sample_load (copy_of_soundfile,
                                              copy_of_soundfile);
        gok_log ("*sample_id = %d", *sample_id);

        if (*sample_id < 0)
        {
            gok_log_x ("Error loading soundfile %s", copy_of_soundfile);
            g_free (copy_of_soundfile);
            g_free (sample_id);
            gok_log_leave ();
            return;
        }
        else
        {
            g_hash_table_insert (stored_sounds, copy_of_soundfile, 
                                 sample_id);
        }
    }
    else
    {
        gok_log ("Playing soundfile %s from stored_sounds with sample_id %d",
                 soundfile, *sample_id);
    }
    
    esd_sample_play (gnome_sound_connection, *sample_id);

    gok_log_leave ();
}
