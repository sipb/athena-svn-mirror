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

#include <gconf/gconf-client.h>

#include <gnome.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "gui.h"
#include "preferences.h"

#include "prog.h"

static GConfClient *client = NULL;
extern gboolean default_file;
extern gboolean able_to_record;

static void
record_timeout_changed (GConfClient *_client,
			guint cnxn_id,
			GConfEntry *entry,
			gpointer data)
{
  	GConfValue *value = gconf_entry_get_value (entry);

	record_timeout = gconf_client_get_int (client, "/apps/gnome-sound-recorder/record-timeout", NULL);
}

static void
stop_on_timeout_changed (GConfClient *_client,
			 guint cnxn_id,
			 GConfEntry *entry,
			 gpointer data)
{
	stop_on_timeout = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/stop-on-timeout", NULL);
}

static void
save_when_finished_changed (GConfClient *_client,
			    guint cnxn_id,
			    GConfEntry *entry,
			    gpointer data)
{
	save_when_finished = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/save-when-finished", NULL);
}

static void
popup_warning_changed (GConfClient *_client,
		       guint cnxn_id,
		       GConfEntry *entry,
		       gpointer data)
{
	popup_warn_mess = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/popup-warning", NULL);
}

static void
stop_record_changed (GConfClient *_client,
		     guint cnxn_id,
		     GConfEntry *entry,
		     gpointer data)
{
	stop_record = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/stop-record", NULL);
}

static void
popup_warn_mess_v_changed (GConfClient *_client,
			   guint cnxn_id,
			   GConfEntry *entry,
			   gpointer data)
{
	GConfValue *value = gconf_entry_get_value (entry);

	popup_warn_mess_v = gconf_client_get_int (client, "/apps/gnome-sound-recorder/popup-warning-v", NULL);
}

static void
stop_recording_v_changed (GConfClient *_client,
			  guint cnxn_id,
			  GConfEntry *entry,
			  gpointer data)
{
	GConfValue *value = gconf_entry_get_value (entry);

	stop_record_v = gconf_client_get_int (client, "/apps/gnome-sound-recorder/stop-recording-v", NULL);
}

static void
play_repeat_changed (GConfClient *_client,
		     guint cnxn_id,
		     GConfEntry *entry,
		     gpointer data)
{
	playrepeat = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/play-repeat", NULL);
}

static void
play_repeat_forever_changed (GConfClient *_client,
			     guint cnxn_id,
			     GConfEntry *entry,
			     gpointer data)
{
	playrepeatforever = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/play-repeat-forever", NULL);
}

static void
play_x_times_changed (GConfClient *_client,
		      guint cnxn_id,
		      GConfEntry *entry,
		      gpointer data)
{
	GConfValue *value = gconf_entry_get_value (entry);

	playxtimes = gconf_client_get_int (client, "/apps/gnome-sound-recorder/play-x-times", NULL);
}

static void
sox_command_changed (GConfClient *_client,
		     guint cnxn_id,
		     GConfEntry *entry,
		     gpointer data)
{
	const char *s;
	char *sox;
	GConfValue *value = gconf_entry_get_value (entry);

	s = gconf_client_get_string (client, "/apps/gnome-sound-recorder/sox-command", NULL);
	sox = g_find_program_in_path (s);
	if (sox == NULL) {
		able_to_record = FALSE;
		gtk_widget_set_sensitive (grecord_widgets.Record_button, FALSE);
		g_free (sox_command);
		sox_command = g_strdup (s);
	} else {
		able_to_record = TRUE;
		gtk_widget_set_sensitive (grecord_widgets.Record_button, TRUE);
		
		g_free (sox_command);
		sox_command = sox;
	}
}

static void
temp_dir_changed (GConfClient *_client,
		  guint cnxn_id,
		  GConfEntry *entry,
		  gpointer data)
{
	const char *t;
	GConfValue *value = gconf_entry_get_value (entry);

	g_free (temp_dir);
	t = gconf_client_get_string (client, "/apps/gnome-sound-recorder/tempdir", NULL);
	temp_dir = g_strdup (t);
}

static void
audio_format_changed (GConfClient *_client,
		      guint cnxn_id,
		      GConfEntry *entry,
		      gpointer data)
{
	audioformat = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/audio-format", NULL);
	if (default_file == FALSE) {
		return;
	}
	
	gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label),
			    audioformat ? _("Audio format: 8bit PCM") : _("Audio format: 16bit PCM"));
}

static void
sample_rate_changed (GConfClient *_client,
		     guint cnxn_id,
		     GConfEntry *entry,
		     gpointer data)
{
	const char *sample;
	char *s;
	GConfValue *value = gconf_entry_get_value (entry);
	
	g_free (samplerate);
	sample = gconf_client_get_string (client, "/apps/gnome-sound-recorder/sample-rate", NULL);
	samplerate = g_strdup (sample);

	if (default_file == FALSE) {
		return;
	}

	s = g_strdup_printf (_("Sample rate: %s"), samplerate);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), s);
	g_free (s);
}

static void
channels_changed (GConfClient *_client,
		  guint cnxn_id,
		  GConfEntry *entry,
		  gpointer data)
{
	channels = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/channels", NULL);

	if (default_file == FALSE) {
		return;
	}
	
	gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label),
			    channels ? _("Channels: mono") : _("Channels: stereo"));
}

void
load_config_file    (void)
{
	char *s;

	if (client == NULL) {
		client = gconf_client_get_default ();
	}

	gconf_client_add_dir (client, "/apps/gnome-sound-recorder",
			      GCONF_CLIENT_PRELOAD_ONELEVEL, NULL);
	
	record_timeout = gconf_client_get_int (client,
					       "/apps/gnome-sound-recorder/record-timeout", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/record-timeout",
				 record_timeout_changed, NULL, NULL, NULL);
				 
	stop_on_timeout = gconf_client_get_bool (client,
						 "/apps/gnome-sound-recorder/stop-on-timeout", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/stop-on-timeout",
				 stop_on_timeout_changed, NULL, NULL, NULL);
	
	save_when_finished = gconf_client_get_bool (client,
						    "/apps/gnome-sound-recorder/save-when-finished", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/save-when-finished",
				 save_when_finished_changed, NULL, NULL, NULL);
	
	popup_warn_mess = gconf_client_get_bool (client,
						 "/apps/gnome-sound-recorder/popup-warning", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/popup-warning",
				 popup_warning_changed, NULL, NULL, NULL);
	
	stop_record = gconf_client_get_bool (client,
					     "/apps/gnome-sound-recorder/stop-record", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/stop-record",
				 stop_record_changed, NULL, NULL, NULL);
	
	popup_warn_mess_v = gconf_client_get_int (client,
						  "/apps/gnome-sound-recorder/popup-warning-v", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/popup-warning-v",
				 popup_warn_mess_v_changed, NULL, NULL, NULL);
	
	stop_record_v = gconf_client_get_int (client,
					      "/apps/gnome-sound-recorder/stop-recording-v", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/stop-recording-v",
				 stop_recording_v_changed, NULL, NULL, NULL);

	
	playrepeat = gconf_client_get_bool (client,
					    "/apps/gnome-sound-recorder/play-repeat", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/play-repeat",
				 play_repeat_changed, NULL, NULL, NULL);
	
	playrepeatforever = gconf_client_get_bool (client,
						   "/apps/gnome-sound-recorder/play-repeat-forever", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/play-repeat-forever",
				 play_repeat_forever_changed, NULL, NULL, NULL);
	
	playxtimes = gconf_client_get_int (client,
					   "/apps/gnome-sound-recorder/play-x-times", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/play-x-times",
				 play_x_times_changed, NULL, NULL, NULL);

	
	s = gconf_client_get_string (client,
				     "/apps/gnome-sound-recorder/sox-command", NULL);
	sox_command = g_find_program_in_path (s);
	if (sox_command == NULL) {
		sox_command = s;
	} else {
		g_free (s);
	}
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/sox-command",
				 sox_command_changed, NULL, NULL, NULL);
				 
	
	temp_dir = gconf_client_get_string (client, "/apps/gnome-sound-recorder/tempdir", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/tempdir",
				 temp_dir_changed, NULL, NULL, NULL);


	audioformat = gconf_client_get_bool (client,
					     "/apps/gnome-sound-recorder/audio-format", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/audio-format",
				 audio_format_changed, NULL, NULL, NULL);
	
	samplerate = gconf_client_get_string (client,
					      "/apps/gnome-sound-recorder/sample-rate", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/sample-rate",
				 sample_rate_changed, NULL, NULL, NULL);
	
	channels = gconf_client_get_bool (client, "/apps/gnome-sound-recorder/channels", NULL);
	gconf_client_notify_add (client, "/apps/gnome-sound-recorder/channels",
				 channels_changed, NULL, NULL, NULL);
}

/* Callbacks --------------------------------------------------------- */

void
widget_in_propertybox_changed (GtkWidget* widget,
			       gpointer propertybox)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (propertybox));
}

void
on_checkbox_clicked_activate_cb (GtkWidget* widget, gpointer w)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
}

void
on_repeat_activate_cb (GtkWidget* widget,
		       gpointer data)
{
#if 0
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playrepeatforever_radiobutton_v), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_radiobutton_v), TRUE);

		/* Don't make it sensitive if playxtimes_radiobutton is selected */
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.playxtimes_radiobutton_v)))
			gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_spinbutton_v), TRUE);
	}
	else {
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playrepeatforever_radiobutton_v), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_radiobutton_v), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_spinbutton_v), FALSE);
	}
#endif
}
