/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@prettypeople.org>
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
#include <bonobo/bonobo-ui-util.h>

#include "gnome-recorder.h"
#include "gsr-window.h"

enum {
	PROP_0,
	PROP_LOCATION
};

typedef struct _GSRWindowPipeline {
	GstElement *pipeline;

	GstElement *src, *sink;
} GSRWindowPipeline;

struct _GSRWindowPrivate {
	GtkWidget *main_vbox, *ev;
	GtkWidget *scale;
	GtkWidget *rate, *time_sec, *format, *channels;
	GtkWidget *name, *length;

	gulong seek_id;

	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	/* Pipelines */
	GSRWindowPipeline *play, *record;
	char *filename, *record_filename;
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
};

static BonoboWindowClass *parent_class = NULL;

static char *temppath = NULL;

static void
finalize (GObject *object)
{
	GSRWindow *window;
	GSRWindowPrivate *priv;

	window = GSR_WINDOW (object);
	priv = window->priv;

	if (priv == NULL) {
		return;
	}

	gst_element_set_state (priv->play->pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (priv->play->pipeline));
	g_free (priv->play);

	gst_element_set_state (priv->record->pipeline, GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (priv->record->pipeline));
	g_free (priv->record);

	unlink (priv->record_filename);
	g_free (priv->record_filename);

	g_free (priv->working_file);
	g_free (priv->filename);
	g_free (priv);
	window->priv = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
set_property (GObject *object,
	      guint prop_id,
	      const GValue *value,
	      GParamSpec *pspec)
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
		
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaPlay", 
					      "sensitive", window->priv->has_file ? "1" : "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaStop", "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaRecord", "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSaveAs", 
					      "sensitive", window->priv->has_file ? "1" : "0", NULL);
		break;

	default:
		break;
	}
}

static void
get_property (GObject *object,
	      guint prop_id,
	      GValue *value,
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
class_init (GSRWindowClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	parent_class = g_type_class_peek_parent (klass);

	g_object_class_install_property (object_class,
					 PROP_LOCATION,
					 g_param_spec_string ("location",
							      "Location",
							      "",
							      "Untitled",
							      G_PARAM_READWRITE));
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

struct _FileOpenData {
	GtkWidget *filesel;
	GSRWindow *window;
};

static void
file_sel_response (GtkDialog *filesel,
		   int response_id,
		   struct _FileOpenData *fd)
{
	GConfClient *client;
	const char *name;
	char *dirname, *d;

	switch (response_id) {
	case GTK_RESPONSE_OK:
		name = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

		d = g_path_get_dirname (name);
		/* Add a trailing / so that the file selector 
		   will cd into it */
		dirname = g_strdup_printf ("%s/", d);

		client = gconf_client_get_default ();
		gconf_client_set_string (client,
					 "/apps/gnome-sound-recorder/system-state/open-file-directory",
					 dirname, NULL);
		g_object_unref (G_OBJECT (client));
		g_free (dirname);
		g_free (d);

		if (fd->window->priv->has_file == TRUE) {
			/* Just open a new window with the file */
			
			gsr_open_window (name);
			
			gtk_widget_destroy (GTK_WIDGET (filesel));
			g_free (fd);
			return;
		} else {
			/* Set the file in this window */
			g_object_set (G_OBJECT (fd->window),
				      "location", name,
				      NULL);
		}

		break;

	default:
		break;
	}

	gtk_widget_destroy (GTK_WIDGET (filesel));
	g_free (fd);
}

static void
file_open (BonoboUIComponent *uic,
	   GSRWindow *window,
	   const char *path)
{
	struct _FileOpenData *fd;
	GConfClient *client;
	char *directory;

	fd = g_new (struct _FileOpenData, 1);
	fd->window = window;
	fd->filesel = gtk_file_selection_new (_("Open a file"));
	gtk_window_set_transient_for (GTK_WINDOW (fd->filesel), GTK_WINDOW (window));

	client = gconf_client_get_default ();
	directory = gconf_client_get_string (client,
					     "/apps/gnome-sound-recorder/system-state/open-file-directory",
					     NULL);
	if (directory != NULL && *directory != 0) {
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (fd->filesel),
						 directory);
	}
	g_object_unref (G_OBJECT (client));
	g_free (directory);

	g_signal_connect (G_OBJECT (fd->filesel), "response",
			  G_CALLBACK (file_sel_response), fd);
	gtk_widget_show (fd->filesel);
}

enum mimetype {
	MIME_TYPE_WAV,
	MIME_TYPE_MP3,
	MIME_TYPE_OGG,
	MIME_TYPE_FLAC
};

static GstElement *
get_encoder_for_mimetype (enum mimetype mime)
{
	switch (mime) {
	case MIME_TYPE_WAV:
		return gst_element_factory_make ("wavenc", "encoder");

	case MIME_TYPE_MP3:
		return gst_element_factory_make ("lame", "encoder");

	case MIME_TYPE_OGG:
		return gst_element_factory_make ("vorbisenc", "encoder");

	case MIME_TYPE_FLAC:
		return gst_element_factory_make ("flacenc", "encoder");
	}
}

struct _eos_data {
	GSRWindow *window;
	char *location;
};

static gboolean
eos_done (struct _eos_data *ed)
{
	GSRWindow *window = ed->window;

	g_object_set (G_OBJECT (window),
		      "location", ed->location,
		      NULL);

	gdk_window_set_cursor (window->priv->main_vbox->window, NULL);

	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaStop", 
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaPlay", 
				      "sensitive", "1", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaRecord", 
				      "sensitive", "1", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/FileSave",
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/FileSaveAs",
				      "sensitive", "1", NULL);
	gtk_widget_set_sensitive (window->priv->scale, TRUE);

	bonobo_ui_component_set_status (window->priv->ui_component,
					_("Ready"), NULL);

	g_free (ed->location);
	g_free (ed);

	return FALSE;
}

static void
save_sink_eos (GstElement *element,
	       GSRWindow *window)
{
	char *filename;
	struct _eos_data *ed;

	ed = g_new (struct _eos_data, 1);
	ed->window = window;
	g_object_get (G_OBJECT (element),
		      "location", &ed->location,
		      NULL);

	g_idle_add ((GSourceFunc) eos_done, ed);
 	gst_element_set_state (GST_ELEMENT (gst_element_get_parent (element)),
			       GST_STATE_NULL);
}

static void
do_save_file (GSRWindow *window,
	      const char *name)
{
	const char *ext;
	char *status_text, *basename;
	enum mimetype mime;
	GstElement *pipeline, *src, *encoder, *spider, *sink;
	GdkCursor *cursor;

	ext = strrchr (name, '.');
	if (ext == NULL || *ext == 0) {
		mime = MIME_TYPE_WAV;
	} else {
		if (strcmp ("wav", ext + 1) == 0) {
			mime = MIME_TYPE_WAV;
		} else if (strcmp ("mp3", ext + 1) == 0) {
			mime = MIME_TYPE_MP3;
		} else if (strcmp ("ogg", ext + 1) == 0) {
			mime = MIME_TYPE_OGG;
		} else if (strcmp ("flac", ext + 1) == 0) {
			mime = MIME_TYPE_FLAC;
		}
	}

	pipeline = gst_thread_new ("save-pipeline");
	src = gst_element_factory_make ("filesrc", "src");
	spider = gst_element_factory_make ("spider", "spider");
	encoder = get_encoder_for_mimetype (mime);
	sink = gst_element_factory_make ("filesink", "sink");

	gst_bin_add (GST_BIN (pipeline), src);
	gst_bin_add (GST_BIN (pipeline), spider);
	gst_bin_add (GST_BIN (pipeline), encoder);
	gst_bin_add (GST_BIN (pipeline), sink);

	gst_element_connect (src, spider);
	gst_element_connect (spider, encoder);
	gst_element_connect (encoder, sink);
	g_signal_connect (G_OBJECT (sink), "eos",
			  G_CALLBACK (save_sink_eos), window);

	g_object_set (G_OBJECT (src), 
		      "location", window->priv->working_file,
		      NULL);
	g_object_set (G_OBJECT (sink),
		      "location", name,
		      NULL);

	cursor = gdk_cursor_new (GDK_WATCH);
	gdk_window_set_cursor (window->priv->ev->window, cursor);
	gdk_cursor_unref (cursor);

	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaStop", 
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaPlay", 
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/MediaRecord", 
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/FileSave",
				      "sensitive", "0", NULL);
	bonobo_ui_component_set_prop (window->priv->ui_component,
				      "/commands/FileSaveAs",
				      "sensitive", "0", NULL);

	basename = g_path_get_basename (name);
	status_text = g_strdup_printf (_("Saving %s..."), basename);
	bonobo_ui_component_set_status (window->priv->ui_component,
					status_text, NULL);
	g_free (basename);
	g_free (status_text);

	gtk_widget_set_sensitive (window->priv->scale, FALSE);
	
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
}

static void
file_sel_save_response (GtkDialog *filesel,
			int response_id,
			struct _FileOpenData *fd)
{
	GConfClient *client;
	const char *name;
	char *dirname, *d;

	switch (response_id) {
	case GTK_RESPONSE_OK:
		name = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));

		d = g_path_get_dirname (name);
		/* Add a trailing / so that the file selector 
		   will cd into it */
		dirname = g_strdup_printf ("%s/", d);

		client = gconf_client_get_default ();
		gconf_client_set_string (client,
					 "/apps/gnome-sound-recorder/system-state/save-file-directory",
					 dirname, NULL);
		g_object_unref (G_OBJECT (client));
		g_free (dirname);
		g_free (d);

		do_save_file (fd->window, name);
		break;

	default:
		break;
	}

	gtk_widget_destroy (GTK_WIDGET (filesel));
	g_free (fd);
}

static void
file_save_as (BonoboUIComponent *uic,
	   GSRWindow *window,
	   const char *path)
{
	struct _FileOpenData *fd;
	GConfClient *client;
	char *directory;

	fd = g_new (struct _FileOpenData, 1);
	fd->window = window;
	fd->filesel = gtk_file_selection_new (_("Save file as"));
	gtk_window_set_transient_for (GTK_WINDOW (fd->filesel), GTK_WINDOW (window));

	client = gconf_client_get_default ();
	directory = gconf_client_get_string (client,
					     "/apps/gnome-sound-recorder/system-state/save-file-directory",
					     NULL);
	if (directory != NULL && *directory != 0) {
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (fd->filesel),
						 directory);
	}
	g_object_unref (G_OBJECT (client));
	g_free (directory);
	
	if (window->priv->filename != NULL) {
		char *basename;

		basename = g_path_get_basename (window->priv->filename);
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (fd->filesel),
						 basename);
		g_free (basename);
	}

	g_signal_connect (G_OBJECT (fd->filesel), "response",
			  G_CALLBACK (file_sel_save_response), fd);
	gtk_widget_show (fd->filesel);
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
	mixer_path = g_find_program_in_path (DEFAULT_MIXER);
	if (mixer_path == NULL) {
		g_warning (_("%s is not installed in the path."), DEFAULT_MIXER);
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
dialog_closed (GtkDialog *dialog,
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
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	fp = g_new (struct _file_props, 1);
	fp->dialog = dialog;
	
	g_signal_connect (G_OBJECT (dialog), "response",
			  G_CALLBACK (dialog_closed), fp);

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
close_window (GSRWindow *window,
	      gpointer data)
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
	const char *authors[2] = {"Iain Holmes <iain@prettypeople.org>", NULL};

	if (about != NULL) {
		gdk_window_raise (about->window);
		gdk_window_show (about->window);
	} else {
		about = gnome_about_new (_("Sound Recorder"), VERSION,
					 "Copyright \xc2\xa9 2002 Iain Holmes",
					 _("A sound recorder for GNOME"),
					 authors, NULL, NULL, NULL);
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
	if (window->priv->has_file == FALSE) {
		return;
	}

	if (gst_element_get_state (window->priv->record->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (window->priv->record->pipeline, GST_STATE_READY);
	}

	gst_element_set_state (window->priv->play->pipeline, GST_STATE_PLAYING);
}

static void
media_stop (BonoboUIComponent *uic,
	    GSRWindow *window,
	    const char *path)
{
	/* Work out whats playing */
	if (gst_element_get_state (window->priv->play->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (window->priv->play->pipeline, GST_STATE_READY);
	} else if (gst_element_get_state (window->priv->record->pipeline) == GST_STATE_PLAYING) {
		gst_element_set_state (window->priv->record->pipeline, GST_STATE_READY);

		g_free (window->priv->working_file);
		window->priv->working_file = g_strdup (window->priv->record_filename);

		g_object_set (G_OBJECT (window->priv->play->src),
			      "location", window->priv->working_file,
			      NULL);

		window->priv->dirty = TRUE;
		window->priv->has_file = TRUE;
	} else {
		/* Nothing is running */
	}
}

static void
media_record (BonoboUIComponent *uic,
	      GSRWindow *window,
	      const char *path)
{
	gst_element_set_state (window->priv->record->pipeline, GST_STATE_PLAYING);
}

static BonoboUIVerb file_verbs[] = {
	BONOBO_UI_VERB ("FileNew", (BonoboUIVerbFn) file_new),
	BONOBO_UI_VERB ("FileOpen", (BonoboUIVerbFn) file_open),
	
	BONOBO_UI_VERB ("FileSave", (BonoboUIVerbFn) file_save),
	BONOBO_UI_VERB ("FileSaveAs", (BonoboUIVerbFn) file_save_as),

	BONOBO_UI_VERB ("FileMixer", (BonoboUIVerbFn) file_mixer),
	BONOBO_UI_VERB ("FileProperties", (BonoboUIVerbFn) file_properties),

	BONOBO_UI_VERB ("FileClose", (BonoboUIVerbFn) file_close),
	BONOBO_UI_VERB ("FileExit", (BonoboUIVerbFn) file_quit),

	BONOBO_UI_VERB_END
};

static BonoboUIVerb help_verbs[] = {
	BONOBO_UI_VERB ("HelpContents", (BonoboUIVerbFn) help_contents),
	BONOBO_UI_VERB ("HelpAbout", (BonoboUIVerbFn) help_about),

	BONOBO_UI_VERB_END
};

static BonoboUIVerb media_verbs[] = {
	BONOBO_UI_VERB ("MediaPlay", (BonoboUIVerbFn) media_play),
	BONOBO_UI_VERB ("MediaStop", (BonoboUIVerbFn) media_stop),
	BONOBO_UI_VERB ("MediaRecord", (BonoboUIVerbFn) media_record),

	BONOBO_UI_VERB_END
};

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

static void
seek_to (GtkRange *range,
	 GSRWindow *window)
{
	double value = range->adjustment->value;
	gint64 time;
	GstEvent *event;
	GstElementState old_state;
	gboolean ret;

	old_state = gst_element_get_state (window->priv->play->pipeline);
	if (old_state == GST_STATE_READY) {
		return;
	}

	gst_element_set_state (window->priv->play->pipeline, GST_STATE_PAUSED);
	time = ((value / 100) * window->priv->len_secs) * GST_SECOND;
	event = gst_event_new_seek (GST_FORMAT_TIME | GST_SEEK_FLAG_FLUSH, time);
	ret = gst_element_send_event (window->priv->play->sink, event);
	gst_element_set_state (window->priv->play->pipeline, old_state);
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

	query_worked = gst_element_query (window->priv->play->sink,
					  GST_QUERY_POSITION, 
					  &format, &value);
	if (query_worked) {
		double percentage;
		secs = value / GST_SECOND;
		
		percentage = ((double) secs / (double) window->priv->len_secs) * 100.0;
		g_signal_handlers_block_matched (G_OBJECT (window->priv->scale),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL,
						 G_CALLBACK (seek_to), NULL);
		gtk_adjustment_set_value (GTK_RANGE (window->priv->scale)->adjustment, percentage + 0.5);
		g_signal_handlers_unblock_matched (G_OBJECT (window->priv->scale),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL,
						   G_CALLBACK (seek_to), NULL);

	}

	return (gst_element_get_state (window->priv->play->pipeline) == GST_STATE_PLAYING);
}

static gboolean
play_iterate (GSRWindow *window)
{
	return gst_bin_iterate (GST_BIN (window->priv->play->pipeline));
}

static void
play_state_changed (GstElement *element,
		    GstElementState old,
		    GstElementState state,
		    GSRWindow *window)
{

	switch (state) {
	case GST_STATE_PLAYING:
		g_idle_add ((GSourceFunc) play_iterate, window);
		g_timeout_add (200, (GSourceFunc) tick_callback, window);
		if (window->priv->len_secs == 0) {
			window->priv->get_length_attempts = 16;
			g_timeout_add (200, (GSourceFunc) get_length, window);
		}

		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaStop", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaPlay", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaRecord", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSave",
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSaveAs",
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Playing..."), NULL);
		gtk_widget_set_sensitive (window->priv->scale, TRUE);
		break;

	case GST_STATE_PAUSED:
	case GST_STATE_READY:
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaStop", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaPlay", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaRecord", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSave",
					      "sensitive", window->priv->dirty ? "1" : "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSaveAs",
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Ready"), NULL);
		gtk_adjustment_set_value (GTK_RANGE (window->priv->scale)->adjustment, 0.0);
		gtk_widget_set_sensitive (window->priv->scale, FALSE);
		break;
	default:
		break;
	}
}

static void
play_deep_notify (GstElement *element,
		  GstElement *orig,
		  GParamSpec *param,
		  GSRWindow *window)
{
	const char *pname;

	pname = g_param_spec_get_name (param);

	if (strstr (pname, "channels")) {
		g_object_get (G_OBJECT (orig), 
			      "channels", &window->priv->n_channels, 
			      NULL);
		return;
	}

	if (strstr (pname, "samplerate")) {
		g_object_get (G_OBJECT (orig),
			      "samplerate", &window->priv->samplerate,
			      NULL);
		return;
	}

	if (strstr (pname, "bitrate")) {
		g_object_get (G_OBJECT (orig),
			      "bitrate", &window->priv->bitrate,
			      NULL);
		return;
	}
}

static GSRWindowPipeline *
make_play_pipeline (GSRWindow *window)
{
	GstElement *spider;
	GSRWindowPipeline *pipeline = g_new (GSRWindowPipeline, 1);

	pipeline->pipeline = gst_pipeline_new ("play-pipeline");
	g_signal_connect (G_OBJECT (pipeline->pipeline), "state-change",
			  G_CALLBACK (play_state_changed), window);
	g_signal_connect (G_OBJECT (pipeline->pipeline), "deep-notify",
			  G_CALLBACK (play_deep_notify), window);

	pipeline->src = gst_element_factory_make ("filesrc", "src");
 	spider = gst_element_factory_make ("spider", "spider");
	pipeline->sink = gst_element_factory_make ("osssink", "sink");

	gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->src);
 	gst_bin_add (GST_BIN (pipeline->pipeline), spider); 
	gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->sink);

	gst_element_connect (pipeline->src, spider);
	gst_element_connect (spider, pipeline->sink);

	return pipeline;
}

static void
record_state_changed (GstElement *element,
		      GstElementState old,
		      GstElementState state,
		      GSRWindow *window)
{
	
	switch (state) {
	case GST_STATE_PLAYING:
#if 0 /* Don't need this yet...but we will eventually I think */
		if (window->priv->len_secs == 0) {
			window->priv->get_length_attempts = 16;
			g_timeout_add (200, (GSourceFunc) get_length, window);
			g_timeout_add (200, (GSourceFunc) tick_callback, window);
		}
#endif
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaStop", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaPlay", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaRecord", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSave",
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSaveAs",
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_status (window->priv->ui_component,
						_("Recording..."), NULL);
		gtk_widget_set_sensitive (window->priv->scale, FALSE);
		break;

	case GST_STATE_PAUSED:
	case GST_STATE_READY:
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaStop", 
					      "sensitive", "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaPlay", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/MediaRecord", 
					      "sensitive", "1", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSave",
					      "sensitive", window->priv->dirty ? "1" : "0", NULL);
		bonobo_ui_component_set_prop (window->priv->ui_component,
					      "/commands/FileSaveAs",
					      "sensitive", "1", NULL);
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
	GstElement *encoder;
	GSRWindowPipeline *pipeline = g_new (GSRWindowPipeline, 1);

	pipeline->pipeline = gst_thread_new ("record-pipeline");
	g_signal_connect (G_OBJECT (pipeline->pipeline), "state-change",
			  G_CALLBACK (record_state_changed), window);
	pipeline->src = gst_element_factory_make ("osssrc", "src");
	encoder = gst_element_factory_make ("wavenc", "encoder");
	if (encoder == NULL) {
		g_warning ("You do not have a new enough gst-plugins.\n"
			   "You really need CVS after 20-Oct-2002");
		return;
	}
	pipeline->sink = gst_element_factory_make ("filesink", "sink");

	gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->src);
	gst_bin_add (GST_BIN (pipeline->pipeline), encoder);
	gst_bin_add (GST_BIN (pipeline->pipeline), pipeline->sink);

	gst_element_connect (pipeline->src, encoder);
	gst_element_connect (encoder, pipeline->sink);

	return pipeline;
}

static void
init (GSRWindow *window)
{
	GSRWindowPrivate *priv;

	if (temppath == NULL) {
		temppath = g_strdup ("/tmp/");
	}

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
					  "/apps/gnome-sound-recorder/UIConf/kvps");
	bonobo_ui_component_set_status (priv->ui_component, _("Ready"), NULL);
					
	gsr_menu_setup (window);
	bonobo_ui_component_thaw (priv->ui_component, NULL);

	/* Make the pipelines */
	priv->play = make_play_pipeline (window);
	priv->record = make_record_pipeline (window);

	priv->len_secs = 0;
	priv->get_length_attempts = 16;
	priv->dirty = FALSE;
}

GType
gsr_window_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (GSRWindowClass), NULL, NULL,
			(GClassInitFunc) class_init, NULL, NULL,
			sizeof (GSRWindow), 0, (GInstanceInitFunc) init
		};
	
		type = g_type_register_static (BONOBO_TYPE_WINDOW,
					       "GSRWindow",
					       &info, 0);
	}

	return type;
}

static char *
calculate_format_value (GtkScale *scale,
			double value,
			GSRWindow *window)
{
	int seconds;

	seconds = window->priv->len_secs * (value / 100);
	return seconds_to_string (seconds);
}
	
GtkWidget *
gsr_window_new (const char *filename)
{
	GSRWindow *window;
	GtkWidget *hbox, *table, *label, *vbox;
	struct stat buf;
	char *short_name;

	window = g_object_new (GSR_WINDOW_TYPE, 
			       "location", filename,
			       NULL);
	window->priv->filename = g_strdup (filename);
	if (stat (filename, &buf) == 0) {
		window->priv->has_file = TRUE;
	} else {
		window->priv->has_file = FALSE;
	}

	short_name = g_path_get_basename (filename);
	if (filename != NULL) {
		char *title;

		title = g_strdup_printf (_("%s - Sound Recorder"), short_name);
		gtk_window_set_title (GTK_WINDOW (window), title);
		g_free (title);
	} else {
		gtk_window_set_title (GTK_WINDOW (window), _("Sound Recorder"));
	}

	window->priv->record_filename = g_strdup_printf ("%s/gsr-record-%s-%d.XXXXXX",
							 temppath, filename, getpid ());
	window->priv->record_fd = mkstemp (window->priv->record_filename);
	close (window->priv->record_fd);

	if (window->priv->has_file == FALSE) {
		g_free (window->priv->working_file);
		window->priv->working_file = g_strdup (window->priv->record_filename);
	} else {
		g_free (window->priv->working_file);
		window->priv->working_file = g_strdup (filename);
	}

	g_object_set (G_OBJECT (window->priv->record->sink),
		      "location", window->priv->record_filename,
		      NULL);

	gtk_window_set_default_size (GTK_WINDOW (window), 512, 200);
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), hbox, FALSE, FALSE, 0);
	
	window->priv->scale = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 1, 0)));
	g_signal_connect (G_OBJECT (window->priv->scale), "format-value",
			  G_CALLBACK (calculate_format_value), window);
	g_signal_connect (G_OBJECT (window->priv->scale), "value-changed",
			  G_CALLBACK (seek_to), window);

	gtk_scale_set_value_pos (GTK_SCALE (window->priv->scale), GTK_POS_BOTTOM);
	/* We can't seek until we find out the length */
	gtk_widget_set_sensitive (window->priv->scale, FALSE);

	gtk_widget_show (window->priv->scale);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), window->priv->scale, TRUE, TRUE, 0);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (window->priv->main_vbox), vbox, TRUE, TRUE, 0);

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
			       
	gtk_widget_show_all (window->priv->main_vbox);
	return GTK_WIDGET (window);
}
