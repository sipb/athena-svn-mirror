/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Iain Holmes <iain@ximian.com>
 *
 *  Copyright 2002 Iain Holmes
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <gnome.h>
#include <gdk/gdkx.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libbonoboui.h>

#include "cddb-disclosure.h"
#include "cddb-slave-client.h"
#include "GNOME_Media_CDDBSlave2.h"

#define CDDBSLAVE_TRACK_EDITOR_IID "OAFIID:GNOME_Media_CDDBSlave2_TrackEditorFactory"

static CDDBSlaveClient *client = NULL;
static int running_objects = 0;

/* accessibility support */
static void set_relation (GtkWidget *widget, GtkWidget *label);

typedef struct _CDDBInfo {
	char *discid;

	char *title;
	char *artist;
	char *comment;
	char *genre;
	int year;

	int ntrks;
	CDDBSlaveClientTrackInfo **track_info;
} CDDBInfo;

typedef struct _TrackEditorDialog {
	gboolean dirty;

	GtkWidget *parent;
	GtkWidget *artist;
	GtkWidget *discid;
	GtkWidget *disctitle;
	GtkWidget *disccomments;
	GtkWidget *year;
	GtkWidget *genre;
	GtkWidget *tracks;
	GtkWidget *extra_info;

	GtkTextBuffer *buffer;
	GtkTreeModel *model;

	int current_track; /* The array offset that is currently
			      selected in the tree */
	CDDBInfo *info;
} TrackEditorDialog;

static char *genres[] = {
	N_("Blues"),
	N_("Classical Rock"),
	N_("Country"),
	N_("Dance"),
	N_("Disco"),
	N_("Funk"),
	N_("Grunge"),
	N_("Hip-Hop"),
	N_("Jazz"),
	N_("Metal"),
	N_("New Age"),
	N_("Oldies"),
	N_("Other"),
	N_("Pop"),
	N_("R&B"),
	N_("Rap"),
	N_("Reggae"),
	N_("Rock"),
	N_("Techno"),
	N_("Industrial"),
	N_("Alternative"),
	N_("Ska"),
	N_("Death Metal"),
	N_("Pranks"),
	N_("Soundtrack"),
	N_("Euro-Techno"),
	N_("Ambient"),
	N_("Trip-Hop"),
	N_("Vocal"),
	N_("Jazz+Funk"),
	N_("Fusion"),
	N_("Trance"),
	N_("Classical"),
	N_("Instrumental"),
	N_("Acid"),
	N_("House"),
	N_("Game"),
	N_("Sound Clip"),
	N_("Gospel"),
	N_("Noise"),
	N_("Alt"),
	N_("Bass"),
	N_("Soul"),
	N_("Punk"),
	N_("Space"),
	N_("Meditative"),
	N_("Instrumental Pop"),
	N_("Instrumental Rock"),
	N_("Ethnic"),
	N_("Gothic"),
	N_("Darkwave"),
	N_("Techno-Industrial"),
	N_("Electronic"),
	N_("Pop-Folk"),
	N_("Eurodance"),
	N_("Dream"),
	N_("Southern Rock"),
	N_("Comedy"),
	N_("Cult"),
	N_("Gangsta Rap"),
	N_("Top 40"),
	N_("Christian Rap"),
	N_("Pop/Funk"),
	N_("Jungle"),
	N_("Native American"),
	N_("Cabaret"),
	N_("New Wave"),
	N_("Psychedelic"),
	N_("Rave"),
	N_("Showtunes"),
	N_("Trailer"),
	N_("Lo-Fi"),
	N_("Tribal"),
	N_("Acid Punk"),
	N_("Acid Jazz"),
	N_("Polka"),
	N_("Retro"),
	N_("Musical"),
	N_("Rock & Roll"),
	N_("Hard Rock"),
	N_("Folk"),
	N_("Folk/Rock"),
	N_("National Folk"),
	N_("Swing"),
	N_("Fast-Fusion"),
	N_("Bebop"),
	N_("Latin"),
	N_("Revival"),
	N_("Celtic"),
	N_("Bluegrass"),
	N_("Avantgarde"),
	N_("Gothic Rock"),
	N_("Progressive Rock"),
	N_("Psychedelic Rock"),
	N_("Symphonic Rock"),
	N_("Slow Rock"),
	N_("Big Band"),
	N_("Chorus"),
	N_("Easy Listening"),
	N_("Acoustic"),
	N_("Humour"),
	N_("Speech"),
	N_("Chanson"),
	N_("Opera"),
	N_("Chamber Music"),
	N_("Sonata"),
	N_("Symphony"),
	N_("Booty Bass"),
	N_("Primus"),
	N_("Porn Groove"),
	N_("Satire"),
	N_("Slow Jam"),
	N_("Club"),
	N_("Tango"),
	N_("Samba"),
	N_("Folklore"),
	N_("Ballad"),
	N_("Power Ballad"),
	N_("Rhythmic Soul"),
	N_("Freestyle"),
	N_("Duet"),
	N_("Punk Rock"),
	N_("Drum Solo"),
	N_("A Cappella"),
	N_("Euro-House"),
	N_("Dance Hall"),
	N_("Goa"),
	N_("Drum & Bass"),
	N_("Club-House"),
	N_("Hardcore"),
	N_("Terror"),
	N_("Indie"),
	N_("BritPop"),
	N_("Negerpunk"),
	N_("Polsk Punk"),
	N_("Beat"),
	N_("Christian Gangsta Rap"),
	N_("Heavy Metal"),
	N_("Black Metal"),
	N_("Crossover"),
	N_("Contemporary Christian"),
	N_("Christian Rock"),
	N_("Merengue"),
	N_("Salsa"),
	N_("Thrash Metal"),
	N_("Anime"),
	N_("JPop"),
	N_("Synthpop"),
	N_("Nu-Metal"),
	N_("Art Rock"),
	NULL
};

#define MODEL_TRACK_NO 0
#define MODEL_NAME 1
#define MODEL_LENGTH 2
#define MODEL_EDITABLE 3

static GtkTreeModel *
make_tree_model (void)
{
	GtkListStore *store;

	store = gtk_list_store_new (4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

	return GTK_TREE_MODEL (store);
}

static char *
secs_to_string (int seconds)
{
	int min, sec;

	min = seconds / 60;
	sec = seconds % 60;

	return g_strdup_printf ("%d:%02d", min, sec);
}

static void
build_track_list (TrackEditorDialog *td)
{
	GtkTreeIter iter;
	GtkListStore *store = GTK_LIST_STORE (td->model);
	int i;

	g_return_if_fail (td->info != NULL);

	for (i = 0; i < td->info->ntrks; i++) {
		char *length;

		length = secs_to_string (td->info->track_info[i]->length);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter,
				    MODEL_TRACK_NO, i + 1,
				    MODEL_NAME, td->info->track_info[i]->name,
				    MODEL_LENGTH, length,
				    MODEL_EDITABLE, TRUE, -1);
		g_free (length);
	}
}

static void
clear_track_list (TrackEditorDialog *td)
{
	gtk_list_store_clear (GTK_LIST_STORE (td->model));
}

static void
free_track_info (TrackEditorDialog *td)
{
	CDDBInfo *info;

	info = td->info;

	g_free (info->discid);
	g_free (info->title);
	g_free (info->artist);
	g_free (info->comment);
	g_free (info->genre);

	cddb_slave_client_free_track_info (info->track_info);

	g_free (info);
	td->info = NULL;
}

static GList *
make_genre_list (void)
{
	GList *genre_list = NULL;
	int i;

	for (i = 0; genres[i]; i++) {
		genre_list = g_list_prepend (genre_list, _(genres[i]));
	}

	genre_list = g_list_sort (genre_list, (GCompareFunc) strcmp);

	return genre_list;
}

static void
extra_info_changed (GtkTextBuffer *tb,
		    TrackEditorDialog *td)
{
	GtkTextIter start, end;
	char *text;

	if (td->current_track == -1) {
		return; /* Hmmm, curious */
	}

	if (td->info->track_info[td->current_track]->comment != NULL) {
		g_free (td->info->track_info[td->current_track]->comment);
	}

	gtk_text_buffer_get_bounds (tb, &start, &end);
	text = gtk_text_buffer_get_text (tb, &start, &end, FALSE);
	td->info->track_info[td->current_track]->comment = text;
}

static void
track_selection_changed (GtkTreeSelection *selection,
			 TrackEditorDialog *td)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	int track;

	if (gtk_tree_selection_get_selected (selection, &model, &iter) == TRUE) {
		char *comment;

		gtk_tree_model_get (model, &iter, 0, &track, -1);
		comment = td->info->track_info[track - 1]->comment;

		td->current_track = track - 1;
		g_signal_handlers_block_matched (G_OBJECT (td->buffer),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL,
						 G_CALLBACK (extra_info_changed),
						 td);
		if (comment != NULL) {
			gtk_text_buffer_set_text (td->buffer, comment, -1);
		} else {
			gtk_text_buffer_set_text (td->buffer, "", -1);
		}
		g_signal_handlers_unblock_matched (G_OBJECT (td->buffer),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL,
						   G_CALLBACK (extra_info_changed),
						   td);
		gtk_widget_set_sensitive (td->extra_info, TRUE);
	} else {
		/* No track selected, you can enter a comment */
		gtk_widget_set_sensitive (td->extra_info, FALSE);
	}
}

static void
artist_changed (GtkEntry *entry,
		TrackEditorDialog *td)
{
	const char *artist;

	if (td->info == NULL) {
		return;
	}

	artist = gtk_entry_get_text (entry);
	if (td->info->artist != NULL) {
		g_free (td->info->artist);
	}
	td->info->artist = g_strdup (artist);
	td->dirty = TRUE;
}

static void
disctitle_changed (GtkEntry *entry,
		   TrackEditorDialog *td)
{
	const char *title;

	if (td->info == NULL) {
		return;
	}

	title = gtk_entry_get_text (entry);
	if (td->info->title != NULL) {
		g_free (td->info->title);
	}

	td->info->title = g_strdup (title);
	td->dirty = TRUE;
}

static void
year_changed (GtkEntry *entry,
	      TrackEditorDialog *td)
{
	const char *year;

	if (td->info == NULL) {
		return;
	}

	year = gtk_entry_get_text (entry);

	if (year != NULL) {
		td->info->year = atoi (year);
	} else {
		td->info->year = -1;
	}

	td->dirty = TRUE;
}

static void
genre_changed (GtkEntry *entry,
	       TrackEditorDialog *td)
{
	const char *genre;

	if (td->info == NULL) {
		return;
	}

	genre = gtk_entry_get_text (entry);

	if (td->info->genre != NULL) {
		g_free (td->info->genre);
	}

	td->info->genre = g_strdup (genre);
	td->dirty = TRUE;
}

static void
comment_changed (GtkEntry *entry,
		 TrackEditorDialog *td)
{
	const char *comment;

	if (td->info == NULL) {
		return;
	}

	comment = gtk_entry_get_text (entry);

	if (td->info->comment != NULL) {
		g_free (td->info->comment);
	}

	td->info->comment = g_strdup (comment);
	td->dirty = TRUE;
}

static void
track_name_edited (GtkCellRendererText *cell,
		   char *path_string,
		   char *new_text,
		   TrackEditorDialog *td)
{
	GtkTreeModel *model = td->model;
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	int track_no;

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, 0, &track_no, -1);

	g_print ("track no %d\n", track_no);
	if (td->info->track_info[track_no - 1]->name != NULL) {
		g_free (td->info->track_info[track_no - 1]->name);
	}
	td->info->track_info[track_no - 1]->name = g_strdup (new_text);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 1, new_text, -1);
	gtk_tree_path_free (path);
}

static void
load_new_track_data (TrackEditorDialog *td,
		     const char *discid)
{
	CDDBInfo *info;
	char *title;

	if (client == NULL) {
		client = cddb_slave_client_new ();
	}

	info = g_new (CDDBInfo, 1);
	td->info = info;

	/* Does the CORBA stuff need to be strduped? */
	info->discid = g_strdup (discid);
        /* The CDDB slave is responsible for making sure that entries exist
         * for new discids.
         * This is done by is_valid, which will create an entry if necessary.
	 * The CDDB slave marks entries as valid when they are valid,
         * either through successful lookup or through a save from the editor.
         * If the entry is not valid, we fill in dummy values here.
         * Invalid entries can still return useful info like ntrks and
         * track_info, which we need for filling in dummy values. */

	if (cddb_slave_client_is_valid (client, discid)) {
		info->artist = cddb_slave_client_get_artist (client, discid);
		info->title = cddb_slave_client_get_disc_title (client, discid);
		info->comment = cddb_slave_client_get_comment (client, discid);
		info->genre = cddb_slave_client_get_genre (client, discid);
		info->year = cddb_slave_client_get_year (client, discid);
	} else {
		info->artist = g_strdup (_("Unknown Artist"));
		info->title = g_strdup (_("Unknown Album"));
		info->comment = NULL;
		info->genre = NULL;
		info->year = 0;
	}
	info->ntrks = cddb_slave_client_get_ntrks (client, discid);
	info->track_info = cddb_slave_client_get_tracks (client, discid);

	title = g_strdup_printf (_("Editing Disc ID: %s"), info->discid);
	gtk_label_set_text (GTK_LABEL (td->discid), title);
	g_free (title);

	if (info->title != NULL) {
		g_signal_handlers_block_matched (G_OBJECT (td->disctitle),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL, G_CALLBACK (disctitle_changed), td);
		gtk_entry_set_text (GTK_ENTRY (td->disctitle), info->title);
		g_signal_handlers_unblock_matched (G_OBJECT (td->disctitle),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL, G_CALLBACK (disctitle_changed), td);
	}

	if (info->artist != NULL) {
		g_signal_handlers_block_matched (G_OBJECT (td->artist),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL, G_CALLBACK (artist_changed), td);
		gtk_entry_set_text (GTK_ENTRY (td->artist), info->artist);
		g_signal_handlers_unblock_matched (G_OBJECT (td->artist),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL, G_CALLBACK (artist_changed), td);
	}

	if (info->year != -1) {
		char *year;

		year = g_strdup_printf ("%d", info->year);
		g_signal_handlers_block_matched (G_OBJECT (td->year),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL, G_CALLBACK (year_changed), td);
		gtk_entry_set_text (GTK_ENTRY (td->year), year);
		g_signal_handlers_unblock_matched (G_OBJECT (td->year),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL, G_CALLBACK (year_changed), td);
		g_free (year);
	}

	if (info->comment != NULL) {
		g_signal_handlers_block_matched (G_OBJECT (td->disccomments),
						 G_SIGNAL_MATCH_FUNC,
						 0, 0, NULL, G_CALLBACK (comment_changed), td);
		gtk_entry_set_text (GTK_ENTRY (td->disccomments), info->comment);
		g_signal_handlers_unblock_matched (G_OBJECT (td->disccomments),
						   G_SIGNAL_MATCH_FUNC,
						   0, 0, NULL, G_CALLBACK (comment_changed), td);
	}

	g_signal_handlers_block_matched (G_OBJECT (GTK_COMBO (td->genre)->entry),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL, G_CALLBACK (genre_changed), td);
	if (info->genre != NULL) {
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (td->genre)->entry), info->genre);
	} else {
		gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (td->genre)->entry), "");
	}
	g_signal_handlers_unblock_matched (G_OBJECT (GTK_COMBO (td->genre)->entry),
					   G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL, G_CALLBACK (genre_changed), td);

	/* Rebuild track list */
	clear_track_list (td);
	build_track_list (td);

	td->dirty = FALSE;
}

static TrackEditorDialog *
make_track_editor_control (void)
{
	GtkWidget *hbox, *vbox, *inner_vbox, *ad_vbox, *ad_vbox2;
	GtkWidget *label;
	GtkWidget *advanced;
	GtkWidget *sw;
	TrackEditorDialog *td;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *col;
	GtkTreeSelection *selection;

	td = g_new0 (TrackEditorDialog, 1);

	td->info = NULL;
	td->current_track = -1;
	td->dirty = FALSE;

	td->parent = gtk_vbox_new (FALSE, 12);
	gtk_container_set_border_width (GTK_CONTAINER (td->parent), 12);

	/* Info label */
	td->discid = gtk_label_new (_("Editing Disc ID: "));
	gtk_misc_set_alignment (GTK_MISC (td->discid), 0.0, 0.5);

	inner_vbox = gtk_vbox_new (FALSE, 6);
	gtk_widget_show (inner_vbox);
	gtk_box_pack_start (GTK_BOX (td->parent), inner_vbox, FALSE, FALSE, 0);

	/* Artist */
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic (_("_Artist:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	td->artist = gtk_entry_new ();
	g_signal_connect (G_OBJECT (td->artist), "changed",
			  G_CALLBACK (artist_changed), td);
	gtk_box_pack_start (GTK_BOX (hbox), td->artist, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), td->artist);
	set_relation (td->artist, label);

	/* Disc title */
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new_with_mnemonic (_("Disc _Title:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	td->disctitle = gtk_entry_new ();
	g_signal_connect (G_OBJECT (td->disctitle), "changed",
			  G_CALLBACK (disctitle_changed), td);
	gtk_box_pack_start (GTK_BOX (hbox), td->disctitle, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), td->disctitle);
	set_relation (td->disctitle, label);

	advanced = cddb_disclosure_new (_("Show advanced disc options"),
					_("Hide advanced disc options"));
	gtk_box_pack_start (GTK_BOX (inner_vbox), advanced, FALSE, FALSE, 0);

	/* Advanced disc options */
	ad_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (inner_vbox), ad_vbox, FALSE, FALSE, 0);
	cddb_disclosure_set_container (CDDB_DISCLOSURE (advanced), ad_vbox);

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (ad_vbox), hbox, FALSE, FALSE, 0);

	/* Top box: Disc comments. Maybe should be a GtkText? */
	label = gtk_label_new_with_mnemonic (_("_Disc comments:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	td->disccomments = gtk_entry_new ();
	g_signal_connect (G_OBJECT (td->disccomments), "changed",
			  G_CALLBACK (comment_changed), td);
	gtk_box_pack_start (GTK_BOX (hbox), td->disccomments, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), td->disccomments);
	set_relation (td->disccomments, label);

	/* Bottom box */
	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (ad_vbox), hbox, FALSE, FALSE, 0);

	/* Genre */
	label = gtk_label_new_with_mnemonic (_("_Genre:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	td->genre = gtk_combo_new ();
	g_signal_connect (G_OBJECT (GTK_COMBO (td->genre)->entry), "changed",
			  G_CALLBACK (genre_changed), td);
	gtk_combo_set_popdown_strings (GTK_COMBO (td->genre),
				       make_genre_list ());
	gtk_combo_set_value_in_list (GTK_COMBO (td->genre), FALSE, TRUE);

	gtk_box_pack_start (GTK_BOX (hbox), td->genre, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label),
				       GTK_COMBO (td->genre)->entry);
	set_relation (td->genre, label);

	/* Year */
	label = gtk_label_new_with_mnemonic (_("_Year:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	td->year = gtk_entry_new ();
	g_signal_connect (G_OBJECT (td->year), "changed",
			  G_CALLBACK (year_changed), td);
	gtk_box_pack_start (GTK_BOX (hbox), td->year, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), td->year);
	set_relation (td->year, label);

	inner_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (td->parent), inner_vbox, TRUE, TRUE, 0);

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (inner_vbox), hbox, TRUE, TRUE, 0);

	/* Tracks */
	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (hbox), sw, TRUE, TRUE, 2);

	td->model = make_tree_model ();
	td->tracks = gtk_tree_view_new_with_model (td->model);
	g_object_unref (td->model);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (td->tracks));
	g_signal_connect (G_OBJECT (selection), "changed",
			  G_CALLBACK (track_selection_changed), td);

	cell = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (" ", cell,
							"text", MODEL_TRACK_NO, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (td->tracks), col);

	cell = gtk_cell_renderer_text_new ();
	g_signal_connect (G_OBJECT (cell), "edited",
			  G_CALLBACK (track_name_edited), td);
	col = gtk_tree_view_column_new_with_attributes (_("Title"), cell,
							"text", MODEL_NAME,
							"editable", MODEL_EDITABLE,
							NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (td->tracks), col);

	cell = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (_("Length"), cell,
							"text", MODEL_LENGTH, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (td->tracks), col);

	gtk_container_add (GTK_CONTAINER (sw), td->tracks);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 2);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);

	/* More advanced options */
	advanced = cddb_disclosure_new (_("Show advanced track options"),
					_("Hide advanced track options"));
	gtk_box_pack_start (GTK_BOX (vbox), advanced, FALSE, FALSE, 0);

	ad_vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (vbox), ad_vbox2, TRUE, TRUE, 0);
	cddb_disclosure_set_container (CDDB_DISCLOSURE (advanced), ad_vbox2);

	/* Extra data */
	label = gtk_label_new_with_mnemonic (_("_Extra track data:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (ad_vbox2), label, FALSE, FALSE, 0);

	td->extra_info = gtk_text_view_new ();
	td->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (td->extra_info));
	g_signal_connect (G_OBJECT (td->buffer), "changed",
			  G_CALLBACK (extra_info_changed), td);
	gtk_widget_set_sensitive (td->extra_info, FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (td->extra_info),
				     GTK_WRAP_WORD);
	gtk_box_pack_start (GTK_BOX (ad_vbox2), td->extra_info, TRUE, TRUE, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), td->extra_info);
	set_relation (td->extra_info, label);

	/* Special show hide all the stuff we want */
	gtk_widget_show_all (td->parent);
	gtk_widget_hide (ad_vbox);
	gtk_widget_hide (ad_vbox2);

	return td;
}

/* Implement GNOME::Media::CDDBTrackEditor */
#define PARENT_TYPE BONOBO_OBJECT_TYPE
static BonoboObjectClass *parent_class = NULL;

typedef struct _CDDBTrackEditor {
	BonoboObject parent;

	TrackEditorDialog *td;
	GtkWidget *dialog;
	char *discid;
} CDDBTrackEditor;

typedef struct _CDDBTrackEditorClass {
	BonoboObjectClass parent_class;

	POA_GNOME_Media_CDDBTrackEditor__epv epv;
} CDDBTrackEditorClass;

static inline CDDBTrackEditor *
cddb_track_editor_from_servant (PortableServer_Servant servant)
{
	return (CDDBTrackEditor *) bonobo_object_from_servant (servant);
}

static void
sync_cddb_info (CDDBInfo *info)
{
	g_return_if_fail (info != NULL);

	g_print ("DEBUG: sync: setting disc title: %s\n", info->title);
	cddb_slave_client_set_disc_title (client, info->discid, info->title);
	cddb_slave_client_set_artist (client, info->discid, info->artist);

	/* Only set the comment if there is one */
	if (info->comment != NULL && *(info->comment) != 0) {
		cddb_slave_client_set_comment (client, info->discid, info->comment);
	}

	if (info->genre != NULL && *(info->genre) != 0) {
		cddb_slave_client_set_genre (client, info->discid, info->genre);
	}

	if (info->year != -1) {
		cddb_slave_client_set_year (client, info->discid, info->year);
	}

	cddb_slave_client_set_tracks (client, info->discid, info->track_info);
}

static void
dialog_response (GtkDialog *dialog,
		 int response_id,
		 CDDBTrackEditor *editor)
{
	GError *error = NULL;
	switch (response_id) {
	case 1:
		g_print ("Saving...\n");
		sync_cddb_info (editor->td->info);
		cddb_slave_client_save (client, editor->discid);

		gtk_widget_destroy (GTK_WIDGET (dialog));
		editor->dialog = NULL;
		break;

	case GTK_RESPONSE_HELP:
		gnome_help_display ("gnome-cd", "gtcd_edit_info", &error);
		if (error) {
			GtkWidget *msg_dialog;
			msg_dialog = gtk_message_dialog_new (GTK_WINDOW(dialog),
							     GTK_DIALOG_DESTROY_WITH_PARENT,
							     GTK_MESSAGE_ERROR,
							     GTK_BUTTONS_CLOSE,
							     ("There was an error displaying help: \n%s"),
							     error->message);
			g_signal_connect (G_OBJECT (msg_dialog), "response",
					  G_CALLBACK (gtk_widget_destroy),
					  NULL);

			gtk_window_set_resizable (GTK_WINDOW (msg_dialog), FALSE);
			gtk_widget_show (msg_dialog);
			g_error_free (error);
		}
		break;

	default:
		gtk_widget_destroy (GTK_WIDGET (dialog));
		editor->dialog = NULL;
		break;
	}
}

static void
impl_GNOME_Media_CDDBTrackEditor_showWindow (PortableServer_Servant servant,
					     CORBA_Environment *ev)
{
	CDDBTrackEditor *editor;

	editor = cddb_track_editor_from_servant (servant);
	if (editor->dialog == NULL) {
		editor->td = make_track_editor_control ();
		if (editor->discid != NULL) {
			load_new_track_data (editor->td, editor->discid);
		}

		editor->dialog = gtk_dialog_new_with_buttons (_("CDDB Track Editor"),
							      NULL,
							      GTK_DIALOG_NO_SEPARATOR,
							      GTK_STOCK_HELP,
							      GTK_RESPONSE_HELP,
							      GTK_STOCK_CANCEL,
							      GTK_RESPONSE_CANCEL,
							      GTK_STOCK_SAVE,
							      1, NULL);
		g_signal_connect (G_OBJECT (editor->dialog), "response",
				  G_CALLBACK (dialog_response), editor);

		gtk_window_set_default_size (GTK_WINDOW (editor->dialog), 640, 400);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (editor->dialog)->vbox),
				    editor->td->parent, TRUE, TRUE, 0);
		gtk_widget_show (editor->dialog);
	} else {
		gdk_window_show (editor->dialog->window);
		gdk_window_raise (editor->dialog->window);
	}
}

static void
impl_GNOME_Media_CDDBTrackEditor_setDiscID (PortableServer_Servant servant,
					    const CORBA_char *discid,
					    CORBA_Environment *ev)
{
	CDDBTrackEditor *editor;

	editor = cddb_track_editor_from_servant (servant);
	if (editor->discid != NULL &&
	    strcmp (editor->discid, discid) == 0) {
		return;
	}

	if (editor->discid != NULL) {
		g_free (editor->discid);
	}
	editor->discid = g_strdup (discid);

	if (editor->td != NULL) {
		if (editor->td->info != NULL) {
			/* FIXME: we still have data, so we should
                         * throw up a dialog, instead of freeing old info
			 * see #105703 */
			free_track_info (editor->td);
		}
		load_new_track_data (editor->td, discid);
	}
}

static void
finalise (GObject *object)
{
	CDDBTrackEditor *editor;

	editor = (CDDBTrackEditor *) object;

	if (editor->dialog == NULL) {
		return;
	}

	gtk_widget_destroy (editor->dialog);
	editor->dialog = NULL;

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
cddb_track_editor_class_init (CDDBTrackEditorClass *klass)
{
	GObjectClass *object_class;
	POA_GNOME_Media_CDDBTrackEditor__epv *epv = &klass->epv;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = finalise;

	parent_class = g_type_class_peek_parent (klass);

	epv->showWindow = impl_GNOME_Media_CDDBTrackEditor_showWindow;
	epv->setDiscID = impl_GNOME_Media_CDDBTrackEditor_setDiscID;
}

static void
cddb_track_editor_init (CDDBTrackEditor *editor)
{
	editor->td = NULL;
	editor->dialog = NULL;
	editor->discid = NULL;
}

BONOBO_TYPE_FUNC_FULL (CDDBTrackEditor, GNOME_Media_CDDBTrackEditor,
		       PARENT_TYPE, cddb_track_editor);

static void
track_editor_destroy_cb (GObject *editor,
			 gpointer data)
{
	running_objects--;

	if (running_objects <= 0) {
		if (client != NULL) {
			g_object_unref (G_OBJECT (client));
		}

		bonobo_main_quit ();
	}
}

static BonoboObject *
factory_fn (BonoboGenericFactory *factory,
	    const char *component_id,
	    void *closure)
{
	CDDBTrackEditor *editor;

	editor = g_object_new (cddb_track_editor_get_type (), NULL);

	/* Keep track of our objects */
	running_objects++;
	g_signal_connect (G_OBJECT (editor), "destroy",
			  G_CALLBACK (track_editor_destroy_cb), NULL);

	return BONOBO_OBJECT (editor);
}

static gboolean
track_editor_init (gpointer data)
{
	BonoboGenericFactory *factory;
	char *display_iid;

	display_iid = bonobo_activation_make_registration_id (CDDBSLAVE_TRACK_EDITOR_IID, DisplayString (gdk_display));
	factory = bonobo_generic_factory_new (display_iid,
					      factory_fn, NULL);
	g_free (display_iid);
	if (factory == NULL) {
		g_print (_("Cannot create CDDBTrackEditor factory.\n"
			   "This may be caused by another copy of cddb-track-editor already running.\n"));
		exit (1);
	}

	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));
	return FALSE;
}

/*
 * set_relation
 * @widget : The Gtk widget which is labelled by @label
 * @label : The label for the @widget.
 * Description : This function establishes atk relation
 * between a gtk widget and a label.
 */
static void
set_relation (GtkWidget *widget, GtkWidget *label)
{
	AtkObject *aobject;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *targets[1];

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (GTK_IS_WIDGET (label));

	aobject = gtk_widget_get_accessible (widget);

	/* Return if GAIL is not loaded */
	if (! GTK_IS_ACCESSIBLE (aobject))
		return;

	targets[0] = gtk_widget_get_accessible (label);

	relation_set = atk_object_ref_relation_set (aobject);

	relation = atk_relation_new (targets, 1, ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
}

int
main (int argc,
      char **argv)
{
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init ("gnome-cd", VERSION, LIBGNOMEUI_MODULE,
			    argc, argv, GNOME_PARAM_APP_DATADIR, DATADIR,NULL);

	g_idle_add (track_editor_init, NULL);
	bonobo_main ();

	exit (0);
}
