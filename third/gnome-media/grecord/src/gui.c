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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gnome.h>

#include <gconf/gconf-client.h>

#include "grec.h"
#include "gui.h"
#include "preferences.h"
#include "sound.h"
#include "prog.h"

static GConfClient *client = NULL;
static void prefs_help_cb (GtkWidget *widget, gpointer data);

static GnomeUIInfo arkiv1_menu_uiinfo[] =
{
	GNOMEUIINFO_MENU_NEW_ITEM (N_("_New"),
				   N_("Create a new sample"),
				   on_new_activate_cb, NULL),
	GNOMEUIINFO_MENU_OPEN_ITEM (on_open_activate_cb, NULL),
	GNOMEUIINFO_MENU_SAVE_ITEM (on_save_activate_cb, NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM (on_saveas_activate_cb, NULL),
	GNOMEUIINFO_SEPARATOR,
#ifdef HAVE_MIXER
	{
		GNOME_APP_UI_ITEM, N_("Run _Mixer"),
		N_("Run GNOME Volume Control"),
		on_runmixer_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_VOLUME,
		'r',GDK_CONTROL_MASK, NULL
	},
	GNOMEUIINFO_SEPARATOR,
#endif
	GNOMEUIINFO_MENU_EXIT_ITEM (on_exit_activate_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo echo_effect_menu_uiinfo [] =
{
	{
		GNOME_APP_UI_ITEM, N_("Add echo"),
		N_("Add echo to the current sample"),
		on_add_echo_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_NONE, NULL,
		0, 0, NULL,
	},
	GNOMEUIINFO_END
};

extern gboolean able_to_record;

/*
  static GnomeUIInfo speed_effect_menu_uiinfo [] =
  {
  {
  GNOME_APP_UI_ITEM, N_("Increase speed"),
  N_("Increase the speed of the current sample"),
  on_increase_speed_activate_cb, NULL, NULL,
  GNOME_APP_PIXMAP_NONE, NULL,
  0, 0, NULL,
  },
  {
  GNOME_APP_UI_ITEM, N_("Decrease speed"),
  N_("Decrease the speed of the current sample"),
  on_decrease_speed_activate_cb, NULL, NULL,
  GNOME_APP_PIXMAP_NONE, NULL,
  0, 0, NULL,
  },
  GNOMEUIINFO_END
  };
*/

static GnomeUIInfo effects_menu_uiinfo [] =
{
	GNOMEUIINFO_SUBTREE (N_("Echo"), echo_effect_menu_uiinfo),
	/* GNOMEUIINFO_SUBTREE (N_("Speed"), speed_effect_menu_uiinfo), */
	GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu_uiinfo[] =
{
	/* GNOMEUIINFO_MENU_UNDO_ITEM (on_undo_activate_cb, NULL), */
	{
		GNOME_APP_UI_ITEM, N_("_Undo All"),
		N_("Undo all changes made on the current sample"),
		on_undoall_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_UNDO,
		0, 0, NULL,
	},
	/* GNOMEUIINFO_MENU_REDO_ITEM (on_redo_activate_cb, NULL), */
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_SUBTREE (N_("Effects"), effects_menu_uiinfo),
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (on_preferences_activate_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo hj_lp1_menu_uiinfo[] =
{
	GNOMEUIINFO_HELP ("grecord"),
	GNOMEUIINFO_MENU_ABOUT_ITEM (on_about_activate_cb, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo menubar1_uiinfo[] =
{
	GNOMEUIINFO_MENU_FILE_TREE (arkiv1_menu_uiinfo),
	GNOMEUIINFO_MENU_EDIT_TREE (edit_menu_uiinfo),
	GNOMEUIINFO_MENU_HELP_TREE (hj_lp1_menu_uiinfo),
	GNOMEUIINFO_END
};

gpointer main_menu = menubar1_uiinfo;

void
save_set_sensitive (gboolean sensitivity)
{
	gtk_widget_set_sensitive (arkiv1_menu_uiinfo[2].widget, sensitivity);
	gtk_widget_set_sensitive (arkiv1_menu_uiinfo[3].widget, sensitivity);
}

void
set_window_title (const char *filename)
{
	char *full, *base;

	base = g_path_get_basename (filename);
	full = g_strdup_printf (_("%s - Sound Recorder"), base);
	gtk_window_set_title (GTK_WINDOW (grecord_widgets.grecord_window), full);
	g_free (full);
	g_free (base);
}

GtkWidget*
create_grecord_window (void)
{
	GtkWidget* vbox;
	GtkWidget* toolbar;
	GtkWidget* tmp_toolbar_icon;
	GtkWidget* hbox1;
	GtkWidget* grecord_appbar;
	GtkWidget* mess;
	GtkWidget* info_hbox;
	GtkWidget* New_button;
	GtkWidget* Record_button;
	GtkWidget* Play_button;
	GtkWidget* Stop_button;
	GtkWidget* grecord_window;
	GtkWidget* Statusbar;
	GtkWidget* audio_format_label;
	GtkWidget* sample_rate_label;
	GtkWidget* nr_of_channels_label;
	
	GtkWidget* timespace_label;
	GtkWidget* timesec_label;
	GtkWidget* timemin_label
;
	gboolean found_file = FALSE;
	gboolean unsupported_soundfile = FALSE;
	char *audioformat_string = NULL;
	char *channels_string = NULL;
	char *temp_string = NULL;
	char *fullname;

	grecord_window = gnome_app_new ("gnome-sound-recorder", "gnome-sound-recorder");
	gtk_window_set_title (GTK_WINDOW (grecord_window), _("Sound Recorder"));
	gtk_window_set_resizable (GTK_WINDOW (grecord_window), FALSE);
	
	if (!audioformat)
		audioformat_string = g_strdup (_("16bit PCM"));
	else
		audioformat_string = g_strdup (_("8bit PCM"));

	if (!channels)
		channels_string = g_strdup (_("stereo"));
	else
		channels_string = g_strdup (_("mono"));

	/* Create menus */
	gnome_app_create_menus (GNOME_APP (grecord_window), menubar1_uiinfo);

	toolbar = gtk_toolbar_new ();
	gtk_widget_show (toolbar);

	tmp_toolbar_icon = gtk_image_new_from_stock (GTK_STOCK_NEW,
						     GTK_ICON_SIZE_BUTTON);
	New_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
						 GTK_TOOLBAR_CHILD_BUTTON,
						 NULL,
						 _("New"),
						 _("Create new sample"), NULL,
						 tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (New_button);
	
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar));

	fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
					      "gnome-media/gnome-sound-recorder/media-play.png",
					      TRUE, NULL);
	tmp_toolbar_icon = gtk_image_new_from_file (fullname);
	g_free (fullname);

	Play_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
						  GTK_TOOLBAR_CHILD_BUTTON,
						  NULL,
						  _("Play"),
						  _("Play current sample"), NULL,
						  tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (Play_button);

	fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
					      "gnome-media/gnome-sound-recorder/media-stop.png",
					      TRUE, NULL);
	tmp_toolbar_icon = gtk_image_new_from_file (fullname);
	g_free (fullname);

	Stop_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
						  GTK_TOOLBAR_CHILD_BUTTON,
						  NULL,
						  _("Stop"),
						  _("Stop playing/recording"), NULL,
						  tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (Stop_button);

	fullname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP,
					      "gnome-media/gnome-sound-recorder/media-rec.png", 
					      TRUE, NULL);
	tmp_toolbar_icon = gtk_image_new_from_file (fullname);
	g_free (fullname);
	
	Record_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar),
						    GTK_TOOLBAR_CHILD_BUTTON,
						    NULL,
						    _("Record"),
						    _("Start recording"), NULL,
						    tmp_toolbar_icon, NULL, NULL);
	if (able_to_record == FALSE) {
		gtk_widget_set_sensitive (Record_button, FALSE);
	}
	gtk_widget_show (Record_button);
      
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_widget_show (vbox);

	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox), hbox1, FALSE, FALSE, 0);

	Statusbar = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1000, 1, 1, 0)));
	gtk_widget_show (Statusbar);
	gtk_box_pack_start (GTK_BOX (hbox1), Statusbar, TRUE, TRUE, 0);
	gtk_widget_set_usize (Statusbar, -2, 0);
	gtk_scale_set_draw_value (GTK_SCALE (Statusbar), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (Statusbar), FALSE);

	timemin_label = gtk_label_new ("00");
	gtk_box_pack_start (GTK_BOX (hbox1), timemin_label, FALSE, TRUE, 4);

	timespace_label = gtk_label_new (":");
	gtk_box_pack_start (GTK_BOX (hbox1), timespace_label, FALSE, TRUE, 0);
	
	timesec_label = gtk_label_new ("00");
	gtk_box_pack_start (GTK_BOX (hbox1), timesec_label, FALSE, TRUE, 4);

	gtk_widget_show (timemin_label);
	gtk_widget_show (timespace_label);
	gtk_widget_show (timesec_label);

	info_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (info_hbox);
	gtk_box_pack_start (GTK_BOX (vbox), info_hbox, FALSE, FALSE, 0);

	temp_string = g_strconcat (_("Audio format: "), audioformat_string, NULL);
	audio_format_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), audio_format_label, TRUE, FALSE, 3);
	gtk_label_set_justify (GTK_LABEL (audio_format_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (audio_format_label), TRUE);

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	sample_rate_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), sample_rate_label, TRUE, FALSE, 3);
	gtk_label_set_justify (GTK_LABEL (sample_rate_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (sample_rate_label), TRUE);

	temp_string = g_strconcat (_("Channels: "), channels_string, NULL);
	nr_of_channels_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), nr_of_channels_label, TRUE, FALSE, 3);
	gtk_label_set_justify (GTK_LABEL (nr_of_channels_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (nr_of_channels_label), TRUE);

	gtk_widget_show (audio_format_label);
	gtk_widget_show (sample_rate_label);
	gtk_widget_show (nr_of_channels_label);

	grecord_appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);
	gtk_widget_show (grecord_appbar);	

	/* Initialize structure */
	grecord_widgets.New_button = New_button;
	grecord_widgets.Record_button = Record_button;
	grecord_widgets.Play_button = Play_button;
	grecord_widgets.Stop_button = Stop_button;
	grecord_widgets.grecord_window = grecord_window;
	grecord_widgets.Statusbar = Statusbar;
	grecord_widgets.appbar = grecord_appbar;
	grecord_widgets.audio_format_label = audio_format_label;
	grecord_widgets.sample_rate_label = sample_rate_label;
	grecord_widgets.nr_of_channels_label = nr_of_channels_label;
	grecord_widgets.timespace_label = timespace_label;
	grecord_widgets.timesec_label = timesec_label;
	grecord_widgets.timemin_label = timemin_label;

	/* Gnome stuff */
	gnome_app_set_statusbar (GNOME_APP (grecord_window),  GTK_WIDGET (grecord_appbar));
	gnome_app_set_toolbar (GNOME_APP (grecord_window), GTK_TOOLBAR (toolbar));
	gnome_app_set_contents (GNOME_APP (grecord_window), GTK_WIDGET (vbox));

	gnome_app_install_menu_hints (GNOME_APP (grecord_window), menubar1_uiinfo);

	save_set_sensitive (FALSE);
	
	/* Initiate mainwindow and set the topic */
	is_file_default ();
	found_file = g_file_test (active_file, G_FILE_TEST_EXISTS);

	if (!found_file && !default_file) {
		gchar* show_mess = g_strdup_printf (_("File '%s' doesn't exist; using default."), active_file);
		mess = gtk_message_dialog_new (NULL,
					       GTK_DIALOG_MODAL,
					       GTK_MESSAGE_WARNING,
					       GTK_BUTTONS_OK, 
					       show_mess);
		g_free (show_mess);
		gtk_dialog_run (GTK_DIALOG(mess));
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		gtk_widget_destroy (mess);

		grecord_set_sensitive_nofile ();
	}
	else if (!found_file && default_file) {
		gtk_widget_set_sensitive (GTK_WIDGET (Play_button), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (Stop_button), FALSE);
	}
	else if (found_file && !default_file) {
		if (!soundfile_supported (active_file))
			unsupported_soundfile = TRUE;
	}
	
	if ((found_file || default_file) && unsupported_soundfile) {
		gchar* show_mess = g_strdup_printf (_("File '%s' isn't a supported soundfile."), active_file);
		mess = gtk_message_dialog_new (NULL,
					       GTK_DIALOG_MODAL,
					       GTK_MESSAGE_WARNING,
					       GTK_BUTTONS_OK,
					       show_mess);
		g_free (show_mess);
		gtk_dialog_run (GTK_DIALOG (mess));
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		gtk_widget_destroy (mess);
		
		grecord_set_sensitive_nofile ();
	}

	set_min_sec_time (get_play_time (active_file));
 
	/* Setup some callbacks */
	g_signal_connect (grecord_window, "delete_event", 
			  G_CALLBACK (on_exit_activate_cb), NULL);
	g_signal_connect (New_button, "clicked", 
			  G_CALLBACK (on_new_activate_cb), NULL);
	g_signal_connect (Record_button, "clicked", 
			  G_CALLBACK (on_record_activate_cb), NULL);
	g_signal_connect (Play_button, "clicked", 
			  G_CALLBACK (on_play_activate_cb), NULL);
	g_signal_connect (Stop_button, "clicked", 
			  G_CALLBACK (on_stop_activate_cb), NULL);
	
	return grecord_window;
}

GtkWidget*
create_about (void)
{
	const gchar *authors[] = {
		/* if your charset allows it, replace the "e" of "Hyden"
		 *  by an "eacute" (U00E9) */
		N_("Andreas Hyden <a.hyden@cyberpoint.se>"),
		"Iain Holmes <iain@ximian.com>",
		NULL
	};
	const char *documenters [] = {
		NULL
	};
	const char *translator_credits = _("translator_credits");
	GtkWidget* about;
	
	authors[0]=_(authors[0]);
	about = gnome_about_new (_("Gnome Sound Recorder"), VERSION,
				/* if your charset allows it, replace the
				   "e" of Hyden by an "eacute" (U00E9) */
				 _("Copyright (C)  2000 Andreas Hyden"),
				 _("A simple soundrecorder and soundplayer for GNOME.\nDedicated to my cat, Malte."),
				 authors,
				 documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				 NULL);
	
	return about;
}

static void
response_cb (GtkDialog *dialog,
	     int response_id,
	     gpointer data)
{
	if (response_id == GTK_RESPONSE_HELP) {
		prefs_help_cb (NULL, NULL);
		return;
	}
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
record_timeout_changed (GtkAdjustment *adj,
			gpointer data)
{
	gconf_client_set_int (client, "/apps/gnome-sound-recorder/record-timeout",
			      (int) adj->value, NULL);
}

static void
stop_on_timeout_changed (GtkToggleButton *tb,
			 gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/stop-on-timeout",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
save_when_finished_changed (GtkToggleButton *tb,
			    gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/save-when-finished",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
popup_warning_changed (GtkToggleButton *tb,
		       gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/popup-warning",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
popup_warn_mess_v_changed (GtkAdjustment *adj,
			   gpointer data)
{
	gconf_client_set_int (client, "/apps/gnome-sound-recorder/popup-warning-v",
			      (int) adj->value, NULL);
}

static void
stop_record_changed (GtkToggleButton *tb,
		     gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/stop-record",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
stop_recording_v_changed (GtkAdjustment *adj,
			  gpointer data)
{
	gconf_client_set_int (client, "/apps/gnome-sound-recorder/stop-recording-v",
			      (int) adj->value, NULL);
}

static void
play_repeat_changed (GtkToggleButton *tb,
		     gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/play-repeat",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
play_repeat_forever_changed (GtkToggleButton *tb,
			     gpointer data)
{
	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/play-repeat-forever",
			       gtk_toggle_button_get_active (tb), NULL);
}

static void
playxtimes_changed (GtkToggleButton *tb,
		    gpointer data)
{
	gtk_widget_set_sensitive (propertywidgets.playxtimes_spinbutton_v,
				  gtk_toggle_button_get_active (tb));
}

static void
play_x_times_changed (GtkAdjustment *adj,
		      gpointer data)
{
	gconf_client_set_int (client, "/apps/gnome-sound-recorder/play-x-times",
			      (int) adj->value, NULL);
}

static void
sox_path_changed (GtkEntry *entry,
		  GtkWidget *button)
{
	const char *text;

	text = gtk_entry_get_text (entry);
	if (text == NULL || *text == 0) {
		gtk_widget_set_sensitive (button, FALSE);
	} else {
		gtk_widget_set_sensitive (button, TRUE);
	}
}

static void
sox_button_clicked (GtkButton *button,
		    GtkEntry *entry)
{
	const char *text;

	text = gtk_entry_get_text (entry);
	if (text == NULL || *text == 0) {
		/* This shouldn't happen */
		return;
	}

	gconf_client_set_string (client, "/apps/gnome-sound-recorder/sox-command",
				 text, NULL);
}

/* I don't like the idea of changing the temp path halfway through... */
static void
tmp_path_changed (GtkEntry *entry,
		  GtkWidget *button)
{
	const char *text;

	text = gtk_entry_get_text (entry);
	if (text == NULL || *text == 0) {
		gtk_widget_set_sensitive (button, FALSE);
	} else {
		gtk_widget_set_sensitive (button, TRUE);
	}
}

static void
tmp_button_clicked (GtkButton *button,
		    GtkEntry *entry)
{
	const char *text;

	text = gtk_entry_get_text (entry);
	if (text == NULL || *text == 0) {
		/* Again, this shouldn't happen */
		return;
	}

	gconf_client_set_string (client, "/apps/gnome-sound-recorder/tempdir", text, NULL);
}

static void
bit8_toggled (GtkToggleButton *tb,
	      gpointer data)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/audio-format",
			       TRUE, NULL);
}

static void
bit16_toggled (GtkToggleButton *tb,
	       gpointer data)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/audio-format",
			       FALSE, NULL);
}

static void
sample_rate_changed (GtkEntry *entry,
		     gpointer data)
{
	gconf_client_set_string (client, "/apps/gnome-sound-recorder/sample-rate",
				 gtk_entry_get_text (entry), NULL);
}

static void
mono_toggled (GtkToggleButton *tb,
	      gpointer data)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/channels", TRUE, NULL);
}

static void
stereo_toggled (GtkToggleButton *tb,
		gpointer data)
{
	if (gtk_toggle_button_get_active (tb) == FALSE) {
		return;
	}

	gconf_client_set_bool (client, "/apps/gnome-sound-recorder/channels", FALSE, NULL);
}

GtkWidget*
create_grecord_propertybox (void)
{
	GtkWidget *grecord_propertybox;
	GtkWidget *frame, *label;
	GtkWidget *notebook;
	GtkWidget *vbox, *inner_vbox, *hbox_vbox;
	GtkWidget *hbox;
	GtkWidget *button, *sep;
	
	GtkObject* spinbutton_adj;
	GtkWidget* RecordTimeout_spinbutton;
	GtkWidget* StopRecordOnTimeout_checkbox;
	GtkWidget* PopupSaveOnTimeout_checkbox;
	GtkWidget* PopupWarnMessSize_checkbox;
	GtkWidget* WarningSize_spinbutton;
	GtkWidget* StopRecordSize_checkbox;
	GtkWidget* StopRecordSize_spinbutton;
	GtkWidget* TempDir_fileentry;
	GtkWidget* TempDir_combo_entry;
	GtkWidget *bit16_radiobutton;
	GtkWidget *bit8_radiobutton;
	GtkWidget* Samplerate_combo;
	GtkWidget* Samplerate_combo_entry;
	GtkWidget* path_to_sox_fileentry;
	GtkWidget* path_to_sox_combo_entry;
	GtkWidget* mainwindow_gui_vbox;
	GtkWidget* show_time_checkbutton;
	GtkWidget* show_soundinfo_checkbutton;
	GtkWidget* playrepeat_radiobutton;
	GtkWidget* playrepeatforever_radiobutton;
	GtkWidget* playxtimes_radiobutton;
	GtkWidget* playxtimes_spinbutton;
	GtkWidget *mono_rb, *stereo_rb;
	GtkWidget* path_to_sox_gnomeentry;
	GtkWidget* TempDir_gnomeentry;
	
	GList *items = NULL;

	if (client == NULL) {
		client = gconf_client_get_default ();
	}
	
	grecord_propertybox = gtk_dialog_new_with_buttons (_("Gnome Sound Recorder Preferences"),
							   GTK_WINDOW (grecord_widgets.grecord_window),
							   GTK_DIALOG_DESTROY_WITH_PARENT,
							   GTK_STOCK_CLOSE,
							   GTK_RESPONSE_CLOSE,
							   GTK_STOCK_HELP,
							   GTK_RESPONSE_HELP,NULL);
	gtk_window_set_title (GTK_WINDOW (grecord_propertybox), _("Sound Recorder Preferences"));
	g_signal_connect (G_OBJECT (grecord_propertybox), "response",
			  G_CALLBACK (response_cb), NULL);

	notebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (grecord_propertybox)->vbox), notebook,
			    TRUE, TRUE, 0);
	gtk_widget_show (notebook);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	label = gtk_label_new (_("Recording"));
	gtk_widget_show (label);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
	
	frame = gtk_frame_new (_("Time"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);
	
	label = gtk_label_new_with_mnemonic (_("_Recording timeout: "));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label), 5, 0);
	
	spinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
	g_signal_connect (G_OBJECT (spinbutton_adj), "value-changed",
			  G_CALLBACK (record_timeout_changed), NULL);
	
	RecordTimeout_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), RecordTimeout_spinbutton);
	gtk_widget_show (RecordTimeout_spinbutton);
	gtk_box_pack_start (GTK_BOX (hbox), RecordTimeout_spinbutton, FALSE, FALSE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (RecordTimeout_spinbutton), TRUE);
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, RecordTimeout_spinbutton, ATK_RELATION_LABELLED_BY);
	
	label = gtk_label_new (_("minutes"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label), 5, 0);
	
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, RecordTimeout_spinbutton, ATK_RELATION_LABELLED_BY);
	
	StopRecordOnTimeout_checkbox = gtk_check_button_new_with_mnemonic (_("_Stop recording on timeout"));

	g_signal_connect (G_OBJECT (StopRecordOnTimeout_checkbox), "toggled",
			  G_CALLBACK (stop_on_timeout_changed), NULL);
	gtk_widget_show (StopRecordOnTimeout_checkbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), StopRecordOnTimeout_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (StopRecordOnTimeout_checkbox), 3);
	
	PopupSaveOnTimeout_checkbox = gtk_check_button_new_with_mnemonic (_("_Open save dialog when recording is finished"));
	g_signal_connect (G_OBJECT (PopupSaveOnTimeout_checkbox), "toggled",
			  G_CALLBACK (save_when_finished_changed), NULL);
	gtk_widget_show (PopupSaveOnTimeout_checkbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), PopupSaveOnTimeout_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (PopupSaveOnTimeout_checkbox), 3);
	
	frame = gtk_frame_new (_("Size"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
	
	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);
	
	PopupWarnMessSize_checkbox = gtk_check_button_new_with_mnemonic (_("Show warning _message if size (MB) of sample becomes bigger than:"));
	g_signal_connect (G_OBJECT (PopupWarnMessSize_checkbox), "toggled",
			  G_CALLBACK (popup_warning_changed), NULL);
			  
	gtk_widget_show (PopupWarnMessSize_checkbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), PopupWarnMessSize_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (PopupWarnMessSize_checkbox), 3);

	spinbutton_adj = gtk_adjustment_new (1, 0, 1000, 1, 10, 10);
	g_signal_connect (G_OBJECT (spinbutton_adj), "value-changed",
			  G_CALLBACK (popup_warn_mess_v_changed), NULL);
	WarningSize_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (WarningSize_spinbutton);
	gtk_container_add (GTK_CONTAINER (inner_vbox), WarningSize_spinbutton);
	
	add_paired_relations (PopupWarnMessSize_checkbox, ATK_RELATION_CONTROLLER_FOR,
			      WarningSize_spinbutton, ATK_RELATION_CONTROLLED_BY);
	
	StopRecordSize_checkbox = gtk_check_button_new_with_mnemonic (_("Sto_p recording if size (MB) of sample becomes bigger than:"));
	g_signal_connect (G_OBJECT (StopRecordSize_checkbox), "toggled",
			  G_CALLBACK (stop_record_changed), NULL);
	
	gtk_widget_show (StopRecordSize_checkbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), StopRecordSize_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (StopRecordSize_checkbox), 3);

	spinbutton_adj = gtk_adjustment_new (1, 0, 1000, 1, 10, 10);
	g_signal_connect (G_OBJECT (spinbutton_adj), "value-changed",
			  G_CALLBACK (stop_recording_v_changed), NULL);
	StopRecordSize_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (StopRecordSize_spinbutton);
	gtk_container_add (GTK_CONTAINER (inner_vbox), StopRecordSize_spinbutton);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (StopRecordSize_spinbutton), TRUE);

	add_paired_relations (StopRecordSize_checkbox, ATK_RELATION_CONTROLLER_FOR,
			      StopRecordSize_spinbutton, ATK_RELATION_CONTROLLED_BY);
	
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	label = gtk_label_new (_("Playing"));
	gtk_widget_show (label);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	frame = gtk_frame_new (_("Repetition"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);

	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);

#if 0
	playrepeat_checkbox = gtk_check_button_new_with_mnemonic (_("_Repeat the sound"));
	g_signal_connect (G_OBJECT (playrepeat_checkbox), "toggled",
			  G_CALLBACK (play_repeat_changed), NULL);
	gtk_widget_show (playrepeat_checkbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), playrepeat_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (playrepeat_checkbox), 3);
#endif
	playrepeat_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("Play the sound _once only."));
	gtk_box_pack_start (GTK_BOX (inner_vbox), playrepeat_radiobutton, FALSE, FALSE, 0);
	gtk_widget_show (playrepeat_radiobutton);
	g_signal_connect (G_OBJECT (playrepeat_radiobutton), "toggled",
			  G_CALLBACK (play_repeat_changed), NULL);
	
	playrepeatforever_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (playrepeat_radiobutton), _("Repeat _forever"));
	g_signal_connect (G_OBJECT (playrepeatforever_radiobutton), "toggled",
			  G_CALLBACK (play_repeat_forever_changed), NULL);
	
	gtk_widget_show (playrepeatforever_radiobutton);
	gtk_box_pack_start (GTK_BOX (inner_vbox), playrepeatforever_radiobutton, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);

	playxtimes_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (playrepeatforever_radiobutton), _("_Number of times:"));
	gtk_widget_show (playxtimes_radiobutton);
	g_signal_connect (G_OBJECT (playxtimes_radiobutton), "toggled",
			  G_CALLBACK (playxtimes_changed), NULL);
	
	gtk_box_pack_start (GTK_BOX (hbox), playxtimes_radiobutton, FALSE, FALSE, 0);

	spinbutton_adj = gtk_adjustment_new (1, 1, 1000, 1, 10, 10);
	g_signal_connect (G_OBJECT (spinbutton_adj), "value-changed",
			  G_CALLBACK (play_x_times_changed), NULL);
	
	playxtimes_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (playxtimes_spinbutton);
	gtk_box_pack_start (GTK_BOX (hbox), playxtimes_spinbutton, TRUE, TRUE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (playxtimes_spinbutton), TRUE);
	
	add_paired_relations (playxtimes_radiobutton, ATK_RELATION_LABEL_FOR, playxtimes_spinbutton, ATK_RELATION_LABELLED_BY);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	label = gtk_label_new (_("Paths"));
	gtk_widget_show (label);
	
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
     
	frame = gtk_frame_new (_("Program files"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
	
	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new_with_mnemonic (_("_Path to sox:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	path_to_sox_fileentry = gnome_file_entry_new (NULL, NULL);
	gtk_widget_show (path_to_sox_fileentry);
	gtk_box_pack_start (GTK_BOX (hbox), path_to_sox_fileentry, TRUE, TRUE, 0);

	path_to_sox_combo_entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (path_to_sox_fileentry));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), path_to_sox_combo_entry);
	
	path_to_sox_gnomeentry = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (path_to_sox_fileentry));
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, path_to_sox_gnomeentry, ATK_RELATION_LABELLED_BY);
	
	button = gtk_button_new_with_mnemonic (_("_Apply"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	g_signal_connect (G_OBJECT (path_to_sox_combo_entry), "changed",
			  G_CALLBACK (sox_path_changed), button);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (sox_button_clicked), path_to_sox_combo_entry);

	frame = gtk_frame_new (_("Folders"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
	
	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);
	
	label = gtk_label_new_with_mnemonic (_("_Temporary folder:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	TempDir_fileentry = gnome_file_entry_new (NULL, NULL);
	gtk_widget_show (TempDir_fileentry);
	gtk_box_pack_start (GTK_BOX (hbox), TempDir_fileentry, TRUE, TRUE, 0);
	
	TempDir_combo_entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (TempDir_fileentry));
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), TempDir_combo_entry);
	
	TempDir_gnomeentry = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (TempDir_fileentry));
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, TempDir_gnomeentry, ATK_RELATION_LABELLED_BY);
	
	button = gtk_button_new_with_label (_("Apply"));
	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);

	g_signal_connect (G_OBJECT (TempDir_combo_entry), "changed",
			  G_CALLBACK (tmp_path_changed), button);
	g_signal_connect (G_OBJECT (button), "clicked",
			  G_CALLBACK (tmp_button_clicked), TempDir_combo_entry);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);

	label = gtk_label_new (_("Sound"));
	gtk_widget_show (label);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);
	
	frame = gtk_frame_new (_("Sound options"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
	
	inner_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (inner_vbox);
	gtk_container_add (GTK_CONTAINER (frame), inner_vbox);

	label = gtk_label_new (_("Note: These options only take effect whenever a new sound sample\n"
				 "is created. They do not operate on an existing sample."));
	gtk_box_pack_start (GTK_BOX (inner_vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);

	sep = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX (inner_vbox), sep, FALSE, FALSE, 2);
	gtk_widget_show (sep);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);
	
	label = gtk_label_new (_("Audio format:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	hbox_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (hbox_vbox);
	gtk_box_pack_start (GTK_BOX (hbox), hbox_vbox, TRUE, TRUE, 0);

	bit8_radiobutton = gtk_radio_button_new_with_mnemonic (NULL, _("8 _bit PCM"));
	g_signal_connect (G_OBJECT (bit8_radiobutton), "toggled",
			  G_CALLBACK (bit8_toggled), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_vbox), bit8_radiobutton, FALSE, FALSE, 0);
	gtk_widget_show (bit8_radiobutton);
	
	bit16_radiobutton = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (bit8_radiobutton), _("16 b_it PCM"));
	g_signal_connect (G_OBJECT (bit16_radiobutton), "toggled",
			  G_CALLBACK (bit16_toggled), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_vbox), bit16_radiobutton, FALSE, FALSE, 0);
	gtk_widget_show (bit16_radiobutton);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);
	
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, bit8_radiobutton, ATK_RELATION_LABELLED_BY);
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, bit16_radiobutton, ATK_RELATION_LABELLED_BY);
	
	/* If only combos didn't suck! */
	label = gtk_label_new_with_mnemonic (_("S_ample rate:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	
	Samplerate_combo = gtk_combo_new ();
	gtk_widget_show (Samplerate_combo);
	gtk_box_pack_start (GTK_BOX (hbox), Samplerate_combo, FALSE, TRUE, 0);
	items = g_list_append (NULL, "8000");
	items = g_list_append (items, "11025");
	items = g_list_append (items, "16000");
	items = g_list_append (items, "22050");
	items = g_list_append (items, "32000");
	items = g_list_append (items, "44100");
	items = g_list_append (items, "48000");
	gtk_combo_set_popdown_strings (GTK_COMBO (Samplerate_combo), items);
	g_list_free (items);
	gtk_container_set_border_width (GTK_CONTAINER (Samplerate_combo), 7);
	
	Samplerate_combo_entry = GTK_COMBO (Samplerate_combo)->entry;
	g_signal_connect (G_OBJECT (Samplerate_combo_entry), "changed",
			  G_CALLBACK (sample_rate_changed), NULL);
	gtk_widget_show (Samplerate_combo_entry);
	gtk_entry_set_text (GTK_ENTRY (Samplerate_combo_entry), _(samplerate));
	
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), Samplerate_combo_entry);
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, Samplerate_combo, ATK_RELATION_LABELLED_BY);
	
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);
	
	label = gtk_label_new (_("Mono or Stereo:"));
	gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	hbox_vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), hbox_vbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox_vbox);

	mono_rb = gtk_radio_button_new_with_mnemonic (NULL, _("_Mono"));
	g_signal_connect (G_OBJECT (mono_rb), "toggled",
			  G_CALLBACK (mono_toggled), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_vbox), mono_rb, FALSE, FALSE, 0);
	gtk_widget_show (mono_rb);

	stereo_rb = gtk_radio_button_new_with_mnemonic_from_widget (GTK_RADIO_BUTTON (mono_rb), _("_Stereo"));
	g_signal_connect (G_OBJECT (stereo_rb), "toggled",
			  G_CALLBACK (stereo_toggled), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_vbox), stereo_rb, FALSE, FALSE, 0);
	gtk_widget_show (stereo_rb);
	
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, mono_rb, ATK_RELATION_LABELLED_BY);
	add_paired_relations (label, ATK_RELATION_LABEL_FOR, stereo_rb, ATK_RELATION_LABELLED_BY);
	
	if (channels) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mono_rb), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (stereo_rb), TRUE);
	}

#if 0
	/* Radiobuttons, not combo */
	NrChannel_combo = gtk_combo_new ();
	gtk_widget_show (NrChannel_combo);
	gtk_box_pack_start (GTK_BOX (hbox), NrChannel_combo, FALSE, FALSE, 0);
	items = g_list_append (NULL, _("Mono"));
	items = g_list_append (items, _("Stereo"));
	gtk_combo_set_popdown_strings (GTK_COMBO (NrChannel_combo), items);
	g_list_free (items);
	gtk_container_set_border_width (GTK_CONTAINER (NrChannel_combo), 7);
	
	NrChannel_combo_entry = GTK_COMBO (NrChannel_combo)->entry;
	gtk_widget_show (NrChannel_combo_entry);
	gtk_entry_set_editable (GTK_ENTRY (NrChannel_combo_entry), FALSE);
	g_signal_connect (G_OBJECT (NrChannel_combo_entry), "changed",
			  G_CALLBACK (channels_changed), NULL);
#endif	

	propertywidgets.RecordTimeout_spinbutton_v = RecordTimeout_spinbutton;
	propertywidgets.StopRecordOnTimeout_checkbox_v = StopRecordOnTimeout_checkbox;
	propertywidgets.PopupSaveOnTimeout_checkbox_v = PopupSaveOnTimeout_checkbox;
	propertywidgets.PopupWarnMessSize_checkbox_v = PopupWarnMessSize_checkbox;
	propertywidgets.WarningSize_spinbutton_v = WarningSize_spinbutton;
	propertywidgets.StopRecordSize_checkbox_v = StopRecordSize_checkbox;
	propertywidgets.StopRecordSize_spinbutton_v = StopRecordSize_spinbutton;

	propertywidgets.playrepeat_checkbox_v = playrepeat_radiobutton;
	propertywidgets.playrepeatforever_radiobutton_v = playrepeatforever_radiobutton;
	propertywidgets.playxtimes_radiobutton_v = playxtimes_radiobutton;
	propertywidgets.playxtimes_spinbutton_v = playxtimes_spinbutton;

	propertywidgets.Sox_fileentry_v = path_to_sox_fileentry;
	propertywidgets.TempDir_fileentry_v = TempDir_fileentry;

	propertywidgets.Samplerate_combo_entry_v = Samplerate_combo_entry;

	gtk_entry_set_text (GTK_ENTRY (path_to_sox_combo_entry), sox_command);
	gtk_entry_set_text (GTK_ENTRY (TempDir_combo_entry), temp_dir);	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (RecordTimeout_spinbutton), record_timeout);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (WarningSize_spinbutton), popup_warn_mess_v);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (StopRecordSize_spinbutton), stop_record_v);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (playxtimes_spinbutton), playxtimes);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (StopRecordOnTimeout_checkbox), stop_on_timeout);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PopupSaveOnTimeout_checkbox), save_when_finished);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PopupWarnMessSize_checkbox), popup_warn_mess);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (StopRecordSize_checkbox), stop_record);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playrepeat_radiobutton), playrepeat);

	if (playrepeat) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playrepeat_radiobutton), TRUE);
	} else if (playrepeatforever) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playrepeatforever_radiobutton), TRUE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playxtimes_radiobutton), TRUE);
	}
	
	if (!popup_warn_mess)
		gtk_widget_set_sensitive (WarningSize_spinbutton, FALSE);
	if (!stop_record)
		gtk_widget_set_sensitive (StopRecordSize_spinbutton, FALSE);

	if (playrepeatforever || playrepeat)
		gtk_widget_set_sensitive (playxtimes_spinbutton, FALSE);

	g_signal_connect (PopupWarnMessSize_checkbox, "clicked", 
			  G_CALLBACK (on_checkbox_clicked_activate_cb), 
			  WarningSize_spinbutton);
	g_signal_connect (StopRecordSize_checkbox, "clicked", 
			  G_CALLBACK (on_checkbox_clicked_activate_cb), 
			  StopRecordSize_spinbutton);

	return grecord_propertybox;
}

void
add_relation (AtkRelationSet* relations, AtkRelationType relation_type,
             AtkObject* target_accessible)
{
	AtkRelation* relation;
	
	relation = atk_relation_set_get_relation_by_type (relations, relation_type);
	
	if (relation) {
		/* add new target accessible to relation */
		GPtrArray* target_array = atk_relation_get_target (relation);
	
		g_ptr_array_remove (target_array, target_accessible);
		g_ptr_array_add (target_array, target_accessible);
	} else {
		/* the relation hasn't been created yet ... */
		relation = atk_relation_new (&target_accessible, 1, relation_type);
		atk_relation_set_add (relations, relation);
		g_object_unref (relation);
	}
}

/**
 * add_paired_relations:
 * @target1: a #GtkWidget
 * @target1_type: an #AtkRelationType
 * @target2: a #GtkWidget
 * @target2_type: an #AtkRelationType
 *
 * This function adds the relationship between the objects to support
 * accessibility
 *
 **/
void
add_paired_relations (GtkWidget* target1, AtkRelationType target1_type,
	GtkWidget* target2, AtkRelationType target2_type)
{
	AtkObject* atk_target1;
	AtkObject* atk_target2;
	AtkRelationSet* target1_relation_set;
	AtkRelationSet* target2_relation_set;
	
	atk_target1 = gtk_widget_get_accessible (target1);
	if (!GTK_IS_ACCESSIBLE (atk_target1))
		return;
	atk_target2 = gtk_widget_get_accessible (target2);
	if (!GTK_IS_ACCESSIBLE (atk_target2))
		return;
		
	target1_relation_set = atk_object_ref_relation_set (atk_target1);
	add_relation (target1_relation_set, target1_type, atk_target2);
	
	target2_relation_set = atk_object_ref_relation_set (atk_target2);
	add_relation (target2_relation_set, target2_type, atk_target1);
}

static void
prefs_help_cb (GtkWidget *widget,
	       gpointer data)
{
	GError *error = NULL;
	gnome_help_display ("grecord", "grecord-prefs", &error);

	if (error) {
		g_warning ("help error: %s\n", error->message);
		g_error_free (error);
	}
}
