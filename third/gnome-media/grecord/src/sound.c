/*
 * GNOME sound-recorder: a soundrecorder and soundplayer for GNOME.
 *
 * Copyright (C) 2000 :
 * Andreas Hyden <a.hyden@cyberpoint.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <sys/stat.h>
#include <unistd.h>
#include <audiofile.h>

#include "sound.h"
#include "gui.h"
#include "prog.h"

gdouble
get_play_time (const gchar* filename)
{
	gdouble play_time;
	gboolean nofile = FALSE;

	/* Check if there is an active file or not */
	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
		nofile = TRUE;
	
	if (!nofile) {
		AFframecount framecount;
		AFfilehandle file;
		AFfilesetup setup;
		gint samplerate;
		
		/* Audiofile setup */
		setup = afNewFileSetup ();
		file = afOpenFile (filename, "r", setup);
		
		/* Get some info from the soundfile */
		framecount  = afGetFrameCount (file, AF_DEFAULT_TRACK);
		samplerate  = afGetRate (file, AF_DEFAULT_TRACK);

		/* Play time */
		play_time = (int) framecount / samplerate;
		
		afFreeFileSetup (setup);
	}
	else
		play_time = 0;
       
	  return play_time;
}    

void
set_min_sec_time (gint sec)
{
	gint minutes = 0;
	gint seconds = 0;
	
	gchar* temp_string = NULL;

        /* Time in seconds -> time in seconds & minutes */
	minutes = (int) sec / 60;
	seconds = sec - (minutes * 60);

        /* Show it on the main window */
	if (minutes <= 0)
		temp_string = g_strdup ("00");
	else if (minutes < 10)
		temp_string = g_strdup_printf ("0%i", minutes);
	else
		temp_string = g_strdup_printf ("%i", minutes);

	gtk_label_set_text (GTK_LABEL (grecord_widgets.timemin_label), temp_string);

	g_free (temp_string);

	if (seconds <= 0)
		temp_string = g_strdup ("00");
	else if (seconds < 10)
		temp_string = g_strdup_printf ("0%i", seconds);
	else
		temp_string = g_strdup_printf ("%i", seconds);

	gtk_label_set_text (GTK_LABEL (grecord_widgets.timesec_label), temp_string);

	g_free (temp_string);
}

/******************************/
/* ------ Effects ----------- */
/******************************/

void
add_echo (gchar* filename, gboolean overwrite)
{
	static gboolean first_time = TRUE;
	gchar* backup_file;

	backup_file = g_build_filename (temp_dir, temp_filename_backup, NULL);

	/* Make a backup only the first time */
	if (first_time) {
		pid_t pid;
		int status;

		pid = fork ();
		if (pid == 0) {
			execlp ("cp", "cp", "-f", active_file, backup_file, NULL);
			perror ("cp");
			_exit (1);
		} else if (pid == -1) {
			g_error ("Cannot fork child process");
		} else {
			waitpid (pid, &status, 0);
		}
		first_time = FALSE;
	}

	run_command (_("Adding echo to sample..."), sox_command,
		     backup_file, active_file, "echo", "0.8", "60.0",
		     "0.4", NULL);
		

	g_free (backup_file);
}

void
increase_speed (gchar* filename, gboolean overwrite)
{
}

void
decrease_speed (gchar* filename, gboolean overwrite)
{
}
