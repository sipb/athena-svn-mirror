/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*  This file is part of the GtkHTML library.

    Copyright (C) 2001 Ximian, Inc.
    Authors:           Radek Doulik (rodo@ximian.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <config.h>
#include <libgnome/gnome-i18n.h>
#include <string.h>
#include <glade/glade.h>
#include <gal/widgets/widget-color-combo.h>

#include "htmlclue.h"
#include "htmlcolor.h"
#include "htmlcolorset.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-table.h"
#include "htmlengine-edit-tablecell.h"
#include "htmlengine-save.h"
#include "htmlimage.h"
#include "htmltable.h"
#include "htmltablecell.h"
#include "htmlsettings.h"

#include "properties.h"
#include "cell.h"
#include "utils.h"

typedef enum
{
	CELL,
	ROW,
	COLUMN,
	TABLE
} CellScope;

typedef struct
{	
	GtkHTMLControlData *cd;
	HTMLTableCell *cell;

	GtkHTML *sample;

	gboolean   has_bg_color;
	gboolean   changed_bg_color;
	GdkColor   bg_color;
	GtkWidget *combo_bg_color;
	GtkWidget *check_bg_color;

	gboolean   has_bg_pixmap;
	gboolean   changed_bg_pixmap;
	const gchar *bg_pixmap;
	GtkWidget *entry_bg_pixmap;
	GtkWidget *check_bg_pixmap;

	gboolean        changed_halign;
	HTMLHAlignType  halign;
	GtkWidget      *option_halign;

	gboolean        changed_valign;
	HTMLVAlignType  valign;
	GtkWidget      *option_valign;

	gboolean   has_width;
	gboolean   changed_width;
	gint       width;
	gboolean   width_percent;
	GtkWidget *spin_width;
	GtkWidget *check_width;
	GtkWidget *option_width;

	gboolean   has_height;
	gboolean   changed_height;
	gint       height;
	gboolean   height_percent;
	GtkWidget *spin_height;
	GtkWidget *check_height;
	GtkWidget *option_height;

	gint cspan;
	gint rspan;
	GtkWidget *spin_cspan;
	GtkWidget *spin_rspan;

	gboolean   wrap;
	gboolean   changed_wrap;
	GtkWidget *option_wrap;

	gboolean   heading;
	gboolean   changed_heading;
	GtkWidget *option_heading;

	CellScope  scope;
	GtkWidget *option_scope;

	gboolean   disable_change;

} GtkHTMLEditCellProperties;

#define CHANGE if (!d->disable_change) gtk_html_edit_properties_dialog_change (d->cd->properties_dialog)
#define FILL   if (!d->disable_change) fill_sample (d)

static void
fill_sample (GtkHTMLEditCellProperties *d)
{
	GString *str;
	gchar *body, *bg_color, *bg_pixmap, *valign, *halign, *width, *height, *wrap, *cell, *cspan, *rspan;
	gint r, c;

	body      = html_engine_save_get_sample_body (d->cd->html->engine, NULL);

	bg_color  = d->has_bg_color
		? g_strdup_printf (" bgcolor=\"#%02x%02x%02x\"",
				   d->bg_color.red >> 8,
				   d->bg_color.green >> 8,
				   d->bg_color.blue >> 8)
		: g_strdup ("");
	bg_pixmap = d->has_bg_pixmap && d->bg_pixmap
		? g_strdup_printf (" background=\"file://%s\"", d->bg_pixmap)
		: g_strdup ("");
	halign    = d->halign != HTML_HALIGN_NONE ? g_strdup_printf (" align=\"%s\"", d->halign == HTML_HALIGN_LEFT
								     ? "left"
								     : (d->halign == HTML_HALIGN_CENTER ? "center" : "right"))
		: g_strdup ("");

	valign    = d->valign != HTML_VALIGN_MIDDLE ? g_strdup_printf (" valign=\"%s\"", d->valign == HTML_VALIGN_TOP
								       ? "top" : "bottom")
		: g_strdup ("");
	width   = d->width != 0 && d->has_width
		? g_strdup_printf (" width=\"%d%s\"", d->width, d->width_percent ? "%" : "") : g_strdup ("");
	height  = d->height != 0 && d->has_height
		? g_strdup_printf (" height=\"%d%s\"", d->height, d->height_percent ? "%" : "") : g_strdup ("");
	wrap    = d->wrap ? " nowrap" : "";

	cspan   = d->cspan > 1 ? g_strdup_printf (" colspan=%d", d->cspan) : g_strdup ("");
	rspan   = d->rspan > 1 ? g_strdup_printf (" rowspan=%d", d->rspan) : g_strdup ("");
	cell    = g_strconcat ("<", d->heading ? "th" : "td",
			       bg_color, bg_pixmap, halign, valign, width, height, cspan, rspan, wrap, ">", NULL);

	str = g_string_new (body);
	g_string_append (str, "<table border=1 cellpadding=4 cellspacing=2>");

	for (r = 0; r < d->rspan + 1; r ++) {
		g_string_append (str, "<tr>");
		for (c = 0; c < (r < d->rspan ? (r == 0 ? 3 : 2) : d->cspan + 2); c ++) {
			if ((r == 0 && c == 1)
			    || (d->scope == ROW && r == 0)
			    || (d->scope == COLUMN && c == 1)
			    || d->scope == TABLE)
				g_string_append (str, cell);
			else
				g_string_append (str, "<td>");

			if (c == 1 && r == 0) {
				g_string_append (str, _("The quick brown fox jumps over the lazy dog."));
				g_string_append (str, " ");
				g_string_append (str, _("The quick brown fox jumps over the lazy dog."));
			} else {
				g_string_append (str, "&nbsp;");
				g_string_append (str, _("Other"));
				g_string_append (str, "&nbsp;");
			}
			g_string_append (str, "</td>");
		}
		g_string_append (str, "</tr>");
	}
	g_string_append (str, "</table>");
	gtk_html_load_from_string (d->sample, str->str, -1);

	g_free (halign);
	g_free (valign);
	g_free (bg_color);
	g_free (bg_pixmap);
	g_free (body);
	g_free (cell);
	g_free (rspan);
	g_free (cspan);
	g_string_free (str, TRUE);
}

static GtkHTMLEditCellProperties *
data_new (GtkHTMLControlData *cd)
{
	GtkHTMLEditCellProperties *data = g_new0 (GtkHTMLEditCellProperties, 1);

	/* fill data */
	data->cd                = cd;
	data->cell              = NULL;

	data->bg_color          = html_colorset_get_color (data->cd->html->engine->settings->color_set,
							   HTMLBgColor)->color;
	data->bg_pixmap         = "";

	return data;
}

static void
set_has_bg_color (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_bg_color = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_bg_color));
	FILL;
	CHANGE;
	if (!d->disable_change)
		d->changed_bg_color = TRUE;
}

static void
set_has_bg_pixmap (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_bg_pixmap = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_bg_pixmap));
	FILL;
	CHANGE;
	if (!d->disable_change)
		d->changed_bg_pixmap = TRUE;
}

static void
changed_bg_color (GtkWidget *w, GdkColor *color, gboolean custom, gboolean by_user, gboolean is_default, GtkHTMLEditCellProperties *d)
{
	/* If the color was changed programatically there's not need to set things */
	if (!by_user)
		return;
		
	d->bg_color = color
		? *color
		: html_colorset_get_color (d->cd->html->engine->defaultSettings->color_set, HTMLBgColor)->color;
	if (!d->disable_change)
		d->changed_bg_color = TRUE;
	if (!d->has_bg_color)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_bg_color), TRUE);
	else {
		FILL;
		CHANGE;
	}
}

static void
changed_bg_pixmap (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->bg_pixmap = gtk_entry_get_text (GTK_ENTRY (w));
	if (!d->disable_change)
		d->changed_bg_pixmap = TRUE;
	if (!d->has_bg_pixmap && d->bg_pixmap && *d->bg_pixmap)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_bg_pixmap), TRUE);
	else {
		if (!d->bg_pixmap || !*d->bg_pixmap)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_bg_pixmap), FALSE);
		FILL;
		CHANGE;
	}
}

static void
changed_halign (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->halign = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) + HTML_HALIGN_LEFT;
	if (!d->disable_change)
		d->changed_halign = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_valign (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->valign = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) + HTML_VALIGN_TOP;
	if (!d->disable_change)
		d->changed_valign = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_cspan (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->cspan = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_cspan));
	FILL;
	CHANGE;
}

static void
changed_rspan (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->rspan = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_rspan));
	FILL;
	CHANGE;
}

static void
changed_width (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->width = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_width));
	if (!d->disable_change) {
		d->disable_change = TRUE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_width), TRUE);
		d->disable_change = FALSE;
		d->changed_width = TRUE;
	}
	FILL;
	CHANGE;
}

static void
set_has_width (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_width = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_width));
	if (!d->disable_change)
		d->changed_width = TRUE;
	FILL;
	CHANGE;
}

static void
changed_width_percent (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->width_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_width = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_height (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->height = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (d->spin_height));
	if (!d->disable_change) {
		d->disable_change = TRUE;
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_height), TRUE);
		d->disable_change = FALSE;
		d->changed_height = TRUE;
	}
	FILL;
	CHANGE;
}

static void
set_has_height (GtkWidget *check, GtkHTMLEditCellProperties *d)
{
	d->has_height = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (d->check_height));
	if (!d->disable_change)
		d->changed_height = TRUE;
	FILL;
	CHANGE;
}

static void
changed_height_percent (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->height_percent = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_height = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_wrap (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->wrap = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_wrap = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_heading (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->heading = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w))) ? TRUE : FALSE;
	if (!d->disable_change)
		d->changed_heading = TRUE;
	FILL;
	CHANGE;	
}

static void
changed_scope (GtkWidget *w, GtkHTMLEditCellProperties *d)
{
	d->scope = g_list_index (GTK_MENU_SHELL (w)->children, gtk_menu_get_active (GTK_MENU (w)));
	FILL;
	CHANGE;	
}

/*
 * FIX: set spin adjustment upper to 100000
 *      as glade cannot set it now
 */
#define UPPER_FIX(x) gtk_spin_button_get_adjustment (GTK_SPIN_BUTTON (d->spin_ ## x))->upper = 100000.0

static GtkWidget *
cell_widget (GtkHTMLEditCellProperties *d)
{
	HTMLColor *color;
	GtkWidget *cell_page;
	GladeXML *xml;

	xml = glade_xml_new (GLADE_DATADIR "/gtkhtml-editor-properties.glade", "cell_page", NULL);
	if (!xml)
		g_error (_("Could not load glade file."));

	cell_page          = glade_xml_get_widget (xml, "cell_page");

        color = html_colorset_get_color (d->cd->html->engine->defaultSettings->color_set, HTMLBgColor);
	html_color_alloc (color, d->cd->html->engine->painter);
	d->combo_bg_color = color_combo_new (NULL, _("Automatic"), &color->color,
					     color_group_fetch ("cell_bg_color", d->cd));
        g_signal_connect (d->combo_bg_color, "color_changed", G_CALLBACK (changed_bg_color), d);
	gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "table_cell_bg")),
			  d->combo_bg_color,
			  1, 2, 0, 1, 0, 0, 0, 0);

	d->check_bg_color  = glade_xml_get_widget (xml, "check_cell_bg_color");
	g_signal_connect (d->check_bg_color, "toggled", G_CALLBACK (set_has_bg_color), d);
	d->check_bg_pixmap = glade_xml_get_widget (xml, "check_cell_bg_pixmap");
	g_signal_connect (d->check_bg_pixmap, "toggled", G_CALLBACK (set_has_bg_pixmap), d);
	d->entry_bg_pixmap = glade_xml_get_widget (xml, "entry_cell_bg_pixmap");

	g_signal_connect (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap)),
			    "changed", G_CALLBACK (changed_bg_pixmap), d);

	d->option_halign = glade_xml_get_widget (xml, "option_cell_halign");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_halign)), "selection-done",
			  G_CALLBACK (changed_halign), d);
	d->option_valign = glade_xml_get_widget (xml, "option_cell_valign");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_valign)), "selection-done",
			  G_CALLBACK (changed_valign), d);

	d->spin_width   = glade_xml_get_widget (xml, "spin_cell_width");
	UPPER_FIX (width);
	g_signal_connect (d->spin_width, "value_changed", G_CALLBACK (changed_width), d);
	d->check_width  = glade_xml_get_widget (xml, "check_cell_width");
	g_signal_connect (d->check_width, "toggled", G_CALLBACK (set_has_width), d);
	d->option_width = glade_xml_get_widget (xml, "option_cell_width");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_width)), "selection-done",
			  G_CALLBACK (changed_width_percent), d);

	d->spin_height   = glade_xml_get_widget (xml, "spin_cell_height");
	UPPER_FIX (height);
	g_signal_connect (d->spin_height, "value_changed", G_CALLBACK (changed_height), d);
	d->check_height  = glade_xml_get_widget (xml, "check_cell_height");
	g_signal_connect (d->check_height, "toggled", G_CALLBACK (set_has_height), d);
	d->option_height = glade_xml_get_widget (xml, "option_cell_height");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_height)), "selection-done",
			  G_CALLBACK (changed_height_percent), d);

	d->option_wrap    = glade_xml_get_widget (xml, "option_cell_wrap");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_wrap)), "selection-done",
			  G_CALLBACK (changed_wrap), d);
	d->option_heading = glade_xml_get_widget (xml, "option_cell_style");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_heading)), "selection-done",
			  G_CALLBACK (changed_heading), d);

	d->option_scope   = glade_xml_get_widget (xml, "option_cell_scope");
	g_signal_connect (gtk_option_menu_get_menu (GTK_OPTION_MENU (d->option_scope)), "selection-done",
			  G_CALLBACK (changed_scope), d);

	d->spin_cspan   = glade_xml_get_widget (xml, "spin_cell_cspan");
	d->spin_rspan   = glade_xml_get_widget (xml, "spin_cell_rspan");
	g_signal_connect (d->spin_cspan, "value_changed", G_CALLBACK (changed_cspan), d);
	g_signal_connect (d->spin_rspan, "value_changed", G_CALLBACK (changed_rspan), d);

	gtk_box_pack_start_defaults (GTK_BOX (cell_page), sample_frame (&d->sample));

	gtk_widget_show_all (cell_page);
	gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (d->entry_bg_pixmap), FALSE);

	return cell_page;
}

static void
set_ui (GtkHTMLEditCellProperties *d)
{
	d->disable_change = TRUE;

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_bg_color), d->has_bg_color);
	/* RM2 gdk_color_alloc (gdk_window_get_colormap (GTK_WIDGET (d->cd->html)->window), &d->bg_color); */
	color_combo_set_color (COLOR_COMBO (d->combo_bg_color), &d->bg_color);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_bg_pixmap), d->has_bg_pixmap);
	gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (d->entry_bg_pixmap))),
			    d->bg_pixmap);

	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_halign), d->halign - HTML_HALIGN_LEFT);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_valign), d->valign - HTML_VALIGN_TOP);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_width), d->has_width);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_width),  d->width);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_width), d->width_percent ? 1 : 0);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d->check_height), d->has_height);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_height),  d->height);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_height), d->height_percent ? 1 : 0);

	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_wrap), d->wrap ? 1 : 0);
	gtk_option_menu_set_history (GTK_OPTION_MENU (d->option_heading), d->heading ? 1 : 0);

	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_cspan),  d->cspan);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (d->spin_rspan),  d->rspan);

	d->disable_change = FALSE;

	FILL;
}

static void
get_data (GtkHTMLEditCellProperties *d)
{
	d->cell = html_engine_get_table_cell (d->cd->html->engine);
	g_return_if_fail (d->cell);

	if (d->cell->have_bg) {
		d->has_bg_color = TRUE;
		d->bg_color     = d->cell->bg;
	}
	if (d->cell->have_bgPixmap) {
		d->has_bg_pixmap = TRUE;
		d->bg_pixmap = strncasecmp ("file://", d->cell->bgPixmap->url, 7)
			? (strncasecmp ("file://", d->cell->bgPixmap->url, 5)
			   ? d->cell->bgPixmap->url
			   : d->cell->bgPixmap->url + 5)
			: d->cell->bgPixmap->url + 7;
	}

	d->halign   = HTML_CLUE (d->cell)->halign;
	d->valign   = HTML_CLUE (d->cell)->valign;
	d->wrap     = d->cell->no_wrap;
	d->heading  = d->cell->heading;

	if (d->cell->percent_width) {
		d->width = d->cell->fixed_width;
		d->width_percent = TRUE;
		d->has_width = TRUE;
	} else if (d->cell->fixed_width) {
		d->width = d->cell->fixed_width;
		d->width_percent = FALSE;
		d->has_width = TRUE;
	}

	d->cspan = d->cell->cspan;
	d->rspan = d->cell->rspan;
}

GtkWidget *
cell_properties (GtkHTMLControlData *cd, gpointer *set_data)
{
	GtkHTMLEditCellProperties *data = data_new (cd);
	GtkWidget *rv;

	get_data (data);
	*set_data = data;
	rv        = cell_widget (data);
	set_ui (data);

	return rv;
}

static void
cell_apply_1 (HTMLTableCell *cell, GtkHTMLEditCellProperties *d)
{
	if (d->changed_bg_color)
		html_engine_table_cell_set_bg_color (d->cd->html->engine, cell, d->has_bg_color ? &d->bg_color : NULL);

	if (d->changed_bg_pixmap) {
		gchar *url = d->has_bg_pixmap ? g_strconcat ("file://", d->bg_pixmap, NULL) : NULL;

		html_engine_table_cell_set_bg_pixmap (d->cd->html->engine, cell, url);
		g_free (url);
	}

	if (d->changed_halign)
		html_engine_table_cell_set_halign (d->cd->html->engine, cell, d->halign);

	if (d->changed_valign)
		html_engine_table_cell_set_valign (d->cd->html->engine, cell, d->valign);

	if (d->changed_wrap)
		html_engine_table_cell_set_no_wrap (d->cd->html->engine, cell, d->wrap);

	if (d->changed_heading)
		html_engine_table_cell_set_heading (d->cd->html->engine, cell, d->heading);

	if (d->changed_width)
		html_engine_table_cell_set_width (d->cd->html->engine, cell,
						  d->has_width ? d->width : 0, d->has_width ? d->width_percent : FALSE);
	html_engine_set_cspan (d->cd->html->engine, d->cspan);
	html_engine_set_rspan (d->cd->html->engine, d->rspan);
}

static void
cell_apply_row (GtkHTMLEditCellProperties *d)
{
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	if (html_engine_table_goto_row (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->row)) {
		cell = html_engine_get_table_cell (e);

		while (cell && cell->row == d->cell->row) {
			if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

static void
cell_apply_col (GtkHTMLEditCellProperties *d)
{
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	if (html_engine_table_goto_col (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->col)) {
		cell = html_engine_get_table_cell (e);

		while (cell) {
			if (cell->col == d->cell->col && HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

static void
cell_apply_table (GtkHTMLEditCellProperties *d)
{
	HTMLTable *table;
	HTMLTableCell *cell;
	HTMLEngine *e = d->cd->html->engine;

	table = html_engine_get_table (e);
	if (table && html_engine_goto_table_0 (e, table)) {
		cell = html_engine_get_table_cell (e);

		while (cell) {
			if (HTML_OBJECT (cell)->parent == HTML_OBJECT (d->cell)->parent)
				cell_apply_1 (cell, d);
			html_engine_next_cell (e, FALSE);
			cell = html_engine_get_table_cell (e);
		}
	}
}

gboolean
cell_apply_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	GtkHTMLEditCellProperties *d = (GtkHTMLEditCellProperties *) get_data;
	HTMLEngine *e = d->cd->html->engine;
	gint position;

	position = e->cursor->position;

	if (html_engine_get_table_cell (e) != d->cell) {
		if (!html_engine_goto_table (e, HTML_TABLE (HTML_OBJECT (d->cell)->parent), d->cell->row, d->cell->col)) {
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new (GTK_WINDOW (d->cd->properties_dialog->dialog),
							 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
							 _("The editted cell was removed from the document.\nCannot apply your changes."));
			gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			html_cursor_jump_to_position (e->cursor, e, position);

			return FALSE;
		}
	}

	switch (d->scope) {
	case CELL:
		cell_apply_1 (d->cell, d);
		break;
	case ROW:
		cell_apply_row (d);
		break;
	case COLUMN:
		cell_apply_col (d);
		break;
	case TABLE:
		cell_apply_table (d);
	}
	html_cursor_jump_to_position (e->cursor, e, position);

	return TRUE;
}

void
cell_close_cb (GtkHTMLControlData *cd, gpointer get_data)
{
	g_free (get_data);
}
