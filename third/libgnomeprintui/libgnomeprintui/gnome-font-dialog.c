/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gnome-font-dialog.c: A font selector dialog
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors:
 *    Chris Lahey <clahey@ximian.com>
 *    Lauris Kaplinski <lauris@ximian.com>
 *
 *  Copyright (C) 2000-2002 Ximian Inc.
 *
 */

#include <config.h>

#include <string.h>
#include <stdlib.h>
#include <atk/atk.h>
#include <gtk/gtk.h>
#include <libgnomeprint/gnome-pgl.h>

#include "gnome-print-i18n.h"
#include "gnome-font-dialog.h"
#include "gnome-printui-marshal.h"

struct _GnomeFontSelection
{
	GtkHBox hbox;
  
	GtkWidget * family;

	GtkWidget * fontbox;

	GtkWidget * stylebox;
	GtkWidget * style;

	GtkWidget * sizebox;
	GtkWidget * size;

	GtkWidget * previewframe;

	guchar *selectedfamily;
	GnomeFontFace *selectedface;
	GnomeFont *selectedfont;
	gdouble selectedsize;
};


struct _GnomeFontSelectionClass
{
	GtkHBoxClass parent_class;

	void (* font_set) (GnomeFontSelection * fontsel, GnomeFont * font);
};

enum {FONT_SET, LAST_SIGNAL};

/* This is the initial and maximum height of the preview entry (it expands
   when large font sizes are selected). Initial height is also the minimum. */
#define MIN_PREVIEW_HEIGHT 44
#define MAX_PREVIEW_HEIGHT 300

/* These are what we use as the standard font sizes, for the size clist.
   Note that when using points we still show these integer point values but
   we work internally in decipoints (and decipoint values can be typed in). */
static gchar * font_sizes[] = {
  "8", "9", "10", "11", "12", "13", "14", "16", "18", "20", "22", "24", "26", "28",
  "32", "36", "40", "48", "56", "64", "72"
};

static void gnome_font_selection_class_init (GnomeFontSelectionClass *klass);
static void gnome_font_selection_init (GnomeFontSelection *fontsel);
static void gnome_font_selection_destroy (GtkObject *object);

static void gnome_font_selection_select_family (GtkTreeSelection *selection, gpointer data);
static void gnome_font_selection_select_style  (GtkTreeSelection *selection, gpointer data);
static void gnome_font_selection_select_size   (GtkEditable * editable, gpointer data);
static void gnome_font_selection_fill_families (GnomeFontSelection * fontsel);
static void gnome_font_selection_fill_styles   (GnomeFontSelection * fontsel);
static gboolean find_row_to_select_cb (GtkTreeModel *model,
				       GtkTreePath *path,
				       GtkTreeIter *iter,
				       gpointer user_data);

static GtkHBoxClass *gfs_parent_class = NULL;
static guint gfs_signals[LAST_SIGNAL] = {0};

GType
gnome_font_selection_get_type ()
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomeFontSelectionClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_selection_class_init,
			NULL, NULL,
			sizeof (GnomeFontSelection),
			0,
			(GInstanceInitFunc) gnome_font_selection_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_HBOX, "GnomeFontSelection", &info, 0);
	}
	return type;
}

static void
gnome_font_selection_class_init (GnomeFontSelectionClass *klass)
{
	GtkObjectClass *gtk_object_class;
	GObjectClass *object_class;
  
	object_class = (GObjectClass *) klass;
	gtk_object_class = (GtkObjectClass *) klass;
  
	gfs_parent_class = gtk_type_class (GTK_TYPE_HBOX);
  
	gfs_signals[FONT_SET] = g_signal_new ("font_set",
					      G_OBJECT_CLASS_TYPE (object_class),
					      G_SIGNAL_RUN_FIRST,
					      G_STRUCT_OFFSET (GnomeFontSelectionClass, font_set),
					      NULL, NULL,
					      libgnomeprintui_marshal_VOID__POINTER,
					      G_TYPE_NONE,
					      1,
					      G_TYPE_POINTER);

	gtk_object_class->destroy = gnome_font_selection_destroy;
}

static void
gnome_font_selection_init (GnomeFontSelection * fontsel)
{
	static GList * sizelist = NULL;
	GtkWidget * f, * sw, * tv, * vb, * hb, * c, * l;
	GtkListStore *store;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *col;
	GtkCellRenderer *rend;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *relation_targets[1];
	AtkObject *atko;

	gtk_box_set_homogeneous ((GtkBox *) fontsel, TRUE);
	gtk_box_set_spacing ((GtkBox *) fontsel, 4);

	/* Family frame */

	f = gtk_frame_new (_("Font family"));
	gtk_widget_show (f);
	gtk_box_pack_start ((GtkBox *) fontsel, f, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width ((GtkContainer *) sw, 4);
	gtk_scrolled_window_set_policy ((GtkScrolledWindow *) sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type ((GtkScrolledWindow *) sw,
					     GTK_SHADOW_IN);
	gtk_widget_show (sw);
	gtk_container_add ((GtkContainer *) f, sw);

	store = gtk_list_store_new (1, G_TYPE_STRING);
	tv = gtk_tree_view_new_with_model ((GtkTreeModel*) store);
	selection = gtk_tree_view_get_selection ((GtkTreeView*) tv);
	gtk_tree_selection_set_mode ((GtkTreeSelection*) selection,
				     GTK_SELECTION_SINGLE);
	g_object_unref ((GObject*) store);
	gtk_tree_view_set_headers_visible ((GtkTreeView*) tv, FALSE);

	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (NULL, rend,
							"text", 0,
							NULL);
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
	gtk_widget_show (tv);
	g_signal_connect ((GObject*) selection, "changed",
			  GTK_SIGNAL_FUNC (gnome_font_selection_select_family),
			  fontsel);
	gtk_container_add ((GtkContainer *) sw, tv);
	fontsel->family = tv;
	fontsel->selectedfamily = NULL;

	atko = gtk_widget_get_accessible (tv);
	atk_object_set_name (atko, _("Font family"));
	atk_object_set_description (atko, _("The list of font families available"));

	/* Fontbox */
	vb = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vb);
	gtk_box_pack_start ((GtkBox *) fontsel, vb, TRUE, TRUE, 0);
	fontsel->fontbox = vb;

	/* Style frame */
	f = gtk_frame_new (_("Style"));
	gtk_widget_show (f);
	gtk_box_pack_start ((GtkBox *) vb, f, TRUE, TRUE, 0);

	/* Stylebox */
	vb = gtk_vbox_new (FALSE, 4);
	gtk_container_set_border_width ((GtkContainer *) vb, 4);
	gtk_widget_show (vb);
	gtk_container_add ((GtkContainer *) f, vb);
	fontsel->stylebox = vb;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy ((GtkScrolledWindow *) sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type ((GtkScrolledWindow *) sw,
					     GTK_SHADOW_IN);
	gtk_widget_show (sw);
	gtk_box_pack_start ((GtkBox *) vb, sw, TRUE, TRUE, 0);

	store = gtk_list_store_new (1, G_TYPE_STRING);
	tv = gtk_tree_view_new_with_model ((GtkTreeModel*) store);
	selection = gtk_tree_view_get_selection ((GtkTreeView*) tv);
	gtk_tree_selection_set_mode ((GtkTreeSelection*) selection,
				     GTK_SELECTION_SINGLE);
	g_object_unref ((GObject*) store);
	gtk_tree_view_set_headers_visible ((GtkTreeView*) tv, FALSE);

	rend = gtk_cell_renderer_text_new ();
	col = gtk_tree_view_column_new_with_attributes (NULL, rend,
							"text", 0,
							NULL);
	gtk_tree_view_column_set_sizing (col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tv), col);
	gtk_widget_show (tv);
	g_signal_connect ((GObject*) selection, "changed",
			  GTK_SIGNAL_FUNC (gnome_font_selection_select_style),
			  fontsel);
	gtk_container_add ((GtkContainer *) sw, tv);
	fontsel->style = tv;
	fontsel->selectedface = NULL;

	atko = gtk_widget_get_accessible (tv);
	atk_object_set_name (atko, _("Font style"));
	atk_object_set_description (atko, _("The list of styles available for the selected font family"));

	/* Sizebox */

	hb = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hb);
	gtk_box_pack_start ((GtkBox *) vb, hb, FALSE, FALSE, 0);
	fontsel->sizebox = hb;

	c = gtk_combo_new ();
	gtk_widget_set_size_request (c, 64, -1);
	gtk_combo_set_value_in_list ((GtkCombo *) c, FALSE, FALSE);
	gtk_combo_set_use_arrows ((GtkCombo *) c, TRUE);
	gtk_combo_set_use_arrows_always ((GtkCombo *) c, TRUE);
	gtk_widget_show (c);
	g_signal_connect (G_OBJECT (((GtkCombo *) c)->entry), "changed",
			  (GCallback) gnome_font_selection_select_size, fontsel);
	gtk_box_pack_end ((GtkBox *) hb, c, FALSE, FALSE, 0);
	fontsel->size = c;

	if (!sizelist) {
		gint i;
		for (i = 0; i < (sizeof (font_sizes) / sizeof (font_sizes[0])); i++) {
			sizelist = g_list_prepend (sizelist, font_sizes[i]);
		}
		sizelist = g_list_reverse (sizelist);
	}

	gtk_combo_set_popdown_strings ((GtkCombo *) c, sizelist);

	gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) c)->entry, "12");
	fontsel->selectedsize = 12.0;

	l = gtk_label_new_with_mnemonic (_("Font _size:"));
	gtk_widget_show (l);
	gtk_box_pack_end ((GtkBox *) hb, l, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget ((GtkLabel *) l,
				       ((GtkCombo *) c)->entry);

	/* Add a LABELLED_BY relation from the combo to the label. */
	atko = gtk_widget_get_accessible (c);
	relation_set = atk_object_ref_relation_set (atko);
	relation_targets[0] = gtk_widget_get_accessible (l);
	relation = atk_relation_new (relation_targets, 1,
				     ATK_RELATION_LABELLED_BY);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
	g_object_unref (G_OBJECT (relation_set));
}

static void
gnome_font_selection_destroy (GtkObject *object)
{
	GnomeFontSelection *fontsel;
  
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_FONT_SELECTION (object));
  
	fontsel = GNOME_FONT_SELECTION (object);

	if (fontsel->selectedfont) {
		gnome_font_unref (fontsel->selectedfont);
		fontsel->selectedfont = NULL;
	}

	if (fontsel->selectedface) {
		gnome_font_face_unref (fontsel->selectedface);
		fontsel->selectedface = NULL;
	}

	if (fontsel->selectedfamily) {
		g_free (fontsel->selectedfamily);
		fontsel->selectedfamily = NULL;
	}

  	if (GTK_OBJECT_CLASS (gfs_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (gfs_parent_class)->destroy) (object);
}

GtkWidget *
gnome_font_selection_new ()
{
	GnomeFontSelection * fontsel;
	GtkTreeModel *model;
	GtkTreeIter iter;

	fontsel = g_object_new (GNOME_TYPE_FONT_SELECTION, NULL);
  
	gnome_font_selection_fill_families (fontsel);

	/* Select first font in family list */

	model = gtk_tree_view_get_model ((GtkTreeView*) fontsel->family);
	if (gtk_tree_model_get_iter_first (model, &iter)) {
		GtkTreeSelection *selection;
		selection = gtk_tree_view_get_selection ((GtkTreeView*) fontsel->family);
		gtk_tree_selection_select_iter (selection, &iter);
	}

	return GTK_WIDGET (fontsel);
}

static void
gnome_font_selection_select_family (GtkTreeSelection *selection, gpointer data)
{
	GnomeFontSelection * fontsel;
	GtkTreeView *tree_view;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GValue value = { 0 };
	const gchar * familyname;

	fontsel = GNOME_FONT_SELECTION (data);

	tree_view = gtk_tree_selection_get_tree_view (selection);
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

	gtk_tree_model_get_value (model, &iter, 0, &value);
	familyname = g_value_get_string (&value);

	if (fontsel->selectedfamily) {
		g_free (fontsel->selectedfamily);
	}

	if (familyname) {
		fontsel->selectedfamily = g_strdup (familyname);
	} else {
		fontsel->selectedfamily = NULL;
	}

	g_value_unset(&value);

	gnome_font_selection_fill_styles (fontsel);
}

static void
gnome_font_selection_select_style (GtkTreeSelection *selection, gpointer data)
{
	GnomeFontSelection * fontsel;
	GtkTreeView *tree_view;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GValue value = { 0 };
	const gchar * style;

	fontsel = GNOME_FONT_SELECTION (data);

	if (!fontsel->selectedfamily)
		return;

	tree_view = gtk_tree_selection_get_tree_view (selection);
	if (!gtk_tree_selection_get_selected (selection, &model, &iter))
		return;

	gtk_tree_model_get_value (model, &iter, 0, &value);
	style = g_value_get_string (&value);

	if (fontsel->selectedface)
		gnome_font_face_unref (fontsel->selectedface);
	fontsel->selectedface = gnome_font_face_find_from_family_and_style (fontsel->selectedfamily, style);

	if (fontsel->selectedfont)
		gnome_font_unref (fontsel->selectedfont);
	fontsel->selectedfont = gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);

	g_value_unset(&value);

	g_signal_emit (G_OBJECT (fontsel), gfs_signals[FONT_SET], 0, fontsel->selectedfont);
}

static void
gnome_font_selection_select_size (GtkEditable * editable, gpointer data)
{
	GnomeFontSelection * fontsel;
	gchar * sizestr;

	fontsel = GNOME_FONT_SELECTION (data);

	if (!fontsel->selectedface)
		return;

	sizestr = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (fontsel->size)->entry), 0, -1);

	fontsel->selectedsize = MAX (atoi (sizestr), 1.0);

	g_free (sizestr);

	if (fontsel->selectedfont)
		gnome_font_unref (fontsel->selectedfont);
	fontsel->selectedfont = gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);

	g_signal_emit (GTK_OBJECT (fontsel), gfs_signals[FONT_SET], 0, fontsel->selectedfont);
}

static void
gnome_font_selection_fill_families (GnomeFontSelection * fontsel)
{
	GList * families, * l;
	GtkListStore *store;

	families = gnome_font_family_list ();
	g_return_if_fail (families != NULL);

	store = (GtkListStore*) gtk_tree_view_get_model ((GtkTreeView*) fontsel->family);

	gtk_list_store_clear (store);

	for (l = families; l != NULL; l = l->next) {
		GtkTreeIter iter;

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, l->data, -1);
	}

	gnome_font_family_list_free (families);
}

static void
gnome_font_selection_fill_styles (GnomeFontSelection * fontsel)
{
	GList * styles, * l;
	GtkListStore *store;
	GtkTreeIter iter;

	store = (GtkListStore*) gtk_tree_view_get_model ((GtkTreeView*) fontsel->style);

	gtk_list_store_clear (store);

	if (fontsel->selectedfamily) {
		styles = gnome_font_style_list (fontsel->selectedfamily);
		for (l = styles; l != NULL; l = l->next) {
			GtkTreeIter iter;

			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 0, l->data, -1);
		}
		gnome_font_style_list_free (styles);
	}

	/* Select first font in style list */
	if (gtk_tree_model_get_iter_first ((GtkTreeModel *) store, &iter)) {
		GtkTreeSelection *selection;
		selection = gtk_tree_view_get_selection ((GtkTreeView*) fontsel->style);
		gtk_tree_selection_select_iter (selection, &iter);
	}
}


/*****************************************************************************
 * These functions are the main public interface for getting/setting the font.
 *****************************************************************************/

gdouble
gnome_font_selection_get_size (GnomeFontSelection * fontsel)
{
	return fontsel->selectedsize;
}

GnomeFont*
gnome_font_selection_get_font (GnomeFontSelection * fontsel)
{
	g_return_val_if_fail (fontsel != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_SELECTION (fontsel), NULL);

	if (!fontsel->selectedface)
		return NULL;

	return gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);
}

GnomeFontFace *
gnome_font_selection_get_face (GnomeFontSelection * fontsel)
{
	g_return_val_if_fail (fontsel != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_SELECTION (fontsel), NULL);

	if (fontsel->selectedface)
		gnome_font_face_ref (fontsel->selectedface);

	return fontsel->selectedface;
}

typedef struct _GnomeFontSelectionSetFontData GnomeFontSelectionSetFontData;
struct _GnomeFontSelectionSetFontData
{
	GtkTreeSelection *selection;
	const gchar *row_to_select;
};

void
gnome_font_selection_set_font (GnomeFontSelection * fontsel, GnomeFont * font)
{
	const GnomeFontFace * face;
	const gchar * familyname, * stylename;
	gdouble size;
	GtkTreeModel *model;
	GnomeFontSelectionSetFontData data;
	gchar b[32];

	g_return_if_fail (fontsel != NULL);
	g_return_if_fail (GNOME_IS_FONT_SELECTION (fontsel));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	face = gnome_font_get_face (font);
	familyname = gnome_font_face_get_family_name (face);
	stylename = gnome_font_face_get_species_name (face);
	size = gnome_font_get_size (font);

	/* Select the matching family. */
	model = gtk_tree_view_get_model ((GtkTreeView*) fontsel->family);
	data.selection = gtk_tree_view_get_selection ((GtkTreeView*) fontsel->family);
	data.row_to_select = familyname;
	gtk_tree_model_foreach (model, find_row_to_select_cb, &data);

	/* Select the matching style. */
	model = gtk_tree_view_get_model ((GtkTreeView*) fontsel->style);
	data.selection = gtk_tree_view_get_selection ((GtkTreeView*) fontsel->style);
	data.row_to_select = stylename;
	gtk_tree_model_foreach (model, find_row_to_select_cb, &data);

	g_snprintf (b, 32, "%2.1f", size);
	b[31] = '\0';
	gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) fontsel->size)->entry, b);
	fontsel->selectedsize = size;
}

static gboolean
find_row_to_select_cb (GtkTreeModel *model,
		       GtkTreePath *path,
		       GtkTreeIter *iter,
		       gpointer user_data)
{
	GnomeFontSelectionSetFontData *data = user_data;
	gchar *family;
	gint cmp;
	
	gtk_tree_model_get (model, iter, 0, &family, -1);
	
	cmp = strcmp (family, data->row_to_select);
	g_free (family);
	
	if (cmp != 0)
		return FALSE;

	gtk_tree_selection_select_path (data->selection, path);
	return TRUE;
}


/********************************************
 * GnomeFontPreview
 ********************************************/

struct _GnomeFontPreview
{
	GtkImage image;

	guchar *text;
	GnomeFont *font;
	guint32 color;
};


struct _GnomeFontPreviewClass
{
	GtkImageClass parent_class;
};

static void gnome_font_preview_class_init (GnomeFontPreviewClass *klass);
static void gnome_font_preview_init (GnomeFontPreview * preview);
static void gnome_font_preview_destroy (GtkObject *object);
static void gnome_font_preview_update (GnomeFontPreview * preview);

static GtkImageClass *gfp_parent_class = NULL;

GType
gnome_font_preview_get_type ()
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomeFontPreviewClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_preview_class_init,
			NULL, NULL,
			sizeof (GnomeFontPreview),
			0,
			(GInstanceInitFunc) gnome_font_preview_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_IMAGE, "GnomeFontPreview", &info, 0);
	}
	return type;
}

static void
gnome_font_preview_class_init (GnomeFontPreviewClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) klass;
  
	gfp_parent_class = gtk_type_class (GTK_TYPE_IMAGE);
  
	object_class->destroy = gnome_font_preview_destroy;
}

static void
gnome_font_preview_init (GnomeFontPreview * preview)
{
	GtkWidget * w;

	w = (GtkWidget *) preview;

	preview->text = NULL;
	preview->font = NULL;
	preview->color = 0x000000ff;

	gtk_widget_set_size_request (w, 64, MIN_PREVIEW_HEIGHT);
}

static void
gnome_font_preview_destroy (GtkObject *object)
{
	GnomeFontPreview *preview;
  
	preview = (GnomeFontPreview *) object;

	if (preview->text) {
		g_free (preview->text);
		preview->text = NULL;
	}

	if (preview->font) {
		gnome_font_unref (preview->font);
		preview->font = NULL;
	}

  	if (GTK_OBJECT_CLASS (gfp_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (gfp_parent_class)->destroy) (object);
}

GtkWidget *
gnome_font_preview_new (void)
{
	GnomeFontPreview * preview;
  
	preview = g_object_new (GNOME_TYPE_FONT_PREVIEW, NULL);

	return GTK_WIDGET (preview);
}

void
gnome_font_preview_set_phrase (GnomeFontPreview * preview, const guchar *phrase)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));

	if (preview->text)
		g_free (preview->text);

	if (phrase) {
		preview->text = g_strdup (phrase);
	} else {
		preview->text = NULL;
	}

	gnome_font_preview_update (preview);
}


void
gnome_font_preview_set_font (GnomeFontPreview * preview, GnomeFont * font)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	gnome_font_ref (font);

	if (preview->font)
		gnome_font_unref (preview->font);

	preview->font = font;

	gnome_font_preview_update (preview);
}

void
gnome_font_preview_set_color (GnomeFontPreview * preview, guint32 color)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));

	preview->color = color;

	gnome_font_preview_update (preview);
}

static void
gnome_font_preview_update (GnomeFontPreview *preview)
{
	const gdouble identity[] = {1.0, 0.0, 0.0, -1.0, 0.0, 0.0};
	GdkPixbuf *pixbuf;
	gint width, height;
	GnomePosGlyphList *pgl;

	if (!preview->font) {
		width = 256;
		height = 32;
		pgl = NULL;
	} else {
		const guchar *sample;
		GnomeGlyphList *gl;
		ArtDRect bbox;

		if (preview->text) {
			sample = preview->text;
		} else {
			sample = gnome_font_face_get_sample (gnome_font_get_face (preview->font));
			if (!sample)
				sample = _("This font does not have sample");
		}
		gl = gnome_glyphlist_from_text_dumb (preview->font, preview->color, 0.0, 0.0, sample);
		pgl = gnome_pgl_from_gl (gl, identity, GNOME_PGL_RENDER_DEFAULT);
		gnome_glyphlist_unref (gl);
		gnome_pgl_bbox (pgl, &bbox);
		width = CLAMP (bbox.x1 - bbox.x0 + 32, 128, 512);
		height = CLAMP (bbox.y1 - bbox.y0 + 16, 32, 256);
	}

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, width, height);
	gdk_pixbuf_fill (pixbuf, 0xffffffff);

	if (pgl) {
		ArtDRect bbox;
		gint x, y;
		gnome_pgl_bbox (pgl, &bbox);
		x = MAX ((width - (bbox.x1 - bbox.x0)) / 2 - bbox.x0, 0);
		y = MIN (height - (height - (bbox.y1 - bbox.y0)) / 2 - bbox.y1, height);
		gnome_pgl_render_rgb8 (pgl, x, y,
				       gdk_pixbuf_get_pixels (pixbuf),
				       gdk_pixbuf_get_width (pixbuf),
				       gdk_pixbuf_get_height (pixbuf),
				       gdk_pixbuf_get_rowstride (pixbuf),
				       GNOME_PGL_RENDER_DEFAULT);
		gnome_pgl_destroy (pgl);
	}

	gtk_image_set_from_pixbuf (GTK_IMAGE (preview), pixbuf);
	g_object_unref (G_OBJECT (pixbuf));
}

/*****************************************************************************
 * GtkFontSelectionDialog
 *****************************************************************************/

struct _GnomeFontDialog {
	GtkDialog dialog;

	GtkWidget *fontsel;
	GtkWidget *preview;
};

struct _GnomeFontDialogClass {
	GtkDialogClass parent_class;
};

static void gnome_font_dialog_class_init (GnomeFontDialogClass *klass);
static void gnome_font_dialog_init (GnomeFontDialog *fontseldiag);

static void gfsd_update_preview (GnomeFontSelection * fs, GnomeFont * font, GnomeFontDialog * gfsd);

static GtkDialogClass *gfsd_parent_class = NULL;

GType
gnome_font_dialog_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (GnomeFontDialogClass),
			NULL, NULL,
			(GClassInitFunc) gnome_font_dialog_class_init,
			NULL, NULL,
			sizeof (GnomeFontDialog),
			0,
			(GInstanceInitFunc) gnome_font_dialog_init
		};
		type = g_type_register_static (GTK_TYPE_DIALOG, "GnomeFontDialog", &info, 0);
	}
	return type;
}

static void
gnome_font_dialog_class_init (GnomeFontDialogClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass*) klass;
  
	gfsd_parent_class = gtk_type_class (GTK_TYPE_DIALOG);
}

static void
gnome_font_dialog_init (GnomeFontDialog *fontseldiag)
{
	GnomeFont * font;
	AtkObject *atko;

	gtk_window_set_default_size (GTK_WINDOW (fontseldiag), 500, 300);
	
	gtk_dialog_add_button (GTK_DIALOG (fontseldiag), GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_add_button (GTK_DIALOG (fontseldiag), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	gtk_dialog_set_default_response (GTK_DIALOG (fontseldiag), GTK_RESPONSE_CANCEL);

	gtk_container_set_border_width (GTK_CONTAINER (fontseldiag), 4);

	fontseldiag->fontsel = gnome_font_selection_new ();
	gtk_widget_show (fontseldiag->fontsel);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (fontseldiag)->vbox), fontseldiag->fontsel, TRUE, TRUE, 0);

	fontseldiag->preview = gnome_font_preview_new ();
	gtk_widget_show (fontseldiag->preview);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (fontseldiag)->vbox), fontseldiag->preview, TRUE, TRUE, 0);

	atko = gtk_widget_get_accessible (fontseldiag->preview);
	atk_object_set_name (atko, _("Font Preview"));
	atk_object_set_description (atko, _("Displays some example text in the selected font"));

	font = gnome_font_selection_get_font ((GnomeFontSelection *) fontseldiag->fontsel);
	gnome_font_preview_set_font ((GnomeFontPreview *) fontseldiag->preview, font);

	g_signal_connect (G_OBJECT (fontseldiag->fontsel), "font_set",
			  (GCallback) gfsd_update_preview, fontseldiag);
}

GtkWidget*
gnome_font_dialog_new	(const gchar	  *title)
{
	GnomeFontDialog *fontseldiag;
  
	fontseldiag = g_object_new (GNOME_TYPE_FONT_DIALOG, NULL);
	gtk_window_set_title (GTK_WINDOW (fontseldiag), title ? title : _("Font Selection"));

	return GTK_WIDGET (fontseldiag);
}

GtkWidget *
gnome_font_dialog_get_fontsel (GnomeFontDialog *gfsd)
{
	g_return_val_if_fail (gfsd != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_DIALOG (gfsd), NULL);

	return gfsd->fontsel;
}

GtkWidget *
gnome_font_dialog_get_preview (GnomeFontDialog *gfsd)
{
	g_return_val_if_fail (gfsd != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_DIALOG (gfsd), NULL);

	return gfsd->preview;
}

static void
gfsd_update_preview (GnomeFontSelection * fs, GnomeFont * font, GnomeFontDialog * gfsd)
{
	gnome_font_preview_set_font ((GnomeFontPreview *) gfsd->preview, font);
}

