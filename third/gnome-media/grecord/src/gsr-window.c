/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
 *           Johan Dahlin <johan@gnome.org>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <gnome.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <gconf/gconf-client.h>
#include <gst/gst.h>
#include <gst/gconf/gconf.h>
#include <bonobo/bonobo-ui-util.h>

#include <profiles/gnome-media-profiles.h>

#include "gnome-recorder.h"
#include "gsr-window.h"

extern GtkWidget * gsr_open_window (const char *filename);
extern void gsr_quit (void);

enum {
	PROP_0,
	PROP_LOCATION
};

#define GCONF_DIR           "/apps/gnome-sound-recorder/"
#define KEY_OPEN_DIR        GCONF_DIR "system-state/open-file-directory"
#define KEY_SAVE_DIR        GCONF_DIR "system-state/save-file-directory"
#define KEY_KVPS            GCONF_DIR "UIConf/kvps"
#define KEY_LAST_PROFILE_ID GCONF_DIR "last-profile-id"

#define CMD_SET_SENSITIVE(window, key, value)               \
  bonobo_ui_component_set_prop (window->priv->ui_component, \
				"/commands/" key,           \
				"sensitive", value, NULL)


typedef struct _GSRWindowPipeline {
	GstElement *pipeline;

	GstElement *src, *sink;
	guint32 state_change_id;
} GSRWindowPipeline;

struct _GSRWindowPrivate {
	GtkWidget *main_vbox, *ev;
	GtkWidget *scale;
	GtkWidget *profile;
	GtkWidget *rate, *time_sec, *format, *channels;
	GtkWidget *name, *length;

	gulong seek_id;

	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	/* Pipelines */
	GSRWindowPipeline *play, *record;
	char *filename, *record_filename;
        char *extension;
	char *working_file; /* Working file: Operations only occur on the
			       working file. The result of that operation then
			       becomes the new working file. */
	int record_fd;

	/* File info */
	int len_secs; /* In seconds */
	int get_length_attempts;

	int n_channels, bitrate, samplerate;
	gboolean has_file;
	gboolean dirty;
	gboolean seek_in_progress;

	guint32 tick_id; /* tick_callback timeout ID */
	guint32 record_id; /* record idle callback timeout ID */
	guint32 gstenc_id; /* encode idle callback iterator ID */
};

static GSRWindowPipeline * make_record_pipeline    (GSRWindow         *window);
static void                gsr_window_init         (GSRWindow         *window);
static void                gsr_window_class_init   (GSRWindowClass    *klass);
static void                gsr_window_finalize     (GObject           *object);
static void                gsr_window_get_property (GObject           *object,
						    guint              prop_id,
						    GValue            *value,
						    GParamSpec        *pspec);
static void                gsr_window_set_property (GObject           *object,
						    guint              prop_id,
						    const GValue      *value,
						    GParamSpec        *pspec);
static void                file_new                (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_open               (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_save_as            (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_save               (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_mixer              (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_properties         (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_close              (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                file_quit               (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                media_play              (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                media_stop              (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                media_record            (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);
static void                help_contents           (BonoboUIComponent *component,
						    GSRWindow         *window,
						    const char        *path);
static void                help_about              (BonoboUIComponent *uic,
						    GSRWindow         *window,
						    const char        *path);    

static BonoboWindowClass *parent_class = NULL;


static BonoboUIVerb file_verbs[] = {
	BONOBO_UI_VERB ("FileNew",        (BonoboUIVerbFn) file_new),
	BONOBO_UI_VERB ("FileOpen",       (BonoboUIVerbFn) file_open),
	
	BONOBO_UI_VERB ("FileSave",       (BonoboUIVerbFn) file_save),
	BONOBO_UI_VERB ("FileSaveAs",     (BonoboUIVerbFn) file_save_as),

	BONOBO_UI_VERB ("FileMixer",      (BonoboUIVerbFn) file_mixer),
	BONOBO_UI_VERB ("FileProperties", (BonoboUIVerbFn) file_properties),

	BONOBO_UI_VERB ("FileClose",      (BonoboUIVerbFn) file_close),
	BONOBO_UI_VERB ("FileExit",       (BonoboUIVerbFn) file_quit),

	BONOBO_UI_VERB_END
};

static BonoboUIVerb help_verbs[] = {
	BONOBO_UI_VERB ("HelpContents",   (BonoboUIVerbFn) help_contents),
	BONOBO_UI_VERB ("HelpAbout",      (BonoboUIVerbFn) help_about),

	BONOBO_UI_VERB_END
};

static BonoboUIVerb media_verbs[] = {
	BONOBO_UI_VERB ("MediaPlay",      (BonoboUIVerbFn) media_play),
	BONOBO_UI_VERB ("MediaStop",      (BonoboUIVerbFn) media_stop),
	BONOBO_UI_VERB ("MediaRecord",    (BonoboUIVerbFn) media_record),

	BONOBO_UI_VERB_END
};

static void
show_error_dialog (GtkWindow *window, gchar *error)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (window,
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_CLOSE,
					 error);
	
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static void
shutdown_pipeline (GSRWindowPipeline *pipe)
{
	if (pipe->state_change_id > 0) {
		g_signal_handler_disconnect (G_OBJECT (pipe->pipeline),
					     pipe->state_change_id);
	}
	gst_element_set_state (pipe->pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (pipe->pipeline));	
}



static char *
seconds_to_string (guint seconds)
{
	int hour, min, sec;
	
	min = (seconds / 60);
	hour = min / 60;
	min -= (hour * 60);
	sec = seconds - ((hour * 3600) + (min * 60));

	if (hour > 0) {
		return g_strdup_printf ("%d:%02d:%02d", hour, min, sec);
	} else {
		return g_strdup_printf ("%d:%02d", min, sec);
	}
}	

static char *
seconds_to_full_string (guint seconds)
{
	int hour, min, sec;

	min = (seconds / 60);
	hour = (min / 60);
	min -= (hour * 60);
	sec = seconds - ((hour * 3600) + (min * 60));

	if (hour > 0) {
		if (min > 0) {
			if (sec > 0) {
				return g_strdup_printf ("%d %s %d %s %d %s",
							hour, hour > 1 ? _("hours") : _("hour"),
							min, min > 1 ? _("minutes") : _("minute"),
							sec, sec > 1 ? _("seconds") : _("second"));
			} else {
				return g_strdup_printf ("%d %s %d %s",
							hour, hour > 1 ? _("hours") : _("hour"),
							min, min > 1 ? _("minutes") : _("minute"));
			}
		} else {
			if (sec > 0) {
				return g_strdup_printf ("%d %s %d %s",
							hour, hour > 1 ? _("hours") : _("hour"),
							sec, sec > 1 ? _("seconds") : _("second"));
			} else {
				return g_strdup_printf ("%d %s",
							hour, hour > 1 ? _("hours") : _("hour"));
			}
		}
	} else {
		if (min > 0) {
			if (sec > 0) {
				return g_strdup_printf ("%d %s %02d %s",
							min, min > 1 ? _("minutes") : _("minute"),
							sec, sec > 1 ? _("seconds") : _("second"));
			} else {
				return g_strdup_printf ("%d %s",
							min, min > 1 ? _("minutes") : _("minute"));
			}
		} else {
			if (sec == 0) {
				return g_strdup_printf ("%d %s",
							sec, _("seconds"));
			} else {
				return g_strdup_printf ("%d %s",
							sec, sec > 1 ? _("seconds") : _("second"));
			}
		}
	}

	return NULL;
}

static void
file_new (BonoboUIComponent *uic,
	  GSRWindow *window,
	  const char *path)
{
	gsr_open_window (NULL);
}

static void
file_chooser_open_response_cb (GtkDialog *file_chooser,
			       int response_id,
			       GSRWindow *window)
{
	GConfClient *client;
	char *name;
	char *dirname;

	if (response_id != GTK_RESPONSE_OK)
		goto out;
	
	name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
	if (name == NULL)
		goto out;
		
	dirname = g_path_get_dirname (name);
		
	client = gconf_client_get_default ();
	gconf_client_set_string (client, KEY_OPEN_DIR, dirname, NULL);
	g_object_unref (G_OBJECT (client));
	g_free (dirname);

	if (window->priv->has_file == TRUE) {
		/* Just open a new window with the file */
		gsr_open_window (name);
	} else {
		/* Set the file in this window */
		g_object_set (G_OBJECT (window), "location", name, NULL);
	}
		
	g_free (name);
	
 out:
	gtk_widget_destroy (GTK_WIDGET (file_chooser));
}

static void
file_open (BonoboUIComponent *uic,
	   GSRWindow *window,
	   const char *path)
{
	GtkWidget *file_chooser;
	GConfClient *client;
	char *directory;

	file_chooser = gtk_file_chooser_dialog_new (_("Open a file"),
						    GTK_WINDOW (window),
						    GTK_FILE_CHOOSER_ACTION_OPEN,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    GTK_STOCK_OPEN, GTK_RESPONSE_OK,
						    NULL);

	client = gconf_client_get_default ();
	directory = gconf_client_get_string (client, KEY_OPEN_DIR, NULL);
	if (directory != NULL && *directory != 0) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser), directory);
	}
	g_object_unref (G_OBJECT (client));
	g_free (directory);

	g_signal_connect (G_OBJECT (file_chooser), "response",
			  G_CALLBACK (file_chooser_open_response_cb), window);
	gtk_widget_show (file_chooser);
}

struct _eos_data {
	GSRWindow *window;
	char *location;
	GstElement *pipeline;
};

static gboolean
eos_done (struct _eos_data *ed)
{
	GSRWindow *window = ed->window;

 	gst_element_set_state (ed->pipeline, GST_STATE_NULL);
	if (window->priv->gstenc_id > 0) {
	    g_source_remove (window->priv->gstenc_id);
	    window->priv->gstenc_id = 0;
	}
 
	g_object_set (G_OBJECT (window),
		      "location", ed->location,
		      NULL);


	gdk_window_set_cursor (window->priv->main_vbox->window, NULL);

	CMD_SET_SENSITIVE (window, "MediaStop", "0");
	CMD_SET_SENSITIVE (window, "MediaPlay", "1");
	CMD_SET_SENSITIVE (window, "MediaRecord", "1");
	CMD_SET_SENSITIVE (window, "FileSave", "0");
	CMD_SET_SENSITIVE (window, "FileSaveAs", "1");
	gtk_widget_set_sensitive (window->priv->scale, TRUE);

	bonobo_ui_component_set_status (window->priv->ui_component,
					_("Ready"), NULL);

	g_free (ed);

	return FALSE;
}

static GstElement *
gst_element_get_child (GstElement* element, const gchar *name)
{
	const GList *kids = GST_BIN (element)->children;
	GstElement *sink;

	while (kids && !(strcmp (gst_object_get_name (GST_OBJECT (kids->data)), name)))
		kids = kids->next;
	g_assert (kids);
	sink = kids->data;

	return sink;
}

#if 0
static gboolean
cb_iterate (GstBin  *bin,
	    gpointer data)
{
	src = gst_element_get_child (bin, "sink");
	sink = gst_element_get_child (bin, "sink");
	
	if (src && sink) {
		gint64 pos, tot, enc;
		GstFormat fmt = GST_FORMAT_BYTES;

		gst_element_query (src, GST_QUERY_POSITION, &fmt, &pos);
		gst_element_query (src, GST_QUERY_TOTAL, &fmt, &tot);
		gst_element_query (sink, GST_QUERY_POSITION, &fmt, &enc);

		g_print ("Iterate: %lld/%lld -> %lld\n", pos, tot, enc);
	} else
		g_print ("Iterate ?\n");

	/* we don't do anything here */
	return FALSE;
}
#endif

static void
pipeline_error_cb (GstElement *parent,
		   GstElement *cause,
		   GError     *error,
		   gchar      *debug,
		   gpointer    data)
{
	GSRWindow *window = data;
	struct _eos_data *ed;
	GstElement *sink;

	g_return_if_fail (GTK_IS_WINDOW (window));
	
	show_error_dialog (GTK_WINDOW (window), error->message);

	ed = g_new (struct _eos_data, 1);
	ed->window = window;
	ed->pipeline = parent;

	sink = gst_element_get_child (parent, "sink");
	g_object_get (G_OBJECT (sink),
		      "location", &ed->location,
		      NULL);

	/*g_idle_add ((GSourceFunc) eos_done, ed);*/
	eos_done (ed);
}

static void
do_save_file (GSRWindow *window,
	      const char *name)
{
	GSRWindowPrivate *priv;
        GMAudioProfile *profile;
	char *tmp;
	
	priv = window->priv;
	
        profile = gm_audio_profile_choose_get_active (priv->profile);

	tmp = g_strdup_printf ("%s.%s", name,
			       gm_audio_profile_get_extension (profile));
	rename (priv->record_filename, tmp);
	g_free (tmp);		
}

static void
file_chooser_save_response_cb (GtkDialog *file_chooser,
			       int response_id,
			       GSRWindow *window)
{
	GConfClient *client;
	char *name;
	char *dirname;

	if (response_id != GTK_RESPONSE_OK)
		goto out;
	
	name = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (file_chooser));
	if (name == NULL)
		goto out;

	dirname = g_path_get_dirname (name);
	
	client = gconf_client_get_default ();
	gconf_client_set_string (client, KEY_SAVE_DIR, dirname, NULL);
	g_object_unref (G_OBJECT (client));
	g_free (dirname);
	
	do_save_file (window, name);
	g_free (name);

 out:	
	gtk_widget_destroy (GTK_WIDGET (file_chooser));
}

static void
file_save_as (BonoboUIComponent *uic,
	      GSRWindow *window,
	      const char *path)
{
	GtkWidget *file_chooser;
	GConfClient *client;
	char *directory;

	file_chooser = gtk_file_chooser_dialog_new (_("Save file as"),
						    GTK_WINDOW (window),
						    GTK_FILE_CHOOSER_ACTION_SAVE,
						    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						    GTK_STOCK_SAVE, GTK_RESPONSE_OK,
						    NULL);

	client = gconf_client_get_default ();
	directory = gconf_client_get_string (client, KEY_SAVE_DIR, NULL);
	if (directory != NULL && *directory != 0) {
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_chooser), directory);
	}
	g_object_unref (G_OBJECT (client));
	g_free (directory);

	if (window->priv->filename != NULL) {
		char *basename;

		basename = g_path_get_basename (window->priv->filename);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (file_chooser),
						   basename);
		g_free (basename);
	}

	g_signal_connect (G_OBJECT (file_chooser), "response",
			  G_CALLBACK (file_chooser_save_response_cb), window);
	gtk_widget_show (file_chooser);
}

static void
file_save (BonoboUIComponent *uic,
	   GSRWindow *window,
	   const char *path)
{
	if (window->priv->filename == NULL ||
	    strncmp (window->priv->filename, "Untitled", 8) == 0) {
		file_save_as (uic, window, NULL);
	} else {
		do_save_file (window, window->priv->filename);
	}
}

static void
file_mixer (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	char *mixer_path;
	char *argv[2] = {NULL, NULL};
	GError *error = NULL;
	gboolean ret;

	/* Open the mixer */
	mixer_path = g_find_program_in_path ("gnome-volume-control");
	if (mixer_path == NULL) {
		char *tmp;
		tmp = g_strdup_printf(_("%s is not installed in the path."), "gnome-volume-control");
		show_error_dialog (GTK_WINDOW (window), tmp);
		g_free(tmp);
		return;
	}

	argv[0] = mixer_path;
	ret = g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, NULL, &error);
	if (ret == FALSE) {
		char *tmp;
		tmp = g_strdup_printf (_("There was an error starting %s: %s"),
				       mixer_path, error->message);
		show_error_dialog (GTK_WINDOW (window), tmp);
		g_free(tmp);
		g_error_free (error);
	}

	g_free (mixer_path);
}

static GtkWidget *
make_title_label (const char *text)
{
	GtkWidget *label;
	char *fulltext;
	
	fulltext = g_strdup_printf ("<span weight=\"bold\">%s</span>", text);
	label = gtk_label_new (fulltext);
	g_free (fulltext);

	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.0);
	return label;
}

static GtkWidget *
make_info_label (const char *text)
{
	GtkWidget *label;

	label = gtk_label_new (text);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_label_set_selectable (GTK_LABEL (label), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (label), GTK_WRAP_WORD);

	return label;
}

static void
pack_table_widget (GtkWidget *table,
		   GtkWidget *widget,
		   int left,
		   int top)
{
	gtk_table_attach (GTK_TABLE (table), widget,
			  left, left + 1, top, top + 1,
			  GTK_FILL, GTK_FILL, 0, 0);
}

struct _file_props {
	GtkWidget *dialog;

	GtkWidget *dirname;
	GtkWidget *filename;
	GtkWidget *size;
	GtkWidget *length;
	GtkWidget *samplerate;
	GtkWidget *channels;
	GtkWidget *bitrate;
};

static void
fill_in_information (GSRWindow *window,
		     struct _file_props *fp)
{
	char *text;
	char *name;
	struct stat buf;

	/* dirname */
	text = g_path_get_dirname (window->priv->filename);
	gtk_label_set_text (GTK_LABEL (fp->dirname), text);
	g_free (text);

	/* filename */
	name = g_path_get_basename (window->priv->filename);
	if (window->priv->dirty) {
		text = g_strdup_printf (_("%s (Has not been saved)"), name);
	} else {
		text = g_strdup (name);
	}
	gtk_label_set_text (GTK_LABEL (fp->filename), text);
	g_free (text);
	g_free (name);
	
	/* Size */
	if (stat (window->priv->working_file, &buf) == 0) {
		char *human;
		human = gnome_vfs_format_file_size_for_display (buf.st_size);

		text = g_strdup_printf ("%s (%llu bytes)", human, buf.st_size);
		g_free (human);
	} else {
		text = g_strdup (_("Unknown size"));
	}
	gtk_label_set_text (GTK_LABEL (fp->size), text);
	g_free (text);

	/* FIXME: Set up and run our own pipeline 
	          till we can get the info */
	/* Length */
	if (window->priv->len_secs == 0) {
		text = g_strdup (_("Unknown"));
	} else {
		text = seconds_to_full_string (window->priv->len_secs);
	}
	gtk_label_set_text (GTK_LABEL (fp->length), text);
	g_free (text);

	/* sample rate */
	if (window->priv->samplerate == 0) {
		text = g_strdup (_("Unknown"));
	} else {
		text = g_strdup_printf (_("%.1f kHz"), (float) window->priv->samplerate / 1000);
	}
	gtk_label_set_text (GTK_LABEL (fp->samplerate), text);
	g_free (text);

	/* bit rate */
	if (window->priv->bitrate == 0) {
		text = g_strdup (_("Unknown"));
	} else {
		text = g_strdup_printf (_("%.0f kb/s"), (float) window->priv->bitrate / 1000);
	}
	gtk_label_set_text (GTK_LABEL (fp->bitrate), text);
	g_free (text);

	/* channels */
	switch (window->priv->n_channels) {
	case 0:
		text = g_strdup (_("Unknown"));
		break;
 
	case 1:
		text = g_strdup (_("1 (mono)"));
		break;

	case 2:
		text = g_strdup (_("2 (stereo)"));
		break;

	default:
		text = g_strdup_printf ("%d", window->priv->n_channels);
		break;
	}
	gtk_label_set_text (GTK_LABEL (fp->channels), text);
	g_free (text);
}

static void
dialog_closed_cb (GtkDialog *dialog,
		  guint response_id,
		  struct _file_props *fp)
{
	gtk_widget_destroy (fp->dialog);
	g_free (fp);
}

static void
file_properties (BonoboUIComponent *uic,
		 GSRWindow *window,
		 const char *path)
{
	GtkWidget *dialog, *vbox, *inner_vbox, *hbox, *table, *label;
	char *title, *shortname;
	struct _file_props *fp;
	shortname = g_path_get_basename (window->priv->filename);
	title = g_strdup_printf (_("%s Information"), shortname);
	g_free (shortname);

	dialog = gtk_dialog_new_with_buttons (title, GTK_WINDOW (window),
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	g_free (title);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	fp = g_new (struct _file_props, 1);
	fp->dialog = dialog;
	
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (dialog_closed_cb), fp);

	vbox = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox, TRUE, TRUE, 0);

	inner_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox), inner_vbox, FALSE, FALSE,0);

	label = make_title_label (_("File Information"));
	gtk_box_pack_start (GTK_BOX (inner_vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* File properties */	
	table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

	label = make_info_label (_("Folder:"));
	pack_table_widget (table, label, 0, 0);

	fp->dirname = make_info_label ("");
	pack_table_widget (table, fp->dirname, 1, 0);

	label = make_info_label (_("Filename:"));
	pack_table_widget (table, label, 0, 1);

	fp->filename = make_info_label ("");
	pack_table_widget (table, fp->filename, 1, 1);

	label = make_info_label (_("File size:"));
	pack_table_widget (table, label, 0, 2);

	fp->size = make_info_label ("");
	pack_table_widget (table, fp->size, 1, 2);

	inner_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox), inner_vbox, FALSE, FALSE, 0);

	label = make_title_label (_("Audio Information"));
	gtk_box_pack_start (GTK_BOX (inner_vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* Audio info */
	table = gtk_table_new (4, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

	label = make_info_label (_("Song length:"));
	pack_table_widget (table, label, 0, 0);

	fp->length = make_info_label ("");
	pack_table_widget (table, fp->length, 1, 0);

	label = make_info_label (_("Number of channels:"));
	pack_table_widget (table, label, 0, 1);

	fp->channels = make_info_label ("");
	pack_table_widget (table, fp->channels, 1, 1);

	label = make_info_label (_("Sample rate:"));
	pack_table_widget (table, label, 0, 2);

	fp->samplerate = make_info_label ("");
	pack_table_widget (table, fp->samplerate, 1, 2);

	label = make_info_label (_("Bit rate:"));
	pack_table_widget (table, label, 0, 3);

	fp->bitrate = make_info_label ("");
	pack_table_widget (table, fp->bitrate, 1, 3);

	fill_in_information (window, fp);
	gtk_widget_show_all (dialog);
}

void
gsr_window_close (GSRWindow *window)
{
	gtk_widget_destroy (GTK_WIDGET (window));
}

static void
file_close (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	gsr_window_close (window);
}

static void
file_quit (BonoboUIComponent *uic,
	   GSRWindow *window,
	   const char *path)
{
	gsr_quit ();
}

static void
help_contents (BonoboUIComponent *component,
	       GSRWindow *window,
	       const char *path)
{
	GError *error = NULL;

	gnome_help_display ("gnome-sound-recorder.xml", NULL, &error);

	if (error != NULL)
	{
		g_warning (error->message);

		g_error_free (error);
	}
}

static void
help_about (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	static GtkWidget *about = NULL;
	GdkPixbuf *pixbuf = NULL;
	const char *authors[2] = {"Iain Holmes <iain@prettypeople.org>", NULL};
	const char *documentors[2] = {"Sun Microsystems", NULL};

	if (about != NULL) {
		gdk_window_raise (about->window);
		gdk_window_show (about->window);
	} else {
		pixbuf = gdk_pixbuf_new_from_file (GNOME_ICONDIR "/gnome-grecord.png", NULL);
		about = gnome_about_new (_("Sound Recorder"), VERSION,
					 "Copyright \xc2\xa9 2002 Iain Holmes",
					 _("A sound recorder for GNOME"),
					 authors, documentors, NULL, pixbuf);

		if (pixbuf != NULL)
			gdk_pixbuf_unref (pixbuf);

		g_signal_connect (G_OBJECT (about), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &about);
		gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (window));
		gtk_widget_show (about);
	}
}

static void
media_play (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	GSRWindowPrivate *priv = window->priv;
	
	if (priv->has_file == FALSE)
		return;

	if (priv->record && gst_element_get_state (priv->record->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (priv->record->pipeline, GST_STATE_READY);
	}

	if (priv->play)
		gst_element_set_state (priv->play->pipeline, GST_STATE_PLAYING);
}

static void
media_stop (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	GSRWindowPrivate *priv = window->priv;

	/* Work out whats playing */
	if (priv->play && gst_element_get_state (priv->play->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (priv->play->pipeline, GST_STATE_READY);
	} else if (priv->record && gst_element_get_state (priv->record->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (priv->record->pipeline, GST_STATE_READY);

		g_free (priv->working_file);
		priv->working_file = g_strdup (priv->record_filename);

		g_object_set (G_OBJECT (priv->play->src),
			      "location", priv->working_file,
			      NULL);

		priv->dirty = TRUE;
		priv->has_file = TRUE;
	}
}

static void
media_record (BonoboUIComponent *uic,
	      GSRWindow *window,
	      const char *path)
{
	GSRWindowPrivate *priv = window->priv;

	if (priv->record) {
		gst_element_set_state (priv->record->pipeline, GST_STATE_NULL);
		g_object_unref (priv->record->pipeline);
	}
	priv->record = make_record_pipeline (window);

	if (priv->record != NULL) {
		g_object_set (G_OBJECT (priv->record->sink),
			      "location", priv->record_filename,
			      NULL);
	}
	
	window->priv->len_secs = 0;
	gst_element_set_state (priv->record->pipeline, GST_STATE_PLAYING);
}

static void
gsr_menu_setup (GSRWindow *window)
{
	BonoboUIComponent *uic;

	uic = window->priv->ui_component;
	bonobo_ui_component_add_verb_list_with_data (uic, file_verbs, window);
	bonobo_ui_component_add_verb_list_with_data (uic, media_verbs, window);
	bonobo_ui_component_add_verb_list_with_data (uic, help_verbs, window);
}

static gboolean
get_length (GSRWindow *window)
{
	gint64 value;
	GstFormat format = GST_FORMAT_TIME;
	gboolean query_worked = FALSE;

	if (window->priv->play->sink != NULL) {
		query_worked = gst_element_query (window->priv->play->sink,
						  GST_QUERY_TOTAL, &format, &value);
	}

	if (query_worked) {
		char *len_str;

		window->priv->len_secs = value / GST_SECOND;

		len_str = seconds_to_full_string (window->priv->len_secs);
		gtk_label_set (GTK_LABEL (window->priv->length), len_str);
		
		g_free (len_str);

		return FALSE;
	} else {
		if (window->priv->get_length_attempts-- < 1) {
			/* Attempts to get length ran out. */
			gtk_label_set (GTK_LABEL (window->priv->length), _("Unknown"));
			return FALSE;
		}
	}

	return (gst_element_get_state (window->priv->play->pipeline) == GST_STATE_PLAYING);
}

static gboolean
seek_started (GtkRange *range,GdkEventButton *event,
	      GSRWindow *window) {
	g_return_val_if_fail (window->priv != NULL, FALSE);
	window->priv->seek_in_progress = TRUE;
	return FALSE;
}

static gboolean
seek_to (GtkRange *range,GdkEventButton *gdkevent,
	 GSRWindow *window)
{
	double value = range->adjustment->value;
	gint64 time;
	GstEvent *event;
	GstElementState old_state;

	old_state = gst_element_get_state (window->priv->play->pipeline);
	if (old_state == GST_STATE_READY || old_state == GST_STATE_NULL) {
		return FALSE;
	}

	gst_element_set_state (window->priv->play->pipeline, GST_STATE_PAUSED);
	time = ((value / 100) * window->priv->len_secs) * GST_SECOND;

	event = gst_event_new_seek (GST_FORMAT_TIME | GST_SEEK_FLAG_FLUSH, time);
	gst_element_send_event (window->priv->play->sink, event);
	gst_element_set_state (window->priv->play->pipeline, old_state);
	window->priv->seek_in_progress = FALSE;

	return FALSE;
}

static gboolean
tick_callback (GSRWindow *window)
{
	int secs;
	gint64 value;
	gboolean query_worked = FALSE;
	GstFormat format = GST_FORMAT_TIME;

	if (gst_element_get_state (window->priv->play->pipeline) != GST_STATE_PLAYING) {
		/* This check stops us from doing an unnecessary query */
		return FALSE;
	}

	if (window->priv->len_secs == 0) {
		/* Check if we've exhausted the length check yet */
		if (window->priv->get_length_attempts == 0) {
			return FALSE;
		} else {
			return TRUE;
		}
	}

	if (window->priv->seek_in_progress)
		return TRUE;

	query_worked = gst_element_query (window->priv->play->sink,
					  GST_QUERY_POSITION, 
					  &format, &value);
	if (query_worked) {
		double percentage;
		secs = value / GST_SECOND;
		
		percentage = ((double) secs / (double) window->priv->len_secs) * 100.0;
		gtk_adjustment_set_value (GTK_RANGE (window->priv->scale)->adjustment, percentage + 0.5);

	}

	return (gst_element_get_state (window->priv->play->pipeline) == GST_STATE_PLAYING);
}

static gboolean
record_tick_callback (GSRWindow *window)
{
	int secs;
	gint64 value;
	gboolean query_worked = FALSE;
	GstFormat format = GST_FORMAT_TIME;

	if (gst_element_get_state (window->priv->record->pipeline) != GST_STATE_PLAYING) {
		/* This check stops us from doing an unnecessary query */
		return FALSE;
	}

	if (window->priv->seek_in_progress)
		return TRUE;

	query_worked = gst_element_query (window->priv->record->sink,
					  GST_QUERY_POSITION, 
					  &format, &value);
	if (query_worked) {
		gchar* len_str;
		secs = value / GST_SECOND;
		
		len_str = seconds_to_full_string (secs);
		window->priv->len_secs = secs;
		gtk_label_set (GTK_LABEL (window->priv->length), len_str);
		g_free (len_str);
	}

	return (gst_element_get_state (window->priv->record->pipeline) == GST_STATE_PLAYING);
}

static gboolean
play_iterate (GSRWindow *window)
{
	gboolean ret;
	ret =  gst_bin_iterate (GST_BIN (window->priv->play->pipeline));

	if (!ret)
		gst_element_set_state (window->priv->play->pipeline, GST_STATE_NULL);
	return ret;
}

static void
play_state_changed_cb (GstElement *element,
		       GstElementState old,
		       GstElementState state,
		       GSRWindow *window)
{
	switch (state) {
	case GST_STATE_PLAYING:
		g_idle_add ((GSourceFunc) play_iterate, window);
		window->priv->tick_id = g_timeout_add (200, (GSourceFunc) tick_callback, window);
		if (window->priv->len_secs == 0) {
			window->priv->get_length_attempts = 16;
			g_timeout_add (200, (GSourceFunc) get_length, window);
		}
		
		CMD_SET_SENSITIVE (window, "MediaStop", "1");
		CMD_SET_SENSITIVE (window, "MediaPlay", "0");
		CMD_SET_SENSITIVE (window, "MediaRecord", "0");
		CMD_SET_SENSITIVE (window, "FileSave", "0");
		CMD_SET_SENSITIVE (window, "FileSaveAs", "0");
		
		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Playing..."), NULL);
		gtk_widget_set_sensitive (window->priv->scale, TRUE);
		break;

	case GST_STATE_READY:
		gtk_adjustment_set_value (GTK_RANGE (window->priv->scale)->adjustment, 0.0);
		gtk_widget_set_sensitive (window->priv->scale, FALSE);
	case GST_STATE_PAUSED:
		CMD_SET_SENSITIVE (window, "MediaStop", "0");
		CMD_SET_SENSITIVE (window, "MediaPlay", "1");
		CMD_SET_SENSITIVE (window, "MediaRecord", "1");
		CMD_SET_SENSITIVE (window, "FileSave", window->priv->dirty ? "1" : "0");
		CMD_SET_SENSITIVE (window, "FileSaveAs", "1");

		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Ready"), NULL);
		break;
	default:
		break;
	}
}

static void
pipeline_deep_notify_cb (GstElement *element,
			 GstElement *orig,
			 GParamSpec *param,
			 GSRWindow *window)
{
	GSRWindowPrivate *priv;
	GObject *obj;
	const char *pname;

	obj = G_OBJECT (orig);
	priv = window->priv;
	
	pname = g_param_spec_get_name (param);
	if (strstr (pname, "channels")) {
		g_object_get (obj, "channels", &priv->n_channels, NULL);
	} else if (strstr (pname, "samplerate")) {
		g_object_get (obj,"samplerate", &priv->samplerate, NULL);
	} else if (strstr (pname, "bitrate")) {
		g_object_get (obj, "bitrate", &priv->bitrate, NULL);
	}
}

/* helper function to change the UI when file extension changes */
static void
set_extension (GSRWindow *window)
{
	gchar *short_name, *filename;
        GMAudioProfile *profile;
        g_object_get (window, "location", &filename, NULL);
	short_name = g_path_get_basename (filename);
        profile = gm_audio_profile_choose_get_active (window->priv->profile);
        g_free (window->priv->extension);
        window->priv->extension = g_strdup (gm_audio_profile_get_extension (profile));
	if (filename != NULL) {
		char *title;

		title = g_strdup_printf (_("%s.%s - Sound Recorder"), short_name, window->priv->extension);
		gtk_window_set_title (GTK_WINDOW (window), title);
		g_free (title);
	} else {
		gtk_window_set_title (GTK_WINDOW (window), _("Sound Recorder"));
	}

}

/* callback for when the recording profile has been changed */
static void
profile_changed_cb (GObject *object, GSRWindow *window)
{
  GMAudioProfile *profile;
  gchar *id;
  GConfClient *client;

  g_return_if_fail (GTK_IS_COMBO_BOX (object));
  profile = gm_audio_profile_choose_get_active (GTK_WIDGET (object));
  
  id = g_strdup (gm_audio_profile_get_id (profile));
  client = gconf_client_get_default ();
  gconf_client_set_string (client, KEY_LAST_PROFILE_ID, id, NULL);
  g_object_unref (G_OBJECT (client));
  g_free (id);

  set_extension (window);
}

static GSRWindowPipeline *
make_play_pipeline (GSRWindow *window)
{
	GSRWindowPipeline *obj;
	GstElement *pipeline;
	guint32 id;
	GstElement *spider;

	pipeline = gst_pipeline_new ("play-pipeline");
	g_signal_connect (pipeline, "deep-notify",
			  G_CALLBACK (pipeline_deep_notify_cb), window);
	g_signal_connect (pipeline, "error",
			  G_CALLBACK (pipeline_error_cb), window);
	id = g_signal_connect (pipeline, "state-change",
			       G_CALLBACK (play_state_changed_cb), window);

	obj = g_new (GSRWindowPipeline, 1);
	obj->pipeline = pipeline;
	obj->state_change_id = id;
	
	obj->src = gst_element_factory_make ("filesrc", "src");
	if (!obj->src)
	{
		g_error ("Could not find filesrc element");
		return NULL;
	}
	
 	spider = gst_element_factory_make ("spider", "spider");
	if (!spider) {
		g_error ("Could not find element spider");
		return NULL;
	}
	
	obj->sink = gst_gconf_get_default_audio_sink ();
	if (!obj->sink)
	{
		g_error ("Could not find default audio src element");
		return NULL;
	}

	gst_bin_add_many (GST_BIN (pipeline), obj->src, spider, obj->sink, NULL);
	gst_element_link_many (obj->src, spider, obj->sink, NULL);

	return obj;
}

static gboolean
record_start (gpointer user_data) 
{
	GSRWindow *window = GSR_WINDOW (user_data);


	window->priv->get_length_attempts = 16;
	g_timeout_add (200, (GSourceFunc) record_tick_callback, window);

	CMD_SET_SENSITIVE (window, "MediaStop", "1");
	CMD_SET_SENSITIVE (window, "MediaPlay", "0");
	CMD_SET_SENSITIVE (window, "MediaRecord", "0");
	CMD_SET_SENSITIVE (window, "FileSave", "0");
	CMD_SET_SENSITIVE (window, "FileSaveAs", "0");

	bonobo_ui_component_set_status (window->priv->ui_component,
					_("Recording..."), NULL);
	gtk_widget_set_sensitive (window->priv->scale, FALSE);
	window->priv->record_id = 0;
	return FALSE;
}

static void
record_state_changed_cb (GstElement *element,
			 GstElementState old,
			 GstElementState state,
			 GSRWindow *window)
{
	switch (state) {
	case GST_STATE_PLAYING:
		window->priv->record_id = g_idle_add (record_start, window);
		gtk_widget_set_sensitive (window->priv->profile, FALSE);
		break;
	case GST_STATE_READY:
		gtk_adjustment_set_value (GTK_RANGE (window->priv->scale)->adjustment, 0.0);
		gtk_widget_set_sensitive (window->priv->scale, FALSE);
		gtk_widget_set_sensitive (window->priv->profile, TRUE);
		/* fall through */
	case GST_STATE_PAUSED:
		CMD_SET_SENSITIVE (window, "MediaStop", "0");
		CMD_SET_SENSITIVE (window, "MediaPlay", "1");
		CMD_SET_SENSITIVE (window, "MediaRecord", "1");
		CMD_SET_SENSITIVE (window, "FileSave", window->priv->dirty ? "1" : "0");
		CMD_SET_SENSITIVE (window, "FileSaveAs", "1");
		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Ready"), NULL);
		gtk_widget_set_sensitive (window->priv->scale, FALSE);
		break;
	default:
		break;
	}
}

static GSRWindowPipeline *
make_record_pipeline (GSRWindow *window)
{
	GSRWindowPipeline *pipeline;
	gint32 id;
	GMAudioProfile *profile;
	gchar *pipeline_desc, *source;
	GstElement *encoder;
	
	pipeline = g_new (GSRWindowPipeline, 1);
	pipeline->pipeline = gst_thread_new ("record-pipeline");
	id = g_signal_connect (G_OBJECT (pipeline->pipeline),
			       "state-change",
			       G_CALLBACK (record_state_changed_cb),
			       window);
	pipeline->state_change_id = id;
	
	g_signal_connect (G_OBJECT (pipeline->pipeline), "error",
			  G_CALLBACK (pipeline_error_cb), window);
	
        profile = gm_audio_profile_choose_get_active (window->priv->profile);
	source = gst_gconf_get_string ("default/audiosrc");
	if (!source) {
		show_error_dialog (NULL, _("There is no default GStreamer "
				   "audio input element set - please install "
				   "the GStreamer-GConf schemas or set one "
				   "manually"));
		return NULL;
	}

	pipeline_desc = g_strdup_printf ("%s ! audioconvert ! %s",
					 gst_gconf_get_string ("default/audiosrc"),
					 gm_audio_profile_get_pipeline (profile));
	g_free (source);
	
	encoder = gst_gconf_render_bin_from_description (pipeline_desc);
	g_free (pipeline_desc);
	if (!encoder) {
		show_error_dialog (NULL, _("Failed to create GStreamer "
				   "encoder elements - check your encoding "
				   "setup"));
		return NULL;
	}
	gst_bin_add (GST_BIN (pipeline->pipeline), encoder);
	
	pipeline->sink = gst_element_factory_make ("filesink", "sink");
	if (!pipeline->sink)
	{
		show_error_dialog (NULL, _("Could not find GStreamer filesink"
				   " plugin - please install it"));
		return NULL;
	}
	gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->sink);
	
	if (!gst_element_link (encoder, pipeline->sink))
	{
		show_error_dialog (NULL, _("Failed to link encoder elements "
				   "with file output element - you probably "
				   "selected an invalid encoder"));
		return NULL;
	}
	
	return pipeline;
}

static char *
calculate_format_value (GtkScale *scale,
			double value,
			GSRWindow *window)
{
	int seconds;

	if (window->priv->record && gst_element_get_state (window->priv->record->pipeline) == GST_STATE_PLAYING) {
		seconds = value;
		return seconds_to_string (seconds);
	} else {
		seconds = window->priv->len_secs * (value / 100);
		return seconds_to_string (seconds);
	}
}
	
GType
gsr_window_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (GSRWindowClass), NULL, NULL,
			(GClassInitFunc) gsr_window_class_init, NULL, NULL,
			sizeof (GSRWindow), 0, (GInstanceInitFunc) gsr_window_init
		};
	
		type = g_type_register_static (BONOBO_TYPE_WINDOW,
					       "GSRWindow",
					       &info, 0);
	}

	return type;
}

GtkWidget *
gsr_window_new (const char *filename)
{
	GSRWindow *window;
	GtkWidget *hbox, *table, *label, *vbox;
	struct stat buf;
	char *id, *short_name;
	GConfClient *client;
	
        /* filename has been changed to be without extension */
	window = g_object_new (GSR_WINDOW_TYPE, 
			       "location", filename,
			       NULL);
        /* FIXME: check extension too */
	window->priv->filename = g_strdup (filename);
	if (stat (filename, &buf) == 0) {
		window->priv->has_file = TRUE;
	} else {
		window->priv->has_file = FALSE;
	}

	window->priv->record_filename = g_strdup_printf ("%s/gsr-record-%s-%d.XXXXXX",
							 g_get_tmp_dir(), filename, getpid ());
	window->priv->record_fd = mkstemp (window->priv->record_filename);
	close (window->priv->record_fd);

	if (window->priv->has_file == FALSE) {
		g_free (window->priv->working_file);
		window->priv->working_file = g_strdup (window->priv->record_filename);
	} else {
		g_free (window->priv->working_file);
		window->priv->working_file = g_strdup (filename);
	}

	gtk_window_set_default_size (GTK_WINDOW (window), 512, 200);
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), hbox, FALSE, FALSE, 0);
	
	window->priv->scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 0)));
	window->priv->seek_in_progress = FALSE;
	g_signal_connect (G_OBJECT (window->priv->scale), "format-value",
			  G_CALLBACK (calculate_format_value), window);
	g_signal_connect (G_OBJECT (window->priv->scale), "button_press_event",
			  G_CALLBACK (seek_started), window);
	g_signal_connect (G_OBJECT (window->priv->scale), "button_release_event",
			  G_CALLBACK (seek_to), window);

	gtk_scale_set_value_pos (GTK_SCALE (window->priv->scale), GTK_POS_BOTTOM);
	/* We can't seek until we find out the length */
	gtk_widget_set_sensitive (window->priv->scale, FALSE);

	gtk_widget_show (window->priv->scale);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), window->priv->scale, TRUE, TRUE, 0);

	vbox = gtk_vbox_new (FALSE, 7);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), vbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

	label = gtk_label_new (_("Record as"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
        window->priv->profile = gm_audio_profile_choose_new ();
	gtk_box_pack_start (GTK_BOX (hbox), window->priv->profile,
                            FALSE, FALSE, 0);
        gtk_widget_show (window->priv->profile);
	client = gconf_client_get_default ();
	id = gconf_client_get_string (client, KEY_LAST_PROFILE_ID, NULL);
	if (id) 
		gm_audio_profile_choose_set_active (window->priv->profile, id);

	g_free (id);
	g_object_unref (client);
        g_signal_connect (G_OBJECT (window->priv->profile), "changed",
                          G_CALLBACK (profile_changed_cb), window);

	label = gtk_label_new (_("File information"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	
	label = gtk_label_new ("    ");
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 0);

	label = gtk_label_new (_("Filename:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 0, 1,
			  GTK_FILL, GTK_FILL, 0, 0);

        short_name = g_path_get_basename (filename);
	window->priv->name = gtk_label_new (short_name ? short_name : _("<none>"));
	gtk_label_set_selectable (GTK_LABEL (window->priv->name), TRUE);
	gtk_label_set_line_wrap (GTK_LABEL (window->priv->name), GTK_WRAP_WORD);
	gtk_misc_set_alignment (GTK_MISC (window->priv->name), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), window->priv->name,
			  1, 2, 0, 1,
			  GTK_FILL, GTK_FILL,
			  0, 0);

	label = gtk_label_new (_("Length:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label,
			  0, 1, 1, 2,
			  GTK_FILL, 0, 0, 0);
	
	window->priv->length = gtk_label_new ("");
	gtk_label_set_selectable (GTK_LABEL (window->priv->length), TRUE);
	gtk_misc_set_alignment (GTK_MISC (window->priv->length), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), window->priv->length,
			  1, 2, 1, 2,
			  GTK_FILL, GTK_FILL,
			  0, 0);
			       
        set_extension (window);

	gtk_widget_show_all (window->priv->main_vbox);
	return GTK_WIDGET (window);
}

static void
gsr_window_init (GSRWindow *window)
{
	GSRWindowPrivate *priv;

	window->priv = g_new0 (GSRWindowPrivate, 1);
	priv = window->priv;

	priv->ui_container = bonobo_ui_container_new ();
	bonobo_window_construct (BONOBO_WINDOW (window),
				 priv->ui_container, "Gnome-Sound-Recorder", NULL);

	priv->ev = gtk_event_box_new ();
	gtk_widget_show (priv->ev);
	
	priv->main_vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (priv->main_vbox), 6);
	gtk_widget_show (priv->main_vbox);
	gtk_container_add (GTK_CONTAINER (priv->ev), priv->main_vbox);

	bonobo_window_set_contents (BONOBO_WINDOW (window), priv->ev);

	priv->ui_component = bonobo_ui_component_new ("Gnome-Sound-Recorder");
	bonobo_ui_component_set_container (priv->ui_component,
					   bonobo_object_corba_objref (BONOBO_OBJECT (priv->ui_container)),
					   NULL);

	bonobo_ui_component_freeze (priv->ui_component, NULL);
	bonobo_ui_util_set_ui (priv->ui_component, GSR_DATADIR,
			       "ui/gsr.xml", "Gnome-Sound-Recorder", NULL);
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (BONOBO_WINDOW (window)),
					  KEY_KVPS);
	bonobo_ui_component_set_status (priv->ui_component, _("Ready"), NULL);
					
	gsr_menu_setup (window);
	bonobo_ui_component_thaw (priv->ui_component, NULL);

	/* Make the pipelines */
	priv->play = make_play_pipeline (window);
	priv->record = NULL; 

	priv->len_secs = 0;
	priv->get_length_attempts = 16;
	priv->dirty = FALSE;
	priv->gstenc_id = 0;
}

static void
gsr_window_class_init (GSRWindowClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = gsr_window_finalize;
	object_class->set_property = gsr_window_set_property;
	object_class->get_property = gsr_window_get_property;

	parent_class = g_type_class_peek_parent (klass);

	g_object_class_install_property (object_class,
					 PROP_LOCATION,
					 g_param_spec_string ("location",
							      "Location",
							      "",
							      "Untitled",
							      G_PARAM_READWRITE));
}

static void
gsr_window_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GSRWindow *window;
	GSRWindowPrivate *priv;
	struct stat buf;
	char *title, *short_name;

	window = GSR_WINDOW (object);
	priv = window->priv;

	switch (prop_id) {
	case PROP_LOCATION:
		if (priv->filename != NULL) {
			if (strcmp (g_value_get_string (value), priv->filename) == 0) {
				return;
			}
		}

		g_free (priv->filename);
		g_free (priv->working_file);

		priv->filename = g_strdup (g_value_get_string (value));
		priv->working_file = g_strdup (priv->filename);
		priv->len_secs = 0;

		short_name = g_path_get_basename (priv->filename);
		if (stat (priv->filename, &buf) == 0) {
			window->priv->has_file = TRUE;
		} else {
			window->priv->has_file = FALSE;
		}

		/* Make the gui in the init? */
		if (priv->name != NULL) {
			gtk_label_set (GTK_LABEL (priv->name), short_name);
		}

		title = g_strdup_printf ("%s - Sound Recorder", short_name);
		gtk_window_set_title (GTK_WINDOW (window), title);
		g_free (title);
		g_free (short_name);
				      
		g_object_set (G_OBJECT (window->priv->play->src),
			      "location", priv->filename,
			      NULL);
		
		CMD_SET_SENSITIVE (window, "MediaPlay", window->priv->has_file ? "1" : "0");
		CMD_SET_SENSITIVE (window, "MediaStop", "0");
		CMD_SET_SENSITIVE (window, "MediaRecord", "1");
		/* CMD_SET_SENSITIVE (window, "FileSave", "0"); */
		CMD_SET_SENSITIVE (window, "FileSaveAs", window->priv->has_file ? "1" : "0");
		break;
	default:
		break;
	}
}

static void
gsr_window_get_property (GObject    *object,
			 guint       prop_id,
			 GValue     *value,
			 GParamSpec *pspec)
{
	switch (prop_id) {
	case PROP_LOCATION:
		g_value_set_string (value, GSR_WINDOW (object)->priv->filename);
		break;

	default:
		break;
	}
}

static void
gsr_window_finalize (GObject *object)
{
	GSRWindow *window;
	GSRWindowPrivate *priv;

	window = GSR_WINDOW (object);
	priv = window->priv;

	if (priv == NULL) {
		return;
	}

	if (priv->tick_id > 0) { 
		g_source_remove (priv->tick_id);
	}

	if (priv->record_id > 0) {
		g_source_remove (priv->record_id);
	}

	g_idle_remove_by_data (window);

	if (priv->play != NULL) {
		shutdown_pipeline (priv->play);
		g_free (priv->play);
	}

	if (priv->record != NULL) {
		shutdown_pipeline (priv->record);
		g_free (priv->record);
	}

	unlink (priv->record_filename);
	g_free (priv->record_filename);

	g_free (priv->working_file);
	g_free (priv->filename);
	g_free (priv);
	window->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

