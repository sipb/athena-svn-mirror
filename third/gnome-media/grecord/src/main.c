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
#include <libgnomeui/gnome-window-icon.h>

#include <gconf/gconf-client.h>

#include "gui.h"
#include "grec.h"
#include "prog.h"
#include "preferences.h"

gboolean able_to_record = TRUE;

static gchar* geometry = NULL;
static gboolean sfiles = FALSE;
static gboolean srecord = FALSE;
static gboolean splay = FALSE;

struct poptOption grec_options[] = {
	{
		"geometry",
		'\0',
		POPT_ARG_STRING,
		&geometry,
		0,
		N_("Specify the geometry of the main window"),
		N_("GEOMETRY")
	},
	{
		"file",
		'f',
		POPT_ARG_NONE,
		&sfiles,
		0,
		N_("Specify a file to be opened"),
		NULL
	},
	{
		"record",
		'r',
		POPT_ARG_NONE,
		&srecord,
		0,
		N_("Specify a file to start recording"),
		NULL
	},
	{
		"play",
		'p',
		POPT_ARG_NONE,
		&splay,
		0,
		N_("Specify a file to start playing"),
		NULL
	},
	{
		NULL,
		'\0',
		0,
		NULL,
		0,
		NULL,
		NULL
	}
};
static int
grec_save_session (GnomeClient* client, int phase,
		   GnomeSaveStyle save_stype,
		   int is_shutdown, GnomeInteractStyle interact_style,
		   int is_fast, gpointer client_data)
{
	char **argv;
	guint argc;

	argv = g_new0 (char *, 4);
	argc = 1;

	argv[0] = client_data;

	if (sfiles && active_file != NULL) {
		argv[1] = "--file";
		argv[2] = active_file;
		argc = 3;
	}

	gnome_client_set_clone_command (client, argc, argv);
	gnome_client_set_restart_command (client, argc, argv);

	return TRUE;
}

static int
grec_kill_session (GnomeClient* client,
		   gpointer client_data)
{
	char* file1 = g_build_filename (temp_dir, temp_filename_record, NULL);
	char* file2 = g_build_filename (temp_dir, temp_filename_play, NULL);
	char* file3 = g_build_filename (temp_dir, temp_filename_backup, NULL);

	remove (file1);
	remove (file2);
	remove (file3);

	g_free (file1);
	g_free (file2);
	g_free (file3);

	gtk_main_quit ();

	return TRUE;
}

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

int
main (int argc,
      char *argv[])
{
	GtkWidget* grecord_window;
	GValue value = { 0, };
    	GnomeProgram *program;
	poptContext pctx;
	GnomeClient* client;
	gchar **args = NULL;
	gboolean show_warningmess;
	GConfClient *gconf_client;
	char *p;
	
	/* i18n stuff ---------------------------------- */
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	
	program = gnome_program_init ("grecord", VERSION,
				      LIBGNOMEUI_MODULE, argc, argv,
				      GNOME_PARAM_POPT_TABLE, grec_options,
				      GNOME_PARAM_HUMAN_READABLE_NAME,
				      _("Sound recorder"),
				      GNOME_PARAM_APP_DATADIR,DATADIR,NULL);
	
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-audio2.png");
	g_value_init (&value, G_TYPE_POINTER);
    	g_object_get_property (G_OBJECT (program), GNOME_PARAM_POPT_CONTEXT, &value);
    	pctx = g_value_get_pointer (&value);
    	g_value_unset (&value);

	args = (gchar**) poptGetArgs (pctx);
	
	mwin.x = -1;
	mwin.y = -1;
	mwin.width = 400;
	mwin.height = 170;

#ifdef GEOMETRY	
	if (geometry)
		gnome_parse_geometry (geometry, &mwin.x, &mwin.y,
				      &mwin.width, &mwin.height);
#endif

	/* Session management ------------------------- */
	client = gnome_master_client ();
	g_signal_connect (client, "save_yourself", 
			  G_CALLBACK (grec_save_session), argv[0]);
	g_signal_connect (client, "die", G_CALLBACK (grec_kill_session), NULL);

	/* Load configuration */
	load_config_file ();

	/* Initate some vars */
	PlayEng.is_running = FALSE;
	RecEng.is_running = FALSE;

	if (sfiles) {
		active_file = g_strdup (args[0]);
	} else if (splay) {
		active_file = g_strdup (args[0]);
	} else if (srecord) {
		active_file = g_strdup (args[0]);
	} else {
		active_file = g_build_filename (temp_dir, 
						temp_filename_play, NULL);
	}

	/* Popup mainwindow */
	grecord_window = create_grecord_window ();

	poptFreeContext (pctx);
	
	if (splay) {
		on_play_activate_cb (NULL, NULL);
	}

	if (srecord) {
		on_record_activate_cb (NULL, NULL);
	}
	
	gtk_widget_show (grecord_window);

	gconf_client = gconf_client_get_default ();
	show_warningmess = gconf_client_get_bool (gconf_client,
						  "/apps/gnome-sound-recorder/show-warning-messages", NULL);
	g_object_unref (gconf_client);
	
	/* Check if the sox command is a path */
	if (sox_command == NULL) {
		sox_command = g_strdup ("sox");
	}

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
			
			on_preferences_activate_cb (NULL, NULL);
		}
	} else {
		g_free (p);
	}

	gtk_main ();
	
	return 0;
}

