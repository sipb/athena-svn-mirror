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


#include <gconf/gconf-client.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <audiofile.h>
#include <esd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#include "grec.h"
#include "gui.h"
#include "preferences.h"
#include "sound.h"

#include "prog.h"

#define READ 0
#define WRITE 1

#define SAVE 0
#define DONTSAVE 1
#define CANCEL 2

const gchar* maintopic = N_("Sound Recorder:");

const gchar* temp_filename_record = "untitled.raw";
const gchar* temp_filename_play = "untitled.wav";
const gchar* temp_filename_backup = "untitled_backup.wav";
const gchar* temp_filename = "grecord_temp.wav";

gint repeat_counter = 0;

gchar* active_file = NULL;
gboolean default_file = FALSE;
gboolean file_changed = FALSE;
gboolean show_message = FALSE;

static guint play_id;
static guint record_id;

extern gboolean able_to_record;

static void
on_dontshowagain_dialog_destroy_activate (GtkWidget* widget,
					  gpointer checkbutton)
{
	GConfClient *client;
	gboolean stat = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
	
	client = gconf_client_get_default ();
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/show-warning-messages", !stat, NULL);
	g_object_unref (client);
}

gboolean
check_for_sox (void)
{
	char *p;
	gboolean show_warningmess;
	GConfClient *client;
	
	if (sox_command == NULL) {
		return FALSE;
	}

	client = gconf_client_get_default ();
	show_warningmess = gconf_client_get_bool (client,
						  "/apps/gnome-sound-recorder/show-warning-messages", NULL);
	g_object_unref (client);

	p = g_find_program_in_path (sox_command);
	if (p == NULL) {
		able_to_record = FALSE;
		gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
		if (show_warningmess) {
			GtkWidget* dont_show_again_checkbutton = gtk_check_button_new_with_label (_("Don't show this message again."));
			
			gchar* show_mess = g_strdup_printf (_("Could not find '%s'.\nSet the correct path to sox in"
							      "preferences under the tab 'paths'.\n\nIf you don't have"
							      " sox, you will not be able to record or do any effects."),
							    sox_command);
			GtkWidget* mess = gtk_message_dialog_new (NULL,
								  GTK_DIALOG_MODAL,	
								  GTK_MESSAGE_WARNING,
								  GTK_BUTTONS_OK,
								  show_mess);
			g_free (show_mess);
			
			gtk_widget_show (dont_show_again_checkbutton);
			gtk_container_add (GTK_CONTAINER (GTK_DIALOG (mess)->vbox), dont_show_again_checkbutton);
			
			/* Connect a signal on ok-button, so we can get the stat on the checkbutton */
			g_signal_connect (mess, "destroy",
					  G_CALLBACK (on_dontshowagain_dialog_destroy_activate), dont_show_again_checkbutton);
			
			gtk_dialog_run (GTK_DIALOG (mess));
			gtk_widget_destroy (mess);
		}

		able_to_record = FALSE;
		gtk_widget_set_sensitive (grecord_widgets.Record_button, FALSE);
		return FALSE;
	} else {
		g_free (p);
	}

	return TRUE;
}

/* ------------------- Callbacks ------------------------------- */
void
on_record_activate_cb (GtkWidget* widget, gpointer data)
{
	show_message = TRUE;
	/* Check if the sounddevice is ready */
	if (!check_if_sounddevice_ready ())
		return;

	if (check_for_sox () == FALSE) {
		return;
	}
	
	grecord_set_sensitive_progress ();
	save_set_sensitive (TRUE);
	file_changed = TRUE;

	/* Reset record-time and stuff */
	UpdateStatusbarRecord (TRUE);

	RecEng.pid = fork ();
	if (RecEng.pid == 0) {
		
		/* Record */
		record_sound ();

		_exit (0);
	}
	else if (RecEng.pid == -1)
		g_error (_("Could not fork child process"));

	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _("Recording..."));
	RecEng.is_running = TRUE;

	record_id = gtk_timeout_add (1000,
				     (GtkFunction) UpdateStatusbarRecord,
				     FALSE);
}

void
on_play_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Check if the sounddevice is ready */
	if (!check_if_sounddevice_ready ())
		return;

	grecord_set_sensitive_progress ();

	/* Show play-time and stuff */
	UpdateStatusbarPlay (TRUE);

	PlayEng.pid = fork ();
	if (PlayEng.pid == 0) {
#if 0
		int i;
		
		/* Play file */
		if (playrepeat == TRUE &&
		    playrepeatforever == FALSE) {
			for (i = 0; i < playxtimes; i++) {
				g_print ("%d of %d: Playing %s\n", i, playxtimes, active_file);
				play_sound (active_file);
			}
		} else if (playrepeat == TRUE &&
			   playrepeatforever == TRUE) {
			while (1) {
				play_sound (active_file);
			}
		} else if (playrepeat == FALSE) {
			play_sound (active_file);
		}
#endif
		play_sound (active_file);
		_exit (0);
	}
	else if (PlayEng.pid == -1)
		g_error (_("Could not fork child process"));
	
	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _("Playing..."));
	PlayEng.is_running = TRUE;
	
	play_id = gtk_timeout_add (1000,
				   (GtkFunction) UpdateStatusbarPlay,
				   (gpointer) FALSE);
}

void
on_stop_activate_cb (GtkWidget* widget, gpointer data)
{
	gchar* temp_string1 = NULL;
	gchar* temp_string2 = NULL;
	mode_t old_mask;

	gnome_appbar_clear_stack (GNOME_APPBAR (grecord_widgets.appbar));

	if (RecEng.is_running) {
		temp_string1 = g_strconcat ("-r ", samplerate, NULL);
		
		if (channels)
			temp_string2 = g_strdup ("-c 1");
		else
			temp_string2 = g_strdup ("-c 2");

	}

	if (PlayEng.is_running) {

		gtk_timeout_remove (play_id);

		kill (PlayEng.pid, SIGKILL);
		waitpid (PlayEng.pid, NULL, WUNTRACED);

		PlayEng.is_running = FALSE;

		grecord_set_sensitive_file ();
	}

	if (RecEng.is_running) {
		gchar* temp_string;
		gchar* tfile1 = g_build_filename (temp_dir, 
						  temp_filename_record, NULL);
		gchar* tfile2 = g_build_filename (temp_dir, 
						  temp_filename_play, NULL);
		
		kill (RecEng.pid, SIGKILL);
		waitpid (RecEng.pid, NULL, WUNTRACED);

		RecEng.is_running = FALSE;

		old_mask = umask(0077);
		run_command (_("Converting file..."), sox_command, temp_string1,
			     temp_string2, "-w", "-s", tfile1, tfile2, NULL);
		umask(old_mask);

		temp_string = g_build_filename (temp_dir, 
						temp_filename_play, NULL);

		g_free (temp_string);
		g_free (tfile1);
		g_free (tfile2);
	}

	g_free (temp_string1);
	g_free (temp_string2);
}

void
on_new_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	gchar* string = NULL;
	gchar* temp_string = NULL;
	gchar* file1 = g_build_filename (temp_dir, temp_filename_record, NULL);
	gchar* file2 = g_build_filename (temp_dir, temp_filename_play, NULL);
	gchar* file3 = g_build_filename (temp_dir, temp_filename_backup, NULL);

	if (PlayEng.is_running || RecEng.is_running || convert_is_running)
		on_stop_activate_cb (widget, data);

	if (file_changed) {
		choice = save_dont_or_cancel (_("Cancel"));
		if (choice == SAVE) {
			save_dialog ();
			return;
		}
		else if (choice == CANCEL)
			return;
	}

	/* Remove old files (if any) */
	remove (file1);
	remove (file2);
	remove (file3);

	g_free (file1);
	g_free (file2);
	g_free (file3);

	
	default_file = TRUE;
	
	if (active_file != NULL) {
		g_free (active_file);
	}
	active_file = g_build_filename (temp_dir, temp_filename_play, NULL);
	file_changed = FALSE;
	save_set_sensitive (FALSE);
	
	set_min_sec_time (get_play_time (active_file));

	grecord_set_sensitive_nofile ();

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar), GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1000, 1, 1, 0)));
/*  	gtk_range_slider_update (GTK_RANGE (grecord_widgets.Statusbar)); */

	/* Reload configuration */
	load_config_file ();

	if (!audioformat)
		string = g_strdup ("16bit pcm");
	else
		string = g_strdup ("8bit pcm");

	temp_string = g_strconcat (_("Audioformat: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label), temp_string);
	g_free (temp_string);

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), temp_string);
	g_free (temp_string);
	g_free (string);

	if (!channels)
		string = g_strdup ("stereo");
	else
		string = g_strdup ("mono");

	temp_string = g_strconcat (_("Channels: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label), temp_string);
	g_free (temp_string);
	g_free (string);
}

void
on_open_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	static GtkWidget* filesel = NULL;

	if (filesel) {
		if (filesel->window == NULL)
			return;
		
		gdk_window_show (filesel->window);
		gdk_window_raise (filesel->window);
		return;
	}

	if (file_changed) {
		choice = save_dont_or_cancel (_("Cancel open"));
		if (choice == SAVE) {
			save_dialog ();
			return;
		}
		else if (choice == CANCEL)
			return;
	}

	filesel = gtk_file_selection_new (_("Select a sound file"));

	g_signal_connect ((GTK_FILE_SELECTION (filesel)->ok_button),
			  "clicked", G_CALLBACK (store_filename),
			  filesel);

	g_signal_connect (filesel, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &filesel);

	g_signal_connect_swapped ((GTK_FILE_SELECTION (filesel)->cancel_button),
				  "clicked", G_CALLBACK (gtk_widget_destroy),
				  (gpointer) filesel);

	gtk_widget_show (filesel);
}

void
on_save_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Check if the current sample is a new (recorded) one */
	is_file_default ();

	if (default_file)
		save_dialog ();

	/* No saving is needed, because the changes goes directly to the file. */
	/* If you don't want to save the changes (when it ask you) or if you select */
	/* 'undo all', it will copy the backup file back to the active file. */
}

void
on_saveas_activate_cb (GtkWidget* widget, gpointer data)
{
	save_dialog ();
}

void
on_exit_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	gchar* tfile;

	if (PlayEng.is_running || RecEng.is_running)
		on_stop_activate_cb (widget, data);

	if (file_changed) {
		choice = save_dont_or_cancel (_("Cancel"));
		if (choice == SAVE) {
			save_dialog ();
		}
		else if (choice == CANCEL)
			return;
	}

	/* User didn't want to save; copy the backup file to the changed file (active file) */
	tfile = g_build_filename (temp_dir, temp_filename_backup, NULL);
	
	if (g_file_test (tfile, G_FILE_TEST_EXISTS)) {
		execlp ("cp", "cp", "-f", tfile, active_file, NULL);
	}

	gtk_main_quit ();

	remove (tfile);
	g_free (tfile);

	tfile = g_build_filename (temp_dir, temp_filename_record, NULL);
	remove (tfile);
	g_free (tfile);
	tfile = g_build_filename (temp_dir, temp_filename_play, NULL);
	remove (tfile);
	g_free (tfile);
}

void
on_preferences_activate_cb (GtkWidget* widget, gpointer data)
{
	static GtkWidget* props = NULL;

	if (props) {
		if (!props->window)
			return;

		gdk_window_show (props->window);
		gdk_window_raise (props->window);
		return;
	}
	props = create_grecord_propertybox ();
	
	g_signal_connect (props, "destroy", G_CALLBACK (gtk_widget_destroyed),
			  &props);
	gtk_widget_show (props);
}

void
on_about_activate_cb (GtkWidget* widget, gpointer data)
{
	static GtkWidget* about = NULL;

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}
	
	about = create_about ();

	g_signal_connect (G_OBJECT (about),
			  "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &about);

	gtk_widget_show (about);
}

/* These need to bring up an error dialog */
void
on_runmixer_activate_cb (GtkWidget* widget, gpointer data)
{
	char *mixer_path;
	char *argv[2] = {NULL, NULL};
	GError *error = NULL;
	gboolean ret;

	/* Open the mixer */
	mixer_path = g_find_program_in_path (DEFAULT_MIXER);
	if (mixer_path == NULL) {
		g_warning (_("%s is not installed in the path"), DEFAULT_MIXER);
		return;
	}
	
	argv[0] = mixer_path;
	ret = g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, NULL, &error);
	if (ret == FALSE) {
		g_warning (_("There was an error starting %s: %s"),
			   mixer_path, error->message);
		g_error_free (error);
	}

	g_free (mixer_path);
}

void 
on_increase_speed_activate_cb (GtkWidget* widget, gpointer data)
{
}

void 
on_decrease_speed_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_add_echo_activate_cb (GtkWidget* widget, gpointer data)
{
	if (!g_file_test (active_file, G_FILE_TEST_EXISTS))
	    return;

	file_changed = TRUE;
	save_set_sensitive (TRUE);
	add_echo (active_file, TRUE);
}

void
on_show_time_activate_cb (GtkWidget* widget, gpointer data)
{
	GConfClient *client;
	GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM (widget);
	gboolean active = item->active;

	if (active) {
		gtk_widget_show (grecord_widgets.timespace_label);
		gtk_widget_show (grecord_widgets.timesec_label);
		gtk_widget_show (grecord_widgets.timemin_label);
	}
	else {
		gtk_widget_hide (grecord_widgets.timespace_label);
		gtk_widget_hide (grecord_widgets.timesec_label);
		gtk_widget_hide (grecord_widgets.timemin_label);
	}

	client = gconf_client_get_default ();
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/show-time", active, NULL);
	g_object_unref (G_OBJECT (client));
}

void
on_show_soundinfo_activate_cb (GtkWidget* widget, gpointer data)
{
	GConfClient *client;
	GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM (widget);
	gboolean active = item->active;

	if (active) {
		gtk_widget_show (grecord_widgets.audio_format_label);
		gtk_widget_show (grecord_widgets.sample_rate_label);
		gtk_widget_show (grecord_widgets.nr_of_channels_label);
	}
	else {
		gtk_widget_hide (grecord_widgets.audio_format_label);
		gtk_widget_hide (grecord_widgets.sample_rate_label);
		gtk_widget_hide (grecord_widgets.nr_of_channels_label);
	}

	client = gconf_client_get_default ();
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/show-sound-info", active, NULL);
	g_object_unref (G_OBJECT (client));
}

void
on_undo_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_redo_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_undoall_activate_cb (GtkWidget* widget, gpointer data)
{
	gchar* temp_string = g_build_filename (temp_dir, 
					       temp_filename_backup, NULL);

	if (g_file_test (temp_string, G_FILE_TEST_EXISTS)) {
		run_command (_("Undoing all changes..."), "cp", "-f",
			     temp_string, active_file, NULL);
	}

	g_free (temp_string);
}

/* --------------------- Help functions -------------------- */
void
record_sound (void)
{
	gchar* tfile;
	gchar buf[ESD_BUF_SIZE];
	gint sock = -1;
	gint rate = 0;
	gint length = 0;
	
	gint bits = ESD_BITS16;
	gint esd_channels = ESD_STEREO;
	gint mode = ESD_STREAM;
	gint func = ESD_RECORD;
	esd_format_t format = 0;

	char *host = NULL;
	char *name = NULL;

	FILE *target = NULL;

	/* Open the file for writing */
	if (temp_dir == NULL) {
		g_warning ("temp_dir == NULL\nFixing to /tmp");
		temp_dir = g_strdup ("/tmp");
	}
	
	tfile = g_build_filename (temp_dir, temp_filename_record, NULL);
	target = fopen (tfile, "w");
	if(target == NULL) {
		g_free (tfile);
		return;
	}

	/* Set permissions on new file */
	if(fchmod (fileno (target), 0600) != 0) {
		g_free (tfile);
		return;
	}
	g_free (tfile);

	/* Set up bits, channels etc after the preferences */
	if (channels == 1) {
		esd_channels = ESD_MONO;
	}
	
	if (audioformat != 0) {
		bits = ESD_BITS8;
	}

	if (samplerate == NULL) {
		rate = 22050;
	} else {
		rate = atoi (samplerate);
	}

	format = bits | esd_channels | mode | func;
	sock = esd_record_stream_fallback (format, rate, host, name);
	
	if (sock <= 0)
		return;

	/* Start recording */
	while ((length = read(sock, buf, ESD_BUF_SIZE)) > 0) {
		if (fwrite (buf, 1, length, target) <= 0)
			return;
	}
	close (sock);
       	return;
}

void
play_sound (const gchar* filename)
{
	/* Play the file */
	esd_play_file ("grecord", filename, 1);
}

void
store_filename (GtkFileSelection* selector, gpointer file_selector)
{
	GtkWidget* mess;
	char* string = NULL;
	char* temp_string = NULL;
	AFfilehandle filename;
	gint in_audioformat, in_channels, in_rate, in_width;
	struct stat file;
	const char* tempfile;

	tempfile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));

	if (!stat(tempfile, &file)) {
		if (S_ISDIR (file.st_mode)) {
			mess = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_OK,
						       _("'%s' is a folder.\nPlease select a sound file to be opened."),
						       tempfile);
			g_signal_connect (G_OBJECT (mess), "response",
					  G_CALLBACK (gtk_widget_destroy), NULL);

			gtk_widget_show (mess);
			return;
		}
	} else if (errno == ENOENT) {
		mess = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_OK,
					    _("File '%s' doesn't exist.\nPlease select an existing sound file to be opened."),
					    tempfile);
		g_signal_connect (G_OBJECT (mess), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);

		gtk_widget_show (mess);
		return;
	}

	if (!soundfile_supported (tempfile)) {
		mess = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("File '%s isn't a valid sound file."),
					       tempfile);
		g_signal_connect (G_OBJECT (mess), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show (mess);
		return;
	}

	if (active_file != NULL) {
		g_free (active_file);
	}
	active_file = g_strdup (tempfile);
	file_changed = FALSE;
	save_set_sensitive (FALSE);
	gtk_widget_destroy (GTK_WIDGET (file_selector));
	
	grecord_set_sensitive_file ();
	
	/* Get info about the file */
	filename = afOpenFile (active_file, "r", NULL);
	if (!filename)
		return;

	in_channels = afGetChannels (filename, AF_DEFAULT_TRACK);
	in_rate     = afGetRate (filename, AF_DEFAULT_TRACK);
	afGetSampleFormat (filename, AF_DEFAULT_TRACK, &in_audioformat, &in_width);

	/* Update mainwindow with the new values and set topic */
	set_min_sec_time (get_play_time (active_file));

	samplerate = g_strdup_printf ("%d", in_rate);
	if (in_channels == 2)
		string = g_strdup ("stereo");
	else
		string = g_strdup ("mono");

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), temp_string);
	g_free (temp_string);

	temp_string =  g_strconcat (_("Channels: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label), temp_string);
	g_free (temp_string);
	g_free (string);

	if (in_audioformat)
		string = g_strdup ("16bit pcm");
	else
		string = g_strdup ("8bit pcm");
	
	temp_string = g_strconcat (_("Audioformat: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label), temp_string);
	g_free (temp_string);
	g_free (string);

	default_file = FALSE;
}

void
save_filename (GtkFileSelection* selector, gpointer file_selector)
{
	gchar* new_file;
	struct stat file;

	new_file = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));

	/* Check if the file already exists */
	if (!stat(new_file, &file)) {
		GtkWidget* mess;

		if (S_ISDIR (file.st_mode)) {
			mess = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_OK,
						       _("'%s' is a folder.\nPlease enter another filename."),
						       new_file);
			g_signal_connect (G_OBJECT (mess), "response",
					  G_CALLBACK (gtk_widget_destroy), NULL);
			
			gtk_widget_show (mess);
			g_free (new_file);
			return;
		}
		mess = gtk_message_dialog_new (NULL, 0,
					       GTK_MESSAGE_WARNING,
					       GTK_BUTTONS_NONE,
					       _("<b>File '%s' already exists.</b>\nDo you want to overwrite it?"),
					       new_file);
		gtk_dialog_add_buttons (GTK_DIALOG (mess),
					_("Cancel save"), 0,
					_("Overwrite"), 1,
					NULL);

		/* Bad hack usage #2 */
		gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), TRUE);
		gtk_label_set_line_wrap (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), FALSE);
		gtk_label_set_justify (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), GTK_JUSTIFY_LEFT);

		switch (gtk_dialog_run (GTK_DIALOG (mess))) {
		case 0:
			gtk_widget_destroy (mess);
			g_free (new_file);
			return;

		default:
			gtk_widget_destroy (mess);
			break;
		}
	}

	/* Check if the soundfile is supported */
	if (!save_sound_file (new_file)) {
		GtkWidget *mess;

		mess = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("Error saving '%s'"),
					       new_file);
		g_signal_connect (G_OBJECT (mess), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		
		gtk_widget_show (mess);
		
		g_free (new_file);
		return;
	}

	if (active_file != NULL) {
		g_free (active_file);
	}
	active_file = new_file;
	
	gtk_widget_destroy (GTK_WIDGET (file_selector));

	set_window_title (active_file);
	file_changed = FALSE;
	save_set_sensitive (FALSE);
	file_selector = NULL;

	gtk_main_quit ();
}

/* Yes I stole this from GEdit. */
static GtkWidget *
grec_button_new_with_stock_image (const char *text,
				  const char *stock_id)
{
	GtkWidget *button;
	GtkStockItem item;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *hbox;
	GtkWidget *align;

	button = gtk_button_new ();

	if (GTK_BIN (button)->child) {
		gtk_container_remove (GTK_CONTAINER (button),
				      GTK_BIN (button)->child);
	}

	if (gtk_stock_lookup (stock_id, &item)) {
		label = gtk_label_new_with_mnemonic (text);
		gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);

		image = gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_BUTTON);
		hbox = gtk_hbox_new (FALSE, 2);
		align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);

		gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

		gtk_container_add (GTK_CONTAINER (button), align);
		gtk_container_add (GTK_CONTAINER (align), hbox);
		gtk_widget_show_all (align);

		return button;
	}

	label = gtk_label_new_with_mnemonic (text);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), button);
	
	gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.5);
	gtk_widget_show (label);
	gtk_container_add (GTK_CONTAINER (button), label);

	return button;
}

static GtkWidget *
grec_dialog_add_button (GtkDialog *dialog,
			const char *text,
			const char *stock_id,
			int response_id)
{
	GtkWidget *button;

	g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
	g_return_val_if_fail (text != NULL, NULL);
	g_return_val_if_fail (stock_id != NULL, NULL);

	button = grec_button_new_with_stock_image (text, stock_id);
	g_return_val_if_fail (button != NULL, NULL);

	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

	gtk_widget_show (button);

	gtk_dialog_add_action_widget (dialog, button, response_id);

	return button;
}

gint
save_dont_or_cancel (const char *quit_text)
{
	int result;
	char *filename;
	char *title;
	GtkWidget *mess;

	if (active_file == NULL) {
		filename = g_strdup ("untitled.wav");
	} else {
		filename = g_path_get_basename (active_file);
	}
	
	mess = gtk_message_dialog_new (grecord_widgets.grecord_window, 0,
				       GTK_MESSAGE_WARNING,
				       GTK_BUTTONS_NONE,
				       _("<b>Do you want to save the changes you made to \"%s\"?</b>\n"
					 "\nYour changes will be lost if you don't save them."),
				       filename);
	
	grec_dialog_add_button (GTK_DIALOG (mess),
				_("Do_n't save"), GTK_STOCK_NO, DONTSAVE);
	grec_dialog_add_button (GTK_DIALOG (mess),
				quit_text, GTK_STOCK_CANCEL, CANCEL);
	gtk_dialog_add_button (GTK_DIALOG (mess),
			       GTK_STOCK_SAVE, SAVE);
	gtk_dialog_set_default_response (GTK_DIALOG (mess), 0);

	title = g_strdup_printf (_("Save %s?"), filename);
	gtk_window_set_title (GTK_WINDOW (mess), title);
	g_free (title);
	g_free (filename);
	
	/* This is a bad hack, it's marked private, but tough */
	gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), FALSE);
	gtk_label_set_justify (GTK_LABEL (GTK_MESSAGE_DIALOG (mess)->label), GTK_JUSTIFY_LEFT);
	
	result = gtk_dialog_run (GTK_DIALOG (mess));
	gtk_widget_destroy (mess);
	return result;
}

gboolean
save_sound_file (const gchar* filename)
{
	/* Check if the file is default (if it's been recorded) */
	if (default_file) {
		gchar* tfile = g_build_filename (temp_dir, 
						 temp_filename_play, NULL);
		
		/* Save the file */
		run_command (_("Saving..."), "cp", "-f", tfile, filename, NULL);

		g_free (tfile);

		/* It's saved now */
		default_file = FALSE;
		save_set_sensitive (FALSE);
		file_changed = FALSE;
	}

	/* No saving is needed, because the changes go directly to the active file; don't worry, */
	/* the file is being saved first time it's edited, so you just have to do 'undo all' to restore it. */
	return TRUE;
}

guint
UpdateStatusbarPlay (gboolean begin)
{
	static gdouble length;
	static gdouble temp;
	static gfloat counter = 0.00;
	static gint countersec = 0;

	if (begin) {
		counter = 0.00;
		countersec = 0;

		length = get_play_time (active_file);
		temp = 1000 / length;
	} else {
		if (waitpid (PlayEng.pid, NULL, WNOHANG | WUNTRACED)) {
			if (playrepeat && playrepeatforever) {
				on_play_activate_cb (NULL, NULL);
				return FALSE;
			}
			else if (playrepeat && !playrepeatforever) {
				if (repeat_counter < playxtimes - 1) {
					on_play_activate_cb (NULL, NULL);
					repeat_counter++;
					return FALSE;
				}
				repeat_counter = 0;
			}
			counter = 0;
			PlayEng.is_running = FALSE;
			grecord_set_sensitive_file ();
			on_stop_activate_cb (NULL, NULL);
			return FALSE;
		}
	}

	counter += (float) temp;

	countersec++;

	set_min_sec_time (countersec);

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar), GTK_ADJUSTMENT (gtk_adjustment_new (counter, 0, 1000, 1, 1, 0)));
/*  	gtk_range_slider_update (GTK_RANGE (grecord_widgets.Statusbar)); */
	
	return TRUE;
}

guint
UpdateStatusbarRecord (gboolean begin)
{
	static gint counter = 0;
	static gint timeout = 0;
	static gint countersec = 0;

	if (begin) {
		counter = 0.00;
		timeout = 0;
		countersec = 0;
		return TRUE;
	}

	/* Timeout */
	if (counter >= record_timeout * 60) {
		if (stop_on_timeout) {
			on_stop_activate_cb (NULL, NULL);

			if (save_when_finished == TRUE) {
				save_dialog ();
			}

			return FALSE;
		}

		counter /= 2;
	}
	
	if (waitpid (RecEng.pid, NULL, WNOHANG | WUNTRACED)) {
		if (convert_is_running)
			return TRUE;

		counter = 0;
		RecEng.is_running = FALSE;
		grecord_set_sensitive_file ();
		on_stop_activate_cb (NULL, NULL);

		if (save_when_finished)
			save_dialog ();

		return FALSE;
	}

	countersec++;
	
	set_min_sec_time (countersec);
	
	if (popup_warn_mess) {
		struct stat fileinfo;
		gint maxfilesize = -1;
		gchar* filename = g_build_filename (temp_dir, 
						    temp_filename_record, NULL);
		
		if (maxfilesize == -1)
			maxfilesize = popup_warn_mess_v;
		
		stat (filename, &fileinfo);
		
		g_free (filename);
		
		if (fileinfo.st_size >= (maxfilesize * 1000000) && show_message) {
			GtkWidget *mess;
			
			mess = gtk_message_dialog_new (grecord_widgets.grecord_window,
						       0, GTK_MESSAGE_WARNING,
						       GTK_BUTTONS_OK,
						       _("The size of the current sample is more than\n%i Mb!"),
						       (int) (fileinfo.st_size / 1000000));
			g_signal_connect (G_OBJECT (mess), "response",
					  G_CALLBACK (gtk_widget_destroy), NULL);
			gtk_widget_show (mess);
			show_message = FALSE;
	        }			
	}
	
	if (stop_record) {
		struct stat fileinfo;
		gint maxfilesize = -1;
		gchar* filename = g_build_filename (temp_dir, 
						    temp_filename_record, NULL);

		if (maxfilesize == -1)
			maxfilesize = stop_record_v;
		
		stat (filename, &fileinfo);

		g_free (filename);

		if (fileinfo.st_size >= (maxfilesize * 1000000 /* In MB */ )) {
		  	on_stop_activate_cb (NULL, NULL);

		if (save_when_finished)
			save_dialog ();

		        return FALSE;
		}
	}

	/* Get the timeout value and convert it to seconds
	   (if it isn't already done) */
	if (timeout == 0) {
		timeout = record_timeout * 60;
	}

	counter++;

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar),
				  GTK_ADJUSTMENT (gtk_adjustment_new (counter,
								      0,
								      timeout,
								      1, 1, 0)));

	return TRUE;
}

void
save_dialog (void)
{
	GtkWidget* filesel = NULL;
	gchar* temp_file = g_build_filename (temp_dir, 
					     temp_filename_play, NULL);

	filesel = gtk_file_selection_new (_("Save sound file"));

	if (!g_strcasecmp (active_file, temp_file)) {
		gchar* home_dir = g_strdup (getenv ("HOME"));
		gchar* d_file = g_build_filename (home_dir, 
						  _(temp_filename_play),
						  NULL);

		gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), d_file);

		g_free (home_dir);
		g_free (d_file);
	}
	else
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), active_file);
		
	g_signal_connect ((GTK_FILE_SELECTION (filesel)->ok_button),
			  "clicked", G_CALLBACK (save_filename), filesel);

	g_signal_connect_swapped ((GTK_FILE_SELECTION (filesel)->cancel_button),
				  "clicked", G_CALLBACK (gtk_widget_destroy),
				  (gpointer) filesel);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	
	gtk_widget_show (filesel);

	gtk_main ();

	g_free (temp_file);
}

void
is_file_default (void)
{
	gchar* temp_string;
	temp_string = g_build_filename (temp_dir, temp_filename_play, NULL);

	if (!g_strcasecmp (temp_string, active_file)) {
	        default_file = TRUE;
	} else {
		default_file = FALSE;
	}

	g_free (temp_string);
}

void grecord_set_sensitive_file (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);

	/* Make the menu sensitive again */
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[0].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[1].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[2].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[3].widget), TRUE);
}

void grecord_set_sensitive_nofile (void)
{
	gtk_widget_set_sensitive (grecord_widgets.Record_button, able_to_record);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);
}

void grecord_set_sensitive_progress (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), TRUE);
}

void
grecord_set_sensitive_loading (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);

	/* Also make the menu insensitive, so you can't save or anything during the process */
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[0].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[1].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[2].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[3].widget), FALSE);
}

void
run_command (const gchar* appbar_comment, ...)
{
	/* Non-GTK+/GNOME types for use with non-GTK+/GNOME
	   POSIX and ANSI calls.
	*/
	pid_t load_pid;
	va_list ap;
	size_t cmdline_size = 0;
	size_t ix;
	char **cmdline = NULL;
	char *ptr = NULL;
	char first_arg = 1;
	char *cmd_name = NULL;

	/* Count the arguments; Record the address of the first
	   argument. */
	
	va_start (ap, appbar_comment);
	do {
		ptr = va_arg (ap, char *);
		if (ptr) {
			if (first_arg) {
				cmd_name = ptr;
				first_arg = 0;
			}

			cmdline_size++;
		}
	} while (ptr);
	va_end (ap);

	if (first_arg) {
		g_error ("Cannot run empty command line.");
	}

	/* Build the argument list */
	cmdline = g_malloc ((cmdline_size + 1) * sizeof (char *));
	va_start (ap, appbar_comment);
	for (ix = 0; ix < cmdline_size; ix++) {
		ptr = va_arg (ap, char *);
		cmdline[ix] = ptr;
	}
	va_end (ap);

	/* The last of the strings in {cmdline} is the NULL pointer
	   required by execvp.
	*/

	cmdline[cmdline_size] = NULL;

	/* Make the widgets insensitive */
	grecord_set_sensitive_loading ();

	/* Add a comment do the appbar about what is going on */
	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _(appbar_comment));

        load_pid = fork ();
	if (load_pid == 0) {
		/* BUG: The stderr output of this child process
		   should be captured. If the exit status is
		   nonzero, the stderr output should be displayed.
		   Errors can be caused either by execvp ()
		   failure, or by the command being executed
		*/

		/* Run the command */
		execvp (cmd_name, cmdline);

		/* If this is reached, then an error occurred. */

		perror (cmd_name);
		_exit (1);
	} else if (load_pid == -1) {
		g_free (cmdline);
		g_error (_("Could not fork child process"));
	}

	g_free (cmdline);
	
	convert_is_running = TRUE;

	/* Add a function for checking when process has died */
	gtk_timeout_add (100, (GtkFunction) check_if_loading_finished, (gpointer) load_pid);
}

guint
check_if_loading_finished (gint pid)
{
	/* Check if process still alive */
	if (waitpid (pid, NULL, WNOHANG | WUNTRACED)) {
		
		/* Show the playtime of the file */
		set_min_sec_time (get_play_time (active_file));
		
		/* Remove the comment from the appbar, because we're finished */
		gnome_appbar_clear_stack (GNOME_APPBAR (grecord_widgets.appbar));

		/* Make widgets sensitive again */
		grecord_set_sensitive_file ();

		convert_is_running = FALSE;

		return FALSE;
	}

	return TRUE;
}

gboolean
soundfile_supported (const gchar* filename)
{
	AFfilehandle filetype  = afOpenFile (filename, "r", NULL);
	gint soundtype = afGetFileFormat (filetype, NULL);

	/* Check if the file exists */
	if (!g_file_test (filename, G_FILE_TEST_EXISTS))
		return FALSE;

	/* Check if the file is a valid soundfile */
	if (soundtype == AF_FILE_UNKNOWN)
		return FALSE;

	return TRUE;
}

gboolean
check_if_sounddevice_ready ()
{
	int fd;
	
	/* Check if the sounddevice is ready */
	fd = esd_open_sound (NULL);
	 
	/* Sounddevice not ready, tell the user */
	if (fd < 0) {
		GtkWidget* mess;

		mess = gtk_message_dialog_new (NULL, 0,
					       GTK_MESSAGE_ERROR,
					       GTK_BUTTONS_OK,
					       _("The sound device is not ready. Please check that there "
						 "isn't\nanother program running that is using the device."));
		g_signal_connect (G_OBJECT (mess), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_dialog_run (GTK_DIALOG (mess));
		return FALSE;
	}

	esd_close (fd);
	return TRUE;
}
