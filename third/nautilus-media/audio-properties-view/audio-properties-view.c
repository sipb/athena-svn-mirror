/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Copyright (C) 2002 James Willcox
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
 * Authors:
 *	James Willcox <jwillcox@gnome.org>
 *	Thomas Vander Stichele <thomas at apestaart dot org>
 */

#include <config.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <glade/glade-xml.h>
#include <libgnome/gnome-macros.h>
#include <eel/eel-gnome-extensions.h>
#include <string.h>

#include "i18n-support.h"
#include "audio-properties-view.h"
#include "../media-info/media-info.h"

struct AudioPropertiesView {
	char *location;
	GstMediaInfo *media_info;
	GtkWidget *vbox;
	GtkWidget *tracklabel;
	GtkWidget *trackselect;

	GtkWidget *title;
	GtkWidget *artist;
	GtkWidget *album;

	GtkWidget *length;
	GtkWidget *bitrate;
	GtkWidget *format;
};

/* private functions */
/* FIXME: this is useful enough to warrant moving to media info */
/* returns an allocated copy of properties of this type, joined by
 * join_string */
typedef enum
{
	GMI_PROPERTY_ARTIST,
	GMI_PROPERTY_TITLE,
	GMI_PROPERTY_ALBUM
} GstMediaInfoProperty;

/* internal helpers */
/* get the integer property given by name from the track's format */
static gint
apv_get_format_int (GstMediaInfoTrack *track, const char *property_name)
{
#if 0
FIXME for tag
	GstProps *props;
	GList *p;
	if (track->format == NULL) return -1;

	props = track->format->properties;
	if (props == NULL) return -1;
	p = props->properties;
	while (p)
	{
		GstPropsEntry *entry = (GstPropsEntry *) p->data;
		const gchar *name;
		gint int_val;

		name = gst_props_entry_get_name (entry);
		if ((strcmp (name, property_name) == 0))
		{
			gst_props_entry_get_int (entry, &int_val);
			return int_val;
		}
		p = g_list_next (p);
      }
#endif
return 0;
}

/* public functions */
void
audio_properties_view_dispose (AudioPropertiesView *view)
{
	g_free (view->location);
	/* FIXME: and media info thingy ? */
	g_free (view);
}

void
audio_properties_view_load_location (AudioPropertiesView *view,
				     const char *location)
{
	GstMediaInfoStream *stream = NULL;
	GstMediaInfoTrack *track = NULL;
	gchar *string = NULL;
	gchar *c_string = NULL;
	gchar *min_string = NULL;
	gchar *sec_string = NULL;
	gint channels, rate, width;
	gint min, sec, msec;
	GError *error = NULL;
	GstStructure *struc;

	g_assert (location != NULL);

	if (view->location)
		g_free (view->location);

	view->location = g_strdup (location);

	stream = gst_media_info_read (view->media_info, location,
				      GST_MEDIA_INFO_ALL, &error);

	if (stream == NULL) return;

	/* FIXME: get real values here */
	/*
	string = g_strdup_printf ("%d", stream->length_tracks);
	gtk_entry_set_text (GTK_ENTRY (view->tracks), string);
	g_free (string);
	*/
	if (stream == NULL) return;
	if (stream->length_tracks == 0) return;
	if (stream->tracks == 0) return;
	/* FIXME: this is the first, what about others ? */
	track = stream->tracks->data;

	/* metadata */
	/* FIXME: we should actually parse metadata into a struct with
	 * sensible arguments, so that we can get comments as everything
	 * that is not otherwise displayed */
	if (!gst_tag_list_get_string (track->metadata, "artist", &string))
		string = g_strdup (_("None"));
	gtk_label_set_text (GTK_LABEL (view->artist), string);
	g_free (string);
	if (!gst_tag_list_get_string (track->metadata, "title", &string))
		string = g_strdup (_("None"));
	gtk_label_set_text (GTK_LABEL (view->title), string);
	g_free (string);
	if (!gst_tag_list_get_string (track->metadata, "album", &string))
		string = g_strdup (_("None"));
	if (string == NULL) string = g_strdup (_("None"));
	gtk_label_set_text (GTK_LABEL (view->album), string);
	g_free (string);

	/* streaminfo */
	struc = gst_caps_get_structure (track->format, 0);
	if (!gst_structure_get_int (struc, "channels", &channels)) channels = 0;
	if (!gst_structure_get_int (struc, "rate", &rate)) rate = -1;
	if (!gst_structure_get_int (struc, "width", &width)) width = -1;
	if (channels == 1) c_string = g_strdup (_("mono"));
	else if (channels == 2) c_string = g_strdup (_("stereo"));
	else if (channels == 0) c_string = g_strdup (_("unknown"));
	else c_string = g_strdup_printf (ngettext ("%d channel", "%d channels", channels),
					 channels);
	string = g_strdup_printf ("%d Hz/%s/%d bit", rate, c_string, width);
	g_free (c_string);
	gtk_label_set_text (GTK_LABEL (view->format), string);
	g_free (string);
	/* FIXME: are we allowed to % on gint64's ??? */
	msec = ((double) (stream->length_time % GST_SECOND)) / 1E6;
	sec = (double) stream->length_time / GST_SECOND;
	min = sec / 60;
	sec %= 60;

	min_string = g_strdup_printf(ngettext("%d minute", "%d minutes", min), min);
	/* Translators: plural form depends on the decimal part */
	sec_string = g_strdup_printf(ngettext("%02d.%03d seconds", "%02d.%03d seconds", msec), sec, msec);
	/* Translators: this is composed of '%d minute' and '%02d.%03d seconds'; change order if needed */
	c_string = g_strdup_printf (_("%1$s %2$s"), min_string, sec_string);

	g_free(min_string);
	g_free(sec_string);

	gtk_label_set_text (GTK_LABEL (view->length), c_string);
	/* FIXME: start heated argument about k = 10^3 or 2^10 */
	gtk_label_set_text (GTK_LABEL (view->bitrate),
			    g_strdup_printf ("%.3f kbps",
				             (double) stream->bitrate / 1024));

	g_free (c_string);
}

GtkWidget *
audio_properties_view_get_widget (AudioPropertiesView *view)
{
	return view->vbox;
}

AudioPropertiesView *
audio_properties_view_new ()
{
	AudioPropertiesView *view;
	GladeXML *xml;
	GError *error = NULL;

	view = g_new0 (AudioPropertiesView, 1);

	xml = glade_xml_new (DATADIR_UNINST
			     "/audio-properties-view/audio-properties-view.glade",
			     "content", NULL);
	if (!xml)
		xml = glade_xml_new (DATADIR
			     "/nautilus/glade/audio-properties-view.glade",
			     "content", NULL);

	g_return_val_if_fail (xml != NULL, NULL);

	/* FIXME: check results */
	view->vbox = glade_xml_get_widget (xml, "content");
	g_assert (GTK_IS_WIDGET (view->vbox));

	view->tracklabel = glade_xml_get_widget (xml, "tracklabel");
	g_assert (GTK_IS_WIDGET (view->tracklabel));
	view->trackselect = glade_xml_get_widget (xml, "trackselect");
	g_assert (GTK_IS_WIDGET (view->trackselect));

	view->artist = glade_xml_get_widget (xml, "artist");
	g_assert (GTK_IS_WIDGET (view->artist));
	/* gtk_widget_set_sensitive (view->artist, FALSE); */
	view->title = glade_xml_get_widget (xml, "title");
	g_assert (GTK_IS_WIDGET (view->title));
	/* gtk_widget_set_sensitive (view->title, FALSE); */
	view->album = glade_xml_get_widget (xml, "album");
	g_assert (GTK_IS_WIDGET (view->album));
	/* gtk_widget_set_sensitive (view->album, FALSE); */
	/*
	view->tracks = glade_xml_get_widget (xml, "tracks");
	g_assert (GTK_IS_WIDGET (view->tracks));
	*/

	view->length = glade_xml_get_widget (xml, "length");
	g_assert (GTK_IS_WIDGET (view->length));
	view->bitrate = glade_xml_get_widget (xml, "bitrate");
	g_assert (GTK_IS_WIDGET (view->bitrate));
	view->format = glade_xml_get_widget (xml, "format");
	g_assert (GTK_IS_WIDGET (view->format));
	view->location = NULL;

	gtk_label_set_text (GTK_LABEL (view->title), _("Unknown"));
	gtk_label_set_text (GTK_LABEL (view->artist), _("Unknown"));
	gtk_label_set_text (GTK_LABEL (view->album), _("Unknown"));

	gtk_label_set_text (GTK_LABEL (view->length), _("Unknown"));
	gtk_label_set_text (GTK_LABEL (view->bitrate), _("Unknown"));
	gtk_label_set_text (GTK_LABEL (view->format), _("Unknown"));

	//gtk_widget_show (view->vbox);

	/* initialize GStreamer and media info reader */
	gst_init (NULL, NULL);
        gst_media_info_init ();
	view->media_info = gst_media_info_new (&error);
	//FIXME: handle possible error
	if (!gst_media_info_set_source (view->media_info, "gnomevfssrc", &error))
	{
		g_print ("Could not set gnomevfssrc as a source\n");
		g_print ("Reason: %s\n", error->message);
		g_error_free (error);
		return NULL;
	}

	return view;
}
