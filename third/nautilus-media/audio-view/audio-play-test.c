/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2002, Thomas Vander Stichele
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* audio-play-test.c - test AudioPlay object
 */

#include <config.h>
#include <gst/gst.h>
#include <gst/gconf/gconf.h>

#include "audio-play.h"

static void
handle_error (GError **error)
{
	if (*error)
	{
		g_print ("ERROR: %s\n", (*error)->message);
		g_error_free (*error);
	}
	*error = NULL;
}

void
tick_callback (AudioPlay *play, guint64 nanosecs)
{
	g_print ("tick: %d\n", (int) (nanosecs / GST_SECOND));
}

void
length_callback (AudioPlay *play, guint64 nanosecs)
{
	g_print ("length: %.3f\n", ((float) nanosecs / GST_SECOND));
}

void
eos_callback (AudioPlay *play, gpointer data)
{
	g_print ("quitting main gst loop\n");
	gst_main_quit ();
}

int
main (int argc, char *argv[])
{
	AudioPlay *play;
	GstElement *audiosink;
	int current_arg = 1;
	GError *error = NULL;

	gst_init (&argc, &argv);

	if (argc < 2)
	{
		g_print ("You need to supply a URI for a file to test\n");
		return -1;
	}

	audiosink = gst_gconf_get_default_audio_sink ();
	g_assert (audiosink);
	play = audio_play_new (NULL);
	g_assert (play);
	g_assert (play->priv);
	audio_play_set_audio_sink (play, audiosink);

	while (current_arg < argc)
	{
		gchar *file = g_strdup (argv[current_arg]);
		g_print ("file: %s\n", file);
		g_print ("loading location\n");
		audio_play_set_location (play, file, &error);
		g_free (file);
		if (error)
			handle_error (&error);
		else
		{
			g_print ("setting to play\n");
			g_signal_connect (G_OBJECT (play), "tick",
				  	  G_CALLBACK (tick_callback), NULL);
			g_signal_connect (G_OBJECT (play), "length",
				  	  G_CALLBACK (length_callback), NULL);
			g_signal_connect (G_OBJECT (play), "eos",
					  G_CALLBACK (eos_callback), NULL);
			audio_play_set_state (play, GST_STATE_PLAYING, &error);
			handle_error (&error);

			g_print ("DEBUG: going into gst_main ()\n");
			gst_main ();
			g_print ("DEBUG: coming out of gst_main ()\n");
			/* we get back on eos */
			audio_play_set_state (play, GST_STATE_READY, &error);
		}
		++current_arg;
		g_print ("\n");
	}
}
