/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 2002 Thomas Vander Stichele
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more view.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* audio-view.c - audio view code to be shared among implementations
 */

#include <config.h>

#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktreeview.h>


#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnome/gnome-macros.h>

#include <gst/gconf/gconf.h>

#include <string.h>

#include "i18n-support.h"
#include "../media-info/media-info.h"
#include "audio-play.h"
#include "audio-view.h"

#define SCAN_LENGTH	5	/* FIXME: gconf key ? */
/* list columns */
enum {
	FILE_COLUMN,
	TYPE_COLUMN,
	LENGTH_COLUMN,
	BITRATE_COLUMN,
	METADATA_COLUMN,
	PATH_URI_COLUMN,
	AUDIO_INFO_COLUMN,
	NUM_COLUMNS
};

typedef enum
{
  AUDIO_VIEW_INFO_STATE_NULL,
  AUDIO_VIEW_INFO_STATE_START,
  AUDIO_VIEW_INFO_STATE_IDLER,
  AUDIO_VIEW_INFO_STATE_DONE
} AudioViewInfoState;

struct AudioView {
	GtkWidget *widget;	/* top-level view widget */

	GtkWidget *scroll_window;
	GtkListStore *list_store;
	GtkWidget *tree_view;

	GtkWidget *control;
	GtkWidget *prev_button;
	GtkWidget *stop_button;
	GtkWidget *play_button;
	GtkWidget *next_button;
	GtkWidget *scan_button;
	GtkWidget *seek_scale;
	/* GtkAdjustment *seek_adj; */
	GtkWidget *time;

	GtkWidget *status;

	GtkWidget *event_box;
	char *location;
	char *selection;	/* which track do you want to play ? */

	gulong seek_changed_id;
	gdouble last_seek_value; /* so we can see if we changed the slider
				    or the user */
	GList *audio_list;
	AudioPlay *audio_play;
	gboolean scanning;
	guint scan_timeout;
	GstMediaInfo *media_info;

	/* error messages for later display */
	gchar *error_message;

	/* state information for idler */
	guint media_info_idler_id;
	AudioViewInfoState state;
	GstMediaInfoStream *stream;	/* set by idler read */
	GList *audio_item;	/* pointer to current audio_list item under
				   inspection */
	GtkTreeIter *iterp;
};

/* forward declarations */
static void	audio_view_set_time	(AudioView *view,
		                         gint seconds, gint length);

/* debug functions */

#ifdef DEBUG
gboolean print_model_row (GtkTreeModel *model, GtkTreePath *path,
		          GtkTreeIter *iter, gpointer data)
{
	GstMediaInfoStream *info;

	gchar *name = gtk_tree_path_to_string (path);
	g_print ("print_model_row: path %s\n", name);
	g_free (name);
	gtk_tree_model_get (model, iter,
			    AUDIO_INFO_COLUMN, &info,
			    -1);
	g_print ("info: %p - ", info);
	if (info)
		g_print ("path: %s\n", info->path);
	else
		g_print ("NULL\n");
}

static void
audio_view_tree_model_dump (AudioView *view)
{
	GtkTreeModel *model;
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->tree_view));
	g_assert (GTK_IS_TREE_MODEL (model));
	gtk_tree_model_foreach (model, print_model_row, NULL);
}
#endif

/* helper functions */
static GtkWidget *
control_box_add_labeled_button (GtkWidget *control,
		                const gchar *label, const gchar *file,
				gboolean toggle)
{
	GtkWidget *button;
	GdkPixbuf *pixbuf;
	GtkWidget *image;
	gchar *location;

	if (toggle)
	{
		button = gtk_toggle_button_new ();
	}
	else
	{
		button = gtk_button_new ();
	}

	location = g_strdup_printf ("%s/%s", PIXMAPSDIR, file);
	pixbuf = gdk_pixbuf_new_from_file (location, NULL);
	g_free (location);
	if (pixbuf)
	{
		image = gtk_image_new_from_pixbuf (pixbuf);
		gtk_container_add (GTK_CONTAINER (button), image);
	}
	else
		gtk_container_add (GTK_CONTAINER (button),
				   gtk_label_new (label));
	gtk_box_pack_start (GTK_BOX (control), button, FALSE, FALSE, 0);

	return button;
}

/* check if the uri is playable and return the mime type if it is, NULL
 * if not */
char *
audio_view_is_playable (const char *uri)
{
        char *type = NULL;

        type = (char *) gnome_vfs_get_mime_type (uri);
        /* FIXME: assuming we get an allocated copy and have ownership */
        if (!type) return NULL;
        if (
            (strcmp (type, "application/x-ogg") != 0) &&
            (strcmp (type, "audio/x-flac") != 0) &&
            (strcmp (type, "audio/x-mp3") != 0) &&
            (strcmp (type, "audio/x-wav") != 0) &&
	    (strcmp (type, "audio/x-mod") != 0) &&
	    (strcmp (type, "audio/x-s3m") != 0) &&
	    (strcmp (type, "audio/x-xm") != 0) &&
	    (strcmp (type, "audio/x-it") != 0)
	   )
        {
                g_free (type);
                return NULL;
        }
        return type;
}

/* extract GList of string properties to a string of different lines
 * this might not be very optimized but occurences of having multiple
 * similar tags aren't that common to worry much about it */
static gchar *
audio_view_props_to_string (GList *list, const gchar *prepend)
{
	GList *p;
	gchar *add;
	gchar *temp;
	gchar *result = NULL;

	p = list;
	while (p)
	{
		GstPropsEntry *entry = (GstPropsEntry *) p->data;
		const gchar *val;
		gst_props_entry_get_string (entry, &val);

		add = g_strdup_printf ("%s%s\n", prepend, val);
		if (result == NULL)
		{
			/* first pass */
			result = add;
		}
		else
		{
			temp = g_strconcat (result, add, NULL);
		        g_free (result);
			g_free (add);
			result = temp;
		}
		p = g_list_next (p);
	}
	return result;
}
/* returns a newly allocated human-readable type for the given mime type */
static gchar *
audio_view_mime_to_type (const gchar *mime)
{
	gchar *result;
        if (strcmp (mime, "application/x-ogg") == 0)
		return g_strdup (_("Ogg/Vorbis"));
        if (strcmp (mime, "application/x-flac") == 0)
		return g_strdup (_("FLAC"));
        if (strcmp (mime, "audio/x-mp3") == 0)
		return g_strdup (_("MPEG"));
        if (strcmp (mime, "audio/x-wav") == 0)
		return g_strdup (_("WAVE"));
	/* FIXME: add more mimes prematurely so they are translated when
	 * we need them ! */
        if (strcmp (mime, "audio/x-mod") == 0)
		return g_strdup (_("Amiga mod"));
	if (strcmp (mime, "audio/x-s3m") == 0)
		return g_strdup (_("Amiga mod"));
	if (strcmp (mime, "audio/x-xm") == 0)
		return g_strdup (_("Amiga mod"));
	if (strcmp (mime, "audio/x-it") == 0)
		return g_strdup (_("Amiga mod"));
        if (strcmp (mime, "audio/x-aiff") == 0)
		return g_strdup (_("Apple AIFF"));
        if (strcmp (mime, "audio/x-midi") == 0)
		return g_strdup (_("MIDI"));
        if (strcmp (mime, "audio/x-ulaw") == 0)
		return g_strdup (_("ulaw audio"));
	return g_strdup (_("Unknown"));
}

/* extract the useful metadata to an allocated string */
static gchar *
audio_view_media_get_metadata (GstMediaInfoStream *info)
{
	gchar *metadata = NULL;
	GList *artist = NULL;
	GList *title = NULL;
	gchar *artists;
	gchar *titles;

	GList *p;
	GstMediaInfoTrack *track;

	NM_DEBUG("getting metadata\n");
	if (info->length_tracks < 1) return NULL;
	if (info->tracks == NULL) return NULL;

	track = info->tracks->data;
	if (track == NULL) return NULL;
	if (track->metadata == NULL) return NULL;
	if (track->metadata->properties == NULL) return NULL;

	p = track->metadata->properties->properties;

	/* construct GLists of GstPropsEntries to artist and title */
	while (p)
	{
		GstPropsEntry *entry = (GstPropsEntry *) p->data;
		const gchar *name;
		const gchar *val;
		GstPropsType type;

		name = gst_props_entry_get_name (entry);
		type = gst_props_entry_get_type (entry);
		if (g_ascii_strncasecmp (name, "artist", 6) == 0 &&
		    type == GST_PROPS_STRING_TYPE)
			artist = g_list_append (artist, entry);
		if (g_ascii_strncasecmp (name, "title", 6) == 0 &&
		    type == GST_PROPS_STRING_TYPE)
			title = g_list_append (title, entry);
		p = g_list_next (p);
	}

	/* now construct a string out of it */
	artists = audio_view_props_to_string (artist, _("Artist: "));
	titles = audio_view_props_to_string (title, _("Title: "));
	if (artists && titles)
	{
		metadata = g_strconcat (artists, titles, NULL);
		g_free (artists);
		g_free (titles);
	}
	else if (artists) metadata = artists;
	else metadata = titles;

	/* we chomp because it looks better in the list view */
	return g_strchomp (metadata);
}

/* set next track to play
 * return FALSE if this is the last track */
static gboolean
audio_view_play_next (AudioView *view)
{
	AudioPlay *play = view->audio_play;
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GstMediaInfoStream *info;

	g_assert (IS_AUDIO_PLAY (play));
	NM_DEBUG("play_next: start\n");

	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->tree_view));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->tree_view));
	g_assert (GTK_IS_TREE_MODEL (model));
	/* get selected into iter and try to get a next track in the model */
	if (gtk_tree_selection_get_selected (sel, &model, &iter))
	{
		GtkTreePath *old_path;
		path = gtk_tree_model_get_path (model, &iter);
		old_path = gtk_tree_path_copy (path);
		gtk_tree_path_next (path);
		if (!gtk_tree_model_get_iter (model, &iter, path))
		{
			NM_DEBUG("play_next: already last\n");
			/* gtk_tree_selection_unselect_path (sel, old_path); */
			gtk_tree_path_free (path);
			return FALSE;
		}
		gtk_tree_model_get (model, &iter,
				    AUDIO_INFO_COLUMN, &info,
				    -1);
		NM_DEBUG("play_next: track to play is %s\n", info->path);
	}
	audio_view_set_playing (view, info, path);
	gtk_tree_path_free (path);
	return TRUE;
}

/* timeout for scanner */
static gboolean
scan_timeout (AudioView *view)
{
	audio_view_play_next (view);
}


/* ui functions */
static void
audio_view_set_time (AudioView *view, gint seconds, gint length)
{
	gchar *text;
	if (length == 0)
		text = g_strdup_printf (_("<span size=\"larger\">Unknown</span>"));
	else
		text = g_strdup_printf ("<span size=\"larger\">"
				        "[%d:%02d/%d.%02d]</span>", 
			                seconds / 60, seconds % 60,
				        length / 60, length % 60);
	gtk_label_set_markup (GTK_LABEL (view->time), text);
	g_free (text);
}

/* load the given location in the audio view */
void
audio_view_load_location (AudioView *view, const char *location)
{
  g_assert (location != NULL);

  g_free (view->location);
  view->location = g_strdup (location);
  view->selection = NULL;

  audio_view_update (view);
}

/* set this track to play and update the treeview to the new path
 * if path is NULL, don't update it */
/* FIXME: if info is NULL, that means we don't know yet.  Maybe
 * force an update of that first before playing ? */
static void
audio_view_set_playing (AudioView *view, GstMediaInfoStream *info,
		        GtkTreePath *path)
{
	gchar *message;
	gchar *unescaped;
	AudioPlay *play;
	GError **error = NULL;
	GtkTreeModel *model;
	GtkTreeSelection *sel;
	GtkTreeIter iter;
	gchar *filename;

	if (info == NULL)
	{
		message = g_strdup_printf (_("ERROR: no information yet"));
		gtk_label_set_text (GTK_LABEL (view->status), message);
		g_free (message);
		return;
	}
	/* play it */
	play = view->audio_play;
	g_assert (IS_AUDIO_PLAY (play));
	if (audio_play_get_state (play) == GST_STATE_PLAYING)
	{
		NM_DEBUG("set_playing: was playing, set to READY\n");
		audio_play_set_state (play, GST_STATE_READY, NULL);
		NM_DEBUG("set_playing: changed state to READY\n");
	}
	NM_DEBUG("set_playing: setting %s to play\n", info->path);
	audio_play_set_location (play, info->path, NULL);
	audio_play_set_state (play, GST_STATE_PLAYING, error);
	if (view->scanning)
	{
		guint64 nanosecs = audio_play_get_length (play);

		if (view->scan_timeout >= 0)
		{
			g_source_remove (view->scan_timeout);
			view->scan_timeout = -1;
		}
		NM_DEBUG("we're scanning, %lld\n", nanosecs);
		if (nanosecs > SCAN_LENGTH * GST_SECOND)
		{
			nanosecs -= SCAN_LENGTH;
			nanosecs /= 2;
			NM_DEBUG("scanning mode, seek to somewhere in middle\n");
			NM_DEBUG("seeking to %lld\n", nanosecs);
			audio_play_seek_to_time (play, nanosecs);
			view->scan_timeout = g_timeout_add (SCAN_LENGTH * 1000,
					                    (GSourceFunc) scan_timeout,
							    view);
		}
	}
	if (error)
	{
		/* FIXME: handleme */
		message = g_strdup_printf (_("ERROR: %s"), (*error)->message);
		gtk_label_set_text (GTK_LABEL (view->status), message);
		g_free (message);
		g_free (error);
		return;
	}

	/* status update */
	message = g_strdup_printf (_("Playing %s"), info->path);
	gtk_label_set_text (GTK_LABEL (view->status), message);
	g_free (message);

	NM_DEBUG("updating selection\n");
	/* selection update */
	if (path)
	{

		gtk_tree_view_set_cursor (GTK_TREE_VIEW (view->tree_view),
			                  path, NULL, FALSE);
	}

	/* now playing update */
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->tree_view));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->tree_view));
	g_assert (GTK_IS_TREE_MODEL (model));

	if (gtk_tree_selection_get_selected (sel, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, FILE_COLUMN, &filename, -1);
		NM_DEBUG("filename is %s\n", filename);
		message = g_strdup_printf (_("Playing %s"), filename);
		gtk_label_set_text (GTK_LABEL (view->status), message);
		g_free (message);
		gtk_tree_model_get (model, &iter,
				    AUDIO_INFO_COLUMN, &info,
				    -1);
	}
	return;
}

/* ui callbacks */
void
audio_view_error_handler (const gchar *message, gpointer data)
{
	GtkWindow *parent = GTK_WINDOW (data);
	GtkWidget *dialog;

	if (parent == NULL || !GTK_IS_WINDOW (parent))
	{
		/* no parent window, so not sure if ui windows will be shown */
		g_warning (message);
		/* save it for possible later display */
		/* FIXME: remove this, we don't get at the view anyway */
		/*
		if (view->error_message) g_free (view->error_message);
		view->error_message = g_strdup (message);
		*/
	}
	dialog = gtk_message_dialog_new (parent, GTK_DIALOG_DESTROY_WITH_PARENT,
			                 GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
					 message);
	/* destroy it if the dialog is run */
	if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_NONE)
		gtk_widget_destroy (dialog);
}

/* FIXME */
static void
row_activated_callback (GtkTreeView *tree_view, GtkTreePath *path,
                        GtkTreeViewColumn *column,
                        AudioView *view)
{
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
	GstMediaInfoStream *info;
	gchar *pathname = gtk_tree_path_to_string (path);
	NM_DEBUG("activated_cb: path %s\n", pathname);
	g_free (pathname);

	if (! (gtk_tree_model_get_iter (model, &iter, path)))
			return;

	gtk_tree_model_get (model, &iter,
			    AUDIO_INFO_COLUMN, &info,
			    -1);
	NM_DEBUG("callback: get info %p (%s) playing (iterp %)\n",
		 info, info->path, &iter);
	audio_view_set_playing (view, info, NULL);
}

static void
prev_activate (GtkButton *prev_button, AudioView *view)
{
	AudioPlay *play;
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GstMediaInfoStream *info;

	play = view->audio_play;
	g_assert (IS_AUDIO_PLAY (play));

	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->tree_view));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->tree_view));
	g_assert (GTK_IS_TREE_MODEL (model));
	if (gtk_tree_selection_get_selected (sel, &model, &iter))
	{
		path = gtk_tree_model_get_path (model, &iter);
		if (!gtk_tree_path_prev (path))
			return;
		if (!gtk_tree_model_get_iter (model, &iter, path))
			return;
		gtk_tree_model_get (model, &iter,
				    AUDIO_INFO_COLUMN, &info,
				    -1);
	}
	audio_view_set_playing (view, info, path);
}

static void
play_activate (GtkButton *play_button, AudioView *view)
{
	gchar *message;
	gchar *selection = view->selection;
	GtkTreeSelection *sel;
	GtkTreeModel *model;
	GtkTreeIter iter;
	AudioPlay *play;
	gchar *path = NULL;
	gchar *file;
	GstMediaInfoStream *info;

	/* try to get the selected track */
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->tree_view));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (view->tree_view));
	g_assert (GTK_IS_TREE_MODEL (model));
	if (gtk_tree_selection_get_selected (sel, &model, &iter))
	{
		gtk_tree_model_get (model, &iter,
				    AUDIO_INFO_COLUMN, &info,
				    -1);
		NM_DEBUG("pressed play on track %s\n", info->path);
		audio_view_set_playing (view, info, NULL);
	}
	else
	{
		message = g_strdup_printf (_("Make a selection first !"));
		gtk_label_set_text (GTK_LABEL (view->status), message);
		g_free (message);
	}
}

static void
stop_activate (GtkButton *stop_button, AudioView *view)
{
	AudioPlay *play;

	play = view->audio_play;
	g_assert (IS_AUDIO_PLAY (play));
	if (audio_play_get_state (play) == GST_STATE_PLAYING)
	{
		NM_DEBUG("pressed stop, setting to ready\n");
		audio_play_set_state (play, GST_STATE_READY, NULL);
	}
	/* remove callback if there */
	if (view->scan_timeout >= 0)
	{
		g_source_remove (view->scan_timeout);
		view->scan_timeout = -1;
	}
	gtk_range_set_value (GTK_RANGE (view->seek_scale),
			     (double) 0.0);
	audio_view_set_time (view, 0, 0);
	gtk_label_set_text (GTK_LABEL (view->status),
			    _("Stopped."));
}

static void
next_activate (GtkButton *next_button, AudioView *view)
{
	audio_view_play_next (view);
}

static void
scan_activate (GtkButton *scan_button, AudioView *view)
{
	NM_DEBUG("toggle scan\n");
	view->scanning = !view->scanning;
	NM_DEBUG("scanning %s\n", view->scanning ? "true" : "false");
	/* FIXME: update toggle ? */
	/* FIXME: act when playing or smth ? */
	if (view->scanning)
	{
		AudioPlay *play = view->audio_play;
		g_assert (IS_AUDIO_PLAY (play));
		if (audio_play_get_state (play) == GST_STATE_PLAYING)
		{
			view->scan_timeout = g_timeout_add (SCAN_LENGTH * 1000,
					            (GSourceFunc) scan_timeout,
						    view);
		}
	}
	else
	{
		/* remove callback if there */
		if (view->scan_timeout >= 0)
			g_source_remove (view->scan_timeout);
		view->scan_timeout = -1;
	}
}
/* GStreamer callbacks */
static void
have_tick_callback (AudioPlay *play, gint64 time_nanos, AudioView *view)
{
	gchar *text;
	gint length_seconds;
	gint seconds = (gint) (time_nanos / GST_SECOND);

	length_seconds = (gint) (audio_play_get_length (play) / GST_SECOND);
	NM_DEBUG("tick: length %d\n", length_seconds);
	/* update time display */
	audio_view_set_time (view, seconds, length_seconds);

	/* update time slider */
	g_signal_handler_block (GTK_RANGE (view->seek_scale),
			        view->seek_changed_id);
	gtk_range_set_increments (GTK_RANGE (view->seek_scale),
			          (1.0 / length_seconds),
				  (10.0 / length_seconds));
	gtk_range_set_value (GTK_RANGE (view->seek_scale),
			     (double) seconds / length_seconds);
	g_signal_handler_unblock (GTK_RANGE (view->seek_scale),
			        view->seek_changed_id);
}

static void
have_eos_callback (AudioPlay *play, AudioView *view)
{
	NM_DEBUG("have_eos callback activated\n");
	/* audio_play_set_state (play, GST_STATE_READY, NULL); */
	if (! audio_view_play_next (view))
	{
		gtk_label_set_text (GTK_LABEL (view->status), _("Stopped."));
		gtk_widget_show (view->status);
	}
}

static void
seek_changed_callback (GtkWidget *widget, AudioView *view)
{
	AudioPlay *play = view->audio_play;
	gdouble value = gtk_range_get_value (GTK_RANGE (widget));
	gdouble last_seek_value = view->last_seek_value;
	g_assert (IS_AUDIO_PLAY (play));
	if (audio_play_get_state (play) != GST_STATE_PLAYING) return;

	audio_play_seek_to_pos (play, value);
	view->last_seek_value = value;
}

/* helper function to advance the media info getter to the next item
 * if there is one */
/* Returns: TRUE if there is a next track, FALSE otherwise */
static gboolean
audio_view_media_info_next (AudioView *view)
{
	/* advance to the next audio item if there is one */
	view->audio_item = view->audio_item->next;
	/* FIXME: remove this debug function */
	/* audio_view_tree_model_dump (view); */
	if (view->audio_item)
	{
		view->state = AUDIO_VIEW_INFO_STATE_START;
		return TRUE;
	}
	view->state = AUDIO_VIEW_INFO_STATE_DONE;
	return FALSE;
}

static gboolean
audio_view_get_media_info_idler (AudioView *view)
{
	GList *p;
	GtkTreeIter iter;
	GstMediaInfoStream *info = NULL;
	gchar *metadata;

	if (view->audio_list == NULL)
	{
		/* no audio items at all ! */
		return FALSE;
	}
	switch (view->state)
	{
		case AUDIO_VIEW_INFO_STATE_NULL:
		{
			NM_DEBUG("STATE is NULL\n");
			view->audio_item = view->audio_list;
			view->state = AUDIO_VIEW_INFO_STATE_START;
			return TRUE;
		}
		case AUDIO_VIEW_INFO_STATE_START:
		{
			gchar *escaped_name;
			gchar *path_uri;
			gchar *uri;

			view->iterp = (GtkTreeIter *) view->audio_item->data;
			gtk_tree_model_get (GTK_TREE_MODEL (view->list_store),
					    view->iterp, FILE_COLUMN, &uri, -1);
			NM_DEBUG("get: view->iterp: %p (%s)\n",
				 view->iterp, uri);

			escaped_name = (char *) gnome_vfs_escape_string (uri);
			path_uri = g_build_filename (view->location,
						     uri, NULL);
			g_free (escaped_name);
			NM_DEBUG("getting media-info for path uri %s\n",
			         path_uri);
			gst_media_info_read_with_idler (view->media_info,
							path_uri,
							GST_MEDIA_INFO_ALL);
			view->stream = NULL;
			view->state = AUDIO_VIEW_INFO_STATE_IDLER;
			return TRUE;
		}
		case AUDIO_VIEW_INFO_STATE_IDLER:
		{
			GstMediaInfoStream *info;
			gchar *time;
			gchar *bitrate;
			gchar *metadata;
			gchar *type;

			if ((view->stream) == NULL)
			{
				if (!gst_media_info_read_idler (view->media_info,
								&(view->stream)))
				{
						g_warning ("error on media info idler !");
						/* FIXME: better cleanup here */
						audio_view_media_info_next (view);
				}
				return TRUE;
			}
			/* got info, update it */
			info = view->stream;
			if (info)
			{
				gint seconds = info->length_time / GST_SECOND;
				gint sec = seconds % 60;
				gint min = seconds / 60;

				time = g_strdup_printf ("%d:%02d", min, sec);
			        metadata = audio_view_media_get_metadata (info);
				if (info->bitrate)
					bitrate = g_strdup_printf ("%d kbs",
						  info->bitrate / 1000);
				else
					bitrate = g_strdup_printf (_("Unknown"));
			}
			else
			{
				time = g_strdup (_("Unknown"));
				metadata = g_strdup (_("Unknown"));
			}
			NM_DEBUG("set: view->iterp: %p (%s)\n",
				 view->iterp, info->path);
			type = audio_view_mime_to_type (info->mime);
			gtk_list_store_set (view->list_store,
					    view->iterp,
					    LENGTH_COLUMN, time,
					    BITRATE_COLUMN, bitrate,
					    METADATA_COLUMN, metadata,
					    TYPE_COLUMN, type,
					    AUDIO_INFO_COLUMN, info,
					    -1);
			/* FIXME: do we have to free type ? */
			view->stream = NULL;
			g_free (time);
			audio_view_media_info_next (view);
			return TRUE;
		}
	case AUDIO_VIEW_INFO_STATE_DONE:
			view->iterp = NULL;
			view->audio_item = NULL;
			view->state = AUDIO_VIEW_INFO_STATE_NULL;
			return FALSE;
		default:
			g_warning ("Don't know what to do here\n");
			return FALSE;
	}
	/* release song list */
	/* FIXME: rather not free the audio info's here, we need them no ? */
	/* eel_g_list_free_deep_custom (song_list, (GFunc) audio_info_free, NULL); */
}


/* updates the view with info from the uri
 * shows file names, then starts handler to get rest of info
 * to get a more responsive ui
 */
static void
audio_view_update (AudioView *view)
{
	GnomeVFSResult result;
	GnomeVFSFileInfo *current_file_info;
	GList *list, *node;

	GList *p;
	GList *audio_list = NULL;
	GList *attributes;
	GstMediaInfoStream *info;
	char *path_uri, *escaped_name;
	char *path;
        gchar *message;
	GtkTreeIter iter;

	int file_index;
	int image_count;

	/* try reading the dir */
	result = gnome_vfs_directory_list_load (&list, view->location,
				GNOME_VFS_FILE_INFO_GET_MIME_TYPE
				| GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
	if (result != GNOME_VFS_OK) {
		path = (char *) gnome_vfs_get_local_path_from_uri (view->location);
		message = g_strdup_printf (_("Sorry, but there was an error reading %s."), path);
		eel_show_error_dialog (message, _("Can't read folder"),
				       GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (view->event_box))));
		g_free (path);
		g_free (message);

		return;
	}

	/* iterate through the directory, collecting playable audio files
	 * and FIXME: extracting media-info data if present */
        /* populate the list with filenames */
        gtk_list_store_clear (view->list_store);
	for (node = list; node != NULL; node = node->next) {
		current_file_info = node->data;

	        /* skip invisible files, for now */
	        if (current_file_info->name[0] == '.') continue;

                escaped_name = (char *) gnome_vfs_escape_string (current_file_info->name);
                path_uri = g_build_filename (view->location,
				             current_file_info->name, NULL);
                g_free (escaped_name);

		/* FIXME: check if the mime type is playable */
		/* we will get NULL if it's not playable */
		if (audio_view_is_playable (path_uri))
		{
			/* add to list */
			gtk_list_store_append (view->list_store, &iter);
			gtk_list_store_set (view->list_store, &iter,
				            FILE_COLUMN,
				            g_strdup (current_file_info->name),
				            -1);
			view->audio_list = g_list_append (view->audio_list,
				                  gtk_tree_iter_copy (&iter));
		}
	}
        gnome_vfs_file_info_list_free (list);

	/* release song list */
	/* FIXME: rather not free the audio info's here, we need them no ? */
	/* so where do we move it to ? */
	/*
	eel_g_list_free_deep_custom (song_list, (GFunc) audio_info_free, NULL);

	g_free (uri);
	*/
	gtk_widget_show (view->widget);
	/* create a media info object */
	view->media_info = gst_media_info_new (NULL);
	NM_DEBUG("adding idler");
	view->media_info_idler_id = g_idle_add ((GSourceFunc) audio_view_get_media_info_idler, view);
}

/* set up the tree view based on the AudioView */
static void
set_up_tree_view (AudioView *view)
{
	GtkCellRenderer *cell;
        GtkTreeViewColumn *column;
        GtkTreeView *tree_view;

        tree_view = GTK_TREE_VIEW (view->tree_view);
	gtk_tree_view_set_rules_hint (tree_view, TRUE);

        /* The file column */
        cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "yalign", 0.0, NULL);

        column = gtk_tree_view_column_new_with_attributes (_("File"),
                                                           cell,
                                                           "text",
                                                           FILE_COLUMN,
							   NULL);
        gtk_tree_view_column_set_sort_column_id (column, FILE_COLUMN);
        gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (tree_view, column);

        /* The type column */
        cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "yalign", 0.0, NULL);
        column = gtk_tree_view_column_new_with_attributes (_("Type"),
                                                           cell,
                                                           "text",
                                                           TYPE_COLUMN,
							   NULL);
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, TYPE_COLUMN);
	gtk_tree_view_append_column (tree_view, column);

        /* The length column */
        cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "yalign", 0.0, NULL);
        column = gtk_tree_view_column_new_with_attributes (_("Length"),
                                                           cell,
                                                           "text",
                                                           LENGTH_COLUMN,
							   NULL);
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, LENGTH_COLUMN);
	gtk_tree_view_append_column (tree_view, column);

        /* The bitrate column */
        cell = gtk_cell_renderer_text_new ();
	g_object_set (cell, "yalign", 0.0, NULL);
        column = gtk_tree_view_column_new_with_attributes (_("Bitrate"),
                                                           cell,
                                                           "text",
                                                           BITRATE_COLUMN,
							   NULL);
        gtk_tree_view_column_set_resizable (column, TRUE);
        gtk_tree_view_column_set_sort_column_id (column, BITRATE_COLUMN);
	gtk_tree_view_append_column (tree_view, column);

        /* The metadata column */
        column = gtk_tree_view_column_new_with_attributes (_("Metadata"),
                                                           cell,
                                                           "text",
                                                           METADATA_COLUMN,
							   NULL);
        gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_append_column (tree_view, column);
}
/* Returns: the error message if there was an error, or NULL if
 * there wasn't */
/* FIXME: this isn't really necessary to have anymore is it ?
 */
const gchar *
audio_view_get_error (AudioView *view)
{
	return view->error_message;
}

/* Returns: the main widget created by the view */
GtkWidget *
audio_view_get_widget (AudioView *view)
{
	return view->widget;
}

AudioView *
audio_view_new ()
{
	GError *error = NULL;
	AudioView *view;
	GstElement *audio_sink;
	GtkWidget *image;

	view = g_new0 (AudioView, 1);
	/* set to NULL what needs to be NULL */
	view->error_message = NULL;
	view->media_info_idler_id = 0;

	/* get a toplevel vbox */
	view->widget = gtk_vbox_new (FALSE, 5);

	/* create list model */
	/* FIXME: do we really need strings for int values ? */
	view->list_store = gtk_list_store_new (
		NUM_COLUMNS,
		G_TYPE_STRING,	/* FILE_COLUMN */
		G_TYPE_STRING,  /* TYPE_COLUMN */
		G_TYPE_STRING,  /* LENGTH_COLUMN */
		G_TYPE_STRING,  /* BITRATE_COLUMN */
		G_TYPE_STRING,  /* METADATA_COLUMN */
		G_TYPE_STRING,	/* path */
		G_TYPE_POINTER	/* AUDIO_INFO_COLUMN */
	);
	view->tree_view = gtk_tree_view_new_with_model (
		GTK_TREE_MODEL (view->list_store)
	);
        g_signal_connect (view->tree_view,
                          "row_activated",
                          G_CALLBACK (row_activated_callback),
                          view);

        g_object_unref (view->list_store);
        set_up_tree_view (view);

	/* get the scroll window */
	view->scroll_window = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (view->scroll_window),
                                        GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
        gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (view->scroll_window),
                                             GTK_SHADOW_ETCHED_IN);
        gtk_container_add (GTK_CONTAINER (view->scroll_window),
                           view->tree_view);
	gtk_box_pack_start (GTK_BOX (view->widget),
			    view->scroll_window, TRUE, TRUE, 0);

	/* get controls */
	view->control = gtk_hbox_new (FALSE, 0);

	/* prev */
	view->prev_button = control_box_add_labeled_button (
			view->control, "Previous", "prev.png", FALSE);
	g_signal_connect (G_OBJECT (view->prev_button),
			  "clicked", G_CALLBACK (prev_activate), view);
	/* stop */
	view->stop_button = control_box_add_labeled_button (
			view->control, "Stop", "stop.png", FALSE);
	g_signal_connect (G_OBJECT (view->stop_button),
			  "clicked", G_CALLBACK (stop_activate), view);

	/* play */
	view->play_button = control_box_add_labeled_button (
			view->control, "Play", "play.png", FALSE);
	g_signal_connect (G_OBJECT (view->play_button),
			  "clicked", G_CALLBACK (play_activate), view);

	/* next */
	view->next_button = control_box_add_labeled_button (
			view->control, "Next", "next.png", FALSE);
	g_signal_connect (G_OBJECT (view->next_button),
			  "clicked", G_CALLBACK (next_activate), view);

	/* scan */
	view->scan_button = control_box_add_labeled_button (
			view->control, "Scan", "scan.png", TRUE);
	g_signal_connect (G_OBJECT (view->scan_button),
			  "clicked", G_CALLBACK (scan_activate), view);

	/* seek bar */
	/*
	view->seek_adj = gtk_adjustment_new (0.0, 0.0, 1.0,
			                              0.01, 0.1, 1);
	view->seek_scale = gtk_hscale_new (view->seek_adj);
						      */
	view->seek_scale = gtk_hscale_new_with_range (0.0, 1.0, 0.001);
	gtk_range_set_update_policy (GTK_RANGE (view->seek_scale),
				     GTK_UPDATE_DELAYED);
	view->seek_changed_id = g_signal_connect (G_OBJECT (view->seek_scale),
			                          "value_changed",
			  G_CALLBACK (seek_changed_callback), view);

	gtk_scale_set_draw_value (GTK_SCALE (view->seek_scale), FALSE);
	gtk_box_pack_start (GTK_BOX (view->control),
			   view->seek_scale, TRUE, TRUE, 0);

	/* time */
	view->time = gtk_label_new (NULL);
	audio_view_set_time (view, 0, 0);
	gtk_box_pack_start (GTK_BOX (view->control),
			   view->time, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (view->widget),
			    view->control, FALSE, FALSE, 0);

	/* get a status area */
	view->status = gtk_label_new (_("Not playing"));
	gtk_misc_set_alignment (GTK_MISC (view->status), 0.0, 0.5);
	gtk_box_pack_start (GTK_BOX (view->widget),
			    view->status, FALSE, FALSE, 0);

	/* initialize GStreamer */
	gst_init (NULL, NULL);

	/* create a play object */
	view->audio_play = audio_play_new (&error);
	if (error)
	{
		/* no window yet, so no parent */
		audio_view_error_handler (error->message, NULL);
		g_error_free (error);
	}
	if (!IS_AUDIO_PLAY (view->audio_play))
	{
		g_warning ("Could not create audio_play object\n");
		return NULL;
	}

	/* install the error handler */
	audio_play_set_error_handler (view->audio_play,
			              (AudioPlayErrorHandler) audio_view_error_handler,
				      NULL);
	audio_sink = gst_gconf_get_default_audio_sink ();
	/* FIXME: do fallback instead of assert */
	g_assert (GST_IS_ELEMENT (audio_sink));
	audio_play_set_audio_sink (view->audio_play, audio_sink);
	g_signal_connect (G_OBJECT (view->audio_play), "eos",
			  G_CALLBACK (have_eos_callback), view);
	g_signal_connect (G_OBJECT (view->audio_play), "tick",
			  G_CALLBACK (have_tick_callback), view);

	view->media_info = NULL;
	view->scanning = FALSE;
	view->scan_timeout = -1; /* FIXME: what is the range for source ids */
	/* state stuff */
	view->state = AUDIO_VIEW_INFO_STATE_NULL;
	view->audio_item = NULL;
	return view;
}

void
audio_view_dispose (AudioView *view)
{
	/* no need to destroy widgets, gets done automatically on toplevel */
	g_print ("audio_view_dispose started\n");
	NM_DEBUG("audio_play: %p\n", view->audio_play);
	/* if it never got created we won't have it will we ? */

	g_print ("unreffing audio object\n");
	if (view->audio_play) {
		g_object_unref (view->audio_play);
		view->audio_play = NULL;
	}

	if (view->media_info_idler_id) {
		g_source_remove (view->media_info_idler_id);
		view->media_info_idler_id = 0;
	}

	if (view->scan_timeout) {
		g_source_remove (view->scan_timeout);
		view->scan_timeout = 0;
	}

	if (view->media_info) {
		g_object_unref (view->media_info);
		view->media_info = NULL;
	}

	if (view->stream) {
		g_object_unref (view->stream);
		view->stream = NULL;
	}
}

static void
free_iter_list (GList *list)
{
	GList *l;
	
	for (l = list; l != NULL; l = l->next) {
		gtk_tree_iter_free (l->data);
	}

	g_list_free (list);
}

void
audio_view_finalize (AudioView *view)
{
	free_iter_list (view->audio_list);

	g_free (view->error_message);
	g_free (view->location);
	g_free (view->selection);
}
